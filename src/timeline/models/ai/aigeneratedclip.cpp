#include "aigeneratedclip.h"
#include "aigenerationtask.h"
#include "airesourcecache.h"
#include "../TimelineDef.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QDebug>
#include <QFile>

AIGeneratedClip::AIGeneratedClip(const SkyResourceBean &bean, SkyClip *clip)
    : BaseClip(bean, clip) {
    qDebug() << "AIGeneratedClip created:" << this;
}

AIGeneratedClip::~AIGeneratedClip() {
    if (m_generationTask) {
        m_generationTask->cancel();
        m_generationTask->deleteLater();
        m_generationTask = nullptr;
    }
    qDebug() << "AIGeneratedClip destroyed:" << this;
}

bool AIGeneratedClip::startGeneration(const QJsonObject &params) {
    qDebug() << "AIGeneratedClip::startGeneration" << params;

    // 1. 验证参数
    QString error;
    if (!validateGenerationParams(params, error)) {
        setGenerationError(error);
        setGenerationState(AIFailed);
        qWarning() << "Parameter validation failed:" << error;
        return false;
    }

    // 2. 保存参数
    m_generationParams = params;

    // 3. 检查缓存
    setGenerationState(AIPreparing);
    if (loadFromCache()) {
        qDebug() << "Cache hit, using cached resource:" << m_generatedResourcePath;
        applyGeneratedResource(m_generatedResourcePath);
        setGenerationState(AICompleted);
        setGenerationProgress(1.0);
        emit generationCompleted(m_generatedResourcePath);
        return true;
    }

    // 4. 创建并启动生成任务
    if (m_generationTask) {
        m_generationTask->cancel();
        m_generationTask->deleteLater();
    }

    m_generationTask = createGenerationTask(params);
    if (!m_generationTask) {
        setGenerationError("Failed to create generation task");
        setGenerationState(AIFailed);
        return false;
    }

    // 连接信号
    connect(m_generationTask, &AIGenerationTask::progressChanged,
            this, &AIGeneratedClip::onTaskProgressChanged);
    connect(m_generationTask, &AIGenerationTask::completed,
            this, &AIGeneratedClip::onTaskCompleted);
    connect(m_generationTask, &AIGenerationTask::failed,
            this, &AIGeneratedClip::onTaskFailed);

    // 启动任务
    setGenerationState(AIGenerating);
    setGenerationProgress(0.0);
    m_generationTask->start();

    qDebug() << "Generation task started";
    return true;
}

void AIGeneratedClip::cancelGeneration() {
    qDebug() << "AIGeneratedClip::cancelGeneration";

    if (m_generationTask && m_generationTask->isRunning()) {
        m_generationTask->cancel();
        setGenerationState(AICancelled);
        setGenerationError("Generation cancelled by user");
    }
}

void AIGeneratedClip::retryGeneration() {
    qDebug() << "AIGeneratedClip::retryGeneration";

    if (m_generationParams.isEmpty()) {
        setGenerationError("No generation parameters available for retry");
        return;
    }

    // 重新开始生成
    startGeneration(m_generationParams);
}

void AIGeneratedClip::setGenerationParams(const QJsonObject &params) {
    m_generationParams = params;
}

QString AIGeneratedClip::getCacheKey() const {
    // 使用参数的 SHA256 哈希作为缓存 Key
    QJsonDocument doc(m_generationParams);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString::fromUtf8(hash.toHex());
}

bool AIGeneratedClip::loadFromCache() {
    QString cacheKey = getCacheKey();
    if (AIResourceCache::instance()->hasCache(cacheKey)) {
        m_generatedResourcePath = AIResourceCache::instance()->getCachedResource(cacheKey);
        return QFile::exists(m_generatedResourcePath);
    }
    return false;
}

void AIGeneratedClip::saveToCache() {
    if (!m_generatedResourcePath.isEmpty() && QFile::exists(m_generatedResourcePath)) {
        QString cacheKey = getCacheKey();
        AIResourceCache::instance()->addCache(cacheKey, m_generatedResourcePath);
        qDebug() << "Resource saved to cache:" << cacheKey << "->" << m_generatedResourcePath;
    }
}

QVariant AIGeneratedClip::get(int role) const {
    switch (role) {
        case RoleAIGenerationState:
            return QVariant::fromValue(m_generationState);
        case RoleAIGenerationProgress:
            return m_generationProgress;
        case RoleAIGenerationError:
            return m_generationError;
        case RoleAIGenerationParams:
            return QJsonDocument(m_generationParams).toJson(QJsonDocument::Compact);
        default:
            return BaseClip::get(role);
    }
}

void AIGeneratedClip::setGenerationState(AIGenerationState state) {
    if (m_generationState != state) {
        m_generationState = state;
        emit generationStateChanged(state);
        emit clipDataChanged(this, {RoleAIGenerationState});
        qDebug() << "Generation state changed to:" << state;
    }
}

void AIGeneratedClip::setGenerationProgress(double progress) {
    if (qAbs(m_generationProgress - progress) > 0.001) {
        m_generationProgress = qBound(0.0, progress, 1.0);
        emit generationProgressChanged(m_generationProgress);
        emit clipDataChanged(this, {RoleAIGenerationProgress});
    }
}

void AIGeneratedClip::setGenerationError(const QString &error) {
    if (m_generationError != error) {
        m_generationError = error;
        emit generationErrorChanged(error);
        emit clipDataChanged(this, {RoleAIGenerationError});
        qDebug() << "Generation error:" << error;
    }
}

void AIGeneratedClip::onSaveInstanceState(QMap<QString, SkyVariant> &bundle) {
    BaseClip::onSaveInstanceState(bundle);

    // 保存 AI 生成相关的状态
    bundle["ai_generation_state"] = SkyVariant((int)m_generationState);
    bundle["ai_generation_progress"] = SkyVariant(m_generationProgress);
    bundle["ai_generation_error"] = SkyVariant(m_generationError.toUtf8().data());

    // 保存生成参数（JSON 格式）
    QJsonDocument paramsDoc(m_generationParams);
    QString paramsJson = QString::fromUtf8(paramsDoc.toJson(QJsonDocument::Compact));
    bundle["ai_generation_params"] = SkyVariant(paramsJson.toUtf8().data());

    // 保存生成的资源路径
    bundle["ai_generated_resource"] = SkyVariant::makePath(m_generatedResourcePath.toUtf8().data());

    qDebug() << "AIGeneratedClip::onSaveInstanceState" << toString();
}

void AIGeneratedClip::onRestoreInstanceState(QMap<QString, SkyVariant> &bundle) {
    BaseClip::onRestoreInstanceState(bundle);

    // 恢复 AI 生成相关的状态
    if (bundle.contains("ai_generation_state")) {
        m_generationState = (AIGenerationState)bundle["ai_generation_state"].toInt();
    }

    if (bundle.contains("ai_generation_progress")) {
        m_generationProgress = bundle["ai_generation_progress"].toDouble();
    }

    if (bundle.contains("ai_generation_error")) {
        m_generationError = QString::fromUtf8(bundle["ai_generation_error"].toString());
    }

    // 恢复生成参数
    if (bundle.contains("ai_generation_params")) {
        QString paramsJson = QString::fromUtf8(bundle["ai_generation_params"].toString());
        QJsonDocument paramsDoc = QJsonDocument::fromJson(paramsJson.toUtf8());
        m_generationParams = paramsDoc.object();
    }

    // 恢复生成的资源路径
    if (bundle.contains("ai_generated_resource")) {
        m_generatedResourcePath = QString::fromUtf8(bundle["ai_generated_resource"].toString());

        // 如果已完成且资源存在，应用资源
        if (m_generationState == AICompleted && QFile::exists(m_generatedResourcePath)) {
            applyGeneratedResource(m_generatedResourcePath);
            qDebug() << "Restored and applied AI generated resource:" << m_generatedResourcePath;
        }
    }

    qDebug() << "AIGeneratedClip::onRestoreInstanceState" << toString();
}

void AIGeneratedClip::onTaskProgressChanged(double progress) {
    setGenerationProgress(progress);
}

void AIGeneratedClip::onTaskCompleted(const QString &resourcePath) {
    qDebug() << "AIGeneratedClip::onTaskCompleted" << resourcePath;

    m_generatedResourcePath = resourcePath;

    // 应用生成的资源
    applyGeneratedResource(resourcePath);

    // 保存到缓存
    saveToCache();

    // 更新状态
    setGenerationState(AICompleted);
    setGenerationProgress(1.0);
    setGenerationError("");

    emit generationCompleted(resourcePath);

    // 清理任务
    if (m_generationTask) {
        m_generationTask->deleteLater();
        m_generationTask = nullptr;
    }
}

void AIGeneratedClip::onTaskFailed(const QString &error) {
    qDebug() << "AIGeneratedClip::onTaskFailed" << error;

    setGenerationState(AIFailed);
    setGenerationError(error);

    emit generationFailed(error);

    // 清理任务
    if (m_generationTask) {
        m_generationTask->deleteLater();
        m_generationTask = nullptr;
    }
}
