QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DESTDIR = $$PWD/bin
TARGET = ARCS_Tool
TEMPLATE = app

# 添加应用程序图标
win32:RC_FILE = app_icon.rc

# 添加SmartPower库支持
LIBS += -L$$PWD/lib -lSmartPower
INCLUDEPATH += $$PWD/lib

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    smartpowercontroller.cpp \
    testthread.cpp \
    blu_serial.cpp \
    blu_protocol.cpp \
    adbcontroller.cpp

HEADERS += \
    mainwindow.h \
    smartpowercontroller.h \
    testthread.h \
    blu_serial.h \
    blu_protocol.h \
    adbcontroller.h
