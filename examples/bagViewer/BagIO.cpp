#include "BagIO.h"

#include <iostream>
#include <memory>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include "BagGL.h"

BagIO::BagIO(QObject *parent):
    QThread(parent),
    tileSize(100),
    restart(false),
    abort(false)
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
                for(u32 j = 0; j < meta.ncols; j += tileSize)
                {
                    Index2D tindex(j/tileSize,trow);
                    std::cerr << "tile: " << tindex.first << "," << tindex.second << std::endl;
                    //auto ftp = QtConcurrent::run(this, &BagIO::loadTile,tindex);
                    //loadingTiles.push_back(ftp);
                    TilePtr t = loadTile(bag,tindex,meta);
                    
                    {
                        QMutexLocker locker(&mutex);
                        if (restart)
                            break;
                        if (abort)
                            return;
                        if(!t->g.elevationVerticies.empty())
                        {
                            overviewTiles[tindex]=t;
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

BagIO::TilePtr BagIO::loadTile(bagHandle &bag, BagIO::Index2D tileIndex, MetaData &meta) const
{
    TilePtr ret(new Tile);
    ret->index = tileIndex;
    
    u32 startRow = tileIndex.second * tileSize;
    u32 endRow = std::min(startRow+tileSize+1,meta.nrows);
    u32 startCol = tileIndex.first * tileSize;
    u32 endCol = std::min(startCol+tileSize+1,meta.ncols);
    
    bool inElement = false;
    typedef std::vector<float> DataRowVec;
    typedef std::shared_ptr<DataRowVec> DataRowVecPtr;
    typedef std::map<std::pair<u32,u32>,GLuint> IndexMap;
    typedef std::shared_ptr<IndexMap> IndexMapPtr;
    
    DataRowVecPtr dataRow,lastDataRow;
    std::vector<float> unRow(tileSize+1);
    IndexMapPtr lastRowIndecies, currentRowIndecies;
    for(u32 i = startRow; i < endRow; ++i)
    {
        inElement = false;
        lastDataRow = dataRow;
        dataRow = DataRowVecPtr(new DataRowVec(tileSize+1));
        bagReadRow(bag,i,startCol,endCol-1,Elevation,dataRow->data());
        bagReadRow(bag,i,startCol,endCol-1,Uncertainty,unRow.data());
        lastRowIndecies = currentRowIndecies;
        currentRowIndecies = IndexMapPtr(new IndexMap);
        for(u32 j = startCol; j < endCol; ++j)
        {
            u32 ri = j-startCol;
            if((*dataRow)[ri]!=BAG_NULL_ELEVATION)
            {
                (*currentRowIndecies)[IndexMap::key_type(i,j)]=ret->g.elevationVerticies.size()/3;
                ret->g.elevationVerticies.push_back(j*meta.dx);
                ret->g.elevationVerticies.push_back(i*meta.dy);
                ret->g.elevationVerticies.push_back((*dataRow)[ri]);
                ret->g.uncertainties.push_back(unRow[ri]);
                if(lastRowIndecies && lastRowIndecies->count(IndexMap::key_type(i-1,j)))
                {
                    ret->g.indecies.push_back((*lastRowIndecies)[IndexMap::key_type(i-1,j)]);
                    ret->g.indecies.push_back((ret->g.elevationVerticies.size()/3)-1);
                    inElement = true;
                    if(j < meta.ncols-1 && (*dataRow)[ri+1] != BAG_NULL_ELEVATION)
                    {
                        QVector3D v1(meta.dx,0.0,(*lastDataRow)[ri]-(*dataRow)[ri]);
                        QVector3D v2(0.0,meta.dy,(*dataRow)[ri+1]-(*dataRow)[ri]);
                        QVector3D n = QVector3D::normal(v1,v2);
                        ret->g.normals.push_back(n.x());
                        ret->g.normals.push_back(n.y());
                        ret->g.normals.push_back(n.z());
                    }
                    else
                    {
                        ret->g.normals.push_back(0.0);
                        ret->g.normals.push_back(0.0);
                        ret->g.normals.push_back(1.0);
                    }
                }
                else
                {
                    if(inElement)
                    {
                        ret->g.indecies.push_back(BagGL::primitiveReset);
                        inElement = false;
                    }
                    ret->g.normals.push_back(0.0);
                    ret->g.normals.push_back(0.0);
                    ret->g.normals.push_back(1.0);
                }
            }
            else
            {
                if(inElement)
                {
                    ret->g.indecies.push_back(BagGL::primitiveReset);
                    inElement = false;
                }
            }
        }
        if(inElement)
            ret->g.indecies.push_back(BagGL::primitiveReset);
    }
    return ret;
}


void BagIO::close()
{
    //vrg.reset();
    mutex.lock();
    overviewTiles.clear();
    mutex.unlock();
}



void BagIO::Geometry::reset()
{
    elevationVerticies.resize(0);
    normals.resize(0);
    uncertainties.resize(0);
    indecies.resize(0);
}

std::vector< BagIO::TilePtr > BagIO::getOverviewTiles()
{
//     for(auto ftp :loadingTiles)
//     {
//         if(ftp.isFinished())
//         {
//             auto t = ftp.result();
//             if(!t->g.elevationVerticies.empty())
//                 overviewTiles[t->index]=t;
//         }
//     }
    
    
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

