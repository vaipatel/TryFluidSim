QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    OpenGLWindow.cpp \
    RenderTargetBuffer.cpp \
    ShaderProgram.cpp \
    Texture.cpp \
    TriangleRotWindow.cpp \
    TrisObject.cpp \
    main.cpp

HEADERS += \
    OpenGLWindow.h \
    RenderTargetBuffer.h \
    ShaderProgram.h \
    Texture.h \
    TriangleRotWindow.h \
    TrisObject.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Resources/Shaders/SampleTriangle.vert \
    Resources/Shaders/SampleTriangle.frag \
    Resources/Shaders/BlitToScreenQuad.vert \
    Resources/Shaders/BlitToScreenQuad.frag

RESOURCES += \
    Resources.qrc
