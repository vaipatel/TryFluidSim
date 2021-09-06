#include "TriangleRotWindow.h"
#include "ImageTextureWindow.h"
#include "FluidSimWindow.h"
#include <QApplication>

#define TRIROT 0
#define PERLIN 1
#define FLUID 2
#define EXPERIMENT_NAME FLUID


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    // If core profile desired, ensure to not use deprecated features.
    // For example, you must use VAOs.
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 2);
    format.setSamples(16);

#if EXPERIMENT_NAME == TRIROT
    TriangleRotWindow triWindow;
    triWindow.setFormat(format);
    triWindow.resize(480, 360);
    triWindow.show();
    triWindow.setAnimating(true);
#endif

#if EXPERIMENT_NAME == PERLIN
    ImageTextureWindow imgTexWindow;
    imgTexWindow.setFormat(format);
    imgTexWindow.resize(500, 500);
    imgTexWindow.show();
    imgTexWindow.setAnimating(true);
#endif

#if EXPERIMENT_NAME == FLUID
    FluidSimWindow fluidSimWindow;
    fluidSimWindow.setFormat(format);
    fluidSimWindow.resize(500, 500);
    fluidSimWindow.show();
    fluidSimWindow.setAnimating(true);
#endif

    return app.exec();
}
