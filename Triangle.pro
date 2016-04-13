QT += gui core

CONFIG += c++11

TARGET = Triangle
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    Triangle.cpp \
    vertex.cpp \
    vertexcol.cpp \
    vertextex.cpp

HEADERS += \
    Triangle.h \
    vertex.h \
    vertexcol.h \
    vertextex.h

OTHER_FILES += \
    fshader.txt \
    vshader.txt \

RESOURCES += \
    shaders.qrc
