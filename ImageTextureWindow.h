#ifndef IMAGETEXTUREWINDOW_H
#define IMAGETEXTUREWINDOW_H

#include "OpenGLWindow.h"

class Texture;
class QWindow;

class ImageTextureWindow : public OpenGLWindow
{
public:
    ImageTextureWindow(QWindow* _parent = nullptr);
    ~ImageTextureWindow();

protected:
    void initialize() override;
    void HandleViewPortUpdated() override;

private:
    const QString m_imageFileName = ":/Resources/Images/Moscow_traffic_congestion.JPG";

    Texture* m_texture = nullptr;
};

#endif // IMAGETEXTUREWINDOW_H
