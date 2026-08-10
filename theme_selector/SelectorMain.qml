import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quasar 1.0

Window {
    id: window
    width: 980
    height: 660
    visible: true
    title: "Quasar Theme Selector & Designer"
    color: "#11111b"
    flags: Qt.Window

    Shortcut {
        sequence: "Escape"
        onActivated: window.close()
    }

    property string selectedCategory: "All"
    property string searchFilter: ""
    property string statusMessage: ""

    FileDialog {
        id: importDialog
        title: "Import Theme JSON File"
        nameFilters: ["JSON Theme Files (*.json)", "All Files (*)"]
        onAccepted: {
            if (ThemeManager.importTheme(selectedFile)) {
                window.statusMessage = "Successfully imported theme from JSON!"
            } else {
                window.statusMessage = "Error importing theme file."
            }
        }
    }

    FileDialog {
        id: exportDialog
        title: "Export Current Theme JSON"
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON Theme Files (*.json)", "All Files (*)"]
        onAccepted: {
            if (ThemeManager.exportTheme(selectedFile)) {
                window.statusMessage = "Theme exported successfully to JSON!"
            } else {
                window.statusMessage = "Error exporting theme file."
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left Panel - Settings Controls
        ScrollView {
            id: settingsScroll
            Layout.fillHeight: true
            Layout.preferredWidth: 520
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: settingsScroll.availableWidth - 20
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 12

                    Text {
                        text: "Theme Presets"
                        color: "#89b4fa"
                        font.pixelSize: 16
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Import JSON"
                        onClicked: importDialog.open()
                    }

                    Button {
                        text: "Export JSON"
                        onClicked: exportDialog.open()
                    }
                }

                // Preset Search Input
                Rectangle {
                    Layout.fillWidth: true
                    height: 32
                    color: "#181825"
                    border.color: "#313244"
                    radius: 6

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 6

                        Text {
                            text: "🔍"
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        TextInput {
                            id: searchInput
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - 24
                            color: "#cdd6f4"
                            font.pixelSize: 12
                            onTextChanged: window.searchFilter = text.toLowerCase()

                            Text {
                                text: "Filter presets by name..."
                                color: "#6c7086"
                                font.pixelSize: 12
                                visible: !parent.text
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }

                // Category Chips
                Row {
                    spacing: 6
                    Repeater {
                        model: ["All", "Dark", "Light", "Neon", "Retro"]
                        delegate: Rectangle {
                            width: chipText.implicitWidth + 16
                            height: 26
                            radius: 13
                            color: window.selectedCategory === modelData ? "#8aadf4" : "#181825"
                            border.color: window.selectedCategory === modelData ? "#8aadf4" : "#313244"

                            Text {
                                id: chipText
                                anchors.centerIn: parent
                                text: modelData
                                color: window.selectedCategory === modelData ? "#11111b" : "#cdd6f4"
                                font.pixelSize: 11
                                font.bold: window.selectedCategory === modelData
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: window.selectedCategory = modelData
                            }
                        }
                    }
                }

                Flow {
                    id: presetsFlow
                    Layout.fillWidth: true
                    Layout.preferredHeight: childrenRect.height
                    height: childrenRect.height
                    spacing: 8

                    Repeater {
                        model: ThemeManager.getAvailablePresets()
                        delegate: PresetButton {
                            presetName: modelData
                            isSelected: false
                            visible: {
                                var matchesCategory = (window.selectedCategory === "All" || ThemeManager.getPresetCategory(modelData) === window.selectedCategory)
                                var matchesSearch = (window.searchFilter === "" || modelData.toLowerCase().indexOf(window.searchFilter) !== -1)
                                return matchesCategory && matchesSearch
                            }
                            onClicked: {
                                ThemeManager.loadPreset(modelData)
                                window.statusMessage = "Loaded preset: " + modelData
                            }
                        }
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

                Text {
                    text: "Layout Mode"
                    color: "#89b4fa"
                    font.pixelSize: 15
                    font.bold: true
                }

                Row {
                    spacing: 10
                    PresetButton {
                        presetName: "Vertical List"
                        isSelected: ThemeManager.layoutMode === "list"
                        onClicked: ThemeManager.layoutMode = "list"
                    }
                    PresetButton {
                        presetName: "Grid View"
                        isSelected: ThemeManager.layoutMode === "grid"
                        onClicked: ThemeManager.layoutMode = "grid"
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

                Text {
                    text: "Colors"
                    color: "#89b4fa"
                    font.pixelSize: 15
                    font.bold: true
                }

                Column {
                    spacing: 6
                    Layout.fillWidth: true

                    ColorPickerRow {
                        label: "Accent / Highlight:"
                        colorValue: ThemeManager.accentColor
                        onColorChanged: function(c) { ThemeManager.accentColor = c }
                    }
                    ColorPickerRow {
                        label: "Background:"
                        colorValue: ThemeManager.backgroundColor
                        onColorChanged: function(c) { ThemeManager.backgroundColor = c }
                    }
                    ColorPickerRow {
                        label: "Card / Input:"
                        colorValue: ThemeManager.cardColor
                        onColorChanged: function(c) { ThemeManager.cardColor = c }
                    }
                    ColorPickerRow {
                        label: "Main Text:"
                        colorValue: ThemeManager.textColor
                        onColorChanged: function(c) { ThemeManager.textColor = c }
                    }
                    ColorPickerRow {
                        label: "Secondary Text:"
                        colorValue: ThemeManager.secondaryTextColor
                        onColorChanged: function(c) { ThemeManager.secondaryTextColor = c }
                    }
                    ColorPickerRow {
                        label: "Border Color:"
                        colorValue: ThemeManager.borderColor
                        onColorChanged: function(c) { ThemeManager.borderColor = c }
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

                Text {
                    text: "Geometry & Style"
                    color: "#89b4fa"
                    font.pixelSize: 15
                    font.bold: true
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    RowLayout {
                        Text { text: "Prompt Prefix:"; color: "#cdd6f4"; font.pixelSize: 13; Layout.preferredWidth: 140 }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 28
                            color: "#181825"
                            border.color: "#313244"
                            radius: 4
                            TextInput {
                                anchors.fill: parent
                                anchors.margins: 4
                                text: ThemeManager.promptText
                                color: "#cdd6f4"
                                font.pixelSize: 12
                                font.family: "Monospace"
                                onEditingFinished: ThemeManager.promptText = text
                            }
                        }
                    }

                    RowLayout {
                        Text { text: "Opacity (" + Math.round(ThemeManager.bgOpacity * 100) + "%):"; color: "#cdd6f4"; font.pixelSize: 13; Layout.preferredWidth: 160 }
                        Slider {
                            Layout.fillWidth: true
                            from: 0.3; to: 1.0
                            value: ThemeManager.bgOpacity
                            onMoved: ThemeManager.bgOpacity = value
                        }
                    }

                    RowLayout {
                        Text { text: "Border Radius (" + ThemeManager.borderRadius + "px):"; color: "#cdd6f4"; font.pixelSize: 13; Layout.preferredWidth: 160 }
                        Slider {
                            Layout.fillWidth: true
                            from: 0; to: 24; stepSize: 1
                            value: ThemeManager.borderRadius
                            onMoved: ThemeManager.borderRadius = value
                        }
                    }

                    RowLayout {
                        Text { text: "Font Size (" + ThemeManager.fontSize + "px):"; color: "#cdd6f4"; font.pixelSize: 13; Layout.preferredWidth: 160 }
                        Slider {
                            Layout.fillWidth: true
                            from: 10; to: 20; stepSize: 1
                            value: ThemeManager.fontSize
                            onMoved: ThemeManager.fontSize = value
                        }
                    }

                    RowLayout {
                        Text { text: "Icon Size (" + ThemeManager.iconSize + "px):"; color: "#cdd6f4"; font.pixelSize: 13; Layout.preferredWidth: 160 }
                        Slider {
                            Layout.fillWidth: true
                            from: 16; to: 64; stepSize: 4
                            value: ThemeManager.iconSize
                            onMoved: ThemeManager.iconSize = value
                        }
                    }

                    RowLayout {
                        Text { text: "Window Width (" + ThemeManager.windowWidth + "px):"; color: "#cdd6f4"; font.pixelSize: 13; Layout.preferredWidth: 160 }
                        Slider {
                            Layout.fillWidth: true
                            from: 400; to: 900; stepSize: 10
                            value: ThemeManager.windowWidth
                            onMoved: ThemeManager.windowWidth = value
                        }
                    }

                    RowLayout {
                        Text { text: "Window Height (" + ThemeManager.windowHeight + "px):"; color: "#cdd6f4"; font.pixelSize: 13; Layout.preferredWidth: 160 }
                        Slider {
                            Layout.fillWidth: true
                            from: 300; to: 700; stepSize: 10
                            value: ThemeManager.windowHeight
                            onMoved: ThemeManager.windowHeight = value
                        }
                    }

                    CheckBox {
                        text: "Show App Icons"
                        checked: ThemeManager.showIcons
                        onCheckedChanged: ThemeManager.showIcons = checked
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 16; color: "transparent" }
            }
        }

        // Right Panel - Live Preview
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#181825"
            border.color: "#313244"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "Live Interactive Preview"
                        color: "#89b4fa"
                        font.pixelSize: 15
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Text {
                        text: window.statusMessage
                        color: "#a6e3a1"
                        font.pixelSize: 12
                        visible: text.length > 0
                    }
                }

                // Simulated Launcher Box
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: ThemeManager.borderRadius
                    color: Qt.alpha(ThemeManager.backgroundColor, ThemeManager.bgOpacity)
                    border.color: ThemeManager.borderColor
                    border.width: ThemeManager.borderWidth
                    clip: true

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10

                        // Preview Input Bar
                        Rectangle {
                            width: parent.width
                            height: ThemeManager.fontSize * 2 + 10
                            radius: ThemeManager.borderRadius
                            color: Qt.alpha(ThemeManager.cardColor, ThemeManager.cardOpacity)
                            border.color: ThemeManager.accentColor
                            border.width: ThemeManager.borderWidth

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 6

                                Text {
                                    text: ThemeManager.promptText
                                    font.pixelSize: ThemeManager.fontSize
                                    font.bold: true
                                    color: ThemeManager.accentColor
                                    anchors.verticalCenter: parent.verticalCenter
                                    visible: text.length > 0
                                }

                                Text {
                                    text: "firefox"
                                    font.pixelSize: ThemeManager.fontSize
                                    color: ThemeManager.textColor
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        // Preview Item List
                        Column {
                            width: parent.width
                            spacing: 4
                            visible: ThemeManager.layoutMode === "list"

                            // Selected item
                            Rectangle {
                                width: parent.width
                                height: ThemeManager.showIcons ? Math.max(ThemeManager.iconSize + 8, 34) : 32
                                radius: Math.min(ThemeManager.borderRadius, 6)
                                color: ThemeManager.accentColor

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    spacing: 8

                                    Rectangle {
                                        width: ThemeManager.iconSize
                                        height: width
                                        radius: 4
                                        color: "#ffffff30"
                                        anchors.verticalCenter: parent.verticalCenter
                                        visible: ThemeManager.showIcons
                                    }

                                    Text {
                                        text: "Firefox Web Browser"
                                        color: "#ffffff"
                                        font.pixelSize: ThemeManager.fontSize
                                        font.bold: true
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }

                            // Normal item 1
                            Rectangle {
                                width: parent.width
                                height: ThemeManager.showIcons ? Math.max(ThemeManager.iconSize + 8, 34) : 32
                                radius: Math.min(ThemeManager.borderRadius, 6)
                                color: "transparent"

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    spacing: 8

                                    Rectangle {
                                        width: ThemeManager.iconSize
                                        height: width
                                        radius: 4
                                        color: "#ffffff15"
                                        anchors.verticalCenter: parent.verticalCenter
                                        visible: ThemeManager.showIcons
                                    }

                                    Text {
                                        text: "Kitty Terminal"
                                        color: ThemeManager.textColor
                                        font.pixelSize: ThemeManager.fontSize
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }

                            // Normal item 2
                            Rectangle {
                                width: parent.width
                                height: ThemeManager.showIcons ? Math.max(ThemeManager.iconSize + 8, 34) : 32
                                radius: Math.min(ThemeManager.borderRadius, 6)
                                color: "transparent"

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    spacing: 8

                                    Rectangle {
                                        width: ThemeManager.iconSize
                                        height: width
                                        radius: 4
                                        color: "#ffffff15"
                                        anchors.verticalCenter: parent.verticalCenter
                                        visible: ThemeManager.showIcons
                                    }

                                    Text {
                                        text: "Dolphin File Manager"
                                        color: ThemeManager.textColor
                                        font.pixelSize: ThemeManager.fontSize
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }
                        }

                        // Preview Grid View
                        Grid {
                            columns: 3
                            spacing: 10
                            visible: ThemeManager.layoutMode === "grid"

                            Rectangle {
                                width: 90; height: 90
                                radius: ThemeManager.borderRadius
                                color: Qt.alpha(ThemeManager.accentColor, 0.3)
                                border.color: ThemeManager.accentColor

                                Column {
                                    anchors.centerIn: parent
                                    spacing: 4
                                    Rectangle { width: 36; height: 36; radius: 6; color: "#ffffff30"; anchors.horizontalCenter: parent.horizontalCenter }
                                    Text { text: "Firefox"; color: ThemeManager.textColor; font.pixelSize: 11 }
                                }
                            }

                            Rectangle {
                                width: 90; height: 90
                                radius: ThemeManager.borderRadius
                                color: "transparent"

                                Column {
                                    anchors.centerIn: parent
                                    spacing: 4
                                    Rectangle { width: 36; height: 36; radius: 6; color: "#ffffff15"; anchors.horizontalCenter: parent.horizontalCenter }
                                    Text { text: "Kitty"; color: ThemeManager.textColor; font.pixelSize: 11 }
                                }
                            }
                        }
                    }
                }

                Button {
                    Layout.fillWidth: true
                    height: 36
                    text: "Apply & Save Theme"
                    onClicked: {
                        ThemeManager.saveTheme()
                        window.statusMessage = "Theme saved to ~/.config/quasar/theme.json"
                    }
                }
            }
        }
    }
}
