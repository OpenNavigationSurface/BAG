#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions_4_3_Core>

class QPainter;
class QOpenGLPaintDevice;
class QOpenGLShaderProgram;

class GLWindow: public QWindow, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT
public:
    explicit GLWindow(QWindow *parent = 0);
    ~GLWindow();

    virtual void render(QPainter *painter);
    virtual void render();
    
    virtual void initialize();
    
    void setAnimating(bool animating);
    
public slots:
    void renderLater();
    void renderNow();
    
protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    
    void exposeEvent(QExposeEvent *event) Q_DECL_OVERRIDE;
    
private:
    bool updatePending;
    bool animating;
    
    QOpenGLContext *context;
    QOpenGLPaintDevice *device;
};

#endif