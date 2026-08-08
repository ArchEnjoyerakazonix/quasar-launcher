import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    property alias text: textInput.text
    property bool isFocused: textInput.activeFocus

    width: textInput.text.length > 0 ? 720 : 680
    height: 56
    radius: 16

    color: Qt.rgba(255/255, 255/255, 255/255, 0.08)
    border.color: isFocused ? "#7C3AED" : Qt.rgba(255/255, 255/255, 255/255, 0.12)
    border.width: isFocused ? 2 : 1

    Behavior on width {
        SpringAnimation { spring: 3; damping: 0.2; epsilon: 0.25 }
    }

    Behavior on border.color {
        ColorAnimation { duration: 200; easing.type: Easing.OutQuint }
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 12

        Text {
            text: "🔍"
            font.pixelSize: 20
            color: Qt.rgba(255/255, 255/255, 255/255, 0.6)
            anchors.verticalCenter: parent.verticalCenter
        }

        TextInput {
            id: textInput
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 40 - 12
            font.pixelSize: 18
            color: "white"
            clip: true

            Text {
                text: "Search applications..."
                color: Qt.rgba(255/255, 255/255, 255/255, 0.4)
                font.pixelSize: 18
                visible: !textInput.text
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    function forceActiveFocus() {
        textInput.forceActiveFocus()
    }
}
