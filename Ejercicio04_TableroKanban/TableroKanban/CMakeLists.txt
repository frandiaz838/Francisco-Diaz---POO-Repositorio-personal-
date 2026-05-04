cmake_minimum_required(VERSION 3.16)
project(TableroKanban LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(SOURCES
    src/main.cpp
    src/Modelos.h
    src/ModeloKanban.h
    src/ModeloKanban.cpp
    src/ApiKanban.h
    src/ApiKanban.cpp
    src/DialogoTarjeta.h
    src/DialogoTarjeta.cpp
    src/TarjetaWidget.h
    src/TarjetaWidget.cpp
    src/ColumnaWidget.h
    src/ColumnaWidget.cpp
    src/KanbanView.h
    src/KanbanView.cpp
    src/VentanaPrincipal.h
    src/VentanaPrincipal.cpp
)

add_executable(TableroKanban WIN32 ${SOURCES})

target_link_libraries(TableroKanban
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)

if(WIN32)
    find_program(WINDEPLOYQT_EXE NAMES windeployqt windeployqt.exe
        HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXE)
        add_custom_command(TARGET TableroKanban POST_BUILD
            COMMAND "${WINDEPLOYQT_EXE}"
                    --no-translations
                    --no-system-d3d-compiler
                    "$<TARGET_FILE:TableroKanban>"
            COMMENT "Desplegando DLLs de Qt...")
    endif()
endif()
