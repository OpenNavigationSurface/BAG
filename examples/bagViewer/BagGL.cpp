#include "BagGL.h"
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>

#include <iostream>

static const char *vertexShaderSource =
"attribute highp vec4 posAttr;\n"
"attribute lowp vec4 colAttr;\n"
"varying lowp vec4 col;\n"
"uniform highp mat4 matrix;\n"
"void main() {\n"
"   col = colAttr;\n"
"   gl_Position = matrix * posAttr;\n"
"}\n";

static const char *fragmentShaderSource =
"varying lowp vec4 col;\n"
"void main() {\n"
"   gl_FragColor = col;\n"
"}\n";

BagGL::BagGL(): 
    program(0),
    bag(0),
    nearPlane(1.0),
    farPlane(100.0),
    zoom(1.0),
    yaw(0.0),
    pitch(0.0),
    rotating(false),
    translateX(0.0),
    translateY(0.0),
    translateZ(0.0),
    translating(false),
    heightExaggeration(25.0)
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
    colAttr = program->attributeLocation("colAttr");
    matrixUniform = program->uniformLocation("matrix");
    glEnable(GL_DEPTH_TEST);
    glPointSize(5.0);
}

QMatrix4x4 BagGL::genMatrix()
{
    QMatrix4x4 matrix;
    matrix.perspective(60.0f, width()/float(height()), nearPlane, farPlane);
    matrix.translate(0, 0, -2);
    matrix.rotate(-90, 1, 0, 0);
    matrix.scale(zoom,zoom,zoom);
    matrix.rotate(pitch, 1, 0, 0);
    matrix.rotate(yaw, 0, 0, 1);
    matrix.scale(1.0,1.0,heightExaggeration);
    matrix.translate(-translateX,-translateY,-translateZ);
    return matrix;
}

void BagGL::render()
{
    if(translating)
    {
        float p = translateStartTime.elapsed()/500.0;
        if(p >= 1.0)
        {
            translateX = translateEndPosition.x();
            translateY = translateEndPosition.y();
            translateZ = translateEndPosition.z();
            translating = false;
        }
        else
        {
            float ip = 1.0-p;
            translateX = translateStartPosition.x()*ip+translateEndPosition.x()*p;
            translateY = translateStartPosition.y()*ip+translateEndPosition.y()*p;
            translateZ = translateStartPosition.z()*ip+translateEndPosition.z()*p;
        }
    }
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    program->bind();
    
    QMatrix4x4 matrix = genMatrix();
    
    program->setUniformValue(matrixUniform, matrix);
    
    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    
    glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, elevationVerticies.data());
    
    glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);
    
    glDrawArrays(GL_POINTS, 0, elevationVerticies.size()/3);
    
    //glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    
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
        std::cerr << "mouse: " << mx << ", " << my << std::endl;
        float gx = -1.0+mx/(width()/2.0);
        float gy = -1.0+my/(height()/2.0);
        renderNow();
        GLfloat gz;
        glReadPixels(mx, my, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &gz);
        gz = (gz-.5)*2.0;
        //gz = (near*far)/(far-gz*(far-near));
        std::cerr << "gl coord: " << gx << ", " << gy << ", " << gz << std::endl;
        QMatrix4x4 matrix = genMatrix().inverted();
        QVector4D mousePos(gx,gy,gz,1.0);
        mousePos = matrix*mousePos;
        std::cerr << mousePos.x()/mousePos.w() <<", " << mousePos.y()/mousePos.w() << ", " << mousePos.z()/mousePos.w() << std::endl;
        if (gz < 1.0)
        {
            translating = true;
            translateStartPosition = QVector3D(translateX,translateY,translateZ);
            translateEndPosition = QVector3D(mousePos.x()/mousePos.w(),mousePos.y()/mousePos.w(),mousePos.z()/mousePos.w());
            translateStartTime.start();
        }
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
        translating = false;
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
        yaw += dx;
    }
}

void BagGL::wheelEvent(QWheelEvent* event)
{
    if(event->angleDelta().y() > 0)
        zoom *= 1.3f;
    else
        zoom /= 1.3f;
}

bool BagGL::openBag(const QString& bagFileName)
{
    if(bag)
        closeBag();
    bagError status = bagFileOpen (&bag, BAG_OPEN_READONLY, reinterpret_cast<const u8*>(bagFileName.toStdString().c_str()));
    if (status != BAG_SUCCESS)
        return false;
    Bool isVarRes, hasExtData;
    bagCheckVariableResolution(bag,&isVarRes,&hasExtData);
    std::cerr << "variable resolution? " << isVarRes << " extended data? " << hasExtData << std::endl;
    
    bagData * bd = bagGetDataPointer(bag);
    
    
    double dx = bd->def.nodeSpacingX;
    double dy = bd->def.nodeSpacingY;

    translateX = dx*bd->def.ncols/2.0f;
    translateY = dy*bd->def.nrows/2.0f;
    
    
    std::cerr << bd->def.ncols << " columns, " << bd->def.nrows << " rows" << std::endl;
    std::cerr << "spacing: " << bd->def.nodeSpacingX << " x " << bd->def.nodeSpacingY << std::endl;
    std::cerr << "sw corner: " << bd->def.swCornerX << ", " << bd->def.swCornerY << std::endl;
    std::cerr << "Horizontal coordinate system: " << bd->def.referenceSystem.horizontalReference << std::endl;
    std::cerr << "Vertical coordinate system: " << bd->def.referenceSystem.verticalReference << std::endl;
    
    s32 numOptDatasets;
    int optDatasetEntities[BAG_OPT_SURFACE_LIMIT];
    bagGetOptDatasets(&bag,&numOptDatasets,optDatasetEntities);
    for(int i = 0; i < numOptDatasets; ++i)
        std::cerr << "optional dataset type: " << optDatasetEntities[i] << std::endl;
    
    float nodeZeroZero;
    f64 *nzzx, *nzzy;
    bagReadNodePos(bag,0,0,Elevation,&nodeZeroZero,&nzzx,&nzzy);
    std::cerr << "0,0: " << nodeZeroZero << " pos: " << *nzzx << ", " << *nzzy << std::endl;
    free(nzzx);
    free(nzzy);
    
    //elevationVerticies.resize(bd->def.ncols*bd->def.nrows*3);
    elevationVerticies.resize(0);
    std::vector<float> dataRow(bd->def.ncols);
    for(u32 i = 0; i < bd->def.nrows; ++i)
    {
        bagReadRow(bag,i,0,bd->def.ncols-1,Elevation,dataRow.data());
        for(u32 j = 0; j < bd->def.ncols; ++j)
        {
            if(dataRow[j]!=BAG_NULL_ELEVATION)
            {
                elevationVerticies.push_back(j*dx);
                elevationVerticies.push_back(i*dy);
                elevationVerticies.push_back(dataRow[j]);
            }
            //elevationVerticies[3*(i*bd->def.ncols+j)] = j;
            //elevationVerticies[3*(i*bd->def.ncols+j)+1] = i;
            //elevationVerticies[3*(i*bd->def.ncols+j)+2] = dataRow[j];
        }
    }
    
    return true;
}

void BagGL::closeBag()
{
    bagFileClose(bag);
    elevationVerticies.resize(0);
    translateX = 0.0f;
    translateY = 0.0f;
    bag = 0;
}

