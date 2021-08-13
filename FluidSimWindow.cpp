#include "FluidSimWindow.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QWindow>

FluidSimWindow::FluidSimWindow(QWindow* _parent) : OpenGLWindow(_parent),
    m_targetFBOProgram(nullptr),
    m_frame(0)
{

}

void FluidSimWindow::initialize()
{
    m_targetFBOProgram = new QOpenGLShaderProgram(this);
    m_targetFBOProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, targetFBOVertexShaderSource);
    m_targetFBOProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, targetFBOFragmentShaderSource);
    m_targetFBOProgram->link();
    m_posAttr = static_cast<GLuint>(m_targetFBOProgram->attributeLocation("posAttr"));
    m_colAttr = static_cast<GLuint>(m_targetFBOProgram->attributeLocation("colAttr"));
    m_matrixUniform = static_cast<GLuint>(m_targetFBOProgram->uniformLocation("matrix"));

    m_screenProgram = new QOpenGLShaderProgram(this);
    m_screenProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, screenVertexShaderSource);
    m_screenProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, screenFragmentShaderSource);
    bool canLinkScreenProg = m_screenProgram->link();
    if ( !canLinkScreenProg )
    {
        exit(1);
    }
    m_screenTextureHandle = static_cast<GLuint>(m_screenProgram->uniformLocation("screenTexture"));
}

void FluidSimWindow::render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    const GLsizei retinaScale = static_cast<GLsizei>(devicePixelRatio());

    // Vai: Calc viewport width/height
    GLsizei viewWidth = width() * retinaScale;
    GLsizei viewHeight = height() * retinaScale;

    glViewport(0, 0, viewWidth, viewHeight);

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

    //
    // 4. Also we might apparently need a render buffer because we have no depth
    //
    // -----
    GLuint renderBuffer;
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewWidth, viewHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    // Always check that our framebuffer is ok
    {
        bool frameBufferStatusOk = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        if ( !frameBufferStatusOk )
        {
            qDebug() << "VAIVAI" << "FrameBuffer status not ok";
            exit(1);
        }
    }

    // Set the drawBuffers
    extraFuncs->glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Unbind here and give back to default FBO. Is this just for good practice here? I'll have to rebind to targetFBO now.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    // 4. First Pass
    //
    // -----
    // first pass
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glClearColor(1.f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
//    glEnable(GL_DEPTH_TEST);
    DrawTargetFBO();

    //
    // 5. Second Pass
    //
    // -----
    // Unbind here and give back to default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, viewWidth, viewHeight);


    bool couldBindShader = m_screenProgram->bind();
    if ( !couldBindShader )
    {
        qDebug() << "VAIVAI" << "Could not bind screen shader";
        exit(1);
    }

//    m_screenProgram->setUniformValue(static_cast<int>(m_screenTextureHandle), targetTexture);

    GLuint quadVAO = 0;
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

//    static const GLfloat g_quad_vertex_buffer_data[] = {
//        -1.0f, -1.0f, 0.0f,
//        1.0f, -1.0f, 0.0f,
//        -1.0f,  1.0f, 0.0f,
//        -1.0f,  1.0f, 0.0f,
//        1.0f, -1.0f, 0.0f,
//        1.0f,  1.0f, 0.0f,
//    };

//    GLuint quad_vertexbuffer;
//    glGenBuffers(1, &quad_vertexbuffer);
//    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    GLuint quadVerticesbuffer;
    glGenBuffers(1, &quadVerticesbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadVerticesbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(1);

    glBindVertexArray(quadVAO);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_screenProgram->release();
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteFramebuffers(1, &targetFBO);

    ++m_frame;
//    if (m_frame >= 2)
//    {
//        m_frame = 0;
    //    }
}

void FluidSimWindow::DrawTargetFBO()
{
    bool couldBindShader = m_targetFBOProgram->bind();
    assert(couldBindShader);

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    matrix.rotate(static_cast<float>(100.0 * m_frame / screen()->refreshRate()), 0, 1, 0);
    m_targetFBOProgram->setUniformValue(static_cast<int>(m_matrixUniform), matrix);

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

    glEnableVertexAttribArray(m_posAttr);
    glEnableVertexAttribArray(m_colAttr);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(m_posAttr);
    glDisableVertexAttribArray(m_colAttr);

    m_targetFBOProgram->release();
}
