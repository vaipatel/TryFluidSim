#ifndef FLUIDSIMWINDOW_H
#define FLUIDSIMWINDOW_H

#include "OpenGLWindow.h"

class Blitter;
class DoubleRenderTargetBuffer;
class RenderTargetBuffer;
class ShaderProgram;
class Texture;
class TrisObject;
class QWindow;

class FluidSimWindow : public OpenGLWindow
{
public:
    FluidSimWindow(QWindow* _parent = nullptr);
    ~FluidSimWindow() override;

protected:
    void initialize() override;
    void render() override;
    void cleanup() override;
    void HandleViewPortUpdated() override;

private:
    void CleanUpTextures();
    void SetupTextures();
    void SetupQuad();
    void Advect(DoubleRenderTargetBuffer* _doubleBuffer, Texture *_velTex, float _dt);

    const QString m_baseVertShaderFileName = ":/Resources/Shaders/FluidBaseVertexShader.vert";
    const QString m_advectFragShaderFileName = ":/Resources/Shaders/FluidAdvect.frag";
    const QString m_splatForceFragShaderFileName = ":/Resources/Shaders/FluidSplatForce.frag";
    const QString m_perlinNoiseImgFileName = ":/Resources/Images/perlin_noise_texture-500x500.png";
    const QString m_moscowImgFileName = ":/Resources/Images/Moscow_traffic_congestion.JPG";

    // Convenience for blitting to framebuffer of choice
    Blitter* m_blitter = nullptr;

    // Input textures
    Texture* m_velocityOutTextureA = nullptr;
    Texture* m_velocityOutTextureB = nullptr;
    DoubleRenderTargetBuffer* m_velocityDoubleTargetBuffer = nullptr;
    Texture* m_uVelocityInputTexture = nullptr;
    Texture* m_uSourceInputTexture = nullptr;
    Texture* m_dyeTextureA = nullptr;
    Texture* m_dyeTextureB = nullptr;
    DoubleRenderTargetBuffer* m_dyeDoubleTargetBuffer = nullptr;


    ShaderProgram* m_advectProgram = nullptr;
    ShaderProgram* m_splatForceProgram = nullptr;

    TrisObject* m_quad = nullptr;

//    // Triangle stage
//    RenderTargetBuffer* m_triTargetBuffer = nullptr;
//    Texture* m_triOutTexture = nullptr;
//    ShaderProgram* m_triangleProgram; //!< Need one of these for each step of fluid sim
//    TrisObject* m_tri = nullptr;

    float m_texelSizeX = 0.0f;
    float m_texelSizeY = 0.0f;
};

#endif // FLUIDSIMWINDOW_H
