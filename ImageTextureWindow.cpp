#include "ImageTextureWindow.h"
#include "Blitter.h"
#include "Texture.h"
#include <QWindow>

ImageTextureWindow::ImageTextureWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}

ImageTextureWindow::~ImageTextureWindow()
{
    if ( m_texture )
    {
        delete m_texture;
    }

    if ( m_blitter )
    {
        delete m_blitter;
    }
}

void ImageTextureWindow::initialize()
{
    m_blitter = new Blitter;
    SetupTexture();
}

void ImageTextureWindow::render()
{
    m_blitter->BlitToScreen(m_texture);
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
    if ( m_texture )
    {
        delete m_texture;
        m_texture = nullptr;
    }
}

void ImageTextureWindow::SetupTexture()
{
    m_texture = new Texture(m_imageFileName);
}

