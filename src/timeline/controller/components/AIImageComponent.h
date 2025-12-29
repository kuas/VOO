#ifndef AIIMAGECOMPONENT_H
#define AIIMAGECOMPONENT_H

#include "BaseComponent.h"

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

    Q_INVOKABLE void generateImage();

signals:
    void promptChanged(const QString &prompt);
    void modelChanged(const QString &model);
    void imageSizeChanged(const QString &size);
    void generationStarted();
    void generationCompleted(const QString &imagePath);
    void generationFailed(const QString &error);

private:
    QString m_prompt;
    QString m_model;
    QString m_imageSize;
};

#endif // AIIMAGECOMPONENT_H
