import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quasar 1.0

Window {
    id: window
    width: 820
    height: 540
    visible: true
    title: "Quasar Theme Studio & Customizer"
    color: "#11111b"
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint | Qt.WindowSystemMenuHint

    onActiveChanged: {
        if (!active && visible) {
            window.close()
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: window.close()
    }

    property int activeTab: 0
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

        // ==========================================
        // 1. LEFT ENTERPRISE NAVIGATION DRAWER
        // ==========================================
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 220
            color: "#181825"
            border.color: "#313244"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                // App Brand Header
                RowLayout {
                    spacing: 10
                    Rectangle {
                        width: 36; height: 36; radius: 10
                        color: "#89b4fa"
                        Text {
                            anchors.centerIn: parent
                            text: "⚡"
                            font.pixelSize: 18
                        }
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: "QUASAR"
                            color: "#cdd6f4"
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Text {
                            text: "Theme Engine v2.0"
                            color: "#6c7086"
                            font.pixelSize: 11
                        }
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

                // Navigation Tabs
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Repeater {
                        model: [
                            { icon: "🎨", title: "Presets Gallery", index: 0 },
                            { icon: "⚙️", title: "Colors & Styling", index: 1 },
                            { icon: "📐", title: "Geometry & Font", index: 2 },
                            { icon: "📁", title: "Import / Export", index: 3 }
                        ]
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            radius: 8
                            color: window.activeTab === modelData.index ? "#313244" : (navNavMouse.containsMouse ? "#1e1e2e" : "transparent")

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 10

                                Text {
                                    text: modelData.icon
                                    font.pixelSize: 14
                                }
                                Text {
                                    text: modelData.title
                                    color: window.activeTab === modelData.index ? "#89b4fa" : "#cdd6f4"
                                    font.pixelSize: 13
                                    font.bold: window.activeTab === modelData.index
                                    Layout.fillWidth: true
                                }
                            }

                            MouseArea {
                                id: navNavMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: window.activeTab = modelData.index
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // Bottom Active Theme Card
                Rectangle {
                    Layout.fillWidth: true
                    height: 64
                    radius: 10
                    color: "#1e1e2e"
                    border.color: "#313244"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 2
                        Text {
                            text: "ACTIVE PRESET"
                            color: "#a6adc8"
                            font.pixelSize: 9
                            font.bold: true
                        }
                        Text {
                            text: ThemeManager.promptText ? ThemeManager.promptText : "Custom Theme"
                            color: "#89b4fa"
                            font.pixelSize: 13
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Text {
                            text: "Auto-synced to config"
                            color: "#a6e3a1"
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }

        // ==========================================
        // 2. MAIN CENTER CONTENT AREA
        // ==========================================
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#11111b"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                // Status Toast
                Rectangle {
                    Layout.fillWidth: true
                    height: window.statusMessage !== "" ? 32 : 0
                    visible: window.statusMessage !== ""
                    radius: 6
                    color: "#a6e3a1"
                    opacity: 0.9

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        Text {
                            text: window.statusMessage
                            color: "#11111b"
                            font.pixelSize: 12
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        Text {
                            text: "✕"
                            color: "#11111b"
                            font.pixelSize: 12
                            MouseArea {
                                anchors.fill: parent
                                onClicked: window.statusMessage = ""
                            }
                        }
                    }
                }

                // TAB 0: PRESETS GALLERY
                ColumnLayout {
                    visible: window.activeTab === 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 14

                    // Filter Chips & Search Bar
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        // Search Bar
                        Rectangle {
                            Layout.fillWidth: true
                            height: 36
                            color: "#181825"
                            border.color: "#313244"
                            radius: 8

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 8

                                Text { text: "🔍"; font.pixelSize: 13 }
                                TextInput {
                                    id: searchInput
                                    Layout.fillWidth: true
                                    color: "#cdd6f4"
                                    font.pixelSize: 13
                                    onTextChanged: window.searchFilter = text.toLowerCase()

                                    Text {
                                        text: "Search presets..."
                                        color: "#6c7086"
                                        font.pixelSize: 13
                                        visible: !parent.text
                                    }
                                }
                            }
                        }
                    }

                    // Category Filter Pills
                    Row {
                        spacing: 8
                        Repeater {
                            model: ["All", "Dark", "Light", "Neon", "Retro"]
                            delegate: Rectangle {
                                width: catText.implicitWidth + 20
                                height: 28
                                radius: 14
                                color: window.selectedCategory === modelData ? "#89b4fa" : "#181825"
                                border.color: window.selectedCategory === modelData ? "#89b4fa" : "#313244"

                                Text {
                                    id: catText
                                    anchors.centerIn: parent
                                    text: modelData
                                    color: window.selectedCategory === modelData ? "#11111b" : "#cdd6f4"
                                    font.pixelSize: 12
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

                    // Presets Grid ScrollView
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        GridView {
                            id: presetsGrid
                            anchors.fill: parent
                            cellWidth: 235
                            cellHeight: 110
                            model: ThemeManager.getAvailablePresets()

                            delegate: Rectangle {
                                width: 220
                                height: 98
                                radius: 10
                                color: cardMouse.containsMouse ? "#24273a" : "#181825"
                                border.color: cardMouse.containsMouse ? details.accentColor : "#313244"
                                border.width: 1

                                visible: {
                                    var matchesCategory = (window.selectedCategory === "All" || ThemeManager.getPresetCategory(modelData) === window.selectedCategory)
                                    var matchesSearch = (window.searchFilter === "" || modelData.toLowerCase().indexOf(window.searchFilter) !== -1)
                                    return matchesCategory && matchesSearch
                                }

                                property var details: ThemeManager.getPresetDetails(modelData)

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 6

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Text {
                                            text: modelData
                                            color: "#cdd6f4"
                                            font.pixelSize: 13
                                            font.bold: true
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }
                                        Rectangle {
                                            width: tagText.implicitWidth + 8
                                            height: 18
                                            radius: 4
                                            color: "#313244"
                                            Text {
                                                id: tagText
                                                anchors.centerIn: parent
                                                text: ThemeManager.getPresetCategory(modelData)
                                                color: "#a6adc8"
                                                font.pixelSize: 9
                                            }
                                        }
                                    }

                                    // Palette Swatches & Prompt
                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 6

                                        // Swatch 1: Background
                                        Rectangle {
                                            width: 16; height: 16; radius: 8
                                            color: details.backgroundColor || "#11111b"
                                            border.color: "#585b70"; border.width: 1
                                        }
                                        // Swatch 2: Card
                                        Rectangle {
                                            width: 16; height: 16; radius: 8
                                            color: details.cardColor || "#1e1e2e"
                                            border.color: "#585b70"; border.width: 1
                                        }
                                        // Swatch 3: Accent
                                        Rectangle {
                                            width: 16; height: 16; radius: 8
                                            color: details.accentColor || "#89b4fa"
                                        }
                                        // Swatch 4: Text
                                        Rectangle {
                                            width: 16; height: 16; radius: 8
                                            color: details.textColor || "#cdd6f4"
                                        }

                                        Item { Layout.fillWidth: true }

                                        Text {
                                            text: details.promptText || ""
                                            color: "#6c7086"
                                            font.pixelSize: 10
                                            font.family: "Monospace"
                                        }
                                    }

                                    // Action Button
                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 24
                                        radius: 6
                                        color: details.accentColor ? details.accentColor : "#89b4fa"
                                        opacity: cardMouse.containsMouse ? 1.0 : 0.8

                                        Text {
                                            anchors.centerIn: parent
                                            text: "Apply Theme"
                                            color: "#11111b"
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                    }
                                }

                                MouseArea {
                                    id: cardMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        ThemeManager.loadPreset(modelData)
                                        window.statusMessage = "Loaded preset: " + modelData
                                    }
                                }
                            }
                        }
                    }
                }

                // TAB 1: COLORS & STYLING
                ScrollView {
                    visible: window.activeTab === 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ColumnLayout {
                        width: parent.width - 20
                        spacing: 12

                        Text { text: "Layout Mode"; color: "#89b4fa"; font.pixelSize: 14; font.bold: true }

                        Row {
                            spacing: 10
                            PresetButton {
                                presetName: "Vertical List (Rofi)"
                                isSelected: ThemeManager.layoutMode === "list"
                                onClicked: ThemeManager.layoutMode = "list"
                            }
                            PresetButton {
                                presetName: "Grid View (Spotlight)"
                                isSelected: ThemeManager.layoutMode === "grid"
                                onClicked: ThemeManager.layoutMode = "grid"
                            }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

                        Text { text: "Color Palette"; color: "#89b4fa"; font.pixelSize: 14; font.bold: true }

                        ColorPickerRow { label: "Accent / Highlight:"; colorValue: ThemeManager.accentColor; onColorChanged: function(c) { ThemeManager.accentColor = c } }
                        ColorPickerRow { label: "Background:"; colorValue: ThemeManager.backgroundColor; onColorChanged: function(c) { ThemeManager.backgroundColor = c } }
                        ColorPickerRow { label: "Card / Input:"; colorValue: ThemeManager.cardColor; onColorChanged: function(c) { ThemeManager.cardColor = c } }
                        ColorPickerRow { label: "Main Text:"; colorValue: ThemeManager.textColor; onColorChanged: function(c) { ThemeManager.textColor = c } }
                        ColorPickerRow { label: "Secondary Text:"; colorValue: ThemeManager.secondaryTextColor; onColorChanged: function(c) { ThemeManager.secondaryTextColor = c } }
                        ColorPickerRow { label: "Border Color:"; colorValue: ThemeManager.borderColor; onColorChanged: function(c) { ThemeManager.borderColor = c } }
                    }
                }

                // TAB 2: GEOMETRY & FONT
                ScrollView {
                    visible: window.activeTab === 2
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ColumnLayout {
                        width: parent.width - 20
                        spacing: 14

                        Text { text: "Window Dimensions"; color: "#89b4fa"; font.pixelSize: 14; font.bold: true }

                        RowLayout {
                            Text { text: "Width (" + ThemeManager.windowWidth + "px):"; color: "#cdd6f4"; font.pixelSize: 12; Layout.preferredWidth: 140 }
                            Slider { Layout.fillWidth: true; from: 400; to: 1200; stepSize: 10; value: ThemeManager.windowWidth; onMoved: ThemeManager.windowWidth = value }
                        }
                        RowLayout {
                            Text { text: "Height (" + ThemeManager.windowHeight + "px):"; color: "#cdd6f4"; font.pixelSize: 12; Layout.preferredWidth: 140 }
                            Slider { Layout.fillWidth: true; from: 250; to: 800; stepSize: 10; value: ThemeManager.windowHeight; onMoved: ThemeManager.windowHeight = value }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

                        Text { text: "Styling & Radius"; color: "#89b4fa"; font.pixelSize: 14; font.bold: true }

                        RowLayout {
                            Text { text: "Opacity (" + Math.round(ThemeManager.bgOpacity * 100) + "%):"; color: "#cdd6f4"; font.pixelSize: 12; Layout.preferredWidth: 140 }
                            Slider { Layout.fillWidth: true; from: 0.3; to: 1.0; value: ThemeManager.bgOpacity; onMoved: ThemeManager.bgOpacity = value }
                        }
                        RowLayout {
                            Text { text: "Corner Radius (" + ThemeManager.borderRadius + "px):"; color: "#cdd6f4"; font.pixelSize: 12; Layout.preferredWidth: 140 }
                            Slider { Layout.fillWidth: true; from: 0; to: 24; stepSize: 1; value: ThemeManager.borderRadius; onMoved: ThemeManager.borderRadius = value }
                        }
                        RowLayout {
                            Text { text: "Border Width (" + ThemeManager.borderWidth + "px):"; color: "#cdd6f4"; font.pixelSize: 12; Layout.preferredWidth: 140 }
                            Slider { Layout.fillWidth: true; from: 0; to: 6; stepSize: 1; value: ThemeManager.borderWidth; onMoved: ThemeManager.borderWidth = value }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

                        Text { text: "Typography & Prompt"; color: "#89b4fa"; font.pixelSize: 14; font.bold: true }

                        RowLayout {
                            Text { text: "Prompt Prefix:"; color: "#cdd6f4"; font.pixelSize: 12; Layout.preferredWidth: 140 }
                            Rectangle {
                                Layout.fillWidth: true; height: 28; color: "#181825"; border.color: "#313244"; radius: 4
                                TextInput { anchors.fill: parent; anchors.margins: 4; text: ThemeManager.promptText; color: "#cdd6f4"; font.pixelSize: 12; font.family: "Monospace"; onEditingFinished: ThemeManager.promptText = text }
                            }
                        }
                    }
                }

                // TAB 3: IMPORT / EXPORT
                ColumnLayout {
                    visible: window.activeTab === 3
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        height: 120
                        radius: 12
                        color: "#181825"
                        border.color: "#313244"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 16

                            Text { text: "📤"; font.pixelSize: 32 }
                            ColumnLayout {
                                Layout.fillWidth: true
                                Text { text: "Export Current Theme"; color: "#cdd6f4"; font.pixelSize: 15; font.bold: true }
                                Text { text: "Save your custom theme colors, fonts, and window geometry to a shareable JSON file."; color: "#a6adc8"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            }
                            Button { text: "Export JSON"; onClicked: exportDialog.open() }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 120
                        radius: 12
                        color: "#181825"
                        border.color: "#313244"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 16

                            Text { text: "📥"; font.pixelSize: 32 }
                            ColumnLayout {
                                Layout.fillWidth: true
                                Text { text: "Import Theme JSON"; color: "#cdd6f4"; font.pixelSize: 15; font.bold: true }
                                Text { text: "Load a community theme file (.json) to instantly transform Quasar's appearance."; color: "#a6adc8"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            }
                            Button { text: "Import JSON"; onClicked: importDialog.open() }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }

        // ==========================================
        // 3. RIGHT REAL-TIME LIVE LAUNCHER PREVIEW
        // ==========================================
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 340
            color: "#141423"
            border.color: "#313244"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "LIVE LAUNCHER PREVIEW"
                        color: "#89b4fa"
                        font.pixelSize: 12
                        font.bold: true
                        Layout.fillWidth: true
                    }
                    Rectangle {
                        width: 24; height: 24; radius: 12
                        color: closeMouse.containsMouse ? "#f38ba8" : "#313244"
                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            color: closeMouse.containsMouse ? "#11111b" : "#cdd6f4"
                            font.pixelSize: 12
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

                // Interactive Live Preview Container
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: ThemeManager.borderRadius
                    color: ThemeManager.backgroundColor
                    opacity: ThemeManager.bgOpacity
                    border.color: ThemeManager.borderColor
                    border.width: ThemeManager.borderWidth
                    clip: true

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10

                        // Search Input Bar Preview
                        Rectangle {
                            Layout.fillWidth: true
                            height: 36
                            radius: Math.max(4, ThemeManager.borderRadius - 4)
                            color: ThemeManager.cardColor
                            opacity: ThemeManager.cardOpacity

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 6

                                Text {
                                    text: ThemeManager.promptText ? ThemeManager.promptText : "search: "
                                    color: ThemeManager.accentColor
                                    font.pixelSize: ThemeManager.fontSize
                                    font.family: ThemeManager.fontFamily
                                    font.bold: true
                                }
                                Text {
                                    text: "firefox|"
                                    color: ThemeManager.textColor
                                    font.pixelSize: ThemeManager.fontSize
                                    font.family: ThemeManager.fontFamily
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        // App List Items Preview
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            // Active Highlighted Item
                            Rectangle {
                                Layout.fillWidth: true
                                height: 38
                                radius: Math.max(4, ThemeManager.borderRadius - 6)
                                color: ThemeManager.accentColor

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    spacing: 8

                                    Text { text: "🌐"; font.pixelSize: ThemeManager.iconSize }
                                    ColumnLayout {
                                        spacing: 0
                                        Text { text: "Firefox Web Browser"; color: "#11111b"; font.pixelSize: ThemeManager.fontSize; font.bold: true }
                                        Text { text: "Fast, Private & Secure"; color: "#313244"; font.pixelSize: 10 }
                                    }
                                }
                            }

                            // Secondary Inactive Item
                            Rectangle {
                                Layout.fillWidth: true
                                height: 38
                                radius: Math.max(4, ThemeManager.borderRadius - 6)
                                color: ThemeManager.cardColor
                                opacity: ThemeManager.cardOpacity

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    spacing: 8

                                    Text { text: "💻"; font.pixelSize: ThemeManager.iconSize }
                                    ColumnLayout {
                                        spacing: 0
                                        Text { text: "Kitty Terminal"; color: ThemeManager.textColor; font.pixelSize: ThemeManager.fontSize }
                                        Text { text: "GPU-accelerated terminal"; color: ThemeManager.secondaryTextColor; font.pixelSize: 10 }
                                    }
                                }
                            }

                            // Third Inactive Item
                            Rectangle {
                                Layout.fillWidth: true
                                height: 38
                                radius: Math.max(4, ThemeManager.borderRadius - 6)
                                color: ThemeManager.cardColor
                                opacity: ThemeManager.cardOpacity

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    spacing: 8

                                    Text { text: "📁"; font.pixelSize: ThemeManager.iconSize }
                                    ColumnLayout {
                                        spacing: 0
                                        Text { text: "Dolphin File Manager"; color: ThemeManager.textColor; font.pixelSize: ThemeManager.fontSize }
                                        Text { text: "Manage your files"; color: ThemeManager.secondaryTextColor; font.pixelSize: 10 }
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                // Apply to Daemon Button
                Button {
                    Layout.fillWidth: true
                    height: 38
                    contentItem: Text {
                        text: "💾 Save Theme Settings"
                        color: "#11111b"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#a6e3a1"
                        radius: 8
                    }
                    onClicked: {
                        ThemeManager.saveTheme()
                        window.statusMessage = "Theme saved and applied to Quasar daemon!"
                    }
                }
            }
        }
    }
}
