#include "PointsObject.h"
#include "ShaderProgram.h"
#include <QDebug>
#include <QList>
#include <QOpenGLExtraFunctions>

PointsObject::PointsObject(const std::vector<Point>& _points) :
    m_vao(0),
    m_vbo(0),
    m_numPoints(_points.size())
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glGenVertexArrays(1, &m_vao);
    extraFuncs->glBindVertexArray(m_vao);
    extraFuncs->glGenBuffers(1, &m_vbo);
    extraFuncs->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    extraFuncs->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(_points.size() * sizeof(Point)), _points.data(), GL_STATIC_DRAW);

    QList<ShaderProgram::enVSAttrIdx> attrIdxVec = {ShaderProgram::VS_POS};
    size_t offset = 0;
    for (int i = 0; i < attrIdxVec.size(); ++i)
    {
        ShaderProgram::enVSAttrIdx attrIdx = attrIdxVec[i];
        GLint numComponents = ShaderProgram::s_VS_ATTR_NAMES[attrIdx].m_numComponents;
        extraFuncs->glEnableVertexAttribArray(attrIdx);
        extraFuncs->glVertexAttribPointer(attrIdx, numComponents, GL_FLOAT, GL_FALSE, sizeof(Point), reinterpret_cast<void*>(offset));
        offset += static_cast<size_t>(numComponents) * sizeof(float);
    }

    extraFuncs->glBindVertexArray(0);
}

PointsObject::~PointsObject()
{
    if ( QOpenGLContext::currentContext() )
    {
        CleanUp();
    }
}

void PointsObject::Draw()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glBindVertexArray(m_vao);
    GLsizei numVerts = static_cast<GLsizei>(m_numPoints);
    extraFuncs->glDrawArrays(GL_POINTS, 0, numVerts);
    extraFuncs->glBindVertexArray(0);
}

void PointsObject::CleanUp()
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
