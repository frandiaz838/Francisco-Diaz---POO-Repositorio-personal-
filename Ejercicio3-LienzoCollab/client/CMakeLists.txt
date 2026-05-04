cmake_minimum_required(VERSION 3.16)
project(CollabDraw LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets)

qt_add_executable(CollabDraw
    main.cpp
    MainWindow.cpp
    MainWindow.h
    DrawingModel.cpp
    DrawingModel.h
    DrawingView.cpp
    DrawingView.h
    SyncService.cpp
    SyncService.h
    StrokeTypes.cpp
    StrokeTypes.h
)

target_link_libraries(CollabDraw PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
)

if(MSVC)
    target_compile_options(CollabDraw PRIVATE /W4 /permissive-)
else()
    target_compile_options(CollabDraw PRIVATE -Wall -Wextra -Wpedantic)
endif()
