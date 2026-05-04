QT += core gui widgets network websockets

TEMPLATE = app
TARGET = CollabDraw

CONFIG += c++17

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    DrawingModel.cpp \
    DrawingView.cpp \
    SyncService.cpp \
    StrokeTypes.cpp

HEADERS += \
    MainWindow.h \
    DrawingModel.h \
    DrawingView.h \
    SyncService.h \
    StrokeTypes.h