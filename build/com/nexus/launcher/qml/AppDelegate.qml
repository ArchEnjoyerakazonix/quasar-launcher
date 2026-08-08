import QtQuick

Item {
    id: root
    width: 110
    height: 130

    signal clicked()

    property bool isCurrentItem: GridView.isCurrentItem

    // Subtle opacity animation on appear
    opacity: 0
    Component.onCompleted: {
        appearAnim.start()
    }

    SequentialAnimation {
        id: appearAnim
        PauseAnimation { duration: index * 20 }
        NumberAnimation { target: root; property: "opacity"; to: 1; duration: 200; easing.type: Easing.OutQuint }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        anchors.margins: 5
        radius: 12
        color: (mouseArea.containsMouse || isCurrentItem) ? Qt.rgba(255/255, 255/255, 255/255, 0.1) : "transparent"

        scale: mouseArea.pressed ? 0.95 : ((mouseArea.containsMouse || isCurrentItem) ? 1.08 : 1.0)

        Behavior on scale {
            SpringAnimation { spring: 3; damping: 0.2; epsilon: 0.01 }
        }
        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        Column {
            anchors.centerIn: parent
            spacing: 8

            Image {
                id: appIcon
                width: 64
                height: 64
                anchors.horizontalCenter: parent.horizontalCenter
                
                // Usually an image provider is registered in C++ (e.g. "image://iconTheme/")
                // or Qt 6's IconImage could be used. Here we use Image with standard prefix assumption.
                source: model.iconName ? "image://icon/" + model.iconName : ""
                sourceSize: Qt.size(64, 64)
                fillMode: Image.PreserveAspectFit
            }

            Text {
                text: model.name || "App"
                color: "white"
                font.pixelSize: 14
                width: 100
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
