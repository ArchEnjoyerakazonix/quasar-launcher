import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quasar 1.0

// Full-screen transparent overlay.
// Left side: embedded live Quasar preview.
// Right side: theme selector panel.
Window {
    id: window
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    property string searchFilter: ""
    property string categoryFilter: "All"
    property string activePresetName: ""
    property bool initialized: false

    Shortcut {
        sequence: "Escape"
        onActivated: window.close()
    }

    // Dark backdrop — click to close.
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        MouseArea {
            anchors.fill: parent
            onClicked: window.close()
        }
    }

    // ====================================================
    // RIGHT SIDE: Theme Selector Panel (fixed 650px wide)
    // ====================================================
    Rectangle {
        id: selectorPanel
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        width: 650
        color: "#181825"
        border.color: "#89b4fa"
        border.width: 1
        radius: 8

        // Absorb clicks so they don't close the window.
        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10

            // ==========================================
            // 1. HEADER BAR: search + counters + close
            // ==========================================
            Rectangle {
                Layout.fillWidth: true
                height: 40
                color: "#1e1e2e"
                radius: 4
                border.color: "#313244"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 10
                    spacing: 8

                    Text {
                        text: "Theme:"
                        color: "#89b4fa"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    TextField {
                        id: searchInput
                        placeholderText: "Type to filter..."
                        placeholderTextColor: "#6c7086"
                        color: "#cdd6f4"
                        font.pixelSize: 14
                        background: Item {}
                        Layout.fillWidth: true
                        focus: true

                        onTextChanged: {
                            window.searchFilter = text.toLowerCase()
                            rebuildModel()
                        }
                        Keys.onDownPressed: themeList.forceActiveFocus()
                    }

                    Text {
                        text: presetsModel.count + "/" + presetsModel.totalCount
                        color: "#6c7086"
                        font.pixelSize: 12
                    }

                    // Red close button
                    Rectangle {
                        width: 22; height: 22; radius: 11
                        color: closeMouse.containsMouse ? "#f38ba8" : "#313244"
                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            color: closeMouse.containsMouse ? "#11111b" : "#cdd6f4"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        MouseArea {
                            id: closeMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: window.close()
                        }
                    }
                }
            }

            // ==========================================
            // 2. INSTRUCTION BANNER
            // ==========================================
            Rectangle {
                Layout.fillWidth: true
                height: 46
                color: "#1e1e2e"
                radius: 4
                border.color: "#313244"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 2

                    Text {
                        text: "Hover or use arrow keys to <b>live-preview</b>. Click or <b>Enter</b> to apply. <b>F</b> — favorite ★"
                        color: "#cdd6f4"
                        font.pixelSize: 11
                        textFormat: Text.RichText
                    }
                    Text {
                        text: "Press <b>Escape</b> or click outside to close. Current theme: <font color='#89b4fa'><b>" +
                              (window.activePresetName === "" ? "custom" : window.activePresetName) + "</b></font>"
                        color: "#a6adc8"
                        font.pixelSize: 11
                        textFormat: Text.RichText
                    }
                }
            }

            // ==========================================
            // 3. CATEGORY + MODE PILLS
            // ==========================================
            Rectangle {
                Layout.fillWidth: true
                height: 34
                color: "#1e1e2e"
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 4

                    Text {
                        text: "  Cat:"
                        color: "#a6adc8"
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Repeater {
                        model: ["All", "Dark", "Light", "Neon", "Retro"]
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 4
                            readonly property bool isActive: window.categoryFilter === modelData
                            color: isActive ? "#89b4fa" : (catMA.containsMouse ? "#313244" : "transparent")
                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                color: parent.isActive ? "#11111b" : "#cdd6f4"
                                font.pixelSize: 11
                                font.bold: parent.isActive
                            }
                            MouseArea {
                                id: catMA
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    window.categoryFilter = modelData
                                    rebuildModel()
                                }
                            }
                        }
                    }

                    Item { Layout.preferredWidth: 8 }
                    Text {
                        text: "Mode:"
                        color: "#a6adc8"
                        font.pixelSize: 11
                        font.bold: true
                    }
                    Repeater {
                        model: [
                            { key: "list", label: "📄 List" },
                            { key: "grid", label: "🔲 Grid" },
                            { key: "compact", label: "📑 Compact" }
                        ]
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 4
                            readonly property bool isActive: ThemeManager.layoutMode === modelData.key
                            color: isActive ? "#89b4fa" : (modeMA.containsMouse ? "#313244" : "transparent")
                            Text {
                                anchors.centerIn: parent
                                text: modelData.label
                                color: parent.isActive ? "#11111b" : "#cdd6f4"
                                font.pixelSize: 11
                                font.bold: parent.isActive
                            }
                            MouseArea {
                                id: modeMA
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: ThemeManager.layoutMode = modelData.key
                            }
                        }
                    }
                }
            }

            // ==========================================
            // 4. THEME LIST (favorites pinned on top)
            // ==========================================
            ListView {
                id: themeList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ListModel {
                    id: presetsModel
                    property int totalCount: 0
                }

                model: presetsModel

                delegate: Rectangle {
                    width: themeList.width
                    height: 34
                    radius: 4

                    readonly property bool isSelected: themeList.currentIndex === index
                    readonly property bool isCurrentTheme: window.activePresetName === name
                    color: isSelected ? "#313244"
                         : isCurrentTheme ? "#2a2a3d"
                         : (delMA.containsMouse ? "#262637" : "transparent")

                    MouseArea {
                        id: delMA
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: {
                            if (window.initialized && themeList.currentIndex !== index) {
                                themeList.currentIndex = index
                            }
                        }
                        onClicked: {
                            themeList.currentIndex = index
                            applyTheme(name)
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Rectangle {
                            width: 3; height: 16; radius: 1.5
                            color: isSelected ? "#89b4fa" : "transparent"
                        }

                        Text {
                            text: (isFavorite ? "★ " : "") + name +
                                  (isCurrentTheme ? " <font color='#89b4fa'>● active</font>" : "") +
                                  " <font color='#6c7086'>by " + author + "</font>"
                            color: isSelected || isCurrentTheme ? "#89b4fa" : "#cdd6f4"
                            font.pixelSize: 13
                            font.bold: isSelected || isCurrentTheme
                            textFormat: Text.RichText
                            Layout.fillWidth: true
                        }

                        Rectangle {
                            width: catLabel.width + 10
                            height: 16
                            radius: 8
                            color: "#1e1e2e"
                            Text {
                                id: catLabel
                                anchors.centerIn: parent
                                text: category
                                color: "#a6adc8"
                                font.pixelSize: 9
                            }
                        }

                        Text {
                            text: isFavorite ? "★" : "☆"
                            color: starMA.containsMouse ? "#f9e2af" : (isFavorite ? "#f9e2af" : "#6c7086")
                            font.pixelSize: 16
                            MouseArea {
                                id: starMA
                                anchors.fill: parent
                                anchors.margins: -6
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    ThemeManager.toggleFavorite(name)
                                    refreshFavoritesUi()
                                }
                            }
                        }
                    }
                }

                onCurrentItemChanged: {
                    if (window.initialized && currentIndex >= 0) {
                        var item = presetsModel.get(currentIndex)
                        if (item) previewTheme(item.name)
                    }
                }

                Keys.onReturnPressed: {
                    if (currentIndex >= 0) {
                        var item = presetsModel.get(currentIndex)
                        if (item) applyTheme(item.name)
                    }
                }

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_F && currentIndex >= 0) {
                        var item = presetsModel.get(currentIndex)
                        if (item) {
                            ThemeManager.toggleFavorite(item.name)
                            refreshFavoritesUi()
                        }
                        event.accepted = true
                    }
                }
            }
        }
    }

    // ====================================================
    // LEFT SIDE: Embedded Live Quasar Preview
    // Fills from left margin to the selector panel.
    // ====================================================
    Item {
        id: previewArea
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.right: selectorPanel.left
        anchors.rightMargin: 30

        // Glow/shadow behind preview box
        Rectangle {
            anchors.centerIn: previewBox
            width: previewBox.width + 16
            height: previewBox.height + 16
            radius: previewBox.radius + 4
            color: "transparent"
            border.color: Qt.alpha(ThemeManager.accentColor, 0.3)
            border.width: 2
            Behavior on border.color { ColorAnimation { duration: 180 } }
        }

        // Preview box: renders Quasar launcher mock using current ThemeManager values
        Rectangle {
            id: previewBox
            anchors.centerIn: parent
            width: Math.min(ThemeManager.windowWidth, parent.width - 40)
            height: Math.min(ThemeManager.windowHeight, parent.height - 80)
            radius: ThemeManager.borderRadius
            color: Qt.alpha(ThemeManager.backgroundColor, Math.max(ThemeManager.bgOpacity, 0.92))
            border.color: ThemeManager.borderColor
            border.width: Math.max(ThemeManager.borderWidth, 1)

            Behavior on color { ColorAnimation { duration: 180 } }
            Behavior on border.color { ColorAnimation { duration: 180 } }

            Column {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                // Mock search bar
                Rectangle {
                    width: parent.width
                    height: 42
                    radius: Math.max(ThemeManager.borderRadius - 2, 4)
                    color: Qt.alpha(ThemeManager.cardColor, ThemeManager.cardOpacity)
                    border.color: ThemeManager.accentColor
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 180 } }
                    Behavior on border.color { ColorAnimation { duration: 180 } }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 14
                        spacing: 6

                        Text {
                            text: "Search apps..."
                            color: Qt.alpha(ThemeManager.textColor, 0.5)
                            font.pixelSize: ThemeManager.fontSize
                            font.family: ThemeManager.fontFamily
                            Behavior on color { ColorAnimation { duration: 180 } }
                        }

                        Rectangle {
                            width: 2
                            height: ThemeManager.fontSize + 2
                            color: ThemeManager.accentColor
                            anchors.verticalCenter: parent.verticalCenter
                            Behavior on color { ColorAnimation { duration: 180 } }
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite
                                NumberAnimation { to: 0; duration: 530 }
                                NumberAnimation { to: 1; duration: 530 }
                            }
                        }
                    }
                }

                // Mock app list
                Column {
                    width: parent.width
                    spacing: 2
                    clip: true

                    Repeater {
                        model: [
                            { appName: "Firefox", appDesc: "Web Browser", appIcon: "web-browser" },
                            { appName: "Terminal", appDesc: "System Terminal", appIcon: "utilities-terminal" },
                            { appName: "Files", appDesc: "File Manager", appIcon: "system-file-manager" },
                            { appName: "Settings", appDesc: "System Settings", appIcon: "preferences-system" },
                            { appName: "Code Editor", appDesc: "Text Editor", appIcon: "text-editor" },
                            { appName: "Music", appDesc: "Audio Player", appIcon: "multimedia-audio-player" },
                            { appName: "Calculator", appDesc: "Accessories", appIcon: "accessories-calculator" }
                        ]

                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            width: parent.width
                            height: ThemeManager.iconSize + 14
                            radius: Math.max(ThemeManager.borderRadius - 4, 2)
                            color: index === 0
                                ? Qt.alpha(ThemeManager.accentColor, 0.15)
                                : "transparent"
                            Behavior on color { ColorAnimation { duration: 180 } }

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 10

                                // Icon
                                Rectangle {
                                    visible: ThemeManager.showIcons
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: ThemeManager.iconSize
                                    height: ThemeManager.iconSize
                                    radius: 4
                                    color: index === 0
                                        ? Qt.alpha(ThemeManager.accentColor, 0.25)
                                        : Qt.alpha(ThemeManager.cardColor, 0.4)
                                    Behavior on color { ColorAnimation { duration: 180 } }

                                    Image {
                                        anchors.centerIn: parent
                                        width: parent.width - 6
                                        height: parent.height - 6
                                        source: "image://icon/" + modelData.appIcon
                                        sourceSize: Qt.size(width, height)
                                        smooth: true
                                    }
                                }

                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 1

                                    Text {
                                        text: modelData.appName
                                        color: index === 0 ? ThemeManager.accentColor : ThemeManager.textColor
                                        font.pixelSize: ThemeManager.fontSize
                                        font.family: ThemeManager.fontFamily
                                        font.bold: index === 0
                                        Behavior on color { ColorAnimation { duration: 180 } }
                                    }

                                    Text {
                                        text: modelData.appDesc
                                        color: ThemeManager.secondaryTextColor
                                        font.pixelSize: Math.max(ThemeManager.fontSize - 3, 10)
                                        font.family: ThemeManager.fontFamily
                                        Behavior on color { ColorAnimation { duration: 180 } }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Label below preview
        Text {
            anchors.horizontalCenter: previewBox.horizontalCenter
            anchors.top: previewBox.bottom
            anchors.topMargin: 12
            text: "▲ Live Preview"
            color: "#89b4fa"
            font.pixelSize: 13
            font.bold: true
            opacity: 0.6
        }
    }

    // ==========================================
    // Logic helpers
    // ==========================================
    function previewTheme(name) {
        window.activePresetName = name
        ThemeManager.loadPreset(name)
    }

    function applyTheme(name) {
        window.activePresetName = name
        ThemeManager.loadPreset(name)
        ThemeManager.saveTheme()
    }

    function rebuildModel() {
        if (typeof ThemeManager === "undefined" || typeof ThemeManager.getAvailablePresets !== "function")
            return
        var presets = ThemeManager.getAvailablePresets()
        if (!presets)
            return
        var favs = (typeof ThemeManager.favoritePresets !== "undefined") ? ThemeManager.favoritePresets : []

        presetsModel.totalCount = presets.length
        var matching = []
        for (var i = 0; i < presets.length; i++) {
            var pName = presets[i]
            var cat = ThemeManager.getPresetCategory(pName)
            if (window.categoryFilter !== "All" && cat !== window.categoryFilter)
                continue
            if (window.searchFilter !== "") {
                var details = ThemeManager.getPresetDetails(pName)
                var author = (details && details.author) ? details.author : "Quasar"
                if (!pName.toLowerCase().includes(window.searchFilter) &&
                    !author.toLowerCase().includes(window.searchFilter))
                    continue
            }
            matching.push(pName)
        }

        matching.sort(function(a, b) {
            var fa = favs.indexOf(a) !== -1
            var fb = favs.indexOf(b) !== -1
            if (fa !== fb) return fa ? -1 : 1
            return a.localeCompare(b)
        })

        presetsModel.clear()
        var restoreIdx = 0
        for (var j = 0; j < matching.length; j++) {
            var nm = matching[j]
            var auth = "Quasar"
            var det = ThemeManager.getPresetDetails(nm)
            if (det && det.author) auth = det.author
            presetsModel.append({
                name: nm,
                author: auth,
                category: ThemeManager.getPresetCategory(nm),
                isFavorite: favs.indexOf(nm) !== -1
            })
            if (nm === window.activePresetName) restoreIdx = j
        }
        themeList.currentIndex = restoreIdx
    }

    Component.onCompleted: {
        if (typeof ThemeManager !== "undefined" && typeof ThemeManager.currentPresetName === "string") {
            window.activePresetName = ThemeManager.currentPresetName
        }
        rebuildModel()
        window.initialized = true
    }

    function refreshFavoritesUi() {
        var savedName = ""
        if (themeList.currentIndex >= 0) {
            var it = presetsModel.get(themeList.currentIndex)
            if (it) savedName = it.name
        }
        rebuildModel()
        for (var i = 0; i < presetsModel.count; i++) {
            if (presetsModel.get(i).name === savedName) {
                themeList.currentIndex = i
                break
            }
        }
    }

    Connections {
        target: typeof ThemeManager !== "undefined" ? ThemeManager : null
        function onFavoritesChanged() {
            refreshFavoritesUi()
        }
    }
}
