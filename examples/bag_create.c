/************************************************************************
/      File:    bagcreate.cpp
/
/      Open Navigation Surface Working Group, 2005
/
/      - Initial implementation
/        Mark Paton, 7/22/2005
/
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "bag.h"
#include "hdf5.h"

#define GRD_SIZE 100
#define SEP_SIZE 3

void report_fatal_bag_error(const char *preamble, bagError const err)
{
    u8 *errstr;
    bagGetErrorString(err, &errstr);
    fprintf(stderr, "%s: %s\n", preamble, (char *)errstr);
    exit(1);
}

int main (int argc, char *argv[])
{
    /* Fake data - normally read from a file */
    float      surf[GRD_SIZE];
    float      uncert[GRD_SIZE];
	float      nominal_depth[GRD_SIZE];
	bagVerticalCorrector      sep_depth[SEP_SIZE][SEP_SIZE];
    float      surfRange[2];
    float      uncertRange[2];
	float	   nominal_depthRange[2];
	float	   sep_depthRange[2];
    
    /* Supporting variables */
    int        i;
    int        j;
    bagError   err;
    bagData    data;
    bagHandle  bagHandle;       /* Primary Pointer to BAG object */
    char       outFileName[256];
    char       xmlFileName[256];   

    if( argc != 3 )
    {
        fprintf( stderr, "Usage is: bagcreate <inputXMLFile> <outputBagFile> \n" );
        exit(-1);
    }
    
    strncpy( xmlFileName, argv[1], 255 );   /* Store the XML fileName */
    strncpy( outFileName, argv[2], 255 );   /* Store the BAG fileName to write */
     
    memset (&data, 0, sizeof(data));
    
    /* Configure the dynamic ranges for the data layers that we're going to write,
     * and set up the separation surface parameters.  We generate the elevation and
     * uncertainty layers one row at a time, and therefore don't have to make the
     * whole thing now.
     */

    surfRange[0] = -10.0;
    surfRange[1] = -10.0 - (float)((GRD_SIZE-1)*(GRD_SIZE-1)+GRD_SIZE)/10.0;

    uncertRange[0] = 1.0;
    uncertRange[1] = 1.0 + (float)((GRD_SIZE-1)*(GRD_SIZE-1)+GRD_SIZE)/100.0;

	nominal_depthRange[0] = 20.0;
    nominal_depthRange[1] = 20.0 + (float)((GRD_SIZE-1)*(GRD_SIZE-1)+GRD_SIZE)/20.0;

    for (i=0; i<SEP_SIZE; i++)
    {
        for (j=0; j<SEP_SIZE; j++)
        {
			sep_depth[i][j].z[0] =  -(i +0.3333) * (j+1);
			sep_depth[i][j].z[1] =  (i +0.55) * (j+1);
			sep_depth[i][j].x =  (i +10.3333) * (j+1);
			sep_depth[i][j].y =  (i +180.3333) * (j+1);
        }
    }

    sep_depthRange[0] = 0.3333;
    sep_depthRange[1] = 103.333;

    /* Convert dynamic ranges into the data for the BAG construction */

    data.min_elevation = surfRange[0];
    data.max_elevation = surfRange[1];
    data.min_uncertainty = uncertRange[0];
    data.max_uncertainty = uncertRange[1];

    /* Initial construction from the XML metadata example file provided */
    
    printf( "Creating the BAG from XML file metadata, " );
  
    err = bagInitDefinitionFromFile(&data, xmlFileName);
    if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error initialising BAG from metadata", err);

    data.compressionLevel = 1;

    err = bagFileCreate((u8*)outFileName, &data, &bagHandle);
    if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error creating BAG file", err);

    printf( "    finished initial creation of the bag, errCode = %d\n", err );

    printf( "Dims from XML r,c = [%d, %d]\n", 
            bagGetDataPointer(bagHandle)->def.nrows,
            bagGetDataPointer(bagHandle)->def.ncols );
    
    /* Write the elevation layer, constructing bogus data as we do so */
    
    for( i=0; i<GRD_SIZE; i++ )
    {
        for (j=0; j < GRD_SIZE; j++)
        {
            surf[j] = (f32)((j * (i )) % GRD_SIZE) + ((f32)j / (f32)GRD_SIZE);
        }
        
        err = bagWriteRow( bagHandle, i, 0, GRD_SIZE-1, Elevation, (void *)surf );
        if (err != BAG_SUCCESS)
            report_fatal_bag_error("Error writing elevation row", err);
    }

    /* Write the uncertainty layer, constructing bogus data as we do so */
    
    for( i=0; i<GRD_SIZE; i++ )
    {
        for (j=0; j < GRD_SIZE; j++)
        {
            uncert[j] = (f32)((j * (i )) % GRD_SIZE)/1000.0;
        }
        
        err = bagWriteRow( bagHandle, i, 0, GRD_SIZE-1, Uncertainty, (void *)uncert );
        if (err != BAG_SUCCESS)
            report_fatal_bag_error("Error writing uncertainty row", err);
    }
    err = bagUpdateSurface( bagHandle, Elevation );
    if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error updating elevation surface", err);

    err = bagUpdateSurface( bagHandle, Uncertainty );
    if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error updating uncertainty surface", err);

	/* Add optional nominal elevation dataset */
    
    bagGetDataPointer(bagHandle)->opt[Nominal_Elevation].datatype = H5T_NATIVE_FLOAT; 
    bagGetDataPointer(bagHandle)->opt[Nominal_Elevation].nrows = GRD_SIZE;
    bagGetDataPointer(bagHandle)->opt[Nominal_Elevation].ncols = GRD_SIZE;
            
	err = bagCreateOptionalDataset (bagHandle, bagGetDataPointer(bagHandle), Nominal_Elevation);
    if (err != BAG_SUCCESS)
        report_fatal_bag_error("Error creating nominal elevation dataset", err);

	bagAllocArray (bagHandle, 0, 0, GRD_SIZE-1, GRD_SIZE-1, Nominal_Elevation);
	
	for( i=0; i < GRD_SIZE; i++ )
    {
        for (j=0; j < GRD_SIZE; j++)
        {
            nominal_depth[j] = (f32)((j * (i )) % GRD_SIZE) + 1.0 + ((f32)j / (f32)GRD_SIZE);
        }
        err = bagWriteRow( bagHandle, i, 0, GRD_SIZE-1, Nominal_Elevation, (void *)nominal_depth );
		if (err != BAG_SUCCESS)
            report_fatal_bag_error("Error writing nominal depth row", err);
    }

	err = bagUpdateSurface (bagHandle, Nominal_Elevation);
    if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error updating nominal elevation surface", err);

    bagFreeArray (bagHandle, Nominal_Elevation);

    /* Add optional sep elevation dataset */
    
    bagGetDataPointer(bagHandle)->opt[Surface_Correction].nrows = SEP_SIZE;
    bagGetDataPointer(bagHandle)->opt[Surface_Correction].ncols = SEP_SIZE;

	err = bagCreateCorrectorDataset (bagHandle, bagGetDataPointer(bagHandle), 2, BAG_SURFACE_IRREGULARLY_SPACED);
    if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error creating separation corrector dataset", err);

    err = bagWriteCorrectorVerticalDatum (bagHandle, 1, (u8 *)"Test");
    if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error writing vertical datum for corrector", err);

    err = bagWriteCorrectorVerticalDatum (bagHandle, 2, (u8 *)"Unknown");
	if( err != BAG_SUCCESS )
        report_fatal_bag_error("Error writing vertical datum for corrector", err);

	for( i=0; i < SEP_SIZE; i++ )
    {
        err = bagWriteRow( bagHandle, i, 0, SEP_SIZE-1, Surface_Correction, (void *)sep_depth[i] );
		if (err != BAG_SUCCESS)
            report_fatal_bag_error("Error writing corrector surface row", err);
    }
    
    /* Close out the BAG file */

    err = bagFileClose( bagHandle );
    if (err != BAG_SUCCESS)
        report_fatal_bag_error("Error closing BAG file", err);
    
    free (data.metadata);

    printf("BAG created, Final ErrorCode = %d\n", err);

    return (0);
}
