#include "Blitter.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QOpenGLExtraFunctions>

Blitter::Blitter()
{
    m_screenProgram = new ShaderProgram(m_blitToScreenQuadVertShaderFileName, m_blitToScreenQuadFragShaderFileName);

    SetupScreenQuad();
}

Blitter::~Blitter()
{
    CleanUp();
}

void Blitter::BlitToTarget(RenderTargetBuffer* _renderTarget)
{
    BindTarget(_renderTarget);
    DrawScreenQuad();
}

void Blitter::BindTarget(RenderTargetBuffer *_renderTarget)
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    if ( _renderTarget )
    {
        // Draw to target
        _renderTarget->Bind();
    }
    else
    {
        // Blit to screen
        extraFuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        extraFuncs->glDisable(GL_DEPTH_TEST); // Disable depth test so screen space quad is not discarded due to depth test
        extraFuncs->glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        extraFuncs->glClear(GL_COLOR_BUFFER_BIT);
    }
}

void Blitter::DrawTextureOnScreenQuad(const Texture* _texture, size_t _texIdx)
{
    // Bind texture at m_handles[1] to context
    _texture->Bind(_texIdx);

    m_screenProgram->Bind();
    int texUnitId = static_cast<int>(_texture->GetUnitId(_texIdx));
    m_screenProgram->SetUniform("screenTexture", texUnitId); // Bind texture unit GL_TEXTURE0 + texUnitId of bound texture as uniform

    m_quad->Draw();

    m_screenProgram->Release();
}

void Blitter::BlitToScreen(const Texture* _textureToBlit, size_t _texIdx)
{
    // Blit texture to screen
    BindTarget(nullptr);
    DrawTextureOnScreenQuad(_textureToBlit, _texIdx);
}

void Blitter::DrawScreenQuad()
{
    m_quad->Draw();
}

void Blitter::CleanUp()
{
    if ( m_screenProgram )
    {
        delete m_screenProgram;
        m_screenProgram = nullptr;
    }

    if ( m_quad )
    {
        delete m_quad;
        m_quad = nullptr;
    }
}

void Blitter::SetupScreenQuad()
{
    std::vector<Tri> quadVertices = {
             // positions          // colors                 // texCoords
        {
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
        },
        {
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        }
    };
    m_quad = new TrisObject(quadVertices);
}
