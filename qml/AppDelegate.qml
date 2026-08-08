import QtQuick

Item {
    id: root
    width: 110
    height: 130

    signal clicked()

    property bool isCurrentItem: GridView.isCurrentItem

    Rectangle {
        id: background
        anchors.fill: parent
        anchors.margins: 4
        radius: typeof ThemeManager !== "undefined" ? ThemeManager.borderRadius : 12
        color: (mouseArea.containsMouse || isCurrentItem) ? 
            Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa", 0.25) : 
            "transparent"

        border.color: isCurrentItem ? 
            (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
            "transparent"
        border.width: isCurrentItem ? 1 : 0

        scale: mouseArea.pressed ? 0.95 : ((mouseArea.containsMouse || isCurrentItem) ? 1.05 : 1.0)

        Behavior on scale {
            SpringAnimation { spring: 3; damping: 0.2; epsilon: 0.01 }
        }
        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        Column {
            anchors.centerIn: parent
            spacing: 6

            Image {
                id: appIcon
                width: typeof ThemeManager !== "undefined" ? ThemeManager.iconSize : 48
                height: width
                anchors.horizontalCenter: parent.horizontalCenter
                visible: typeof ThemeManager !== "undefined" ? ThemeManager.showIcons : true
                source: model.iconName ? "image://icon/" + model.iconName : ""
                sourceSize: Qt.size(width, height)
                fillMode: Image.PreserveAspectFit
            }

            Text {
                text: model.name || "App"
                color: typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#ffffff"
                font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 13
                font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                width: 96
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                maximumLineCount: 2
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.clicked()
        }
    }
}
