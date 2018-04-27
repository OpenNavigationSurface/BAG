#include "BagGL.h"
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>
#include <QLabel>
#include <QStatusBar>
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
    lodBias(3),
    statusBar(nullptr),
    statusLabel(new QLabel())
{
    qRegisterMetaType<TilePtr>("TilePtr");
    connect(&bag, SIGNAL(metaLoaded()), this, SLOT(resetView()));
    connect(&bag, SIGNAL(tileLoaded(TilePtr,bool)), this, SLOT(newTile(TilePtr,bool)));
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
    
    eastElevationMapUniform = program->uniformLocation("eastElevationMap");
    eastNormalMapUniform = program->uniformLocation("eastNormalMap");
    eastSpacingUniform = program->uniformLocation("eastSpacing");
    eastLowerLeftUniform = program->uniformLocation("eastLowerLeft");
    eastTileSizeUniform = program->uniformLocation("eastTileSize");
    hasEastUniform = program->uniformLocation("hasEast");
    
    northElevationMapUniform = program->uniformLocation("northElevationMap");
    northNormalMapUniform = program->uniformLocation("northNormalMap");
    northSpacingUniform = program->uniformLocation("northSpacing");
    northLowerLeftUniform = program->uniformLocation("northLowerLeft");
    northTileSizeUniform = program->uniformLocation("northTileSize");
    hasNorthUniform = program->uniformLocation("hasNorth");
    
    northEastElevationMapUniform = program->uniformLocation("northEastElevationMap");
    northEastNormalMapUniform = program->uniformLocation("northEastNormalMap");
    northEastSpacingUniform = program->uniformLocation("northEastSpacing");
    northEastLowerLeftUniform = program->uniformLocation("northEastLowerLeft");
    hasNorthEastUniform = program->uniformLocation("hasNorthEast");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(primitiveReset);
    
    colormaps["topographic"] = QOpenGLTexturePtr(new QOpenGLTexture(QImage(QString(":/colormaps/topographic.png"))));
    colormaps["omnimap"] = QOpenGLTexturePtr(new QOpenGLTexture(QImage(QString(":/colormaps/omnimap.png"))));
    
    glGenVertexArrays(1,&tileVAO);
    glBindVertexArray(tileVAO);

    glGenBuffers(1,&tileBuffer);

    // Verticies get generated and displaced by the Tessellation and Geometry shaders so we only need one token vertex to draw a tile.
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

    // only one token vertex needed to specify a patch for the tessellation engine.
    glPatchParameteri(GL_PATCH_VERTICES,1);
        
    QMatrix4x4 matrix = camera.getMatrix();
    QMatrix4x4 normMatrix = camera.genNormalMatrix();
    
    
    BagIO::MetaData meta = bag.getMeta();
    
    program->setUniformValue(matrixUniform, matrix);
    program->setUniformValue(normMatrixUniform, normMatrix.normalMatrix());
    program->setUniformValue(minElevationUniform, meta.minElevation);
    program->setUniformValue(maxElevationUniform, meta.maxElevation);
    
    program->setUniformValue(colorMapUniform,0);
    program->setUniformValue(elevationMapUniform,1);
    program->setUniformValue(normalMapUniform,2);
    program->setUniformValue(eastElevationMapUniform,3);
    program->setUniformValue(eastNormalMapUniform,4);
    program->setUniformValue(northElevationMapUniform,5);
    program->setUniformValue(northNormalMapUniform,6);
    program->setUniformValue(northEastElevationMapUniform,7);
    program->setUniformValue(northEastNormalMapUniform,8);
    
    QVector3D lightDirection(0.0,0.0,1.0);
    program->setUniformValue(lightDirectionUniform,lightDirection);
    
    colormaps[currentColormap]->bind(0);
    
    glBindVertexArray(tileVAO);
    
    for (auto t: overviewTiles)
        updateLOD(t.second);
    
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
    
    
        //for(TilePtr t: bag.getOverviewTiles())
        for (auto t: overviewTiles)
        {
            drawTile(t.second);
            //break;
        }
    }
    program->release();
}

void BagGL::checkGL(TilePtr t)
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
        t->gl->maxLod = log2f(t->ncols)-1;
        t->gl->lod = t->gl->maxLod;
        t->data.reset();
    }
}

void BagGL::updateLOD(TilePtr t)
{
    checkGL(t);
    float sizeInPix = camera.getSizeInPixels(t->bounds);
    if(sizeInPix>0.0)
    {
        t->gl->lod = std::floor(log2f(t->ncols/sizeInPix));
        t->gl->lod+=lodBias;
        
        if(t->gl->lod < 0 && !t->subTiles.empty())
        {
            for (auto tp: t->subTiles)
            {
                updateLOD(tp.second);
            }
        }
        else
            t->gl->lod = std::max(0, std::min(t->gl->lod,t->gl->maxLod-1));
    }
    else
        t->gl->lod = t->gl->maxLod;
}

void BagGL::drawTile(TilePtr t)
{
    if(t->gl->lod < t->gl->maxLod)
    {
        if(t->gl->lod < 0 && !t->subTiles.empty())
        {
            for (auto tp: t->subTiles)
            {
                drawTile(tp.second);
                //break;
            }
        }
        else
        {
            t->gl->elevations.bind(1);
            t->gl->normals.bind(2);
            program->setUniformValue(spacingUniform,QVector2D(t->dx,t->dy));
            program->setUniformValue(tileSizeUniform,QSize(t->ncols,t->nrows));
            program->setUniformValue(lowerLeftUniform,QVector2D(t->bounds.min().x(),t->bounds.min().y()));
            
            // For a stand alone tile, we would normally have n-1 segments where n is the number of vertices,
            // but we must account for the seam so number of segments == number of vertices
            GLfloat tessLevel = (t->ncols/pow(2.0,t->gl->lod));
            GLfloat tlEast = tessLevel;
            GLfloat tlNorth = tessLevel;
            
            if(t->east)
            {
                checkGL(t->east);
                program->setUniformValue(hasEastUniform,true);
                t->east->gl->elevations.bind(3);
                t->east->gl->normals.bind(4);
                program->setUniformValue(eastSpacingUniform,QVector2D(t->east->dx,t->east->dy));
                program->setUniformValue(eastTileSizeUniform,QSize(t->east->ncols,t->east->nrows));
                program->setUniformValue(eastLowerLeftUniform,QVector2D(t->east->bounds.min().x(),t->east->bounds.min().y()));
                tlEast = (t->east->ncols/pow(2.0,t->east->gl->lod));
            }
            else
                program->setUniformValue(hasEastUniform,false);

            if(t->north)
            {
                checkGL(t->north);
                program->setUniformValue(hasNorthUniform,true);
                t->north->gl->elevations.bind(5);
                t->north->gl->normals.bind(6);
                program->setUniformValue(northSpacingUniform,QVector2D(t->north->dx,t->north->dy));
                program->setUniformValue(northTileSizeUniform,QSize(t->north->ncols,t->north->nrows));
                program->setUniformValue(northLowerLeftUniform,QVector2D(t->north->bounds.min().x(),t->north->bounds.min().y()));
                tlNorth = (t->north->ncols/pow(2.0,t->north->gl->lod));
            }
            else
                program->setUniformValue(hasNorthUniform,false);
            
            if(t->northEast)
            {
                checkGL(t->northEast);
                program->setUniformValue(hasNorthEastUniform,true);
                t->northEast->gl->elevations.bind(7);
                t->northEast->gl->normals.bind(8);
                program->setUniformValue(northEastSpacingUniform,QVector2D(t->northEast->dx,t->northEast->dy));
                program->setUniformValue(northEastLowerLeftUniform,QVector2D(t->northEast->bounds.min().x(),t->northEast->bounds.min().y()));
            }
            else
                program->setUniformValue(hasNorthEastUniform,false);

            // Tessellation parameters used to generate quads.
            // They specify the number of edges for a specific side of the patch.
            // Outer edges can differ from inner ones to help in stitching tiles of different resolution together.
            GLfloat tlInner[2];
            GLfloat tlOuter[4];
            tlInner[0] = tessLevel; // number of inner edges in the y direction.
            tlInner[1] = tessLevel; // number of inner edges in the x direction.
            tlOuter[0] = tessLevel; // number of segments on the left edge.
            tlOuter[1] = tessLevel; // number of segments on the bottom edge.
            tlOuter[2] = tlEast; // number of segments on the right edge.
            tlOuter[3] = tlNorth; // number of segments on the top edge.
            glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL,tlInner);
            glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL,tlOuter);
            
            // Drawing this single vertex launches the Tessellation shader and get expanded into the verticies for the tile. 
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
    QString posText = "Mouse: " + QString::number(event->pos().x())+","+QString::number(event->pos().y());
    
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
        auto swCorner = bag.getMeta().swBottomCorner;
        posText += " World: "+QString::number(swCorner.x()+mousePos.x()/mousePos.w(),'f')+","+QString::number(swCorner.y()+mousePos.y()/mousePos.w(),'f')+","+QString::number(mousePos.z()/mousePos.w(),'f');    
    }
    statusLabel->setText(posText);
    
    
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

void BagGL::newTile(TilePtr tile, bool isVR)
{
    if(isVR)
    {
        u32 ts = bag.getTileSize();
        vrTiles[tile->lowerLeftIndex] = tile;
        TileIndex2D parentIndex(tile->lowerLeftIndex.first/ts,tile->lowerLeftIndex.second/ts);
        if(overviewTiles.count(parentIndex))
            overviewTiles[parentIndex]->subTiles[tile->index]=tile;
        TileIndex2D east(tile->lowerLeftIndex.first+1,tile->lowerLeftIndex.second);
        if(vrTiles.count(east))
            tile->east = vrTiles[east];
        TileIndex2D north(tile->lowerLeftIndex.first,tile->lowerLeftIndex.second+1);
        if(vrTiles.count(north))
            tile->north = vrTiles[north];
        TileIndex2D northEast(tile->lowerLeftIndex.first+1,tile->lowerLeftIndex.second+1);
        if(vrTiles.count(northEast))
            tile->northEast = vrTiles[northEast];
        TileIndex2D west(tile->lowerLeftIndex.first-1,tile->lowerLeftIndex.second);
        if(vrTiles.count(west))
            vrTiles[west]->east = tile;
        TileIndex2D south(tile->lowerLeftIndex.first,tile->lowerLeftIndex.second-1);
        if(vrTiles.count(south))
            vrTiles[south]->north = tile;
        TileIndex2D southWest(tile->lowerLeftIndex.first-1,tile->lowerLeftIndex.second-1);
        if(vrTiles.count(southWest))
            vrTiles[southWest]->northEast = tile;
    }
    else
    {
        overviewTiles[tile->index] = tile;
        TileIndex2D east(tile->index.first+1,tile->index.second);
        if(overviewTiles.count(east))
            tile->east = overviewTiles[east];
        TileIndex2D north(tile->index.first,tile->index.second+1);
        if(overviewTiles.count(north))
            tile->north = overviewTiles[north];
        TileIndex2D northEast(tile->index.first+1,tile->index.second+1);
        if(overviewTiles.count(northEast))
            tile->northEast = overviewTiles[northEast];
        TileIndex2D west(tile->index.first-1,tile->index.second);
        if(overviewTiles.count(west))
            overviewTiles[west]->east = tile;
        TileIndex2D south(tile->index.first,tile->index.second-1);
        if(overviewTiles.count(south))
            overviewTiles[south]->north = tile;
        TileIndex2D southWest(tile->index.first-1,tile->index.second-1);
        if(overviewTiles.count(southWest))
            overviewTiles[southWest]->northEast = tile;
    }
    renderLater();
}

void BagGL::setStatusBar(QStatusBar* sb)
{
    statusBar = sb;
    statusBar->addWidget(statusLabel);
}


