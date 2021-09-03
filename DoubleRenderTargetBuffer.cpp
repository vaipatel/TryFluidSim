#include "DoubleRenderTargetBuffer.h"
#include "RenderTargetBuffer.h"

DoubleRenderTargetBuffer::DoubleRenderTargetBuffer(RenderTargetBuffer* _fboA, RenderTargetBuffer* _fboB)
{
    m_firstFBO = _fboA;
    m_secondFBO = _fboB;
}

void DoubleRenderTargetBuffer::Swap()
{
    RenderTargetBuffer* temp = m_secondFBO;
    m_secondFBO = m_firstFBO;
    m_firstFBO = temp;
}
