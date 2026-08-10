import QtQuick
import QtQuick.Controls
import com.quasar.launcher 1.0

Item {
    id: root
    width: Math.min(parent.width - 80, 720)

    signal launchApp(string exec, string id)
    signal requestSearchFocus()

    property string query: ""

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
                if (grid.count > 0) {
                    grid.currentIndex = 0
                }
            }
        }

        Keys.onReturnPressed: {
            if (grid.currentItem) {
                grid.currentItem.clicked()
            }
        }
        
        Keys.onUpPressed: {
            if (grid.currentIndex >= Math.floor(grid.width / grid.cellWidth)) {
                grid.moveCurrentIndexUp()
            } else {
                root.requestSearchFocus()
            }
        }
    }

    function forceActiveFocus() {
        grid.forceActiveFocus()
    }
}
