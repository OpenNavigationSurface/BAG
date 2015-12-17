#include "BagGL.h"
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>

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
    lodBias(5)
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

    glGenBuffers(2,tileBuffers);

    u32 ts = bag.getTileSize();
    std::vector<GLfloat> verts;
    std::vector<GLuint> indecies;
    for(u32 row=0; row < ts; ++row)
    {
        for(u32 col=0; col < ts; ++col)
        {
            verts.push_back(col);
            verts.push_back(row);
        }
    }
    
    for(uint lod=1; lod < ts/2; lod*=2)
    {
        lodIndecies.push_back(static_cast<int>(indecies.size()));
        for(u32 row=0; row < ts; row+=lod)
        {
            for(u32 col=0; col < ts; col+=lod)
            {
                if(row>0)
                {
                    indecies.push_back(((row-lod)*ts+col));
                    indecies.push_back((row*ts+col));
                    if(col+lod >= ts && col < ts-1)
                    {
                        indecies.push_back(((row-lod)*ts+ts-1));
                        indecies.push_back((row*ts+ts-1));
                    }
                }
            }
            if(row>0)
                indecies.push_back(primitiveReset);
            if (row+lod >= ts && row < ts-1)
                row = ts-1-lod;
        }
    }
    lodIndecies.push_back(static_cast<int>(indecies.size()));
    
    
    glBindBuffer(GL_ARRAY_BUFFER, tileBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*verts.size(),verts.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileBuffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indecies.size(),indecies.data(),GL_STATIC_DRAW);
    glBindAttribLocation(program->programId(),0,"inPosition");
}


void BagGL::render()
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
    
    glPolygonMode(GL_FRONT_AND_BACK,polygonMode);
    
    program->bind();
    
    
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
    
    for(TilePtr t: bag.getOverviewTiles())
    {
        if(!t->gl)
        {
            t->gl = std::make_shared<TileGL>();
            t->gl->elevations.setSize(tileSize,tileSize);
            t->gl->elevations.setFormat(QOpenGLTexture::R32F);
            t->gl->elevations.allocateStorage();
            t->gl->elevations.setData(QOpenGLTexture::Red,QOpenGLTexture::Float32,t->g.elevations.data());
            t->g.elevations.resize(0);
            t->gl->elevations.setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);
            t->gl->normals.setData(t->g.normalMap);
            //t->g.normalMap.save("debugNormalMap.png");
            t->gl->normals.setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);
        }
        float sizeInPix = camera.getSizeInPixels(t->bounds);
        //qDebug() << "tile: " << t->index.first << "," << t->index.second << "size (px)" << sizeInPix;
        if(sizeInPix>0.0)
        {
            //float pixelSize = camera.getPixelSize(t->bounds);
            //qDebug() << "eye distance:" << eyed << "pixel size:" << tpixSize;
            
            int lod = std::floor(log2f(tileSize/sizeInPix));
            //qDebug() << "LOD (pre-bias):" << lod;
            
            lod+=lodBias;
            
            if(lod < 0 && !t->varResTiles.empty())
            {
                for (auto vrtp: t->varResTiles)
                {
                    VarResTilePtr vrt = vrtp.second;
                    
                    float vrSizeInPix = camera.getSizeInPixels(vrt->bounds);
                    //qDebug() << "vrTile" << vrt->index.first << "," << vrt->index.second << "size (px)" << vrSizeInPix;
                    if(vrSizeInPix>0.0)
                    {
                        if(!vrt->gl)
                        {
                            vrt->gl = std::make_shared<TileGL>();
                            vrt->gl->elevations.setSize(vrt->ncols,vrt->nrows);
                            vrt->gl->elevations.setFormat(QOpenGLTexture::R32F);
                            vrt->gl->elevations.allocateStorage();
                            vrt->gl->elevations.setData(QOpenGLTexture::Red,QOpenGLTexture::Float32,vrt->g.elevations.data());
                            vrt->g.elevations.resize(0);
                            vrt->gl->elevations.setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);
                            vrt->gl->normals.setData(vrt->g.normalMap);
                            //vrt->g.normalMap.save("debugNormalMap.png");
                            vrt->gl->normals.setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);
                        }
                        
                        program->setUniformValue(spacingUniform,QVector2D(vrt->dx,vrt->dy));
                        program->setUniformValue(tileSizeUniform,vrt->ncols);
                        
                        int vrlod = std::floor(log2f(vrt->ncols/vrSizeInPix));
                        //qDebug() << "vrLOD (pre-bias):" << vrlod;
                        
                        //std::cerr << "vr draw size (px): " << vrMaxDS << ", preliminary VR lod: " << vrlod << std::endl;
                        vrlod+=lodBias;
                        
                        
                        GridSize gs(vrt->ncols,vrt->nrows);
                        if(!grids[gs])
                        {
                            grids[gs] = GridPtr(new Grid);
                            grids[gs]->initialize(*this,vrt->ncols,vrt->nrows);
                        }
                        GridPtr g = grids[gs];
                        
                        glBindVertexArray(g->vao);
                        
//                         int maxLOD = g->lodIndecies.size()-2;
//                         int rlod = maxLOD+lod;
//                         rlod = std::max(0,rlod);
                        vrlod = std::max(0, std::min(vrlod,int(g->lodIndecies.size()-2)));
                        
                        //qDebug() << "clamped:" << vrlod;
                        
                        
                        GLsizei i0 = g->lodIndecies[vrlod];
                        GLsizei count = g->lodIndecies[vrlod+1]-g->lodIndecies[vrlod];
                        
                        //std::cerr << "lod: " << lod << "\ti0 " << i0 << "\tcount " << count << std::endl;
                        
                        vrt->gl->elevations.bind(0);
                        vrt->gl->normals.bind(2);
                        QVector2D ll(vrt->bounds.min().x(),vrt->bounds.min().y());
                        program->setUniformValue(lowerLeftUniform,ll);
                        
                        glDrawElements(GL_TRIANGLE_STRIP, count,GL_UNSIGNED_INT,(void*)(i0*sizeof(GLuint)));
                        //break;
                    }
                }
            }
            else
            {
                glBindVertexArray(tileVAO);
                program->setUniformValue(tileSizeUniform,tileSize);
                program->setUniformValue(spacingUniform,QVector2D(meta.dx,meta.dy));
                
                lod = std::max(0, std::min(lod,int(lodIndecies.size()-2)));
                
                //qDebug() << " clamped: " << lod;

                GLsizei i0 = lodIndecies[lod];
                GLsizei count = lodIndecies[lod+1]-lodIndecies[lod];

                //std::cerr << "lod: " << lod << "\ti0 " << i0 << "\tcount " << count << std::endl;
                
                t->gl->elevations.bind(0);
                t->gl->normals.bind(2);
                QVector2D ll(t->bounds.min().x(),t->bounds.min().y());
                program->setUniformValue(lowerLeftUniform,ll);
                glDrawElements(GL_TRIANGLE_STRIP, count,GL_UNSIGNED_INT,(void*)(i0*sizeof(GLuint)));
            }
        }
        //break;
    }
    program->release();
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
        renderNow();
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


void BagGL::Grid::initialize(BagGL& gl, uint ncols, uint nrows)
{
    qDebug() << "grid init: " << ncols << "," << nrows;
    gl.glGenVertexArrays(1,&vao);
    gl.glBindVertexArray(vao);
    
    gl.glGenBuffers(2,buffers);

    std::vector<GLfloat> verts;
    std::vector<GLuint> indecies;
    for(u32 row=0; row < nrows; ++row)
    {
        for(u32 col=0; col < ncols; ++col)
        {
            verts.push_back(col);
            verts.push_back(row);
        }
    }
    uint maxDim = std::max(ncols,nrows);
    for(uint lod=1; lod < maxDim; lod*=2)
    {
        lodIndecies.push_back(static_cast<int>(indecies.size()));
        for(u32 row=0; row < nrows; row+=lod)
        {
            for(u32 col=0; col < ncols; col+=lod)
            {
                if(row>0)
                {
                    indecies.push_back(((row-lod)*ncols+col));
                    indecies.push_back((row*ncols+col));
                    if(col+lod >= ncols && col < ncols-1)
                    {
                        indecies.push_back(((row-lod)*ncols+ncols-1));
                        indecies.push_back((row*ncols+ncols-1));
                    }
                }
            }
            if(row>0)
                indecies.push_back(BagGL::primitiveReset);
            if (row+lod >= nrows && row < nrows-1)
                row = nrows-1-lod;
        }
    }
    lodIndecies.push_back(static_cast<int>(indecies.size()));
    
    gl.glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    gl.glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*verts.size(),verts.data(),GL_STATIC_DRAW);
    gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    gl.glEnableVertexAttribArray(0);
    
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indecies.size(),indecies.data(),GL_STATIC_DRAW);
}

