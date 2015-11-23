#ifndef BAGGL_H
#define BAGGL_H

#include "GLWindow.h"
#include "bag.h"

class BagGL: public GLWindow
{
public:
    BagGL();
    ~BagGL();
    
    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;
    
    bool openBag(QString const &bagFileName);
    void closeBag();
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    
private:
    GLuint loadShader(GLenum type, const char *source);
    
    GLuint posAttr;
    GLuint colAttr;
    GLuint matrixUniform;
    
    QOpenGLShaderProgram *program;
    
    bagHandle bag;
    
    float zoom;
    float yaw;
    float pitch;
    bool rotating;
    QPoint lastPosition;
    
    std::vector<GLfloat> elevationVerticies;
};

#endif