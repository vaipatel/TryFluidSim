#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <QString>

class QMatrix4x4;
class QOpenGLShaderProgram;

class ShaderProgram
{
public:
    ShaderProgram(const QString& _vertexShaderFileName, const QString& _fragmentShaderFileName);
    ~ShaderProgram();

    unsigned int GetAttributeLocation(const QString& _attrName) const;
    unsigned int GetUniformLocation(const QString& _uniformName) const;

    void SetAttribute(const QString& _name, float _value);
    void SetAttribute(unsigned int _loc, float _value);
    void SetUniform(const QString& _name, const QMatrix4x4& _value);
    void SetUniform(const QString& _name, int _value);
    void SetUniform(unsigned int _loc, const QMatrix4x4& _value);
    void SetUniform(unsigned int _loc, int _value);

    void Bind();
    void Release();

private:
    QOpenGLShaderProgram* m_program;
};

#endif // SHADERPROGRAM_H
