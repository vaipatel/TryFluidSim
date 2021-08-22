#include "ImageTextureWindow.h"
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
}

void ImageTextureWindow::initialize()
{
    m_texture = new Texture(m_imageFileName);
}

void ImageTextureWindow::HandleViewPortUpdated()
{
    // Update viewport
    glViewport(0, 0, m_viewWidth, m_viewHeight);

    if ( m_texture )
    {
        delete m_texture;
        m_texture = nullptr;
    }
}

