#ifndef DOUBLERENDERTARGETBUFFER_H
#define DOUBLERENDERTARGETBUFFER_H

#include <vector>

class RenderTargetBuffer;
class Texture;

class DoubleRenderTargetBuffer
{
public:
    DoubleRenderTargetBuffer(Texture* _texA, Texture* _texB);
    ~DoubleRenderTargetBuffer();

    RenderTargetBuffer* GetFirst() const { return m_firstFBO; }
    RenderTargetBuffer* GetSecond() const { return m_secondFBO; }
    std::vector<RenderTargetBuffer*> GetBoth() const { return {m_firstFBO, m_secondFBO}; }
    void SwapBuffers();

private:
    RenderTargetBuffer* m_firstFBO = nullptr;
    RenderTargetBuffer* m_secondFBO = nullptr;
};

#endif // DOUBLERENDERTARGETBUFFER_H
