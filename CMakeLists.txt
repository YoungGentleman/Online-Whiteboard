cmake_minimum_required(VERSION 3.22)

project(Whiteboard
    DESCRIPTION "Совместная онлайн-доска"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

add_library(whiteboard_common STATIC
    src/data/DrawObject.h
    src/data/BoardModel.h
    src/data/BoardModel.cpp
    src/data/Serializer.h
    src/data/Serializer.cpp
    src/data/PacketType.h
    src/network/VectorClock.h
)
target_include_directories(whiteboard_common PUBLIC src)
target_link_libraries(whiteboard_common PUBLIC Qt6::Core Qt6::Gui)

add_executable(whiteboard
    src/main.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/ConnectionDialog.h
    src/ui/ConnectionDialog.cpp
    src/canvas/CanvasView.h
    src/canvas/CanvasView.cpp
    src/tools/DrawTool.h
    src/tools/PencilTool.h
    src/tools/ShapeTool.h
    src/tools/EraserTool.h

)
target_link_libraries(whiteboard PRIVATE
    whiteboard_common
    Qt6::Widgets
    Qt6::Network
)

if(WIN32)
    set_target_properties(whiteboard PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(APPLE)
    set_target_properties(whiteboard PROPERTIES MACOSX_BUNDLE TRUE)
endif()
