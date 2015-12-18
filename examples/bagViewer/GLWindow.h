#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions_4_0_Core>

class QPainter;
class QOpenGLPaintDevice;
class QOpenGLShaderProgram;

class GLWindow: public QWindow, protected QOpenGLFunctions_4_0_Core
{
    Q_OBJECT
public:
    explicit GLWindow(QWindow *parent = 0);
    ~GLWindow();

    virtual void render(QPainter *painter);
    virtual void render(bool picking = false);
    
    virtual void initialize();
    
    void setAnimating(bool animating);
    
    void printFormat();
    
public slots:
    void renderLater();
    void renderNow(bool picking=false);
    
protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    
    void exposeEvent(QExposeEvent *event) Q_DECL_OVERRIDE;
    
    void makeContextCurrent();
    
private:
    bool updatePending;
    bool animating;
    
    QOpenGLContext *context;
    QOpenGLPaintDevice *device;
};

#endif