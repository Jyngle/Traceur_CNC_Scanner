QT += core
QT -= gui
QT += serialport

CONFIG += c++11

TARGET = scanner
CONFIG += console
CONFIG -= app_bundle

LIBS += -L/usr/local/lib/ -lwiringPi

TEMPLATE = app

SOURCES += main.cpp \
    grbl.cpp \
    sensor.cpp

HEADERS += \
    grbl.h \
    sensor.h \
    config.h
