#ifndef AIRESOURCECACHE_H
#define AIRESOURCECACHE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QMutex>
#include <QDateTime>

/**
 * @brief 缓存条目结构
 */
struct CacheEntry {
    QString resourcePath;      // 资源文件路径
    QDateTime createdTime;     // 创建时间
    qint64 fileSize;           // 文件大小（字节）
    QString cacheKey;          // 缓存键

    CacheEntry() : fileSize(0) {}
};

/**
 * @brief AI 资源缓存管理器
 *
 * 单例模式，负责管理 AI 生成资源的缓存，功能包括：
 * - 基于参数哈希的缓存 Key 管理
 * - 缓存索引持久化
 * - 缓存读写
 * - 缓存清理（按时间或大小）
 * - 线程安全
 */
class AIResourceCache : public QObject {
    Q_OBJECT

public:
    // 获取单例实例
    static AIResourceCache* instance();

    // 缓存操作
    bool hasCache(const QString &cacheKey) const;
    QString getCachedResource(const QString &cacheKey) const;
    void addCache(const QString &cacheKey, const QString &resourcePath);
    void removeCache(const QString &cacheKey);
    void clearAllCache();

    // 缓存管理
    qint64 getTotalCacheSize() const;
    int getCacheCount() const;
    void cleanOldCache(int daysToKeep = 7);
    void cleanCacheBySize(qint64 maxSizeBytes);

    // 缓存目录
    QString getCacheDirectory() const;

private:
    explicit AIResourceCache(QObject *parent = nullptr);
    ~AIResourceCache();

    // 禁止拷贝
    AIResourceCache(const AIResourceCache&) = delete;
    AIResourceCache& operator=(const AIResourceCache&) = delete;

    // 缓存索引管理
    void loadCacheIndex();
    void saveCacheIndex();
    QString getCacheIndexFilePath() const;

    // 辅助方法
    void ensureCacheDirectoryExists();

private:
    static AIResourceCache *s_instance;
    static QMutex s_instanceMutex;

    QMap<QString, CacheEntry> m_cacheIndex;
    mutable QMutex m_mutex;
};

#endif // AIRESOURCECACHE_H
