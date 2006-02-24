/* read_test.c
	purpose:  sample programs to read and dump to stdout a bag.
	authors:  dave fabre, onswg, 7/05  
                  webb mcdonald, 08/05
                  Mark Paton, 11/05     - Some simple cleanup and build support
*/

#include <stdio.h>

#include "bag.h" 

/******************************************************************************/
int main( int argc, char **argv )
{
    bagHandle hnd;
    u32 i, j;
    bagError stat;
    f32 *data = NULL;
    
    if ( argc < 2 )
    {
        printf("usage:  %s <bagFilename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fprintf( stdout, "trying to open: {%s}...\n", argv[1] );
    fflush( stdout );
    stat = bagFileOpen (&hnd, BAG_OPEN_READ_WRITE, argv[1]);
    
    if (stat != BAG_SUCCESS)
    {
        fprintf(stderr, "bag file unavailable ! %d\n", stat);
        fflush(stderr);
        return EXIT_FAILURE;
    }


    printf("bagFileOpen status = %d\n", stat);
    
    printf("BAG extents: %dx%d\n", bagGetDataPointer(hnd)->def.nrows, bagGetDataPointer(hnd)->def.ncols);
    
    stat = bagReadXMLStream( hnd );

    /*
       Note that the metadata is not null terminated so the printf will probably
       print a bunch of junk at the end of the XML file.
    */
    printf("stat for bagReadDataset(Metadata) = %d\n", stat);
    printf("metadata = {%s}\n\n", bagGetDataPointer(hnd)->metadata);

    /* uses ReadRow */
    if (1)
    {
        fprintf(stdout, "Elevation:=  {\n\t");
        fflush(stdout);
                
        data = calloc (bagGetDataPointer(hnd)->def.ncols, sizeof(f32));
        for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++)
        {
            bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->def.ncols-1, Elevation, data);
            for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
            {
                fprintf(stdout, "%0.3f\t", data[j]);
            }
            fprintf(stdout, "\n\t");
            fflush(stdout); 
        }
        free(data);
        fprintf(stdout, "\n\t}");
        fflush(stdout);

        fprintf(stdout, "uncertainty:=  {\n\t");
        fflush(stdout);

        data = calloc (bagGetDataPointer(hnd)->def.ncols, sizeof(f32));
        for (i=0; i < bagGetDataPointer(hnd)->def.nrows; i++)
        {
            bagReadRow (hnd, i, 0, bagGetDataPointer(hnd)->def.ncols-1, Uncertainty, data);
            for (j=0; j < bagGetDataPointer(hnd)->def.ncols; j++)
            {
                fprintf(stdout, "%0.3f\t", data[j]);
            }
            fprintf(stdout, "\n\t");
            fflush(stdout); 
        }
        free(data);
        fprintf(stdout, "\n\t}\n");
        fflush(stdout);
    } 


    /* uses ReadDataset - reads the data arrays in one shot - an alternate method to read the data */
/*
    if (1)
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
*/

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


