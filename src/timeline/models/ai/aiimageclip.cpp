#include "aiimageclip.h"
#include "aigenerationtask.h"
#include "../TimelineDef.h"
#include <base/utils/JsonUtils.h>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFile>

AIImageClip::AIImageClip(const SkyResourceBean &bean, SkyClip *clip)
    : AIGeneratedClip(bean, clip) {
    qDebug() << "AIImageClip created:" << this;
}

AIImageClip::~AIImageClip() {
    qDebug() << "AIImageClip destroyed:" << this;
}

QJsonDocument AIImageClip::getUIConfig() {
    // 从 JSON 配置文件读取 UI 配置
    QString uiinfoPath = QCoreApplication::applicationDirPath() + "/configs/default_aiimage_uiinfo.conf";

    QJsonDocument config = JsonUtils::jsonObjectFromFile(uiinfoPath);
    return config;
}

QString AIImageClip::getPlaceholderResource() const {
    // 返回生成中占位图
    QString placeholderPath = QCoreApplication::applicationDirPath() + "/assets/ai_image_placeholder.png";

    // 如果占位图不存在，返回空字符串（让系统使用默认处理）
    if (!QFile::exists(placeholderPath)) {
        qDebug() << "Placeholder image not found:" << placeholderPath;
        return QString();
    }

    return placeholderPath;
}

void AIImageClip::setReferenceImage(const QString &imagePath) {
    if (m_referenceImagePath != imagePath) {
        m_referenceImagePath = imagePath;
        emit referenceImageChanged(imagePath);
        qDebug() << "Reference image set:" << imagePath;
    }
}

bool AIImageClip::validateGenerationParams(const QJsonObject &params, QString &error) {
    // 验证必需的参数

    // 1. 检查 prompt
    if (!params.contains("ai_prompt") || params["ai_prompt"].toString().isEmpty()) {
        error = "Prompt cannot be empty";
        return false;
    }

    QString prompt = params["ai_prompt"].toString();
    if (prompt.length() < 3) {
        error = "Prompt is too short (minimum 3 characters)";
        return false;
    }

    if (prompt.length() > 2000) {
        error = "Prompt is too long (maximum 2000 characters)";
        return false;
    }

    // 2. 检查图片尺寸
    if (params.contains("ai_image_size")) {
        QString size = params["ai_image_size"].toString();
        QStringList validSizes = {"512x512", "768x768", "1024x1024", "1024x1792", "1792x1024"};
        if (!validSizes.contains(size)) {
            error = "Invalid image size: " + size;
            return false;
        }
    }

    // 3. 检查参考图片（如果提供）
    if (params.contains("ai_reference_image")) {
        QString refImage = params["ai_reference_image"].toString();
        if (!refImage.isEmpty() && !QFile::exists(refImage)) {
            error = "Reference image file not found: " + refImage;
            return false;
        }
    }

    // 4. 检查生成强度
    if (params.contains("ai_strength")) {
        double strength = params["ai_strength"].toDouble();
        if (strength < 0.0 || strength > 1.0) {
            error = "Generation strength must be between 0.0 and 1.0";
            return false;
        }
    }

    // 5. 检查生成步数
    if (params.contains("ai_steps")) {
        int steps = params["ai_steps"].toInt();
        if (steps < 10 || steps > 100) {
            error = "Generation steps must be between 10 and 100";
            return false;
        }
    }

    return true;
}

AIGenerationTask* AIImageClip::createGenerationTask(const QJsonObject &params) {
    return new AIImageGenerationTask(params, this);
}

void AIImageClip::applyGeneratedResource(const QString &resourcePath) {
    qDebug() << "AIImageClip::applyGeneratedResource" << resourcePath;

    // 创建新的 SkyResourceBean
    SkyResourceBean newBean(resourcePath, "AI Generated Image", AIImageResource);

    // 更新资源
    setResourceBean(newBean);

    // 更新 SkyClip
    if (skyClip()) {
        SkyResource skyResource = newBean.skyResouce();
        if (skyResource.isAvailable()) {
            // 更新 Clip 的资源
            skyClip()->updateResource(skyResource, skyClip()->getTrimRange());
            qDebug() << "AIImageClip resource updated successfully";
        } else {
            qWarning() << "Failed to load AI generated image:" << resourcePath;
        }
    }
}

void AIImageClip::onSaveInstanceState(QMap<QString, SkyVariant> &bundle) {
    AIGeneratedClip::onSaveInstanceState(bundle);

    // 保存参考图片路径
    bundle["ai_reference_image"] = SkyVariant(m_referenceImagePath.toUtf8().data());

    qDebug() << "AIImageClip::onSaveInstanceState" << toString();
}

void AIImageClip::onRestoreInstanceState(QMap<QString, SkyVariant> &bundle) {
    AIGeneratedClip::onRestoreInstanceState(bundle);

    // 恢复参考图片路径
    if (bundle.contains("ai_reference_image")) {
        m_referenceImagePath = QString::fromUtf8(bundle["ai_reference_image"].toString());
    }

    qDebug() << "AIImageClip::onRestoreInstanceState" << toString();
}
