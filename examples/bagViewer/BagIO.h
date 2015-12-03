#ifndef BAGIO_H
#define BAGIO_H

#include <GL/gl.h>
#include <vector>
#include <memory>
#include <QVector3D>
#include <QFuture>
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
    
    typedef std::pair<u32,u32> Index2D;
    
    struct Tile
    {
        Geometry g;
        Index2D index;
    };
    
    typedef std::shared_ptr<Tile> TilePtr;
    
    typedef std::map<Index2D,TilePtr> TileMap;
    
    
    
    BagIO(GLuint primitiveReset);
    ~BagIO();
    
    bool open(QString const &bagFileName);
    void close();
    
    std::vector<TilePtr> getOverviewTiles();

    float minElevation, maxElevation;
    QVector3D size;
    QVector3D swBottomCorner;
    
    u32 ncols,nrows;
    double dx,dy;
    Geometry vrg;
private:
    TilePtr loadTile(Index2D tileIndex) const; 
    bagHandle bag;
    
    const GLuint primitiveReset;
    
    u32 tileSize;
    TileMap overviewTiles;
    std::vector<QFuture<TilePtr> > loadingTiles;
};

#endif