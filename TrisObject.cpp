#include "TrisObject.h"
#include "ShaderProgram.h"
#include <QDebug>
#include <QOpenGLExtraFunctions>

TrisObject::TrisObject(const std::vector<Tri>& _tris) :
    m_vao(0),
    m_vbo(0),
    m_numTris(_tris.size())
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glGenVertexArrays(1, &m_vao);
    extraFuncs->glBindVertexArray(m_vao);
    extraFuncs->glGenBuffers(1, &m_vbo);
    extraFuncs->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    extraFuncs->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(_tris.size() * sizeof(Tri)), _tris.data(), GL_STATIC_DRAW);

    GLint sizeOfFloat = static_cast<GLint>(sizeof(float));
    GLint numComponents = 3;
    GLint offset = 0;
    extraFuncs->glEnableVertexAttribArray(ShaderProgram::VS_POS);
    extraFuncs->glVertexAttribPointer(ShaderProgram::VS_POS, numComponents, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offset));

    offset += numComponents * sizeOfFloat;
    numComponents = 4;
    extraFuncs->glEnableVertexAttribArray(ShaderProgram::VS_COLOR);
    extraFuncs->glVertexAttribPointer(ShaderProgram::VS_COLOR, numComponents, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offset));

    offset += numComponents * sizeOfFloat;
    numComponents = 2;
    extraFuncs->glEnableVertexAttribArray(ShaderProgram::VS_TEXCOORDS);
    extraFuncs->glVertexAttribPointer(ShaderProgram::VS_TEXCOORDS, numComponents, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offset));

    extraFuncs->glBindVertexArray(0);
}

TrisObject::~TrisObject()
{
    if ( QOpenGLContext::currentContext() )
    {
        CleanUp();
    }
}

void TrisObject::Draw()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glBindVertexArray(m_vao);
    GLsizei numVerts = static_cast<GLsizei>(m_numTris) * 3;
    extraFuncs->glDrawArrays(GL_TRIANGLES, 0, numVerts);
    extraFuncs->glBindVertexArray(0);
}

void TrisObject::CleanUp()
{
    if ( QOpenGLContext::currentContext() )
    {
        QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
        if ( extraFuncs )
        {
            extraFuncs->glDeleteVertexArrays(1, &m_vao);
            extraFuncs->glDeleteBuffers(1, &m_vbo);
        }
    }
}
