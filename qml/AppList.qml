import QtQuick
import QtQuick.Controls
import com.quasar.launcher 1.0

Item {
    id: root
    width: parent ? parent.width : 600
    height: parent ? parent.height : 400

    signal launchApp(string exec, string id)

    property var query: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher.query : ""

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
            spacing: 2
            visible: root.query.length > 0

            // Command runner
            Rectangle {
                width: parent.width - 8
                height: 34
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 4
                color: cmdArea.containsMouse ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12

                    Text { 
                        text: "Run command"
                        color: cmdArea.containsMouse ? Qt.alpha("#ffffff", 0.8) : Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7)
                        font.pixelSize: 11
                        font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                        anchors.verticalCenter: parent.verticalCenter
                        width: 80
                    }

                    Text { 
                        text: root.query
                        color: cmdArea.containsMouse ? "#ffffff" : (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4")
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
                color: webArea.containsMouse ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12

                    Text { 
                        text: "Search web"
                        color: webArea.containsMouse ? Qt.alpha("#ffffff", 0.8) : Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7)
                        font.pixelSize: 11
                        font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
                        anchors.verticalCenter: parent.verticalCenter
                        width: 80
                    }

                    Text { 
                        text: root.query
                        color: webArea.containsMouse ? "#ffffff" : (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4")
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
                if (list.count > 0) {
                    list.currentIndex = 0
                }
            }
        }

        Keys.onReturnPressed: {
            if (list.currentIndex >= 0 && list.currentIndex < list.count) {
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
            if (list.currentIndex > 0) {
                list.currentIndex--
            } else {
                searchBar.forceActiveFocus()
            }
        }

        Keys.onDownPressed: {
            if (list.currentIndex < list.count - 1) {
                list.currentIndex++
            }
        }
    }

    function forceActiveFocus() {
        list.forceActiveFocus()
        if (list.count > 0 && list.currentIndex < 0) {
            list.currentIndex = 0
        }
    }
}
