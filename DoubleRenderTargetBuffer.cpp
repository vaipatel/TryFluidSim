#include "DoubleRenderTargetBuffer.h"
#include "RenderTargetBuffer.h"
#include "Texture.h"

DoubleRenderTargetBuffer::DoubleRenderTargetBuffer(Texture *_texA, Texture *_texB)
{
    m_firstFBO = new RenderTargetBuffer(_texA);
    m_secondFBO = new RenderTargetBuffer(_texB);
}

DoubleRenderTargetBuffer::~DoubleRenderTargetBuffer()
{
    delete m_firstFBO;
    delete m_secondFBO;
}

void DoubleRenderTargetBuffer::SwapBuffers()
{
    RenderTargetBuffer* temp = m_secondFBO;
    m_secondFBO = m_firstFBO;
    m_firstFBO = temp;
}
