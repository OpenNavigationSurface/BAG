#ifndef BAGIO_H
#define BAGIO_H

#include <QOpenGLContext>
#include <vector>
#include <memory>
#include <QVector3D>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "bag.h"

struct TileGL;

class BagIO: public QThread
{
    Q_OBJECT
public:
    struct Geometry
    {
        std::vector<GLfloat> elevations;
        std::vector<GLfloat> uncertainties;
        void reset();
    };
    
    typedef std::pair<u32,u32> Index2D;
    
    struct Tile
    {
        Geometry g;
        Index2D index;
        std::shared_ptr<TileGL> gl;
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