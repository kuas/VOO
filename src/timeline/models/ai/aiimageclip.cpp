#include "aiimageclip.h"
#include "aigenerationtask.h"
#include "../TimelineDef.h"

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
    QJsonObject root;
    root["duration"] = 5000;  // 默认时长 5 秒

    QJsonArray paramList;

    // 1. 模型选择
    QJsonObject modelParam;
    modelParam["id"] = 1;
    modelParam["uiType"] = "DropdownList";
    modelParam["paramTitle"] = "Model";
    modelParam["paramName"] = "ai_model";
    modelParam["valueType"] = "string";
    modelParam["defValue"] = "gemini-3-pro-image-preview";
    QJsonArray modelList;
    modelList << "Gemini 3 Pro Image Preview" << "Gemini 2.5 Flash Image";
    QJsonArray modelValueList;
    modelValueList << "gemini-3-pro-image-preview" << "vertex_ai/gemini-2.5-flash-image";
    modelParam["dataList"] = modelList;
    modelParam["dataValueList"] = modelValueList;
    modelParam["group"] = "generation";
    paramList.append(modelParam);

    // 2. 提示词输入
    QJsonObject promptParam;
    promptParam["id"] = 2;
    promptParam["uiType"] = "Text";
    promptParam["paramTitle"] = "Prompt";
    promptParam["paramName"] = "ai_prompt";
    promptParam["valueType"] = "string";
    promptParam["defValue"] = "";
    promptParam["group"] = "generation";
    paramList.append(promptParam);

    // 2. 参考图片（图生图）
    QJsonObject referenceParam;
    referenceParam["id"] = 3;
    referenceParam["uiType"] = "Image";
    referenceParam["paramTitle"] = "Reference Image (Optional)";
    referenceParam["paramName"] = "ai_reference_image";
    referenceParam["valueType"] = "string";
    referenceParam["defValue"] = "";
    referenceParam["group"] = "generation";
    paramList.append(referenceParam);

    // 3. 图片尺寸
    QJsonObject sizeParam;
    sizeParam["id"] = 4;
    sizeParam["uiType"] = "DropdownList";
    sizeParam["paramTitle"] = "Image Size";
    sizeParam["paramName"] = "ai_image_size";
    sizeParam["valueType"] = "string";
    sizeParam["defValue"] = "1024x1024";
    QJsonArray sizeList;
    sizeList << "512x512" << "768x768" << "1024x1024" << "1024x1792" << "1792x1024";
    sizeParam["dataList"] = sizeList;
    sizeParam["dataValueList"] = sizeList;
    sizeParam["group"] = "generation";
    paramList.append(sizeParam);

    // 4. 生成风格
    QJsonObject styleParam;
    styleParam["id"] = 5;
    styleParam["uiType"] = "DropdownList";
    styleParam["paramTitle"] = "Style Preset";
    styleParam["paramName"] = "ai_style_preset";
    styleParam["valueType"] = "string";
    styleParam["defValue"] = "none";
    QJsonArray styleList;
    styleList << "None" << "Photorealistic" << "Anime" << "Digital Art" << "Oil Painting" << "Watercolor";
    QJsonArray styleValueList;
    styleValueList << "none" << "photorealistic" << "anime" << "digital-art" << "oil-painting" << "watercolor";
    styleParam["dataList"] = styleList;
    styleParam["dataValueList"] = styleValueList;
    styleParam["group"] = "generation";
    paramList.append(styleParam);

    // 5. 生成强度（图生图时使用）
    QJsonObject strengthParam;
    strengthParam["id"] = 6;
    strengthParam["uiType"] = "Slider";
    strengthParam["paramTitle"] = "Generation Strength";
    strengthParam["paramName"] = "ai_strength";
    strengthParam["valueType"] = "float";
    strengthParam["minValue"] = "0.0";
    strengthParam["maxValue"] = "1.0";
    strengthParam["defValue"] = "0.8";
    strengthParam["decimals"] = 2;
    strengthParam["group"] = "generation";
    paramList.append(strengthParam);

    // 6. 生成步数
    QJsonObject stepsParam;
    stepsParam["id"] = 7;
    stepsParam["uiType"] = "Slider";
    stepsParam["paramTitle"] = "Generation Steps";
    stepsParam["paramName"] = "ai_steps";
    stepsParam["valueType"] = "int";
    stepsParam["minValue"] = "10";
    stepsParam["maxValue"] = "100";
    stepsParam["defValue"] = "30";
    stepsParam["group"] = "generation";
    paramList.append(stepsParam);

    // 7. 随机种子
    QJsonObject seedParam;
    seedParam["id"] = 8;
    seedParam["uiType"] = "Slider";
    seedParam["paramTitle"] = "Random Seed";
    seedParam["paramName"] = "ai_seed";
    seedParam["valueType"] = "int";
    seedParam["minValue"] = "-1";
    seedParam["maxValue"] = "999999999";
    seedParam["defValue"] = "-1";
    seedParam["group"] = "generation";
    paramList.append(seedParam);

    // 8. AI 生成控制组件
    QJsonObject controlsParam;
    controlsParam["id"] = 9;
    controlsParam["uiType"] = "AIGenerationControls";
    controlsParam["paramTitle"] = "Generation Controls";
    controlsParam["group"] = "generation";
    controlsParam["groupTitle"] = "AI Generation";
    controlsParam["groupLayout"] = "Column";
    paramList.append(controlsParam);

    // 9. Transform 组件
    QJsonObject transformParam;
    transformParam["id"] = 10;
    transformParam["uiType"] = "Transform";
    transformParam["group"] = "transform";
    transformParam["groupTitle"] = "Transform";
    transformParam["groupLayout"] = "Column";
    paramList.append(transformParam);

    root["paramList"] = paramList;

    // 多语言翻译
    QJsonObject translations;
    QJsonObject zhTranslations;
    zhTranslations["Prompt"] = "提示词";
    zhTranslations["Reference Image (Optional)"] = "参考图片（可选）";
    zhTranslations["Image Size"] = "图片尺寸";
    zhTranslations["Style Preset"] = "生成风格";
    zhTranslations["Generation Strength"] = "生成强度";
    zhTranslations["Generation Steps"] = "生成步数";
    zhTranslations["Random Seed"] = "随机种子";
    zhTranslations["Generation Controls"] = "生成控制";
    zhTranslations["Transform"] = "变换";
    zhTranslations["AI Generation"] = "AI 生成";
    translations["zh"] = zhTranslations;
    root["translations"] = translations;

    return QJsonDocument(root);
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
