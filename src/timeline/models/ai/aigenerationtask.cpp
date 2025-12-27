#include "aigenerationtask.h"
#include <QUuid>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

// ============================================================================
// AIGenerationTask 基类实现
// ============================================================================

AIGenerationTask::AIGenerationTask(AITaskType type, const QJsonObject &params, QObject *parent)
    : QObject(parent)
    , m_taskType(type)
    , m_params(params) {
    // 生成唯一的任务 ID
    m_taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qDebug() << "AIGenerationTask created:" << m_taskId << "Type:" << type;
}

AIGenerationTask::~AIGenerationTask() {
    if (m_networkManager) {
        delete m_networkManager;
        m_networkManager = nullptr;
    }
    qDebug() << "AIGenerationTask destroyed:" << m_taskId;
}

void AIGenerationTask::start() {
    if (m_isRunning) {
        qWarning() << "Task already running:" << m_taskId;
        return;
    }

    qDebug() << "Starting task:" << m_taskId;
    m_isRunning = true;
    setProgress(0.0);

    // 执行具体任务
    executeTask();
}

void AIGenerationTask::cancel() {
    if (!m_isRunning) {
        return;
    }

    qDebug() << "Cancelling task:" << m_taskId;
    m_isRunning = false;

    // 执行具体的取消逻辑
    cancelTask();

    emit failed("Task cancelled");
}

void AIGenerationTask::setProgress(double progress) {
    m_progress = qBound(0.0, progress, 1.0);
    emit progressChanged(m_progress);
}

void AIGenerationTask::emitCompleted(const QString &resourcePath) {
    m_isRunning = false;
    setProgress(1.0);
    qDebug() << "Task completed:" << m_taskId << "Resource:" << resourcePath;
    emit completed(resourcePath);
}

void AIGenerationTask::emitFailed(const QString &error) {
    m_isRunning = false;
    qWarning() << "Task failed:" << m_taskId << "Error:" << error;
    emit failed(error);
}

QNetworkAccessManager* AIGenerationTask::networkManager() {
    if (!m_networkManager) {
        m_networkManager = new QNetworkAccessManager(this);
    }
    return m_networkManager;
}

// ============================================================================
// AIImageGenerationTask 实现
// ============================================================================

AIImageGenerationTask::AIImageGenerationTask(const QJsonObject &params, QObject *parent)
    : AIGenerationTask(ImageGeneration, params, parent) {
}

AIImageGenerationTask::~AIImageGenerationTask() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void AIImageGenerationTask::executeTask() {
    qDebug() << "AIImageGenerationTask::executeTask" << m_params;

    // 使用 LiteLLM Gemini API 生成图片
    QNetworkRequest request(QUrl("http://litellm.test.bloomeverybody.work/v1/images/generations"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer XXX");  // TODO: 从配置读取 API Key
    request.setRawHeader("User-Agent", "VOO-Video-Editor/1.0");
    request.setRawHeader("Accept", "*/*");

    // 构建请求体
    QJsonObject requestBody;

    // 从参数中获取配置，优先使用 gemini-3-pro-image-preview
    QString model = m_params.contains("ai_model") ?
                    m_params["ai_model"].toString() :
                    "gemini-3-pro-image-preview";
    requestBody["model"] = model;

    // 提示词
    QString prompt = m_params["ai_prompt"].toString();
    requestBody["prompt"] = prompt;

    // 图片尺寸
    QString size = m_params.contains("ai_image_size") ?
                   m_params["ai_image_size"].toString() :
                   "1024x1024";
    requestBody["size"] = size;

    // 生成数量（固定为 1）
    requestBody["n"] = 1;

    // 如果有参考图片（图生图），需要添加到请求中
    // 注意：LiteLLM 的 Gemini 可能需要不同的参数格式，具体看文档
    if (m_params.contains("ai_reference_image") && !m_params["ai_reference_image"].toString().isEmpty()) {
        // TODO: 实现图生图的参数传递
        // 可能需要将图片转为 base64 或 URL
    }

    qDebug() << "Sending request to LiteLLM:" << requestBody;

    setProgress(0.1);

    // 发送请求
    m_currentReply = networkManager()->post(request, QJsonDocument(requestBody).toJson());
    connect(m_currentReply, &QNetworkReply::finished,
            this, &AIImageGenerationTask::onNetworkReplyFinished);

    // 设置超时（60秒）
    QTimer::singleShot(60000, this, [this]() {
        if (m_currentReply && m_currentReply->isRunning()) {
            m_currentReply->abort();
            emitFailed("Request timeout after 60 seconds");
        }
    });
}

void AIImageGenerationTask::cancelTask() {
    if (m_currentReply) {
        m_currentReply->abort();
    }
}

void AIImageGenerationTask::onNetworkReplyFinished() {
    if (!m_currentReply) {
        return;
    }

    setProgress(0.5);

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString error = QString("Network error: %1").arg(m_currentReply->errorString());
        qWarning() << "AIImageGenerationTask failed:" << error;
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        emitFailed(error);
        return;
    }

    // 解析响应
    QByteArray responseData = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    qDebug() << "Received response, size:" << responseData.size() << "bytes";

    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        emitFailed("Invalid JSON response");
        return;
    }

    QJsonObject obj = doc.object();

    setProgress(0.7);

    // 解析 LiteLLM 响应格式
    // {"created":1766658046,"data":[{"b64_json":"XXXX","revised_prompt":null,"url":null}],...}
    if (!obj.contains("data")) {
        QString error = QString("Invalid response format: %1").arg(QString::fromUtf8(responseData));
        qWarning() << error;
        emitFailed(error);
        return;
    }

    QJsonArray dataArray = obj["data"].toArray();
    if (dataArray.isEmpty()) {
        emitFailed("No image data in response");
        return;
    }

    QJsonObject imageData = dataArray[0].toObject();

    // 检查是 base64 还是 URL
    QString base64Data = imageData["b64_json"].toString();
    QString imageUrl = imageData["url"].toString();

    setProgress(0.9);

    if (!base64Data.isEmpty()) {
        // 处理 base64 数据
        QByteArray imageBytes = QByteArray::fromBase64(base64Data.toUtf8());

        // 保存图片
        QString outputDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/VOO/ai_generated";
        QDir().mkpath(outputDir);
        QString outputPath = QString("%1/ai_image_%2.png").arg(outputDir).arg(m_taskId);

        QFile file(outputPath);
        if (!file.open(QIODevice::WriteOnly)) {
            emitFailed(QString("Failed to create output file: %1").arg(outputPath));
            return;
        }

        file.write(imageBytes);
        file.close();

        qDebug() << "Image saved to:" << outputPath;
        emitCompleted(outputPath);

    } else if (!imageUrl.isEmpty()) {
        // 处理 URL（如果 API 返回 URL 而不是 base64）
        // TODO: 实现从 URL 下载图片
        emitFailed("URL-based image download not implemented yet");
    } else {
        emitFailed("No image data (neither b64_json nor url) in response");
    }
}

// ============================================================================
// AIVideoGenerationTask 实现
// ============================================================================

AIVideoGenerationTask::AIVideoGenerationTask(const QJsonObject &params, QObject *parent)
    : AIGenerationTask(VideoGeneration, params, parent) {
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &AIVideoGenerationTask::pollGenerationStatus);
}

AIVideoGenerationTask::~AIVideoGenerationTask() {
    if (m_pollTimer) {
        m_pollTimer->stop();
    }
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void AIVideoGenerationTask::executeTask() {
    qDebug() << "AIVideoGenerationTask::executeTask" << m_params;

    // 注意：这里是模拟实现
    // 实际使用时需要替换为真实的视频生成 API（如 Runway, Pika Labs 等）

    QString outputDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/VOO/ai_generated";
    QDir().mkpath(outputDir);

    QString outputPath = QString("%1/ai_video_%2.mp4").arg(outputDir).arg(m_taskId);

    // 模拟视频生成（通常需要较长时间）
    setProgress(0.1);
    QTimer::singleShot(2000, this, [this, outputPath]() {
        setProgress(0.5);
        QTimer::singleShot(3000, this, [this, outputPath]() {
            setProgress(0.9);

            // 创建一个占位视频文件（实际应该是从 API 获取）
            QFile file(outputPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write("PLACEHOLDER_VIDEO_DATA");
                file.close();
                emitCompleted(outputPath);
            } else {
                emitFailed("Failed to save generated video");
            }
        });
    });
}

void AIVideoGenerationTask::cancelTask() {
    if (m_pollTimer) {
        m_pollTimer->stop();
    }
    if (m_currentReply) {
        m_currentReply->abort();
    }
}

void AIVideoGenerationTask::pollGenerationStatus() {
    // TODO: 实现状态轮询逻辑
    // 大多数视频生成 API 都是异步的，需要轮询检查生成状态
}

void AIVideoGenerationTask::onSubmitReplyFinished() {
    // TODO: 处理提交响应
}

void AIVideoGenerationTask::onPollReplyFinished() {
    // TODO: 处理轮询响应
}

// ============================================================================
// AITTSGenerationTask 实现
// ============================================================================

AITTSGenerationTask::AITTSGenerationTask(const QJsonObject &params, QObject *parent)
    : AIGenerationTask(TTSGeneration, params, parent) {
}

AITTSGenerationTask::~AITTSGenerationTask() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void AITTSGenerationTask::executeTask() {
    qDebug() << "AITTSGenerationTask::executeTask" << m_params;

    // 注意：这里是模拟实现
    // 实际使用时需要替换为真实的 TTS API（如 OpenAI TTS, ElevenLabs 等）

    QString outputDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/VOO/ai_generated";
    QDir().mkpath(outputDir);

    QString outputPath = QString("%1/ai_tts_%2.mp3").arg(outputDir).arg(m_taskId);

    // 模拟 TTS 生成
    setProgress(0.3);
    QTimer::singleShot(500, this, [this, outputPath]() {
        setProgress(0.7);
        QTimer::singleShot(500, this, [this, outputPath]() {
            setProgress(0.9);

            // 创建一个占位音频文件（实际应该是从 API 获取）
            QFile file(outputPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write("PLACEHOLDER_AUDIO_DATA");
                file.close();
                emitCompleted(outputPath);
            } else {
                emitFailed("Failed to save generated audio");
            }
        });
    });
}

void AITTSGenerationTask::cancelTask() {
    if (m_currentReply) {
        m_currentReply->abort();
    }
}

void AITTSGenerationTask::onNetworkReplyFinished() {
    if (!m_currentReply) {
        return;
    }

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString error = m_currentReply->errorString();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        emitFailed(error);
        return;
    }

    // 保存音频文件
    QByteArray audioData = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    QString outputDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/VOO/ai_generated";
    QDir().mkpath(outputDir);
    QString outputPath = QString("%1/ai_tts_%2.mp3").arg(outputDir).arg(m_taskId);

    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(audioData);
        file.close();
        emitCompleted(outputPath);
    } else {
        emitFailed("Failed to save audio file");
    }
}
