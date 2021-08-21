#ifndef TEXTURE_H
#define TEXTURE_H

#include <qopengl.h>
#include <vector>

struct TextureData
{
    int m_width;
    int m_height;
    GLenum m_format;
    GLenum m_type;
    const char* data;
};

class Texture
{
public:
    struct StoredTextureData
    {
        unsigned int m_id;
        int m_width;
        int m_height;
        GLenum m_format;
        GLenum m_type;
        GLint m_internalFormat;
//        const char* data;
    };

    Texture(int _width, int _height, GLenum _format, GLenum _type, const char* _data);
    Texture(const std::vector<TextureData>& _dataForTextures);
    ~Texture();

    void Bind(size_t _textureIdx = 0);
    void CleanUp();

    size_t GetNumTextures() const { return m_numTextures; }
    unsigned int GetTextureLoc() const { return m_textureLoc; }
    unsigned int GetId(size_t _textureIdx = 0) const { return m_storedTextureData[_textureIdx].m_id; }
    int GetWidth(size_t _textureIdx = 0) const { return m_storedTextureData[_textureIdx].m_width; }
    int GetHeight(size_t _textureIdx = 0) const { return m_storedTextureData[_textureIdx].m_height; }
    GLenum GetFormat(size_t _textureIdx = 0) const { return m_storedTextureData[_textureIdx].m_format; }
    GLenum GetType(size_t _textureIdx = 0) const { return m_storedTextureData[_textureIdx].m_type; }
    GLint GetInternalFormat(size_t _textureIdx = 0) const { return m_storedTextureData[_textureIdx].m_internalFormat; }

private:
    int CalcInternalFormat(GLenum _format, GLenum _type) const;

    size_t m_numTextures = 0;
    unsigned int m_textureLoc = 0;    
    std::vector<StoredTextureData> m_storedTextureData;
};

#endif // TEXTURE_H
