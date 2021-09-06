#ifndef RENDERTARGETBUFFER_H
#define RENDERTARGETBUFFER_H

#include <QColor>
#include <qopengl.h>

class Texture;

class RenderTargetBuffer
{
public:
    RenderTargetBuffer(Texture* _targetTexture);
    ~RenderTargetBuffer();

    void Bind();
    void CleanUp();

    unsigned int GetTargetFBO() const { return m_targetFBO; }
    Texture* GetTargetTexture() const { return m_targetTexture; }
    bool GetDepthTestEnabled() const { return m_depthTestEnabled; }
    void SetDepthTestEnabled(bool _enable) { m_depthTestEnabled = _enable; }
    QColor GetClearColor() const { return m_clearColor; }
    void SetClearColor(QColor _color) { m_clearColor = _color; }
    GLbitfield GetClearMask() const { return m_clearMask; }
    void SetClearMask(GLbitfield _mask) { m_clearMask = _mask; }

private:
    Texture* m_targetTexture = nullptr;
    unsigned int m_targetFBO = 0;
    bool m_depthTestEnabled = false;
    QColor m_clearColor;
    GLbitfield m_clearMask = 0x0000;
};

#endif // RENDERTARGETBUFFER_H
