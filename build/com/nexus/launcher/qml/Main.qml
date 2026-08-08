import QtQuick
import QtQuick.Window

Window {
    id: root
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint

    // Full-screen overlay (can be adjusted for LayerShell)
    width: Screen.width
    height: Screen.height

    function toggleVisibility() {
        if (visible) {
            hide()
        } else {
            showFullScreen()
            contentItemOpacity = 1.0
            searchBar.forceActiveFocus()
            
            // Reset search query
            if (typeof fuzzyMatcher !== "undefined") {
                fuzzyMatcher.query = ""
                searchBar.text = ""
            }
        }
    }

    property real contentItemOpacity: 0.0

    Component.onCompleted: {
        // Initial setup if needed
    }

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
            NumberAnimation { duration: 200; easing.type: Easing.OutQuint }
        }

        SearchBar {
            id: searchBar
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.horizontalCenter: parent.horizontalCenter

            onTextChanged: {
                if (typeof fuzzyMatcher !== "undefined") {
                    fuzzyMatcher.query = text
                }
            }
            
            Keys.onDownPressed: {
                appGrid.forceActiveFocus()
            }
        }

        AppGrid {
            id: appGrid
            anchors.top: searchBar.bottom
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter

            // Max height logic
            height: Math.min(parent.height - searchBar.height - 120, contentItem.childrenRect.height)
            
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
