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
    void SetupTriangle();
    void DrawPerlinNoiseOnQuad();
    void DrawTriangle();

    const QString m_perlinVertShaderFileName = ":/Resources/Shaders/Perlin.vert";
    const QString m_perlinFragShaderFileName = ":/Resources/Shaders/Perlin.frag";
    const QString m_perlinNoiseImgFileName = ":/Resources/Images/perlin_noise_texture-500x500.png";
    const QString m_redPepperImgFileName = ":/Resources/Images/red_pepper_strip_2.png";
    const QString m_rotTexturedTriVertShaderFileName = ":/Resources/Shaders/RotTexturedTri.vert";
    const QString m_rotTexturedTriFragShaderFileName = ":/Resources/Shaders/RotTexturedTri.frag";

    // Convenience for blitting to framebuffer of choice
    Blitter* m_blitter = nullptr;

    // Input textures
    Texture* m_perlinNoiseTexture = nullptr;
    Texture* m_redPepperTexture = nullptr;

    // Perlin stage
    RenderTargetBuffer* m_perlinTargetBuffer = nullptr;
    Texture* m_perlinOutTexture = nullptr;
    ShaderProgram* m_perlinProgram = nullptr;
    TrisObject* m_quad = nullptr;    

    // Triangle stage
    RenderTargetBuffer* m_triTargetBuffer = nullptr;
    Texture* m_triOutTexture = nullptr;
    ShaderProgram* m_triangleProgram; //!< Need one of these for each step of fluid sim
    TrisObject* m_tri = nullptr;

    float m_timeS = 0.f;
};

#endif // IMAGETEXTUREWINDOW_H
