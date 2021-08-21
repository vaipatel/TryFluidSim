#include "Texture.h"
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QString>

Texture::Texture(int _width, int _height, GLenum _format, GLenum _type, const char* _data) :
    Texture({{_width, _height, _format, _type, _data}})
{

}

Texture::Texture(const std::vector<TextureData>& _dataForTextures)
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    m_numTextures = _dataForTextures.size();
    Q_ASSERT_X(m_numTextures > 0, __FUNCTION__, "Data for textures was empty. Cannot generate 0 textures.");
    extraFuncs->glGenTextures(m_numTextures, &m_textureLoc);
    extraFuncs->glBindTexture(GL_TEXTURE_2D, m_textureLoc);

    for (size_t textureIdx = 0; textureIdx < m_numTextures; ++textureIdx)
    {
        const TextureData& dataAtIdx = _dataForTextures[textureIdx];
        Q_ASSERT_X(dataAtIdx.m_format != GL_NONE, __FUNCTION__, QString("Texture format for texture %1 cannot be none.").arg(textureIdx).toUtf8().constData());
        Q_ASSERT_X(dataAtIdx.m_type != GL_NONE, __FUNCTION__, QString("Texture type for texture %1 cannot be none.").arg(textureIdx).toUtf8().constData());

        StoredTextureData storedDataAtIdx;
        storedDataAtIdx.m_id = textureIdx;
        storedDataAtIdx.m_width = dataAtIdx.m_width;
        storedDataAtIdx.m_height = dataAtIdx.m_height;
        storedDataAtIdx.m_format = dataAtIdx.m_format;
        storedDataAtIdx.m_type = dataAtIdx.m_type;
        storedDataAtIdx.m_internalFormat = CalcInternalFormat(storedDataAtIdx.m_format, storedDataAtIdx.m_type);

        extraFuncs->glActiveTexture(GL_TEXTURE0 + storedDataAtIdx.m_id);
        extraFuncs->glTexImage2D(GL_TEXTURE_2D,
                                 0,
                                 storedDataAtIdx.m_internalFormat,
                                 storedDataAtIdx.m_width,
                                 storedDataAtIdx.m_height,
                                 0,
                                 storedDataAtIdx.m_format,
                                 storedDataAtIdx.m_type,
                                 dataAtIdx.data);

        // Poor filtering. Needed !
        extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        m_storedTextureData.push_back(storedDataAtIdx);
    }

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

void Texture::Bind(size_t _textureIdx)
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    extraFuncs->glActiveTexture(GL_TEXTURE0 + GetId(_textureIdx));
    extraFuncs->glBindTexture(GL_TEXTURE_2D, m_textureLoc);
}

void Texture::CleanUp()
{
    if ( QOpenGLContext::currentContext() )
    {
        QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
        if ( extraFuncs )
        {
            extraFuncs->glDeleteTextures(m_numTextures, &m_textureLoc);
            m_storedTextureData.clear();
        }
    }
}

int Texture::CalcInternalFormat(GLenum _format, GLenum _type) const
{
    int internalFormat = GL_NONE;

    bool failed = false;
    switch(_type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    {
        switch(_format)
        {
        case GL_RED:
        case GL_RGB:
        case GL_RGBA:
            internalFormat = static_cast<GLint>(_format);
            break;
        default:
            failed = true;
            break;
        }
        break;
    }
    case GL_FLOAT:
    {
        switch(_format)
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
                          .arg(_format, 4, 16, QChar('0')).arg(_type, 4, 16, QChar('0'));
    Q_ASSERT_X(!failed, __FUNCTION__, msgIfFailed.toUtf8().constData());

    return internalFormat;
}
