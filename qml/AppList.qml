import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: parent ? parent.width : 600
    height: parent ? parent.height : 400

    signal launchApp(string exec, string id)

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

        Text {
            anchors.centerIn: parent
            text: "No applications found"
            color: Qt.alpha(
                typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8",
                0.5
            )
            font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize + 2 : 16
            font.family: typeof ThemeManager !== "undefined" ? ThemeManager.fontFamily : "Sans"
            visible: list.count === 0 && (typeof fuzzyMatcher !== "undefined" && fuzzyMatcher.query.length > 0)
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
                var execStr = list.model.data(itemModel, 259) // ExecRole
                var desktopFileStr = list.model.data(itemModel, 262) // DesktopFileRole
                root.launchApp(execStr || "", desktopFileStr || "")
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
