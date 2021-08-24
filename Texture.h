#ifndef TEXTURE_H
#define TEXTURE_H

#include <qopengl.h>
#include <vector>

struct TextureData
{
    // I might need some level of the following commented out params if I want to be able to
    // flexibly pass in the internal format instead of the format.
//    bool m_needsFormatCalc;
//    GLint m_internalFormat;
    int m_width;
    int m_height;
    GLenum m_format;
    GLenum m_type;
    const void* m_data;
};

class Texture
{
public:
    struct StoredTextureData
    {
        unsigned int m_id;
        GLint m_internalFormat;
        int m_width;
        int m_height;
        GLenum m_format;
        GLenum m_type;
        void* m_data;
    };

    Texture(int _width, int _height, GLenum _format, GLenum _type, const char *_data, unsigned int _textureIdOffset = 0);
    Texture(const std::vector<TextureData>& _dataForTextures, unsigned int _textureIdOffset = 0);
    Texture(const QString& _imageFileName, unsigned int _textureIdOffset = 0);
    ~Texture();

    void Bind(size_t _textureIdx = 0) const;
    void CleanUp();

    size_t GetNumTextures() const { return m_numTextures; }
    unsigned int GetHandle(size_t _textureIdx) const { Q_ASSERT(_textureIdx < m_numTextures); return m_handles[_textureIdx]; }
    unsigned int GetId(size_t _textureIdx = 0) const { Q_ASSERT(_textureIdx < m_numTextures); return m_storedTextureData[_textureIdx].m_id; }
    int GetWidth(size_t _textureIdx = 0) const { Q_ASSERT(_textureIdx < m_numTextures); return m_storedTextureData[_textureIdx].m_width; }
    int GetHeight(size_t _textureIdx = 0) const { Q_ASSERT(_textureIdx < m_numTextures); return m_storedTextureData[_textureIdx].m_height; }
    GLenum GetFormat(size_t _textureIdx = 0) const { Q_ASSERT(_textureIdx < m_numTextures); return m_storedTextureData[_textureIdx].m_format; }
    GLenum GetType(size_t _textureIdx = 0) const { Q_ASSERT(_textureIdx < m_numTextures); return m_storedTextureData[_textureIdx].m_type; }
    GLint GetInternalFormat(size_t _textureIdx = 0) const { Q_ASSERT(_textureIdx < m_numTextures); return m_storedTextureData[_textureIdx].m_internalFormat; }

private:
    void Construct(const std::vector<TextureData>& _dataForTextures, unsigned int _textureIdOffset = 0);
    static GLenum CalcFormat(GLint _internalFormat);
    static size_t CalcNumComponents(GLenum _format);
    static size_t CalcPerComponentSize(GLenum _type);
    int CalcInternalFormat(GLenum _format, GLenum _type) const;

    size_t m_numTextures = 0;
    std::vector<unsigned int> m_handles;
    std::vector<StoredTextureData> m_storedTextureData;
};

#endif // TEXTURE_H
