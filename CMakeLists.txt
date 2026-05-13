cmake_minimum_required(VERSION 3.22)

project(Whiteboard
    VERSION 0.2.0
    DESCRIPTION "Совместная онлайн-доска"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp

    src/ui/MainWindow.cpp
    src/ui/ConnectionDialog.cpp

    src/data/BoardModel.cpp

    src/canvas/CanvasView.cpp
)

set(HEADERS
    src/ui/MainWindow.h
    src/ui/ConnectionDialog.h

    src/data/DrawObject.h
    src/data/BoardModel.h

    src/canvas/CanvasView.h

    src/tools/DrawTool.h
    src/tools/PencilTool.h
    src/tools/ShapeTool.h
    src/tools/EraserTool.h

    src/network/VectorClock.h
    src/network/PacketType.h
)

qt_add_executable(whiteboard
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(whiteboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(whiteboard PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS whiteboard
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
