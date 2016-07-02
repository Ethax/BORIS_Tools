#-------------------------------------------------
#
# Project created by QtCreator 2016-06-28T22:30:00
#
#-------------------------------------------------

QT += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FakeBORIS
TEMPLATE = app

SOURCES += main.cpp mainwindow.cpp serialcomm.cpp
HEADERS += mainwindow.h serialcomm.h
FORMS += mainwindow.ui
win32:RC_ICONS += main_icon.ico

CONFIG += static
