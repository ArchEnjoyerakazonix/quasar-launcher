import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    property alias text: textInput.text
    property bool isFocused: textInput.activeFocus

    width: parent ? parent.width - 32 : 600
    height: (typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 14) * 2 + 16
    radius: typeof ThemeManager !== "undefined" ? ThemeManager.borderRadius : 10

    color: Qt.alpha(
        typeof ThemeManager !== "undefined" ? ThemeManager.cardColor : "#1e1e2e",
        typeof ThemeManager !== "undefined" ? ThemeManager.cardOpacity : 0.9
    )
    border.color: isFocused ? 
        (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
        (typeof ThemeManager !== "undefined" ? ThemeManager.borderColor : "#313244")
    border.width: typeof ThemeManager !== "undefined" ? ThemeManager.borderWidth : 1

    Behavior on border.color {
        ColorAnimation { duration: 150 }
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        spacing: 8

        Text {
            id: promptLabel
            text: typeof ThemeManager !== "undefined" ? ThemeManager.promptText : ""
            font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize + 1 : 15
            font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
            font.bold: true
            color: typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa"
            anchors.verticalCenter: parent.verticalCenter
            visible: text.length > 0
        }

        TextInput {
            id: textInput
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - (promptLabel.visible ? promptLabel.width + parent.spacing : 0)
            font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize + 1 : 15
            font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
            color: typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4"
            clip: true

            Text {
                text: "Type to filter..."
                color: Qt.alpha(
                    typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8",
                    0.5
                )
                font.pixelSize: textInput.font.pixelSize
                font.family: textInput.font.family
                visible: !textInput.text
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    function forceActiveFocus() {
        textInput.forceActiveFocus()
    }
}
