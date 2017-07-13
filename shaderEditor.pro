TEMPLATE = app
TARGET = shaderEditor

DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc
UI_DIR = src

QT += qml quick core gui widgets
#include(Tools/CommonLibs/defines.pri)

INCLUDEPATH += /usr/local/include/\
                MOC_DIR
QMAKE_LIBDIR += /usr/local/lib
LIBS += -l KF5SyntaxHighlighting


SOURCES += src/main.cpp \
    src/ShaderEditor.cpp \
    src/RenderWidget.cpp \
    src/ShaderEditorApp.cpp \
    src/openglwindow.cpp \
    src/UIShaderEditor.cpp

HEADERS += \
    src/ShaderEditor.h \
    src/RenderWidget.h \
    src/ShaderEditorApp.h \
    src/openglwindow.h \
    src/UIShaderEditor.h

RESOURCES += \
    assets.qrc

FORMS += \
    src/UIShaderEditor.ui

