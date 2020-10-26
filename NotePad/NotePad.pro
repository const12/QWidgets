#-------------------------------------------------
#
# Project created by QtCreator 2020-08-21T10:06:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NotePad
TEMPLATE = app


SOURCES += main.cpp\
        notepad.cpp \
    finddialog.cpp

HEADERS  += notepad.h \
    finddialog.h

FORMS    += notepad.ui \
    finddialog.ui

RESOURCES += \
    images.qrc
