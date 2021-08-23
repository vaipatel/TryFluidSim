#ifndef IMAGETEXTUREWINDOW_H
#define IMAGETEXTUREWINDOW_H

#include "OpenGLWindow.h"

class Blitter;
class Texture;
class QWindow;

class ImageTextureWindow : public OpenGLWindow
{
public:
    ImageTextureWindow(QWindow* _parent = nullptr);
    ~ImageTextureWindow() override;

protected:
    void initialize() override;
    void render() override;
    void cleanup() override;
    void HandleViewPortUpdated() override;

private:
    void CleanUpTexture();
    void SetupTexture();

    const QString m_imageFileName = ":/Resources/Images/Moscow_traffic_congestion.JPG";

    Texture* m_texture = nullptr;
    Blitter* m_blitter = nullptr;
};

#endif // IMAGETEXTUREWINDOW_H
