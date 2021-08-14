#include "FluidSimWindow.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QWindow>
#include <QtGlobal>

FluidSimWindow::FluidSimWindow(QWindow* _parent) : OpenGLWindow(_parent),
    m_triangleProgram(nullptr)
{

}

FluidSimWindow::~FluidSimWindow()
{
}

void FluidSimWindow::initialize()
{
    glEnable(GL_DEPTH_TEST);

    m_triangleProgram = new QOpenGLShaderProgram(this);
    m_triangleProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, targetFBOVertexShaderSource);
    m_triangleProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, targetFBOFragmentShaderSource);
    m_triangleProgram->link();
    m_posAttr = static_cast<GLuint>(m_triangleProgram->attributeLocation("posAttr"));
    m_colAttr = static_cast<GLuint>(m_triangleProgram->attributeLocation("colAttr"));
    m_matrixUniform = static_cast<GLuint>(m_triangleProgram->uniformLocation("matrix"));

    m_screenProgram = new QOpenGLShaderProgram(this);
    bool canLinkScreenProg = m_screenProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, screenVertexShaderSource);
    canLinkScreenProg &= m_screenProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, screenFragmentShaderSource);
    canLinkScreenProg &= m_screenProgram->link();
    Q_ASSERT(canLinkScreenProg);

    m_screenTextureLoc = static_cast<GLuint>(m_screenProgram->uniformLocation("screenTexture"));
    Q_ASSERT(m_screenTextureLoc >= 0); // -1 is bad

    SetupRenderTargetFBO();
    SetupScreenQuad();
    SetupTriangle();
}

void FluidSimWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    QPair<int, int> viewWidthAndHeight = CalcViewPortWidthHeight();
    GLsizei viewWidth = viewWidthAndHeight.first;
    GLsizei viewHeight = viewWidthAndHeight.second;

    glViewport(0, 0, viewWidth, viewHeight);

    //
    // 4. First Pass
    //
    // -----
    // first pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO);
//    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);//| GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

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
    glViewport(0, 0, viewWidth, viewHeight);

    bool couldBindShader = m_screenProgram->bind();
    if ( !couldBindShader )
    {
        qDebug() << "VAIVAI" << "Could not bind screen shader";
        exit(1);
    }

    m_screenProgram->setUniformValue(static_cast<int>(m_screenTextureLoc), 0);
    extraFuncs->glBindVertexArray(m_quadVAO);
//    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_targetTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_screenProgram->release();
//    glDisableVertexAttribArray(0);
//    glDisableVertexAttribArray(1);

    ++m_frame;
}

void FluidSimWindow::cleanup()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    glDeleteFramebuffers(1, &m_targetFBO);
    extraFuncs->glDeleteVertexArrays(1, &m_quadVAO);
    extraFuncs->glDeleteBuffers(1, &m_quadVBO);
}

void FluidSimWindow::DrawRotatingTriangle()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    bool couldBindShader = m_triangleProgram->bind();
    Q_ASSERT(couldBindShader);

    QPair<int, int> viewWidthAndHeight = CalcViewPortWidthHeight();
    GLsizei viewWidth = viewWidthAndHeight.first;
    GLsizei viewHeight = viewWidthAndHeight.second;

    QMatrix4x4 matrix;
//    matrix.perspective(60.0f, viewWidth/viewHeight, 0.1f, 100.0f);
//    matrix.translate(0, 0, -4);
    matrix.rotate(static_cast<float>(100.0 * m_frame / screen()->refreshRate()), 0, 1, 0);
    m_triangleProgram->setUniformValue(static_cast<int>(m_matrixUniform), matrix);

    extraFuncs->glBindVertexArray(m_triVAO);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    m_triangleProgram->release();

    extraFuncs->glBindVertexArray(0);
}

void FluidSimWindow::SetupRenderTargetFBO()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    QPair<int, int> viewWidthAndHeight = CalcViewPortWidthHeight();
    GLsizei viewWidth = viewWidthAndHeight.first;
    GLsizei viewHeight = viewWidthAndHeight.second;

    //
    // 1. The framebuffer is our container for our target texture
    //
    // -----
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &m_targetFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO);
    // -----

    //
    // 2. Let's generate and bind the actual target texture
    //
    // -----
    // The texture we're going to render to
    glGenTextures(1, &m_targetTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
//    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_targetTexture);

    // Give an empty image to OpenGL ( the last nullptr )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewWidth, viewHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // -----

    //
    // 3. Marry framebuffer and target texture by giving target texture as color attachment to framebuffer
    //
    // -----
    // Set "renderedTexture" as our colour attachement #0
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[0], GL_TEXTURE_2D, m_targetTexture, 0);
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
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glGenVertexArrays(1, &m_triVAO);
    extraFuncs->glBindVertexArray(m_triVAO);

    float triVertices[] = {
        // verts      // colors
        0.0f, 0.707f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.0f, 0.0f, 1.0f
    };

    glGenBuffers(1, &m_triVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_triVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), &triVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(m_posAttr);
    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(m_colAttr);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
}

void FluidSimWindow::SetupScreenQuad()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glGenVertexArrays(1, &m_quadVAO);
    extraFuncs->glBindVertexArray(m_quadVAO);

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f*0.9f,  1.0f*0.9f,  0.0f*0.9f, 1.0f,
        -1.0f*0.9f, -1.0f*0.9f,  0.0f*0.9f, 0.0f,
         1.0f*0.9f, -1.0f*0.9f,  1.0f*0.9f, 0.0f,

        -1.0f*0.9f,  1.0f*0.9f,  0.0f*0.9f, 1.0f,
         1.0f*0.9f, -1.0f*0.9f,  1.0f*0.9f, 0.0f,
         1.0f*0.9f,  1.0f*0.9f,  1.0f*0.9f, 1.0f
    };

    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
}

QPair<int, int> FluidSimWindow::CalcViewPortWidthHeight()
{
    const GLsizei retinaScale = static_cast<GLsizei>(devicePixelRatio());

    // Vai: Calc viewport width/height
    GLsizei viewWidth = width() * retinaScale;
    GLsizei viewHeight = height() * retinaScale;

    return {viewWidth, viewHeight};
}
