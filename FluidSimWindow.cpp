#include "FluidSimWindow.h"
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
    const GLsizei retinaScale = static_cast<GLsizei>(devicePixelRatio());
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    matrix.rotate(static_cast<float>(100.0 * m_frame / screen()->refreshRate()), 0, 1, 0);

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

    m_program->release();

    ++m_frame;
    if (m_frame >= 2)
    {
        m_frame = 0;
    }
}
