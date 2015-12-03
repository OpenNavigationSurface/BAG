#include "BagGL.h"
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>

#include <iostream>

static const char *vertexShaderSource =
"attribute highp vec4 posAttr;\n"
"attribute float uncertainty;\n"
"attribute vec3 normal;\n"
"varying float elevation;\n"
"varying float lighting;\n"
"varying float vsUncertainty;\n"
"uniform highp mat4 matrix;\n"
"uniform highp mat3 normMatrix;\n"
"uniform vec3 lightDirection;\n"
"uniform float minElevation;\n"
"uniform float maxElevation;\n"
"void main() {\n"
"   vec3 vsNormal = normalize(normMatrix*normal);\n"
"   lighting = max(dot(vsNormal,lightDirection), 0.0);\n"
"   vsUncertainty = uncertainty;\n"
"   elevation = (posAttr.z-minElevation)/(maxElevation-minElevation);\n"
"   gl_Position = matrix * posAttr;\n"
"}\n";

static const char *fragmentShaderSource =
"varying float lighting;\n"
"varying float elevation;\n"
"uniform sampler2D texture;\n"
"void main() {\n"
"   vec2 tc;\n"
"   tc.x = elevation;\n"
"   tc.y = .5;\n"
"   vec4 color = texture2D(texture,tc);\n"
"   color.rgb *= lighting;\n"
"   color.a = 1.0;\n"
"   gl_FragColor = color;\n"
"}\n";

BagGL::BagGL(): 
    primitiveReset(0xffffffff),
    bag(primitiveReset),
    program(0),
    currentColormap("omnimap"),
    drawStyle("solid"),
    nearPlane(1.0),
    farPlane(100.0),
    zoom(1.0),
    yaw(0.0),
    pitch(30.0),
    rotating(false),
    translatePosition(0.0,0.0,0.0),
    defaultZoom(1.0),
    translating(false),
    heightExaggeration(1.0),
    adjustingHeightExaggeration(false)
{

}

BagGL::~BagGL()
{
    closeBag();
}


GLuint BagGL::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void BagGL::initialize()
{
    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    program->link();
    posAttr = program->attributeLocation("posAttr");
    unAttr = program->attributeLocation("uncertainty");
    normAttr = program->attributeLocation("normal");
    matrixUniform = program->uniformLocation("matrix");
    normMatrixUniform = program->uniformLocation("normMatrix");
    lightDirectionUniform = program->uniformLocation("lightDirection");
    minElevationUniform = program->uniformLocation("minElevation");
    maxElevationUniform = program->uniformLocation("maxElevation");
    glEnable(GL_DEPTH_TEST);
    glPointSize(2.0);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(primitiveReset);
    
    colormaps["topographic"] = QOpenGLTexturePtr(new QOpenGLTexture(QImage(QString(":/colormaps/topographic.png"))));
    colormaps["omnimap"] = QOpenGLTexturePtr(new QOpenGLTexture(QImage(QString(":/colormaps/omnimap.png"))));
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
    
    program->setUniformValue(matrixUniform, matrix);
    program->setUniformValue(normMatrixUniform, normMatrix.normalMatrix());
    program->setUniformValue(minElevationUniform, bag.minElevation);
    program->setUniformValue(maxElevationUniform, bag.maxElevation);
    
    QVector3D lightDirection(0.0,0.0,1.0);
    program->setUniformValue(lightDirectionUniform,lightDirection);
    
    if(!bag.vrg.elevationVerticies.empty())
    {
        glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, bag.vrg.elevationVerticies.data());
        glVertexAttribPointer(normAttr, 3, GL_FLOAT, GL_FALSE, 0, bag.vrg.normals.data());
    }
    else
    {
//         glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, bag.g.elevationVerticies.data());
//         glVertexAttribPointer(unAttr, 1, GL_FLOAT, GL_FALSE, 0, bag.g.uncertainties.data());
//         glVertexAttribPointer(normAttr, 3, GL_FLOAT, GL_FALSE, 0, bag.g.normals.data());
    }    
    glEnableVertexAttribArray(posAttr);
    glEnableVertexAttribArray(unAttr);
    glEnableVertexAttribArray(normAttr);
    
    colormaps[currentColormap]->bind();


    if(!bag.vrg.elevationVerticies.empty())
    {
        if(drawStyle == "points")
            glDrawArrays(GL_POINTS, 0, bag.vrg.elevationVerticies.size()/3);
        if(drawStyle == "solid")
            glDrawElements(GL_TRIANGLE_STRIP,bag.vrg.indecies.size(),GL_UNSIGNED_INT,bag.vrg.indecies.data());
        if(drawStyle == "wireframe")
            glDrawElements(GL_LINE_STRIP,bag.vrg.indecies.size(),GL_UNSIGNED_INT,bag.vrg.indecies.data());
    }
    else
    {
//         if(drawStyle == "points")
//             glDrawArrays(GL_POINTS, 0, bag.g.elevationVerticies.size()/3);
//         if(drawStyle == "solid")
//             glDrawElements(GL_TRIANGLE_STRIP,bag.g.indecies.size(),GL_UNSIGNED_INT,bag.g.indecies.data());
//         if(drawStyle == "wireframe")
//             glDrawElements(GL_LINE_STRIP,bag.g.indecies.size(),GL_UNSIGNED_INT,bag.g.indecies.data());
    
    }
    
    if(bag.vrg.elevationVerticies.empty())
    {
        for(auto t: bag.getOverviewTiles())
        {
            glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, t->g.elevationVerticies.data());
            glVertexAttribPointer(unAttr, 1, GL_FLOAT, GL_FALSE, 0, t->g.uncertainties.data());
            glVertexAttribPointer(normAttr, 3, GL_FLOAT, GL_FALSE, 0, t->g.normals.data());

            if(drawStyle == "points")
                glDrawArrays(GL_POINTS, 0, t->g.elevationVerticies.size()/3);
            if(drawStyle == "solid")
                glDrawElements(GL_TRIANGLE_STRIP,t->g.indecies.size(),GL_UNSIGNED_INT,t->g.indecies.data());
            if(drawStyle == "wireframe")
                glDrawElements(GL_LINE_STRIP,t->g.indecies.size(),GL_UNSIGNED_INT,t->g.indecies.data());
        }
    }
    
    glDisableVertexAttribArray(normAttr);
    glDisableVertexAttribArray(unAttr);
    glDisableVertexAttribArray(posAttr);
    
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
    bag.close();
}

void BagGL::resetView()
{
    translatePosition = bag.size/2.0;
    translatePosition.setZ(0.0);
    pitch = 30.0;
    yaw = 0.0;
    zoom = 2/std::max(bag.size.x(),bag.size.y());
    
    heightExaggeration = 1.0;
}

void BagGL::setColormap(const std::string& cm)
{
    currentColormap = cm;
}

void BagGL::setDrawStyle(const std::string& ds)
{
    drawStyle = ds;
}

