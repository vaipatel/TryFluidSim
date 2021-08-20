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
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, _tris.size() * sizeof(Tri), _tris.data(), GL_STATIC_DRAW);
    size_t numComponents = 3;
    size_t offset = 0;
    glEnableVertexAttribArray(ShaderProgram::VS_POS);
    glVertexAttribPointer(ShaderProgram::VS_POS, numComponents, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offset));

    offset += numComponents * sizeof(float);
    numComponents = 4;
    glEnableVertexAttribArray(ShaderProgram::VS_COLOR);
    glVertexAttribPointer(ShaderProgram::VS_COLOR, numComponents, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offset));

    offset += numComponents * sizeof(float);
    numComponents = 2;
    glEnableVertexAttribArray(ShaderProgram::VS_TEXCOORDS);
    glVertexAttribPointer(ShaderProgram::VS_TEXCOORDS, numComponents, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offset));

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
    glDrawArrays(GL_TRIANGLES, 0, m_numTris*3);
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
