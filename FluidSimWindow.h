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
    // Convenience for blitting to framebuffer of choice
    Blitter* m_blitter = nullptr;

    // Input textures
    Texture* m_perlinNoiseTexture = nullptr;
    Texture* m_redPepperTexture = nullptr;

    RenderTargetBuffer* m_velocityTargetBufferA = nullptr;
    Texture* m_velocityOutTextureA = nullptr;
    RenderTargetBuffer* m_velocityTargetBufferB = nullptr;
    Texture* m_velocityOutTextureB = nullptr;
    DoubleRenderTargetBuffer* m_velocityDoubleTargetBuffer = nullptr;

//    Texture* m_perlinOutTexture = nullptr;
//    ShaderProgram* m_perlinProgram = nullptr;
//    TrisObject* m_quad = nullptr;

//    // Triangle stage
//    RenderTargetBuffer* m_triTargetBuffer = nullptr;
//    Texture* m_triOutTexture = nullptr;
//    ShaderProgram* m_triangleProgram; //!< Need one of these for each step of fluid sim
//    TrisObject* m_tri = nullptr;

    float m_texelSizeX = 0.0f;
    float m_texelSizeY = 0.0f;
};

#endif // FLUIDSIMWINDOW_H
