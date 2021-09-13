#include "Texture.h"
#include <QDebug>
#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QString>

Texture::Texture(int _width, int _height, GLenum _format, GLenum _type, TextureData::FilterParam _filterParam, const char *_data, unsigned int _textureIdOffset)
{
     Construct({{_width, _height, _format, _type, _filterParam, _data}}, _textureIdOffset);
}

Texture::Texture(const std::vector<TextureData>& _dataForTextures, unsigned int _textureIdOffset)
{
    Construct(_dataForTextures, _textureIdOffset);
}

Texture::Texture(const QString& _imageFileName, TextureData::FilterParam _filterParam, unsigned int _textureIdOffset)
{
    QImage image(_imageFileName);
    image = image.convertToFormat(QImage::Format_RGBA8888);
    image = image.mirrored(false, true); // Need to flip vertically because OpenGL origin is bottom left while Qt origin is top left
    GLenum type = GL_UNSIGNED_BYTE;

    const uchar* srcData = image.constBits();
    TextureData texData = {image.width(), image.height(), GL_RGBA, type, _filterParam, srcData};
    Construct({texData}, _textureIdOffset);
}


Texture::~Texture()
{
    if ( QOpenGLContext::currentContext() )
    {
        CleanUp();
    }
}

void Texture::Bind(size_t _textureIdx, size_t* _unitId) const
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    if ( _unitId )
    {
        extraFuncs->glActiveTexture(GL_TEXTURE0 + *_unitId);
    }
    else
    {
        extraFuncs->glActiveTexture(GL_TEXTURE0 + GetUnitId(_textureIdx));
    }
    extraFuncs->glBindTexture(GL_TEXTURE_2D, GetHandle(_textureIdx));
}

void Texture::CleanUp()
{
    if ( QOpenGLContext::currentContext() )
    {
        QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
        if ( extraFuncs )
        {
            extraFuncs->glDeleteTextures(static_cast<GLsizei>(m_numTextures), m_handles.data());
            for ( size_t idx = 0; idx < m_numTextures; idx++ )
            {
                if ( m_storedTextureData[idx].m_data )
                {
                    free(m_storedTextureData[idx].m_data);
                }
            }
            m_storedTextureData.clear();
        }
    }
}

void Texture::Construct(const std::vector<TextureData> &_dataForTextures, unsigned int _textureIdOffset)
{
    QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
    m_numTextures = _dataForTextures.size();
    Q_ASSERT_X(m_numTextures > 0, __FUNCTION__, "Data for textures was empty. Cannot generate 0 textures.");

    m_handles.resize(m_numTextures, 0);
    extraFuncs->glGenTextures(static_cast<GLsizei>(m_numTextures), m_handles.data());

    for (size_t textureIdx = 0; textureIdx < m_numTextures; ++textureIdx)
    {
        const TextureData& dataAtIdx = _dataForTextures[textureIdx];
        Q_ASSERT_X(dataAtIdx.m_type != GL_NONE, __FUNCTION__, QString("Texture type for texture %1 cannot be none.").arg(textureIdx).toUtf8().constData());

        StoredTextureData storedDataAtIdx;
        storedDataAtIdx.m_unitId = static_cast<unsigned int>(textureIdx) + _textureIdOffset;
        storedDataAtIdx.m_width = dataAtIdx.m_width;
        storedDataAtIdx.m_height = dataAtIdx.m_height;
        storedDataAtIdx.m_type = dataAtIdx.m_type;
        storedDataAtIdx.m_filterParam = static_cast<GLint>(dataAtIdx.m_filterParam);

        // If we might pass internal format instead of format, do this conditionally.
        Q_ASSERT_X(dataAtIdx.m_format != GL_NONE, __FUNCTION__, QString("Format for texture %1 cannot be none.").arg(textureIdx).toUtf8().constData());
        storedDataAtIdx.m_format = dataAtIdx.m_format;
        storedDataAtIdx.m_internalFormat = CalcInternalFormat(storedDataAtIdx.m_format, storedDataAtIdx.m_type);
        storedDataAtIdx.m_data = nullptr;

        // Copy the data if we have any
        if ( dataAtIdx.m_data )
        {
            // If this image size calc is too unreliable maybe just pass in the image size.
            size_t imageSize = static_cast<size_t>(storedDataAtIdx.m_width) *
                               static_cast<size_t>(storedDataAtIdx.m_height) * CalcNumComponents(storedDataAtIdx.m_format) * CalcPerComponentSize(storedDataAtIdx.m_type);
            if ( imageSize > 0 )
            {
                storedDataAtIdx.m_data = malloc(imageSize);
                memcpy(storedDataAtIdx.m_data, dataAtIdx.m_data, imageSize);
            }
        }

        m_storedTextureData.push_back(storedDataAtIdx);

        // !!! I HAVE to set the active texture unit before binding !!!
        extraFuncs->glActiveTexture(GL_TEXTURE0 + storedDataAtIdx.m_unitId);
        extraFuncs->glBindTexture(GL_TEXTURE_2D, m_handles[textureIdx]);
        extraFuncs->glTexImage2D(GL_TEXTURE_2D,
                                 0,
                                 storedDataAtIdx.m_internalFormat,
                                 storedDataAtIdx.m_width,
                                 storedDataAtIdx.m_height,
                                 0,
                                 storedDataAtIdx.m_format,
                                 storedDataAtIdx.m_type,
                                 storedDataAtIdx.m_data);

        // Setting to GL_NEAREST, seems to be desirable for render target applications.
        extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, storedDataAtIdx.m_filterParam);
        extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, storedDataAtIdx.m_filterParam);
        extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        extraFuncs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Unbind
        extraFuncs->glBindTexture(GL_TEXTURE_2D, 0);
    }

    extraFuncs->glBindTexture(GL_TEXTURE_2D, 0);
}

///
/// \brief Texture::CalcFormat
///
///        From https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml:
///        Specifies the format of the pixel data. The following symbolic values are accepted (reordered and commented by me):
///        GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL, GL_RED, GL_RG, GL_RGB, GL_RGBA, <- Base Internal Formats
///        GL_BGR, GL_BGRA,
///        GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_BGR_INTEGER, GL_RGBA_INTEGER, GL_BGRA_INTEGER,
///        GL_STENCIL_INDEX
///
/// \param _internalFormat
/// \return
///
GLenum Texture::CalcFormat(GLint _internalFormat)
{
    GLenum format = GL_NONE;

    bool failed = false;
    switch (_internalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_STENCIL:
    case GL_RED:
    case GL_RG:
    case GL_RGB:
    case GL_RGBA:
        format = static_cast<GLenum>(_internalFormat);
        break;
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
        format = GL_RED;
        break;
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
        format = GL_RG;
        break;
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16_SNORM:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_SRGB8:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB32I:
    case GL_RGB32UI:
        format = GL_RGB;
        break;
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGB10_A2:
    case GL_RGB10_A2UI:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
        format = GL_RGBA;
        break;
    default:
        failed = true;
    }

    QString msgIfFailed = QString("Could not deduce Texture for internal format %1").arg(_internalFormat, 4, 16, QChar('0'));
    Q_ASSERT_X(!failed, __FUNCTION__, msgIfFailed.toUtf8().constData());

    return format;
}

size_t Texture::CalcNumComponents(GLenum _format)
{
    size_t numComponents = 0;

    bool failed = false;
    switch (_format)
    {
    case GL_DEPTH_COMPONENT:
    case GL_RED:
        numComponents = 1;
        break;
    case GL_DEPTH_STENCIL:
    case GL_RG:
        numComponents = 2;
        break;
    case GL_RGB:
    case GL_BGR:
        numComponents = 3;
        break;
    case GL_RGBA:
    case GL_BGRA:
        numComponents = 4;
        break;
    default:
        failed = true;
    }

    QString msgIfFailed = QString("Could not deduce num components for format %1").arg(_format, 4, 16, QChar('0'));
    Q_ASSERT_X(!failed, __FUNCTION__, msgIfFailed.toUtf8().constData());

    return numComponents;
}

size_t Texture::CalcPerComponentSize(GLenum _type)
{
    size_t componentSize = 0;

    bool failed = false;
    switch(_type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
        componentSize = 1;
        break;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
        componentSize = 2;
        break;
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        componentSize = 4;
        break;
    default:
        failed = true;
    }

    QString msgIfFailed = QString("Could not deduce per component size for type %1").arg(_type, 4, 16, QChar('0'));
    Q_ASSERT_X(!failed, __FUNCTION__, msgIfFailed.toUtf8().constData());

    return componentSize;
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
    case GL_INT:
    case GL_UNSIGNED_INT:
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
