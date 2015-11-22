#ifndef BAGGL_H
#define BAGGL_H

#include "GLWindow.h"

class BagGL: public GLWindow
{
public:
    BagGL();
    
    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;
    
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    
private:
    GLuint loadShader(GLenum type, const char *source);
    
    GLuint posAttr;
    GLuint colAttr;
    GLuint matrixUniform;
    
    QOpenGLShaderProgram *program;
    int frame;
};

#endif