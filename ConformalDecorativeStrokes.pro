#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T10:11:57
#
#-------------------------------------------------

QT       += core gui opengl svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ConformalDecorativeStrokes
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    GLContainer.cpp \
    GLWidget.cpp \
    StrokePainter.cpp \
    UtilityFunctions.cpp \
    SystemParams.cpp \
    CurveRDP.cpp \
    VertexDataHelper.cpp

HEADERS  += mainwindow.h \
    AVector.h \
    GLContainer.h \
    GLWidget.h \
    VertexData.h \
    ALine.h \
    StrokePainter.h \
    UtilityFunctions.h \
    SystemParams.h \
    CurveRDP.h \
    PlusSignVertex.h \
    VertexDataHelper.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -frounding-math -O3

QMAKE_CXXFLAGS += -std=gnu++1y
