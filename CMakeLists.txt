cmake_minimum_required(VERSION 3.16)

project(nexus-launcher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui DBus)
find_package(LayerShellQt REQUIRED)

qt_standard_project_setup()

qt_add_executable(nexus-launcher
    src/main.cpp
    src/appindexer.cpp
    src/appindexer.h
    src/fuzzymatcher.cpp
    src/fuzzymatcher.h
    src/frecencyranker.cpp
    src/frecencyranker.h
)

qt_add_qml_module(nexus-launcher
    URI com.nexus.launcher
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/SearchBar.qml
        qml/AppGrid.qml
        qml/AppDelegate.qml
        qml/GlassBackground.qml
)

target_link_libraries(nexus-launcher
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::DBus
    LayerShellQt::Interface
)

install(TARGETS nexus-launcher
    RUNTIME DESTINATION /usr/local/bin
)
