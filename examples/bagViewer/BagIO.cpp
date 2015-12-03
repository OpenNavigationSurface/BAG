#include "BagIO.h"

#include <iostream>
#include <memory>
#include <QString>
#include <QtConcurrent/QtConcurrent>

BagIO::BagIO(GLuint primitiveReset):
    bag(0),
    primitiveReset(primitiveReset),
    tileSize(100)
{

}

BagIO::~BagIO()
{
    close();
}

bool BagIO::open(const QString& bagFileName)
{
    if(bag)
        close();
    bagError status = bagFileOpen (&bag, BAG_OPEN_READONLY, reinterpret_cast<const u8*>(bagFileName.toStdString().c_str()));
    if (status != BAG_SUCCESS)
        return false;
    Bool isVarRes, hasExtData;
    bagCheckVariableResolution(bag,&isVarRes,&hasExtData);
    std::cerr << "variable resolution? " << isVarRes << " extended data? " << hasExtData << std::endl;
    
    bagData * bd = bagGetDataPointer(bag);
    
    
    dx = bd->def.nodeSpacingX;
    dy = bd->def.nodeSpacingY;
    
    minElevation = bd->min_elevation;
    maxElevation = bd->max_elevation;
    
    ncols = bd->def.ncols;
    nrows = bd->def.nrows;

    size.setX(dx*ncols);
    size.setY(dy*nrows);
    size.setZ(maxElevation-minElevation);
    
    swBottomCorner.setX(bd->def.swCornerX);
    swBottomCorner.setY(bd->def.swCornerY);
    swBottomCorner.setZ(minElevation);
    
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
    
    for(u32 i = 0; i < nrows; i += tileSize)
    {
        u32 trow = i/tileSize;
        for(u32 j = 0; j < ncols; j += tileSize)
        {
            Index2D tindex(j/tileSize,trow);
            std::cerr << "tile: " << tindex.first << "," << tindex.second << std::endl;
            auto ftp = QtConcurrent::run(this, &BagIO::loadTile,tindex);
            loadingTiles.push_back(ftp);
            //TilePtr t = loadTile(tindex);
            
            //if(!t->g.elevationVerticies.empty())
            //    overviewTiles[tindex]=t;
        }
    }
    
    
    bool inElement = false;
    typedef std::map<std::pair<u32,u32>,GLuint> IndexMap;
    typedef std::shared_ptr<IndexMap> IndexMapPtr;
    
    IndexMapPtr lastRowIndecies, currentRowIndecies;
    
    
    if(isVarRes)
    {
        bagGetOptDatasetInfo(&bag, VarRes_Metadata_Group);
        bagGetOptDatasetInfo(&bag, VarRes_Refinement_Group);
        currentRowIndecies.reset();
        std::vector<bagVarResMetadataGroup> metadata(bd->def.ncols);
        std::vector<bagVarResRefinementGroup> refinements;
        for(u32 i = 0; i < bd->def.nrows; ++i)
        {
            float cy = i*dy;
            bagReadRow(bag,i,0,bd->def.ncols-1,VarRes_Metadata_Group,metadata.data());
            for(u32 j = 0; j < bd->def.ncols; ++j)
            {
                float cx = j*dx;
                if(metadata[j].dimensions > 0)
                {
                    refinements.resize(metadata[j].dimensions*metadata[j].dimensions);
                    bagReadRow(bag,0,metadata[j].index,metadata[j].index+refinements.size()-1,VarRes_Refinement_Group,refinements.data());
                    float llx = cx - (metadata[j].dimensions-1)*metadata[j].resolution/2.0;
                    float lly = cy - (metadata[j].dimensions-1)*metadata[j].resolution/2.0;
                    for (u32 ri = 0; ri < metadata[j].dimensions; ++ri)
                    {
                        inElement = false;
                        lastRowIndecies = currentRowIndecies;
                        currentRowIndecies = IndexMapPtr(new IndexMap);
                        for (u32 rj = 0; rj < metadata[j].dimensions; ++rj)
                        {
                            u32 rindex = ri*metadata[j].dimensions+rj;
                            if(refinements[rindex].depth!=BAG_NULL_ELEVATION)
                            {
                                (*currentRowIndecies)[IndexMap::key_type(ri,rj)]=vrg.elevationVerticies.size()/3;
                                vrg.elevationVerticies.push_back(llx+rj*metadata[j].resolution);
                                vrg.elevationVerticies.push_back(lly+ri*metadata[j].resolution);
                                vrg.elevationVerticies.push_back(refinements[rindex].depth);
                                vrg.uncertainties.push_back(refinements[rindex].depth_uncrt);
                                if(lastRowIndecies && lastRowIndecies->count(IndexMap::key_type(ri-1,rj)))
                                {
                                    vrg.indecies.push_back((*lastRowIndecies)[IndexMap::key_type(ri-1,rj)]);
                                    vrg.indecies.push_back((vrg.elevationVerticies.size()/3)-1);
                                    inElement = true;
                                    if(rj < metadata[j].dimensions-1 && refinements[rindex+1].depth != BAG_NULL_ELEVATION)
                                    {
                                        QVector3D v1(metadata[j].resolution,0.0,refinements[rindex-metadata[j].dimensions].depth-refinements[rindex].depth);
                                        QVector3D v2(0.0,metadata[j].resolution,refinements[rindex+1].depth-refinements[rindex].depth);
                                        QVector3D n = QVector3D::normal(v1,v2);
                                        vrg.normals.push_back(n.x());
                                        vrg.normals.push_back(n.y());
                                        vrg.normals.push_back(n.z());
                                    }
                                    else
                                    {
                                        vrg.normals.push_back(0.0);
                                        vrg.normals.push_back(0.0);
                                        vrg.normals.push_back(1.0);
                                    }
                                }
                                else
                                {
                                    if(inElement)
                                    {
                                        vrg.indecies.push_back(primitiveReset);
                                        inElement = false;
                                    }
                                    vrg.normals.push_back(0.0);
                                    vrg.normals.push_back(0.0);
                                    vrg.normals.push_back(1.0);
                                }
                            }
                            else
                            {
                                if(inElement)
                                {
                                    vrg.indecies.push_back(primitiveReset);
                                    inElement = false;
                                }
                            }
                            
                        }
                        if(inElement)
                            vrg.indecies.push_back(primitiveReset);
                    }
                }
            }
        }
    }
    
    return true;
    
}

BagIO::TilePtr BagIO::loadTile(BagIO::Index2D tileIndex) const
{
    TilePtr ret(new Tile);
    ret->index = tileIndex;
    
    u32 startRow = tileIndex.second * tileSize;
    u32 endRow = std::min(startRow+tileSize+1,nrows);
    u32 startCol = tileIndex.first * tileSize;
    u32 endCol = std::min(startCol+tileSize+1,ncols);
    
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
                ret->g.elevationVerticies.push_back(j*dx);
                ret->g.elevationVerticies.push_back(i*dy);
                ret->g.elevationVerticies.push_back((*dataRow)[ri]);
                ret->g.uncertainties.push_back(unRow[ri]);
                if(lastRowIndecies && lastRowIndecies->count(IndexMap::key_type(i-1,j)))
                {
                    ret->g.indecies.push_back((*lastRowIndecies)[IndexMap::key_type(i-1,j)]);
                    ret->g.indecies.push_back((ret->g.elevationVerticies.size()/3)-1);
                    inElement = true;
                    if(j < ncols-1 && (*dataRow)[ri+1] != BAG_NULL_ELEVATION)
                    {
                        QVector3D v1(dx,0.0,(*lastDataRow)[ri]-(*dataRow)[ri]);
                        QVector3D v2(0.0,dy,(*dataRow)[ri+1]-(*dataRow)[ri]);
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
                        ret->g.indecies.push_back(primitiveReset);
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
                    ret->g.indecies.push_back(primitiveReset);
                    inElement = false;
                }
            }
        }
        if(inElement)
            ret->g.indecies.push_back(primitiveReset);
    }
    return ret;
}


void BagIO::close()
{
    bagFileClose(bag);
    bag = 0;
    vrg.reset();
    overviewTiles.clear();
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
    for(auto ftp :loadingTiles)
    {
        if(ftp.isFinished())
        {
            auto t = ftp.result();
            if(!t->g.elevationVerticies.empty())
                overviewTiles[t->index]=t;
        }
    }
    
    
    std::vector<TilePtr> ret;
    for(auto it: overviewTiles)
        ret.push_back(it.second);
    return ret;
}
