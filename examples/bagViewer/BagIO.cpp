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
            
            if(isVarRes)
            {
                bagVarResRefinementGroup vrmin, vrmax;
                if(bagReadMinMaxVarResRefinementGroup(bag, &vrmin, &vrmax) == BAG_SUCCESS)
                {
                    meta.minElevation = vrmin.depth;
                    meta.maxElevation = vrmax.depth;
                }
            }
            
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
            
            std::vector<TilePtr> goodOverviewTiles;
            
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
                        goodOverviewTiles.push_back(t);
                        //std::cerr << "\tsaved" << std::endl;
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
                            //overviewTiles[tindex]=t;
                            emit tileLoaded(t,false);
                        }
                    }
                }
            }
            if(meta.variableResolution)
            {
                for(TilePtr t: goodOverviewTiles)
                    for (u32 ti = 0; ti < tileSize; ti++)
                        for(u32 tj = 0; tj < tileSize; tj++)
                        {
                            TilePtr vrTile = loadVarResTile(bag,TileIndex2D(tj,ti),meta,*t);
                            if(vrTile)
                                //t->subTiles[vrTile->index]=vrTile;
                                emit tileLoaded(vrTile,true);
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
    ret->dx = meta.dx;
    ret->dy = meta.dy;
    
    u32 startRow = tileIndex.second * tileSize;
    u32 endRow = std::min(startRow+tileSize,meta.nrows-1);
    u32 startCol = tileIndex.first * tileSize;
    u32 endCol = std::min(startCol+tileSize,meta.ncols-1);
    
    ret->ncols = endCol-startCol+1;
    ret->nrows = endRow-startRow+1;
    
    ret->lowerLeftIndex = TileIndex2D(startCol,startRow);
    
    bagReadRegion(bag, startRow, startCol, endRow, endCol, Elevation);
    
    ret->data = TileDataPtr(new TileData);
    ret->data->elevations.resize(ret->nrows*ret->ncols,BAG_NULL_ELEVATION);
    for(uint i=0; i < ret->nrows; ++i)
        memcpy(&(ret->data->elevations.data()[i*ret->ncols]),bagGetDataPointer(bag)->elevation[i], (ret->ncols)*sizeof(GLfloat));
    
    bool notEmpty = false;
    for(auto e: ret->data->elevations)
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
        for(auto e:ret->data->elevations)
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
        
        ret->data->normalMap = QImage(ret->ncols, ret->nrows, QImage::Format_RGB888);
        for(u32 ti = 0; ti < ret->nrows; ++ti)
        {
            for(u32 tj = 0; tj < ret->ncols; ++tj)
            {
                float p00 = bagGetDataPointer(bag)->elevation[ti][tj];
                float p10 = bagGetDataPointer(bag)->elevation[ti][tj+1];
                float p01 = p00;
                if (ti < ret->nrows-1)
                    p01 = bagGetDataPointer(bag)->elevation[ti+1][tj];
                if(p00 != BAG_NULL_ELEVATION && p10 != BAG_NULL_ELEVATION && p01 != BAG_NULL_ELEVATION)
                {
                    QVector3D v1(meta.dx,0.0,p10-p00);
                    QVector3D v2(0.0,meta.dy,p01-p00);
                    QVector3D n = QVector3D::normal(v1,v2);
                    ret->data->normalMap.setPixel(tj,ti,QColor(127+128*n.x(),127+128*n.y(),127+128*n.z()).rgb());
                }
                else
                    ret->data->normalMap.setPixel(tj,ti,QColor(127,127,255).rgb());
            }
        }
    }
    
    return ret;
}

TilePtr BagIO::loadVarResTile(bagHandle& bag, const TileIndex2D tileIndex, const BagIO::MetaData& meta, const Tile& parentTile) const
{
    TilePtr ret(new Tile);
    ret->data = TileDataPtr(new TileData);
    
    ret->index = tileIndex;
    bagVarResMetadataGroup vrMetadata;
    std::vector<bagVarResRefinementGroup> refinements;
    uint i = parentTile.lowerLeftIndex.first+tileIndex.first;
    uint j = parentTile.lowerLeftIndex.second+tileIndex.second;
    if(i >= meta.ncols || j >= meta.nrows)
        return TilePtr();
    ret->lowerLeftIndex = TileIndex2D(i,j);
    bagReadNode(bag,j,i,VarRes_Metadata_Group,&vrMetadata);
    if(vrMetadata.dimensions_x == 0)
        return TilePtr();
    refinements.resize(vrMetadata.dimensions_x*vrMetadata.dimensions_y);
    ret->data->normalMap = QImage(vrMetadata.dimensions_x, vrMetadata.dimensions_y, QImage::Format_RGB888);
    ret->data->normalMap.fill(QColor(127,127,255).rgb());
    bagReadRow(bag,0,vrMetadata.index,vrMetadata.index+(u32)refinements.size()-1,VarRes_Refinement_Group,refinements.data());
    float minz = BAG_NULL_ELEVATION;
    float maxz = BAG_NULL_ELEVATION;
    for(bagVarResRefinementGroup r:refinements)
    {
        ret->data->elevations.push_back(r.depth);
        if(minz == BAG_NULL_ELEVATION || r.depth < minz)
            minz = r.depth;
        if(maxz == BAG_NULL_ELEVATION || r.depth > maxz)
            maxz = r.depth;
    }
    
    for(u32 ti = 0; ti < vrMetadata.dimensions_y; ++ti)
    {
        if(ti < vrMetadata.dimensions_y-1)
            for(u32 tj = 0; tj < vrMetadata.dimensions_x; ++tj)
            {
                if(tj < vrMetadata.dimensions_x-1)
                {
                    float p00 = ret->data->elevations[ti*vrMetadata.dimensions_x+tj];
                    float p10 = ret->data->elevations[ti*vrMetadata.dimensions_x+tj+1];
                    float p01 = ret->data->elevations[(ti+1)*vrMetadata.dimensions_x+tj];
                    if(p00 != BAG_NULL_ELEVATION && p10 != BAG_NULL_ELEVATION && p01 != BAG_NULL_ELEVATION)
                    {
                        QVector3D v1(vrMetadata.resolution_x,0.0,p10-p00);
                        QVector3D v2(0.0,vrMetadata.resolution_y,p01-p00);
                        QVector3D n = QVector3D::normal(v1,v2);
                        ret->data->normalMap.setPixel(tj,ti,QColor(127+128*n.x(),127+128*n.y(),127+128*n.z()).rgb());
                    }
                    else
                        ret->data->normalMap.setPixel(tj,ti,QColor(127,127,255).rgb());
                }
                else
                    ret->data->normalMap.setPixel(tj,ti,ret->data->normalMap.pixel(tj-1,ti));
            }
        else
            for(u32 tj = 0; tj < vrMetadata.dimensions_x; ++tj)
                ret->data->normalMap.setPixel(tj,ti,ret->data->normalMap.pixel(tj,ti-1));
    }
    
    
    float cx = i*meta.dx;
    float cy = j*meta.dy;
    float pllx = cx-meta.dx/2.0;
    float plly = cy-meta.dy/2.0;
    float llx = pllx+vrMetadata.sw_corner_x;
    float lly = plly+vrMetadata.sw_corner_y;
    ret->dx = vrMetadata.resolution_x;
    ret->dy = vrMetadata.resolution_y;
    ret->ncols = vrMetadata.dimensions_x;
    ret->nrows = vrMetadata.dimensions_y;
    ret->bounds.add(QVector3D(llx,lly,minz));
    ret->bounds.add(QVector3D(llx+(vrMetadata.dimensions_x-1)*vrMetadata.resolution_x,lly+(vrMetadata.dimensions_y-1)*vrMetadata.resolution_y,maxz));
    return ret;
}

void BagIO::close()
{
    //mutex.lock();
    //overviewTiles.clear();
    //mutex.unlock();
}


BagIO::MetaData BagIO::getMeta()
{
    MetaData ret;
    QMutexLocker locker(&mutex);
    ret = meta;
    return ret;
}

