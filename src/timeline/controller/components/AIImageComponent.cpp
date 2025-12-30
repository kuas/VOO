#include "AIImageComponent.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QFile>
#include <QBuffer>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "base/utils/AppConstance.h"
#include "base/http/SEHttpClient.h"

AIImageComponent::AIImageComponent(QObject *parent)
    : BaseComponent(parent)
    , m_prompt("")
    , m_model("Gemini 3 Pro Image Preview")
    , m_imageSize("1024x1024")
    , m_aspectRatio("1:1")
    , m_currentReply(nullptr) {
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
    if (paramInfo.contains("aspectRatio")) {
        m_aspectRatio = paramInfo["aspectRatio"].toString();
    }
    if (paramInfo.contains("referenceImages")) {
        QJsonArray imagesArray = paramInfo["referenceImages"].toArray();
        m_referenceImages.clear();
        for (const QJsonValue &value : imagesArray) {
            m_referenceImages.append(value.toString());
        }
    }

    // Emit signals to update QML UI
    emit promptChanged(m_prompt);
    emit modelChanged(m_model);
    emit imageSizeChanged(m_imageSize);
    emit aspectRatioChanged(m_aspectRatio);
    emit referenceImagesChanged(m_referenceImages);

    // Notify QML that binding is complete
    emit bindComponent();
}

void AIImageComponent::onUnBind() {
    qDebug() << "AIImageComponent::onUnBind";
    m_prompt.clear();
    m_referenceImages.clear();

    // Cancel any ongoing network request
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
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

QString AIImageComponent::getAspectRatio() const {
    return m_aspectRatio;
}

void AIImageComponent::setAspectRatio(const QString &ratio) {
    if (m_aspectRatio != ratio) {
        m_aspectRatio = ratio;
        emit aspectRatioChanged(m_aspectRatio);
        qDebug() << "AIImageComponent: Aspect ratio set to" << m_aspectRatio;
    }
}

QStringList AIImageComponent::getReferenceImages() const {
    return m_referenceImages;
}

void AIImageComponent::setReferenceImages(const QStringList &images) {
    if (m_referenceImages != images) {
        m_referenceImages = images;
        emit referenceImagesChanged(m_referenceImages);
        qDebug() << "AIImageComponent: Reference images set, count:" << m_referenceImages.size();
    }
}

void AIImageComponent::addReferenceImage(const QString &imagePath) {
    if (!m_referenceImages.contains(imagePath)) {
        m_referenceImages.append(imagePath);
        emit referenceImagesChanged(m_referenceImages);
        qDebug() << "AIImageComponent: Added reference image:" << imagePath;
    }
}

void AIImageComponent::clearReferenceImages() {
    if (!m_referenceImages.isEmpty()) {
        m_referenceImages.clear();
        emit referenceImagesChanged(m_referenceImages);
        qDebug() << "AIImageComponent: Cleared all reference images";
    }
}

void AIImageComponent::generateImage() {
    qDebug() << "AIImageComponent::generateImage called";
    qDebug() << "  Prompt:" << m_prompt;
    qDebug() << "  Model:" << m_model;
    qDebug() << "  Size:" << m_imageSize;
    qDebug() << "  Aspect Ratio:" << m_aspectRatio;
    qDebug() << "  Reference Images:" << m_referenceImages.size();

    // Validate inputs
    if (m_prompt.trimmed().isEmpty()) {
        emit generationFailed("Prompt cannot be empty");
        return;
    }

    emit generationStarted();

    // Build the request JSON according to the API format
    QJsonObject requestBody;

    // Convert UI model name to API model name
    QString apiModel = convertModelName(m_model);
    requestBody["model"] = apiModel;

    // Build messages array
    QJsonArray messages;
    QJsonObject userMessage;
    userMessage["role"] = "user";

    // If we have reference images, build content array (for image editing)
    if (!m_referenceImages.isEmpty()) {
        QJsonArray contentArray;

        // Add reference images first
        for (const QString &imagePath : m_referenceImages) {
            QJsonObject imageContent;
            imageContent["type"] = "image_url";

            QJsonObject imageUrl;
            QString base64Image = imageToBase64(imagePath);
            if (base64Image.isEmpty()) {
                qWarning() << "Failed to convert image to base64:" << imagePath;
                continue;
            }
            imageUrl["url"] = base64Image;
            imageContent["image_url"] = imageUrl;

            contentArray.append(imageContent);
        }

        // Add text prompt
        QJsonObject textContent;
        textContent["type"] = "text";
        textContent["text"] = m_prompt;
        contentArray.append(textContent);

        userMessage["content"] = contentArray;
    } else {
        // Simple text prompt for image generation
        userMessage["content"] = m_prompt;
    }

    messages.append(userMessage);
    requestBody["messages"] = messages;

    // Set modalities to image
    QJsonArray modalities;
    modalities.append("image");
    requestBody["modalities"] = modalities;

    // Build imageConfig
    QJsonObject imageConfig;
    imageConfig["aspectRatio"] = m_aspectRatio;

    // Map size to imageSize config (only for high-quality models)
    if (apiModel.contains("gemini-3-pro")) {
        if (m_imageSize == "4096x4096") {
            imageConfig["imageSize"] = "4K";
        } else if (m_imageSize == "2048x2048") {
            imageConfig["imageSize"] = "2K";
        } else {
            imageConfig["imageSize"] = "1K";
        }
    }

    requestBody["imageConfig"] = imageConfig;

    // Get API endpoint from server domain
    QString apiUrl = "https://litellm.test.bloomeverybody.work/v1/chat/completions";

    qDebug() << "Sending request to:" << apiUrl;
    qDebug() << "Request body:" << QJsonDocument(requestBody).toJson(QJsonDocument::Compact);

    // Use SEHttpClient to make the request
    SEHttpClient *client = new SEHttpClient(apiUrl);
    client->debug(true);
    client->addJsonParam(requestBody);
    client->addHeader("Content-Type", "application/json");
    client->addHeader("Authorization", "Bearer sk-kwQKnXgj6IpuConW2d8QTg");

    // Make POST request
    m_currentReply = client->post(
        [this, client](const QString &response) {
            qDebug() << "Image generation response received";
            // Log response summary (truncate base64 data for readability)
            QString responseSummary = response;
            if (responseSummary.length() > 500) {
                responseSummary = responseSummary.left(500) + "... [truncated, total length: " + QString::number(response.length()) + " chars]";
            }
            qDebug() << "Response summary:" << responseSummary;

            // Parse the response
            QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
            if (doc.isNull() || !doc.isObject()) {
                emit generationFailed("Invalid response format");
                delete client;
                m_currentReply = nullptr;
                return;
            }

            QJsonObject responseObj = doc.object();

            // Check for error in response
            if (responseObj.contains("error")) {
                QString errorMsg = responseObj["error"].toObject()["message"].toString();
                emit generationFailed("API Error: " + errorMsg);
                delete client;
                m_currentReply = nullptr;
                return;
            }

            // Extract the generated image data
            // The image might be in different places depending on the API response format
            // Common formats:
            // 1. choices[0].message.images[0].image_url.url (Gemini format)
            // 2. choices[0].message.content (base64 or URL)
            // 3. data[0].url or data[0].b64_json

            QString imageData;

            if (responseObj.contains("choices")) {
                QJsonArray choices = responseObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject firstChoice = choices[0].toObject();
                    QJsonObject message = firstChoice["message"].toObject();

                    // First check for images array (Gemini format)
                    if (message.contains("images")) {
                        QJsonArray imagesArray = message["images"].toArray();
                        if (!imagesArray.isEmpty()) {
                            QJsonObject firstImage = imagesArray[0].toObject();
                            if (firstImage.contains("image_url")) {
                                imageData = firstImage["image_url"].toObject()["url"].toString();
                            }
                        }
                    }

                    // If not found, check content field
                    if (imageData.isEmpty()) {
                        QJsonValue content = message["content"];
                        if (content.isArray()) {
                            // Find image content in array
                            QJsonArray contentArray = content.toArray();
                            for (const QJsonValue &item : contentArray) {
                                QJsonObject itemObj = item.toObject();
                                if (itemObj["type"].toString() == "image_url") {
                                    imageData = itemObj["image_url"].toObject()["url"].toString();
                                    break;
                                }
                            }
                        } else if (content.isString()) {
                            imageData = content.toString();
                        }
                    }
                }
            } else if (responseObj.contains("data")) {
                QJsonArray dataArray = responseObj["data"].toArray();
                if (!dataArray.isEmpty()) {
                    QJsonObject firstData = dataArray[0].toObject();
                    if (firstData.contains("b64_json")) {
                        imageData = "data:image/png;base64," + firstData["b64_json"].toString();
                    } else if (firstData.contains("url")) {
                        imageData = firstData["url"].toString();
                    }
                }
            }

            if (imageData.isEmpty()) {
                qWarning() << "No image data found in response";
                emit generationFailed("No image data found in response");
                delete client;
                m_currentReply = nullptr;
                return;
            }

            qDebug() << "Found image data, length:" << imageData.length();
            qDebug() << "Image data starts with:" << imageData.left(50);

            // If imageData is base64, decode and save it
            if (imageData.startsWith("data:image")) {
                qDebug() << "Processing base64 image data";
                // Extract base64 data
                int commaIndex = imageData.indexOf(',');
                if (commaIndex > 0) {
                    QString base64Data = imageData.mid(commaIndex + 1);
                    qDebug() << "Base64 data length:" << base64Data.length();

                    QByteArray imageBytes = QByteArray::fromBase64(base64Data.toUtf8());
                    qDebug() << "Decoded image bytes:" << imageBytes.size();

                    if (imageBytes.isEmpty()) {
                        qWarning() << "Failed to decode base64 image data";
                        emit generationFailed("Failed to decode image data");
                    } else {
                        QString savedPath = saveGeneratedImage(imageBytes);

                        if (savedPath.isEmpty()) {
                            emit generationFailed("Failed to save generated image");
                        } else {
                            qDebug() << "Successfully saved image to:" << savedPath;
                            emit generationCompleted(savedPath);
                        }
                    }
                } else {
                    qWarning() << "Invalid base64 format, no comma found";
                    emit generationFailed("Invalid image data format");
                }
            } else {
                // It's a URL, we might need to download it
                qDebug() << "Image data is a URL:" << imageData;
                emit generationCompleted(imageData);
            }

            delete client;
            m_currentReply = nullptr;
        },
        [this, client](const QString &error) {
            qDebug() << "Image generation failed:" << error;
            emit generationFailed(error);
            delete client;
            m_currentReply = nullptr;
        }
    );
}

QString AIImageComponent::convertModelName(const QString &uiModelName) const {
    // Convert UI-friendly model names to API model names
    if (uiModelName == "Gemini 3 Pro Image Preview") {
        return "vertex_ai/gemini-3-pro-image-preview";
    } else if (uiModelName == "Gemini 2.5 Flash Image") {
        return "vertex_ai/gemini-2.5-flash-image";
    }

    // Default fallback
    return "vertex_ai/gemini-2.5-flash-image";
}

QString AIImageComponent::imageToBase64(const QString &imagePath) const {
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open image file:" << imagePath;
        return QString();
    }

    QByteArray imageData = file.readAll();
    file.close();

    // Determine image format from file extension
    QString format = "png";
    if (imagePath.endsWith(".jpg", Qt::CaseInsensitive) || imagePath.endsWith(".jpeg", Qt::CaseInsensitive)) {
        format = "jpeg";
    } else if (imagePath.endsWith(".webp", Qt::CaseInsensitive)) {
        format = "webp";
    } else if (imagePath.endsWith(".gif", Qt::CaseInsensitive)) {
        format = "gif";
    }

    QString base64 = imageData.toBase64();
    return QString("data:image/%1;base64,%2").arg(format, base64);
}

QString AIImageComponent::saveGeneratedImage(const QByteArray &imageData) const {
    // Create a directory for generated images
    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString saveDir = picturesPath + "/VOO_AI_Generated";

    QDir dir(saveDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Generate filename with timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("ai_generated_%1.png").arg(timestamp);
    QString fullPath = saveDir + "/" + filename;

    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create file:" << fullPath;
        return QString();
    }

    file.write(imageData);
    file.close();

    qDebug() << "Saved generated image to:" << fullPath;
    return fullPath;
}
