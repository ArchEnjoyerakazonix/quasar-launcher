import QtQuick
import QtQuick.Window

Window {
    id: root
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint

    width: Screen.width
    height: Screen.height

    function toggleVisibility() {
        if (visible) {
            hide()
        } else {
            showFullScreen()
            contentItemOpacity = 1.0
            searchBar.forceActiveFocus()
            
            if (typeof fuzzyMatcher !== "undefined") {
                fuzzyMatcher.query = ""
                searchBar.text = ""
            }
        }
    }

    property real contentItemOpacity: 0.0

    // Press Escape to hide the window
    Shortcut {
        sequence: "Escape"
        onActivated: root.hide()
    }

    GlassBackground {
        anchors.fill: parent
        onBackgroundClicked: root.hide()
    }

    Item {
        id: mainContent
        anchors.fill: parent
        opacity: root.contentItemOpacity
        
        Behavior on opacity {
            NumberAnimation { duration: 150; easing.type: Easing.OutQuint }
        }

        // Launcher Window Box (Rofi Box)
        Rectangle {
            id: launcherBox
            width: typeof ThemeManager !== "undefined" ? ThemeManager.windowWidth : 640
            height: typeof ThemeManager !== "undefined" ? ThemeManager.windowHeight : 420
            anchors.centerIn: parent
            radius: typeof ThemeManager !== "undefined" ? ThemeManager.borderRadius : 12

            color: Qt.alpha(
                typeof ThemeManager !== "undefined" ? ThemeManager.backgroundColor : "#11111b",
                typeof ThemeManager !== "undefined" ? ThemeManager.bgOpacity : 0.90
            )
            border.color: typeof ThemeManager !== "undefined" ? ThemeManager.borderColor : "#313244"
            border.width: typeof ThemeManager !== "undefined" ? ThemeManager.borderWidth : 1

            Column {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

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

                // Vertical Rofi-style List View
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
                        root.contentItemOpacity = 0.0
                        Qt.callLater(function() { root.hide() })
                    }
                }

                // Spotlight Grid View
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
                        root.contentItemOpacity = 0.0
                        Qt.callLater(function() { root.hide() })
                    }
                }
            }
        }
    }
}
