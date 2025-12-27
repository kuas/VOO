#ifndef AIIMAGECLIP_H
#define AIIMAGECLIP_H

#include "aigeneratedclip.h"

/**
 * @brief AI 图片生成 Clip
 *
 * 支持功能：
 * - 文生图（通过 prompt）
 * - 图生图（通过 prompt + referenceImage）
 * - 图片尺寸选择
 * - 生成风格选择
 * - 生成强度控制
 * - Transform 变换
 */
class AIImageClip : public AIGeneratedClip {
    Q_OBJECT
    Q_PROPERTY(QString referenceImage READ referenceImage WRITE setReferenceImage NOTIFY referenceImageChanged)

public:
    explicit AIImageClip(const SkyResourceBean &bean, SkyClip *clip = nullptr);
    ~AIImageClip() override;

    // Clip 类型
    ClipType clipType() const override { return ImageClipType; }

    // UI 配置
    QJsonDocument getUIConfig() override;

    // 占位符资源
    QString getPlaceholderResource() const override;

    // 图生图支持
    QString referenceImage() const { return m_referenceImagePath; }
    Q_INVOKABLE void setReferenceImage(const QString &imagePath);

signals:
    void referenceImageChanged(const QString &imagePath);

protected:
    // 实现抽象方法
    bool validateGenerationParams(const QJsonObject &params, QString &error) override;
    AIGenerationTask* createGenerationTask(const QJsonObject &params) override;
    void applyGeneratedResource(const QString &resourcePath) override;

    // 实例状态保存/恢复
    void onSaveInstanceState(QMap<QString, SkyVariant> &bundle) override;
    void onRestoreInstanceState(QMap<QString, SkyVariant> &bundle) override;

private:
    QString m_referenceImagePath;  // 图生图参考图
};

#endif // AIIMAGECLIP_H
