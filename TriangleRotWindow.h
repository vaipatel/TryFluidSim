#ifndef TRIANGLEROTWINDOW_H
#define TRIANGLEROTWINDOW_H

#include "OpenGLWindow.h"

#include <QPair>

class Blitter;
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

protected:
    void initialize() override;
    void render() override;
    void cleanup() override;
    void HandleViewPortUpdated() override;

private:
    const QString m_sampleTriangleVertShaderFileName = ":/Resources/Shaders/SampleTriangle.vert";
    const QString m_sampleTriangleFragShaderFileName = ":/Resources/Shaders/SampleTriangle.frag";

    ShaderProgram* m_triangleProgram; //!< Need one of these for each step of fluid sim
    uint m_frame = 0;
    RenderTargetBuffer* m_renderTargetBuffer = nullptr;
    Texture* m_targetTexture = nullptr;
    TrisObject* m_tri = nullptr;
    Blitter* m_blitter = nullptr;

    void CleanUpRenderTargetFBO();
    void SetupRenderTargetFBO();
    void SetupTriangle();
    void DrawRotatingTriangle();
};

#endif // TRIANGLEROTWINDOW_H
