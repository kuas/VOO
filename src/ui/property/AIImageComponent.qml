import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQml 2.12
import QtQuick.Layouts 1.3
import AIImageComponent 1.0
import "../widget/"

Item {
    id: aiImageRoot
    width: parent.width
    height: mainColumn.height + 20

    // AI Image 编辑界面的专用组件
    // 此组件管理 AI 图片生成的复杂内部数据和 UI 交互

    AIImageComponent {
        id: aiImageComponent

        onBindComponent: {
            console.log("AIImageComponent bound successfully")
            promptInput.text = aiImageComponent.getPrompt()
            modelSelector.currentIndex = 0  // TODO: Set based on saved model
            sizeSelector.currentIndex = 2   // Default to 1024x1024
        }

        onGenerationStarted: {
            console.log("AI Image generation started")
            // TODO: Show loading indicator
        }

        onGenerationCompleted: {
            console.log("AI Image generation completed:", imagePath)
            // TODO: Update preview with generated image
        }

        onGenerationFailed: {
            console.log("AI Image generation failed:", error)
            // TODO: Show error message
        }
    }

    Column {
        id: mainColumn
        width: parent.width - 16
        x: 8
        spacing: 10
        topPadding: 10
        bottomPadding: 10

        // AI 生成配置区域
        Rectangle {
                width: parent.width
                height: generationSection.height + 20
                color: "#1E2028"
                radius: 4

                Column {
                    id: generationSection
                    width: parent.width - 20
                    x: 10
                    y: 10
                    spacing: 12

                    // 标题
                    Text {
                        text: qsTr("AI Generation")
                        color: "#FFFFFF"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    // 提示词输入区
                    Column {
                        width: parent.width
                        spacing: 6

                        Text {
                            text: qsTr("Prompt")
                            color: "#BFBFBF"
                            font.pixelSize: 12
                        }

                        TextArea {
                            id: promptInput
                            width: parent.width
                            height: 80
                            color: "#FFFFFF"
                            placeholderText: qsTr("Enter your prompt here...")
                            placeholderTextColor: "#666666"
                            wrapMode: TextArea.Wrap
                            onTextChanged: {
                                aiImageComponent.setPrompt(text)
                            }
                            background: Rectangle {
                                color: "#15171C"
                                border.color: promptInput.activeFocus ? "#4A90E2" : "#2A2C33"
                                border.width: 1
                                radius: 3
                            }
                        }
                    }

                    // 模型选择
                    Column {
                        width: parent.width
                        spacing: 6

                        Text {
                            text: qsTr("Model")
                            color: "#BFBFBF"
                            font.pixelSize: 12
                        }

                        ComboBox {
                            id: modelSelector
                            width: parent.width
                            model: ["Gemini 3 Pro Image Preview", "Gemini 2.5 Flash Image"]
                            onCurrentTextChanged: {
                                aiImageComponent.setModel(currentText)
                            }
                            background: Rectangle {
                                color: "#15171C"
                                border.color: "#2A2C33"
                                border.width: 1
                                radius: 3
                            }
                        }
                    }

                    // 图片尺寸选择
                    Column {
                        width: parent.width
                        spacing: 6

                        Text {
                            text: qsTr("Image Size")
                            color: "#BFBFBF"
                            font.pixelSize: 12
                        }

                        ComboBox {
                            id: sizeSelector
                            width: parent.width
                            model: ["512x512", "768x768", "1024x1024", "1024x1792", "1792x1024"]
                            currentIndex: 2
                            onCurrentTextChanged: {
                                aiImageComponent.setImageSize(currentText)
                            }
                            background: Rectangle {
                                color: "#15171C"
                                border.color: "#2A2C33"
                                border.width: 1
                                radius: 3
                            }
                        }
                    }

                    // 生成按钮
                    Rectangle {
                        width: parent.width
                        height: 36
                        color: generateButton.pressed ? "#3A7BC8" : (generateButton.hovered ? "#5A9FE2" : "#4A90E2")
                        radius: 4

                        Text {
                            text: qsTr("Generate")
                            color: "#FFFFFF"
                            anchors.centerIn: parent
                            font.pixelSize: 13
                        }

                        MouseArea {
                            id: generateButton
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                console.log("Generate AI Image clicked")
                                aiImageComponent.generateImage()
                            }
                        }
                    }
                }
            }
    }

    Component.onCompleted: {
        console.log("AIImageComponent QML loaded")
        onComponentLoaded(aiImageComponent)
    }
}
