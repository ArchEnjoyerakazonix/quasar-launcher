import QtQuick
import QtQuick.Window
import com.quasar.launcher 1.0

Window {
    id: root
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    width: Screen.width
    height: Screen.height

    function onOpened() {
        searchBar.forceActiveFocus()
        if (typeof fuzzyMatcher !== "undefined") {
            fuzzyMatcher.query = ""
            searchBar.text = ""
        }
    }

    // Press Escape to hide the window
    Shortcut {
        sequence: "Escape"
        onActivated: root.hide()
    }

    // Dark backdrop overlay that closes launcher on click
    GlassBackground {
        anchors.fill: parent
        onBackgroundClicked: root.hide()
    }

    // Main Rofi Window Container (Centered compact box)
    Rectangle {
        id: container
        width: typeof ThemeManager !== "undefined" ? ThemeManager.windowWidth : 640
        height: typeof ThemeManager !== "undefined" ? ThemeManager.windowHeight : 420
        anchors.centerIn: parent
        radius: typeof ThemeManager !== "undefined" ? ThemeManager.borderRadius : 8

        color: Qt.alpha(
            typeof ThemeManager !== "undefined" ? ThemeManager.backgroundColor : "#1e1e2e",
            typeof ThemeManager !== "undefined" ? ThemeManager.bgOpacity : 0.95
        )
        border.color: typeof ThemeManager !== "undefined" ? ThemeManager.borderColor : "#313244"
        border.width: typeof ThemeManager !== "undefined" ? ThemeManager.borderWidth : 1

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            SearchBar {
                id: searchBar
                width: parent.width

                onTextChanged: {
                    if (typeof fuzzyMatcher !== "undefined") {
                        fuzzyMatcher.query = text
                    }
                }
                
                Keys.onDownPressed: {
                    if (appList.visible) appList.forceActiveFocus()
                    else if (appGrid.visible) appGrid.forceActiveFocus()
                }
            }

            // Rofi Vertical List View
            AppList {
                id: appList
                width: parent.width
                height: parent.height - searchBar.height - parent.spacing
                visible: typeof ThemeManager === "undefined" || ThemeManager.layoutMode === "list"

                onLaunchApp: function(appExec, appId) {
                    if (typeof appIndexer !== "undefined") {
                        appIndexer.launch(appExec)
                    }
                    if (typeof frecencyRanker !== "undefined") {
                        frecencyRanker.recordLaunch(appId)
                    }
                    root.hide()
                }
            }

            // Grid View
            AppGrid {
                id: appGrid
                width: parent.width
                height: parent.height - searchBar.height - parent.spacing
                visible: typeof ThemeManager !== "undefined" && ThemeManager.layoutMode === "grid"

                onLaunchApp: function(appExec, appId) {
                    if (typeof appIndexer !== "undefined") {
                        appIndexer.launch(appExec)
                    }
                    if (typeof frecencyRanker !== "undefined") {
                        frecencyRanker.recordLaunch(appId)
                    }
                    root.hide()
                }
            }
        }
    }
}
