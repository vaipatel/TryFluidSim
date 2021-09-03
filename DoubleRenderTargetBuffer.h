#ifndef DOUBLERENDERTARGETBUFFER_H
#define DOUBLERENDERTARGETBUFFER_H

class RenderTargetBuffer;

class DoubleRenderTargetBuffer
{
public:
    DoubleRenderTargetBuffer(RenderTargetBuffer* _fboA, RenderTargetBuffer* _fboB);
    RenderTargetBuffer* GetFirst() const { return m_firstFBO; }
    void Swap();

private:
    RenderTargetBuffer* m_firstFBO = nullptr;
    RenderTargetBuffer* m_secondFBO = nullptr;
};

#endif // DOUBLERENDERTARGETBUFFER_H
