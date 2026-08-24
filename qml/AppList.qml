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
    property bool isSpecialMode: {
        var q = root.query.trim().toLowerCase()
        return q.startsWith("w:") || q.startsWith("w.") || q.startsWith("window:") || q.startsWith("w ") ||
               q.startsWith("e:") || q.startsWith("e.") || q.startsWith("emoji:") || q.startsWith("e ") || q.startsWith(":") ||
               q.startsWith("c:") || q.startsWith("c.") || q.startsWith("clip:") || q.startsWith("cb:") || q.startsWith("c ") ||
               q.startsWith("/")
    }
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
                // Pipe menu item: round-trip the line back to the script.
                // The launcher stays open — the script either prints a
                // follow-up menu or triggers closing via pipeActionDone.
                if (execCmd.startsWith("__pipe__:")) {
                    if (typeof ActionModel !== "undefined") {
                        ActionModel.selectPipeItem(execCmd.substring(8))
                    }
                    return
                }
                if (categories === "Action" || (typeof ActionModel !== "undefined" && (execCmd.startsWith("gsettings") || execCmd.startsWith("quasar") || execCmd.startsWith("bash") || execCmd.startsWith("hyprctl") || execCmd.startsWith("cliphist")))) {
                    if (typeof ActionModel !== "undefined") {
                        ActionModel.execute(execCmd)
                    }
                    // The theme selector keeps the launcher open as the
                    // live-preview canvas.
                    root.launchApp(execCmd.indexOf("quasar-theme-selector") !== -1
                                   ? "__preview__" : "__action__", "")
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
            visible: root.query.length > 0 && !root.isSpecialMode

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

        // Pipe script consumed the selection and printed nothing — close.
        Connections {
            target: typeof ActionModel !== "undefined" ? ActionModel : null
            function onPipeActionDone() {
                root.launchApp("__action__", "")
            }
            function onPipeMenuUpdated() {
                if (list.count > 0) {
                    list.currentIndex = 0
                }
            }
        }

        Keys.onReturnPressed: {
            root.handleReturn()
        }

        Keys.onUpPressed: {
            root.moveUp()
        }

        Keys.onDownPressed: {
            root.moveDown()
        }

        Keys.onLeftPressed: {
            root.moveLeft()
        }

        Keys.onRightPressed: {
            root.moveRight()
        }
    }

    function moveUp() {
        if (list.count === 0) return
        if (root.extraSelectionIndex > 0) {
            root.extraSelectionIndex--
            return
        } else if (root.extraSelectionIndex === 0) {
            root.extraSelectionIndex = -1
            if (list.count > 0) {
                list.currentIndex = list.count - 1
                list.positionViewAtIndex(list.currentIndex, ListView.Contain)
            }
            return
        }

        if (list.currentIndex > 0) {
            list.currentIndex--
            list.positionViewAtIndex(list.currentIndex, ListView.Contain)
        } else {
            root.requestSearchFocus()
        }
    }

    function moveDown() {
        if (list.count === 0) {
            if (root.query.length > 0 && !root.isWindowMode && root.extraSelectionIndex < 1) {
                root.extraSelectionIndex++
            }
            return
        }

        if (list.currentIndex < list.count - 1) {
            list.currentIndex++
            list.positionViewAtIndex(list.currentIndex, ListView.Contain)
        } else if (root.query.length > 0 && !root.isWindowMode) {
            if (root.extraSelectionIndex < 1) {
                root.extraSelectionIndex++
            }
        }
    }

    function moveLeft() {
        moveUp()
    }

    function moveRight() {
        moveDown()
    }

    function handleReturn() {
        var q = root.query.trim()
        var qLower = q.toLowerCase()
        if (root.extraSelectionIndex === 1 || q.startsWith("?") || qLower.startsWith("g:") || qLower.startsWith("web:") || qLower.startsWith("b:") || qLower.startsWith("browser:") || qLower.startsWith("google:") || qLower.startsWith("chrome:") || qLower.startsWith("search:") || q.startsWith("http://") || q.startsWith("https://") || q.startsWith("www.")) {
            root.launchApp("__web__:" + q, "")
        } else if (root.extraSelectionIndex === 0 || q.startsWith("$") || q.startsWith(">")) {
            root.launchApp("__shell__:" + q, "")
        } else if (list.count > 0 && list.currentIndex >= 0) {
            if (list.currentItem && typeof list.currentItem.activate === "function") {
                list.currentItem.activate()
            } else if (list.currentItem && typeof list.currentItem.clicked === "function") {
                list.currentItem.clicked()
            } else if (list.model) {
                var item = list.model.get ? list.model.get(list.currentIndex) : (list.model[list.currentIndex] || null)
                if (item) {
                    var execCmd = item.exec || ""
                    root.launchApp(execCmd, item.desktopFile || item.id || "")
                }
            }
        } else if (q.length > 0) {
            if (q.startsWith("/")) {
                root.launchApp(q, "")
            } else {
                root.launchApp("__web__:" + q, "")
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

