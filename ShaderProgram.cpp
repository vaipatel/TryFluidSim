#include "ShaderProgram.h"
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

const QMap<ShaderProgram::enVSAttrIdx, ShaderProgram::VSAttrData> ShaderProgram::s_VS_ATTR_NAMES =
{
    {VS_POS,       {"VertexPosition",  3}}, // {x, y, z}
    {VS_COLOR,     {"VertexColor",     4}}, // {r, g, b, a}
    {VS_TEXCOORDS, {"VertexTexCoords", 2}}  // {u, v}
};

ShaderProgram::ShaderProgram(const QString& _vertexShaderFileName, const QString& _fragmentShaderFileName)
{
    m_program = new QOpenGLShaderProgram;

    bool canAddShaders = m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, _vertexShaderFileName);
    canAddShaders &= m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, _fragmentShaderFileName);
    Q_ASSERT(canAddShaders);

    foreach (enVSAttrIdx attrIdx, s_VS_ATTR_NAMES.keys())
    {
        m_program->bindAttributeLocation(s_VS_ATTR_NAMES[attrIdx].m_attrName.toUtf8().constData(), static_cast<int>(attrIdx));
    }

    bool canLink = m_program->link();
    Q_ASSERT(canLink);
}

ShaderProgram::~ShaderProgram()
{
    delete m_program;
}

unsigned int ShaderProgram::GetAttributeLocation(const QString& _attrName) const
{
    int attrLocInt = m_program->attributeLocation(_attrName);
    Q_ASSERT(attrLocInt > -1);
    unsigned int attrLoc = static_cast<unsigned int>(attrLocInt);
    return attrLoc;
}

unsigned int ShaderProgram::GetUniformLocation(const QString& _uniformName) const
{
    int uniformLocInt = m_program->uniformLocation(_uniformName);
    Q_ASSERT(uniformLocInt > -1);
    unsigned int uniformLoc = static_cast<unsigned int>(uniformLocInt);
    return uniformLoc;
}

void ShaderProgram::SetAttribute(const QString &_name, float _value)
{
    unsigned int loc = GetAttributeLocation(_name);
    SetAttribute(loc, _value);
}

void ShaderProgram::SetAttribute(unsigned int _loc, float _value)
{
    m_program->setAttributeValue(static_cast<int>(_loc), _value);
}

void ShaderProgram::SetUniform(const QString &_name, const QMatrix4x4 &_value)
{
    unsigned int loc = GetUniformLocation(_name);
    SetUniform(loc, _value);
}

void ShaderProgram::SetUniform(const QString &_name, int _value)
{
    unsigned int loc = GetUniformLocation(_name);
    SetUniform(loc, _value);
}

void ShaderProgram::SetUniform(const QString &_name, float _value)
{
    unsigned int loc = GetUniformLocation(_name);
    SetUniform(loc, _value);
}

void ShaderProgram::SetUniform(unsigned int _loc, const QMatrix4x4 &_value)
{
    m_program->setUniformValue(static_cast<int>(_loc), _value);
}

void ShaderProgram::SetUniform(unsigned int _loc, int _value)
{
    m_program->setUniformValue(static_cast<int>(_loc), _value);
}

void ShaderProgram::SetUniform(unsigned int _loc, float _value)
{
    m_program->setUniformValue(static_cast<int>(_loc), _value);
}

void ShaderProgram::Bind()
{
    bool canBind = m_program->bind();
    Q_ASSERT(canBind);
}

void ShaderProgram::Release()
{
    m_program->release();
}
