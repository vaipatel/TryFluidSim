#include "FluidSimWindow.h"
#include "Blitter.h"
#include "DoubleRenderTargetBuffer.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"

FluidSimWindow::FluidSimWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

FluidSimWindow::~FluidSimWindow()
{
    delete m_velocityOutTextureA;
    delete m_velocityOutTextureB;
    delete m_velocityTargetBufferA;
    delete m_velocityTargetBufferB;
    delete m_velocityDoubleTargetBuffer;
}

void FluidSimWindow::initialize()
{
    m_blitter = new Blitter;

    m_velocityOutTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, nullptr);
    m_velocityOutTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, nullptr);

//    m_perlinProgram = new ShaderProgram(m_perlinVertShaderFileName, m_perlinFragShaderFileName);
//    m_triangleProgram = new ShaderProgram(m_rotTexturedTriVertShaderFileName, m_rotTexturedTriFragShaderFileName);
}

void FluidSimWindow::render()
{

}

void FluidSimWindow::cleanup()
{

}

void FluidSimWindow::HandleViewPortUpdated()
{
    m_texelSizeX = 1.0f / static_cast<float>(m_viewWidth);
    m_texelSizeY = 1.0f / static_cast<float>(m_viewHeight);
}
