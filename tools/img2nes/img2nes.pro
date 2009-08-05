TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
win32: CONFIG += console
mac:CONFIG -= app_bundle

# Input
SOURCES += img2nes.cpp
