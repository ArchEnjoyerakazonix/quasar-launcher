import QtQuick

Rectangle {
    id: root
    property string presetName: "Preset"
    property bool isSelected: false
    signal clicked()

    width: Math.max(130, labelText.implicitWidth + 24)
    height: 36
    radius: 8
    color: isSelected ? "#8aadf4" : (mouseArea.containsMouse ? "#313244" : "#181825")
    border.color: isSelected ? "#8aadf4" : "#313244"
    border.width: 1

    Behavior on color { ColorAnimation { duration: 150 } }

    Text {
        id: labelText
        anchors.centerIn: parent
        text: root.presetName
        color: root.isSelected ? "#11111b" : "#cdd6f4"
        font.pixelSize: 12
        font.bold: root.isSelected
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}

