import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Quasar 1.0

Window {
    id: window
    width: 620
    height: 450
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    property string searchFilter: ""
    property string activePresetName: "Catppuccin Mocha"

    onActiveChanged: {
        if (!active && visible) {
            window.close()
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: window.close()
    }

    Rectangle {
        id: container
        anchors.fill: parent
        color: "#181825"
        border.color: "#89b4fa"
        border.width: 1
        radius: 8

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10

            // ==========================================
            // 1. ROFI FILTER & HEADER BAR (Matching Image 2)
            // ==========================================
            Rectangle {
                Layout.fillWidth: true
                height: 38
                color: "#1e1e2e"
                radius: 4
                border.color: "#313244"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
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

                        onTextChanged: window.searchFilter = text.toLowerCase()
                        Keys.onDownPressed: themeList.forceActiveFocus()
                    }

                    // Red close button
                    Rectangle {
                        width: 20; height: 20; radius: 10
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
            // 2. ROFI HELP INSTRUCTIONS BANNER
            // ==========================================
            Rectangle {
                Layout.fillWidth: true
                height: 50
                color: "#1e1e2e"
                radius: 4
                border.color: "#313244"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 2

                    Text {
                        text: "You can preview themes by hitting <b>Enter</b> or moving selection."
                        color: "#cdd6f4"
                        font.pixelSize: 11
                        textFormat: Text.RichText
                    }
                    Text {
                        text: "<b>Escape</b> to cancel. Current theme: <font color='#89b4fa'><b>" + window.activePresetName + "</b></font>"
                        color: "#a6adc8"
                        font.pixelSize: 11
                        textFormat: Text.RichText
                    }
                }
            }

            // ==========================================
            // 3. LAYOUT MODE PILLS BAR (3 Layout Modes!)
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
                        text: "  Mode:"
                        color: "#a6adc8"
                        font.pixelSize: 11
                        font.bold: true
                    }

                    // 1. List Mode
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 4
                        color: ThemeManager.layoutMode === "list" ? "#89b4fa" : (m1.containsMouse ? "#313244" : "transparent")
                        Text {
                            anchors.centerIn: parent
                            text: "📄 List"
                            color: ThemeManager.layoutMode === "list" ? "#11111b" : "#cdd6f4"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        MouseArea {
                            id: m1
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: ThemeManager.layoutMode = "list"
                        }
                    }

                    // 2. Grid Mode
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 4
                        color: ThemeManager.layoutMode === "grid" ? "#89b4fa" : (m2.containsMouse ? "#313244" : "transparent")
                        Text {
                            anchors.centerIn: parent
                            text: "🔲 Grid"
                            color: ThemeManager.layoutMode === "grid" ? "#11111b" : "#cdd6f4"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        MouseArea {
                            id: m2
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: ThemeManager.layoutMode = "grid"
                        }
                    }

                    // 3. Compact Mode
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 4
                        color: ThemeManager.layoutMode === "compact" ? "#89b4fa" : (m3.containsMouse ? "#313244" : "transparent")
                        Text {
                            anchors.centerIn: parent
                            text: "📑 Compact"
                            color: ThemeManager.layoutMode === "compact" ? "#11111b" : "#cdd6f4"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        MouseArea {
                            id: m3
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: ThemeManager.layoutMode = "compact"
                        }
                    }
                }
            }

            // ==========================================
            // 4. ROFI THEME SELECTABLE LIST
            // ==========================================
            ListView {
                id: themeList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ListModel {
                    id: presetsModel
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

                model: presetsModel

                delegate: Rectangle {
                    width: themeList.width
                    height: visible ? 34 : 0
                    radius: 4
                    visible: window.searchFilter === "" || name.toLowerCase().includes(window.searchFilter) || author.toLowerCase().includes(window.searchFilter)

                    property bool isSelected: themeList.currentIndex === index || window.activePresetName === name
                    color: isSelected ? "#313244" : (delMouse.containsMouse ? "#262637" : "transparent")

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8

                        Rectangle {
                            width: 3; height: 16; radius: 1.5
                            color: isSelected ? "#89b4fa" : "transparent"
                        }

                        Text {
                            text: name + " <font color='#6c7086'>by " + author + "</font>"
                            color: isSelected ? "#89b4fa" : "#cdd6f4"
                            font.pixelSize: 13
                            font.bold: isSelected
                            textFormat: Text.RichText
                            Layout.fillWidth: true
                        }

                        Rectangle {
                            width: cText.width + 10
                            height: 16
                            radius: 8
                            color: "#1e1e2e"
                            Text {
                                id: cText
                                anchors.centerIn: parent
                                text: category
                                color: "#a6adc8"
                                font.pixelSize: 9
                            }
                        }
                    }

                    MouseArea {
                        id: delMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            themeList.currentIndex = index
                            window.activePresetName = name
                            ThemeManager.loadPreset(name)
                            ThemeManager.saveCurrentTheme()
                            window.close()
                        }
                    }
                }

                onCurrentItemChanged: {
                    if (currentItem && currentIndex >= 0) {
                        var item = presetsModel.get(currentIndex)
                        if (item) {
                            window.activePresetName = item.name
                            ThemeManager.loadPreset(item.name)
                        }
                    }
                }

                Keys.onReturnPressed: {
                    if (currentIndex >= 0) {
                        var item = presetsModel.get(currentIndex)
                        if (item) {
                            ThemeManager.loadPreset(item.name)
                            ThemeManager.saveCurrentTheme()
                            window.close()
                        }
                    }
                }
            }
        }
    }
}
