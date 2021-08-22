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

    ImageTextureWindow window;
    window.setFormat(format);
    window.resize(640, 480);
    window.show();
    window.setAnimating(true);

    return app.exec();
}
