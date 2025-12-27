#ifndef AIGENERATEDCLIP_H
#define AIGENERATEDCLIP_H

#include "../baseclip.h"
#include <QJsonObject>
#include <QJsonDocument>

class AIGenerationTask;
class AIResourceCache;

// AI 生成状态枚举
enum AIGenerationState {
    AIIdle = 0,           // 空闲，未开始生成
    AIPreparing = 1,      // 准备中（参数验证、队列中）
    AIGenerating = 2,     // 生成中
    AICompleted = 3,      // 已完成
    AIFailed = 4,         // 生成失败
    AICancelled = 5       // 已取消
};

Q_DECLARE_METATYPE(AIGenerationState)

/**
 * @brief AI 生成 Clip 基类
 *
 * 为 AI 生图、AI 生视频、AI TTS 提供统一的基础架构，包括：
 * - 状态管理（Idle → Preparing → Generating → Completed/Failed）
 * - 参数管理和验证
 * - 异步生成任务管理
 * - 智能缓存机制
 * - 实例状态保存和恢复
 */
class AIGeneratedClip : public BaseClip {
    Q_OBJECT
    Q_PROPERTY(AIGenerationState generationState READ generationState NOTIFY generationStateChanged)
    Q_PROPERTY(double generationProgress READ generationProgress NOTIFY generationProgressChanged)
    Q_PROPERTY(QString generationError READ generationError NOTIFY generationErrorChanged)

public:
    explicit AIGeneratedClip(const SkyResourceBean &bean, SkyClip *clip = nullptr);
    virtual ~AIGeneratedClip();

    // 状态查询
    AIGenerationState generationState() const { return m_generationState; }
    double generationProgress() const { return m_generationProgress; }
    QString generationError() const { return m_generationError; }

    // 生成控制
    Q_INVOKABLE virtual bool startGeneration(const QJsonObject &params);
    Q_INVOKABLE virtual void cancelGeneration();
    Q_INVOKABLE virtual void retryGeneration();

    // 参数管理
    QJsonObject getGenerationParams() const { return m_generationParams; }
    void setGenerationParams(const QJsonObject &params);

    // 缓存管理
    QString getCacheKey() const;
    bool loadFromCache();
    void saveToCache();

    // 占位符资源（生成中显示）
    virtual QString getPlaceholderResource() const = 0;

    // BaseClip 接口实现
    QVariant get(int role) const override;

protected:
    // 子类需要实现的抽象方法

    /**
     * @brief 验证生成参数
     * @param params 生成参数
     * @param error 错误信息输出
     * @return true 验证通过，false 验证失败
     */
    virtual bool validateGenerationParams(const QJsonObject &params, QString &error) = 0;

    /**
     * @brief 创建生成任务
     * @param params 生成参数
     * @return 生成任务实例
     */
    virtual AIGenerationTask* createGenerationTask(const QJsonObject &params) = 0;

    /**
     * @brief 应用生成的资源
     * @param resourcePath 生成的资源文件路径
     */
    virtual void applyGeneratedResource(const QString &resourcePath) = 0;

    // 状态管理
    void setGenerationState(AIGenerationState state);
    void setGenerationProgress(double progress);
    void setGenerationError(const QString &error);

    // 实例状态保存/恢复
    void onSaveInstanceState(QMap<QString, SkyVariant> &bundle) override;
    void onRestoreInstanceState(QMap<QString, SkyVariant> &bundle) override;

signals:
    void generationStateChanged(AIGenerationState state);
    void generationProgressChanged(double progress);
    void generationErrorChanged(const QString &error);
    void generationCompleted(const QString &resourcePath);
    void generationFailed(const QString &error);

private slots:
    void onTaskProgressChanged(double progress);
    void onTaskCompleted(const QString &resourcePath);
    void onTaskFailed(const QString &error);

protected:
    AIGenerationState m_generationState = AIIdle;
    double m_generationProgress = 0.0;
    QString m_generationError;
    QJsonObject m_generationParams;
    AIGenerationTask *m_generationTask = nullptr;
    QString m_generatedResourcePath;
};

#endif // AIGENERATEDCLIP_H
