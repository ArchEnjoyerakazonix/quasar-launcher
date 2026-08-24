import QtQuick
import QtQuick.Controls
import com.quasar.launcher 1.0

Item {
    id: root
    width: Math.min(parent.width - 80, 720)

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

    property int extraSelectionIndex: -1 // -1: normal grid, 0: Run command, 1: Search web

    GridView {
        id: grid
        anchors.fill: parent
        cellWidth: Math.floor(width / 5)
        cellHeight: 130

        model: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher : null
        delegate: AppDelegate {
            function runItem() {
                var execCmd = model.exec || ""
                if (execCmd.startsWith("__pipe__:")) {
                    if (typeof ActionModel !== "undefined") {
                        ActionModel.selectPipeItem(execCmd.substring(8))
                    }
                    return
                }
                root.launchApp(execCmd, model.desktopFile || model.id || "")
            }
            onClicked: runItem()
            function activate() {
                runItem()
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

        // Pipe script consumed the selection and printed nothing — close.
        Connections {
            target: typeof ActionModel !== "undefined" ? ActionModel : null
            function onPipeActionDone() {
                root.launchApp("__action__", "")
            }
            function onPipeMenuUpdated() {
                if (grid.count > 0) {
                    grid.currentIndex = 0
                }
            }
        }

        footer: Column {
            width: grid.width
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
            root.handleReturn()
        }

        Keys.onLeftPressed: {
            root.moveLeft()
        }

        Keys.onRightPressed: {
            root.moveRight()
        }
        
        Keys.onUpPressed: {
            root.moveUp()
        }

        Keys.onDownPressed: {
            root.moveDown()
        }
    }

    function moveLeft() {
        if (grid.count === 0) return
        if (root.extraSelectionIndex !== -1) {
            root.extraSelectionIndex = -1
        }
        if (grid.currentIndex > 0) {
            grid.currentIndex--
        } else {
            grid.currentIndex = grid.count - 1
        }
        grid.positionViewAtIndex(grid.currentIndex, GridView.Contain)
    }

    function moveRight() {
        if (grid.count === 0) return
        if (root.extraSelectionIndex !== -1) {
            root.extraSelectionIndex = -1
        }
        if (grid.currentIndex < grid.count - 1) {
            grid.currentIndex++
        } else {
            grid.currentIndex = 0
        }
        grid.positionViewAtIndex(grid.currentIndex, GridView.Contain)
    }

    function moveUp() {
        if (grid.count === 0) return
        if (root.extraSelectionIndex > 0) {
            root.extraSelectionIndex--
            return
        } else if (root.extraSelectionIndex === 0) {
            root.extraSelectionIndex = -1
            if (grid.count > 0) {
                grid.currentIndex = grid.count - 1
                grid.positionViewAtIndex(grid.currentIndex, GridView.Contain)
            }
            return
        }

        var cols = Math.max(1, Math.floor(grid.width / grid.cellWidth))
        if (grid.currentIndex >= cols) {
            grid.currentIndex -= cols
            grid.positionViewAtIndex(grid.currentIndex, GridView.Contain)
        } else {
            root.requestSearchFocus()
        }
    }

    function moveDown() {
        if (grid.count === 0) {
            if (root.query.length > 0 && root.extraSelectionIndex < 1) {
                root.extraSelectionIndex++
            }
            return
        }

        var cols = Math.max(1, Math.floor(grid.width / grid.cellWidth))
        if (grid.currentIndex + cols < grid.count) {
            grid.currentIndex += cols
            grid.positionViewAtIndex(grid.currentIndex, GridView.Contain)
        } else if (grid.currentIndex < grid.count - 1) {
            grid.currentIndex = grid.count - 1
            grid.positionViewAtIndex(grid.currentIndex, GridView.Contain)
        } else if (root.query.length > 0) {
            if (root.extraSelectionIndex < 1) {
                root.extraSelectionIndex++
            }
        }
    }

    function handleReturn() {
        var q = root.query.trim()
        var qLower = q.toLowerCase()
        if (root.extraSelectionIndex === 1 || q.startsWith("?") || qLower.startsWith("g:") || qLower.startsWith("web:") || qLower.startsWith("b:") || qLower.startsWith("browser:") || qLower.startsWith("google:") || qLower.startsWith("chrome:") || qLower.startsWith("search:") || q.startsWith("http://") || q.startsWith("https://") || q.startsWith("www.")) {
            root.launchApp("__web__:" + q, "")
        } else if (root.extraSelectionIndex === 0 || q.startsWith("$") || q.startsWith(">")) {
            root.launchApp("__shell__:" + q, "")
        } else if (grid.count > 0 && grid.currentIndex >= 0) {
            if (grid.currentItem && typeof grid.currentItem.activate === "function") {
                grid.currentItem.activate()
            } else if (grid.currentItem && typeof grid.currentItem.clicked === "function") {
                grid.currentItem.clicked()
            } else if (grid.model) {
                var item = grid.model.get ? grid.model.get(grid.currentIndex) : (grid.model[grid.currentIndex] || null)
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
        grid.forceActiveFocus()
        root.extraSelectionIndex = -1
        if (grid.count > 0 && grid.currentIndex < 0) {
            grid.currentIndex = 0
        }
    }
}

