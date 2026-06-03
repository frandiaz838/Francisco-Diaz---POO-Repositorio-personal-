cmake_minimum_required(VERSION 3.16)
project(CoordenadasBD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Forzar UTF-8 en fuente y ejecucion para que tildes y ñ se vean correctas
if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

set(SOURCES
    src/main.cpp
    src/Pantalla.h
    src/Modelos.h
    src/Logger.h
    src/Logger.cpp
    src/Config.h
    src/Config.cpp
    src/BaseDatos.h
    src/BaseDatos.cpp
    src/Login.h
    src/Login.cpp
    src/Login.ui
    src/Pintura.h
    src/Pintura.cpp
    src/Ventana.h
    src/Ventana.cpp
)

add_executable(CoordenadasBD WIN32 ${SOURCES})

target_link_libraries(CoordenadasBD
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
)

# Copiar config.ini al directorio de salida en cada build
add_custom_command(TARGET CoordenadasBD POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/config.ini"
            "$<TARGET_FILE_DIR:CoordenadasBD>/config.ini"
    COMMENT "Copiando config.ini al directorio de salida...")

if(WIN32)
    find_program(WINDEPLOYQT_EXE NAMES windeployqt windeployqt.exe
        HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXE)
        add_custom_command(TARGET CoordenadasBD POST_BUILD
            COMMAND "${WINDEPLOYQT_EXE}"
                    --no-translations
                    --no-system-d3d-compiler
                    "$<TARGET_FILE:CoordenadasBD>"
            COMMENT "Desplegando DLLs de Qt (incluye plugin sqldrivers/qsqlite)...")
    endif()
endif()
