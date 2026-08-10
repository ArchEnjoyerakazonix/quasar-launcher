import QtQuick
import QtQuick.Controls
import com.quasar.launcher 1.0

Item {
    id: root
    width: parent ? parent.width : 600
    height: parent ? parent.height : 400

    signal launchApp(string exec, string id)

    property int extraSelectionIndex: -1 // -1: normal list, 0: Run command, 1: Search web

    ListView {
        id: list
        anchors.fill: parent
        clip: true

        model: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher : null
        delegate: AppListDelegate {
            onClicked: {
                root.launchApp(model.exec || "", model.desktopFile || model.id || "")
            }
        }

        flickDeceleration: 1500
        boundsBehavior: Flickable.StopAtBounds

        highlightFollowsCurrentItem: true
        highlightMoveDuration: 100

        keyNavigationEnabled: true

        footer: Column {
            width: list.width
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
                        text: root.query
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
                        root.launchApp("xdg-open 'https://www.google.com/search?q=" + encodeURIComponent(root.query) + "'", "")
                    }
                }
            }
        }

        Text {
            anchors.centerIn: parent
            text: "No matching applications"
            color: Qt.alpha(
                typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8",
                0.5
            )
            font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize : 14
            font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
            visible: list.count === 0 && root.query.length > 0
        }

        Connections {
            target: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher : null
            function onQueryChanged() {
                root.extraSelectionIndex = -1
                if (list.count > 0) {
                    list.currentIndex = 0
                }
            }
        }

        Keys.onReturnPressed: {
            if (root.extraSelectionIndex === 1) {
                root.launchApp("xdg-open 'https://www.google.com/search?q=" + encodeURIComponent(root.query) + "'", "")
            } else if (root.extraSelectionIndex === 0) {
                root.launchApp(root.query, "")
            } else if (list.currentIndex >= 0 && list.currentIndex < list.count) {
                var itemModel = list.model.index(list.currentIndex, 0)
                var roles = list.model.roleNames ? list.model.roleNames() : {}
                var execRole = 260
                var desktopRole = 263
                for (var r in roles) {
                    if (roles[r] === "exec") execRole = parseInt(r)
                    if (roles[r] === "desktopFile") desktopRole = parseInt(r)
                }
                var execStr = list.model.data(itemModel, execRole)
                var desktopFileStr = list.model.data(itemModel, desktopRole)
                root.launchApp(execStr || "", desktopFileStr || "")
            } else if (root.query.length > 0) {
                root.launchApp(root.query, "")
            }
        }

        Keys.onUpPressed: {
            if (root.extraSelectionIndex > 0) {
                root.extraSelectionIndex--
            } else if (root.extraSelectionIndex === 0) {
                root.extraSelectionIndex = -1
                if (list.count > 0) {
                    list.currentIndex = list.count - 1
                } else {
                    searchBar.forceActiveFocus()
                }
            } else if (list.currentIndex > 0) {
                list.currentIndex--
            } else {
                searchBar.forceActiveFocus()
            }
        }

        Keys.onDownPressed: {
            if (list.currentIndex < list.count - 1) {
                list.currentIndex++
            } else if (root.query.length > 0) {
                if (root.extraSelectionIndex < 1) {
                    root.extraSelectionIndex++
                }
            }
        }
    }

    function forceActiveFocus() {
        list.forceActiveFocus()
        root.extraSelectionIndex = -1
        if (list.count > 0 && list.currentIndex < 0) {
            list.currentIndex = 0
        }
    }
}
