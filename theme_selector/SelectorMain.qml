import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quasar 1.0

Window {
    id: window
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    property string searchFilter: ""
    property string activePresetName: "Catppuccin Mocha"

    Shortcut {
        sequence: "Escape"
        onActivated: window.close()
    }

    // Outer Fullscreen Transparent Backdrop Overlay
    Rectangle {
        anchors.fill: parent
        color: "#50000000"

        // Clicking outside the centered dialog CLOSES THE WINDOW INSTANTLY!
        MouseArea {
            anchors.fill: parent
            onClicked: window.close()
        }

        // Centered Rofi-Style Theme Selector Dialog Window
        Rectangle {
            id: dialogBox
            width: 860
            height: 560
            anchors.centerIn: parent
            radius: 12
            color: "#181825"
            border.color: "#313244"
            border.width: 1

            // Consume clicks inside the dialog so self-click doesn't close it
            MouseArea {
                anchors.fill: parent
                onClicked: {}
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                // ==========================================
                // 1. TOP ROFI HEADER BAR
                // ==========================================
                Rectangle {
                    Layout.fillWidth: true
                    height: 48
                    radius: 8
                    color: "#1e1e2e"
                    border.color: "#313244"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 10

                        Text {
                            text: "Theme:"
                            color: "#89b4fa"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        TextField {
                            id: searchInput
                            placeholderText: "Type to filter themes..."
                            placeholderTextColor: "#6c7086"
                            color: "#cdd6f4"
                            font.pixelSize: 14
                            font.family: ThemeManager.fontFamily
                            background: Item {}
                            Layout.fillWidth: true
                            focus: true

                            onTextChanged: window.searchFilter = text.toLowerCase()
                            Keys.onDownPressed: themeList.forceActiveFocus()
                        }

                        // Help legend
                        Text {
                            text: "Enter: Apply | ↑/↓: Navigate | Esc: Cancel"
                            color: "#6c7086"
                            font.pixelSize: 11
                        }

                        // Red Close Button
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
                }

                // ==========================================
                // 2. MAIN CONTENT AREA (2 COLUMNS)
                // ==========================================
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 14

                    // LEFT PANE: Theme Selector & 3 Layout Modes
                    ColumnLayout {
                        Layout.preferredWidth: 380
                        Layout.fillHeight: true
                        spacing: 10

                        // Layout Mode Selector (3 Modes!)
                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            radius: 8
                            color: "#1e1e2e"
                            border.color: "#313244"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 4
                                spacing: 4

                                // 1. Vertical List Mode
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    radius: 6
                                    color: ThemeManager.layoutMode === "list" ? "#89b4fa" : (mode1Mouse.containsMouse ? "#313244" : "transparent")
                                    Text {
                                        anchors.centerIn: parent
                                        text: "📄 List"
                                        color: ThemeManager.layoutMode === "list" ? "#11111b" : "#cdd6f4"
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                    MouseArea {
                                        id: mode1Mouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: ThemeManager.layoutMode = "list"
                                    }
                                }

                                // 2. Grid View Mode
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    radius: 6
                                    color: ThemeManager.layoutMode === "grid" ? "#89b4fa" : (mode2Mouse.containsMouse ? "#313244" : "transparent")
                                    Text {
                                        anchors.centerIn: parent
                                        text: "🔲 Grid"
                                        color: ThemeManager.layoutMode === "grid" ? "#11111b" : "#cdd6f4"
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                    MouseArea {
                                        id: mode2Mouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: ThemeManager.layoutMode = "grid"
                                    }
                                }

                                // 3. Compact Dock Mode (NEW 3rd LAYOUT MODE!)
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    radius: 6
                                    color: ThemeManager.layoutMode === "compact" ? "#89b4fa" : (mode3Mouse.containsMouse ? "#313244" : "transparent")
                                    Text {
                                        anchors.centerIn: parent
                                        text: "📑 Compact"
                                        color: ThemeManager.layoutMode === "compact" ? "#11111b" : "#cdd6f4"
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                    MouseArea {
                                        id: mode3Mouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: ThemeManager.layoutMode = "compact"
                                    }
                                }
                            }
                        }

                        // Rofi Theme List
                        ListView {
                            id: themeList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            ListModel {
                                id: themePresetsModel
                                ListElement { name: "Catppuccin Mocha"; author: "Catppuccin Team"; category: "Dark" }
                                ListElement { name: "Catppuccin Macchiato"; author: "Catppuccin Team"; category: "Dark" }
                                ListElement { name: "Catppuccin Latte"; author: "Catppuccin Team"; category: "Light" }
                                ListElement { name: "Tokyo Night"; author: "folke"; category: "Dark" }
                                ListElement { name: "Tokyo Night Light"; author: "folke"; category: "Light" }
                                ListElement { name: "Nord Dark"; author: "arcticicestudio"; category: "Dark" }
                                ListElement { name: "Cyberpunk 2077"; author: "CD Projekt Red"; category: "Neon" }
                                ListElement { name: "Dracula"; author: "Zeno Rocha"; category: "Dark" }
                                ListElement { name: "Gruvbox Dark"; author: "morhetz"; category: "Retro" }
                                ListElement { name: "Gruvbox Light"; author: "morhetz"; category: "Light" }
                                ListElement { name: "One Dark Pro"; author: "binaryify"; category: "Dark" }
                                ListElement { name: "Monokai Pro"; author: "monokai"; category: "Dark" }
                                ListElement { name: "Rose Pine"; author: "rosepine"; category: "Dark" }
                                ListElement { name: "Kanagawa"; author: "rebelot"; category: "Dark" }
                                ListElement { name: "Synthwave '84"; author: "robbowen"; category: "Neon" }
                                ListElement { name: "OLED Black"; author: "Quasar"; category: "Dark" }
                                ListElement { name: "Modern Glass"; author: "Quasar"; category: "Dark" }
                                ListElement { name: "Rofi Adapta-Nokto"; author: "PyGeek03"; category: "Rofi" }
                                ListElement { name: "Rofi Arc-Dark"; author: "leofa"; category: "Rofi" }
                                ListElement { name: "Rofi Solarized"; author: "altercation"; category: "Rofi" }
                                ListElement { name: "Rofi Monokai"; author: "monokai"; category: "Rofi" }
                                ListElement { name: "Rofi Material"; author: "material"; category: "Rofi" }
                                ListElement { name: "Rofi DarkBlue"; author: "Qball"; category: "Rofi" }
                            }

                            model: themePresetsModel

                            delegate: Rectangle {
                                width: themeList.width
                                height: visible ? 42 : 0
                                radius: 6
                                visible: window.searchFilter === "" || name.toLowerCase().includes(window.searchFilter) || author.toLowerCase().includes(window.searchFilter)

                                property bool isSelected: window.activePresetName === name
                                color: (delegateMouse.containsMouse || isSelected) ? "#313244" : "transparent"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 10
                                    spacing: 8

                                    // Selection Marker Indicator
                                    Rectangle {
                                        width: 4; height: 20; radius: 2
                                        color: isSelected ? "#89b4fa" : "transparent"
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text {
                                            text: name + "  <font color='#6c7086'>by " + author + "</font>"
                                            color: isSelected ? "#89b4fa" : "#cdd6f4"
                                            font.pixelSize: 13
                                            font.bold: isSelected
                                            textFormat: Text.RichText
                                        }
                                    }

                                    // Category Pill
                                    Rectangle {
                                        width: catText.width + 12
                                        height: 18
                                        radius: 9
                                        color: "#1e1e2e"
                                        Text {
                                            id: catText
                                            anchors.centerIn: parent
                                            text: category
                                            color: "#a6adc8"
                                            font.pixelSize: 10
                                        }
                                    }
                                }

                                MouseArea {
                                    id: delegateMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        window.activePresetName = name
                                        ThemeManager.loadPreset(name)
                                    }
                                }
                            }

                            Keys.onReturnPressed: {
                                if (currentItem) {
                                    ThemeManager.loadPreset(themePresetsModel.get(currentIndex).name)
                                    ThemeManager.saveCurrentTheme()
                                    window.close()
                                }
                            }
                        }
                    }

                    // RIGHT PANE: Real-time Live Interactive Launcher Preview
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 10

                        Text {
                            text: "LIVE LAUNCHER PREVIEW (" + ThemeManager.layoutMode.toUpperCase() + " MODE)"
                            color: "#89b4fa"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        // Live Quasar Launcher Preview Container
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
                                anchors.margins: 14
                                spacing: 10

                                // Search Bar Preview
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

                                // List & Compact Preview
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    spacing: 6
                                    visible: ThemeManager.layoutMode === "list" || ThemeManager.layoutMode === "compact"

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: ThemeManager.layoutMode === "compact" ? 32 : 40
                                        radius: Math.min(ThemeManager.borderRadius, 6)
                                        color: ThemeManager.accentColor

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 10
                                            anchors.rightMargin: 10
                                            spacing: 10

                                            Rectangle {
                                                width: ThemeManager.layoutMode === "compact" ? 18 : 24
                                                height: width
                                                radius: 4
                                                color: ThemeManager.textColor
                                            }
                                            Text {
                                                text: "Firefox Web Browser"
                                                color: "#11111b"
                                                font.pixelSize: ThemeManager.fontSize
                                                font.bold: true
                                            }
                                        }
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: ThemeManager.layoutMode === "compact" ? 32 : 40
                                        radius: Math.min(ThemeManager.borderRadius, 6)
                                        color: "transparent"

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 10
                                            anchors.rightMargin: 10
                                            spacing: 10

                                            Rectangle {
                                                width: ThemeManager.layoutMode === "compact" ? 18 : 24
                                                height: width
                                                radius: 4
                                                color: ThemeManager.accentColor
                                            }
                                            Text {
                                                text: "Kitty Terminal"
                                                color: ThemeManager.textColor
                                                font.pixelSize: ThemeManager.fontSize
                                            }
                                        }
                                    }
                                }

                                // Grid Preview
                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    spacing: 12
                                    visible: ThemeManager.layoutMode === "grid"

                                    Rectangle {
                                        width: 80; height: 90
                                        radius: ThemeManager.borderRadius
                                        color: ThemeManager.cardColor
                                        border.color: ThemeManager.accentColor
                                        border.width: 2

                                        ColumnLayout {
                                            anchors.centerIn: parent
                                            spacing: 6
                                            Rectangle { width: 32; height: 32; radius: 6; color: ThemeManager.accentColor; Layout.alignment: Qt.AlignHCenter }
                                            Text { text: "Firefox"; color: ThemeManager.textColor; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
                                        }
                                    }

                                    Rectangle {
                                        width: 80; height: 90
                                        radius: ThemeManager.borderRadius
                                        color: ThemeManager.cardColor

                                        ColumnLayout {
                                            anchors.centerIn: parent
                                            spacing: 6
                                            Rectangle { width: 32; height: 32; radius: 6; color: ThemeManager.accentColor; Layout.alignment: Qt.AlignHCenter }
                                            Text { text: "Kitty"; color: ThemeManager.textColor; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
                                        }
                                    }
                                }
                            }
                        }

                        // Apply Button
                        Rectangle {
                            Layout.fillWidth: true
                            height: 42
                            radius: 8
                            color: applyMouse.containsMouse ? "#a6e3a1" : "#89b4fa"

                            Text {
                                anchors.centerIn: parent
                                text: "Apply & Save Theme Settings"
                                color: "#11111b"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            MouseArea {
                                id: applyMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    ThemeManager.saveCurrentTheme()
                                    window.close()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
