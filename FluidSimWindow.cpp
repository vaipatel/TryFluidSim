#include "FluidSimWindow.h"
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

    if ( m_targetTexture )
    {
        delete m_targetTexture;
        m_targetTexture = nullptr;
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

    //
    // 4. First Pass
    //
    // -----
    // first pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

    DrawRotatingTriangle();

    //
    // 5. Second Pass
    //
    // -----
    // Unbind here and give back to default FBO
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
    if ( m_targetFBO != 0 )
    {
        glDeleteFramebuffers(1, &m_targetFBO);
        // This grossness is hopefully only needed till I have a proper RenderTexture/RenderLayer class.
        // Calling cleanup on that should cleanup the texture (but will I have just moved the problem?)
        if ( m_targetTexture )
        {
            delete m_targetTexture;
            m_targetTexture = nullptr;
        }
    }
}

void FluidSimWindow::SetupRenderTargetFBO()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    //
    // 1. The framebuffer is our container for our target texture
    //
    // -----
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &m_targetFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO);
    // -----

    //
    // 2. Generate the target texture
    //
    // -----
    m_targetTexture = new Texture(m_viewWidth, m_viewHeight, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    //
    // 3. Marry framebuffer and target texture by giving target texture as color attachment to framebuffer
    //
    // -----
    // Set target texture as our colour attachement #0
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[0], GL_TEXTURE_2D, m_targetTexture->GetTextureLoc(), 0);
    // Set the drawBuffers
    extraFuncs->glDrawBuffers(1, drawBuffers); // "1" is the size of DrawBuffers

//    //
//    // 4. Also we might apparently need a render buffer because we have no depth
//    //
//    // -----
//    GLuint renderBuffer;
//    glGenRenderbuffers(1, &renderBuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewWidth, viewHeight);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    // Always check that our framebuffer is ok
    {
        bool frameBufferStatusOk = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        if ( !frameBufferStatusOk )
        {
            qDebug() << "VAIVAI" << "FrameBuffer status not ok";
            exit(1);
        }
    }

    // Unbind here and give back to default FBO. Is this just for good practice here? I'll have to rebind to targetFBO now.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    matrix.rotate(static_cast<float>(100.0 * m_frame / screen()->refreshRate()), 0, 1, 0);
    m_triangleProgram->SetUniform("matrix", matrix);

    m_tri->Draw();

    m_triangleProgram->Release();
}

///
/// \brief FluidSimWindow::DrawScreenQuad
///
void FluidSimWindow::DrawScreenQuad()
{
    m_targetTexture->Bind();

    m_screenProgram->Bind();
    m_screenProgram->SetUniform("screenTexture", static_cast<int>(m_targetTexture->GetId()));

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
