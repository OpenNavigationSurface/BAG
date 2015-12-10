#include "BagIO.h"

#include <iostream>
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
            std::cerr << "variable resolution? " << isVarRes << " extended data? " << hasExtData << std::endl;
            
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
                std::cerr << "tile row: " << trow << std::endl;
                for(u32 j = 0; j < meta.ncols; j += tileSize)
                {
                    TileIndex2D tindex(j/tileSize,trow);
                    //std::cerr << "tile: " << tindex.first << "," << tindex.second;
                    TilePtr t = loadTile(bag,tindex,meta);
                    if(t)
                    {
                        //std::cerr << "\tsaved" << std::endl;
                        t->g.normalMap = QImage(tileSize, tileSize, QImage::Format_RGB888);
                        for(u32 ti = 0; ti < tileSize; ++ti)
                        {
                            for(u32 tj = 0; tj < tileSize; ++tj)
                            {
                                if(ti == tileSize-1)
                                {
                                    // last row, copy previous
                                    t->g.normalMap.setPixel(tj,ti,t->g.normalMap.pixel(tj,ti-1));
                                }
                                else
                                {
                                    if(tj == tileSize-1)
                                        t->g.normalMap.setPixel(tj,ti,t->g.normalMap.pixel(tj-1,ti));
                                    else
                                    {
                                        float p00 = t->g.elevations[ti*tileSize+tj];
                                        float p10 = t->g.elevations[ti*tileSize+tj+1];
                                        float p01 = t->g.elevations[(ti+1)*tileSize+tj];
                                        if(p00 != BAG_NULL_ELEVATION && p10 != BAG_NULL_ELEVATION && p01 != BAG_NULL_ELEVATION)
                                        {
                                            QVector3D v1(meta.dx,0.0,p10-p00);
                                            QVector3D v2(0.0,meta.dy,p01-p00);
                                            QVector3D n = QVector3D::normal(v1,v2);
                                            t->g.normalMap.setPixel(tj,ti,QColor(127+128*n.x(),127+128*n.y(),127+128*n.z()).rgb());
                                        }
                                        else
                                            t->g.normalMap.setPixel(tj,ti,QColor(127,127,255).rgb());
                                    }
                                }
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
    
    
//     bool inElement = false;
//     typedef std::map<std::pair<u32,u32>,GLuint> IndexMap;
//     typedef std::shared_ptr<IndexMap> IndexMapPtr;
//     
//     IndexMapPtr lastRowIndecies, currentRowIndecies;
//     
//     
//     if(isVarRes)
//     {
//         bagGetOptDatasetInfo(&bag, VarRes_Metadata_Group);
//         bagGetOptDatasetInfo(&bag, VarRes_Refinement_Group);
//         currentRowIndecies.reset();
//         std::vector<bagVarResMetadataGroup> metadata(bd->def.ncols);
//         std::vector<bagVarResRefinementGroup> refinements;
//         for(u32 i = 0; i < bd->def.nrows; ++i)
//         {
//             float cy = i*dy;
//             bagReadRow(bag,i,0,bd->def.ncols-1,VarRes_Metadata_Group,metadata.data());
//             for(u32 j = 0; j < bd->def.ncols; ++j)
//             {
//                 float cx = j*dx;
//                 if(metadata[j].dimensions > 0)
//                 {
//                     refinements.resize(metadata[j].dimensions*metadata[j].dimensions);
//                     bagReadRow(bag,0,metadata[j].index,metadata[j].index+refinements.size()-1,VarRes_Refinement_Group,refinements.data());
//                     float llx = cx - (metadata[j].dimensions-1)*metadata[j].resolution/2.0;
//                     float lly = cy - (metadata[j].dimensions-1)*metadata[j].resolution/2.0;
//                     for (u32 ri = 0; ri < metadata[j].dimensions; ++ri)
//                     {
//                         inElement = false;
//                         lastRowIndecies = currentRowIndecies;
//                         currentRowIndecies = IndexMapPtr(new IndexMap);
//                         for (u32 rj = 0; rj < metadata[j].dimensions; ++rj)
//                         {
//                             u32 rindex = ri*metadata[j].dimensions+rj;
//                             if(refinements[rindex].depth!=BAG_NULL_ELEVATION)
//                             {
//                                 (*currentRowIndecies)[IndexMap::key_type(ri,rj)]=vrg.elevationVerticies.size()/3;
//                                 vrg.elevationVerticies.push_back(llx+rj*metadata[j].resolution);
//                                 vrg.elevationVerticies.push_back(lly+ri*metadata[j].resolution);
//                                 vrg.elevationVerticies.push_back(refinements[rindex].depth);
//                                 vrg.uncertainties.push_back(refinements[rindex].depth_uncrt);
//                                 if(lastRowIndecies && lastRowIndecies->count(IndexMap::key_type(ri-1,rj)))
//                                 {
//                                     vrg.indecies.push_back((*lastRowIndecies)[IndexMap::key_type(ri-1,rj)]);
//                                     vrg.indecies.push_back((vrg.elevationVerticies.size()/3)-1);
//                                     inElement = true;
//                                     if(rj < metadata[j].dimensions-1 && refinements[rindex+1].depth != BAG_NULL_ELEVATION)
//                                     {
//                                         QVector3D v1(metadata[j].resolution,0.0,refinements[rindex-metadata[j].dimensions].depth-refinements[rindex].depth);
//                                         QVector3D v2(0.0,metadata[j].resolution,refinements[rindex+1].depth-refinements[rindex].depth);
//                                         QVector3D n = QVector3D::normal(v1,v2);
//                                         vrg.normals.push_back(n.x());
//                                         vrg.normals.push_back(n.y());
//                                         vrg.normals.push_back(n.z());
//                                     }
//                                     else
//                                     {
//                                         vrg.normals.push_back(0.0);
//                                         vrg.normals.push_back(0.0);
//                                         vrg.normals.push_back(1.0);
//                                     }
//                                 }
//                                 else
//                                 {
//                                     if(inElement)
//                                     {
//                                         vrg.indecies.push_back(BagGL::primitiveReset);
//                                         inElement = false;
//                                     }
//                                     vrg.normals.push_back(0.0);
//                                     vrg.normals.push_back(0.0);
//                                     vrg.normals.push_back(1.0);
//                                 }
//                             }
//                             else
//                             {
//                                 if(inElement)
//                                 {
//                                     vrg.indecies.push_back(BagGL::primitiveReset);
//                                     inElement = false;
//                                 }
//                             }
//                             
//                         }
//                         if(inElement)
//                             vrg.indecies.push_back(BagGL::primitiveReset);
//                     }
//                 }
//             }
//         }
//     }
    
    return true;
    
}

TilePtr BagIO::loadTile(bagHandle &bag, TileIndex2D tileIndex, MetaData &meta) const
{
    TilePtr ret(new Tile);
    ret->index = tileIndex;
    
    
    u32 startRow = tileIndex.second * tileSize;
    u32 endRow = std::min(startRow+tileSize-1,meta.nrows-1);
    u32 startCol = tileIndex.first * tileSize;
    u32 endCol = std::min(startCol+tileSize-1,meta.ncols-1);
    
    bagReadRegion(bag, startRow, startCol, endRow, endCol, Elevation);
    
    ret->g.elevations.resize(tileSize*tileSize,BAG_NULL_ELEVATION);
    if(endCol < startCol+tileSize-1)
    {
        for(uint i=0; i < endRow-startRow+1; ++i)
            memcpy(&(ret->g.elevations.data()[i*tileSize]),bagGetDataPointer(bag)->elevation[i], (endCol-startCol+1)*sizeof(GLfloat));
        
    }
    else
        memcpy(ret->g.elevations.data(),*(bagGetDataPointer(bag)->elevation), ((endRow-startRow)+1)*tileSize*sizeof(GLfloat));
    
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
        ret->lowerLeft = QVector3D(tileIndex.first * tileSize * meta.dx, tileIndex.second * tileSize * meta.dy, minElevation);
        ret->upperRight = QVector3D((tileIndex.first+1) * tileSize * meta.dx, (tileIndex.second+1) * tileSize * meta.dy, maxElevation);
    }
    
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

