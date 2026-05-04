cmake_minimum_required(VERSION 3.16)
project(LoginClima LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(SOURCES
    src/main.cpp
    src/Pantalla.h
    src/Modelos.h
    src/Logger.h
    src/Logger.cpp
    src/Config.h
    src/Config.cpp
    src/ApiClima.h
    src/ApiClima.cpp
    src/Clima.h
    src/Clima.cpp
    src/Login.h
    src/Login.cpp
    src/Login.ui
    src/Ventana.h
    src/Ventana.cpp
)

add_executable(LoginClima WIN32 ${SOURCES})

target_link_libraries(LoginClima
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)

# Copiar config.ini en cada build (no solo al configurar CMake)
add_custom_command(TARGET LoginClima POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/config.ini"
            "$<TARGET_FILE_DIR:LoginClima>/config.ini"
    COMMENT "Copiando config.ini al directorio de salida...")

if(WIN32)
    find_program(WINDEPLOYQT_EXE NAMES windeployqt windeployqt.exe
        HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXE)
        add_custom_command(TARGET LoginClima POST_BUILD
            COMMAND "${WINDEPLOYQT_EXE}"
                    --no-translations
                    --no-system-d3d-compiler
                    "$<TARGET_FILE:LoginClima>"
            COMMENT "Desplegando DLLs de Qt...")
    endif()
endif()
