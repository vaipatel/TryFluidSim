#include "FluidSimWindow.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QWindow>

FluidSimWindow::FluidSimWindow(QWindow* _parent) : OpenGLWindow(_parent),
    m_program(nullptr),
    m_frame(0)
{

}

void FluidSimWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = static_cast<GLuint>(m_program->attributeLocation("posAttr"));
    m_colAttr = static_cast<GLuint>(m_program->attributeLocation("colAttr"));
    m_matrixUniform = static_cast<GLuint>(m_program->uniformLocation("matrix"));
}

void FluidSimWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    const GLsizei retinaScale = static_cast<GLsizei>(devicePixelRatio());

    // Vai: Calc viewport width/height
    GLsizei viewWidth = width() * retinaScale;
    GLsizei viewHeight = height() * retinaScale;

    glViewport(0, 0, viewWidth * retinaScale, viewHeight * retinaScale);

    //
    // 1. The framebuffer is our container for our target texture
    //
    // -----
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    GLuint targetFBO = 0;
    glGenFramebuffers(1, &targetFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    // -----

    //
    // 2. Let's generate and bind the actual target texture
    //
    // -----
    // The texture we're going to render to
    GLuint targetTexture;
    glGenTextures(1, &targetTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, targetTexture);

    // Give an empty image to OpenGL ( the last nullptr )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewWidth, viewHeight, 0,GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // -----

    //
    // 3. Marry framebuffer and target texture by giving target texture as color attachment to framebuffer
    //
    // -----
    // Set "renderedTexture" as our colour attachement #0
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glFramebufferTexture2D(GL_FRAMEBUFFER, DrawBuffers[0], GL_TEXTURE_2D, targetTexture, 0);
    extraFuncs->glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    {
        bool frameBufferStatusOk = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        assert(frameBufferStatusOk);
    }

    bool couldBindShader = m_program->bind();
    assert(couldBindShader);

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
//    matrix.rotate(static_cast<float>(100.0 * m_frame / screen()->refreshRate()), 0, 1, 0);
    m_program->setUniformValue(static_cast<int>(m_matrixUniform), matrix);

    GLfloat vertices[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glDeleteFramebuffers(1, &targetFBO);

    m_program->release();

//    ++m_frame;
//    if (m_frame >= 2)
//    {
//        m_frame = 0;
//    }
}
