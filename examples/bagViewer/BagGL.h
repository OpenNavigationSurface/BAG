#ifndef BAGGL_H
#define BAGGL_H

#include <QOpenGLWidget>
#include "GLCamera.h"
#include "BagIO.h"
#include <QVector3D>
#include <QTime>
#include <memory>
#include <QOpenGLTexture>
#include <QOpenGLDebugLogger>
#include <QMatrix4x4>
#include <QVector2D>
#include "Bounds.h"
#include <QOpenGLShader>
#include <QtGui/QOpenGLFunctions_4_0_Core>

class QStatusBar;
class QLabel;

struct TileGL
{
    QOpenGLTexture elevations;
    QOpenGLTexture normals;
    QOpenGLTexture uncertainties;
    int lod;
    int maxLod;
    TileGL(): elevations(QOpenGLTexture::Target2D), normals(QOpenGLTexture::Target2D), uncertainties(QOpenGLTexture::Target2D) {}
    ~TileGL(){}
};


class BagGL: public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core
{
    Q_OBJECT
    
public:
    BagGL(QWidget *parent=nullptr);
    ~BagGL();
    
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    
    void render(bool picking=false);
    
    bool openBag(QString const &bagFileName);
    void closeBag();
    void setColormap(std::string const &cm);
    void setDrawStyle(std::string const &ds);
    
    static const GLuint primitiveReset;

    
    typedef std::pair<uint,uint> GridSize;
    
    void setStatusBar(QStatusBar *sb);
    
    
public slots:
    void resetView();
    void messageLogged(const QOpenGLDebugMessage & debugMessage);
    void newTile(TilePtr tile, bool isVR);
    void checkAnimation();
    
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    
private:
    void checkGL(TilePtr t);
    void updateLOD(TilePtr t);
    void drawTile(TilePtr t);
    
    GLCamera camera;
    
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
    GLuint normalMapUniform;

    GLuint eastElevationMapUniform;
    GLuint eastNormalMapUniform;
    GLuint eastSpacingUniform;
    GLuint eastLowerLeftUniform;
    GLuint eastTileSizeUniform;
    GLuint hasEastUniform;
    
    GLuint northElevationMapUniform;
    GLuint northNormalMapUniform;
    GLuint northSpacingUniform;
    GLuint northLowerLeftUniform;
    GLuint northTileSizeUniform;
    GLuint hasNorthUniform;
    
    GLuint northEastElevationMapUniform;
    GLuint northEastNormalMapUniform;
    GLuint northEastSpacingUniform;
    GLuint northEastLowerLeftUniform;
    GLuint hasNorthEastUniform;
    
    GLuint tileVAO;
    GLuint tileBuffer;
    
    BagIO bag;

    QOpenGLShaderProgram *program;

    GLuint polygonMode;
    
#ifndef NDEBUG
    QOpenGLDebugLogger gldebug;
#endif
    
    typedef std::shared_ptr<QOpenGLTexture> QOpenGLTexturePtr;
    typedef std::map<std::string,QOpenGLTexturePtr> ColormapMap;
    ColormapMap colormaps;
    std::string currentColormap;
    
    bool rotating;
    QPoint lastPosition;
    
    
    bool translating;
    QVector3D translateStartPosition, translateEndPosition;
    QTime translateStartTime;
    
    bool adjustingHeightExaggeration;
    
    int lodBias;
    
    TileMap overviewTiles;
    TileMap vrTiles;
    
    QStatusBar * statusBar;
    QLabel * statusLabel;
    
    bool m_animating;
    
};

#endif
