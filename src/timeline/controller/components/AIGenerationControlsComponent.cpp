#include "AIGenerationControlsComponent.h"
#include <QJsonDocument>
#include <QDebug>

AIGenerationControlsComponent::AIGenerationControlsComponent(QObject *parent)
    : BaseComponent(parent) {
}

AIGenerationControlsComponent::~AIGenerationControlsComponent() {
}

void AIGenerationControlsComponent::onBindQml(QJsonObject &data) {
    BaseComponent::onBindQml(data);

    qDebug() << "AIGenerationControlsComponent::onBindQml";

    // 获取 AI Clip
    m_aiClip = qobject_cast<AIGeneratedClip*>(curClip());

    if (m_aiClip) {
        // 连接信号
        connect(m_aiClip, &AIGeneratedClip::generationStateChanged,
                this, &AIGenerationControlsComponent::onClipStateChanged);
        connect(m_aiClip, &AIGeneratedClip::generationProgressChanged,
                this, &AIGenerationControlsComponent::onClipProgressChanged);
        connect(m_aiClip, &AIGeneratedClip::generationErrorChanged,
                this, &AIGenerationControlsComponent::onClipErrorChanged);

        // 初始化状态
        m_state = m_aiClip->generationState();
        m_progress = m_aiClip->generationProgress();
        m_errorMessage = m_aiClip->generationError();

        emit stateChanged(m_state);
        emit progressChanged(m_progress);
        emit errorMessageChanged(m_errorMessage);

        qDebug() << "AIGenerationControlsComponent bound to AI Clip, initial state:" << m_state;
    } else {
        qWarning() << "AIGenerationControlsComponent: curClip is not an AIGeneratedClip!";
    }

    emit bindComponent();
}

void AIGenerationControlsComponent::onUnBind() {
    qDebug() << "AIGenerationControlsComponent::onUnBind";

    if (m_aiClip) {
        // 断开信号
        disconnect(m_aiClip, nullptr, this, nullptr);
        m_aiClip = nullptr;
    }

    m_state = AIIdle;
    m_progress = 0.0;
    m_errorMessage.clear();

    BaseComponent::onUnBind();
}

void AIGenerationControlsComponent::startGeneration() {
    qDebug() << "AIGenerationControlsComponent::startGeneration";

    if (!m_aiClip) {
        qWarning() << "No AI Clip bound";
        return;
    }

    // 收集所有生成参数
    QJsonObject params = collectGenerationParams();

    qDebug() << "Generation params:" << params;

    // 调用 Clip 的生成方法
    bool success = m_aiClip->startGeneration(params);

    if (!success) {
        qWarning() << "Failed to start generation";
    }
}

void AIGenerationControlsComponent::cancelGeneration() {
    qDebug() << "AIGenerationControlsComponent::cancelGeneration";

    if (!m_aiClip) {
        return;
    }

    m_aiClip->cancelGeneration();
}

void AIGenerationControlsComponent::retryGeneration() {
    qDebug() << "AIGenerationControlsComponent::retryGeneration";

    if (!m_aiClip) {
        return;
    }

    m_aiClip->retryGeneration();
}

QJsonObject AIGenerationControlsComponent::collectGenerationParams() {
    QJsonObject params;

    // 从组件中获取所有 AI 相关的参数
    // 这些参数名称对应 AIImageClip 的 UIConfig 中定义的 paramName

    // 模型
    params["ai_model"] = getOfParamVariantValue("ai_model", "gemini-3-pro-image-preview").toString();

    // 提示词
    params["ai_prompt"] = getOfParamVariantValue("ai_prompt", "").toString();

    // 参考图片
    params["ai_reference_image"] = getOfParamVariantValue("ai_reference_image", "").toString();

    // 图片尺寸
    params["ai_image_size"] = getOfParamVariantValue("ai_image_size", "1024x1024").toString();

    // 生成风格
    params["ai_style_preset"] = getOfParamVariantValue("ai_style_preset", "none").toString();

    // 生成强度
    params["ai_strength"] = getOfParamVariantValue("ai_strength", 0.8).toDouble();

    // 生成步数
    params["ai_steps"] = getOfParamVariantValue("ai_steps", 30).toInt();

    // 随机种子
    params["ai_seed"] = getOfParamVariantValue("ai_seed", -1).toInt();

    return params;
}

void AIGenerationControlsComponent::onClipStateChanged(AIGenerationState state) {
    m_state = state;
    emit stateChanged(state);
    qDebug() << "AIGenerationControlsComponent: state changed to" << state;
}

void AIGenerationControlsComponent::onClipProgressChanged(double progress) {
    m_progress = progress;
    emit progressChanged(progress);
}

void AIGenerationControlsComponent::onClipErrorChanged(const QString &error) {
    m_errorMessage = error;
    emit errorMessageChanged(error);
    qDebug() << "AIGenerationControlsComponent: error:" << error;
}
