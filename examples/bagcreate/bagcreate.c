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

#define GRD_SIZE 1200
#define SEP_SIZE 3

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
    
    int        nrows, ncols;
 
    /* Supporting variables */
    int        i;
    int        j;
    bagError   err;
    bagData    data;
	bagDataOpt opt_data_nominal;
	bagDataOpt opt_data_sep;
    bagHandle  bagHandle;       /* Primary Pointer to BAG object */
	bagHandle_opt  bagHandle_nominal; /* pointer to the optional BAG dataset */
	bagHandle_opt  bagHandle_sep; /* pointer to the optional BAG dataset */
    char       outFileName[256];
    char       xmlFileName[256];   
    FILE *xmlF;
    s32  xmlLength;
    char *xmlBuffer;

    if( argc != 3 )
    {
        fprintf( stderr, "Usage is: bagcreate <inputXMLFile> <outputBagFile> \n" );
        exit(-1);
    }
    
    strncpy( xmlFileName, argv[1], 255 );   /* Store the XML fileName */
    strncpy( outFileName, argv[2], 255 );   /* Store the BAG fileName to write */
     
    memset (&data, 0, sizeof(data));
	memset (&opt_data_nominal, 0, sizeof(opt_data_nominal));
	memset (&opt_data_sep, 0, sizeof(opt_data_sep));

    /* for (i=0; i<GRD_SIZE; i++) */
/*     { */
/*         for (j=0; j<GRD_SIZE; j++) */
/*         { */
/*             surf[i][j] = 0 - (10.0 + (float)(GRD_SIZE*i + j) / 10.0); */
/*             uncert[i][j] =   1.0 + (float)(GRD_SIZE*i + j) / 100.0;  */
/* 			nominal_depth[i][j] = 20.0 + (float)(GRD_SIZE*i + j) / 20.0; */
/*         } */
/*     } */

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

    printf( "Attempting to initialize a BAG!\n" );

    data.min_elevation = surfRange[0];
    data.max_elevation = surfRange[1];
    data.min_uncertainty = uncertRange[0];
    data.max_uncertainty = uncertRange[1];

    printf( "Attempting to read the XML file for Bagination of it!\n" );
/*    
    if( (xmlF = fopen( xmlFileName, "rb" )) == NULL )
    {
        fprintf( stderr, "ERROR: Failed to open XML file to read into buffer\n" );
        exit(-1);
    }
    fseek( xmlF, SEEK_END, 0 );
    xmlLength = ftell(xmlF) + 1;
    xmlBuffer = (char *)malloc( sizeof(char)*(xmlLength+1) );
    rewind( xmlF );
    fread( xmlBuffer, sizeof(char), xmlLength, xmlF );
    xmlBuffer[xmlLength] = '\0';
    fclose( xmlF );
    
    printf( "   ---> Read XML data into buffer\n" );
//    bagValidateMetadataBuffer( xmlBuffer, xmlLength, &err );
    printf( "   ---> Finished validating xml, errCode = %d\n", err );
*/
    printf( "Creating the BAG, " );    
  
    err = bagInitDefinitionFromFile(&data, xmlFileName);
    printf( "    ErrorCode for bagInitDefinition = %d\n", err );
    if( err != BAG_SUCCESS )
    {
        char *errstr;
        if( bagGetErrorString( err, &errstr ) == BAG_SUCCESS )
        {
            fprintf( stderr, "Error create Bag: {%s}\n", errstr );
        }
    }

    opt_data_nominal.compressionLevel = 
        opt_data_sep.compressionLevel = 
        data.compressionLevel = 1;

    err = bagFileCreate( outFileName, &data, &bagHandle);
    if( err != BAG_SUCCESS )
    {
        char *errstr;
        if( bagGetErrorString( err, &errstr ) == BAG_SUCCESS )
        {
            fprintf( stderr, "Error create Bag: {%s}\n", errstr );
        }
    }
    printf( "    finished initial creation of the bag, errCode = %d\n", err );

    printf( "Dims from XML r,c = [%d, %d]\n", 
            bagGetDataPointer(bagHandle)->def.nrows,
            bagGetDataPointer(bagHandle)->def.ncols );
    for( i=0; i<GRD_SIZE; i++ )
    {
        for (j=0; j < GRD_SIZE; j++)
        {
            if ( j > 500 && j < 1000)
                surf[j] = 20.0 + (float)(GRD_SIZE*i + j) / 20.0;
            else
                surf[j] = 0;
        }
        
        err = bagWriteRow( bagHandle, i, 0, GRD_SIZE-1, Elevation, (void *)surf );
    }

    for( i=0; i<GRD_SIZE; i++ )
    {
        for (j=0; j < GRD_SIZE; j++)
        {
            if ( j > 500 && j < 1000)
                uncert[j] = 20.0 + (float)(GRD_SIZE*i + j) / 20.0;
            else
                uncert[j] = 0;
        }
        
        err = bagWriteRow( bagHandle, i, 0, GRD_SIZE-1, Uncertainty, (void *)uncert );
    }
    err = bagUpdateSurface( bagHandle, Elevation );
    if( err != BAG_SUCCESS )
    {
        char *errstr;
        if( bagGetErrorString( err, &errstr ) == BAG_SUCCESS )
        {
            fprintf( stderr, "Error create Bag: {%s}\n", errstr );
        }
    }
    err = bagUpdateSurface( bagHandle, Uncertainty );
    if( err != BAG_SUCCESS )
    {
        char *errstr;
        if( bagGetErrorString( err, &errstr ) == BAG_SUCCESS )
        {
            fprintf( stderr, "Error create Bag: {%s}\n", errstr );
        }
    }


	/* adding optional nominal elevation dataset */
	opt_data_nominal.def = data.def;
	opt_data_nominal.datatype = H5T_NATIVE_FLOAT; 
	err = bagCreateOptionalDataset (bagHandle, &bagHandle_nominal, &opt_data_nominal, Nominal_Elevation);

	bagAllocOptArray (bagHandle_nominal, 0, 0, 
                        GRD_SIZE-1, GRD_SIZE-1);
	
	for( i=0; i < GRD_SIZE; i++ )
    {
        for (j=0; j < GRD_SIZE; j++)
        {
            if ( j > 500 && j < 1000)
                nominal_depth[j] = 20.0 + (float)(GRD_SIZE*i + j) / 20.0;
            else
                nominal_depth[j] = 0;

        }
        err = bagWriteOptRow( bagHandle, bagHandle_nominal, i, 0, GRD_SIZE-1, Nominal_Elevation, (void *)nominal_depth );
		
    }

	err = bagUpdateOptSurface (bagHandle, bagHandle_nominal, Nominal_Elevation);
    if( err != BAG_SUCCESS )
    {
        char *errstr;
        if( bagGetErrorString( err, &errstr ) == BAG_SUCCESS )
        {
            fprintf( stderr, "Error create Bag: {%s}\n", errstr );
        }
    }
    bagFreeOptArray (bagHandle_nominal);
    bagFreeInfoOpt  (bagHandle_nominal);

    /* adding optional sep elevation dataset */
	opt_data_sep.def = data.def;
    opt_data_sep.def.ncols = SEP_SIZE;
    opt_data_sep.def.nrows = SEP_SIZE;

	err = bagCreateCorrectorDataset (bagHandle, &bagHandle_sep, &opt_data_sep, 2, BAG_SURFACE_IRREGULARLY_SPACED);
    if( err != BAG_SUCCESS )
    {
        char *errstr;
        if( bagGetErrorString( err, &errstr ) == BAG_SUCCESS )
        {
            fprintf( stderr, "Error create Bag: {%s}\n", errstr );
        }
    }

	for( i=0; i < SEP_SIZE; i++ )
    {
        err = bagWriteOptRow( bagHandle, bagHandle_sep, i, 0, SEP_SIZE-1, Surface_Correction, (void *)sep_depth[i] );
		
    }

    bagFreeInfoOpt  (bagHandle_sep);

    err = bagFileClose( bagHandle );   
    if( err != BAG_SUCCESS )
    {
        char *errstr;
        if( bagGetErrorString( err, &errstr ) == BAG_SUCCESS )
        {
            fprintf( stderr, "Error create Bag: {%s}\n", errstr );
        }
    }
    printf(    "Excellent... our bag is cooked!, Final ErrorCode = %d\n", err );

    return (0);
}
