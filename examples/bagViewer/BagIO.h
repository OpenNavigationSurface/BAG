#ifndef BAGIO_H
#define BAGIO_H

#include <GL/gl.h>
#include <vector>
#include <memory>
#include <QVector3D>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "bag.h"

class BagIO: public QThread
{
    Q_OBJECT
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
    
    struct MetaData
    {
        float minElevation, maxElevation;
        QVector3D size;
        QVector3D swBottomCorner;
        
        u32 ncols,nrows;
        double dx,dy;
    };
    
    BagIO(QObject *parent = 0);
    ~BagIO();
    
    bool open(QString const &bagFileName);
    void close();
    
    std::vector<TilePtr> getOverviewTiles();
    MetaData getMeta();

    //Geometry vrg;
    
signals:
    void metaLoaded();
    
protected:
    void run() Q_DECL_OVERRIDE;
    
private:
    TilePtr loadTile(bagHandle &bag, Index2D tileIndex, MetaData &meta) const; 
    
    QMutex mutex;
    QWaitCondition condition;
    bool restart;
    bool abort;
    
    u32 tileSize;
    TileMap overviewTiles;
    MetaData meta;
    QString filename;
};

#endif