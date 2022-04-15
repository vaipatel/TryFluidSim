QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += thirdparty/DSPFilters/include
#INCLUDEPATH += thirdparty/DSPFilters/include/DspFilters
#INCLUDEPATH += thirdparty/DSPFilters/source

SOURCES += \
    Blitter.cpp \
    DoubleRenderTargetBuffer.cpp \
    FluidEqualizerWindow.cpp \
    FluidSimWindow.cpp \
    ImageTextureWindow.cpp \
    OpenGLWindow.cpp \
    PointsObject.cpp \
    RenderTargetBuffer.cpp \
    ShaderProgram.cpp \
    Shared.cpp \
    SpectrogramWindow.cpp \
    Texture.cpp \
    TriangleRotWindow.cpp \
    TrisObject.cpp \
    main.cpp \
    thirdparty/DSPFilters/source/Bessel.cpp \
    thirdparty/DSPFilters/source/Biquad.cpp \
    thirdparty/DSPFilters/source/Butterworth.cpp \
    thirdparty/DSPFilters/source/Cascade.cpp \
    thirdparty/DSPFilters/source/ChebyshevI.cpp \
    thirdparty/DSPFilters/source/ChebyshevII.cpp \
    thirdparty/DSPFilters/source/Custom.cpp \
    thirdparty/DSPFilters/source/Design.cpp \
    thirdparty/DSPFilters/source/Documentation.cpp \
    thirdparty/DSPFilters/source/Elliptic.cpp \
    thirdparty/DSPFilters/source/Filter.cpp \
    thirdparty/DSPFilters/source/Legendre.cpp \
    thirdparty/DSPFilters/source/Param.cpp \
    thirdparty/DSPFilters/source/PoleFilter.cpp \
    thirdparty/DSPFilters/source/RBJ.cpp \
    thirdparty/DSPFilters/source/RootFinder.cpp \
    thirdparty/DSPFilters/source/State.cpp

HEADERS += \
    Blitter.h \
    DoubleRenderTargetBuffer.h \
    FluidEqualizerWindow.h \
    FluidSimWindow.h \
    ImageTextureWindow.h \
    OpenGLWindow.h \
    PointsObject.h \
    RenderTargetBuffer.h \
    ShaderProgram.h \
    Shared.h \
    SpectrogramWindow.h \
    Texture.h \
    TriangleRotWindow.h \
    TrisObject.h \
    thirdparty/DSPFilters/include/DspFilters/Bessel.h \
    thirdparty/DSPFilters/include/DspFilters/Biquad.h \
    thirdparty/DSPFilters/include/DspFilters/Butterworth.h \
    thirdparty/DSPFilters/include/DspFilters/Cascade.h \
    thirdparty/DSPFilters/include/DspFilters/ChebyshevI.h \
    thirdparty/DSPFilters/include/DspFilters/ChebyshevII.h \
    thirdparty/DSPFilters/include/DspFilters/Common.h \
    thirdparty/DSPFilters/include/DspFilters/Custom.h \
    thirdparty/DSPFilters/include/DspFilters/Design.h \
    thirdparty/DSPFilters/include/DspFilters/Dsp.h \
    thirdparty/DSPFilters/include/DspFilters/Elliptic.h \
    thirdparty/DSPFilters/include/DspFilters/Filter.h \
    thirdparty/DSPFilters/include/DspFilters/Layout.h \
    thirdparty/DSPFilters/include/DspFilters/Legendre.h \
    thirdparty/DSPFilters/include/DspFilters/MathSupplement.h \
    thirdparty/DSPFilters/include/DspFilters/Params.h \
    thirdparty/DSPFilters/include/DspFilters/PoleFilter.h \
    thirdparty/DSPFilters/include/DspFilters/RBJ.h \
    thirdparty/DSPFilters/include/DspFilters/RootFinder.h \
    thirdparty/DSPFilters/include/DspFilters/SmoothedFilter.h \
    thirdparty/DSPFilters/include/DspFilters/State.h \
    thirdparty/DSPFilters/include/DspFilters/Types.h \
    thirdparty/DSPFilters/include/DspFilters/Utilities.h

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
    Resources/Shaders/FluidVectors.frag \
    Resources/Shaders/FluidVectors.geom \
    Resources/Shaders/FluidVectors.vert \
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
