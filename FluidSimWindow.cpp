#include "FluidSimWindow.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QWindow>
#include <QtGlobal>

FluidSimWindow::FluidSimWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

FluidSimWindow::~FluidSimWindow()
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

void FluidSimWindow::initialize()
{
    glEnable(GL_DEPTH_TEST);

    m_triangleProgram = new ShaderProgram(m_sampleTriangleVertShaderFileName, m_sampleTriangleFragShaderFileName);
    m_screenProgram = new ShaderProgram(m_blitToScreenQuadVertShaderFileName, m_blitToScreenQuadFragShaderFileName);

    UpdateViewPortIfNeeded();
    SetupRenderTargetFBO();
    SetupScreenQuad();
    SetupTriangle();
}

void FluidSimWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Handle window resizing
    UpdateViewPortIfNeeded();

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

void FluidSimWindow::cleanup()
{
    CleanUpRenderTargetFBO();
    m_quad->CleanUp();
    m_tri->CleanUp();
}

void FluidSimWindow::CleanUpRenderTargetFBO()
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

void FluidSimWindow::SetupRenderTargetFBO()
{
    m_targetTexture = new Texture({{m_viewWidth, m_viewHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr},
                                   {m_viewWidth, m_viewHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr}});

    m_renderTargetBuffer = new RenderTargetBuffer(m_targetTexture);

    m_renderTargetBuffer->SetDepthTestEnabled(false);
    m_renderTargetBuffer->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    m_renderTargetBuffer->SetClearColor({38, 38, 38, 255});
}

void FluidSimWindow::SetupTriangle()
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

void FluidSimWindow::SetupScreenQuad()
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


void FluidSimWindow::DrawRotatingTriangle()
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
void FluidSimWindow::DrawScreenQuad()
{
    // Bind texture at m_handles[1] to context
    m_targetTexture->Bind(1);

    m_screenProgram->Bind();
    m_screenProgram->SetUniform("screenTexture", static_cast<int>(m_targetTexture->GetId(1))); // Bind texture unit GL_TEXTURE0 + 1 of bound texture as uniform

    m_quad->Draw();

    m_screenProgram->Release();
}

QPair<int, int> FluidSimWindow::CalcViewPortWidthHeight() const
{
    const GLsizei retinaScale = static_cast<GLsizei>(devicePixelRatio());

    // Vai: Calc viewport width/height
    GLsizei viewWidth = width() * retinaScale;
    GLsizei viewHeight = height() * retinaScale;

    return {viewWidth, viewHeight};
}

///
/// \brief Updates the viewport width and height if necessary. If updated, this function additionally
///        updates the aspect ratio value, calls glViewport() and recreates the render target FBOs.
///
void FluidSimWindow::UpdateViewPortIfNeeded()
{
    QPair<int, int> viewWidthAndHeight = CalcViewPortWidthHeight();
    int viewWidth = viewWidthAndHeight.first;
    int viewHeight = viewWidthAndHeight.second;

    if ( viewWidth != m_viewWidth || viewHeight != m_viewHeight )
    {
        // Update width, height and aspect
        m_viewWidth = viewWidth;
        m_viewHeight = viewHeight;
        m_viewAspect = static_cast<float>(m_viewWidth)/static_cast<float>(m_viewHeight);

        // Update viewport
        glViewport(0, 0, m_viewWidth, m_viewHeight);

        // Recreate render layer (right now just cleans up and remakes the FBO + texture combo)
        if ( m_targetTexture )
        {
            CleanUpRenderTargetFBO();
            SetupRenderTargetFBO();
        }
    }
}
