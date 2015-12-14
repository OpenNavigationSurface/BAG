#ifndef BAGIO_H
#define BAGIO_H

#include <QOpenGLContext>
#include <vector>
#include <memory>
#include <QVector3D>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include "bag.h"

struct TileGL;

struct TileGeometry
{
    std::vector<GLfloat> elevations;
    QImage normalMap;
    std::vector<GLfloat> uncertainties;
    void reset();
};

typedef std::pair<u32,u32> TileIndex2D;

struct TileBase
{
    TileGeometry g;
    TileIndex2D index;
    std::shared_ptr<TileGL> gl;
    QVector3D lowerLeft,upperRight;
};

struct VarResTile: public TileBase
{
    float dx,dy;
    uint ncols,nrows;
};

typedef std::shared_ptr<VarResTile> VarResTilePtr;
typedef std::map<TileIndex2D,VarResTilePtr> VarResTileMap;

struct Tile: public TileBase
{
    TileIndex2D lowerLeftIndex;
    VarResTileMap varResTiles;
};

typedef std::shared_ptr<Tile> TilePtr;

typedef std::map<TileIndex2D,TilePtr> TileMap;


class BagIO: public QThread
{
    Q_OBJECT
public:
    
    struct MetaData
    {
        float minElevation, maxElevation;
        QVector3D size;
        QVector3D swBottomCorner;
        
        u32 ncols,nrows;
        double dx,dy;
        bool variableResolution;
        
        MetaData():
            minElevation(0.0),
            maxElevation(0.0),
            ncols(0),
            nrows(0),
            dx(0.0),
            dy(0.0)
        {
        }
    };
    
    BagIO(QObject *parent = 0);
    ~BagIO();
    
    bool open(QString const &bagFileName);
    void close();
    
    u32 getTileSize() const;
    
    std::vector<TilePtr> getOverviewTiles();
    MetaData getMeta();

    //Geometry vrg;
    
    
signals:
    void metaLoaded();
    void tileLoaded();
    
protected:
    void run() Q_DECL_OVERRIDE;
    
private:
    TilePtr loadTile(bagHandle &bag, TileIndex2D tileIndex, MetaData &meta) const; 
    VarResTilePtr loadVarResTile(bagHandle &bag, TileIndex2D const tileIndex, MetaData const &meta, Tile const &parentTile) const; 
    
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