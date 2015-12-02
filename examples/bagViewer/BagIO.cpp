#include "BagIO.h"

#include <iostream>
#include <memory>
#include <QString>

BagIO::BagIO(GLuint primitiveReset):
    bag(0),
    primitiveReset(primitiveReset)
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
    
    
    double dx = bd->def.nodeSpacingX;
    double dy = bd->def.nodeSpacingY;
    
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
    
    float nodeZeroZero;
    f64 *nzzx, *nzzy;
    bagReadNodePos(bag,0,0,Elevation,&nodeZeroZero,&nzzx,&nzzy);
    std::cerr << "0,0: " << nodeZeroZero << " pos: " << *nzzx << ", " << *nzzy << std::endl;
    free(nzzx);
    free(nzzy);
    
    bool inElement = false;
    typedef std::vector<float> DataRowVec;
    typedef std::shared_ptr<DataRowVec> DataRowVecPtr;
    typedef std::map<std::pair<u32,u32>,GLuint> IndexMap;
    typedef std::shared_ptr<IndexMap> IndexMapPtr;
    
    DataRowVecPtr dataRow,lastDataRow;
    std::vector<float> unRow(bd->def.ncols);
    IndexMapPtr lastRowIndecies, currentRowIndecies;
    for(u32 i = 0; i < bd->def.nrows; ++i)
    {
        inElement = false;
        lastDataRow = dataRow;
        dataRow = DataRowVecPtr(new DataRowVec(bd->def.ncols));
        bagReadRow(bag,i,0,bd->def.ncols-1,Elevation,dataRow->data());
        bagReadRow(bag,i,0,bd->def.ncols-1,Uncertainty,unRow.data());
        lastRowIndecies = currentRowIndecies;
        currentRowIndecies = IndexMapPtr(new IndexMap);
        for(u32 j = 0; j < bd->def.ncols; ++j)
        {
            if((*dataRow)[j]!=BAG_NULL_ELEVATION)
            {
                (*currentRowIndecies)[IndexMap::key_type(i,j)]=g.elevationVerticies.size()/3;
                g.elevationVerticies.push_back(j*dx);
                g.elevationVerticies.push_back(i*dy);
                g.elevationVerticies.push_back((*dataRow)[j]);
                g.uncertainties.push_back(unRow[j]);
                if(lastRowIndecies && lastRowIndecies->count(IndexMap::key_type(i-1,j)))
                {
                    g.indecies.push_back((*lastRowIndecies)[IndexMap::key_type(i-1,j)]);
                    g.indecies.push_back((g.elevationVerticies.size()/3)-1);
                    inElement = true;
                    if(j < bd->def.ncols-1 && (*dataRow)[j+1] != BAG_NULL_ELEVATION)
                    {
                        QVector3D v1(dx,0.0,(*lastDataRow)[j]-(*dataRow)[j]);
                        QVector3D v2(0.0,dy,(*dataRow)[j+1]-(*dataRow)[j]);
                        QVector3D n = QVector3D::normal(v1,v2);
                        g.normals.push_back(n.x());
                        g.normals.push_back(n.y());
                        g.normals.push_back(n.z());
                    }
                    else
                    {
                        g.normals.push_back(0.0);
                        g.normals.push_back(0.0);
                        g.normals.push_back(1.0);
                    }
                }
                else
                {
                    if(inElement)
                    {
                        g.indecies.push_back(primitiveReset);
                        inElement = false;
                    }
                    g.normals.push_back(0.0);
                    g.normals.push_back(0.0);
                    g.normals.push_back(1.0);
                }
            }
            else
            {
                if(inElement)
                {
                    g.indecies.push_back(primitiveReset);
                    inElement = false;
                }
            }
        }
        if(inElement)
            g.indecies.push_back(primitiveReset);
    }
    inElement = false;
    
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

void BagIO::close()
{
    bagFileClose(bag);
    bag = 0;
    g.reset();
    vrg.reset();
}



void BagIO::Geometry::reset()
{
    elevationVerticies.resize(0);
    normals.resize(0);
    uncertainties.resize(0);
    indecies.resize(0);
}
