import QtQuick
import com.quasar.launcher 1.0

Item {
    id: root
    width: ListView.view ? ListView.view.width : 600
    height: (typeof ThemeManager !== "undefined" && ThemeManager.layoutMode === "compact") ? 32 : (typeof ThemeManager !== "undefined" && ThemeManager.showIcons ? Math.max(ThemeManager.iconSize + 12, 40) : 38)

    signal clicked()

    function activate() {
        root.clicked()
    }

    property bool isCurrentItem: ListView.isCurrentItem

    Rectangle {
        id: background
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        anchors.topMargin: 2
        anchors.bottomMargin: 2
        radius: typeof ThemeManager !== "undefined" ? Math.min(ThemeManager.borderRadius, 6) : 6

        // Glass "pill" highlight: translucent accent fill + hairline border
        // instead of a solid slab — reads as premium and keeps text legible.
        color: (mouseArea.containsMouse || isCurrentItem) ?
            Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa",
                     isCurrentItem ? 0.22 : 0.12) :
            "transparent"
        border.color: (mouseArea.containsMouse || isCurrentItem) ?
            Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa",
                     isCurrentItem ? 0.55 : 0.25) :
            "transparent"
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: 100 }
        }
        Behavior on border.color {
            ColorAnimation { duration: 100 }
        }

        scale: mouseArea.pressed ? 0.98 : 1.0
        Behavior on scale { NumberAnimation { duration: 80 } }

        Row {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            Item {
                width: (typeof ThemeManager !== "undefined" && ThemeManager.layoutMode === "compact") ? 20 : (typeof ThemeManager !== "undefined" ? ThemeManager.iconSize : 24)
                height: width
                anchors.verticalCenter: parent.verticalCenter
                visible: typeof ThemeManager !== "undefined" ? ThemeManager.showIcons : true

                Image {
                    id: appIcon
                    anchors.fill: parent
                    source: (typeof modelData !== "undefined" && modelData && modelData.class) ? 
                        ("image://icon/" + modelData.class.toLowerCase()) : 
                        (model.iconName ? "image://icon/" + model.iconName : "")
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
                    visible: appIcon.status === Image.Error || appIcon.status === Image.Null || (!appIcon.source.toString())

                    Text {
                        anchors.centerIn: parent
                        text: {
                            if (model.desktopFile && model.desktopFile.startsWith("emoji:")) {
                                return model.desktopFile.substring(6)
                            }
                            var t = (typeof modelData !== "undefined" && modelData && modelData.title) ? modelData.title : (model.name || "")
                            return (t && t.length > 0) ? t.substring(0, 1).toUpperCase() : "◆"
                        }
                        color: typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa"
                        font.pixelSize: (model.desktopFile && model.desktopFile.startsWith("emoji:")) ? Math.max(16, parent.width * 0.7) : Math.max(10, parent.width * 0.5)
                        font.bold: true
                        font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                    }
                }
            }

            Text {
                id: appName
                text: (typeof modelData !== "undefined" && modelData && modelData.title) ? 
                    modelData.title : 
                    (model.highlightedName || model.name || "")
                textFormat: Text.StyledText
                color: typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4"
                font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 14
                font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                font.bold: isCurrentItem
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideRight
                width: Math.min(implicitWidth, parent.width * 0.42)
            }

            Text {
                id: commentText
                text: (typeof modelData !== "undefined" && modelData && modelData.class) ? 
                    (modelData.class + (modelData.workspace ? " | Workspace " + modelData.workspace : "")) : 
                    (model.genericName || model.comment || "")
                color: Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8",
                               (mouseArea.containsMouse || isCurrentItem) ? 0.9 : 0.7)
                font.pixelSize: (typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 14) - 2
                font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                anchors.verticalCenter: parent.verticalCenter
                visible: text.length > 0 && parent.width > 250
                elide: Text.ElideRight
                width: Math.max(0, parent.width - (appIcon.visible ? appIcon.width + parent.spacing : 0) - appName.width - (actionBadge.visible ? actionBadge.width + parent.spacing + 20 : 0) - parent.spacing * 2)
            }
        }

        // Action badge on right for current item (like "Open" / "↵ Launch")
        Rectangle {
            id: actionBadge
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            height: 22
            width: actionText.width + 16
            radius: 4
            color: "#ffffff25"
            visible: isCurrentItem
            opacity: isCurrentItem ? 1 : 0

            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }

            Text {
                id: actionText
                anchors.centerIn: parent
                text: "Open ↵"
                color: "#ffffff"
                font.pixelSize: 11
                font.bold: true
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
