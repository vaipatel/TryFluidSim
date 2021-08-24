#ifndef IMAGETEXTUREWINDOW_H
#define IMAGETEXTUREWINDOW_H

#include "OpenGLWindow.h"

class Blitter;
class RenderTargetBuffer;
class ShaderProgram;
class Texture;
class TrisObject;
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
    void SetupQuad();

    const QString m_perlinVertShaderFileName = ":/Resources/Shaders/Perlin.vert";
    const QString m_perlinFragShaderFileName = ":/Resources/Shaders/Perlin.frag";
    const QString m_perlinNoiseImgFileName = ":/Resources/Images/perlin_noise_texture-500x500.png";
    const QString m_redPepperImgFileName = ":/Resources/Images/red_pepper_strip.png";

    Texture* m_perlinNoiseTexture = nullptr;
    Texture* m_redPepperTexture = nullptr;
    Blitter* m_blitter = nullptr;
    RenderTargetBuffer* m_renderTargetBuffer = nullptr;
    Texture* m_targetTexture = nullptr;
    ShaderProgram* m_perlinProgram = nullptr;
    TrisObject* m_quad = nullptr;
    float m_timeS = 0.f;
};

#endif // IMAGETEXTUREWINDOW_H
