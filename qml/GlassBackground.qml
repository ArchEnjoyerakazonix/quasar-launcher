import QtQuick

Item {
    id: root
    signal backgroundClicked()

    // Screen overlay backdrop
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.4)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.backgroundClicked()
    }
}
