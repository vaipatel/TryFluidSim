#include "RenderTargetBuffer.h"
#include "Texture.h"
#include <QOpenGLExtraFunctions>
#include <QtGlobal>

RenderTargetBuffer::RenderTargetBuffer(Texture* _targetTexture) :
    m_targetTexture(_targetTexture)
{
    Q_ASSERT(m_targetTexture != nullptr);
    size_t numTextures = m_targetTexture->GetNumTextures();
    Q_ASSERT(numTextures > 0);

    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    extraFuncs->glGenFramebuffers(1, &m_targetFBO);
    extraFuncs->glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO);

    // Is this how we do depth? Or we pass in a texture with some "depth" format? I don't know.
    // But if we do we'll probably have to have GL_NONE attachment in the draw buffers below
//    GLuint renderBuffer;
//    glGenRenderbuffers(1, &renderBuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewWidth, viewHeight);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    std::vector<GLenum> drawBuffers(numTextures);
    for (unsigned int idx = 0; idx < numTextures; ++idx)
    {
        drawBuffers[idx] = GL_COLOR_ATTACHMENT0 + idx;
        unsigned int texHandle = m_targetTexture->GetHandle(idx);
        extraFuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[idx], GL_TEXTURE_2D, texHandle, 0);
    }

    // Set the drawBuffers
    extraFuncs->glDrawBuffers(static_cast<GLsizei>(numTextures), drawBuffers.data());

    // Always check that our framebuffer is ok
    bool frameBufferStatusOk = extraFuncs->glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    Q_ASSERT(frameBufferStatusOk);

    // Unbind here and give back to default FBO. Is this just for good practice here? I'll have to rebind to targetFBO now.
    extraFuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderTargetBuffer::~RenderTargetBuffer()
{
    if ( QOpenGLContext::currentContext() )
    {
        CleanUp();
    }
}

void RenderTargetBuffer::Bind()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO);
    if ( m_depthTestEnabled )
    {
        extraFuncs->glEnable(GL_DEPTH_TEST);
    }

    if ( m_clearMask & GL_COLOR_BUFFER_BIT )
    {
        GLclampf r = static_cast<GLclampf>(m_clearColor.redF());
        GLclampf g = static_cast<GLclampf>(m_clearColor.greenF());
        GLclampf b = static_cast<GLclampf>(m_clearColor.blueF());
        GLclampf a = static_cast<GLclampf>(m_clearColor.alphaF());
        extraFuncs->glClearColor(r, g, b, a);
    }
    if ( m_clearMask != 0 )
    {
        extraFuncs->glClear(m_clearMask);
    }
}

void RenderTargetBuffer::CleanUp()
{
    if ( QOpenGLContext::currentContext() )
    {
        QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
        if ( extraFuncs )
        {
            extraFuncs->glDeleteFramebuffers(1, &m_targetFBO);
            m_targetFBO = 0;
        }
    }
}
