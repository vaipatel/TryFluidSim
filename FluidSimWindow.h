#ifndef FLUIDSIMWINDOW_H
#define FLUIDSIMWINDOW_H

#include "OpenGLWindow.h"
#include <QPair>

class QOpenGLShaderProgram;
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
    GLuint m_matrixUniform;

    const QString m_sampleTriangleVertShaderFileName = ":/Resources/Shaders/SampleTriangle.vert";
    const QString m_sampleTriangleFragShaderFileName = ":/Resources/Shaders/SampleTriangle.frag";


    GLuint m_screenTextureLoc;

    const QString m_blitToScreenQuadVertShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.vert";
    const QString m_blitToScreenQuadFragShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.frag";


    void DrawRotatingTriangle();

    QOpenGLShaderProgram* m_triangleProgram; //!< Need one of these for each step of fluid sim
    QOpenGLShaderProgram* m_screenProgram;
    uint m_frame = 0;
    GLuint m_targetFBO = 0;
    GLuint m_targetTexture = 0;
    GLuint m_triVAO = 0;
    GLuint m_triVBO = 0;
    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;
    int m_viewWidth = 0;
    int m_viewHeight = 0;
    float m_viewAspect = 0;

    void SetupRenderTargetFBO();
    void SetupTriangle();
    void SetupScreenQuad();
    void DrawScreenQuad(GLuint _targetTextureHandle);
    void CleanUpRenderTargetFBO();
    QPair<int, int> CalcViewPortWidthHeight() const;
    void UpdateViewPortIfNeeded();
};

#endif // FLUIDSIMWINDOW_H
