#include "airesourcecache.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AIResourceCache* AIResourceCache::s_instance = nullptr;
QMutex AIResourceCache::s_instanceMutex;

AIResourceCache* AIResourceCache::instance() {
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new AIResourceCache();
        }
    }
    return s_instance;
}

AIResourceCache::AIResourceCache(QObject *parent)
    : QObject(parent) {
    ensureCacheDirectoryExists();
    loadCacheIndex();
    qDebug() << "AIResourceCache initialized, cache directory:" << getCacheDirectory();
}

AIResourceCache::~AIResourceCache() {
    saveCacheIndex();
}

bool AIResourceCache::hasCache(const QString &cacheKey) const {
    QMutexLocker locker(&m_mutex);
    return m_cacheIndex.contains(cacheKey);
}

QString AIResourceCache::getCachedResource(const QString &cacheKey) const {
    QMutexLocker locker(&m_mutex);

    if (!m_cacheIndex.contains(cacheKey)) {
        return QString();
    }

    const CacheEntry &entry = m_cacheIndex[cacheKey];

    // 检查文件是否存在
    if (!QFile::exists(entry.resourcePath)) {
        qWarning() << "Cached file not found:" << entry.resourcePath;
        return QString();
    }

    qDebug() << "Cache hit:" << cacheKey << "->" << entry.resourcePath;
    return entry.resourcePath;
}

void AIResourceCache::addCache(const QString &cacheKey, const QString &resourcePath) {
    QMutexLocker locker(&m_mutex);

    if (!QFile::exists(resourcePath)) {
        qWarning() << "Cannot cache non-existent file:" << resourcePath;
        return;
    }

    QFileInfo fileInfo(resourcePath);

    CacheEntry entry;
    entry.cacheKey = cacheKey;
    entry.resourcePath = resourcePath;
    entry.createdTime = QDateTime::currentDateTime();
    entry.fileSize = fileInfo.size();

    m_cacheIndex[cacheKey] = entry;

    qDebug() << "Cache added:" << cacheKey << "->" << resourcePath
             << "Size:" << entry.fileSize << "bytes";

    // 保存索引
    saveCacheIndex();
}

void AIResourceCache::removeCache(const QString &cacheKey) {
    QMutexLocker locker(&m_mutex);

    if (!m_cacheIndex.contains(cacheKey)) {
        return;
    }

    const CacheEntry &entry = m_cacheIndex[cacheKey];

    // 删除文件
    if (QFile::exists(entry.resourcePath)) {
        QFile::remove(entry.resourcePath);
        qDebug() << "Cached file removed:" << entry.resourcePath;
    }

    // 从索引中移除
    m_cacheIndex.remove(cacheKey);

    // 保存索引
    saveCacheIndex();
}

void AIResourceCache::clearAllCache() {
    QMutexLocker locker(&m_mutex);

    // 删除所有缓存文件
    for (const CacheEntry &entry : m_cacheIndex) {
        if (QFile::exists(entry.resourcePath)) {
            QFile::remove(entry.resourcePath);
        }
    }

    m_cacheIndex.clear();

    qDebug() << "All cache cleared";

    // 保存索引
    saveCacheIndex();
}

qint64 AIResourceCache::getTotalCacheSize() const {
    QMutexLocker locker(&m_mutex);

    qint64 totalSize = 0;
    for (const CacheEntry &entry : m_cacheIndex) {
        totalSize += entry.fileSize;
    }

    return totalSize;
}

int AIResourceCache::getCacheCount() const {
    QMutexLocker locker(&m_mutex);
    return m_cacheIndex.size();
}

void AIResourceCache::cleanOldCache(int daysToKeep) {
    QMutexLocker locker(&m_mutex);

    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-daysToKeep);

    QStringList keysToRemove;
    for (auto it = m_cacheIndex.constBegin(); it != m_cacheIndex.constEnd(); ++it) {
        if (it.value().createdTime < cutoffTime) {
            keysToRemove.append(it.key());
        }
    }

    locker.unlock();

    // 删除过期缓存
    for (const QString &key : keysToRemove) {
        removeCache(key);
    }

    if (!keysToRemove.isEmpty()) {
        qDebug() << "Cleaned" << keysToRemove.size() << "old cache entries";
    }
}

void AIResourceCache::cleanCacheBySize(qint64 maxSizeBytes) {
    QMutexLocker locker(&m_mutex);

    qint64 currentSize = 0;
    for (const CacheEntry &entry : m_cacheIndex) {
        currentSize += entry.fileSize;
    }

    if (currentSize <= maxSizeBytes) {
        return;
    }

    // 按创建时间排序，删除最旧的
    QList<CacheEntry> entries = m_cacheIndex.values();
    std::sort(entries.begin(), entries.end(), [](const CacheEntry &a, const CacheEntry &b) {
        return a.createdTime < b.createdTime;
    });

    QStringList keysToRemove;
    for (const CacheEntry &entry : entries) {
        if (currentSize <= maxSizeBytes) {
            break;
        }
        keysToRemove.append(entry.cacheKey);
        currentSize -= entry.fileSize;
    }

    locker.unlock();

    // 删除缓存
    for (const QString &key : keysToRemove) {
        removeCache(key);
    }

    if (!keysToRemove.isEmpty()) {
        qDebug() << "Cleaned" << keysToRemove.size() << "cache entries to reduce size";
    }
}

QString AIResourceCache::getCacheDirectory() const {
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return cacheDir + "/AIGenerated";
}

void AIResourceCache::ensureCacheDirectoryExists() {
    QString cacheDir = getCacheDirectory();
    QDir dir;
    if (!dir.exists(cacheDir)) {
        dir.mkpath(cacheDir);
        qDebug() << "Created cache directory:" << cacheDir;
    }
}

QString AIResourceCache::getCacheIndexFilePath() const {
    return getCacheDirectory() + "/cache_index.json";
}

void AIResourceCache::loadCacheIndex() {
    QString indexFilePath = getCacheIndexFilePath();

    if (!QFile::exists(indexFilePath)) {
        qDebug() << "No cache index file found, starting with empty cache";
        return;
    }

    QFile file(indexFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open cache index file:" << indexFilePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "Invalid cache index format";
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray entries = root["entries"].toArray();

    m_cacheIndex.clear();

    for (const QJsonValue &value : entries) {
        QJsonObject obj = value.toObject();

        CacheEntry entry;
        entry.cacheKey = obj["cacheKey"].toString();
        entry.resourcePath = obj["resourcePath"].toString();
        entry.createdTime = QDateTime::fromString(obj["createdTime"].toString(), Qt::ISODate);
        entry.fileSize = obj["fileSize"].toVariant().toLongLong();

        // 只加载存在的文件
        if (QFile::exists(entry.resourcePath)) {
            m_cacheIndex[entry.cacheKey] = entry;
        } else {
            qDebug() << "Cached file not found, skipping:" << entry.resourcePath;
        }
    }

    qDebug() << "Loaded" << m_cacheIndex.size() << "cache entries";
}

void AIResourceCache::saveCacheIndex() {
    QString indexFilePath = getCacheIndexFilePath();

    QJsonArray entries;
    for (const CacheEntry &entry : m_cacheIndex) {
        QJsonObject obj;
        obj["cacheKey"] = entry.cacheKey;
        obj["resourcePath"] = entry.resourcePath;
        obj["createdTime"] = entry.createdTime.toString(Qt::ISODate);
        obj["fileSize"] = QJsonValue::fromVariant(entry.fileSize);
        entries.append(obj);
    }

    QJsonObject root;
    root["version"] = 1;
    root["entries"] = entries;

    QJsonDocument doc(root);

    QFile file(indexFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save cache index:" << indexFilePath;
        return;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Cache index saved:" << m_cacheIndex.size() << "entries";
}
