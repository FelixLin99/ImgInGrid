QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
    headers/

HEADERS += \
    headers/form_canvas.h \
    headers/mainwindow.h

SOURCES += \
    sources/form_canvas.cpp \
    sources/main.cpp \
    sources/mainwindow.cpp

FORMS += \
    forms/form_canvas.ui \
    forms/mainwindow.ui

RESOURCES += \
    resources/input.qrc
