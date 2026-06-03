cmake_minimum_required(VERSION 3.16)
project(TRexExtremo LANGUAGES CXX)

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

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    src/main.cpp
    src/Obstaculo.h
    src/Obstaculo.cpp
    src/Cactus.h
    src/Cactus.cpp
    src/Pajaro.h
    src/Pajaro.cpp
    src/TRex.h
    src/TRex.cpp
    src/Juego.h
    src/Juego.cpp
)

add_executable(TRexExtremo WIN32 ${SOURCES})

target_link_libraries(TRexExtremo
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

if(WIN32)
    find_program(WINDEPLOYQT_EXE NAMES windeployqt windeployqt.exe
        HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXE)
        add_custom_command(TARGET TRexExtremo POST_BUILD
            COMMAND "${WINDEPLOYQT_EXE}"
                    --no-translations
                    --no-system-d3d-compiler
                    "$<TARGET_FILE:TRexExtremo>"
            COMMENT "Desplegando DLLs de Qt...")
    endif()
endif()
