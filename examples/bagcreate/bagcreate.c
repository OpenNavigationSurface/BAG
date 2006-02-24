//************************************************************************
//      File:    bagcreate.cpp
//
//      Open Navigation Surface Working Group, 2005
//
//      - Initial implementation
//        Mark Paton, 7/22/2005
//
//************************************************************************

#include <stdlib.h>
#include <stdio.h>

#include "bag.h"

#define GRD_SIZE 10

int main (int argc, char *argv[])
{
    /* Fake data - normally read from a file */
    float      surf[GRD_SIZE][GRD_SIZE];
    float      uncert[GRD_SIZE][GRD_SIZE];
    float      surfRange[2];
    float      uncertRange[2];
    
    int        nrows, ncols;
 
    /* Supporting variables */
    int        i;
    int        j;
    bagError   err;
    bagData    data;
    bagHandle  bagHandle;       /* Primary Pointer to BAG object */
    char       outFileName[256];
    char       xmlFileName[256];   
    FILE *xmlF;
    s32  xmlLength;
    char *xmlBuffer;

    if( argc != 3 )
    {
        fprintf( stderr, "Usage is: bagcreate <inputXMLFile> <outputBagFile>\n" );
        exit(-1);
    }
    
    strncpy( xmlFileName, argv[1], 255 );   /* Store the XML fileName */
    strncpy( outFileName, argv[2], 255 );   /* Store the BAG fileName to write */
///*     
    memset (&data, 0, sizeof(data));

    for (i=0; i<GRD_SIZE; i++)
    {
        for (j=0; j<GRD_SIZE; j++)
        {
            surf[i][j] = 10.0 + (float)(GRD_SIZE*i + j) / 10.0;
            uncert[i][j] =   1.0 + (float)(GRD_SIZE*i + j) / 100.0; 
        }
    }
    surfRange[0] = 10.0;
    surfRange[1] = 10.0 + (float)((GRD_SIZE-1)*(GRD_SIZE-1)+GRD_SIZE)/10.0;
    uncertRange[0] = 1.0;
    uncertRange[1] = 1.0 + (float)((GRD_SIZE-1)*(GRD_SIZE-1)+GRD_SIZE)/100.0;

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
        err = bagWriteRow( bagHandle, i, 0, GRD_SIZE-1, Elevation, (void *)surf[i] );
    }

    for( i=0; i<GRD_SIZE; i++ )
    {
        err = bagWriteRow( bagHandle, i, 0, GRD_SIZE-1, Uncertainty, (void *)uncert[i] );
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
//*/
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
