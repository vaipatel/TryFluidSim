#ifndef TRIANGLEROTWINDOW_H
#define TRIANGLEROTWINDOW_H

#include "OpenGLWindow.h"

#include <QPair>

class RenderTargetBuffer;
class ShaderProgram;
class Texture;
class TrisObject;
class QWindow;

class TriangleRotWindow : public OpenGLWindow
{
public:
    TriangleRotWindow(QWindow* _parent = nullptr);
    ~TriangleRotWindow() override;

    void initialize() override;
    void render() override;
    void cleanup() override;

private:
    const QString m_sampleTriangleVertShaderFileName = ":/Resources/Shaders/SampleTriangle.vert";
    const QString m_sampleTriangleFragShaderFileName = ":/Resources/Shaders/SampleTriangle.frag";

    const QString m_blitToScreenQuadVertShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.vert";
    const QString m_blitToScreenQuadFragShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.frag";

    ShaderProgram* m_triangleProgram; //!< Need one of these for each step of fluid sim
    ShaderProgram* m_screenProgram;
    uint m_frame = 0;
    RenderTargetBuffer* m_renderTargetBuffer = nullptr;
    Texture* m_targetTexture = nullptr;
    TrisObject* m_tri = nullptr;
    TrisObject* m_quad = nullptr;
    int m_viewWidth = 0;
    int m_viewHeight = 0;
    float m_viewAspect = 0;

    void CleanUpRenderTargetFBO();
    void SetupRenderTargetFBO();
    void SetupTriangle();
    void SetupScreenQuad();
    void DrawRotatingTriangle();
    void DrawScreenQuad();
    QPair<int, int> CalcViewPortWidthHeight() const;
    void UpdateViewPortIfNeeded();
};

#endif // TRIANGLEROTWINDOW_H
