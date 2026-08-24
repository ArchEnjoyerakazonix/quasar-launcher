import QtQuick
import com.quasar.launcher 1.0

Item {
    id: root
    width: GridView.view ? GridView.view.cellWidth : 120
    height: GridView.view ? GridView.view.cellHeight : 140

    signal clicked()
    function activate() {
        root.clicked()
    }

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

            Item {
                width: typeof ThemeManager !== "undefined" ? ThemeManager.iconSize : 48
                height: width
                anchors.horizontalCenter: parent.horizontalCenter
                visible: typeof ThemeManager !== "undefined" ? ThemeManager.showIcons : true

                Image {
                    id: appIcon
                    anchors.fill: parent
                    source: model.iconName ? "image://icon/" + model.iconName : ""
                    sourceSize: Qt.size(parent.width, parent.height)
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                }

                Rectangle {
                    anchors.fill: parent
                    radius: Math.max(parent.width * 0.2, 4)
                    color: Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa", 0.2)
                    border.color: Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa", 0.5)
                    border.width: 1
                    visible: appIcon.status === Image.Error || appIcon.status === Image.Null || !model.iconName

                    Text {
                        anchors.centerIn: parent
                        text: {
                            if (model.desktopFile && model.desktopFile.startsWith("emoji:")) {
                                return model.desktopFile.substring(6)
                            }
                            return (model.name && model.name.length > 0) ? model.name.substring(0, 1).toUpperCase() : "◆"
                        }
                        color: typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa"
                        font.pixelSize: (model.desktopFile && model.desktopFile.startsWith("emoji:")) ? Math.max(18, parent.width * 0.7) : Math.max(12, parent.width * 0.45)
                        font.bold: true
                        font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                    }
                }
            }

            Text {
                text: model.name || "App"
                color: typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#ffffff"
                font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 13
                font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                width: root.width - 16
                anchors.horizontalCenter: parent.horizontalCenter
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
