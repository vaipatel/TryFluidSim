#include "ImageTextureWindow.h"
#include "Blitter.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Shared.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QMatrix4x4>
#include <QScreen>
#include <QWindow>

ImageTextureWindow::ImageTextureWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

ImageTextureWindow::~ImageTextureWindow()
{
    delete m_perlinNoiseTexture;
    delete m_redPepperTexture;
    delete m_blitter;
    delete m_perlinTargetBuffer;
    delete m_perlinOutTexture;
    delete m_quad;
    delete m_tri;
    delete m_perlinProgram;
    delete m_triangleProgram;
}

void ImageTextureWindow::initialize()
{
    m_blitter = new Blitter;

    m_perlinProgram = new ShaderProgram(m_perlinVertShaderFileName, m_perlinFragShaderFileName);
    m_triangleProgram = new ShaderProgram(m_rotTexturedTriVertShaderFileName, m_rotTexturedTriFragShaderFileName);

    SetupTexture();
    SetupQuad();
    SetupTriangle();
}

void ImageTextureWindow::render()
{
    m_blitter->BindTarget(m_perlinTargetBuffer);
    DrawPerlinNoiseOnQuad();

    m_blitter->BindTarget(m_triTargetBuffer);
    DrawTriangle();
    
    m_blitter->BindTarget(nullptr);
    m_blitter->DrawTextureOnScreenQuad(m_triOutTexture);
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
    // Delete input textures
    SafeDelete(m_perlinNoiseTexture);
    SafeDelete(m_redPepperTexture);

    // Delete perlin target
    SafeDelete(m_perlinTargetBuffer);
    SafeDelete(m_perlinOutTexture);

    // Delete triangle target
    SafeDelete(m_triTargetBuffer);
    SafeDelete(m_triOutTexture);
}

void ImageTextureWindow::SetupTexture()
{
    // Create input textures
    m_perlinNoiseTexture = new Texture(m_perlinNoiseImgFileName, TextureData::FilterParam::NEAREST, 0);
    m_redPepperTexture = new Texture(m_redPepperImgFileName, TextureData::FilterParam::NEAREST, 1);
    
    // Create perlin target
    m_perlinOutTexture = new Texture({{m_viewWidth, m_viewHeight, GL_RGBA, GL_UNSIGNED_BYTE, TextureData::FilterParam::LINEAR, nullptr}});
    m_perlinTargetBuffer = new RenderTargetBuffer(m_perlinOutTexture);
    m_perlinTargetBuffer->SetDepthTestEnabled(false);
    m_perlinTargetBuffer->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    m_perlinTargetBuffer->SetClearColor({38, 38, 38, 255});

    // Create triangle target
    m_triOutTexture = new Texture({{m_viewWidth, m_viewHeight, GL_RGBA, GL_UNSIGNED_BYTE, TextureData::FilterParam::LINEAR, nullptr}});
    m_triTargetBuffer = new RenderTargetBuffer(m_triOutTexture);
    m_triTargetBuffer->SetDepthTestEnabled(false);
    m_triTargetBuffer->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    m_triTargetBuffer->SetClearColor({90, 90, 90, 255});
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

void ImageTextureWindow::SetupTriangle()
{
    std::vector<Tri> triVertices = {
             // positions            // colors                 // texCoords
        {
            {{ 0.0f,  0.707f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}},
            {{-0.5f, -0.5f,   0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,   0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}
        }
    };
    m_tri = new TrisObject(triVertices);
}

void ImageTextureWindow::DrawPerlinNoiseOnQuad()
{
    // Bind the shader program
    m_perlinProgram->Bind();

    // Pass the noise and strip textures as sampler2D uniforms
    size_t texIdx = 0;
    m_perlinNoiseTexture->Bind(texIdx);
    int perlinTexUnitId = static_cast<int>(m_perlinNoiseTexture->GetUnitId(texIdx));
    m_perlinProgram->SetUniform("PerlinNoise", perlinTexUnitId);
    m_redPepperTexture->Bind(texIdx);
    int redPepperTexUnitId = static_cast<int>(m_redPepperTexture->GetUnitId(texIdx));
    m_perlinProgram->SetUniform("RedPepperStrip", redPepperTexUnitId);

    // Pass time
    m_timeS += static_cast<float>(1/(screen()->refreshRate()));
    m_perlinProgram->SetUniform("Time", m_timeS);

    // Draw the quad
    m_quad->Draw();

    // Unbind the shader program
    m_perlinProgram->Release();
}

void ImageTextureWindow::DrawTriangle()
{
    m_triangleProgram->Bind();

    // Pass rotating matrix to vert shader
    QMatrix4x4 matrix;
    matrix.perspective(60.0f, m_viewAspect, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    float angle = 50.f * m_timeS;
    matrix.rotate(angle, 0, 1, 0);
    m_triangleProgram->SetUniform("matrix", matrix);

    // Pass the perlin out texture as a sampler2D uniform
    size_t texIdx = 0;
    m_perlinOutTexture->Bind(texIdx);
    int perlinOutTexUnitId = static_cast<int>(m_perlinOutTexture->GetUnitId(texIdx));
    m_triangleProgram->SetUniform("PerlinOut", perlinOutTexUnitId);

    // Draw the tri
    m_tri->Draw();

    // Unbind the shader program
    m_triangleProgram->Release();
}

