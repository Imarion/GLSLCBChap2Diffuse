QT += gui core

CONFIG += c++11

TARGET = Diffuse
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    Diffuse.cpp \
    vertex.cpp \
    vertexcol.cpp \
    vertextex.cpp \
    torus.cpp

HEADERS += \
    Diffuse.h \
    vertex.h \
    vertexcol.h \
    vertextex.h \
    torus.h

OTHER_FILES += \
    fshader.txt \
    vshader.txt \
    vshader_diffuse.txt \
    fshader_diffuse.txt \
    fshader_ads.txt \
    vshader_ads.txt

RESOURCES += \
    shaders.qrc

DISTFILES += \
    fshader.txt \
    vshader.txt \
    vshader_diffuse.txt \
    fshader_diffuse.txt \
    fshader_ads.txt \
    vshader_ads.txt
