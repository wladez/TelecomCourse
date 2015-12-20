QT += core
QT -= gui

TARGET = project_t
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    user.cpp \
    server.cpp

DISTFILES += \
    us.txt \
    money.txt

HEADERS += \
    user.h \
    server.h

