#include "BagGL.h"
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>

#include <iostream>

const GLuint BagGL::primitiveReset = 0xffffffff;

BagGL::BagGL(): 
    program(0),
#ifndef NDEBUG
    gldebug(this),
#endif
    currentColormap("omnimap"),
    drawStyle("solid"),
    nearPlane(1.0),
    farPlane(100.0),
    zoom(1.0),
    yaw(0.0),
    pitch(30.0),
    rotating(false),
    translatePosition(0.0,0.0,0.0),
    translating(false),
    heightExaggeration(1.0),
    adjustingHeightExaggeration(false)
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
    std::cerr << "GL debug? " << gldebug.initialize() << std::endl;
    connect(&gldebug, SIGNAL(messageLogged(const QOpenGLDebugMessage &)),this,SLOT(messageLogged(const QOpenGLDebugMessage &)));
    gldebug.startLogging();
#endif
    
    printFormat();
    
    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex.glsl");
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
            if(row>0)
            {
                indecies.push_back(((row-1)*ts+col));
                indecies.push_back((row*ts+col));
            }
        }
        if(row>0)
            indecies.push_back(primitiveReset);
    }
    glBindBuffer(GL_ARRAY_BUFFER, tileBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*verts.size(),verts.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileBuffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indecies.size(),indecies.data(),GL_STATIC_DRAW);
    tileIndeciesCount = indecies.size();
    glBindAttribLocation(program->programId(),0,"inPosition");
}

QMatrix4x4 BagGL::genMatrix()
{
    QMatrix4x4 matrix;
    matrix.perspective(60.0f, width()/float(height()), nearPlane, farPlane);
    matrix.translate(0, 0, -5);
    matrix.rotate(-90, 1, 0, 0);
    matrix.scale(zoom,zoom,zoom);
    matrix.rotate(pitch, 1, 0, 0);
    matrix.rotate(yaw, 0, 0, 1);
    matrix.scale(1.0,1.0,heightExaggeration);
    matrix.translate(-translatePosition.x(),-translatePosition.y(),-translatePosition.z());
    return matrix;
}

void BagGL::render()
{
    if(translating)
    {
        float p = translateStartTime.elapsed()/250.0;
        if(p >= 1.0)
        {
            translatePosition = translateEndPosition;
            translating = false;
            setAnimating(rotating||translating||adjustingHeightExaggeration);
        }
        else
        {
            float ip = 1.0-p;
            translatePosition = translateStartPosition*ip+translateEndPosition*p;
        }
    }
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    program->bind();
    
    QMatrix4x4 matrix = genMatrix();
    QMatrix4x4 normMatrix;
    normMatrix.scale(1.0,1.0,heightExaggeration);
    
    Frustum f(matrix);
    f.viewportSize = QVector2D(width() * retinaScale, height() * retinaScale);
    
    BagIO::MetaData meta = bag.getMeta();
    
    program->setUniformValue(matrixUniform, matrix);
    program->setUniformValue(normMatrixUniform, normMatrix.normalMatrix());
    program->setUniformValue(minElevationUniform, meta.minElevation);
    program->setUniformValue(maxElevationUniform, meta.maxElevation);
    
    program->setUniformValue(spacingUniform,QVector2D(meta.dx,meta.dy));

    program->setUniformValue(elevationMapUniform,0);
    program->setUniformValue(colorMapUniform,1);
    program->setUniformValue(normalMapUniform,2);
    
    GLuint tileSize = bag.getTileSize();
    program->setUniformValue(tileSizeUniform,tileSize);
    
    QVector3D lightDirection(0.0,0.0,1.0);
    program->setUniformValue(lightDirectionUniform,lightDirection);
    
    colormaps[currentColormap]->bind(1);
    
    glBindVertexArray(tileVAO);
    
    uint cullCount = 0;
    uint drawCount = 0;
    
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
        }
        bool culled = isCulled(f,*t,meta);
        //std::cerr << "tile: " << t->index.first << "," << t->index.second << " culled? " << culled << std::endl;
        if(!culled)
        {
            drawCount++;
            t->gl->elevations.bind(0);
            t->gl->normals.bind(2);
            QVector2D ll(t->index.first*meta.dx*tileSize,t->index.second*meta.dy*tileSize);
            program->setUniformValue(lowerLeftUniform,ll);
            if(drawStyle == "solid")
                glDrawElements(GL_TRIANGLE_STRIP, tileIndeciesCount,GL_UNSIGNED_INT,0);
            if(drawStyle == "wireframe")
                glDrawElements(GL_LINE_STRIP, tileIndeciesCount,GL_UNSIGNED_INT,0);
            if(drawStyle == "points")
                glDrawArrays(GL_POINTS, 0, tileSize*tileSize);
        }
        else
            cullCount++;
        //break;
    }
    std::cerr << drawCount << " drawn, " << cullCount << " culled." << std::endl;
    //std::cerr << std::endl;
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
        QMatrix4x4 matrix = genMatrix().inverted();
        QVector4D mousePos(gx,gy,gz,1.0);
        mousePos = matrix*mousePos;
        if (gz < 1.0)
        {
            translating = true;
            translateStartPosition = translatePosition;
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
        pitch += dy;
        pitch = std::max(-90.0f,std::min(90.0f,pitch));
        yaw += dx;
    }
    if(adjustingHeightExaggeration)
    {
        int dy = event->pos().y()- lastPosition.y();
        lastPosition = event->pos();
        heightExaggeration -= dy/10.0;
        heightExaggeration = std::max(1.0f,std::min(heightExaggeration,500.0f));
    }
}

void BagGL::wheelEvent(QWheelEvent* event)
{
    if(event->angleDelta().y() > 0)
        zoom *= 1.3f;
    else
        zoom /= 1.3f;
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
    bool ret = bag.open(bagFileName);
    if(ret)
        resetView();
    return ret;
}

void BagGL::closeBag()
{
    for(auto t: bag.getOverviewTiles())
        t->gl.reset();
    bag.close();
}

void BagGL::resetView()
{
    BagIO::MetaData meta = bag.getMeta();
    translatePosition = meta.size/2.0;
    translatePosition.setZ(0.0);
    pitch = 30.0;
    yaw = 0.0;
    float maxDim = std::max(meta.size.x(),meta.size.y());
    if(maxDim > 0.0)
        zoom = 2/maxDim;
    else
        zoom = 1.0;
    
    heightExaggeration = 1.0;
    renderLater();
}

void BagGL::setColormap(const std::string& cm)
{
    currentColormap = cm;
    renderLater();
}

void BagGL::setDrawStyle(const std::string& ds)
{
    drawStyle = ds;
    renderLater();
}

void BagGL::messageLogged(const QOpenGLDebugMessage& debugMessage)
{
    std::cerr << debugMessage.message().toStdString() << std::endl;
}

bool BagGL::isCulled(const BagGL::Frustum& f, const Tile& t, BagIO::MetaData const &meta) const
{
//     std::cerr << "tile: " << t.index.first << "," << t.index.second;
//     std::cerr << "\televation range: " << t.minElevation << ", " << t.maxElevation << std::endl;
    QVector3D p0 = t.lowerLeft;
    QVector3D p7 = t.upperRight;
    QVector3D p1(p7.x(),p0.y(),p0.z());
    QVector3D p2(p0.x(),p7.y(),p0.z());
    QVector3D p3(p7.x(),p7.y(),p0.z());
    QVector3D p4(p0.x(),p0.y(),p7.z());
    QVector3D p5(p7.x(),p0.y(),p7.z());
    QVector3D p6(p0.x(),p7.y(),p7.z());
    
    if(f.n.whichSide(p0)<0.0 && f.n.whichSide(p1)<0.0 && f.n.whichSide(p2)<0.0 && f.n.whichSide(p3)<0.0
        && f.n.whichSide(p4)<0.0 && f.n.whichSide(p5)<0.0 && f.n.whichSide(p6)<0.0 && f.n.whichSide(p7)<0.0)
        return true;

    if(f.f.whichSide(p0)<0.0 && f.f.whichSide(p1)<0.0 && f.f.whichSide(p2)<0.0 && f.f.whichSide(p3)<0.0
        && f.f.whichSide(p4)<0.0 && f.f.whichSide(p5)<0.0 && f.f.whichSide(p6)<0.0 && f.f.whichSide(p7)<0.0)
        return true;
    
    if(f.l.whichSide(p0)<0.0 && f.l.whichSide(p1)<0.0 && f.l.whichSide(p2)<0.0 && f.l.whichSide(p3)<0.0
        && f.l.whichSide(p4)<0.0 && f.l.whichSide(p5)<0.0 && f.l.whichSide(p6)<0.0 && f.l.whichSide(p7)<0.0)
        return true;
    
    if(f.r.whichSide(p0)<0.0 && f.r.whichSide(p1)<0.0 && f.r.whichSide(p2)<0.0 && f.r.whichSide(p3)<0.0
        && f.r.whichSide(p4)<0.0 && f.r.whichSide(p5)<0.0 && f.r.whichSide(p6)<0.0 && f.r.whichSide(p7)<0.0)
        return true;
    
    if(f.t.whichSide(p0)<0.0 && f.t.whichSide(p1)<0.0 && f.t.whichSide(p2)<0.0 && f.t.whichSide(p3)<0.0
        && f.t.whichSide(p4)<0.0 && f.t.whichSide(p5)<0.0 && f.t.whichSide(p6)<0.0 && f.t.whichSide(p7)<0.0)
        return true;
    
    if(f.b.whichSide(p0)<0.0 && f.b.whichSide(p1)<0.0 && f.b.whichSide(p2)<0.0 && f.b.whichSide(p3)<0.0
        && f.b.whichSide(p4)<0.0 && f.b.whichSide(p5)<0.0 && f.b.whichSide(p6)<0.0 && f.b.whichSide(p7)<0.0)
        return true;
    
    return false;
}
