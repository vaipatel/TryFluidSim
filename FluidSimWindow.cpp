#include "FluidSimWindow.h"
#include "Blitter.h"
#include "DoubleRenderTargetBuffer.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Shared.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QDebug>
#include <QMouseEvent>
#include <QScreen>
#include <QVector2D>
#include <QVector3D>

FluidSimWindow::FluidSimWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

FluidSimWindow::~FluidSimWindow()
{
    delete m_blitter;
    delete m_velocityOutTextureA;
    delete m_velocityOutTextureB;
    delete m_velocityDoubleTargetBuffer;
    delete m_dyeTextureA;
    delete m_dyeTextureB;
    delete m_dyeDoubleTargetBuffer;
    delete m_divergenceTexture;
    delete m_divergenceTargetBuffer;
    delete m_pressureTextureA;
    delete m_pressureTextureB;
    delete m_pressureDoubleTargetBuffer;
    delete m_advectProgram;
    delete m_splatForceProgram;
    delete m_divergenceProgram;
    delete m_pressureSolveProgram;
    delete m_gradientSubtractProgram;
}

void FluidSimWindow::initialize()
{
    m_blitter = new Blitter;

    m_uVelocityInputTexture = new Texture(m_perlinNoiseImgFileName, TextureData::FilterParam::LINEAR, 0);
    m_uSourceInputTexture = new Texture(m_moscowImgFileName, TextureData::FilterParam::LINEAR, 1);

    m_advectProgram = new ShaderProgram(m_baseVertShaderFileName, m_advectFragShaderFileName);
    m_splatForceProgram = new ShaderProgram(m_baseVertShaderFileName, m_splatForceFragShaderFileName);
    m_divergenceProgram = new ShaderProgram(m_baseVertShaderFileName, m_divergenceFragShaderFileName);
    m_pressureSolveProgram = new ShaderProgram(m_baseVertShaderFileName, m_pressureSolveFragShaderFileName);
    m_gradientSubtractProgram = new ShaderProgram(m_baseVertShaderFileName, m_gradientSubtractFragShaderFileName);

    SetupTextures();
}

void FluidSimWindow::render()
{
    float dtS = static_cast<float>(1/(screen()->refreshRate()));

    // Add forces
    {
        static int maxPosToProcess = 10;
        int processedPos = 0;
        while ( !m_mousePosList.empty() && processedPos < maxPosToProcess )
        {
            processedPos++;

            QVector2D mousePos = m_mousePosList.front();
            m_mousePosList.pop_front();

            QVector2D delta;
            if ( !m_mousePosList.empty() )
            {
                delta = m_mousePosList.front() - mousePos;
            }

            Splat(mousePos.x(), mousePos.y(), SPLAT_FORCE * delta.x(), SPLAT_FORCE * delta.y(), {1.0f, 0.0, 1.0});
        }
    }

    ComputeDivergence();

    SolvePressure();

    SubtractGradient();

    // Advect velocity
    Advect(m_velocityDoubleTargetBuffer, dtS);

    // Advect dye
    Advect(m_dyeDoubleTargetBuffer, dtS);

    m_blitter->BindTarget(nullptr);
    m_blitter->DrawTextureOnScreenQuad(m_dyeDoubleTargetBuffer->GetFirst()->GetTargetTexture());
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

void FluidSimWindow::mouseMoveEvent(QMouseEvent* _ev)
{
    if ( _ev->buttons() & Qt::LeftButton )
    {
        float x = static_cast<float>(_ev->x()) / static_cast<float>(width());
        float y = 1.0f - static_cast<float>(_ev->y()) / static_cast<float>(height());
        m_mousePosList.push_back({x, y});
    }
}

void FluidSimWindow::CleanUpTextures()
{
    SafeDelete(m_velocityDoubleTargetBuffer);
    SafeDelete(m_velocityOutTextureA);
    SafeDelete(m_velocityOutTextureB);
    SafeDelete(m_dyeDoubleTargetBuffer);
    SafeDelete(m_dyeTextureA);
    SafeDelete(m_dyeTextureB);
    SafeDelete(m_divergenceTargetBuffer);
    SafeDelete(m_divergenceTexture);
    SafeDelete(m_pressureDoubleTargetBuffer);
    SafeDelete(m_pressureTextureA);
    SafeDelete(m_pressureTextureB);
}

void FluidSimWindow::SetupTextures()
{
    m_velocityOutTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_velocityOutTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_velocityDoubleTargetBuffer = new DoubleRenderTargetBuffer(m_velocityOutTextureA, m_velocityOutTextureB);
    std::vector<RenderTargetBuffer*> velocityBuffers = m_velocityDoubleTargetBuffer->GetBoth();
    foreach (RenderTargetBuffer* velocityBuffer, velocityBuffers)
    {
        ConfigureRenderTarget(velocityBuffer);
    }

    m_dyeTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_dyeTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_dyeDoubleTargetBuffer = new DoubleRenderTargetBuffer(m_dyeTextureA, m_dyeTextureB);
    std::vector<RenderTargetBuffer*> dyeBuffers = m_dyeDoubleTargetBuffer->GetBoth();
    foreach (RenderTargetBuffer* dyeBuffer, dyeBuffers)
    {
        ConfigureRenderTarget(dyeBuffer);
    }

    m_divergenceTexture = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::NEAREST, nullptr, 0);
    m_divergenceTargetBuffer = new RenderTargetBuffer(m_divergenceTexture);
    ConfigureRenderTarget(m_divergenceTargetBuffer);

    m_pressureTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::NEAREST, nullptr, 0);
    m_pressureTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::NEAREST, nullptr, 0);
    m_pressureDoubleTargetBuffer =  new DoubleRenderTargetBuffer(m_pressureTextureA, m_pressureTextureB);
    std::vector<RenderTargetBuffer*> pressureBuffers = m_pressureDoubleTargetBuffer->GetBoth();
    foreach(RenderTargetBuffer* pressureBuffer, pressureBuffers)
    {
        ConfigureRenderTarget(pressureBuffer);
    }
}

void FluidSimWindow::Advect(DoubleRenderTargetBuffer* _doubleBuffer, float _dt)
{
    // Bind advect shader program
    m_advectProgram->Bind();

    // Pass quantity to be advected
    Texture* textureToBeAdvected = _doubleBuffer->GetFirst()->GetTargetTexture();
    size_t textureToBeAdvectedUnitId = 0;
    textureToBeAdvected->Bind(0, &textureToBeAdvectedUnitId);
    m_advectProgram->SetUniform("uSource", static_cast<int>(textureToBeAdvectedUnitId));

    // Pass velocity buffer
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    if ( _doubleBuffer != m_velocityDoubleTargetBuffer )
    {
        velTexUnitId = 1; // If advected quantity is not velocity itself, bind velocity to different texture unit
    }
    velTex->Bind(0, &velTexUnitId);
    m_advectProgram->SetUniform("uVelocity", static_cast<int>(velTexUnitId));

    // Pass time
    m_advectProgram->SetUniform("dt", _dt);

    // Pass dissipation
    m_advectProgram->SetUniform("dissipation", 0.5f);

    // Pass cell size
    m_advectProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    // Blit advected result onto second buffer
    m_blitter->BlitToTarget(_doubleBuffer->GetSecond());

    // Release advect shader program
    m_advectProgram->Release();

    // Swap double buffers. Next time advected result will be read from.
    _doubleBuffer->SwapBuffers();
}

void FluidSimWindow::Splat(float _x, float _y, float _dx, float _dy, const QVector3D& _color)
{
    // Bind the splat force shader
    m_splatForceProgram->Bind();

    // Pass first velocity buffer's texture
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    velTex->Bind(0, &velTexUnitId);
    m_splatForceProgram->SetUniform("uSource", static_cast<int>(velTexUnitId));

    // Pass other params
    m_splatForceProgram->SetUniform("aspectRatio", m_viewAspect);
    m_splatForceProgram->SetUniform("color", QVector3D(_dx, _dy, 0.0f));
    m_splatForceProgram->SetUniform("point", QVector2D(_x, _y));
    m_splatForceProgram->SetUniform("radius", 0.25f / 100.0f);
    m_splatForceProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    // Blit result onto second velocity buffer
    m_blitter->BlitToTarget(m_velocityDoubleTargetBuffer->GetSecond());

    // Swap velocity buffers
    m_velocityDoubleTargetBuffer->SwapBuffers();

    // Pass first dye buffer's texture
    Texture* dyeTex = m_dyeDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t dyeTexUnitId = 1;
    dyeTex->Bind(0, &dyeTexUnitId);
    m_splatForceProgram->SetUniform("uSource", static_cast<int>(dyeTexUnitId));

    // Pass the color
    m_splatForceProgram->SetUniform("color", _color);

    // Blit result to second dye buffer
    m_blitter->BlitToTarget(m_dyeDoubleTargetBuffer->GetSecond());

    // Release the splat force shader
    m_splatForceProgram->Release();

    // Swap dye buffers
    m_dyeDoubleTargetBuffer->SwapBuffers();
}

void FluidSimWindow::ComputeDivergence()
{
    m_divergenceProgram->Bind();

    // Pass first velocity buffer's texture
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    velTex->Bind(0, &velTexUnitId);
    m_divergenceProgram->SetUniform("uVelocity", static_cast<int>(velTexUnitId));

    // Pass cell size
    m_divergenceProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    m_blitter->BlitToTarget(m_divergenceTargetBuffer);

    m_divergenceProgram->Release();
}

void FluidSimWindow::SolvePressure()
{
    m_pressureSolveProgram->Bind();

    // Pass divergence texture
    Texture* divTex = m_divergenceTargetBuffer->GetTargetTexture();
    size_t divTexUnitId = 0;
    divTex->Bind(0, &divTexUnitId);
    m_pressureSolveProgram->SetUniform("uDivergence", static_cast<int>(divTexUnitId));

    // Pass cell size
    m_pressureSolveProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    for (size_t i = 0; i < 2; i++)
    {
        // Pass curr pressure texture
        Texture* pressureTex = m_pressureDoubleTargetBuffer->GetFirst()->GetTargetTexture();
        size_t pressureTexUnitId = 1;
        pressureTex->Bind(0, &pressureTexUnitId);
        m_pressureSolveProgram->SetUniform("uPressure", static_cast<int>(pressureTexUnitId));

        m_blitter->BlitToTarget(m_pressureDoubleTargetBuffer->GetSecond());

        m_pressureDoubleTargetBuffer->SwapBuffers();
    }

    m_pressureSolveProgram->Release();
}

void FluidSimWindow::SubtractGradient()
{
    m_gradientSubtractProgram->Bind();

    // Pass cell size
    m_gradientSubtractProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    // Pass first velocity buffer's texture
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    velTex->Bind(0, &velTexUnitId);
    m_gradientSubtractProgram->SetUniform("uVelocity", static_cast<int>(velTexUnitId));

    // Pass first pressure buffer's texture
    Texture* pressureTex = m_pressureDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t pressureTexUnitId = 1;
    pressureTex->Bind(0, &pressureTexUnitId);
    m_gradientSubtractProgram->SetUniform("uPressure", static_cast<int>(pressureTexUnitId));

    m_blitter->BlitToTarget(m_velocityDoubleTargetBuffer->GetSecond());

    m_velocityDoubleTargetBuffer->SwapBuffers();

    m_gradientSubtractProgram->Release();
}

void FluidSimWindow::ConfigureRenderTarget(RenderTargetBuffer* _renderTarget)
{
    _renderTarget->SetDepthTestEnabled(false);
    _renderTarget->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    _renderTarget->SetClearColor({38, 38, 38, 255});
}
