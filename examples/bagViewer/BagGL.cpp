#include "BagGL.h"
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>

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
    matrix.scale(zoom,zoom,zoom);
    matrix.rotate(pitch, 1, 0, 0);
    matrix.rotate(yaw, 0, 1, 0);
    
    program->setUniformValue(matrixUniform, matrix);
    
    GLfloat vertices[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };
    
    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    
    glVertexAttribPointer(posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    
    program->release();
}

void BagGL::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::MiddleButton)
    {
        rotating= true;
        lastPosition = event->pos();
    }
}

void BagGL::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::MiddleButton)
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
    {
        return false;
    }
    return true;
}

void BagGL::closeBag()
{
    bagFileClose(bag);
    bag = 0;
}

