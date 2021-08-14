#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions>

class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;


class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit OpenGLWindow(QWindow* _parent = nullptr);
    ~OpenGLWindow() override;

    virtual void render(QPainter* _painter);
    virtual void render();

    virtual void initialize();
    virtual void cleanup() {}

    void setAnimating(bool _animating);

public slots:
    void renderLater();
    void renderNow();

protected:
    bool event(QEvent *_event) override;

    void exposeEvent(QExposeEvent *_event) override;

    void checkGLError();

private:
    bool m_animating;

    QOpenGLContext* m_context;
    QOpenGLPaintDevice* m_device;
};

#endif // OPENGLWINDOW_H
