import QtQuick

Item {
    id: root
    signal backgroundClicked()

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(10/255, 10/255, 20/255, 0.65)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.backgroundClicked()
    }
}
