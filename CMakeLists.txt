cmake_minimum_required(VERSION 3.16)

project(quasar VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui DBus QuickControls2 Concurrent Svg Test Qml)

find_package(LayerShellQt 6.0 QUIET)
if(LayerShellQt_FOUND AND DEFINED LayerShellQt_VERSION AND LayerShellQt_VERSION VERSION_LESS "6.0")
    message(STATUS "Found LayerShellQt ${LayerShellQt_VERSION} but Qt6 requires >= 6.0. Disabling layer-shell support.")
    set(LayerShellQt_FOUND FALSE)
endif()
set(QUASAR_COMMON_SOURCES
    src/platform.cpp
    src/platform.h
    src/logging.cpp
    src/logging.h
)

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
    src/actionmodel.cpp
    src/actionmodel.h
    src/calculator.cpp
    src/calculator.h
    src/emojimanager.cpp
    src/emojimanager.h
    src/clipboardmanager.cpp
    src/clipboardmanager.h
    ${QUASAR_COMMON_SOURCES}
)

if(LayerShellQt_FOUND)
    target_compile_definitions(quasar PRIVATE QUASAR_HAVE_LAYERSHELL)
endif()

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
    RESOURCES
        assets/presets.json
        assets/emoji.json
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
    Qt6::Qml
)

if(LayerShellQt_FOUND)
    target_link_libraries(quasar PRIVATE LayerShellQt::Interface)
else()
    message(WARNING "LayerShellQt not found: building without layer-shell support (X11/fallback mode)")
endif()

# 2. Separate Theme Selector Executable
qt_add_executable(quasar-theme-selector
    src/main_selector.cpp
    src/thememanager.cpp
    src/thememanager.h
    ${QUASAR_COMMON_SOURCES}
)

qt_add_qml_module(quasar-theme-selector
    URI com.quasar.themeselector
    VERSION 1.0
    QML_FILES
        qml/theme_selector/SelectorMain.qml
        qml/theme_selector/ColorPickerRow.qml
        qml/theme_selector/PresetButton.qml
    RESOURCES
        assets/presets.json
)

target_link_libraries(quasar-theme-selector
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::QuickControls2
    Qt6::DBus
)

# The theme selector is a screen-sized transparent overlay; on Wayland that
# is only possible as a layer-shell surface (xdg toplevels get tiled and
# lose alpha).
if(LayerShellQt_FOUND)
    target_compile_definitions(quasar-theme-selector PRIVATE QUASAR_HAVE_LAYERSHELL)
    target_link_libraries(quasar-theme-selector PRIVATE LayerShellQt::Interface)
endif()

install(TARGETS quasar quasar-theme-selector
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES assets/quasar.desktop assets/quasar-theme-selector.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES assets/icons/quasar-icon.svg RENAME quasar.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
install(FILES assets/icons/quasar-mono-white.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/status
)
install(FILES assets/icons/quasar-mono-black.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/status
)
install(FILES assets/presets.json assets/emoji.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/quasar
)
install(FILES assets/actions.json.example
    DESTINATION ${CMAKE_INSTALL_DATADIR}/quasar
)
install(FILES assets/scripts/calc.sh assets/scripts/clipboard.sh assets/scripts/session.sh
    DESTINATION ${CMAKE_INSTALL_DATADIR}/quasar/scripts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# Defines the BUILD_TESTING option (ON by default) and enables ctest.
include(CTest)
if(BUILD_TESTING)
qt_add_executable(test_fuzzymatcher
    tests/test_fuzzymatcher.cpp
    src/fuzzymatcher.cpp
    src/fuzzymatcher.h
    src/appindexer.cpp
    src/appindexer.h
    src/frecencyranker.cpp
    src/frecencyranker.h
    src/thememanager.cpp
    src/thememanager.h
    src/windowswitcher.cpp
    src/windowswitcher.h
    src/actionmodel.cpp
    src/actionmodel.h
    src/calculator.cpp
    src/calculator.h
    src/emojimanager.cpp
    src/emojimanager.h
    src/clipboardmanager.cpp
    src/clipboardmanager.h
    ${QUASAR_COMMON_SOURCES}
)
target_link_libraries(test_fuzzymatcher PRIVATE Qt6::Core Qt6::Gui Qt6::Test Qt6::Concurrent Qt6::Qml)
add_test(NAME test_fuzzymatcher COMMAND test_fuzzymatcher)

qt_add_executable(test_thememanager
    tests/test_thememanager.cpp
    src/thememanager.cpp
    src/thememanager.h
    ${QUASAR_COMMON_SOURCES}
)
target_link_libraries(test_thememanager PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
add_test(NAME test_thememanager COMMAND test_thememanager)

qt_add_executable(test_actionmodel
    tests/test_actionmodel.cpp
    src/actionmodel.cpp
    src/actionmodel.h
    ${QUASAR_COMMON_SOURCES}
)
target_link_libraries(test_actionmodel PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
add_test(NAME test_actionmodel COMMAND test_actionmodel)
endif()
