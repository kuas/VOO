import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQml 2.12
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import AIImageComponent 1.0
import "../widget/"

Item {
    id: aiImageRoot
    width: parent.width
    height: mainColumn.height + 20

    // AI Image 编辑界面的专用组件
    // 此组件管理 AI 图片生成的复杂内部数据和 UI 交互

    property var selectedImages: []
    property bool isGenerating: false
    property string generatedImagePath: ""
    property string errorMessage: ""

    AIImageComponent {
        id: aiImageComponent

        onBindComponent: {
            console.log("AIImageComponent bound successfully")
            promptInput.text = aiImageComponent.getPrompt()
        }

        onGenerationStarted: {
            console.log("AI Image generation started")
            isGenerating = true
            errorMessage = ""
            generatedImagePath = ""
        }

        onGenerationCompleted: {
            console.log("AI Image generation completed:", imagePath)
            isGenerating = false
            generatedImagePath = imagePath
            errorMessage = ""
        }

        onGenerationFailed: {
            console.log("AI Image generation failed:", error)
            isGenerating = false
            errorMessage = error
            generatedImagePath = ""
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

                    // 参考图片选择区
                    Column {
                        width: parent.width
                        spacing: 6

                        Text {
                            text: qsTr("Reference Images (Optional)")
                            color: "#BFBFBF"
                            font.pixelSize: 12
                        }

                        // 添加图片按钮
                        Rectangle {
                            width: parent.width
                            height: 36
                            color: addImageButton.pressed ? "#15171C" : (addImageButton.hovered ? "#2A2C33" : "#1E2028")
                            border.color: "#4A90E2"
                            border.width: 1
                            radius: 4

                            Row {
                                anchors.centerIn: parent
                                spacing: 8

                                Text {
                                    text: "+"
                                    color: "#4A90E2"
                                    font.pixelSize: 18
                                    font.bold: true
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    text: qsTr("Add Images")
                                    color: "#4A90E2"
                                    font.pixelSize: 13
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                id: addImageButton
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    fileDialog.open()
                                }
                            }
                        }

                        // 已选择的图片列表
                        Flow {
                            id: imageGrid
                            width: parent.width
                            spacing: 8
                            visible: selectedImages.length > 0

                            Repeater {
                                model: selectedImages

                                Rectangle {
                                    width: 80
                                    height: 80
                                    color: "#15171C"
                                    border.color: "#2A2C33"
                                    border.width: 1
                                    radius: 4

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        source: "file://" + modelData
                                        fillMode: Image.PreserveAspectCrop
                                        asynchronous: true
                                    }

                                    // 删除按钮
                                    Rectangle {
                                        width: 20
                                        height: 20
                                        radius: 10
                                        color: "#E74C3C"
                                        anchors.top: parent.top
                                        anchors.right: parent.right
                                        anchors.margins: -6

                                        Text {
                                            text: "×"
                                            color: "#FFFFFF"
                                            font.pixelSize: 14
                                            font.bold: true
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                var newImages = []
                                                for (var i = 0; i < selectedImages.length; i++) {
                                                    if (i !== index) {
                                                        newImages.push(selectedImages[i])
                                                    }
                                                }
                                                selectedImages = newImages
                                                // Update backend
                                                aiImageComponent.setReferenceImages(selectedImages)
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // 图片数量提示
                        Text {
                            text: qsTr("%1 image(s) selected").arg(selectedImages.length)
                            color: "#666666"
                            font.pixelSize: 11
                            visible: selectedImages.length > 0
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

                        ButtonGroup {
                            id: modelGroup
                            onCheckedButtonChanged: {
                                if (checkedButton) {
                                    aiImageComponent.setModel(checkedButton.text)
                                }
                            }
                        }

                        Column {
                            width: parent.width
                            spacing: 8

                            RadioButton {
                                id: gemini3ProRadio
                                text: "Gemini 3 Pro Image Preview"
                                checked: true
                                ButtonGroup.group: modelGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"

                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }

                            RadioButton {
                                id: gemini25FlashRadio
                                text: "Gemini 2.5 Flash Image"
                                ButtonGroup.group: modelGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"

                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                        }
                    }

                    // 长宽比选择
                    Column {
                        width: parent.width
                        spacing: 6

                        Text {
                            text: qsTr("Aspect Ratio")
                            color: "#BFBFBF"
                            font.pixelSize: 12
                        }

                        ButtonGroup {
                            id: aspectRatioGroup
                            onCheckedButtonChanged: {
                                if (checkedButton) {
                                    var ratioValue = checkedButton.ratioValue
                                    aiImageComponent.setAspectRatio(ratioValue)
                                }
                            }
                        }

                        Row {
                            width: parent.width
                            spacing: 16

                            RadioButton {
                                property string ratioValue: "1:1"
                                text: "1:1"
                                checked: true
                                ButtonGroup.group: aspectRatioGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }

                            RadioButton {
                                property string ratioValue: "16:9"
                                text: "16:9"
                                ButtonGroup.group: aspectRatioGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }

                            RadioButton {
                                property string ratioValue: "9:16"
                                text: "9:16"
                                ButtonGroup.group: aspectRatioGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }

                            RadioButton {
                                property string ratioValue: "4:3"
                                text: "4:3"
                                ButtonGroup.group: aspectRatioGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
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

                        ButtonGroup {
                            id: sizeGroup
                            onCheckedButtonChanged: {
                                if (checkedButton) {
                                    var sizeValue = checkedButton.sizeValue
                                    aiImageComponent.setImageSize(sizeValue)
                                }
                            }
                        }

                        Row {
                            width: parent.width
                            spacing: 16

                            RadioButton {
                                property string sizeValue: "1024x1024"
                                text: "1K"
                                checked: true
                                ButtonGroup.group: sizeGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }

                            RadioButton {
                                property string sizeValue: "2048x2048"
                                text: "2K"
                                ButtonGroup.group: sizeGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }

                            RadioButton {
                                property string sizeValue: "4096x4096"
                                text: "4K"
                                ButtonGroup.group: sizeGroup
                                contentItem: Text {
                                    text: parent.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    leftPadding: parent.indicator.width + parent.spacing
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 16
                                    implicitHeight: 16
                                    x: 0
                                    y: parent.height / 2 - height / 2
                                    radius: 8
                                    border.color: parent.checked ? "#4A90E2" : "#BFBFBF"
                                    border.width: 2
                                    color: "transparent"
                                    Rectangle {
                                        width: 8
                                        height: 8
                                        x: 4
                                        y: 4
                                        radius: 4
                                        color: "#4A90E2"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                        }
                    }

                    // 生成按钮
                    Rectangle {
                        width: parent.width
                        height: 36
                        color: isGenerating ? "#666666" : (generateButton.pressed ? "#3A7BC8" : (generateButton.hovered ? "#5A9FE2" : "#4A90E2"))
                        radius: 4

                        Row {
                            anchors.centerIn: parent
                            spacing: 8

                            // Loading indicator
                            Rectangle {
                                width: 16
                                height: 16
                                color: "transparent"
                                visible: isGenerating
                                anchors.verticalCenter: parent.verticalCenter

                                Canvas {
                                    id: loadingCanvas
                                    anchors.fill: parent
                                    property real rotation: 0

                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.clearRect(0, 0, width, height);
                                        ctx.strokeStyle = "#FFFFFF";
                                        ctx.lineWidth = 2;
                                        ctx.lineCap = "round";

                                        ctx.translate(width/2, height/2);
                                        ctx.rotate(rotation * Math.PI / 180);
                                        ctx.translate(-width/2, -height/2);

                                        ctx.beginPath();
                                        ctx.arc(width/2, height/2, 6, 0, 1.5 * Math.PI);
                                        ctx.stroke();
                                    }

                                    Timer {
                                        running: isGenerating
                                        repeat: true
                                        interval: 16
                                        onTriggered: {
                                            parent.rotation = (parent.rotation + 5) % 360
                                            parent.requestPaint()
                                        }
                                    }
                                }
                            }

                            Text {
                                text: isGenerating ? qsTr("Generating...") : qsTr("Generate")
                                color: "#FFFFFF"
                                font.pixelSize: 13
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            id: generateButton
                            anchors.fill: parent
                            hoverEnabled: true
                            enabled: !isGenerating
                            onClicked: {
                                console.log("Generate AI Image clicked")
                                aiImageComponent.generateImage()
                            }
                        }
                    }

                    // Error message display
                    Rectangle {
                        width: parent.width
                        height: errorText.height + 16
                        color: "#4D2929"
                        border.color: "#E74C3C"
                        border.width: 1
                        radius: 4
                        visible: errorMessage !== ""

                        Text {
                            id: errorText
                            text: errorMessage
                            color: "#FF6B6B"
                            font.pixelSize: 12
                            wrapMode: Text.Wrap
                            width: parent.width - 16
                            anchors.centerIn: parent
                        }
                    }

                    // Generated image preview
                    Rectangle {
                        width: parent.width
                        height: 200
                        color: "#15171C"
                        border.color: "#2A2C33"
                        border.width: 1
                        radius: 4
                        visible: generatedImagePath !== ""

                        Column {
                            anchors.centerIn: parent
                            spacing: 8
                            width: parent.width - 16

                            Text {
                                text: qsTr("Generated Image")
                                color: "#BFBFBF"
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Image {
                                width: parent.width
                                height: 160
                                source: generatedImagePath !== "" ? "file://" + generatedImagePath : ""
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                            }

                            Text {
                                text: qsTr("Saved to: ") + generatedImagePath
                                color: "#666666"
                                font.pixelSize: 10
                                wrapMode: Text.Wrap
                                width: parent.width
                            }
                        }
                    }
                }
            }
    }

    // 文件选择对话框
    FileDialog {
        id: fileDialog
        title: qsTr("Select Images")
        folder: shortcuts.pictures
        selectMultiple: true
        nameFilters: ["Image files (*.png *.jpg *.jpeg *.bmp *.gif *.webp)", "All files (*)"]

        onAccepted: {
            var newImages = []
            // 保留已有的图片
            for (var i = 0; i < selectedImages.length; i++) {
                newImages.push(selectedImages[i])
            }
            // 添加新选择的图片
            for (var j = 0; j < fileDialog.fileUrls.length; j++) {
                var filePath = fileDialog.fileUrls[j].toString()
                // 移除 "file://" 前缀
                if (filePath.startsWith("file://")) {
                    filePath = filePath.substring(7)
                }
                newImages.push(filePath)
            }
            selectedImages = newImages
            console.log("Selected images:", selectedImages.length)
            // Update backend
            aiImageComponent.setReferenceImages(selectedImages)
        }
    }

    Component.onCompleted: {
        console.log("AIImageComponent QML loaded")
        onComponentLoaded(aiImageComponent)
    }
}
