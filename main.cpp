#include "FluidSimWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(4, 1);
    format.setSamples(16);

    FluidSimWindow window;
    window.setFormat(format);
    window.resize(640, 480);
    window.show();
//    window.setAnimating(true);

    return app.exec();
}
