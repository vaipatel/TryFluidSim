#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions>
#include <cassert>

class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit OpenGLWindow(QWindow* _parent = nullptr);
    ~OpenGLWindow() override;

    void setAnimating(bool _animating);

public slots:
    void renderLater();
    void renderNow();

protected:
    virtual void render(QPainter* _painter);
    virtual void render();
    virtual void initialize();
    virtual void HandleViewPortUpdated() {}
    virtual void cleanup() {}

    bool event(QEvent *_event) override;
    void exposeEvent(QExposeEvent *_event) override;
    void checkGLError();
    QPair<int, int> CalcViewPortWidthHeight() const;
    void UpdateViewPortIfNeeded();

    int m_viewWidth = 0;    //!< View width. Equals window width times device pixel ratio.
    int m_viewHeight = 0;   //!< View height. Equals window height times device pixel ratio.
    float m_viewAspect = 0; //!< Ratio of width to height

private:
    bool m_animating;
    QOpenGLContext* m_context;
    QOpenGLPaintDevice* m_device;
};

#endif // OPENGLWINDOW_H
