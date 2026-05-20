cmake_minimum_required(VERSION 3.16)
project(Whiteboard VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    src/data/PacketType.h
    src/network/VectorClock.h
    src/network/Protocol.h
)
target_include_directories(whiteboard_common PUBLIC src)
target_link_libraries(whiteboard_common PUBLIC Qt6::Core Qt6::Gui Qt6::Network)

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
    src/tools/FillTool.h
    src/network/NetworkManager.h
    src/network/NetworkManager.cpp
    src/server/server.h
    src/server/server.cpp
)
target_link_libraries(whiteboard PRIVATE
    whiteboard_common
    Qt6::Widgets
    Qt6::Network
)

add_executable(whiteboard_server
    src/server/main_server.cpp
    src/server/server.h
    src/server/server.cpp
)
target_link_libraries(whiteboard_server PRIVATE
    whiteboard_common
    Qt6::Network
)

if(WIN32)
    set_target_properties(whiteboard PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
if(APPLE)
    set_target_properties(whiteboard PROPERTIES MACOSX_BUNDLE TRUE)
endif()

include(GNUInstallDirs)
install(TARGETS whiteboard
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE  DESTINATION .
)
install(TARGETS whiteboard_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ── CPack / deb ────────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME            "whiteboard")
set(CPACK_PACKAGE_VERSION         "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT         "whiteboard@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Collaborative whiteboard application")
set(CPACK_PACKAGE_VENDOR          "WhiteboardTeam")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER  "WhiteboardTeam <whiteboard@example.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION     "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY    "optional")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6core6, libqt6gui6, libqt6widgets6, libqt6network6, libxcb-cursor0")

set(CPACK_GENERATOR "DEB")

include(CPack)
