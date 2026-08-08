cmake_minimum_required(VERSION 3.16)

project(nexus-launcher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui DBus QuickControls2)
find_package(LayerShellQt REQUIRED)

qt_standard_project_setup()

# 1. Main Launcher Executable
qt_add_executable(nexus-launcher
    src/main.cpp
    src/launchercontroller.h
    src/appindexer.cpp
    src/appindexer.h
    src/fuzzymatcher.cpp
    src/fuzzymatcher.h
    src/frecencyranker.cpp
    src/frecencyranker.h
    src/thememanager.cpp
    src/thememanager.h
)

qt_add_qml_module(nexus-launcher
    URI com.nexus.launcher
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/SearchBar.qml
        qml/AppList.qml
        qml/AppListDelegate.qml
        qml/AppGrid.qml
        qml/AppDelegate.qml
        qml/GlassBackground.qml
        theme_selector/SelectorMain.qml
        theme_selector/ColorPickerRow.qml
        theme_selector/PresetButton.qml
)

target_link_libraries(nexus-launcher
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::DBus
    Qt6::QuickControls2
    LayerShellQt::Interface
)

# 2. Separate Theme Selector Executable
qt_add_executable(nexus-theme-selector
    src/main_selector.cpp
    src/thememanager.cpp
    src/thememanager.h
)

qt_add_qml_module(nexus-theme-selector
    URI com.nexus.themeselector
    VERSION 1.0
    QML_FILES
        theme_selector/SelectorMain.qml
        theme_selector/ColorPickerRow.qml
        theme_selector/PresetButton.qml
)

target_link_libraries(nexus-theme-selector
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::QuickControls2
)

install(TARGETS nexus-launcher nexus-theme-selector
    RUNTIME DESTINATION /usr/local/bin
)
