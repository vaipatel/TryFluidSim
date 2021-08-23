#ifndef BLITTER_H
#define BLITTER_H

#include <QString>

class ShaderProgram;
class RenderTargetBuffer;
class Texture;
class TrisObject;

class Blitter
{
public:
    Blitter();
    ~Blitter();

    void Blit(const Texture* _textureToBlit, size_t _texIdx = 0);
    void CleanUp();

private:
    void SetupScreenQuad();
    void DrawTextureOnScreenQuad(const Texture *_texture, size_t _texIdx);

    const QString m_blitToScreenQuadVertShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.vert";
    const QString m_blitToScreenQuadFragShaderFileName = ":/Resources/Shaders/BlitToScreenQuad.frag";
    ShaderProgram* m_screenProgram = nullptr;
    TrisObject* m_quad = nullptr;
};

#endif // BLITTER_H