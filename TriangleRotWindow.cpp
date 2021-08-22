#include "TriangleRotWindow.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QWindow>
#include <QtGlobal>

TriangleRotWindow::TriangleRotWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

TriangleRotWindow::~TriangleRotWindow()
{
    delete m_tri;
    delete m_quad;

    if ( m_renderTargetBuffer )
    {
        delete m_renderTargetBuffer;
    }

    if ( m_targetTexture )
    {
        delete m_targetTexture;
    }
}

void TriangleRotWindow::initialize()
{
    glEnable(GL_DEPTH_TEST);

    m_triangleProgram = new ShaderProgram(m_sampleTriangleVertShaderFileName, m_sampleTriangleFragShaderFileName);
    m_screenProgram = new ShaderProgram(m_blitToScreenQuadVertShaderFileName, m_blitToScreenQuadFragShaderFileName);

    SetupRenderTargetFBO();
    SetupScreenQuad();
    SetupTriangle();
}

void TriangleRotWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Render to texture
    m_renderTargetBuffer->Bind();
    DrawRotatingTriangle();

    // Blit texture to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // Disable depth test so screen space quad is not discarded due to depth test
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    DrawScreenQuad();

    ++m_frame;
}

void TriangleRotWindow::cleanup()
{
    CleanUpRenderTargetFBO();
    m_quad->CleanUp();
    m_tri->CleanUp();
}

void TriangleRotWindow::HandleViewPortUpdated()
{
    // Update viewport
    glViewport(0, 0, m_viewWidth, m_viewHeight);

    // Recreate render layer (right now just cleans up and remakes the FBO + texture combo)
    if ( m_targetTexture )
    {
        CleanUpRenderTargetFBO();
        SetupRenderTargetFBO();
    }
}

void TriangleRotWindow::CleanUpRenderTargetFBO()
{
    if ( m_renderTargetBuffer )
    {
        delete m_renderTargetBuffer;
        m_renderTargetBuffer = nullptr;
    }

    if ( m_targetTexture )
    {
        delete m_targetTexture;
        m_targetTexture = nullptr;
    }
}

void TriangleRotWindow::SetupRenderTargetFBO()
{
    m_targetTexture = new Texture({{m_viewWidth, m_viewHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr},
                                   {m_viewWidth, m_viewHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr}});

    m_renderTargetBuffer = new RenderTargetBuffer(m_targetTexture);

    m_renderTargetBuffer->SetDepthTestEnabled(false);
    m_renderTargetBuffer->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    m_renderTargetBuffer->SetClearColor({38, 38, 38, 255});
}

void TriangleRotWindow::SetupTriangle()
{
    std::vector<Tri> triVertices = {
             // positions            // colors                 // texCoords
        {
            {{ 0.0f,  0.707f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f,   0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,   0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
        }
    };
    m_tri = new TrisObject(triVertices);
}

void TriangleRotWindow::SetupScreenQuad()
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


void TriangleRotWindow::DrawRotatingTriangle()
{
    m_triangleProgram->Bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, m_viewAspect, 0.1f, 100.0f);
    matrix.translate(0, 0, -4);
    float angle = static_cast<float>(100.0 * m_frame / screen()->refreshRate());
    matrix.rotate(angle, 0, 1, 0);
    m_triangleProgram->SetUniform("matrix", matrix);
    m_triangleProgram->SetUniform("angle", angle);

    m_tri->Draw();

    m_triangleProgram->Release();
}

///
/// \brief FluidSimWindow::DrawScreenQuad
///
void TriangleRotWindow::DrawScreenQuad()
{
    // Bind texture at m_handles[1] to context
    m_targetTexture->Bind(1);

    m_screenProgram->Bind();
    m_screenProgram->SetUniform("screenTexture", static_cast<int>(m_targetTexture->GetId(1))); // Bind texture unit GL_TEXTURE0 + 1 of bound texture as uniform

    m_quad->Draw();

    m_screenProgram->Release();
}
