QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Blitter.cpp \
    DoubleRenderTargetBuffer.cpp \
    FluidSimWindow.cpp \
    ImageTextureWindow.cpp \
    OpenGLWindow.cpp \
    RenderTargetBuffer.cpp \
    ShaderProgram.cpp \
    Shared.cpp \
    Texture.cpp \
    TriangleRotWindow.cpp \
    TrisObject.cpp \
    main.cpp

HEADERS += \
    Blitter.h \
    DoubleRenderTargetBuffer.h \
    FluidSimWindow.h \
    ImageTextureWindow.h \
    OpenGLWindow.h \
    RenderTargetBuffer.h \
    ShaderProgram.h \
    Shared.h \
    Texture.h \
    TriangleRotWindow.h \
    TrisObject.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Resources/Images/perlin_noise_texture-500x500.png \
    Resources/Images/Moscow_traffic_congestion.JPG \
    Resources/Images/red_pepper_strip.png \
    Resources/Images/red_pepper_strip_2.png \
    Resources/Shaders/FluidAdvect.frag \
    Resources/Shaders/FluidBaseVertexShader.vert \
    Resources/Shaders/FluidDivergence.frag \
    Resources/Shaders/FluidGradientSubtract.frag \
    Resources/Shaders/FluidPressureSolve.frag \
    Resources/Shaders/FluidSplatForce.frag \
    Resources/Shaders/Perlin.frag \
    Resources/Shaders/Perlin.vert \
    Resources/Shaders/RotTexturedTri.frag \
    Resources/Shaders/RotTexturedTri.vert \
    Resources/Shaders/SampleTriangle.vert \
    Resources/Shaders/SampleTriangle.frag \
    Resources/Shaders/BlitToScreenQuad.vert \
    Resources/Shaders/BlitToScreenQuad.frag

RESOURCES += \
    Resources.qrc
