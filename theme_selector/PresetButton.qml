import QtQuick

Rectangle {
    id: root
    property string presetName: "Preset"
    property bool isSelected: false
    signal clicked()

    width: 140
    height: 38
    radius: 8
    color: isSelected ? "#8aadf4" : (mouseArea.containsMouse ? "#313244" : "#1e1e2e")
    border.color: isSelected ? "#8aadf4" : "#45475a"
    border.width: 1

    Behavior on color { ColorAnimation { duration: 150 } }

    Text {
        anchors.centerIn: parent
        text: root.presetName
        color: root.isSelected ? "#11111b" : "#cdd6f4"
        font.pixelSize: 13
        font.bold: root.isSelected
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
