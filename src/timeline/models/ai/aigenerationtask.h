#ifndef AIGENERATIONTASK_H
#define AIGENERATIONTASK_H

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

// AI 任务类型
enum AITaskType {
    ImageGeneration,
    VideoGeneration,
    TTSGeneration
};

/**
 * @brief AI 生成任务基类
 *
 * 负责执行实际的 AI 生成任务，包括：
 * - 调用 LLM API（OpenAI、Stability AI、本地模型等）
 * - 进度上报
 * - 任务取消
 * - 错误处理
 */
class AIGenerationTask : public QObject {
    Q_OBJECT

public:
    explicit AIGenerationTask(AITaskType type, const QJsonObject &params, QObject *parent = nullptr);
    virtual ~AIGenerationTask();

    // 任务信息
    AITaskType taskType() const { return m_taskType; }
    QString taskId() const { return m_taskId; }

    // 任务控制
    void start();
    void cancel();

    // 状态查询
    bool isRunning() const { return m_isRunning; }
    double progress() const { return m_progress; }

signals:
    void progressChanged(double progress);
    void completed(const QString &resourcePath);
    void failed(const QString &error);

protected:
    // 子类需要实现的方法
    virtual void executeTask() = 0;
    virtual void cancelTask() = 0;

    // 辅助方法
    void setProgress(double progress);
    void emitCompleted(const QString &resourcePath);
    void emitFailed(const QString &error);

    // 网络请求辅助
    QNetworkAccessManager* networkManager();

protected:
    AITaskType m_taskType;
    QString m_taskId;
    QJsonObject m_params;
    bool m_isRunning = false;
    double m_progress = 0.0;
    QNetworkAccessManager *m_networkManager = nullptr;
};

/**
 * @brief AI 图片生成任务
 */
class AIImageGenerationTask : public AIGenerationTask {
    Q_OBJECT

public:
    explicit AIImageGenerationTask(const QJsonObject &params, QObject *parent = nullptr);
    ~AIImageGenerationTask() override;

protected:
    void executeTask() override;
    void cancelTask() override;

private slots:
    void onNetworkReplyFinished();

private:
    QNetworkReply *m_currentReply = nullptr;
};

/**
 * @brief AI 视频生成任务
 */
class AIVideoGenerationTask : public AIGenerationTask {
    Q_OBJECT

public:
    explicit AIVideoGenerationTask(const QJsonObject &params, QObject *parent = nullptr);
    ~AIVideoGenerationTask() override;

protected:
    void executeTask() override;
    void cancelTask() override;

private slots:
    void pollGenerationStatus();
    void onSubmitReplyFinished();
    void onPollReplyFinished();

private:
    QTimer *m_pollTimer = nullptr;
    QString m_generationJobId;
    QNetworkReply *m_currentReply = nullptr;
};

/**
 * @brief AI TTS 生成任务
 */
class AITTSGenerationTask : public AIGenerationTask {
    Q_OBJECT

public:
    explicit AITTSGenerationTask(const QJsonObject &params, QObject *parent = nullptr);
    ~AITTSGenerationTask() override;

protected:
    void executeTask() override;
    void cancelTask() override;

private slots:
    void onNetworkReplyFinished();

private:
    QNetworkReply *m_currentReply = nullptr;
};

#endif // AIGENERATIONTASK_H
