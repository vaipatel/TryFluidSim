#ifndef FLUIDSIMWINDOW_H
#define FLUIDSIMWINDOW_H

#include "OpenGLWindow.h"

#include <QPair>

class ShaderProgram;
class Texture;
class TrisObject;
class QWindow;

class FluidSimWindow : public OpenGLWindow
{
public:
    FluidSimWindow(QWindow* _parent = nullptr);
    ~FluidSimWindow() override;

    void initialize() override;
    void render() override;
    void cleanup() override;

private:
    GLuint m_posAttr;
    GLuint m_colAttr;

    const QString m_sampleTriangleVertShaderFileName = ":/Resources/Shaders/SampleTriangle.vert";
    const QString m_sampleTriangleFragShaderFileName = ":/Resources/Shaders/SampleTriangle.frag";


    GLuint m_screenTextureLoc;

    const QString m_blitToScreenQuadVertShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.vert";
    const QString m_blitToScreenQuadFragShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.frag";


    void DrawRotatingTriangle();

    ShaderProgram* m_triangleProgram; //!< Need one of these for each step of fluid sim
    ShaderProgram* m_screenProgram;
    uint m_frame = 0;
    GLuint m_targetFBO = 0;
//    GLuint m_targetTexture = 0;
    Texture* m_targetTexture = nullptr;
    TrisObject* m_tri = nullptr;
    TrisObject* m_quad = nullptr;
    int m_viewWidth = 0;
    int m_viewHeight = 0;
    float m_viewAspect = 0;

    void SetupRenderTargetFBO();
    void SetupTriangle();
    void SetupScreenQuad();
    void DrawScreenQuad();
    void CleanUpRenderTargetFBO();
    QPair<int, int> CalcViewPortWidthHeight() const;
    void UpdateViewPortIfNeeded();
};

#endif // FLUIDSIMWINDOW_H
