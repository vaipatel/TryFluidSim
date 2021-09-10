#include "FluidSimWindow.h"
#include "Blitter.h"
#include "DoubleRenderTargetBuffer.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Shared.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QDebug>
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
    delete m_splatForceProgram;
}

void FluidSimWindow::initialize()
{
    m_blitter = new Blitter;

    m_uVelocityInputTexture = new Texture(m_perlinNoiseImgFileName, TextureData::FilterParam::LINEAR, 0);
    m_uSourceInputTexture = new Texture(m_moscowImgFileName, TextureData::FilterParam::LINEAR, 1);

    m_advectProgram = new ShaderProgram(m_baseVertShaderFileName, m_advectFragShaderFileName);
    m_splatForceProgram = new ShaderProgram(m_baseVertShaderFileName, m_splatForceFragShaderFileName);
//    m_triangleProgram = new ShaderProgram(m_rotTexturedTriVertShaderFileName, m_rotTexturedTriFragShaderFileName);

    SetupTextures();
    SetupQuad();
}

void FluidSimWindow::render()
{   
    float dtS = static_cast<float>(1/(screen()->refreshRate()));

    // Advect velocity
    {
        Texture* velTex = nullptr;
        // TODO: Rem temporary seeding of velocity with perlin noise
        static bool doneOnce = false;
        if ( !doneOnce )
        {
            velTex = m_uVelocityInputTexture;
            doneOnce = true;
        }
        else
        {
            velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
        }
        Advect(m_velocityDoubleTargetBuffer, velTex, dtS);
    }

    // Add forces

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

void FluidSimWindow::Advect(DoubleRenderTargetBuffer* _doubleBuffer, Texture* _velTex, float _dt)
{
    // Bind advect shader program
    m_advectProgram->Bind();

    // Pass buffer texture to read from
    Texture* textureToBeAdvected = _doubleBuffer->GetFirst()->GetTargetTexture();
    textureToBeAdvected->Bind();
    m_advectProgram->SetUniform("uSource", static_cast<int>(textureToBeAdvected->GetUnitId()));

    // Pass velocity buffer
    _velTex->Bind();
    m_advectProgram->SetUniform("uVelocity", static_cast<int>(_velTex->GetUnitId()));

    // Pass time
    m_advectProgram->SetUniform("dt", _dt);

    // Pass dissipation
    m_advectProgram->SetUniform("dissipation", 0.0f);

    // Pass cell size
    m_advectProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    // Blit advected result onto second buffer
    m_blitter->BlitToTarget(_doubleBuffer->GetSecond());

    // Release advect shader program
    m_advectProgram->Release();

    // Swap double buffers. Next time advected result will be read from.
    _doubleBuffer->SwapBuffers();
}
