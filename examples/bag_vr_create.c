/*! \file bag_vr_create.c
 * \brief Generate a BAG with variable resolution extensions with synthetic data
 *
 * In order to demonstrate how to deal with variable resolution extensions in
 * a BAG-VR, this code generates a file ab initio (assuming that the XML
 * metadata information is available), and makes synthetic data for it.
 *
 * Open Navigation Surface Working Group, 2015.  Visit the project website at
 * http://www.opennavsurf.org
 */

#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

#include "getopt.h"

#include "bag.h"

typedef enum {
    XML_FILENAME = 1,
    OUTPUT_BAG,
    ARGC_EXPECTED
} Cmd;

void Syntax(void)
{
    printf("bag_vr_create [%s] - Create a basic variable-resolution BAG file.\n", __DATE__);
    printf("Syntax: bag_vr_create [opt] <xml_file><output>\n");
    printf("Options:\n");
    printf(" -h Generate this help information.\n");
}

void report_library_error(char *prelude, bagError errcode)
{
    char *errstr = NULL;
    bagGetErrorString(errcode, (u8**)errstr);
    printf("error: %s (BAG library said: \"%s\").\n", prelude, errstr);
}

void generate_data(u32 const x, u32 const length, f32 const y_min, f32 const y_max, f32 *data)
{
    u32 p;
    
    for (p = 0; p < length; ++p) {
        data[p] = (f32)(y_min + 0.5*(y_max - y_min)*(1.0+cos(2.0*M_PI*p/length + x/length)));
    }
}

Bool construct_basic_bag(bagHandle *handle, bagData *data, char *xml_file, char *bag_file)
{
    const f32 max_elevation = 10.0f;
    const f32 min_elevation = -10.0f;
    const f32 max_uncertainty = 5.0f;
    const f32 min_uncertainty = 0.0f;
    
    bagError    errcode;
    
    u32 row, lowres_rows, lowres_cols;
    
    memset(data, 0, sizeof(bagData));
    
    data->min_elevation = min_elevation;
    data->max_elevation = max_elevation;
    data->min_uncertainty = min_uncertainty;
    data->max_uncertainty = max_uncertainty;
    
    if ((errcode = bagInitDefinitionFromFile(data, xml_file)) != BAG_SUCCESS) {
        report_library_error("failed to use XML for definition", errcode);
        return False;
    }
    
    data->compressionLevel = 1;
    if ((errcode = bagFileCreate((u8*)bag_file, data, handle)) != BAG_SUCCESS) {
        report_library_error("failed creating BAG file", errcode);
        return False;
    }
    
    lowres_rows = bagGetDataPointer(*handle)->def.nrows;
    lowres_cols = bagGetDataPointer(*handle)->def.ncols;
    
    f32 *depth = (f32*)malloc(sizeof(f32)*lowres_cols);
    f32 *uncrt = (f32*)malloc(sizeof(f32)*lowres_cols);
    
    for (row = 0; row < lowres_rows; ++row) {
        generate_data(row, lowres_cols, min_elevation, max_elevation, depth);
        generate_data(row, lowres_cols, min_uncertainty, max_uncertainty, uncrt);
        if ((errcode = bagWriteRow(*handle, row, 0, lowres_cols-1, Elevation, depth)) != BAG_SUCCESS) {
            report_library_error("failed writing depth data", errcode);
            free(depth); free(uncrt);
            return False;
        }
        if ((errcode = bagWriteRow(*handle, row, 0, lowres_cols-1, Uncertainty, uncrt)) != BAG_SUCCESS) {
            report_library_error("failed writing uncertainty data", errcode);
            free(depth); free(uncrt);
            return False;
        }
    }
    free(depth); free(uncrt);
    if ((errcode = bagUpdateSurface(*handle, Elevation)) != BAG_SUCCESS) {
        report_library_error("failed updating limits in depth", errcode);
        return False;
    }
    if ((errcode = bagUpdateSurface(*handle, Uncertainty)) != BAG_SUCCESS) {
        report_library_error("failed updating limits in uncertainty", errcode);
        return False;
    }
    return True;
}

void generate_vr_data(u32 const rows, f32 const min_z, f32 const max_z, f32 const min_u, f32 const max_u, bagVarResRefinementGroup *data)
{
    u32 row, col, index = 0;
    
    for (row = 0; row < rows; ++row) {
        for (col = 0; col < rows; ++col, ++index) {
            data[index].depth = min_z + 0.5f*(max_z - min_z)*(1.0f+ (f32) cos(2*M_PI*col/rows - (f32)row/rows));
            data[index].depth_uncrt = min_u + 0.5f*(max_u - min_u)*(1.0f + (f32) cos(2*M_PI*col/rows - (f32)row/rows));
        }
    }
}

Bool augment_vr_bag(bagHandle handle, bagData *data)
{
    const f32 min_elevation = -10.0f;
    const f32 max_elevation = 10.0f;
    const f32 min_uncertainty = 0.0f;
    const f32 max_uncertainty = 10.0f;
    
    bagError    errcode;
    u32 lowres_rows = data->def.nrows;
    u32 lowres_cols = data->def.ncols;
    u32 n_refinements;
    
    u32 *refinement_cols = NULL;
    u32 i, row, col, n_cells, total_refinements, ref;
    
    bagOptNodeGroup             *lowres_aux = NULL;
    bagVarResMetadataGroup      *vr_metadata = NULL;
    bagVarResRefinementGroup    *vr_refinements = NULL;
    bagVarResNodeGroup          *vr_aux = NULL;
    
    refinement_cols = (u32*)malloc(sizeof(u32)*lowres_cols);
    lowres_aux = (bagOptNodeGroup*)malloc(sizeof(bagOptNodeGroup)*lowres_cols);
    vr_metadata = (bagVarResMetadataGroup*)malloc(sizeof(bagVarResMetadataGroup)*lowres_cols);
    vr_refinements = (bagVarResRefinementGroup*)malloc(sizeof(bagVarResRefinementGroup)*21*21);
    vr_aux = (bagVarResNodeGroup*)malloc(sizeof(bagVarResNodeGroup)*21*21);
    
    if (lowres_aux == NULL || vr_metadata == NULL || vr_refinements == NULL || vr_aux == NULL) {
        printf("error: out of memory getting data buffers.\n");
        if (refinement_cols != NULL) free(refinement_cols);
        if (lowres_aux != NULL) free(lowres_aux);
        if (vr_metadata != NULL) free(vr_metadata);
        if (vr_refinements != NULL) free(vr_refinements);
        if (vr_aux != NULL) free(vr_aux);
        return False;
    }
    
    n_refinements = 0;
    for (i = 0; i < lowres_cols; ++i) {
        refinement_cols[i] = 2 + (i % 20);
        n_refinements += refinement_cols[i]*refinement_cols[i];
    }
    n_refinements *= lowres_rows;
    
    if ((errcode = bagCreateVariableResolutionLayers(handle, n_refinements, True)) != BAG_SUCCESS) {
        report_library_error("failed adding VR layers", errcode);
        free(refinement_cols); free(lowres_aux); free(vr_metadata); free(vr_refinements); free(vr_aux);
        return False;
    }
    if ((errcode = bagGetOptDatasetInfo(&handle, VarRes_Metadata_Group)) != BAG_SUCCESS ||
        (errcode = bagGetOptDatasetInfo(&handle, VarRes_Refinement_Group)) != BAG_SUCCESS ||
        (errcode = bagGetOptDatasetInfo(&handle, VarRes_Node_Group)) != BAG_SUCCESS ||
        (errcode = bagGetOptDatasetInfo(&handle, Node_Group)) != BAG_SUCCESS) {
        report_library_error("failed getting variable resolution layer information", errcode);
        free(refinement_cols); free(lowres_aux); free(vr_metadata); free(vr_refinements); free(vr_aux);
        return False;
    }
    
    /* The Node_Group layer is used to preserve the auxiliary information for
     * the low resolution data */
    for (col = 0; col < lowres_cols; ++col) {
        lowres_aux[col].num_hypotheses = 1 + col % 10;
        lowres_aux[col].hyp_strength = 4.0f*(col % 10)/10.0f;
    }
    for (row = 0; row < lowres_rows; ++row) {
        if ((errcode = bagWriteRow(handle, row, 0, lowres_cols-1, Node_Group, lowres_aux)) != BAG_SUCCESS) {
            report_library_error("failed writing low resolution auxiliary data", errcode);
            free(refinement_cols); free(lowres_aux); free(vr_metadata); free(vr_refinements); free(vr_aux);
            return False;
        }
    }
    /* The VarRes_Metadata_Group is used to keep information about the size of the
     * refined grids, and where to find them in the list of refinements and
     * auxiliary data.  The VarRes_Refinement_Group has the actual refinements,
     * and the VarRes_Node_Group has the auxiliary information.
     */
    n_cells = 0;
    for (row = 0; row < lowres_rows; ++row) {
        for(col = 0; col < lowres_cols; ++col) {
            vr_metadata[col].index = n_cells;
            vr_metadata[col].dimensions_x = refinement_cols[col];
			vr_metadata[col].dimensions_y = refinement_cols[col];
            vr_metadata[col].resolution_x = (f32)(data->def.nodeSpacingX - 0.1)/(refinement_cols[col] - 1);
            vr_metadata[col].resolution_y = (f32)(data->def.nodeSpacingY - 0.1)/(refinement_cols[col] - 1);
			vr_metadata[col].sw_corner_x = (f32)(data->def.nodeSpacingX - (vr_metadata[col].dimensions_x - 1)*vr_metadata[col].resolution_x)/2.0f;
			vr_metadata[col].sw_corner_y = (f32)(data->def.nodeSpacingY - (vr_metadata[col].dimensions_y - 1)*vr_metadata[col].resolution_y)/2.0f;

            /* Refinement information */
            total_refinements = refinement_cols[col]*refinement_cols[col];

            generate_vr_data(refinement_cols[col], min_elevation, max_elevation, min_uncertainty, max_uncertainty, vr_refinements);

            if ((errcode = bagWriteRow(handle, 0, n_cells, n_cells + total_refinements - 1, VarRes_Refinement_Group, vr_refinements)) != BAG_SUCCESS) {
                free(refinement_cols); report_library_error("failed writing VR refinements", errcode);
                free(lowres_aux); free(vr_metadata); free(vr_refinements); free(vr_aux);
                return False;
            }

            /* Auxiliary information */
            for (ref = 0; ref < total_refinements; ++ref) {
                vr_aux[ref].hyp_strength = 4.0f*(ref % 10)/10.0f;
                vr_aux[ref].num_hypotheses = 1 + ref % 10;
                vr_aux[ref].n_samples = 1 + ref % 15;
            }

            if ((errcode = bagWriteRow(handle, 0, n_cells, n_cells + total_refinements - 1, VarRes_Node_Group, vr_aux)) != BAG_SUCCESS) {
                report_library_error("failed writing VR node groups", errcode);
                free(refinement_cols); free(lowres_aux); free(vr_metadata); free(vr_refinements); free(vr_aux);
                return False;
            }

            /* Move on in the refinement/node group arrays */
            n_cells += total_refinements;
        }
        if ((errcode = bagWriteRow(handle, row, 0, lowres_cols-1, VarRes_Metadata_Group, vr_metadata)) != BAG_SUCCESS) {
            report_library_error("failed writing VR metadata", errcode);
            free(refinement_cols); free(lowres_aux); free(vr_metadata); free(vr_refinements); free(vr_aux);
            return False;
        }
    }
    bagUpdateOptSurface(handle, VarRes_Metadata_Group);
    bagUpdateOptSurface(handle, VarRes_Refinement_Group);
    bagUpdateOptSurface(handle, VarRes_Node_Group);
    bagUpdateOptSurface(handle, Node_Group);

    free(refinement_cols); free(lowres_aux); free(vr_metadata); free(vr_refinements); free(vr_aux);
    return True;
}

int main(int argc, char **argv)
{
    int     c;
    Bool    generate_help = False;
    
    bagHandle   handle;
    bagData     data;
    
    opterr = 0;
    while ((c = getopt(argc, argv, "h")) != EOF) {
        switch (c) {
            case 'h':
                generate_help = True;
                break;
            case '?':
                fprintf(stderr, "error: unknown option flag '%c'\n", optopt);
                break;
        }
    }
    argc -= optind-1; argv += optind-1;
    if (argc != ARGC_EXPECTED || generate_help) {
        Syntax();
        return 1;
    }
    
    if (!construct_basic_bag(&handle, &data, argv[XML_FILENAME], argv[OUTPUT_BAG])) {
        fprintf(stderr, "error: failed to initialise basic BAG file.\n");
        return 1;
    }
    printf("Basic BAG construction complete (%d, %d) cells, spacing (%.3lf, %.3lf).\n",
           data.def.nrows, data.def.ncols,
           data.def.nodeSpacingX, data.def.nodeSpacingY);
    
    if (!augment_vr_bag(handle, &data)) {
        fprintf(stderr, "error: failed to add variable resolution layers to BAG\n");
        return 1;
    }
    printf("Extended BAG-VR construction complete.\n");
 
    bagFileClose(handle);
    printf("BAG-VR file closed.\n");
    
    return 0;
}