#ifndef FLUIDSIMWINDOW_H
#define FLUIDSIMWINDOW_H

#include "OpenGLWindow.h"
#include <QVector2D>
#include <list>

class Blitter;
class DoubleRenderTargetBuffer;
class PointsObject;
class RenderTargetBuffer;
class ShaderProgram;
class Texture;
class TrisObject;
class QKeyEvent;
class QMouseEvent;
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
    void mouseMoveEvent(QMouseEvent* _ev) override;
    void keyPressEvent(QKeyEvent* _ev) override;

private:
    void CleanUpTextures();
    void SetupTextures();
    void SetupParticles();
    void Advect(DoubleRenderTargetBuffer* _doubleBuffer, float _dt);
    void Splat(float _x, float _y, float _dx, float _dy, const QVector3D& _color);
    void ComputeDivergence();
    void SolvePressure();
    void SubtractGradient();
    void DrawVectors();
    void ConfigureRenderTarget(RenderTargetBuffer* _renderTarget);

    const QString m_baseVertShaderFileName = ":/Resources/Shaders/FluidBaseVertexShader.vert";
    const QString m_advectFragShaderFileName = ":/Resources/Shaders/FluidAdvect.frag";
    const QString m_splatForceFragShaderFileName = ":/Resources/Shaders/FluidSplatForce.frag";
    const QString m_divergenceFragShaderFileName = ":/Resources/Shaders/FluidDivergence.frag";
    const QString m_pressureSolveFragShaderFileName = ":/Resources/Shaders/FluidPressureSolve.frag";
    const QString m_gradientSubtractFragShaderFileName = ":/Resources/Shaders/FluidGradientSubtract.frag";
    const QString m_vectorsVertShaderFileName = ":/Resources/Shaders/FluidVectors.vert";
    const QString m_vectorsGeomShaderFileName = ":/Resources/Shaders/FluidVectors.geom";
    const QString m_vectorsFragShaderFileName = ":/Resources/Shaders/FluidVectors.frag";
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
    Texture* m_divergenceTexture = nullptr;
    RenderTargetBuffer* m_divergenceTargetBuffer = nullptr;
    Texture* m_pressureTextureA = nullptr;
    Texture* m_pressureTextureB = nullptr;
    DoubleRenderTargetBuffer* m_pressureDoubleTargetBuffer = nullptr;

    ShaderProgram* m_advectProgram = nullptr;
    ShaderProgram* m_splatForceProgram = nullptr;
    ShaderProgram* m_divergenceProgram = nullptr;
    ShaderProgram* m_pressureSolveProgram = nullptr;
    ShaderProgram* m_gradientSubtractProgram = nullptr;
    ShaderProgram* m_vectorsProgram = nullptr;

    float m_texelSizeX = 0.0f;
    float m_texelSizeY = 0.0f;
    const int SIM_RES = 512;
    const float SPLAT_FORCE = 80000;
    std::list<QVector2D> m_mousePosList;

    PointsObject* m_vectors;
    bool m_showVectors = false;
};

#endif // FLUIDSIMWINDOW_H
