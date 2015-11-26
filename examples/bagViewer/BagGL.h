#ifndef BAGGL_H
#define BAGGL_H

#include "GLWindow.h"
#include "bag.h"
#include <QVector3D>
#include <QTime>
#include <memory>


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
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    
private:
    GLuint loadShader(GLenum type, const char *source);
    QMatrix4x4 genMatrix();
    void resetView();
    
    GLuint posAttr;
    GLuint colAttr;
    GLuint matrixUniform;
    
    QOpenGLShaderProgram *program;
    
    bagHandle bag;
    
    float nearPlane, farPlane;
    
    float zoom;
    float yaw;
    float pitch;
    bool rotating;
    QPoint lastPosition;
    
    QVector3D translatePosition;
    QVector3D bagCenter;
    
    bool translating;
    QVector3D translateStartPosition, translateEndPosition;
    QTime translateStartTime;
    
    float heightExaggeration; 
    
    std::vector<GLfloat> elevationVerticies;
    std::vector<GLuint> indecies;
    GLsizei elementCount;
    const GLuint primitiveReset;
    typedef std::map<std::pair<u32,u32>,GLuint> IndexMap;
    typedef std::shared_ptr<IndexMap> IndexMapPtr;
};

#endif