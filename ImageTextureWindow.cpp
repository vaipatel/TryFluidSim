#include "ImageTextureWindow.h"
#include "Blitter.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QScreen>
#include <QWindow>

ImageTextureWindow::ImageTextureWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

ImageTextureWindow::~ImageTextureWindow()
{
    if ( m_perlinNoiseTexture )
    {
        delete m_perlinNoiseTexture;
    }

    if ( m_redPepperTexture )
    {
        delete m_redPepperTexture;
    }

    if ( m_blitter )
    {
        delete m_blitter;
    }

    if ( m_renderTargetBuffer )
    {
        delete m_renderTargetBuffer;
    }

    if ( m_targetTexture )
    {
        delete m_targetTexture;
    }

    if ( m_quad )
    {
         delete m_quad;
    }

    delete m_perlinProgram;
}

void ImageTextureWindow::initialize()
{
    m_blitter = new Blitter;

    m_perlinProgram = new ShaderProgram(m_perlinVertShaderFileName, m_perlinFragShaderFileName);

    SetupTexture();
    SetupQuad();
}

void ImageTextureWindow::render()
{
    m_renderTargetBuffer->Bind();

    m_perlinProgram->Bind();

    int texIdx = 0;
    m_perlinNoiseTexture->Bind(texIdx);
    int perlinTexUnitId = static_cast<int>(m_perlinNoiseTexture->GetUnitId(texIdx));
    m_perlinProgram->SetUniform("PerlinNoise", perlinTexUnitId);
    m_redPepperTexture->Bind(texIdx);
    m_timeS += 1/(screen()->refreshRate());
    int redPepperTexUnitId = static_cast<int>(m_redPepperTexture->GetUnitId(texIdx));
    m_perlinProgram->SetUniform("RedPepperStrip", redPepperTexUnitId);
    m_perlinProgram->SetUniform("Time", m_timeS);

    m_quad->Draw();

    m_perlinProgram->Release();
    
    m_blitter->BlitToScreen(m_targetTexture);
}

void ImageTextureWindow::cleanup()
{
    CleanUpTexture();

    if ( m_blitter )
    {
        m_blitter->CleanUp();
    }
}

void ImageTextureWindow::HandleViewPortUpdated()
{
    // Update viewport
    glViewport(0, 0, m_viewWidth, m_viewHeight);

    CleanUpTexture();
    SetupTexture();
}

void ImageTextureWindow::CleanUpTexture()
{
    if ( m_perlinNoiseTexture )
    {
        delete m_perlinNoiseTexture;
        m_perlinNoiseTexture = nullptr;
    }
    
    if ( m_redPepperTexture )
    {
        delete m_redPepperTexture;
        m_redPepperTexture = nullptr;
    }
    
    if ( m_renderTargetBuffer )
    {
        delete m_renderTargetBuffer;
        m_renderTargetBuffer = nullptr;
    }
    
    if ( m_targetTexture )
    {
        delete m_targetTexture;
        m_targetTexture = nullptr;
    }
}

void ImageTextureWindow::SetupTexture()
{
    m_perlinNoiseTexture = new Texture(m_perlinNoiseImgFileName, 0);
    m_redPepperTexture = new Texture(m_redPepperImgFileName, 1);
    
    m_targetTexture = new Texture({{m_viewWidth, m_viewHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr}});    
    m_renderTargetBuffer = new RenderTargetBuffer(m_targetTexture);
    m_renderTargetBuffer->SetDepthTestEnabled(false);
    m_renderTargetBuffer->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    m_renderTargetBuffer->SetClearColor({38, 38, 38, 255});
}

void ImageTextureWindow::SetupQuad()
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

