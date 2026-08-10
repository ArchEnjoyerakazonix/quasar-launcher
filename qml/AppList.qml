import QtQuick
import QtQuick.Controls
import com.quasar.launcher 1.0

Item {
    id: root
    width: parent ? parent.width : 600
    height: parent ? parent.height : 400

    signal launchApp(string exec, string id)
    signal requestSearchFocus()

    property string query: ""
    property bool isWindowMode: root.query.startsWith("w:") || root.query.startsWith("window:")
    property int extraSelectionIndex: -1 // -1: normal list, 0: Run command, 1: Search web

    ListView {
        id: list
        anchors.fill: parent
        clip: true

        model: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher : null
        delegate: AppListDelegate {
            onClicked: {
                var execCmd = model.exec || ""
                var categories = model.categories || ""
                if (categories === "Action" || (typeof ActionModel !== "undefined" && (execCmd.startsWith("gsettings") || execCmd.startsWith("quasar") || execCmd.startsWith("bash") || execCmd.startsWith("hyprctl") || execCmd.startsWith("cliphist")))) {
                    if (typeof ActionModel !== "undefined") {
                        ActionModel.execute(execCmd)
                    }
                    root.launchApp("__action__", "")
                } else {
                    root.launchApp(execCmd, model.desktopFile || model.id || "")
                }
            }
        }

        flickDeceleration: 1500
        boundsBehavior: Flickable.StopAtBounds

        reuseItems: true
        cacheBuffer: 300
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 100

        keyNavigationEnabled: true

        footer: Column {
            width: list.width
            spacing: 4
            visible: root.query.length > 0 && !root.isWindowMode

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
                        root.launchApp("__shell__:" + root.query, "")
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
                        root.launchApp("__web__:" + root.query, "")
                    }
                }
            }
        }

        Text {
            anchors.centerIn: parent
            text: "No results"
            color: Qt.rgba(255/255, 255/255, 255/255, 0.5)
            font.pixelSize: 18
            visible: list.count === 0 && root.query.length > 0
            opacity: visible ? 1 : 0
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }

        Connections {
            target: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher : null
            function onQueryChanged() {
                var q = root.query.trim().toLowerCase()
                if (q.startsWith("?") || q.startsWith("g:") || q.startsWith("web:")) {
                    root.extraSelectionIndex = 1
                } else if (q.startsWith("$")) {
                    root.extraSelectionIndex = 0
                } else {
                    root.extraSelectionIndex = -1
                    if (list.count > 0) {
                        list.currentIndex = 0
                    }
                }
            }
        }

        function handleReturn() {
            var q = root.query.trim()
            if (root.extraSelectionIndex === 1 || q.startsWith("?") || q.startsWith("g:") || q.startsWith("web:")) {
                root.launchApp("__web__:" + q, "")
            } else if (root.extraSelectionIndex === 0 || q.startsWith("$")) {
                root.launchApp("__shell__:" + q, "")
            } else if (root.isWindowMode && list.count > 0 && list.currentIndex >= 0) {
                var itemData = list.model[list.currentIndex]
                if (itemData && itemData.address) {
                    if (typeof WindowSwitcher !== "undefined") {
                        WindowSwitcher.focusWindow(itemData.address)
                    }
                    root.launchApp("__focus__", itemData.address)
                }
            } else if (list.count > 0 && list.currentIndex >= 0 && list.currentItem && typeof list.currentItem.activate === "function") {
                list.currentItem.activate()
            } else if (q.length > 0) {
                root.launchApp("__web__:" + q, "")
            }
        }

        Keys.onReturnPressed: {
            handleReturn()
        }

        Keys.onUpPressed: {
            if (root.extraSelectionIndex > 0) {
                root.extraSelectionIndex--
            } else if (root.extraSelectionIndex === 0) {
                root.extraSelectionIndex = -1
                if (list.count > 0) {
                    list.currentIndex = list.count - 1
                } else {
                    root.requestSearchFocus()
                }
            } else if (list.currentIndex > 0) {
                list.currentIndex--
            } else {
                root.requestSearchFocus()
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
