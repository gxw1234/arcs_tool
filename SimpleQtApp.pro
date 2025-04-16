QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DESTDIR = $$PWD/bin
TARGET = SimpleQtApp
TEMPLATE = app

# 添加SmartPower库支持
LIBS += -L$$PWD/lib -lSmartPower
INCLUDEPATH += $$PWD/lib

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    smartpowercontroller.cpp

HEADERS += \
    mainwindow.h \
    smartpowercontroller.h


