import QtQuick
import QtQuick.Controls
import com.quasar.launcher 1.0

Item {
    id: root
    width: Math.min(parent.width - 80, 720)

    signal launchApp(string exec, string id)
    signal requestSearchFocus()

    property string query: ""

    property int extraSelectionIndex: -1 // -1: normal grid, 0: Run command, 1: Search web

    GridView {
        id: grid
        anchors.fill: parent
        cellWidth: Math.floor(width / 5)
        cellHeight: 130

        model: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher : null
        delegate: AppDelegate {
            onClicked: {
                root.launchApp(model.exec || "", model.desktopFile || model.id || "")
            }
            function activate() {
                root.launchApp(model.exec || "", model.desktopFile || model.id || "")
            }
        }

        flickDeceleration: 1500
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        reuseItems: true
        cacheBuffer: 300
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 200
        
        // Ensure keyboard navigation works out of the box
        keyNavigationEnabled: true

        footer: Column {
            width: grid.width
            spacing: 4
            visible: root.query.length > 0

            // Command runner
            Rectangle {
                width: parent.width - 8
                height: 34
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 4
                color: (cmdArea.containsMouse || root.extraSelectionIndex === 0) ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12

                    Text { 
                        text: "Run command"
                        color: (cmdArea.containsMouse || root.extraSelectionIndex === 0) ? "#ffffff" : Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7)
                        font.pixelSize: 11
                        font.bold: (cmdArea.containsMouse || root.extraSelectionIndex === 0)
                        font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                        anchors.verticalCenter: parent.verticalCenter
                        width: 80
                    }

                    Text { 
                        text: root.query
                        color: (cmdArea.containsMouse || root.extraSelectionIndex === 0) ? "#ffffff" : (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4")
                        font.pixelSize: 13
                        font.family: "Monospace"
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                    }
                }

                MouseArea {
                    id: cmdArea
                    anchors.fill: parent
                    hoverEnabled: true
                    preventStealing: true
                    onClicked: {
                        root.launchApp(root.query, "")
                    }
                }
            }

            // Web Search runner
            Rectangle {
                width: parent.width - 8
                height: 34
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 4
                color: (webArea.containsMouse || root.extraSelectionIndex === 1) ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12

                    Text { 
                        text: "Search web"
                        color: (webArea.containsMouse || root.extraSelectionIndex === 1) ? "#ffffff" : Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7)
                        font.pixelSize: 11
                        font.bold: (webArea.containsMouse || root.extraSelectionIndex === 1)
                        font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                        anchors.verticalCenter: parent.verticalCenter
                        width: 80
                    }

                    Text { 
                        text: "Google: " + root.query
                        color: (webArea.containsMouse || root.extraSelectionIndex === 1) ? "#ffffff" : (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4")
                        font.pixelSize: 13
                        font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                    }
                }

                MouseArea {
                    id: webArea
                    anchors.fill: parent
                    hoverEnabled: true
                    preventStealing: true
                    onClicked: {
                        root.launchApp("__web__", "")
                    }
                }
            }
        }

        Text {
            anchors.centerIn: parent
            text: "No results"
            color: Qt.rgba(255/255, 255/255, 255/255, 0.5)
            font.pixelSize: 18
            visible: grid.count === 0 && (typeof fuzzyMatcher !== "undefined" && fuzzyMatcher.query.length > 0)
            opacity: visible ? 1 : 0
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }

        Connections {
            target: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher : null
            function onQueryChanged() {
                root.extraSelectionIndex = -1
                if (grid.count > 0) {
                    grid.currentIndex = 0
                }
            }
        }

        Keys.onReturnPressed: {
            if (root.extraSelectionIndex === 1) {
                root.launchApp("__web__", "")
            } else if (root.extraSelectionIndex === 0) {
                root.launchApp(root.query, "")
            } else if (grid.currentItem && typeof grid.currentItem.activate === "function") {
                grid.currentItem.activate()
            } else if (grid.currentItem) {
                grid.currentItem.clicked()
            } else if (root.query.length > 0) {
                root.launchApp(root.query, "")
            }
        }
        
        Keys.onUpPressed: {
            if (root.extraSelectionIndex > 0) {
                root.extraSelectionIndex--
            } else if (root.extraSelectionIndex === 0) {
                root.extraSelectionIndex = -1
                if (grid.count > 0) {
                    grid.currentIndex = grid.count - 1
                } else {
                    root.requestSearchFocus()
                }
            } else if (grid.currentIndex >= Math.floor(grid.width / grid.cellWidth)) {
                grid.moveCurrentIndexUp()
            } else {
                root.requestSearchFocus()
            }
        }

        Keys.onDownPressed: {
            var cols = Math.max(1, Math.floor(grid.width / grid.cellWidth))
            if (grid.currentIndex + cols < grid.count) {
                grid.moveCurrentIndexDown()
            } else if (grid.currentIndex < grid.count - 1) {
                grid.currentIndex = grid.count - 1
            } else if (root.query.length > 0) {
                if (root.extraSelectionIndex < 1) {
                    root.extraSelectionIndex++
                }
            }
        }
    }

    function forceActiveFocus() {
        grid.forceActiveFocus()
        root.extraSelectionIndex = -1
        if (grid.count > 0 && grid.currentIndex < 0) {
            grid.currentIndex = 0
        }
    }
}
