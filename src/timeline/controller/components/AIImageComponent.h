#ifndef AIIMAGECOMPONENT_H
#define AIIMAGECOMPONENT_H

#include "BaseComponent.h"
#include <QStringList>

class QNetworkReply;

class AIImageComponent : public BaseComponent {
    Q_OBJECT

public:
    AIImageComponent(QObject *parent = nullptr);

    void onBindQml(QJsonObject &params) override;
    void onUnBind() override;

    // Q_INVOKABLE methods that can be called from QML
    Q_INVOKABLE QString getPrompt() const;
    Q_INVOKABLE void setPrompt(const QString &prompt);

    Q_INVOKABLE QString getModel() const;
    Q_INVOKABLE void setModel(const QString &model);

    Q_INVOKABLE QString getImageSize() const;
    Q_INVOKABLE void setImageSize(const QString &size);

    Q_INVOKABLE QString getAspectRatio() const;
    Q_INVOKABLE void setAspectRatio(const QString &ratio);

    Q_INVOKABLE QStringList getReferenceImages() const;
    Q_INVOKABLE void setReferenceImages(const QStringList &images);
    Q_INVOKABLE void addReferenceImage(const QString &imagePath);
    Q_INVOKABLE void clearReferenceImages();

    Q_INVOKABLE void generateImage();

signals:
    void promptChanged(const QString &prompt);
    void modelChanged(const QString &model);
    void imageSizeChanged(const QString &size);
    void aspectRatioChanged(const QString &ratio);
    void referenceImagesChanged(const QStringList &images);
    void generationStarted();
    void generationCompleted(const QString &imagePath);
    void generationFailed(const QString &error);

private:
    QString convertModelName(const QString &uiModelName) const;
    QString imageToBase64(const QString &imagePath) const;
    QString saveGeneratedImage(const QByteArray &imageData) const;

    QString m_prompt;
    QString m_model;
    QString m_imageSize;
    QString m_aspectRatio;
    QStringList m_referenceImages;
    QNetworkReply *m_currentReply;
};

#endif // AIIMAGECOMPONENT_H
