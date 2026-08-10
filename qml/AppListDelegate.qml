import QtQuick
import com.quasar.launcher 1.0

Item {
    id: root
    width: ListView.view ? ListView.view.width : 600
    height: (typeof ThemeManager !== "undefined" && ThemeManager.showIcons ? Math.max(ThemeManager.iconSize + 12, 40) : 38)

    signal clicked()

    property bool isCurrentItem: ListView.isCurrentItem

    Rectangle {
        id: background
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        anchors.topMargin: 2
        anchors.bottomMargin: 2
        radius: typeof ThemeManager !== "undefined" ? Math.min(ThemeManager.borderRadius, 6) : 6

        color: (mouseArea.containsMouse || isCurrentItem) ? 
            (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
            "transparent"

        Behavior on color {
            ColorAnimation { duration: 100 }
        }

        Row {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            Image {
                id: appIcon
                width: typeof ThemeManager !== "undefined" ? ThemeManager.iconSize : 24
                height: width
                anchors.verticalCenter: parent.verticalCenter
                visible: typeof ThemeManager !== "undefined" ? ThemeManager.showIcons : true
                source: model.iconName ? "image://icon/" + model.iconName : ""
                sourceSize: Qt.size(width, height)
                fillMode: Image.PreserveAspectFit
            }

            Text {
                id: appName
                text: (mouseArea.containsMouse || isCurrentItem) ? 
                    (model.name || "") : 
                    (model.highlightedName || model.name || "")
                textFormat: Text.StyledText
                color: (mouseArea.containsMouse || isCurrentItem) ? 
                    "#ffffff" : 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4")
                font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 14
                font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                font.bold: isCurrentItem
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - (appIcon.visible ? appIcon.width + parent.spacing : 0) - (actionBadge.visible ? actionBadge.width + parent.spacing + 20 : 0) - (commentText.visible ? commentText.width + parent.spacing : 0)
                elide: Text.ElideRight
            }

            Text {
                id: commentText
                text: model.genericName || model.comment || ""
                color: (mouseArea.containsMouse || isCurrentItem) ? 
                    Qt.alpha("#ffffff", 0.75) : 
                    Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7)
                font.pixelSize: (typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 14) - 2
                font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                anchors.verticalCenter: parent.verticalCenter
                visible: text.length > 0 && parent.width > 380
                elide: Text.ElideRight
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
