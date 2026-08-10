import QtQuick
import QtQuick.Controls
import com.quasar.launcher 1.0

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

    Shortcut {
        sequence: "Ctrl+U"
        onActivated: textInput.text = ""
    }

    signal returnPressed()
    signal downPressed()

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
            width: parent.width - (promptLabel.visible ? promptLabel.width + parent.spacing : 0) - (clearBtn.visible ? clearBtn.width + parent.spacing : 0)
            font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize + 1 : 15
            font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
            color: typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4"
            clip: true

            onAccepted: root.returnPressed()
            Keys.onReturnPressed: root.returnPressed()
            Keys.onDownPressed: root.downPressed()

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

        Rectangle {
            id: clearBtn
            width: 22
            height: 22
            radius: 11
            anchors.verticalCenter: parent.verticalCenter
            visible: textInput.text.length > 0
            color: clearMouse.containsMouse ? 
                Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa", 0.3) : 
                "transparent"

            Text {
                text: "✕"
                anchors.centerIn: parent
                font.pixelSize: 12
                color: clearMouse.containsMouse ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.6)
            }

            MouseArea {
                id: clearMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    textInput.text = ""
                    textInput.forceActiveFocus()
                }
            }
        }
    }

    function forceActiveFocus() {
        textInput.forceActiveFocus()
    }
}

