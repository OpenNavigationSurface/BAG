#ifndef BAGGL_H
#define BAGGL_H

#include "GLWindow.h"
#include "BagIO.h"
#include <QVector3D>
#include <QTime>
#include <memory>
#include <QOpenGLTexture>
#include <QOpenGLDebugLogger>

struct TileGL
{
    QOpenGLTexture elevations;
    QOpenGLTexture uncertainties;
    TileGL(): elevations(QOpenGLTexture::Target2D), uncertainties(QOpenGLTexture::Target2D) {}
    ~TileGL(){}
};

class BagGL: public GLWindow
{
    Q_OBJECT
    
public:
    BagGL();
    ~BagGL();
    
    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;
    
    bool openBag(QString const &bagFileName);
    void closeBag();
    void setColormap(std::string const &cm);
    void setDrawStyle(std::string const &ds);
    
    static const GLuint primitiveReset;
    
public slots:
    void resetView();
    void messageLogged(const QOpenGLDebugMessage & debugMessage);
    
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    
private:
    QMatrix4x4 genMatrix();
    
    GLuint matrixUniform;
    GLuint normMatrixUniform;
    GLuint lightDirectionUniform;
    GLuint minElevationUniform;
    GLuint maxElevationUniform;
    GLuint elevationMapUniform;
    GLuint spacingUniform;
    GLuint lowerLeftUniform;
    GLuint tileSizeUniform;
    GLuint colorMapUniform;
    
    GLuint tileVAO;
    GLuint tileBuffers[2];
    GLsizei tileIndeciesCount;
    
    BagIO bag;

    QOpenGLShaderProgram *program;
    
#ifndef NDEBUG
    QOpenGLDebugLogger gldebug;
#endif
    
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
    
    bool translating;
    QVector3D translateStartPosition, translateEndPosition;
    QTime translateStartTime;
    
    float heightExaggeration;
    bool adjustingHeightExaggeration;
};

#endif