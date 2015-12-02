#ifndef BAGIO_H
#define BAGIO_H

#include <GL/gl.h>
#include <vector>
#include <QVector3D>
#include "bag.h"

class BagIO
{
public:
    struct Geometry
    {
        std::vector<GLfloat> elevationVerticies;
        std::vector<GLfloat> normals;
        std::vector<GLfloat> uncertainties;
        std::vector<GLuint> indecies;
        
        void reset();
    };
    
    BagIO(GLuint primitiveReset);
    ~BagIO();
    
    bool open(QString const &bagFileName);
    void close();

    float minElevation, maxElevation;
    QVector3D size;
    QVector3D swBottomCorner;
    
    u32 ncols,nrows;
    
    Geometry g, vrg;
private:
    bagHandle bag;
    
    const GLuint primitiveReset;
};

#endif