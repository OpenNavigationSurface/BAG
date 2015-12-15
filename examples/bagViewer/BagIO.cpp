#include "BagIO.h"

#include <memory>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include "BagGL.h"

BagIO::BagIO(QObject *parent):
    QThread(parent),
    restart(false),
    abort(false),
    tileSize(128)
{

}

BagIO::~BagIO()
{
    close();
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();
    
    wait();
}

u32 BagIO::getTileSize() const
{
    return tileSize;
}


void BagIO::run()
{
    forever {
        {
            QMutexLocker locker(&mutex);
            if (restart)
                break;
            if (abort)
                return;
        }
        bagHandle bag;
        
        mutex.lock();
        QString filename = this->filename;
        mutex.unlock();
        
        bagError status = bagFileOpen (&bag, BAG_OPEN_READONLY, reinterpret_cast<const u8*>(filename.toStdString().c_str()));
        if (status == BAG_SUCCESS)
        {
            MetaData meta;
            
            Bool isVarRes, hasExtData;
            bagCheckVariableResolution(bag,&isVarRes,&hasExtData);
            qDebug() << "variable resolution? " << isVarRes << " extended data? " << hasExtData;
           
            if(isVarRes)
            {
                bagGetOptDatasetInfo(&bag, VarRes_Metadata_Group);
                bagGetOptDatasetInfo(&bag, VarRes_Refinement_Group);
            }
            
            meta.variableResolution = isVarRes;
            
            bagData * bd = bagGetDataPointer(bag);
            
            
            meta.dx = bd->def.nodeSpacingX;
            meta.dy = bd->def.nodeSpacingY;
            
            meta.minElevation = bd->min_elevation;
            meta.maxElevation = bd->max_elevation;
            
            meta.ncols = bd->def.ncols;
            meta.nrows = bd->def.nrows;
            
            meta.size.setX(meta.dx*meta.ncols);
            meta.size.setY(meta.dy*meta.nrows);
            meta.size.setZ(meta.maxElevation-meta.minElevation);
            
            meta.swBottomCorner.setX(bd->def.swCornerX);
            meta.swBottomCorner.setY(bd->def.swCornerY);
            meta.swBottomCorner.setZ(meta.minElevation);
            
            qDebug() << bd->def.ncols << " columns, " << bd->def.nrows << " rows";
            qDebug() << "spacing: " << bd->def.nodeSpacingX << " x " << bd->def.nodeSpacingY;
            qDebug() << "sw corner: " << bd->def.swCornerX << ", " << bd->def.swCornerY;
            qDebug() << "Horizontal coordinate system: " << QString((char*)(bd->def.referenceSystem.horizontalReference));
            qDebug() << "Vertical coordinate system: " << QString((char*)(bd->def.referenceSystem.verticalReference));
            
            s32 numOptDatasets;
            int optDatasetEntities[BAG_OPT_SURFACE_LIMIT];
            bagGetOptDatasets(&bag,&numOptDatasets,optDatasetEntities);
            for(int i = 0; i < numOptDatasets; ++i)
                qDebug() << "optional dataset type: " << optDatasetEntities[i];
            
            {
                QMutexLocker locker(&mutex);
                this->meta = meta;
            }
            emit metaLoaded();
            
            for(u32 i = 0; i < meta.nrows; i += tileSize)
            {
                {
                    QMutexLocker locker(&mutex);
                    if (restart)
                        break;
                    if (abort)
                        return;
                }
                u32 trow = i/tileSize;
                qDebug() << "tile row: " << trow;
                for(u32 j = 0; j < meta.ncols; j += tileSize)
                {
                    TileIndex2D tindex(j/tileSize,trow);
                    //std::cerr << "tile: " << tindex.first << "," << tindex.second;
                    TilePtr t = loadTile(bag,tindex,meta);
                    if(t)
                    {
                        //std::cerr << "\tsaved" << std::endl;
                        if(meta.variableResolution)
                        {
                            for (u32 ti = 0; ti < tileSize; ti++)
                                for(u32 tj = 0; tj < tileSize; tj++)
                                {
                                    VarResTilePtr vrTile = loadVarResTile(bag,TileIndex2D(tj,ti),meta,*t);
                                    if(vrTile)
                                        t->varResTiles[vrTile->index]=vrTile;
                                }
                        }
                    }
                    else
                    {
                        //std::cerr << "\tdiscarded" << std::endl;
                    }
                    {
                        QMutexLocker locker(&mutex);
                        if (restart)
                            break;
                        if (abort)
                            return;
                        if(t)
                        {
                            overviewTiles[tindex]=t;
                            emit tileLoaded();
                        }
                    }
                }
            }
        }
        bagFileClose(bag);
        bag = 0;
        
        
        {
            QMutexLocker locker(&mutex);
            if (!restart)
                condition.wait(&mutex);
            restart = false;
        }
    }
}


bool BagIO::open(const QString& bagFileName)
{
    close();
    QMutexLocker locker(&mutex);
    filename = bagFileName;
    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
    
    return true;
    
}

TilePtr BagIO::loadTile(bagHandle &bag, TileIndex2D tileIndex, MetaData &meta) const
{
    TilePtr ret(new Tile);
    ret->index = tileIndex;
    
    
    u32 startRow = tileIndex.second * (tileSize-1);
    u32 endRow = std::min(startRow+tileSize,meta.nrows-1);
    u32 startCol = tileIndex.first * (tileSize-1);
    u32 endCol = std::min(startCol+tileSize,meta.ncols-1);
    
    ret->lowerLeftIndex = TileIndex2D(startCol,startRow);
    
    bagReadRegion(bag, startRow, startCol, endRow, endCol, Elevation);
    
    ret->g.elevations.resize(tileSize*tileSize,BAG_NULL_ELEVATION);
    for(uint i=0; i < endRow-startRow; ++i)
        memcpy(&(ret->g.elevations.data()[i*tileSize]),bagGetDataPointer(bag)->elevation[i], (endCol-startCol)*sizeof(GLfloat));
    
    bool notEmpty = false;
    for(auto e: ret->g.elevations)
    {
        if(e != BAG_NULL_ELEVATION)
        {
            notEmpty = true;
            break;
        }
    }
    
    if(!notEmpty)
        ret.reset();
    else
    {
        float minElevation = BAG_NULL_ELEVATION;
        float maxElevation = BAG_NULL_ELEVATION;
        for(auto e:ret->g.elevations)
        {
            if(e != BAG_NULL_ELEVATION)
            {
                if(minElevation == BAG_NULL_ELEVATION || e < minElevation)
                    minElevation = e;
                if(maxElevation == BAG_NULL_ELEVATION || e > maxElevation)
                    maxElevation = e;
            }
        }
        ret->bounds.add(QVector3D(startCol * meta.dx, startRow * meta.dy, minElevation));
        ret->bounds.add(QVector3D((endCol+1) * meta.dx, (endRow+1) * meta.dy, maxElevation));
        
        ret->g.normalMap = QImage(tileSize, tileSize, QImage::Format_RGB888);
        for(u32 ti = 0; ti < tileSize && ti < endRow-startRow; ++ti)
        {
            for(u32 tj = 0; tj < tileSize && tj < endCol-startCol; ++tj)
            {
                float p00 = bagGetDataPointer(bag)->elevation[ti][tj];
                float p10 = bagGetDataPointer(bag)->elevation[ti][tj+1];
                float p01 = bagGetDataPointer(bag)->elevation[ti+1][tj];
                if(p00 != BAG_NULL_ELEVATION && p10 != BAG_NULL_ELEVATION && p01 != BAG_NULL_ELEVATION)
                {
                    QVector3D v1(meta.dx,0.0,p10-p00);
                    QVector3D v2(0.0,meta.dy,p01-p00);
                    QVector3D n = QVector3D::normal(v1,v2);
                    ret->g.normalMap.setPixel(tj,ti,QColor(127+128*n.x(),127+128*n.y(),127+128*n.z()).rgb());
                }
                else
                    ret->g.normalMap.setPixel(tj,ti,QColor(127,127,255).rgb());
            }
        }
    }
    
    return ret;
}

VarResTilePtr BagIO::loadVarResTile(bagHandle& bag, const TileIndex2D tileIndex, const BagIO::MetaData& meta, const Tile& parentTile) const
{
    VarResTilePtr ret(new VarResTile);
    ret->index = tileIndex;
    bagVarResMetadataGroup vrMetadata;
    std::vector<bagVarResRefinementGroup> refinements;
    uint i = parentTile.lowerLeftIndex.first+tileIndex.first;
    uint j = parentTile.lowerLeftIndex.second+tileIndex.second;
    if(i >= meta.ncols || j >= meta.nrows)
        return VarResTilePtr();
    bagReadNode(bag,j,i,VarRes_Metadata_Group,&vrMetadata);
    if(vrMetadata.dimensions == 0)
        return VarResTilePtr();
    refinements.resize(vrMetadata.dimensions*vrMetadata.dimensions);
    ret->g.normalMap = QImage(vrMetadata.dimensions, vrMetadata.dimensions, QImage::Format_RGB888);
    ret->g.normalMap.fill(QColor(127,127,255).rgb());
    bagReadRow(bag,0,vrMetadata.index,vrMetadata.index+refinements.size()-1,VarRes_Refinement_Group,refinements.data());
    float minz = BAG_NULL_ELEVATION;
    float maxz = BAG_NULL_ELEVATION;
    for(bagVarResRefinementGroup r:refinements)
    {
        ret->g.elevations.push_back(r.depth);
        if(minz == BAG_NULL_ELEVATION || r.depth < minz)
            minz = r.depth;
        if(maxz == BAG_NULL_ELEVATION || r.depth > maxz)
            maxz = r.depth;
    }
    
    for(u32 ti = 0; ti < vrMetadata.dimensions; ++ti)
    {
        if(ti < vrMetadata.dimensions-1)
            for(u32 tj = 0; tj < vrMetadata.dimensions; ++tj)
            {
                if(tj < vrMetadata.dimensions-1)
                {
                    float p00 = ret->g.elevations[ti*vrMetadata.dimensions+tj];
                    float p10 = ret->g.elevations[ti*vrMetadata.dimensions+tj+1];
                    float p01 = ret->g.elevations[(ti+1)*vrMetadata.dimensions+tj];
                    if(p00 != BAG_NULL_ELEVATION && p10 != BAG_NULL_ELEVATION && p01 != BAG_NULL_ELEVATION)
                    {
                        QVector3D v1(vrMetadata.resolution,0.0,p10-p00);
                        QVector3D v2(0.0,vrMetadata.resolution,p01-p00);
                        QVector3D n = QVector3D::normal(v1,v2);
                        ret->g.normalMap.setPixel(tj,ti,QColor(127+128*n.x(),127+128*n.y(),127+128*n.z()).rgb());
                    }
                    else
                        ret->g.normalMap.setPixel(tj,ti,QColor(127,127,255).rgb());
                }
                else
                    ret->g.normalMap.setPixel(tj,ti,ret->g.normalMap.pixel(tj-1,ti));
            }
        else
            for(u32 tj = 0; tj < vrMetadata.dimensions; ++tj)
                ret->g.normalMap.setPixel(tj,ti,ret->g.normalMap.pixel(tj,ti-1));
    }
    
    
    float cx = i*meta.dx;
    float cy = j*meta.dy;
    float llx = cx - (vrMetadata.dimensions-1)*vrMetadata.resolution/2.0;
    float lly = cy - (vrMetadata.dimensions-1)*vrMetadata.resolution/2.0;
    ret->dx = vrMetadata.resolution;
    ret->dy = vrMetadata.resolution;
    ret->ncols = vrMetadata.dimensions;
    ret->nrows = vrMetadata.dimensions;
    ret->bounds.add(QVector3D(llx,lly,minz));
    ret->bounds.add(QVector3D(llx+(vrMetadata.dimensions-1)*vrMetadata.resolution,lly+(vrMetadata.dimensions-1)*vrMetadata.resolution,maxz));
    return ret;
}

void BagIO::close()
{
    mutex.lock();
    overviewTiles.clear();
    mutex.unlock();
}



void TileGeometry::reset()
{
    elevations.resize(0);
    uncertainties.resize(0);
    normalMap = QImage();
}

std::vector< TilePtr > BagIO::getOverviewTiles()
{
    std::vector<TilePtr> ret;
    QMutexLocker locker(&mutex);
    for(auto it: overviewTiles)
        ret.push_back(it.second);
    return ret;
}

BagIO::MetaData BagIO::getMeta()
{
    MetaData ret;
    QMutexLocker locker(&mutex);
    ret = meta;
    return ret;
}

