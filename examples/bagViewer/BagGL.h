#ifndef BAGGL_H
#define BAGGL_H

#include "GLWindow.h"
#include "bag.h"
#include <QVector3D>
#include <QTime>
#include <memory>
#include <QOpenGLTexture>

class BagGL: public GLWindow
{
public:
    BagGL();
    ~BagGL();
    
    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;
    
    bool openBag(QString const &bagFileName);
    void closeBag();
    void setColormap(std::string const &cm);
    void setDrawStyle(std::string const &ds);
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
    GLuint unAttr;
    GLuint normAttr;
    GLuint matrixUniform;
    GLuint normMatrixUniform;
    GLuint lightDirectionUniform;
    GLuint minElevationUniform;
    GLuint maxElevationUniform;
    
    QOpenGLShaderProgram *program;
    
    bagHandle bag;
    
    float minElevation, maxElevation;
    
    typedef std::shared_ptr<QOpenGLTexture> QOpenGLTexturePtr;
    typedef std::map<std::string,QOpenGLTexturePtr> ColormapMap;
    ColormapMap colormaps;
    std::string currentColormap;
    
    std::string drawStyle;
    
    float nearPlane, farPlane;
    
    float zoom;
    float yaw;
    float pitch;
    bool rotating;
    QPoint lastPosition;
    
    QVector3D translatePosition;
    QVector3D bagCenter;
    float defaultZoom;
    
    bool translating;
    QVector3D translateStartPosition, translateEndPosition;
    QTime translateStartTime;
    
    float heightExaggeration;
    bool adjustingHeightExaggeration;
    
    std::vector<GLfloat> elevationVerticies;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> uncertainties;
    std::vector<GLuint> indecies;
    const GLuint primitiveReset;
    typedef std::map<std::pair<u32,u32>,GLuint> IndexMap;
    typedef std::shared_ptr<IndexMap> IndexMapPtr;
};

#endif