/* read_test.c
	purpose:  sample programs to read and dump to stdout a bag.
	authors:  dave fabre, onswg, 7/05  
                  webb mcdonald, 08/05
                  Mark Paton, 11/05   - Some simple cleanup and build support
                  Mark Paton, 04/06   - Significant enhancements, ability to export to
                                        ArcGis Ascii files, and export the XML component of
                                        the bag file.
                  Webb McDonald -- Mon Aug  2 11:27:16 2010 - added surface correctors opt dataset
*/

#include <stdio.h>

#include "bag.h" 
#include "bag_opt_surfaces.h"

/******************************************************************************/

int ProcessCommandInput( int argc, char **argv, char *gisFile, char *xmlFile, char *bagFile, int *summaryOnly );


int main( int argc, char **argv )
{
    bagHandle hnd;
    u32 i, j, k;
    bagError stat;
    f32 *data = NULL;
    bagVerticalCorrector *vdata = NULL;
    char gisExportFileName[512], xmlFileName[512], bagFileName[512], datum[55];
    int summaryOnly;
    FILE *oFile;
	s32 num_opt_datasets; 
	int opt_dataset_entities[BAG_OPT_SURFACE_LIMIT];
	bagData    xml_data;
	int NominalFound =0, sepFound = 0;

    gisExportFileName[0] = '\0';
    xmlFileName[0]       = '\0';
    bagFileName[0]       = '\0';
    datum[0]       = '\0';
    summaryOnly = 0;    /* By default verbosly output information to standard out */

    if ( argc < 2 )
    {
        printf("usage:  %s [-e arcgisAsciiFileName ] [-xml xmlFileName ] <bagFilename>\n", argv[0]);
        return EXIT_FAILURE;
    }

	ProcessCommandInput( argc, argv, gisExportFileName, xmlFileName, bagFileName, &summaryOnly );

    if( bagFileName[0] == '\0' )
    {
        printf( "Error: No bag input file specified. Exiting.\n" );
        exit(-1);
    }
    else
    {
        printf( "Input BAG file: %s\n", bagFileName );
    }

    fprintf( stdout, "trying to open: {%s}...\n", bagFileName );
    fflush( stdout );

    stat = bagFileOpen (&hnd, BAG_OPEN_READONLY, bagFileName);
    if (stat != BAG_SUCCESS)
    {
        fprintf(stderr, "bag file unavailable ! %d\n", stat);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    printf("bagFileOpen status = %d\n", stat);

    if (xmlFileName[0] != '\0')
        stat = bagInitDefinitionFromFile(&xml_data, xmlFileName);

	switch(xml_data.def.depthCorrectionType)
	{
		case Unknown_Correction:
		default:
			printf("Depth Correction type is unknown.\n");
			break;
		case True_Depth:
			printf("Depths in elevation dataset are TRUE\n");
			break;
		case Nominal_Depth_Meters:
			printf("Depths in elevation dataset are Nominal at 1500m\\s.\n");
			break;
		case Nominal_Depth_Feet:
			printf("Depths in elevation dataset are Nominal at 4800ft\\s.\n");
			break;
		case Corrected_Carters:
			printf("Depths in elevation dataset were corrected via Carter's tables\n");
			break;
		case Corrected_Matthews:
			printf("Depths in elevation dataset were corrected via Matthew's tables\n");
			break;
	}
		

    printf("BAG row/column extents: %dx%d\n", bagGetDataPointer(hnd)->def.nrows, bagGetDataPointer(hnd)->def.ncols);
    printf("BAG South-West Corner : %lf, %lf\n", bagGetDataPointer(hnd)->def.swCornerX, bagGetDataPointer(hnd)->def.swCornerY );
    printf("BAG Node Spacing      : %lf, %lf\n", bagGetDataPointer(hnd)->def.nodeSpacingX, bagGetDataPointer(hnd)->def.nodeSpacingY );
    printf("BAG Horiz. CoordSys   : %s\n", bagGetDataPointer(hnd)->def.referenceSystem.horizontalReference);
    printf("BAG Vert.  CoordSys   : %s\n", bagGetDataPointer(hnd)->def.referenceSystem.verticalReference);
    
    
/*
    stat = bagReadXMLStream( hnd );
    printf("status for bagReadDataset(Metadata) = %d\n", stat);
*/
	/* read the BAG file to determine whether or not any optional datasets exist */
	stat = bagGetOptDatasets(&hnd, &num_opt_datasets, opt_dataset_entities);

	if(num_opt_datasets ==0)
	{
		printf("\n Only mandatory datasets were found in the BAG.\n");
		if(xml_data.def.depthCorrectionType == Nominal_Depth_Meters || 
			xml_data.def.depthCorrectionType == Nominal_Depth_Feet)
			printf("\nThe elevation dataset however includes only Nominal depth not True depth\n\n");
	}
    else
    {
        printf("\nOptional datasets have been found\n");
    }

	for(i=0; i<(u32)num_opt_datasets; i++)
	{
		if(opt_dataset_entities[i] == Nominal_Elevation)
		{
			printf("\n Nominal data has been found in the bag and is contained in the optional dataset\n\n");
			NominalFound = 1;
		}
        if(opt_dataset_entities[i] == Surface_Correction)
		{
			printf("\n Vertical Datum Corrections have been found in the bag and are contained in an optional dataset\n\n");
			sepFound = 1;
		}
	}

    if( !summaryOnly )    /* Don't display if just the summary is requested */
    {
        printf("metadata = {%s}\n\n", bagGetDataPointer(hnd)->metadata);
    }

    if( xmlFileName[0] != '\0' )   /* Check request to export the XML Metadata to a file */
    {
        if( (oFile = fopen( xmlFileName, "wb" )) == NULL )
        {
            fprintf( stderr, "ERROR: Opening file %s for XML export\n", xmlFileName );
            exit(-1);
        }
        fprintf( oFile, "%s", bagGetDataPointer(hnd)->metadata );
        fclose( oFile );
    }

    /* Read data using the ReadRow method */
    if( !summaryOnly )      /* Display heights and uncertainties  */
    {
        /* fprintf(stdout, "Elevation:=  {\n\t"); */
/*         fflush(stdout); */
                
/*         data = calloc (bagGetDataPointer(hnd)->def.ncols, sizeof(f32)); */
/*         for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++) */
/*         { */
/*             bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->def.ncols-1, Elevation, data); */
/*             for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++) */
/*             { */
/*                 fprintf(stdout, "%0.3f\t", data[j]); */
/*             } */
/*             fprintf(stdout, "\n\t"); */
/*             fflush(stdout);  */
/*         } */
/*         free(data); */
/*         fprintf(stdout, "\n\t}"); */
/*         fflush(stdout); */

/*         fprintf(stdout, "uncertainty:=  {\n\t"); */
/*         fflush(stdout); */

/*         data = calloc (bagGetDataPointer(hnd)->def.ncols, sizeof(f32)); */
/*         for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++) */
/*         { */
/*             bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->def.ncols-1, Uncertainty, data); */
/*             for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++) */
/*             { */
/*                 fprintf(stdout, "%0.3f\t", data[j]); */
/*             } */
/*             fprintf(stdout, "\n\t"); */
/*             fflush(stdout);  */
/*         } */
/*         free(data); */
/*         fprintf(stdout, "\n\t}\n"); */
/*         fflush(stdout); */

		/* if optional datasets are present read them in */
		if(num_opt_datasets > 0)
		{
			if(NominalFound)
			{
				fprintf(stdout, "Nominal Elevation:=  {\n\t");
				fflush(stdout);

				bagGetOptDatasetInfo(&hnd, Nominal_Elevation);

				                
				data = calloc (bagGetDataPointer(hnd)->opt[Nominal_Elevation].ncols, sizeof(f32));
				for (i=0; i < bagGetDataPointer(hnd)->opt[Nominal_Elevation].nrows; i++)
				{
					bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->opt[Nominal_Elevation].ncols-1, Nominal_Elevation, data);
					for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
					{
						fprintf(stdout, "%0.3f\t", data[j]);
					}
					fprintf(stdout, "\n\t");
					fflush(stdout); 
				}
				
				free(data);
                bagFreeInfoOpt (hnd);
				fprintf(stdout, "}\n\t");
				fflush(stdout);
            }

            if(sepFound)
			{
				fprintf(stdout, "Vertical Datum Correctors :=  {\n\t");
				fflush(stdout);

				bagGetOptDatasetInfo(&hnd, Surface_Correction);

				                
				vdata = calloc (bagGetDataPointer(hnd)->opt[Surface_Correction].ncols, sizeof(bagVerticalCorrector));
				for (i=0; i < bagGetDataPointer(hnd)->opt[Surface_Correction].nrows; i++)
				{
					bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->opt[Surface_Correction].ncols-1, Surface_Correction, vdata);
					for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
					{
                        u32 limit;
                        bagGetNumSurfaceCorrectors  (hnd, &limit);
                        for (k=0; k < limit; k++)
                        {
                            fprintf(stdout, "Z%d=%0.3lf ", k, vdata[j].z[k]); 
                        }
						fprintf(stdout, "X=%0.3lf Y=%0.3lf\t", vdata[j].x, vdata[j].y);
					}
					fprintf(stdout, "\n\t");
					fflush(stdout); 

                    fprintf(stdout, "ROW %d\n", i);
				}
				
				free(vdata);
				bagFreeInfoOpt (hnd);
				fprintf(stdout, "\t}\n");
				fflush(stdout);
			}
		}
    } 
    
    if( gisExportFileName[0] != '\0' )
    {
        /* This is similar to the code above but I choose to implement the export
           separately to keep the code easier to read which is suible for a sample
           demonstration application. 
        *** */
        char outName[512];
        /* Export the height data to <file>_heights.asc */
        sprintf( outName, "%s_heights.asc", gisExportFileName );
        if( (oFile = fopen( outName, "wb" )) == NULL )
        {
            fprintf( stderr, "ERROR: Opening file %s for height data export\n", outName );
            exit(-1);
        }
        fprintf( oFile, "ncols %d\n", bagGetDataPointer(hnd)->def.ncols );
        fprintf( oFile, "nrows %d\n", bagGetDataPointer(hnd)->def.nrows );
        fprintf( oFile, "xllcenter %lf\n", bagGetDataPointer(hnd)->def.swCornerX );
        fprintf( oFile, "yllcenter %lf\n", bagGetDataPointer(hnd)->def.swCornerY);
        fprintf( oFile, "cellsize %lf\n", bagGetDataPointer(hnd)->def.nodeSpacingX );
        data = calloc (bagGetDataPointer(hnd)->def.ncols, sizeof(f32));
        for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++)
        {
            bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->def.ncols-1, Elevation, data);
            for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
            {
                fprintf(oFile, "%0.3f\t", data[j]);
            }
            fprintf(oFile, "\n");
        }
        free(data);
        fclose( oFile );

        /* Export the uncertainty data to <file>_heights.asc */
        sprintf( outName, "%s_uncrt.asc", gisExportFileName );
        if( (oFile = fopen( outName, "wb" )) == NULL )
        {
            fprintf( stderr, "ERROR: Opening file %s for height data export\n", outName );
            exit(-1);
        }
        fprintf( oFile, "ncols %d\n", bagGetDataPointer(hnd)->def.ncols );
        fprintf( oFile, "nrows %d\n", bagGetDataPointer(hnd)->def.nrows );
        fprintf( oFile, "xllcenter %lf\n", bagGetDataPointer(hnd)->def.swCornerX );
        fprintf( oFile, "yllcenter %lf\n", bagGetDataPointer(hnd)->def.swCornerY);
        fprintf( oFile, "cellsize %lf\n", bagGetDataPointer(hnd)->def.nodeSpacingX );
        data = calloc (bagGetDataPointer(hnd)->def.ncols, sizeof(f32));
        for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++)
        {
            bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->def.ncols-1, Uncertainty, data);
            for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
            {
                fprintf(oFile, "%0.3f\t", data[j]);
            }
            fprintf(oFile, "\n");
        }
        free(data);

		/* Export the uncertainty data to <file>_nominal.asc */
		if(num_opt_datasets > 0)
		{
			sprintf( outName, "%s_nominal.asc", gisExportFileName );
			if( (oFile = fopen( outName, "wb" )) == NULL )
			{
				fprintf( stderr, "ERROR: Opening file %s for nominal data export\n", outName );
				exit(-1);
			}
			fprintf( oFile, "ncols %d\n", bagGetDataPointer(hnd)->def.ncols );
			fprintf( oFile, "nrows %d\n", bagGetDataPointer(hnd)->def.nrows );
			fprintf( oFile, "xllcenter %lf\n", bagGetDataPointer(hnd)->def.swCornerX );
			fprintf( oFile, "yllcenter %lf\n", bagGetDataPointer(hnd)->def.swCornerY);
			fprintf( oFile, "cellsize %lf\n", bagGetDataPointer(hnd)->def.nodeSpacingX );
			data = calloc (bagGetDataPointer(hnd)->def.ncols, sizeof(f32));
			for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++)
			{
				bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->def.ncols-1, Nominal_Elevation, data);
				for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
				{
					fprintf(oFile, "%0.3f\t", data[j]);
				}
				fprintf(oFile, "\n");
			}
			free(data);
		}
    }

    /* uses ReadDataset - reads the data arrays in one shot - an alternate method to read the data */

    if (!summaryOnly)
    {
        bagAllocArray (hnd, 0, 0, bagGetDataPointer(hnd)->def.nrows-1, bagGetDataPointer(hnd)->def.ncols-1, Elevation);
        bagAllocArray (hnd, 0, 0, bagGetDataPointer(hnd)->def.nrows-1, bagGetDataPointer(hnd)->def.ncols-1, Uncertainty);

        bagReadDataset (hnd, Elevation);
        bagReadDataset (hnd, Uncertainty);

        fprintf(stdout, "Elevation:=  {\n\t");
        fflush(stdout);

        for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++)
        {
            for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
            {
                fprintf(stdout, "%0.3f\t", bagGetDataPointer(hnd)->elevation[i][j]);
            }
            fprintf(stdout, "\n\t");
            fflush(stdout); 
        }

        fprintf(stdout, "\n\t}");
        fflush(stdout);

        fprintf(stdout, "uncertainty:=  {\n\t");
        fflush(stdout);

        for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++)
        {
            for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
            {
                fprintf(stdout, "%0.3f\t", bagGetDataPointer(hnd)->uncertainty[i][j]);
            }
            fprintf(stdout, "\n\t");
            fflush(stdout); 
        }

        fprintf(stdout, "\n\t}\n");
        fflush(stdout);

        bagFreeArray (hnd, Elevation);
        bagFreeArray (hnd, Uncertainty);
    }

    bagUpdateSurface (hnd, Elevation);
    bagUpdateSurface (hnd, Uncertainty);


    fprintf(stdout, "min_elv    %0.3f, max_elv    %0.3f\n", 
            bagGetDataPointer(hnd)->min_elevation, 
            bagGetDataPointer(hnd)->max_elevation);
    fflush(stdout);
    fprintf(stdout, "min_uncert %0.3f, max_uncert %0.3f\n", 
            bagGetDataPointer(hnd)->min_uncertainty, 
            bagGetDataPointer(hnd)->max_uncertainty);
    fflush(stdout);

    stat =  bagFileClose( hnd );
    printf("stat for bagFileClose = %d\n", stat);

    return EXIT_SUCCESS;

} /* main */

int ProcessCommandInput( int argc, char **argv, char *gisFile, char *xmlFile, char *bagFile, int *summaryOnly )
{
    int i;

    i = 1;
    while( i < argc )   /*  Process until done */
    {
        if( strcmp( argv[i], "-e") == 0 )
        {
            if( i < (argc-1) )
            {
                strncpy( gisFile, argv[i+1], 511 );
            }
            else
            {
                fprintf( stderr, "ERROR: Missing filename for the -e option\n" );
                exit(-1);
            }
            i += 2;
        }
        else if( strcmp( argv[i], "-xml" ) == 0 )
        {
            if( i < (argc-1) )
            {
                strncpy( xmlFile, argv[i+1], 511 );
            }
            else
            {
                fprintf( stderr, "ERROR: Missing filename for the -xml option\n" );
                exit(-1);
            }
            i += 2;            
        }
        else if( strcmp( argv[i], "-summary" ) == 0 )
        {
            *summaryOnly = 1;    /* Set flag to indicate that only a summary of output is desired */
            i++;
        }
        else if( strcmp( argv[i], "-h" ) == 0 )
        {
            printf("usage: bagread [-summary ] [-e arcgisAsciiBaseName ] [-xml xmlFileName ] <bagFilename>\n" );
            printf("    If -summary is given the XML and data arrays will not be displayed on the screen. The\n" );
            printf("-e option writes out two ArcGIS compatible ascii files for the height and uncertainty\n" );
            printf(" data. The files use the supplied filename and add _height.asc and _uncrt.asc for the\n" );
            printf("two exported.grids. The -xml option writes the XML metadata to the specified filename.\n\n" );
            exit( 0 );
        }
        else
        {
            if( argv[i][0] == '-' )
            {
                fprintf( stderr, "ERROR: Unexpected command line parameter: %s\n", argv[i] );
                exit( -1 );
            }
            strncpy( bagFile, argv[i], 511 );
            i++;
        }
    }
    return EXIT_SUCCESS;
}


