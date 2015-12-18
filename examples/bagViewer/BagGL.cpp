#include "BagGL.h"
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>
#include <cmath>

const GLuint BagGL::primitiveReset = 0xffffffff;

BagGL::BagGL(): 
    program(0),
    polygonMode(GL_FILL),
#ifndef NDEBUG
    gldebug(this),
#endif
    currentColormap("omnimap"),
    rotating(false),
    translating(false),
    adjustingHeightExaggeration(false),
    lodBias(3)
{
    connect(&bag, SIGNAL(metaLoaded()), this, SLOT(resetView()));
    connect(&bag, SIGNAL(tileLoaded()), this, SLOT(renderLater()));
}

BagGL::~BagGL()
{
    closeBag();
}

void BagGL::initialize()
{

#ifndef NDEBUG
    qDebug() << "GL debug? " << gldebug.initialize();
    connect(&gldebug, SIGNAL(messageLogged(const QOpenGLDebugMessage &)),this,SLOT(messageLogged(const QOpenGLDebugMessage &)));
    gldebug.startLogging();
#endif
    
    printFormat();
    
    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex.glsl");
    program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/tes.glsl");
    program->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/geometry.glsl");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment,":/fragment.glsl");
    
    program->link();

    matrixUniform = program->uniformLocation("matrix");
    normMatrixUniform = program->uniformLocation("normMatrix");
    lightDirectionUniform = program->uniformLocation("lightDirection");
    minElevationUniform = program->uniformLocation("minElevation");
    maxElevationUniform = program->uniformLocation("maxElevation");
    
    elevationMapUniform = program->uniformLocation("elevationMap");
    colorMapUniform = program->uniformLocation("colorMap");
    normalMapUniform = program->uniformLocation("normalMap");
    spacingUniform = program->uniformLocation("spacing");
    lowerLeftUniform = program->uniformLocation("lowerLeft");
    tileSizeUniform = program->uniformLocation("tileSize");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(primitiveReset);
    
    colormaps["topographic"] = QOpenGLTexturePtr(new QOpenGLTexture(QImage(QString(":/colormaps/topographic.png"))));
    colormaps["omnimap"] = QOpenGLTexturePtr(new QOpenGLTexture(QImage(QString(":/colormaps/omnimap.png"))));
    
    glGenVertexArrays(1,&tileVAO);
    glBindVertexArray(tileVAO);

    glGenBuffers(1,&tileBuffer);

    GLfloat vert[2] = {0.0,0.0};
    
    glBindBuffer(GL_ARRAY_BUFFER, tileBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*2,vert,GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindAttribLocation(program->programId(),0,"inPosition");
}


void BagGL::render(bool picking)
{
    if(translating)
    {
        float p = translateStartTime.elapsed()/250.0;
        if(p >= 1.0)
        {
            camera.setCenterPosition(translateEndPosition);
            translating = false;
            setAnimating(rotating||translating||adjustingHeightExaggeration);
        }
        else
        {
            float ip = 1.0-p;
            camera.setCenterPosition(translateStartPosition*ip+translateEndPosition*p);
        }
    }
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    camera.setViewport(width() * retinaScale, height() * retinaScale);
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    int passes = 1;
    if(picking && polygonMode != GL_FILL)
        passes = 2;
    
    program->bind();
    
    glPatchParameteri(GL_PATCH_VERTICES,1);
        
    QMatrix4x4 matrix = camera.getMatrix();
    QMatrix4x4 normMatrix = camera.genNormalMatrix();
    
    
    BagIO::MetaData meta = bag.getMeta();
    
    program->setUniformValue(matrixUniform, matrix);
    program->setUniformValue(normMatrixUniform, normMatrix.normalMatrix());
    program->setUniformValue(minElevationUniform, meta.minElevation);
    program->setUniformValue(maxElevationUniform, meta.maxElevation);
    
    
    program->setUniformValue(elevationMapUniform,0);
    program->setUniformValue(colorMapUniform,1);
    program->setUniformValue(normalMapUniform,2);
    
    GLuint tileSize = bag.getTileSize();
    program->setUniformValue(tileSizeUniform,tileSize);
    
    QVector3D lightDirection(0.0,0.0,1.0);
    program->setUniformValue(lightDirectionUniform,lightDirection);
    
    colormaps[currentColormap]->bind(1);
    
    glBindVertexArray(tileVAO);
    
    
    for(int pass = 0; pass < passes; ++pass)
    {
        if(pass == 1)
        {
            glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
            glColorMaski(0,GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK,polygonMode);
            glColorMaski(0,GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
        }
    
    
        for(TilePtr t: bag.getOverviewTiles())
        {
            drawTile(t);
            //break;
        }
    }
    program->release();
}

void BagGL::drawTile(TilePtr t)
{
    float sizeInPix = camera.getSizeInPixels(t->bounds);
    if(sizeInPix>0.0)
    {
        if(!t->gl)
        {
            t->gl = std::make_shared<TileGL>();
            t->gl->elevations.setSize(t->ncols,t->nrows);
            t->gl->elevations.setFormat(QOpenGLTexture::R32F);
            t->gl->elevations.allocateStorage();
            t->gl->elevations.setData(QOpenGLTexture::Red,QOpenGLTexture::Float32,t->data->elevations.data());
            t->gl->elevations.setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);
            t->gl->elevations.setWrapMode(QOpenGLTexture::ClampToEdge);
            t->gl->normals.setData(t->data->normalMap);
            t->gl->normals.setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);
            t->gl->normals.setWrapMode(QOpenGLTexture::ClampToEdge);
            
            t->data.reset();
        }
        
        program->setUniformValue(spacingUniform,QVector2D(t->dx,t->dy));
        program->setUniformValue(tileSizeUniform,t->ncols);
        
        int lod = std::floor(log2f(t->ncols/sizeInPix));
        //qDebug() << "vrLOD (pre-bias):" << vrlod;
        
        //std::cerr << "vr draw size (px): " << vrMaxDS << ", preliminary VR lod: " << vrlod << std::endl;
        lod+=lodBias;
        
        if(lod < 0 && !t->subTiles.empty())
        {
            for (auto tp: t->subTiles)
            {
                drawTile(tp.second);
                //break;
            }
        }
        else
        {
            lod = std::max(0, std::min(lod,int(log2f(t->ncols)-2)));
            
            //qDebug() << "clamped:" << vrlod;
            
            t->gl->elevations.bind(0);
            t->gl->normals.bind(2);
            QVector2D ll(t->bounds.min().x(),t->bounds.min().y());
            program->setUniformValue(lowerLeftUniform,ll);
            
            GLfloat tl = (t->ncols/pow(2.0,lod)) -1.0;
            
            GLfloat tlOuter[4];
            GLfloat tlInner[2];
            tlOuter[0] = tl;
            tlOuter[1] = tl;
            tlOuter[2] = tl;
            tlOuter[3] = tl;
            tlInner[0] = tl;
            tlInner[1] = tl;
            glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL,tlOuter);
            glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL,tlInner);
            
            glDrawArrays(GL_PATCHES,0,1);
        }
    }
}


void BagGL::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        rotating= true;
        lastPosition = event->pos();
    }
    if(event->button() == Qt::MiddleButton)
    {
        int mx = event->pos().x();
        int my = height()-event->pos().y();
        float gx = -1.0+mx/(width()/2.0);
        float gy = -1.0+my/(height()/2.0);
        if(polygonMode != GL_FILL)
            renderNow(true);
        GLfloat gz;
        glReadPixels(mx, my, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &gz);
        gz = (gz-.5)*2.0;
        QMatrix4x4 matrix = camera.getMatrix().inverted();
        QVector4D mousePos(gx,gy,gz,1.0);
        mousePos = matrix*mousePos;
        if (gz < 1.0)
        {
            translating = true;
            translateStartPosition = camera.getCenterPosition();
            translateEndPosition = QVector3D(mousePos.x()/mousePos.w(),mousePos.y()/mousePos.w(),mousePos.z()/mousePos.w());
            translateStartTime.start();
        }
    }
    if(event->button() == Qt::RightButton)
    {
        adjustingHeightExaggeration = true;
        lastPosition = event->pos();
    }
    setAnimating(rotating||translating||adjustingHeightExaggeration);
}

void BagGL::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        rotating = false;
    }
    
    if(event->button() == Qt::MiddleButton)
    {
        //translating = false;
    }
    if(event->button() == Qt::RightButton)
    {
        adjustingHeightExaggeration = false;
    }
    setAnimating(rotating||translating||adjustingHeightExaggeration);
    
}

void BagGL::mouseMoveEvent(QMouseEvent* event)
{
    if(rotating)
    {
        int dx = event->pos().x() - lastPosition.x();
        int dy = event->pos().y()- lastPosition.y();
        lastPosition = event->pos();
        camera.setPitch(camera.getPitch() + dy);
        camera.setYaw(camera.getYaw() + dx);
    }
    if(adjustingHeightExaggeration)
    {
        int dy = event->pos().y()- lastPosition.y();
        lastPosition = event->pos();
        camera.setHeightExaggeration(camera.getHeightExaggeration() - dy/10.0);
    }
}

void BagGL::wheelEvent(QWheelEvent* event)
{
    if(event->angleDelta().y() > 0)
        camera.setZoom(camera.getZoom() * 1.3f);
    else
        camera.setZoom(camera.getZoom() / 1.3f);
    renderLater();
}

void BagGL::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
    case Qt::Key_R:
        resetView();
        break;
    case Qt::Key_BracketLeft:
        lodBias--;
        qDebug() << "lod bias:" << lodBias;
        renderLater();
        break;
    case Qt::Key_BracketRight:
        lodBias++;
        qDebug() << "lod bias:" << lodBias;
        renderLater();
        break;
    default:
        event->ignore();
    }
}


bool BagGL::openBag(const QString& bagFileName)
{
    closeBag();
    bool ret = bag.open(bagFileName);
    if(ret)
        resetView();
    return ret;
}

void BagGL::closeBag()
{
//     for(TilePtr t: bag.getOverviewTiles())
//     {
//         t->gl.reset();
//     }
    bag.close();
}

void BagGL::resetView()
{
    BagIO::MetaData meta = bag.getMeta();
    QVector3D p = meta.size/2.0;
    p.setZ(0.0);
    camera.setCenterPosition(p);
    camera.setPitch(30.0);
    camera.setYaw(0.0);
    float maxDim = std::max(meta.size.x(),meta.size.y());
    if(maxDim > 0.0)
        camera.setZoom(2/maxDim);
    else
        camera.setZoom(1.0);
    
    camera.setHeightExaggeration(1.0);
    renderLater();
}

void BagGL::setColormap(const std::string& cm)
{
    currentColormap = cm;
    renderLater();
}

void BagGL::setDrawStyle(const std::string& ds)
{
    if(ds == "solid")
        polygonMode = GL_FILL;
    if(ds == "wireframe")
        polygonMode = GL_LINE;
    if(ds == "points")
        polygonMode = GL_POINT;
    renderLater();
}

void BagGL::messageLogged(const QOpenGLDebugMessage& debugMessage)
{
    qDebug() << debugMessage.message();
}


