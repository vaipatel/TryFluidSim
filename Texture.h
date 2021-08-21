#ifndef TEXTURE_H
#define TEXTURE_H

#include <qopengl.h>

class Texture
{
public:
    Texture(int _width, int _height, GLenum _format, GLenum _type, const char* _data);
    ~Texture();

    void Bind();
    void CleanUp();

    unsigned int GetTextureLoc() const { return m_textureLoc; }
    unsigned int GetId() const { return 0; } //!< Single textures will always be at the first texture unit
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    GLenum GetFormat() const { return m_format; }
    GLenum GetType() const { return m_type; }
    GLint GetInternalFormat() const { return m_internalFormat; }

private:
    int CalcInternalFormat() const;

    unsigned int m_textureLoc = 0;
    int m_width = 0;
    int m_height = 0;
    GLenum m_format = GL_NONE;
    GLenum m_type = GL_NONE;
    GLint m_internalFormat = 0;
};

#endif // TEXTURE_H
