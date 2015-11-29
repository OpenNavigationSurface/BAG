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
"varying lowp vec4 color;\n"
"uniform highp mat4 matrix;\n"
"uniform highp mat3 normMatrix;\n"
"uniform vec3 lightDirection;\n"
"void main() {\n"
"   vec3 vsNormal = normalize(normMatrix*normal);\n"
"   float NdotL = max(dot(vsNormal,lightDirection), 0.0);\n"
"   color.r = uncertainty;\n"
"   //color.r = NdotL;\n"
"   color.g = NdotL;\n"
"   color.b = NdotL;\n"
"   //color.rgb = vsNormal.xyz;\n"
"   //color.g = .1;\n"
"   color.a = 1.0;\n"
"   gl_Position = matrix * posAttr;\n"
"}\n";

static const char *fragmentShaderSource =
"varying lowp vec4 color;\n"
"void main() {\n"
"   gl_FragColor = color;\n"
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
    translatePosition(0.0,0.0,0.0),
    bagCenter(0.0,0.0,0.0),
    translating(false),
    heightExaggeration(25.0),
    primitiveReset(0xffffffff)
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
    glEnable(GL_DEPTH_TEST);
    glPointSize(5.0);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(primitiveReset);
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
    
//     std::cerr << "normMatrix" << std::endl;
//     std::cerr << normMatrix.data()[0] << ", " << normMatrix.data()[1] << ", " << normMatrix.data()[2] << ", " << normMatrix.data()[3] << ", " << std::endl;
//     std::cerr << normMatrix.data()[4] << ", " << normMatrix.data()[5] << ", " << normMatrix.data()[6] << ", " << normMatrix.data()[7] << ", " << std::endl;
//     std::cerr << normMatrix.data()[8] << ", " << normMatrix.data()[9] << ", " << normMatrix.data()[10] << ", " << normMatrix.data()[11] << ", " << std::endl;
//     std::cerr << normMatrix.data()[12] << ", " << normMatrix.data()[13] << ", " << normMatrix.data()[14] << ", " << normMatrix.data()[15] << ", " << std::endl;
    
    //normMatrix = normMatrix.inverted().transposed();
    
//     std::cerr << "normMatrix inverted transposed" << std::endl;
//     std::cerr << normMatrix.data()[0] << ", " << normMatrix.data()[1] << ", " << normMatrix.data()[2] << ", " << normMatrix.data()[3] << ", " << std::endl;
//     std::cerr << normMatrix.data()[4] << ", " << normMatrix.data()[5] << ", " << normMatrix.data()[6] << ", " << normMatrix.data()[7] << ", " << std::endl;
//     std::cerr << normMatrix.data()[8] << ", " << normMatrix.data()[9] << ", " << normMatrix.data()[10] << ", " << normMatrix.data()[11] << ", " << std::endl;
//     std::cerr << normMatrix.data()[12] << ", " << normMatrix.data()[13] << ", " << normMatrix.data()[14] << ", " << normMatrix.data()[15] << ", " << std::endl;
    
    program->setUniformValue(matrixUniform, matrix);
    program->setUniformValue(normMatrixUniform, normMatrix.normalMatrix());
    
    QVector3D lightDirection(0.0,0.0,1.0);
    program->setUniformValue(lightDirectionUniform,lightDirection);
    
    glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, elevationVerticies.data());
    glVertexAttribPointer(unAttr, 1, GL_FLOAT, GL_FALSE, 0, uncertainties.data());
    glVertexAttribPointer(normAttr, 3, GL_FLOAT, GL_FALSE, 0, normals.data());
    
    glEnableVertexAttribArray(posAttr);
    glEnableVertexAttribArray(unAttr);
    glEnableVertexAttribArray(normAttr);
    
    //glDrawArrays(GL_POINTS, 0, elevationVerticies.size()/3);
    glDrawElements(GL_TRIANGLE_STRIP,indecies.size(),GL_UNSIGNED_INT,indecies.data());
    
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

    bagCenter.setX(dx*bd->def.ncols/2.0f);
    bagCenter.setY(dy*bd->def.nrows/2.0f);
    resetView();
    
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

    elevationVerticies.resize(0);
    uncertainties.resize(0);
    normals.resize(0);
    indecies.resize(0);
    bool inElement = false;
    typedef std::vector<float> DataRowVec;
    typedef std::shared_ptr<DataRowVec> DataRowVecPtr;
    DataRowVecPtr dataRow,lastDataRow;
    //std::vector<float> dataRow(bd->def.ncols);
    std::vector<float> unRow(bd->def.ncols);
    IndexMapPtr lastRowIndecies, currentRowIndecies;
    for(u32 i = 0; i < bd->def.nrows; ++i)
    {
        inElement = false;
        lastDataRow = dataRow;
        dataRow = DataRowVecPtr(new DataRowVec(bd->def.ncols));
        bagReadRow(bag,i,0,bd->def.ncols-1,Elevation,dataRow->data());
        bagReadRow(bag,i,0,bd->def.ncols-1,Uncertainty,unRow.data());
        lastRowIndecies = currentRowIndecies;
        currentRowIndecies = IndexMapPtr(new IndexMap);
        for(u32 j = 0; j < bd->def.ncols; ++j)
        {
            if((*dataRow)[j]!=BAG_NULL_ELEVATION)
            {
                (*currentRowIndecies)[IndexMap::key_type(i,j)]=elevationVerticies.size()/3;
                elevationVerticies.push_back(j*dx);
                elevationVerticies.push_back(i*dy);
                elevationVerticies.push_back((*dataRow)[j]);
                uncertainties.push_back(unRow[j]);
                if(lastRowIndecies && lastRowIndecies->count(IndexMap::key_type(i-1,j)))
                {
                    indecies.push_back((*lastRowIndecies)[IndexMap::key_type(i-1,j)]);
                    indecies.push_back((elevationVerticies.size()/3)-1);
                    inElement = true;
                    if(j < bd->def.ncols-1 && (*dataRow)[j+1] != BAG_NULL_ELEVATION)
                    {
                        QVector3D v1(dx,0.0,(*lastDataRow)[j]-(*dataRow)[j]);
                        QVector3D v2(0.0,dy,(*dataRow)[j+1]-(*dataRow)[j]);
                        QVector3D n = QVector3D::normal(v1,v2);
                        normals.push_back(n.x());
                        normals.push_back(n.y());
                        normals.push_back(n.z());
                    }
                    else
                    {
                        normals.push_back(0.0);
                        normals.push_back(0.0);
                        normals.push_back(1.0);
                    }
                }
                else
                {
                    if(inElement)
                    {
                        indecies.push_back(primitiveReset);
                        inElement = false;
                    }
                    normals.push_back(0.0);
                    normals.push_back(0.0);
                    normals.push_back(1.0);
                    
                }
            }
            else
            {
                if(inElement)
                {
                    indecies.push_back(primitiveReset);
                    inElement = false;
                }
            }
        }
        if(inElement)
            indecies.push_back(primitiveReset);
    }
    std::cerr << elevationVerticies.size() << " ev size, " << normals.size() << " normals size" << std::endl;
    
    return true;
}

void BagGL::closeBag()
{
    bagFileClose(bag);
    elevationVerticies.resize(0);
    bagCenter.setX(0.0f);
    bagCenter.setY(0.0f);
    bag = 0;
}

void BagGL::resetView()
{
    translatePosition = bagCenter;
    pitch = 0.0;
    yaw = 0.0;
    zoom = 1.0;
}

