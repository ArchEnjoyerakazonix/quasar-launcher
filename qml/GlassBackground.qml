import QtQuick

Item {
    id: root
    signal backgroundClicked()

    // Screen overlay backdrop
    Rectangle {
        anchors.fill: parent
        color: typeof ThemeManager !== "undefined" && ThemeManager.enableDimOverlay ? Qt.rgba(0, 0, 0, 0.4) : "transparent"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.backgroundClicked()
    }
}
