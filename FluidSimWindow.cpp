#include "FluidSimWindow.h"
#include "Blitter.h"
#include "DoubleRenderTargetBuffer.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Shared.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QScreen>
#include <QVector2D>

FluidSimWindow::FluidSimWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

FluidSimWindow::~FluidSimWindow()
{
    delete m_blitter;
    delete m_velocityOutTextureA;
    delete m_velocityOutTextureB;
    delete m_velocityDoubleTargetBuffer;
    delete m_advectProgram;
}

void FluidSimWindow::initialize()
{
    m_blitter = new Blitter;

    m_uVelocityInputTexture = new Texture(m_perlinNoiseImgFileName, TextureData::FilterParam::LINEAR, 0);
    m_uSourceInputTexture = new Texture(m_moscowImgFileName, TextureData::FilterParam::LINEAR, 1);

    m_advectProgram = new ShaderProgram(m_baseVertShaderFileName, m_advectFragShaderFileName);
//    m_triangleProgram = new ShaderProgram(m_rotTexturedTriVertShaderFileName, m_rotTexturedTriFragShaderFileName);

    SetupTextures();
    SetupQuad();
}

void FluidSimWindow::render()
{
    m_blitter->BindTarget(m_velocityDoubleTargetBuffer->GetFirst());

    m_advectProgram->Bind();
    m_uSourceInputTexture->Bind();
    m_advectProgram->SetUniform("uSource", static_cast<int>(m_uSourceInputTexture->GetUnitId()));
    m_uVelocityInputTexture->Bind();
    m_advectProgram->SetUniform("uVelocity", static_cast<int>(m_uVelocityInputTexture->GetUnitId()));

    float timeS = static_cast<float>(1/(screen()->refreshRate()));
    m_advectProgram->SetUniform("dt", timeS);

    m_advectProgram->SetUniform("dissipation", 1.0f);

    m_advectProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    m_quad->Draw();

    m_advectProgram->Release();

    m_blitter->BindTarget(nullptr);
    m_blitter->DrawTextureOnScreenQuad(m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture());
}

void FluidSimWindow::cleanup()
{

}

void FluidSimWindow::HandleViewPortUpdated()
{
    // Update viewport
    glViewport(0, 0, m_viewWidth, m_viewHeight);

    m_texelSizeX = 1.0f / static_cast<float>(m_viewWidth);
    m_texelSizeY = 1.0f / static_cast<float>(m_viewHeight);

    CleanUpTextures();
    SetupTextures();
}

void FluidSimWindow::CleanUpTextures()
{
    SafeDelete(m_velocityDoubleTargetBuffer);
    SafeDelete(m_velocityOutTextureA);
    SafeDelete(m_velocityOutTextureB);
}

void FluidSimWindow::SetupTextures()
{
    m_velocityOutTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr);
    m_velocityOutTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr);
    m_velocityDoubleTargetBuffer = new DoubleRenderTargetBuffer(m_velocityOutTextureA, m_velocityOutTextureB);
    std::vector<RenderTargetBuffer*> velocityBuffers = m_velocityDoubleTargetBuffer->GetBoth();
    foreach (RenderTargetBuffer* velocityBuffer, velocityBuffers)
    {
        velocityBuffer->SetDepthTestEnabled(false);
        velocityBuffer->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
        velocityBuffer->SetClearColor({38, 38, 38, 255});
    }
}

void FluidSimWindow::SetupQuad()
{
    std::vector<Tri> quadVertices = {
             // positions          // colors                 // texCoords
        {
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
        },
        {
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        }
    };
    m_quad = new TrisObject(quadVertices);
}
