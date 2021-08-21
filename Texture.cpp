#include "Texture.h"
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QString>

Texture::Texture(int _width, int _height, GLenum _format, GLenum _type, const char* _data) :
    m_width(_width),
    m_height(_height),
    m_format(_format),
    m_type(_type)
{
    Q_ASSERT_X(_format != GL_NONE, __FUNCTION__, "Texture format cannot be none.");
    Q_ASSERT_X(_type != GL_NONE, __FUNCTION__, "Texture type cannot be none.");

    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();

    //
    // 2. Let's generate and bind the actual target texture
    //
    // -----
    // The texture we're going to render to
    extraFuncs->glGenTextures(1, &m_textureLoc);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    extraFuncs->glActiveTexture(GL_TEXTURE0 + GetId()); // GetId() is 0 for single texture, but I'm just doing this for my own clarity
    extraFuncs->glBindTexture(GL_TEXTURE_2D, m_textureLoc);

    m_internalFormat = CalcInternalFormat();
    extraFuncs->glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_width, m_height, 0, m_format, m_type, _data);

    // Poor filtering. Needed !
    extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    extraFuncs->glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
    if ( QOpenGLContext::currentContext() )
    {
        qDebug() << "Deleting texture" << m_textureLoc;
        CleanUp();
    }
}

void Texture::Bind()
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glActiveTexture(GL_TEXTURE0 + GetId()); // GetId() is 0 for single texture, but I'm just doing this for my own clarity
    extraFuncs->glBindTexture(GL_TEXTURE_2D, m_textureLoc);
}

void Texture::CleanUp()
{
    if ( QOpenGLContext::currentContext() )
    {
        QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
        if ( extraFuncs )
        {
            extraFuncs->glDeleteTextures(1, &m_textureLoc);
        }
    }
}

int Texture::CalcInternalFormat() const
{
    int internalFormat = GL_NONE;

    bool failed = false;
    switch(m_type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    {
        switch(m_format)
        {
        case GL_RED:
        case GL_RGB:
        case GL_RGBA:
            internalFormat = static_cast<GLint>(m_format);
            break;
        default:
            failed = true;
            break;
        }
        break;
    }
    case GL_FLOAT:
    {
        switch(m_format)
        {
        case GL_RED:
            internalFormat = GL_R32F;
            break;
        case GL_RGB:
            internalFormat = GL_RGB32F;
            break;
        case GL_RGBA:
            internalFormat = GL_RGBA32F;
            break;
        default:
            failed = true;
            break;
        }
        break;
    }
    default:
        failed = true;
        break;
    }

    QString msgIfFailed = QString("Could not deduce Texture internal format for format %1 using type %2")
                          .arg(m_format, 4, 16, QChar('0')).arg(m_type, 4, 16, QChar('0'));
    Q_ASSERT_X(!failed, __FUNCTION__, msgIfFailed.toUtf8().constData());

    return internalFormat;
}
