/*! \file bag_vr_read.c
 * \brief Example BAG-VR reader code to dump the contents of a variable-resolution BAG
 *
 * Authors:
 *  2015-08-10  Brian Calder
 *
 * Open Navigation Surface Working Group, 2015.  Visit the project website at
 * http://www.opennavsurf.org
 */

#include <stdio.h>
#include <string.h>

#include "getopt.h"
#include "bag.h"

void report_library_error(char *prelude, bagError errcode)
{
    char *errstr = NULL;
    bagGetErrorString(errcode, (u8**)errstr);
    printf("error: %s (BAG library said: \"%s\").\n", prelude, errstr);
}

Bool write_bag_summary(bagHandle handle, Bool const has_extended_data)
{
    bagData *bag = bagGetDataPointer(handle);

	bagError errcode;
    s32 n_opt_datasets;
    s32 opt_dataset_names[BAG_OPT_SURFACE_LIMIT];
    u32 n;
    bagVarResMetadataGroup min_meta_group, max_meta_group;
    bagVarResRefinementGroup min_ref_group, max_ref_group;

    printf("BAG File summary:\n");
    printf("- Dimensions: %d rows, %d cols.\n", bag->def.nrows, bag->def.ncols);
    printf("- Southwest corner: (%lf, %lf).\n", bag->def.swCornerX, bag->def.swCornerY);
    printf("- Spacings: (%lf, %lf).\n", bag->def.nodeSpacingX, bag->def.nodeSpacingY);
    printf("- Horizontal: \"%s\".\n", bag->def.referenceSystem.horizontalReference);
    printf("- Vertical: \"%s\".\n", bag->def.referenceSystem.verticalReference);
    
    /* Report on the optional datasets that are available within the BAG (may be more
     * than just the VR-related ones).
     */
    errcode = bagGetOptDatasets(&handle, &n_opt_datasets, opt_dataset_names);
    printf("- Optional datasets: %d found.\n", n_opt_datasets);
    if (n_opt_datasets == 0) {
        printf("BAG-VR file claims to have no optional datasets?!\n");
        return False;
    }
    for (n = 0; n < (u32)n_opt_datasets; ++n) {
        printf("-  [%d] ID %d = ", n, opt_dataset_names[n]);
        switch(opt_dataset_names[n]) {
            case Num_Hypotheses:
                printf("Hypothesis counts");
                break;
            case Average:
                printf("Average uncertainty");
                break;
            case Standard_Dev:
                printf("Standard deviation");
                break;
            case Nominal_Elevation:
                printf("Nominal Elevation");
                break;
            case Surface_Correction:
                printf("Surface Correction");
                break;
            case Node_Group:
                printf("Node auxiliary information");
                break;
            case Elevation_Solution_Group:
                printf("CUBE auxiliary depth information");
                break;
            case VarRes_Metadata_Group:
                printf("Variable Resolution Metadata Information");
                break;
            case VarRes_Refinement_Group:
                printf("Variable Resolution Refinements");
                break;
            case VarRes_Node_Group:
                printf("Variable Resolution Node auxiliary information");
                break;
            case VarRes_Tracking_List:
                printf("Variable Resolution Tracking List");
                break;
            default:
                printf("Unknown");
                break;
        }
        printf("\n");
    }
    
    /* Get some information on the VR-related elements.  Note that we don't need
     * to check that these exist, because this is checked in bag_is_varres() before
     * the summary routine is called.
     */
    bagGetOptDatasetInfo(&handle, VarRes_Metadata_Group);
    bagReadMinMaxVarResMetadataGroup(handle, &min_meta_group, &max_meta_group);
    printf("- Finest refinement %.2f m (%d cells).\n", min_meta_group.resolution, max_meta_group.dimensions);
    printf("- Coarsest refinement %.2f m (%d cells).\n", max_meta_group.resolution, min_meta_group.dimensions);
    
    bagGetOptDatasetInfo(&handle, VarRes_Refinement_Group);
    printf("- Total %d refinement cells (not all may be active).\n", bag->opt[VarRes_Refinement_Group].ncols);
    bagReadMinMaxVarResRefinementGroup(handle, &min_ref_group, &max_ref_group);
    printf("- Refined depth range [%.2f, %.2f]m\n", min_ref_group.depth, max_ref_group.depth);
    printf("- Refined uncertainty range [%.2f, %.2f]m (1sd).\n", min_ref_group.depth_uncrt, max_ref_group.depth_uncrt);
    
    if (has_extended_data) {
        bagVarResNodeGroup min_node_group, max_node_group;
        bagGetOptDatasetInfo(&handle, VarRes_Node_Group);
        bagReadMinMaxVarResNodeGroup(handle, &min_node_group, &max_node_group);
        printf("- Refined num. hypo. range [%d, %d].\n", min_node_group.num_hypotheses, max_node_group.num_hypotheses);
        printf("- Refined sample count range [%d, %d].\n", min_node_group.n_samples, max_node_group.n_samples);
        printf("- Refined hyp. strength range [%.3f, %.3f].\n", min_node_group.hyp_strength, max_node_group.hyp_strength);
    }
    
    bagFreeInfoOpt(handle);
    return True;
}

Bool dump_data(bagHandle handle, const char * const filename, Bool has_extended_data)
{
    FILE *f;
    bagError errcode;
	bagData *bag;
    bagVarResMetadataGroup minMetadataGroup, maxMetadataGroup;
    u32 maximum_ref_nodes;
	bagVarResRefinementGroup *estimates;
	u32 n_rows, n_cols;
	bagVarResMetadataGroup *metadata;
    bagVarResNodeGroup *auxinfo = NULL;
	u32 row, col, i;

    if ((f = fopen(filename, "w")) == NULL) {
        printf("error: failed to open \"%s\" for output.\n", filename);
        return False;
    }
    bag = bagGetDataPointer(handle);
    
    bagGetOptDatasetInfo(&handle, VarRes_Metadata_Group);
    bagGetOptDatasetInfo(&handle, VarRes_Refinement_Group);
    if (has_extended_data)
        bagGetOptDatasetInfo(&handle, VarRes_Node_Group);
    
    bagReadMinMaxVarResMetadataGroup(handle, &minMetadataGroup, &maxMetadataGroup);
    
    maximum_ref_nodes = maxMetadataGroup.dimensions * maxMetadataGroup.dimensions;
    estimates = (bagVarResRefinementGroup*)malloc(sizeof(bagVarResRefinementGroup)*maximum_ref_nodes);
    
    n_rows = bag->def.nrows;
	n_cols = bag->def.ncols;
    metadata = (bagVarResMetadataGroup*)malloc(sizeof(bagVarResMetadataGroup)*n_cols);
    
    if (has_extended_data) {
        auxinfo = (bagVarResNodeGroup*)malloc(sizeof(bagVarResNodeGroup)*maximum_ref_nodes);
    }
    
    fprintf(f, "Total %d rows, %d columns, with at most %d refined nodes per cell.\n", n_rows, n_cols, maximum_ref_nodes);
    for (row = 0; row < n_rows; ++row) {
        if ((errcode = bagReadRow(handle, row, 0, n_cols-1, VarRes_Metadata_Group, metadata)) != BAG_SUCCESS) {
            report_library_error("failed reading VR metadata row", errcode);
            fclose(f);
            free(metadata);
            free(estimates);
            if (auxinfo != NULL) free(auxinfo);
            return False;
        }
        for (col = 0; col < n_cols; ++col) {
            u32 n_nodes = metadata[col].dimensions*metadata[col].dimensions;
            u32 start_index = metadata[col].index;
            u32 end_index = start_index + n_nodes - 1;
            
            fprintf(f, "Cell (%d, %d) metadata: width/height %d index base %d resolution %.3f m\n", row, col, metadata[col].dimensions, metadata[col].index, metadata[col].resolution);
            
            if (n_nodes == 0) {
                /* There are no refinements in this cell */
                fprintf(f, "No refinements in cell (%d, %d) - skipping.\n", row, col);
                continue;
            }
            
            fprintf(f, "Refinements in cell (%d, %d), indices [%d, %d]:\n", row, col, start_index, end_index);
            /* Read the basic information, depth & uncertainty */
            if ((errcode = bagReadRow(handle, 0, start_index, end_index, VarRes_Refinement_Group, estimates)) != BAG_SUCCESS) {
                report_library_error("failed reading refined estimates", errcode);
                fclose(f);
                free(metadata);
                free(estimates);
                if (auxinfo != NULL) free(auxinfo);
                return False;
            }
            /* If we have it, read the auxiliary information */
            if (has_extended_data) {
                if ((errcode = bagReadRow(handle, 0, start_index, end_index, VarRes_Node_Group, auxinfo)) != BAG_SUCCESS) {
                    report_library_error("failed reading auxiliary information", errcode);
                    fclose(f);
                    free(metadata);
                    free(estimates);
                    if (auxinfo != NULL) free(auxinfo);
                    return False;
                }
            }
            /* Report some useful information on the refinements */
            for (i = 0; i < n_nodes; ++i) {
                fprintf(f, "%d %d %d", i, i/metadata[col].dimensions, i % metadata[col].dimensions);
                if (estimates[i].depth == BAG_NULL_ELEVATION) {
                    fprintf(f, " - No valid refinement at this node");
                } else {
                    fprintf(f, " %.3f %.3f", estimates[i].depth, estimates[i].depth_uncrt);
                    if (has_extended_data)
                        fprintf(f, " %d %d %.3f", auxinfo[i].n_samples, auxinfo[i].num_hypotheses, auxinfo[i].hyp_strength);
                }
                fprintf(f, "\n");
            }
        }
    }
    free(metadata);
    free(estimates);
    if (auxinfo != NULL) free(auxinfo);
    fclose(f);
    bagFreeInfoOpt(handle);
    
    return True;
}

void Syntax(void)
{
    printf("bag_vr_read [%s] - Read variable resolution BAG files and print summary information.\n", __DATE__);
    printf("Syntax: bag_vr_read [opt] <input>\n");
    printf("Options:\n");
    printf(" -h         Generate this help information\n");
    printf(" -d <file>  Dump data to <file>\n");
}

char *copy_string(const char * const input)
{
    char *rtn = (char*)malloc(strlen(input)+1);
    strcpy(rtn, input);
    return rtn;
}

typedef enum {
    INPUT_FILE = 1,
    ARGC_EXPECTED
} Cmd;

int main(int argc, char **argv)
{
    Bool    generate_help = False;
    char    *input_filename, *dump_file = NULL;
    int     c;
    
    bagHandle   handle;
    bagError    errcode;
    Bool        is_var_res, has_extended_data;
    
    opterr = 0;
    while ((c = getopt(argc, argv, "hd:")) != EOF) {
        switch (c) {
            case 'h':
                generate_help = True;
                break;
            case 'd':
                dump_file = copy_string(optarg);
                break;
            case '?':
                fprintf(stderr, "error: unknown flag: '%c'\n", optopt);
                Syntax();
                return 1;
        }
    }
    argc -= optind-1; argv += optind-1;
    if (argc != ARGC_EXPECTED || generate_help) {
        Syntax();
        return 1;
    }
    
    input_filename = argv[INPUT_FILE];
    
    if ((errcode = bagFileOpen(&handle, BAG_OPEN_READONLY, (u8*)input_filename)) != BAG_SUCCESS) {
        report_library_error("failed to open input file", errcode);
        return 1;
    }
    printf("opened \"%s\" for input.\n", input_filename);
    
    if ((errcode = bagCheckVariableResolution(handle, &is_var_res, &has_extended_data)) != BAG_SUCCESS) {
        report_library_error("failed in checking for variable resolution", errcode);
        return 1;
    }
    
    if (!is_var_res) {
        printf("BAG file doesn't have variable-resolution extensions.\n");
        return 1;
    } else
        printf("BAG file has variable resolution extensions");
    
    if (has_extended_data) {
        printf(" and auxiliary information.\n");
    } else
        printf(".\n");
    
    write_bag_summary(handle, has_extended_data);
    
    if (NULL != dump_file) {
        if (!dump_data(handle, dump_file, has_extended_data)) {
            printf("failed to dump data to output file.\n");
            return 1;
        }
    }
    
    bagFileClose(handle);
    
    return 0;
}
