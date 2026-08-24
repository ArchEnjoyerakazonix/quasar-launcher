import QtQuick
import QtQuick.Controls

Row {
    id: root
    property string label: "Color"
    property string colorValue: "#ffffff"
    signal colorChanged(string newColor)

    spacing: 12
    width: parent ? parent.width : 300

    Text {
        text: root.label
        color: "#cdd6f4"
        font.pixelSize: 13
        width: 140
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        width: 24
        height: 24
        radius: 4
        color: root.colorValue
        border.color: "#ffffff40"
        border.width: 1
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        width: 110
        height: 28
        radius: 6
        color: "#181825"
        border.color: "#313244"
        border.width: 1
        anchors.verticalCenter: parent.verticalCenter

        TextInput {
            anchors.fill: parent
            anchors.margins: 4
            text: root.colorValue
            color: "#cdd6f4"
            font.pixelSize: 12
            font.family: "Monospace"
            verticalAlignment: TextInput.AlignVCenter
            onEditingFinished: {
                root.colorChanged(text)
            }
        }
    }
}
