#include "AIImageComponent.h"
#include <QJsonArray>
#include <QDebug>
#include "base/utils/AppConstance.h"

AIImageComponent::AIImageComponent(QObject *parent)
    : BaseComponent(parent)
    , m_prompt("")
    , m_model("Gemini 3 Pro Image Preview")
    , m_imageSize("1024x1024") {
    qDebug() << "AIImageComponent created";
}

void AIImageComponent::onBindQml(QJsonObject &paramInfo) {
    qDebug() << "AIImageComponent::onBindQml" << paramInfo;

    // Read any saved parameters from paramInfo if needed
    if (paramInfo.contains("prompt")) {
        m_prompt = paramInfo["prompt"].toString();
    }
    if (paramInfo.contains("model")) {
        m_model = paramInfo["model"].toString();
    }
    if (paramInfo.contains("imageSize")) {
        m_imageSize = paramInfo["imageSize"].toString();
    }

    // Emit signals to update QML UI
    emit promptChanged(m_prompt);
    emit modelChanged(m_model);
    emit imageSizeChanged(m_imageSize);

    // Notify QML that binding is complete
    emit bindComponent();
}

void AIImageComponent::onUnBind() {
    qDebug() << "AIImageComponent::onUnBind";
    m_prompt.clear();
}

QString AIImageComponent::getPrompt() const {
    return m_prompt;
}

void AIImageComponent::setPrompt(const QString &prompt) {
    if (m_prompt != prompt) {
        m_prompt = prompt;
        emit promptChanged(m_prompt);
        qDebug() << "AIImageComponent: Prompt set to" << m_prompt;
    }
}

QString AIImageComponent::getModel() const {
    return m_model;
}

void AIImageComponent::setModel(const QString &model) {
    if (m_model != model) {
        m_model = model;
        emit modelChanged(m_model);
        qDebug() << "AIImageComponent: Model set to" << m_model;
    }
}

QString AIImageComponent::getImageSize() const {
    return m_imageSize;
}

void AIImageComponent::setImageSize(const QString &size) {
    if (m_imageSize != size) {
        m_imageSize = size;
        emit imageSizeChanged(m_imageSize);
        qDebug() << "AIImageComponent: Image size set to" << m_imageSize;
    }
}

void AIImageComponent::generateImage() {
    qDebug() << "AIImageComponent::generateImage called";
    qDebug() << "  Prompt:" << m_prompt;
    qDebug() << "  Model:" << m_model;
    qDebug() << "  Size:" << m_imageSize;

    emit generationStarted();

    // TODO: Implement actual AI image generation logic here
    // For now, just emit a failure to indicate it's not implemented yet
    emit generationFailed("AI Image generation not yet implemented");
}
