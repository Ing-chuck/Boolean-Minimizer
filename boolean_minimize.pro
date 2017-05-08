QT += core
QT -= gui

CONFIG += c++11

TARGET = boolean_minimize
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    minimizer.cpp

HEADERS += \
    minimizer.h
