cmake_minimum_required(VERSION 3.16)

project(quasar VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui DBus QuickControls2 Concurrent Svg Test)
find_package(LayerShellQt REQUIRED)

qt_standard_project_setup()

# 1. Main Launcher Executable
qt_add_executable(quasar
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
    src/windowswitcher.cpp
    src/windowswitcher.h
)

qt_add_qml_module(quasar
    URI com.quasar.launcher
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

target_link_libraries(quasar
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::DBus
    Qt6::QuickControls2
    Qt6::Concurrent
    Qt6::Svg
    LayerShellQt::Interface
)

# 2. Separate Theme Selector Executable
qt_add_executable(quasar-theme-selector
    src/main_selector.cpp
    src/thememanager.cpp
    src/thememanager.h
)

qt_add_qml_module(quasar-theme-selector
    URI com.quasar.themeselector
    VERSION 1.0
    QML_FILES
        theme_selector/SelectorMain.qml
        theme_selector/ColorPickerRow.qml
        theme_selector/PresetButton.qml
)

target_link_libraries(quasar-theme-selector
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::QuickControls2
    LayerShellQt::Interface
)

install(TARGETS quasar quasar-theme-selector
    RUNTIME DESTINATION /usr/local/bin
)

install(FILES assets/icons/quasar-icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME quasar.svg)
install(FILES assets/icons/quasar-mono-white.svg DESTINATION share/icons/hicolor/scalable/status RENAME quasar-mono-white.svg)
install(FILES assets/icons/quasar-mono-black.svg DESTINATION share/icons/hicolor/scalable/status RENAME quasar-mono-black.svg)

enable_testing()
qt_add_executable(test_fuzzymatcher
    tests/test_fuzzymatcher.cpp
    src/fuzzymatcher.cpp
    src/fuzzymatcher.h
    src/appindexer.cpp
    src/appindexer.h
    src/frecencyranker.cpp
    src/frecencyranker.h
    src/windowswitcher.cpp
    src/windowswitcher.h
)
target_link_libraries(test_fuzzymatcher PRIVATE Qt6::Core Qt6::Gui Qt6::Test Qt6::Concurrent)
add_test(NAME test_fuzzymatcher COMMAND test_fuzzymatcher)

