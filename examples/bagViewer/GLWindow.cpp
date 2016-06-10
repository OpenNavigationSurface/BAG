#include <iostream>

#include "GLWindow.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>


GLWindow::GLWindow(QWindow* parent): QWindow(parent)
    ,updatePending(false),animating(false),context(0),device(0)
{
    setSurfaceType(QWindow::QSurface::OpenGLSurface);
}

GLWindow::~GLWindow()
{
    delete device;
}

void GLWindow::initialize()
{

}


void GLWindow::render(QPainter *painter)
{
    Q_UNUSED(painter)
}

void GLWindow::render(bool picking)
{
    if (!device)
        device = new QOpenGLPaintDevice;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    device->setSize(size());
    
    QPainter painter(device);
    render(&painter);
}

void GLWindow::renderLater()
{
    if(!updatePending)
    {
        updatePending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool GLWindow::event(QEvent* event)
{
    switch(event->type()){
        case QEvent::UpdateRequest:
            updatePending = false;
            renderNow();
            return true;
        default:
            return QWindow::event(event);
    }
}

void GLWindow::exposeEvent(QExposeEvent* event)
{
    Q_UNUSED(event);

    if(isExposed())
        renderNow();
}

void GLWindow::renderNow(bool picking)
{
    if(!isExposed())
        return;
    
    bool needInit = false;
    
    if(!context){
        context = new QOpenGLContext(this);
        context->setFormat(requestedFormat());
        if (!context->create()) {
            std::cerr << "Failed to create OpenGL context." << std::endl;
            exit(1);
        }
        needInit = true;
    }
    
    context->makeCurrent(this);
    
    if(needInit){
        initializeOpenGLFunctions();
        initialize();
    }
    
    render(picking);
    
    context->swapBuffers(this);
    if(animating)
        renderLater();
}

void GLWindow::setAnimating(bool a)
{
    animating = a;
    if(animating)
        renderLater();
}

void GLWindow::makeContextCurrent()
{
    if(context)
        context->makeCurrent(this);
}


void GLWindow::printFormat()
{
    QSurfaceFormat f = context->format();
    std::cerr << "OpenGL version: " << f.majorVersion() << "." << f.minorVersion();
    switch(f.profile())
    {
        case(QSurfaceFormat::NoProfile):
            std::cerr << " No Profile" << std::endl;
            break;
        case(QSurfaceFormat::CoreProfile):
            std::cerr << " Core Profile" << std::endl;
            break;
        case(QSurfaceFormat::CompatibilityProfile):
            std::cerr << " Compatibility Profile" << std::endl;
            break;
        default:
            std::cerr << " Unknown Profile" << std::endl;
    }
}
