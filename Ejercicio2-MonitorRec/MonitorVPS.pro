QT += core gui network widgets

CONFIG += c++11

TARGET = MonitorVPS
TEMPLATE = app

SOURCES += main.cpp \
           MonitorController.cpp \
           MainWindow.cpp

HEADERS += MonitorController.h \
           MainWindow.h