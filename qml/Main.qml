import QtQuick
import QtQuick.Window
import com.quasar.launcher 1.0

Window {
    id: root
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    width: ThemeManager.windowWidth
    height: ThemeManager.windowHeight

    property bool previewMode: false

    onActiveChanged: {
        if (!active && visible && !previewMode) {
            root.hide()
        }
    }

    function onOpened() {
        previewMode = false
        searchBar.forceActiveFocus()
        fuzzyMatcher.query = ""
        searchBar.text = ""
        if (typeof windowSwitcher !== "undefined") {
            windowSwitcher.getOpenWindows()
        }
    }

    function hideAfterLaunch(appExec) {
        // Launching the theme selector keeps the launcher visible as the
        // live-preview canvas; focus loss to the selector overlay must not
        // close it.
        if (appExec === "__preview__") {
            previewMode = true
            return
        }
        root.hide()
    }

    // Press Escape to hide the window
    Shortcut {
        sequence: "Escape"
        onActivated: root.hide()
    }

    // Main Rofi Window Container (Compact floating box)
    Rectangle {
        id: container
        anchors.fill: parent
        radius: ThemeManager.borderRadius

        color: Qt.alpha(ThemeManager.backgroundColor, ThemeManager.bgOpacity)
        border.color: ThemeManager.borderColor
        border.width: ThemeManager.borderWidth

        // Subtle appear animation: the panel slides/fades in on every open
        opacity: root.visible ? 1 : 0
        scale: root.visible ? 1 : 0.96

        Behavior on opacity {
            NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            SearchBar {
                id: searchBar
                width: parent.width

                onTextChanged: {
                    fuzzyMatcher.query = text
                }

                onDownPressed: {
                    if (appList.visible) appList.moveDown()
                    else if (appGrid.visible) appGrid.moveDown()
                }

                onUpPressed: {
                    if (appList.visible) appList.moveUp()
                    else if (appGrid.visible) appGrid.moveUp()
                }

                onLeftPressed: {
                    if (appList.visible) appList.moveLeft()
                    else if (appGrid.visible) appGrid.moveLeft()
                }

                onRightPressed: {
                    if (appList.visible) appList.moveRight()
                    else if (appGrid.visible) appGrid.moveRight()
                }

                onReturnPressed: {
                    if (appList.visible) appList.handleReturn()
                    else if (appGrid.visible) appGrid.handleReturn()
                }
            }

            // Rofi Vertical List View
            AppList {
                id: appList
                width: parent.width
                height: parent.height - searchBar.height - parent.spacing
                query: searchBar.text
                visible: ThemeManager.layoutMode === "list" || ThemeManager.layoutMode === "compact"

                onRequestSearchFocus: {
                    searchBar.forceActiveFocus()
                }

                onLaunchApp: function(appExec, appId) {
                    hideAfterLaunch(appExec)
                    appIndexer.launch(appExec)
                    frecencyRanker.recordLaunch(appId)
                }
            }

            // Grid View
            AppGrid {
                id: appGrid
                width: parent.width
                height: parent.height - searchBar.height - parent.spacing
                query: searchBar.text
                visible: ThemeManager.layoutMode === "grid"

                onRequestSearchFocus: {
                    searchBar.forceActiveFocus()
                }

                onLaunchApp: function(appExec, appId) {
                    hideAfterLaunch(appExec)
                    appIndexer.launch(appExec)
                    frecencyRanker.recordLaunch(appId)
                }
            }
        }
    }
}
