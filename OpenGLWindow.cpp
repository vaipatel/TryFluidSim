#include "OpenGLWindow.h"
#include "Shared.h"
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QPainter>

OpenGLWindow::OpenGLWindow(QWindow* _parent)
    : QWindow(_parent)
    , m_animating(false)
    , m_context(nullptr)
    , m_device(nullptr)
{
    setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLWindow::~OpenGLWindow()
{
    SafeDelete(m_device);
    SafeDelete(m_context);
}

void OpenGLWindow::render(QPainter* _painter)
{
    Q_UNUSED(_painter)
}

void OpenGLWindow::initialize()
{
}

void OpenGLWindow::render()
{
    if (!m_device)
    {
        m_device = new QOpenGLPaintDevice;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_device->setSize(size());

    QPainter painter(m_device);
    render(&painter);
}

void OpenGLWindow::renderLater()
{
    requestUpdate();
}

bool OpenGLWindow::event(QEvent* _event)
{
    switch (_event->type())
    {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    case QEvent::Close:
        cleanup();
        [[clang::fallthrough]];
    default:
        return QWindow::event(_event);
    }
}

void OpenGLWindow::exposeEvent(QExposeEvent* _event)
{
    Q_UNUSED(_event)

    if (isExposed())
    {
        renderNow();
    }
}

void OpenGLWindow::checkGLError()
{
    GLenum errorCode;
    if ( (errorCode = glGetError()) != GL_NO_ERROR )
    {
        QString error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        Q_ASSERT_X(false,__FUNCTION__,error.toUtf8());
    }
}

void OpenGLWindow::renderNow()
{
    if (!isExposed())
    {
        return;
    }

    // Make the context current before calling the child's render.
    // If there is no context, create one, and then initialize.
    bool needsInitialize = false;
    if (!m_context)
    {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }
    m_context->makeCurrent(this);

    if (needsInitialize)
    {
        initializeOpenGLFunctions();
        UpdateViewPortIfNeeded();
        initialize();
    }
    else
    {
        UpdateViewPortIfNeeded();
    }

    // Call child's render
    render();

    // Child's render done. Swap buffers.
    m_context->swapBuffers(this);

    if (m_animating)
    {
        renderLater();
    }
}

void OpenGLWindow::setAnimating(bool _animating)
{
    m_animating = _animating;

    if (_animating)
    {
        renderLater();
    }
}

QPair<int, int> OpenGLWindow::CalcViewPortWidthHeight() const
{
    const GLsizei retinaScale = static_cast<GLsizei>(devicePixelRatio());

    // Vai: Calc viewport width/height
    GLsizei viewWidth = width() * retinaScale;
    GLsizei viewHeight = height() * retinaScale;

    return {viewWidth, viewHeight};
}

///
/// \brief Updates the viewport width and height if necessary. If updated, this function additionally
///        updates the aspect ratio value, calls glViewport() and recreates the render target FBOs.
///
void OpenGLWindow::UpdateViewPortIfNeeded()
{
    QPair<int, int> viewWidthAndHeight = CalcViewPortWidthHeight();
    int viewWidth = viewWidthAndHeight.first;
    int viewHeight = viewWidthAndHeight.second;

    if ( viewWidth != m_viewWidth || viewHeight != m_viewHeight )
    {
        // Update width, height and aspect
        m_viewWidth = viewWidth;
        m_viewHeight = viewHeight;
        m_viewAspect = static_cast<float>(m_viewWidth)/static_cast<float>(m_viewHeight);

        HandleViewPortUpdated();
    }
}

