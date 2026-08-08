import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: parent ? parent.width : 600
    height: parent ? parent.height : 400

    signal launchApp(string exec, string id)

    function evalMath(expr) {
        if (!expr || !/^[0-9+\-*/().\s^]+$/.test(expr)) return null;
        try {
            var result = Function('"use strict"; return (' + expr + ')')();
            if (typeof result === "number" && !isNaN(result) && isFinite(result)) {
                return result;
            }
        } catch (e) {}
        return null;
    }

    property var query: typeof fuzzyMatcher !== "undefined" ? fuzzyMatcher.query : ""
    property var mathVal: evalMath(query)

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
                height: 36
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 6
                color: cmdArea.containsMouse ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    spacing: 10
                    Text { text: "💻"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Text { text: "Command"; color: Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7); font.pixelSize: 10 }
                        Text { text: root.query; color: cmdArea.containsMouse ? "#ffffff" : (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4"); font.pixelSize: 13; font.family: "Monospace" }
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

            // Math result (if math expression)
            Rectangle {
                width: parent.width - 8
                height: 36
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 6
                visible: root.mathVal !== null
                color: mathArea.containsMouse ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    spacing: 10
                    Text { text: "🧮"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Text { text: "Math result"; color: Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7); font.pixelSize: 10 }
                        Text { text: "= " + root.mathVal; color: mathArea.containsMouse ? "#ffffff" : (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4"); font.pixelSize: 13; font.bold: true }
                    }
                }

                MouseArea {
                    id: mathArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        root.launchApp("wl-copy " + root.mathVal, "")
                    }
                }
            }

            // Web Search runner
            Rectangle {
                width: parent.width - 8
                height: 36
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 6
                color: webArea.containsMouse ? 
                    (typeof ThemeManager !== "undefined" ? ThemeManager.accentColor : "#89b4fa") : 
                    "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    spacing: 10
                    Text { text: "🌐"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Text { text: "Web search"; color: Qt.alpha(typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8", 0.7); font.pixelSize: 10 }
                        Text { text: root.query; color: webArea.containsMouse ? "#ffffff" : (typeof ThemeManager !== "undefined" ? ThemeManager.textColor : "#cdd6f4"); font.pixelSize: 13 }
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
            text: "No applications found"
            color: Qt.alpha(
                typeof ThemeManager !== "undefined" ? ThemeManager.secondaryTextColor : "#a6adc8",
                0.5
            )
            font.pixelSize: typeof ThemeManager !== "undefined" ? ThemeManager.fontSize + 2 : 16
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
                var execStr = list.model.data(itemModel, 259) // ExecRole
                var desktopFileStr = list.model.data(itemModel, 262) // DesktopFileRole
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
