#include "TriangleRotWindow.h"
#include "ImageTextureWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    // If core profile desired, ensure to not use deprecated features.
    // For example, you must use VAOs.
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 2);
    format.setSamples(16);

#if 0
    TriangleRotWindow triWindow;
    triWindow.setFormat(format);
    triWindow.resize(480, 360);
    triWindow.show();
    triWindow.setAnimating(true);
#else
    ImageTextureWindow imgTexWindow;
    imgTexWindow.setFormat(format);
    imgTexWindow.resize(500, 500);
    imgTexWindow.show();
    imgTexWindow.setAnimating(true);
#endif

    return app.exec();
}
