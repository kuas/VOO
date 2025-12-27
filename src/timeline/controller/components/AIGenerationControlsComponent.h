#ifndef AIGENERATIONCONTROLSCOMPONENT_H
#define AIGENERATIONCONTROLSCOMPONENT_H

#include "BaseComponent.h"
#include "../../models/ai/aigeneratedclip.h"

/**
 * @brief AI 生成控制组件
 *
 * 用于 AI Clip 的编辑面板，提供：
 * - 显示生成状态
 * - 显示生成进度
 * - 显示错误信息
 * - 生成按钮（Generate）
 * - 取消按钮（Cancel）
 * - 重试按钮（Retry）
 */
class AIGenerationControlsComponent : public BaseComponent {
    Q_OBJECT
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit AIGenerationControlsComponent(QObject *parent = nullptr);
    ~AIGenerationControlsComponent() override;

    void onBindQml(QJsonObject &data) override;
    void onUnBind() override;

    // 状态访问
    int state() const { return m_state; }
    double progress() const { return m_progress; }
    QString errorMessage() const { return m_errorMessage; }

    // 控制方法
    Q_INVOKABLE void startGeneration();
    Q_INVOKABLE void cancelGeneration();
    Q_INVOKABLE void retryGeneration();

signals:
    void stateChanged(int state);
    void progressChanged(double progress);
    void errorMessageChanged(const QString &error);

private slots:
    void onClipStateChanged(AIGenerationState state);
    void onClipProgressChanged(double progress);
    void onClipErrorChanged(const QString &error);

private:
    // 收集所有生成参数
    QJsonObject collectGenerationParams();

private:
    int m_state = AIIdle;
    double m_progress = 0.0;
    QString m_errorMessage;
    AIGeneratedClip *m_aiClip = nullptr;
};

#endif // AIGENERATIONCONTROLSCOMPONENT_H
