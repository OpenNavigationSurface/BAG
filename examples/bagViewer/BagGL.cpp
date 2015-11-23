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
    zoom(1.0),
    yaw(0.0),
    pitch(0.0),
    rotating(false)
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
}

void BagGL::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    program->bind();
    
    QMatrix4x4 matrix;
    matrix.perspective(60.0f, width()/float(height()), 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    matrix.rotate(-90, 1, 0, 0);
    matrix.scale(zoom,zoom,zoom);
    matrix.rotate(pitch, 1, 0, 0);
    matrix.rotate(yaw, 0, 0, 1);
    
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
}

void BagGL::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        rotating = false;
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
        zoom *= 1.3;
    else
        zoom /= 1.3;
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
    
    elevationVerticies.resize(bd->def.ncols*bd->def.nrows*3);
    std::vector<float> dataRow(bd->def.ncols);
    for(int i = 0; i < bd->def.nrows; ++i)
    {
        bagReadRow(bag,i,0,bd->def.ncols-1,Elevation,dataRow.data());
        for(int j = 0; j < bd->def.ncols; ++j)
        {
            elevationVerticies[3*(i*bd->def.ncols+j)] = j;
            elevationVerticies[3*(i*bd->def.ncols+j)+1] = i;
            elevationVerticies[3*(i*bd->def.ncols+j)+2] = dataRow[j];
        }
    }
    
    return true;
}

void BagGL::closeBag()
{
    bagFileClose(bag);
    elevationVerticies.resize(0);
    bag = 0;
}

