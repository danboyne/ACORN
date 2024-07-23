#include "global_defs.h"
#include "design_rules_PNG_data.h"
#include "layerColors.h"



//-----------------------------------------------------------------------------
// Name: start_HTML_table_of_contents
// Desc: Open an HTML output file that will contain key output data and 
//       hyperlinks to detailed information for each iteration.
//-----------------------------------------------------------------------------
FILE * start_HTML_table_of_contents(const char *input_filename, const InputValues_t *user_inputs,
                                    const MapInfo_t *mapInfo,  const int DRC_free_threshold, int num_threads)  {

  FILE *fp_TOC;

  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);

  // printf("DEBUG: Entered function 'start_HTML_table_of_contents' with input_filename = '%s'\n", input_filename);

  char *output_filename; // String to hold name of output HTML file
  output_filename = malloc(30 * sizeof(char)); // Allocate memory for file name
  strcpy(output_filename, "routingProgress.html");

  char *temp_input_filename;
  temp_input_filename = malloc(300 * sizeof(char));
  strcpy(temp_input_filename, input_filename);  // Create local copy of input_filename

  // Create a pointer to the base filename. This does not need its own memory allocation;
  // the pointer will point to memory that's already allocated in the variable
  // 'temp_input_filename' (above).
  char *base_input_filename;
  base_input_filename = basename(temp_input_filename);   // Calculate the base filename, removing the directory path

  // printf("DEBUG: The base name of input_filename is '%s'\n", base_input_filename);
  // printf("DEBUG: output_filename is '%s'\n", output_filename);
  fp_TOC = fopen(output_filename, "w");

  setbuf(fp_TOC, NULL); // Don't buffer the file. This allows user to view PNG files when they're written
  fprintf(fp_TOC, "<!DOCTYPE HTML>\n<HTML>\n<HEAD><TITLE>Routing Progress</TITLE>\n");

  fprintf(fp_TOC, " <script type=\"text/javascript\">\n");
  fprintf(fp_TOC, "   function toggleMe(a){\n");
  fprintf(fp_TOC, "     var e=document.getElementById(a);\n");
  fprintf(fp_TOC, "     if(!e)return true;\n");
  fprintf(fp_TOC, "     if(e.style.display==\"none\"){\n");
  fprintf(fp_TOC, "       e.style.display=\"block\"\n");
  fprintf(fp_TOC, "     }\n");
  fprintf(fp_TOC, "     else{\n");
  fprintf(fp_TOC, "       e.style.display=\"none\"\n");
  fprintf(fp_TOC, "     }\n");
  fprintf(fp_TOC, "     return true;\n");
  fprintf(fp_TOC, "   }\n");
  fprintf(fp_TOC, " </script>\n");
  fprintf(fp_TOC, "</HEAD>\n\n<BODY>\n");
  fprintf(fp_TOC, "<H1>Routing Progress</H1>\n");
  // Write time-stamp to top of HTML file:
  fprintf(fp_TOC, "<FONT size=\"2\">Started at %02d:%02d on %02d-%02d-%d using %d threads with Acorn version '%s'</FONT><BR><BR>\n",
                   now->tm_hour, now->tm_min, now->tm_mon+1, now->tm_mday, now->tm_year+1900, num_threads, VERSION);

  fprintf(fp_TOC, "<TABLE><TR>\n");
  fprintf(fp_TOC, "  <TD valign=\"top\">\n");
  // Create links to input file, to pre-routing map, to HTML file showing design
  // rules, and to HTML file showing cost-zones:
  fprintf(fp_TOC, "    <B><U>Pre-routing Information:</U></B>\n");
  fprintf(fp_TOC, "    <UL>\n");
  fprintf(fp_TOC, "      <LI>Input file: <FONT size=\"2\"><A href=\"%s\">%s</A></FONT>\n", base_input_filename, base_input_filename);
  fprintf(fp_TOC, "      <LI><A href=\"preRouting_map.html\">Pre-routing map</A>\n");
  fprintf(fp_TOC, "      <LI><A href=\"designRules.html\">Design rules</A>\n");
  fprintf(fp_TOC, "      <LI><A href=\"costZones.html\">Cost zones</A>\n");
  fprintf(fp_TOC, "    </UL>\n");
  fprintf(fp_TOC, "  </TD>\n");
 
  // Create a blank column to add horizontal spacing:
  fprintf(fp_TOC, "  <TD width=\"200px\">&nbsp;</TD>\n");

  // Write the value of key parameters to the output HTML file:
  fprintf(fp_TOC, "  <TD valign=\"top\">\n");
  fprintf(fp_TOC, "    <FONT size=\"1\" color=\"#B0B0B0\">Key parameters:\n");
  fprintf(fp_TOC, "    <UL>\n");
  fprintf(fp_TOC, "      <LI>maxIterations: %d\n", user_inputs->maxIterations);
  fprintf(fp_TOC, "      <LI>violationFreeThreshold: %d\n", user_inputs->userDRCfreeThreshold);
  fprintf(fp_TOC, "      <LI>DRC_free_threshold: %d\n", DRC_free_threshold);
  fprintf(fp_TOC, "      <LI>baseVertCostMicrons: %6.1f um\n", user_inputs->baseVertCostMicrons);
  fprintf(fp_TOC, "      <LI>baseVertCostCells: %'d cells\n", user_inputs->baseVertCostCells);
  fprintf(fp_TOC, "      <LI>baseVertCost: %'lu\n", user_inputs->baseVertCost);
  fprintf(fp_TOC, "      <LI>preEvaporationIterations: %d\n", user_inputs->preEvaporationIterations);
  fprintf(fp_TOC, "      <LI>runsPerPngMap: %d\n", user_inputs->runsPerPngMap);
  fprintf(fp_TOC, "      <LI>baseCellCost: %'lu\n", user_inputs->baseCellCost);
  fprintf(fp_TOC, "      <LI>baseDiagCost: %'lu\n", user_inputs->baseDiagCost);
  fprintf(fp_TOC, "      <LI>baseKnightCost: %'lu\n", user_inputs->baseKnightCost);
  fprintf(fp_TOC, "    </UL></FONT>\n");
  fprintf(fp_TOC, "  </TD>\n");
  fprintf(fp_TOC, "</TR></TABLE>\n\n");


  // Write the value of key parameters to STDOUT:
  printf("\n\nKey parameters:\n");
  printf("---------------\n");
  printf("  maxIterations = %d\n", user_inputs->maxIterations);
  printf("  userDRCfreeThreshold = %d\n", user_inputs->userDRCfreeThreshold);
  printf("  baseVertCostMicrons = %6.1f um\n", user_inputs->baseVertCostMicrons);
  printf("  baseVertCostCells = %'d cells\n", user_inputs->baseVertCostCells);
  printf("  baseVertCost = %'lu\n", user_inputs->baseVertCost);
  printf("  preEvaporationIterations = %d\n", user_inputs->preEvaporationIterations);
  for (int i = 0; i < user_inputs->numDesignRuleSets; i++)  {
    for (int j = 0; j < user_inputs->numDesignRuleSubsets[i]; j++)  {
      printf("    Design rule set #%d ('%s'), subset #%d (%s)", i, user_inputs->designRuleSetName[i], j, user_inputs->designRules[i][j].subsetName);

      // Check whether this subset is dedicated to pseudo-paths:
      if (user_inputs->designRules[i][j].isPseudoNetSubset)  {
        printf(" for pseudo-paths\n");  // Subset is for pseudo-nets, so indicate this in printed-out comment.
      }
      else  {
        printf("\n");  // Subset is not dedicated to pseudo-nets, so no need to indicate anything special.
      }

      printf("      linePitchCells: %.2f\n", (  user_inputs->designRules[i][j].lineWidthMicrons
                                              + user_inputs->designRules[i][j].lineSpacingMicrons)
                                                / user_inputs->cell_size_um);
      printf("      lineWidthCells: %.2f\n", user_inputs->designRules[i][j].lineWidthMicrons / user_inputs->cell_size_um);
      printf("      spacing[TRACE][TRACE]: %.2f\n", user_inputs->designRules[i][j].spacing[TRACE][TRACE]);
      printf("      radius[VIA_UP]: %.2f\n", user_inputs->designRules[i][j].radius[VIA_UP]);
      printf("      radius[VIA_DOWN]: %.2f\n", user_inputs->designRules[i][j].radius[VIA_DOWN]);
    }  // End of for-block for index 'j'
    printf("\n");
  }  // End of for-block for index 'i'
  printf("  runsPerPngMap = %d\n", user_inputs->runsPerPngMap);
  printf("  baseCellCost = %'lu\n", user_inputs->baseCellCost);
  printf("  baseDiagCost = %'lu\n", user_inputs->baseDiagCost);
  printf("  baseKnightCost = %'lu\n", user_inputs->baseKnightCost);

  // Free memory that was allocated for strings at beginning of this function:
  free(temp_input_filename);  // Free memory allocated for this string variable.
  temp_input_filename = NULL; // As a precaution, set pointer to NULL after free'ing the memory.
  free(output_filename);
  output_filename = NULL; // As a precaution, set pointer to NULL after free'ing the memory

  // Do not 'free' the 'base_input_filename' variable, since its memory was already free'd
  // above when we free'd the 'temp_input_filename' variable. ('base_input_filename' is
  // simply a pointer into the larger variable 'temp_input_filename'):
  //free(base_input_filename);  // Free space used by string variable 'base_input_filename'
  //base_input_filename = NULL; // As a precaution, set pointer to NULL after free'ing the memory

  // Print the 'Iterations' header to the HTML file, below which information will be printed
  // later after each iteration is completed:
  fprintf(fp_TOC, "<B><U>Iterations:</U></B>\n");
  fprintf(fp_TOC, "<UL>\n");

  return fp_TOC;

}  // End of function start_HTML_table_of_contents


//-----------------------------------------------------------------------------
// Name: setRGBA
// Desc: Set the 4 values of RGBA (red-green-blue-alpha) for a pixel.
//       For red, green, and blue, zero equals black, and the maximum
//       value for the bit-depth is the full color. For the alpha value,
//       zero is fully transparent, while a value of (2^bitdepth)-1
//       represents a fully opaque pixel.
//-----------------------------------------------------------------------------
static void setRGBA(png_byte *ptr, int red, int green, int blue, int opacity)  {

  ptr[0] = red;
  ptr[1] = green;
  ptr[2] = blue;
  ptr[3] = opacity;

}  // End of function 'setRGBA'


//-----------------------------------------------------------------------------
// Name: createPathTerminalsMatrix
// Desc: Create a 3-dimensional matrix of bytes with the dimensions of the
//       entire map. The contents of the matrix shows which cells in the map
//       are terminals:
//         START_TERM (= 1)  = start-terminal of a non-pseudo-net
//         END_TERM (= 2)    = end-terminal of a non-pseudo-net
//         PSEUDO_TERM (= 3) = start- or end-terminal of a pseudo-net
//-----------------------------------------------------------------------------
static unsigned char*** createPathTerminalsMatrix(const MapInfo_t *mapInfo, const InputValues_t *user_inputs)  {

  unsigned char*** pathTerminals;

  // Allocate memory for the pathTerminals 3D matrix, and initialize each element to zero:
  pathTerminals = malloc(mapInfo->mapWidth * sizeof(unsigned char **));  // Allocate memory for x-dimension
  for (int x = 0; x < mapInfo->mapWidth; x++)  {
    pathTerminals[x] = malloc(mapInfo->mapHeight * sizeof(unsigned char *));  // Allocate memory for y-dimension
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      pathTerminals[x][y] = malloc(mapInfo->numLayers * sizeof(unsigned char));  // Allocate memory for z-dimension
      for (int z = 0; z < mapInfo->numLayers; z++)  {
        pathTerminals[x][y][z] = 0;  // Initialize array element to zero at (x,y,z)
      }  // End of for-loop for index 'z'
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'x'

  // We now iterate over each path and store the start- and end-terminals in the new pathTerminals[][][]
  // array using the following codes:
  //    START_TERM (= 1)  = start-terminal of a non-pseudo-net
  //    END_TERM (= 2)    = end-terminal of a non-pseudo-net
  //    PSEUDO_TERM (= 3) = start- or end-terminal of a pseudo-net
  for (int pathNum = 0; pathNum < (mapInfo->numPaths + mapInfo->numPseudoPaths); pathNum++)  {
    if (user_inputs->isPseudoNet[pathNum])  {
      // Path is a pseudo-path, so mark the start- and end-terminals with 'PSEUDO_TERM':
      pathTerminals[mapInfo->start_cells[pathNum].X][mapInfo->start_cells[pathNum].Y][mapInfo->start_cells[pathNum].Z] = PSEUDO_TERM;
      pathTerminals[mapInfo->end_cells[pathNum].X][mapInfo->end_cells[pathNum].Y][mapInfo->end_cells[pathNum].Z] = PSEUDO_TERM;
    }  // End of if-block for path being a pseudo-net
    else  {
      // Path is a non-pseudo-path, so mark the start- and end-terminals with START_TERM and END_TERM, respectively:
      pathTerminals[mapInfo->start_cells[pathNum].X][mapInfo->start_cells[pathNum].Y][mapInfo->start_cells[pathNum].Z] = START_TERM;
      pathTerminals[mapInfo->end_cells[pathNum].X][mapInfo->end_cells[pathNum].Y][mapInfo->end_cells[pathNum].Z] = END_TERM;
    }  // End of else-block for path being a non-pseudo-net
  }  // End of for-loop for index 'pathNum'

  return(pathTerminals);
}  // End of function 'createPathTerminalsMatrix'


//-----------------------------------------------------------------------------
// Name: freePathTerminalsMatrix
// Desc: Free the memory of the 3-dimensional matrix of bytes with the
//       dimensions of the entire map, and which flagged the locations of
//       terminals in the map.
//-----------------------------------------------------------------------------
static void freePathTerminalsMatrix(unsigned char*** pathTerminals, const MapInfo_t *mapInfo)  {

  // Free the memory that was allocated for 3-dimensional array 'pathTerminals':
  for (int x = 0; x < mapInfo->mapWidth; x++)  {
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      free(pathTerminals[x][y]);
      pathTerminals[x][y] = NULL;
    }
    free(pathTerminals[x]);
    pathTerminals[x] = NULL;
  }
  free(pathTerminals);
  pathTerminals = NULL;

}  // End of function 'freePathTerminalsMatrix'


//-----------------------------------------------------------------------------
// Name: get_RGBA_values_for_pixel
// Desc: Calculate the red, green, blue, and opacity values for a pixel
//       represented by the values (x, y, z_map) in the routing map, and
//       (equivalently) at the coordinate (x, y, z_PNG) among the PNG
//       maps, where z_map = z_PNG / 2.
//-----------------------------------------------------------------------------
static void get_RGBA_values_for_pixel(int x, int y, int z_PNG, int z_map, int isViaLayer,
                                      CellInfo_t ***cellInfo, const MapInfo_t *mapInfo,
                                      unsigned char ***pathTerminals,
                                      int *red, int *green, int *blue, int *opacity)  {

  // Initialize the values that will be modified by this function:
  *red     = 0;
  *green   = 0;
  *blue    = 0;
  *opacity = 0;

  // Color the cell as semi-transparent black if the cell is unwalkable. A cell
  // is unwalkable if it satisfies the following criteria:
  //   Routing layer: 'forbiddenTraceBarrier' flag is set
  //       *or*
  //   Via layer: At least one of the following 4 flags is set:
  //       1) forbiddenUpViaBarrier on 'z_map'
  //       2) forbiddenDownViaBarrier on 'z_map'
  //       3) forbiddenDownViaBarrier on z_map + 1 (if this layer exists)
  //       4) forbiddenUpViaBarrier on z_map - 1 (if this layer exists)
  if (((isViaLayer) && (cellInfo[x][y][z_map].forbiddenUpViaBarrier))
   || ((! isViaLayer) && (cellInfo[x][y][z_map].forbiddenTraceBarrier)))  {
    *red = 0x00; *green = 0x00; *blue = 0x00; *opacity = 0x80;
  }

  // If the cell is on a routing layer, then color it semi-transparent black
  // if 'forbiddenTraceBarrier' flag is set
  if ((! isViaLayer) && (cellInfo[x][y][z_map].forbiddenTraceBarrier))  {
    *red = 0x00; *green = 0x00; *blue = 0x00; *opacity = 0x80;
  }

  // If the cell is on a via layer, then color it semi-transparent black
  // if the forbiddenUpViaBarrier flag is set on the routing layer beneath,
  // or the forbiddenDownViaBarrier flag is set on the routing layer above (if
  // that routing layer exists):
  else if ((isViaLayer) && ((cellInfo[x][y][z_map].forbiddenUpViaBarrier)
       || ((z_map + 1 < mapInfo->numLayers) && (cellInfo[x][y][z_map+1].forbiddenDownViaBarrier))))  {
    *red = 0x00; *green = 0x00; *blue = 0x00; *opacity = 0x80;
  }

  // If cell is the starting point of a non-psuedo-path, then color it opaque grey:
  else if ((! isViaLayer) && (pathTerminals[x][y][z_map] == START_TERM))  {
    *red = 0x99; *green = 0x99; *blue = 0x66; *opacity = 0xFF;
  }

  // If cell is the ending point of a non-pseudo-path, then color it opaque green:
  else if ((! isViaLayer) && (pathTerminals[x][y][z_map] == END_TERM))  {
    *red = 0x00; *green = 0xFF; *blue = 0x00; *opacity = 0xFF;
  }

  // If cell is the starting or ending point of a pseudo-path, then color it opaque black:
  else if ((! isViaLayer) && (pathTerminals[x][y][z_map] == PSEUDO_TERM))  {
    *red = 0x00; *green = 0x00; *blue = 0x00; *opacity = 0xFF;
  }

  // If cell is in a pin-swappable zone on a routing layer with a path's
  // center-line through it, then color it semi-transparent dark yellow:
  else if ((! isViaLayer) && (cellInfo[x][y][z_map].center_line_flag)
           && (cellInfo[x][y][z_map].swap_zone))  {
    *red = 0xE6; *green = 0xE6; *blue = 0x00; *opacity = 0x80;
  }

  // If cell has been marked as a DRC violation, then color it opaque orange:
  else if (((! isViaLayer) && (cellInfo[x][y][z_map].DRC_flag))
        || ((isViaLayer) && (cellInfo[x][y][z_map].via_above_DRC_flag)))  {

    // If pixel is part of the path's (sparse) center-line, then make the
    // pixel darker by 20%. Otherwise, keep the pixel brightness at the
    // default orange value:
    float brightness = 1.0;
    if ((! isViaLayer) && (cellInfo[x][y][z_map].center_line_flag))  {
      brightness = 0.8;
    }

    *red     =  (int) (0xFF * brightness);
    *green   =  (int) (0x99 * brightness);
    *blue    =  (int) (0x00 * brightness);
    *opacity =         0xFF;
  }

  // If cell is traversed by a single path, then color the cell with a color that's unique
  // to this layer. For via layers, we color the cell if the routing layer below has a via-up
  // AND the layer below has a via-down. This creates a via whose size is the smaller of the
  // two vias:
  else if (   ((! isViaLayer) && (cellInfo[x][y][z_map].routing_layer_metal_fill))
           || ((  isViaLayer) && cellInfo[x][y][z_map].via_above_metal_fill && cellInfo[x][y][z_map+1].via_below_metal_fill))  {

    // If pixel is part of the path's (sparse) center-line, then make the
    // pixel darker by 20%. Otherwise, keep the pixel brightness at the
    // default value specified in the RGBA[] array:
    float brightness = 1.0;
    if (   ((! isViaLayer) && (cellInfo[x][y][z_map].center_line_flag))
        || ((  isViaLayer) && (cellInfo[x][y][z_map].center_viaUp_flag || cellInfo[x][y][z_map+1].center_viaDown_flag)))  {
      brightness = 0.8;
    }

    *red     =  (int) (RGBA[z_PNG*4    ]  *  brightness);
    *green   =  (int) (RGBA[z_PNG*4 + 1]  *  brightness);
    *blue    =  (int) (RGBA[z_PNG*4 + 2]  *  brightness);
    *opacity =         RGBA[z_PNG*4 + 3];

  }

  // If cell is traversed by a single pseudo-path, then color the cell with a
  // color that's unique to this layer, but with an opacity of 20% of normal:

  // If cell is traversed by a single pseudo-path, then color the cell with a color that's unique
  // to this layer. For pseudo-via layers, we color the cell if the routing layer below has a pseudo-via-up
  // AND the layer below has a pseudo-via-down. This creates a via whose size is the smaller of the
  // two vias:
  else if (   ((! isViaLayer) && cellInfo[x][y][z_map].pseudo_routing_layer_metal_fill)
           || ((  isViaLayer) && cellInfo[x][y][z_map].pseudo_via_above_metal_fill && cellInfo[x][y][z_map+1].pseudo_via_below_metal_fill))  {

    float opacity_multiplier = 0.20;  // Opacity is 20% for pseudo-net

    // If pixel is part of the pseudo-path's (sparse) center-line, then make the
    // pixel more opaque and darker. Otherwise, keep the pixel brightness at the
    // default value specified in the RGBA[] array:
    float brightness = 1.0;
    if (   ((! isViaLayer) && (cellInfo[x][y][z_map].center_line_flag))
        || ((  isViaLayer) && (cellInfo[x][y][z_map].center_viaUp_flag))
        || ((  isViaLayer) && (cellInfo[x][y][z_map+1].center_viaDown_flag)))  {
      brightness = 0.2;  // Make centerline darker than rest of pseudo-net
      opacity_multiplier = 0.6;  // Make centerline more opaque than rest of pseudo-net
      // printf("DEBUG: In function 'makePngPathMaps()', location (%d,%d,%d) was marked as the centerline of a pseudo-net. center_line_flag=%d, center_viaUp_flag=%d, center_viaDown_flag=%d\n",
      //        x, y, mapLayer, cellInfo[x][y][mapLayer].center_line_flag, cellInfo[x][y][mapLayer].center_viaUp_flag, cellInfo[x][y][mapLayer+1].center_viaDown_flag);
    }

    *red     =  (int) (RGBA[z_PNG*4    ]  *  brightness);
    *green   =  (int) (RGBA[z_PNG*4 + 1]  *  brightness);
    *blue    =  (int) (RGBA[z_PNG*4 + 2]  *  brightness);
    *opacity =         RGBA[z_PNG*4 + 3]  *  opacity_multiplier;  // Reduced opacity for pseudo-nets
  }

  // If the cell is on a routing layer, then color it semi-transparent light
  // yellow if 'swap_zone' is non-zero (i.e., cell is in a pin-swap zone)
  else if ((! isViaLayer) && (cellInfo[x][y][z_map].swap_zone))  {
    *red = 0xFF; *green = 0xFF; *blue = 0x33; *opacity = 0x80;
  }

  // Cell is traversed by no paths, so make it 100% transparent:
  else  {
    *red = 0x00; *green = 0x00; *blue = 0x00; *opacity = 0x00;
  }

}  // End of function 'get_RGBA_values_for_pixel)


//-----------------------------------------------------------------------------
// Name: getAggregateCongestion
// Desc: Return the aggregate congestion due to all paths of shape-type
//       'shape_type' that traverse the cell 'cellInfo', regardless of
//       the design-rule subset. If 'shape_type' is negative, then the
//       aggregate includes *all* shape-types.
//-----------------------------------------------------------------------------
//
// Define 'SHOW_SELECTED_CONGESTION' and re-compile if you want to only
// display congestion from certain paths:
//
// #define SHOW_SELECTED_CONGESTION 1
#undef SHOW_SELECTED_CONGESTION

static unsigned getAggregateCongestion(CellInfo_t *cellInfo, int shape_type)  {

  unsigned aggregate_congestion = 0;

  #ifdef SHOW_SELECTED_CONGESTION
  //
  // Specify the path number and design-rule subset whose congestion you want
  // displayed in the congestion maps:
  //
  int SHOW_SELECTED_PATH_1 = 0;
  int SHOW_SELECTED_PATH_2 = 1;
  int SHOW_SELECTED_PATH_3 = 4;
  int SHOW_SELECTED_PATH_4 = 5;
  int SHOW_SELECTED_DRsubset = 2;
  #endif

  // Get number of paths that traverse cell at (x,y,z):
  const unsigned int num_paths = cellInfo->numTraversingPaths;

  // printf("DEBUG: In function 'getAggregateCongestion' at with %d traversing paths...\n", num_paths);

  if (num_paths == 0)
    return(0);

  //
  // Number of traversing paths is at least 1, so sum up all the
  // congestion from the paths that traverse this cell:
  //
  if (shape_type < 0 )  {  // 'shape_type' is negative, so include all shape types:
    for (int path_index = 0; path_index < num_paths; path_index++)  {

      #ifdef SHOW_SELECTED_CONGESTION
      if (   (   (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_1)
              || (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_2)
              || (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_3)
              || (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_4))
          && (cellInfo->congestion[path_index].DR_subset == SHOW_SELECTED_DRsubset))  {
        aggregate_congestion += cellInfo->congestion[path_index].pathTraversalsTimes100;
      }
      #else
      aggregate_congestion += cellInfo->congestion[path_index].pathTraversalsTimes100;
      #endif

      // printf("DEBUG: Aggregate congestion is %d after path_index %d.\n", aggregate_congestion, path_index);
    }  // End of for-loop for index 'path_index'
  }  // End of if-block for shape_type < 0

  else  {  // Shape type is >=0, so select only congestion of type 'shape_type'
    for (int path_index = 0; path_index < num_paths; path_index++)  {
      if (shape_type == cellInfo->congestion[path_index].shapeType)  {

        #ifdef SHOW_SELECTED_CONGESTION
        if (   (   (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_1)
                || (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_2)
                || (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_3)
                || (cellInfo->congestion[path_index].pathNum == SHOW_SELECTED_PATH_4))
            && (cellInfo->congestion[path_index].DR_subset == SHOW_SELECTED_DRsubset))  {
          aggregate_congestion += cellInfo->congestion[path_index].pathTraversalsTimes100;
        }
        #else
        aggregate_congestion += cellInfo->congestion[path_index].pathTraversalsTimes100;
        #endif

        // printf("DEBUG: Aggregate congestion is %d after path_index %d.\n", aggregate_congestion, path_index);
      }  // End of if-block for (shape_type == ...)
    }  // End of for-loop for index 'path_index'

  }  // End of else-block (shape_type >= 0)

  return (aggregate_congestion);

}  // End of function 'getAggregateCongestion'


//-----------------------------------------------------------------------------
// Name: makePngPathThumbnail
// Desc: Create a single PNG file that overlays all the routing and via layers
//       into a single image with maximum height or width of 'maxDimension'
//       pixels. Images will retain their original aspect ratio.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_makePngPathThumbnail' and re-compile if you want verbose debugging
// print-statements enabled:
//
// #define DEBUG_makePngPathThumbnail 1
#undef DEBUG_makePngPathThumbnail

int makePngPathThumbnail(int maxDimension, char *thumbnailFileName, const MapInfo_t *mapInfo,
                         const InputValues_t *user_inputs, CellInfo_t ***cellInfo, char *title)  {

  #ifdef DEBUG_makePngPathThumbnail
  // DEBUG code follows:
  printf("\nDEBUG: Entered function makePngPathThumbnail in iteration %d with maxDimension = %d\n\n",
         mapInfo->current_iteration, maxDimension);
  #endif

  int returnCode = 0;  // Return-code to indicate errors from this function
  //
  // Before generating the PNG files, we first generate a 3D array 'pathTerminals' that contains locations
  // of the terminals of each path. This allows the PNG files to display special colors for the start- and
  // end-terminals of each path, including user-defined paths and pseudo-paths.
  //
  unsigned char ***pathTerminals = createPathTerminalsMatrix(mapInfo, user_inputs);

  // Based on the lateral dimensions of the map and the 'maxDimsion' variable, calculate
  // the width and height of the thumbnail in units of pixels.
  int thumbnail_width, thumbnail_height;
  if (mapInfo->mapWidth > mapInfo->mapHeight)  {
    thumbnail_width = maxDimension;
    thumbnail_height = (int)(maxDimension * mapInfo->mapHeight / mapInfo->mapWidth);
  }
  else {
    thumbnail_height = maxDimension;
    thumbnail_width = (int)(maxDimension * mapInfo->mapWidth / mapInfo->mapHeight);
  }
  #ifdef DEBUG_makePngPathThumbnail
  printf("DEBUG: In makePngPathThumbnail, thumbnail_width = %d and thumbnail_height = %d\n", thumbnail_width, thumbnail_height);
  #endif

  // Calculate floating-point magnification factor to convert X/Y coordinates in the map
  // to X/Y coordinates in the thumbnail.
  float map_to_thumbnail_ratio = (float)mapInfo->mapWidth / thumbnail_width;
  #ifdef DEBUG_makePngPathThumbnail
  printf("DEBUG: In makePngPathThumbnail, map_to_thumbnail_ratio = %.3f\n", map_to_thumbnail_ratio);
  #endif

  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep row = NULL;

  // Allocate memory for one row (4 bytes per pixel: red, green, blue, and alpha)
  row = (png_bytep) malloc(4 * thumbnail_width * sizeof(png_byte));

  // Open file for writing (binary mode)
  fp = fopen(thumbnailFileName, "wb");
  if (fp == NULL) {
    fprintf(stderr, "\nERROR: Could not open PNG thumbnail file '%s' for writing.\n\n", thumbnailFileName);
    returnCode = 1;
    goto finalize_thumbnail;
  }

  // Initialize write structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct for thumbnail image.\n\n");
    returnCode = 1;
    goto finalize_thumbnail;
  }

  // Initialize info structure
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct for thumbnail image.\n\n");
    returnCode = 1;
    goto finalize_thumbnail;
  }

  // Setup Exception handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "\nERROR during PNG creation for thumbnail image.\n\n");
    returnCode = 1;
    goto finalize_thumbnail;
  }

  png_init_io(png_ptr, fp);

  // Write header (8 bit colour depth)
  // The 'IHDR' chunk is the first chunk in a PNG file. It contains the image's width,
  // height, color type and bit depth.
  png_set_IHDR(png_ptr, info_ptr, thumbnail_width, thumbnail_height,
               8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  // Set title
  if (title != NULL) {
    png_text title_text;
    title_text.compression = PNG_TEXT_COMPRESSION_NONE;
    title_text.key = "Title";
    title_text.text = title;
    png_set_text(png_ptr, info_ptr, &title_text, 1);
  }

  png_write_info(png_ptr, info_ptr);

  //
  // Write image data
  //
  #ifdef DEBUG_makePngPathThumbnail
  printf("DEBUG: About to raster over all cells in the thumbnail file: %d X %d X %d cells.\n",
         thumbnail_width, thumbnail_height, 2 * mapInfo->numLayers - 1);
  #endif
  for (int Y_thumbnail = thumbnail_height - 1; Y_thumbnail >= 0; Y_thumbnail--) {
    for (int X_thumbnail = 0; X_thumbnail < thumbnail_width; X_thumbnail++) {

      // First, calculate the Red, Green, Blue, and Opacity (alpha) values for
      // each routing and via layer that will be represented by this pixel in
      // the thumbnail image:
      unsigned char red_thumbnail[2 * maxRoutingLayers - 1]      = { 0 };
      unsigned char green_thumbnail[2 * maxRoutingLayers - 1]    = { 0 };
      unsigned char blue_thumbnail[2 * maxRoutingLayers - 1]     = { 0 };
      unsigned char opacity_thumbnail[2 * maxRoutingLayers - 1]  = { 0 };

      //
      // Iterate over the trace and via layers at the current X/Y pixel in the thumbnail image:
      //
      for (int traceViaLayer = 0; traceViaLayer < 2 * mapInfo->numLayers - 1; traceViaLayer++)  {

        #ifdef DEBUG_makePngPathThumbnail
        printf("DEBUG: Analyzing thumbnail pixel (%d,%d,%d) in makePngPathThumbnail...\n", X_thumbnail, Y_thumbnail, traceViaLayer);
        #endif

        int mapLayer   = traceViaLayer / 2;  // mapLayer is the z-value to be used in the cellInfo[x][y][z] matrix
        int isViaLayer = traceViaLayer % 2;  // (traceViaLayer % 2) = TRUE if layer is a via layer (FALSE if routing layer)

        // Average the R, G, and B of the map-cells that will be represented
        // by this pixel on layer 'traceViaLayer':
        int sum_red     = 0;
        int sum_green   = 0;
        int sum_blue    = 0;
        int sum_opacity = 0;

        // Variables for specifying color of each pixel in the main map:
        int red_map, green_map, blue_map, opacity_map;

        // Number of cells in the main map that will be averaged into the thumbnail pixel at (X_thumbnail, Y_thumbnail):
        int map_cell_count = 0;

        #ifdef DEBUG_makePngPathThumbnail
        printf("DEBUG: About to raster over selected cells in the map file:\n");
        printf("DEBUG:    X_map range: %d to %d\n", (int)(X_thumbnail * map_to_thumbnail_ratio), (int)((X_thumbnail + 1) * map_to_thumbnail_ratio));
        printf("DEBUG:    Y_map range: %d to %d\n", (int)(Y_thumbnail * map_to_thumbnail_ratio), (int)((Y_thumbnail + 1) * map_to_thumbnail_ratio));
        #endif

        for (int X_map = (int)(X_thumbnail * map_to_thumbnail_ratio); X_map <= (int)((X_thumbnail + 1) * map_to_thumbnail_ratio); X_map++)  {
          for (int Y_map = (int)(Y_thumbnail * map_to_thumbnail_ratio); Y_map <= (int)((Y_thumbnail + 1) * map_to_thumbnail_ratio); Y_map++)  {

            #ifdef DEBUG_makePngPathThumbnail
            printf("DEBUG: Analyzing map cell (%d,%d,%d) for thumbnail pixel (%d,%d,%d)...\n", X_map, Y_map, mapLayer,
                   X_thumbnail, Y_thumbnail, traceViaLayer);
            #endif

            // Confirm that the calculated coordinate (X_map, Y_map) is indeed within the map:
            if ( XY_coords_are_outside_of_map(X_map, Y_map, mapInfo))  {
              continue; // Continue on to the next (X_map, Y_map) coordinate
            }

            map_cell_count++;  // Increment the counter for the number of map-cells that correspond to the current thumbnail pixel

            // Determine the color and opacity at the (X_map, Y_map, traceViaLayer) coordinate:
            get_RGBA_values_for_pixel(X_map, Y_map, traceViaLayer, mapLayer, isViaLayer, cellInfo, mapInfo, pathTerminals,
                                      &red_map, &green_map, &blue_map, &opacity_map);

            // Sum the values for each color, which we'll use to calculate an average:
            sum_red     += red_map;
            sum_green   += green_map;
            sum_blue    += blue_map;
            sum_opacity += opacity_map;

          }  // End of for-loop for index 'Y_map'
        }  // End of for-loop for index 'X_map'

        // Calculate the average RGBA values for this thumbnail pixel for layer 'traceViaLayer':
        if (map_cell_count)  {
          red_thumbnail[traceViaLayer]     = sum_red     / map_cell_count;
          green_thumbnail[traceViaLayer]   = sum_green   / map_cell_count;
          blue_thumbnail[traceViaLayer]    = sum_blue    / map_cell_count;
          opacity_thumbnail[traceViaLayer] = sum_opacity / map_cell_count;
        }
        else  {
          red_thumbnail[traceViaLayer]     = 0;
          green_thumbnail[traceViaLayer]   = 0;
          blue_thumbnail[traceViaLayer]    = 0;
          opacity_thumbnail[traceViaLayer] = 0;
        }

      }  // End of for-loop for index 'traceViaLayer'

      // Next, calculate the cumulative visibility of each routing and via layer,
      // accounting for the opacity of the layer(s) in front of it. The visibility of the
      // bottom layer is zero, since no layers cover it. The visibility is defined as
      // a value from 0 to 1, with 1 denoting 100% visible, and 0 being completely
      // invisible due to opaque layers above.
      float visibility[2 * maxRoutingLayers - 1]  = { 0.0 };
      visibility[0] = 1.0;  // Bottom layer has 100% visibility
      for (int traceViaLayer = 1; traceViaLayer < 2 * mapInfo->numLayers - 1; traceViaLayer++)  {

        // We calculate the visibility of each layer to be equal to the visibility of the
        // layer below multiplied by the transparency of the layer below. A layer's transparency
        // is simply 1 minus its opacity. (We divide the opacity by 255 to normalize it to unity.)
        visibility[traceViaLayer] = visibility[traceViaLayer - 1] * (1 - opacity_thumbnail[traceViaLayer - 1]/255);

        #ifdef DEBUG_makePngPathThumbnail
        printf("\nDEBUG: For thumbnail pixel (%d,%d), visibility[%d] = %.3f\n", X_thumbnail, Y_thumbnail,
               traceViaLayer, visibility[traceViaLayer]);
        #endif

      }  // End of for-loop for index 'traceViaLayer'

      // We next calculate the RGB of the current thumbnail pixel by taking the average of each
      // layer's color, weighted by that layer's visibility from the top:
      float visibility_sum = 0;  // Sum of visibilities on each layer, for calculating weighted sum
      float red_sum        = 0.0;
      float green_sum      = 0.0;
      float blue_sum       = 0.0;

      for (int traceViaLayer = 0; traceViaLayer < 2 * mapInfo->numLayers - 1; traceViaLayer++)  {
        red_sum        += red_thumbnail[traceViaLayer]      * visibility[traceViaLayer];
        green_sum      += green_thumbnail[traceViaLayer]    * visibility[traceViaLayer];
        blue_sum       += blue_thumbnail[traceViaLayer]     * visibility[traceViaLayer];
        visibility_sum += visibility[traceViaLayer];
      }  // End of for-loop for index 'traceViaLayer'

      // Calculated the weighted average of the colors across all the layers:
      int red_thumbnail_pixel     = red_sum     / visibility_sum;
      int green_thumbnail_pixel   = green_sum   / visibility_sum;
      int blue_thumbnail_pixel    = blue_sum    / visibility_sum;
      int opacity_thumbnail_pixel;
      if (red_thumbnail_pixel || green_thumbnail_pixel || blue_thumbnail_pixel)  {
       opacity_thumbnail_pixel = 255;
      }
      else  {
        opacity_thumbnail_pixel = 0;
      }

      #ifdef DEBUG_makePngPathThumbnail
      printf("\nDEBUG: For thumbnail pixel (%d,%d):\n", X_thumbnail, Y_thumbnail);
      printf(  "DEBUG:            red_sum = %.2f\n", red_sum);
      printf(  "DEBUG:          green_sum = %.2f\n", green_sum);
      printf(  "DEBUG:           blue_sum = %.2f\n", blue_sum);
      printf(  "DEBUG:     visibility_sum = %.2f\n", visibility_sum);
      printf(  "DEBUG:   RGBA = (%d,%d,%d,%d)\n\n", red_thumbnail_pixel, green_thumbnail_pixel, blue_thumbnail_pixel, opacity_thumbnail_pixel);
      #endif

      //
      // Write the RGBA data to the pixel:
      //
      setRGBA(&(row[X_thumbnail * 4]), red_thumbnail_pixel, green_thumbnail_pixel, blue_thumbnail_pixel, opacity_thumbnail_pixel);

    }  // End of for-loop for index 'X_thumbnail'

    // Finalize the row of data:
    png_write_row(png_ptr, row);

  }  // End of for-loop for index 'Y_thumbnail'

  // End write
  png_write_end(png_ptr, NULL);

  finalize_thumbnail:
  if (fp != NULL)  {
    fclose(fp);
    fp = NULL;
  }

  if (png_ptr != NULL) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    png_ptr = NULL;
  }

  if (row != NULL)  {
    free(row);
    row = NULL;
  }

  // Free the memory that was allocated in this function for 3-dimensional
  // array 'pathTerminals':
  freePathTerminalsMatrix(pathTerminals, mapInfo);

  return(returnCode);
}  // End of function 'makePngPathThumbnail'


//-----------------------------------------------------------------------------
// Name: makePngPathMaps
// Desc: Create PNG files for routing and via layers to display the paths of
//       each routed net. Maps will be magnified by zoom factor 'mag'
//       (mag = 1 or larger integer).
//-----------------------------------------------------------------------------
static int makePngPathMaps(int mag, int numPngLayers, char *pngPathFileNames[], const MapInfo_t *mapInfo,
                           const InputValues_t *user_inputs, CellInfo_t ***cellInfo, char *title)  {

  //
  // Before generating the PNG files, we first generate a 3D array 'pathTerminals' that contains locations
  // of the terminals of each path. This allows the PNG files to display special colors for the start- and
  // end-terminals of each path, including user-defined paths and pseudo-paths.
  //
  unsigned char ***pathTerminals = createPathTerminalsMatrix(mapInfo, user_inputs);

  // 'returnCode' array element contains zero for each layer if creating a PNG file was successful:
  int returnCode[numPngLayers];

  // 'aggregateReturnCode' is zero if PNG files for *ALL* layers were created successfully:
  int aggregateReturnCode = 0;

  //
  // Generate a PNG map file for each routing and via layer. We use parallel processing so that
  // each layer can be generated by a different thread.
  //
  // printf("  DEBUG: About to generate PNG map file for routing and via layers...\n");
  #pragma omp parallel for schedule (dynamic, 1)
  for (int pngLayer = 0; pngLayer < numPngLayers; pngLayer++)  {
    // printf("    DEBUG: Generating PNG map file for PNG layer %d.\n", pngLayer);

    returnCode[pngLayer] = 0;  // Initialize returnCode for this layer
    int red, green, blue, opacity;  // Variables for specifying color of each pixel
    int mapLayer   = pngLayer / 2; // mapLayer is the z-value to be used in the cellInfo[x][y][z] matrix
    int isViaLayer = pngLayer % 2; // (pngLayer % 2) = TRUE if layer is a via layer (FALSE if routing layer)

    // printf("    DEBUG: mapLayer is %d. isViaLayer is %d.\n", mapLayer, isViaLayer);

    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row = NULL;

    // printf("DEBUG sizeof(png_byte) is %lu.\n", sizeof(png_byte));
    // printf("DEBUG sizeof(png_bytep) is %lu.\n", sizeof(png_bytep));
    // printf("DEBUG mapInfo->mapWidth is %d.\n", mapInfo->mapWidth);
    // printf("DEBUG pngPathFileNames[pngLayer] is '%s'.\n", pngPathFileNames[pngLayer]);

    // printf("DEBUG: Before malloc, address of 'row' is %p.\n", row);

    // Allocate memory for one row (4 bytes per pixel: red, green, blue, and alpha)
    row = (png_bytep) malloc(4 * mapInfo->mapWidth * mag * sizeof(png_byte));

    // printf("DEBUG: After malloc, address of 'row' is %p.\n", row);

    // Open file for writing (binary mode)
    fp = fopen(pngPathFileNames[pngLayer], "wb");
    if (fp == NULL) {
      fprintf(stderr, "\nERROR: Could not open PNG file '%s' for writing.\n\n", pngPathFileNames[pngLayer]);
      returnCode[pngLayer] = 1;
      goto finalize_paths;
    }
    // printf("DEBUG: Opened PNG file for writing in thread %d: '%s'\n", omp_get_thread_num(), pngPathFileNames[pngLayer]);

    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    // printf("    DEBUG: Completed 'png_create_write_struct' function.\n");
    if (png_ptr == NULL) {
      fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct.\n\n");
      returnCode[pngLayer] = 1;
      goto finalize_paths;
    }
    // printf("    DEBUG: Initialized write structure.\n");

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
      fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct.\n\n");
      returnCode[pngLayer] = 1;
      goto finalize_paths;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr))) {
      fprintf(stderr, "\nERROR during PNG creation.\n\n");
      returnCode[pngLayer] = 1;
      goto finalize_paths;
    }

    // printf("    DEBUG: Completed 'setjmp' for exception-handling.\n");

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    // The 'IHDR' chunk is the first chunk in a PNG file. It contains the image's width,
    // height, color type and bit depth.
    png_set_IHDR(png_ptr, info_ptr, mapInfo->mapWidth*mag, mapInfo->mapHeight*mag,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != NULL) {
      png_text title_text;
      title_text.compression = PNG_TEXT_COMPRESSION_NONE;
      title_text.key = "Title";
      title_text.text = title;
      png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    //
    // Write image data
    //
    for (int y = mapInfo->mapHeight-1 ; y >= 0; y--) {
      for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
        // printf("      DEBUG: Creating pixel at (x,y) = (%d, %d).\n", x, y);


        // Calculate the red, green, blue, and opacity values for a pixel represented by the
        // coordinate (x, y, mapLayer) in the routing map, and (equivalently) at the coordinate
        // (x, y, pngLayer) among the PNG maps, where mapLayer = pngLayer / 2.
        get_RGBA_values_for_pixel(x, y, pngLayer, mapLayer, isViaLayer, cellInfo, mapInfo, pathTerminals,
                                  &red, &green, &blue, &opacity);

        // Account for the magnification factor 'mag' in the x-direction using the 'repeat_x' variable:
        for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {

          //
          // Write data to the pixel:
          //
          setRGBA(&(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);

        }  // end of repeat_x for-loop

      }  // End of x for-loop

      // Account for the magnification factor 'mag' in the y-direction:
      for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
        png_write_row(png_ptr, row);
      }  // End of repeat_y for-loop

    }  // End of y for-loop

    // End write
    png_write_end(png_ptr, NULL);

    finalize_paths:
    if (fp != NULL)  {
      fclose(fp);
      fp = NULL;
    }
    // printf("DEBUG: Closed PNG file in thread %d: '%s'\n", omp_get_thread_num(), pngPathFileNames[pngLayer]);

    if (png_ptr != NULL) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      png_ptr = NULL;
    }

    // printf("DEBUG: Before free, address of 'row' is %p.\n", row);
    if (row != NULL)  {
      free(row);
      row = NULL;
    }
    // printf("DEBUG: After free, address of 'row' is %p.\n", row);

  }  // End of for-loop for variable 'pngLayer'
  //
  // The above line is the end of parallel processing
  //

  // Free the memory that was allocated in this function for 3-dimensional
  // array 'pathTerminals':
  freePathTerminalsMatrix(pathTerminals, mapInfo);

  // Determine if all layers were successfully created:
  for (int pngLayer = 0; pngLayer < numPngLayers; pngLayer++)  {
    if (returnCode[pngLayer]) {
      // At least one layer was not successfully created, so return a non-zero code
      // from this function:
      aggregateReturnCode = 1;
      break;
    }
  }

  return(aggregateReturnCode);

}  // End of function 'makePngPathMaps'


//-----------------------------------------------------------------------------
// Name: makePngCongestionMaps
// Desc: Create PNG files to display the congestion associated with each
//       routed net. Maps will be magnified by zoom factor 'mag'
//       (mag = 1 or larger integer).
//-----------------------------------------------------------------------------
static int makePngCongestionMaps(int mag, const MapInfo_t *mapInfo, char *pngCongestionFileNames[][NUM_SHAPE_TYPES],
                                 CellInfo_t ***cellInfo, char *title)  {

  // 'returnCode' array element contains zero for each layer if creating a PNG file was successful:
  int returnCode[mapInfo->numLayers];

  // 'aggregateReturnCode' is zero if PNG files for *ALL* layers were created successfully:
  int aggregateReturnCode = 0;

  // Determine range of congestion values in 'congestion' matrix:
  unsigned cell_congestion;
  unsigned max_congestion = 0;  // Maximum congestion in entire 'congestion' matrix
  for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
    for (int y = 0 ; y < mapInfo->mapHeight; y++) {
      for (int z = 0 ; z < mapInfo->numLayers; z++) {

        // Get aggregate congestion at (x,y,z), including all shape-types:
        cell_congestion = getAggregateCongestion(&(cellInfo[x][y][z]), -1);

        if (cell_congestion > max_congestion)
          max_congestion = cell_congestion;

      }  // End of for-loop for index 'z'
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'x'

  printf("\nDEBUG: max_congestion in all of map is %'d\n\n", max_congestion);

  // If the 'max_congestion' in the map is zero (i.e., no congestion was added), then
  // re-define max_congestion to '1' to avoid divide-by-zero errors:
  if (max_congestion == 0)  {
    max_congestion = 1;
  }

  //
  // Generate a PNG map file for congestion on each routing layer (not for via layers):
  //
  #pragma omp parallel for schedule (dynamic, 1)
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    int pngLayer = layer * 2;  // Routing layer in PNG space is twice the layer # in pathMap space
    // printf("    DEBUG: Generating PNG map files for congestion on PNG layer %d (%s).\n",
    //         pngLayer, routingLayerNames[layer]);

    returnCode[layer] = 0;  // Initialize returnCode for this layer
    int red, green, blue, opacity; // Variables for specifying color of each pixel
    unsigned char cellValue;  // Value of cell (0 to 255) at location (x,y,layer)

    //
    // For each routing layer, generate N congestion PNG maps, with N = NUM_SHAPE_TYPES.
    // (for TRACEs, upward-going vias, and downward-going vias).
    //
    for (int shape_type = 0; shape_type < NUM_SHAPE_TYPES; shape_type++)  {
      // printf("      DEBUG: Generating PNG map file for congestion on PNG layer %d, shape-type %d.\n",
      //          pngLayer, shape_type);

      FILE *fp;
      png_structp png_ptr;
      png_infop info_ptr;
      png_bytep row = NULL;

      // printf("DEBUG sizeof(png_byte) is %lu.\n", sizeof(png_byte));
      // printf("DEBUG sizeof(png_bytep) is %lu.\n", sizeof(png_bytep));
      // printf("DEBUG mapInfo->mapWidth is %d.\n", mapInfo->mapWidth);
      // printf("DEBUG pngCongestionFileNames[pngLayer][shape_type] is '%s'.\n", pngCongestionFileNames[pngLayer][shape_type]);

      // printf("DEBUG: Before malloc, address of 'row' is %p.\n", row);

      // Allocate memory for one row (4 bytes per pixel: red, green, blue, and alpha)
      row = (png_bytep) malloc(4 * mapInfo->mapWidth * mag * sizeof(png_byte));

      // printf("DEBUG: After malloc, address of 'row' is %p.\n", row);

      // Open file for writing (binary mode)
      fp = fopen(pngCongestionFileNames[pngLayer][shape_type], "wb");
      if (fp == NULL) {
        fprintf(stderr, "\nERROR: Could not open PNG file '%s' for writing\n\n", pngCongestionFileNames[pngLayer][shape_type]);
        returnCode[layer] = 1;
        goto finalize_congestion;
      }

      // printf("DEBUG: Opened PNG file for writing in thread %d: '%s'\n", omp_get_thread_num(), pngCongestionFileNames[pngLayer][shape_type]);


      // Initialize write structure
      png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      if (png_ptr == NULL) {
        fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct.\n\n");
        returnCode[layer] = 1;
        goto finalize_congestion;
      }

      // printf("    DEBUG: Initialized write structure.\n");

      // Initialize info structure
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL) {
        fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct.\n\n");
        returnCode[layer] = 1;
        goto finalize_congestion;
      }

      // Setup Exception handling
      if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "\nERROR during PNG creation.\n\n");
        returnCode[layer] = 1;
        goto finalize_congestion;
      }

      // printf("    DEBUG: Completed 'setjmp' for exception-handling.\n");

      png_init_io(png_ptr, fp);

      // Write header (8 bit colour depth)
      // The 'IHDR' chunk is the first chunk in a PNG file. It contains the image's width,
      // height, color type and bit depth.
      png_set_IHDR(png_ptr, info_ptr, mapInfo->mapWidth*mag, mapInfo->mapHeight*mag,
                   8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

      // Set title
      if (title != NULL) {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
      }

      png_write_info(png_ptr, info_ptr);

      // Write image data
      unsigned congestion_value;
      for (int y = mapInfo->mapHeight-1 ; y >= 0; y--) {
        for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
          // printf("      DEBUG: Creating pixel at (x,y) = (%d, %d).\n", x, y);

          congestion_value = getAggregateCongestion(&(cellInfo[x][y][layer]), shape_type);
          // printf("        congestion_value at (%d, %d) is %d for shape-type %d.\n", x, y,
          //                 congestion_value, shape_type);
          cellValue = (255 * congestion_value) / max_congestion; // Scale congestion by max_congestion.
                                                                 // Max congestion now equals 255.
                                                                 // Min congestion now equals 0.
          cellValue = 255 - cellValue; //  Max congestion now equals 0.
                                       //  Min congestion now equals 255.

          red = green = blue = cellValue; // All colors weighted equally --> shades of gray
          if (congestion_value)
            opacity = 0x80;  // Semi-opaque if non-zero congestion
          else
            opacity = 0x00;  // 100% transparent if no congestion

          for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {
            setRGBA(&(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);

          }  // end of repeat_x for-loop

        }  // End of x for-loop

        for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
          png_write_row(png_ptr, row);
        }  // End of repeat_y for-loop

      }  // End of y for-loop

      // End write
      png_write_end(png_ptr, NULL);

      finalize_congestion:
      if (fp != NULL)  {
        fclose(fp);
        fp = NULL;
      }
      // printf("DEBUG: Closed PNG file in thread %d: '%s'\n", omp_get_thread_num(), pngCongestionFileNames[pngLayer][shape_type]);


      if (png_ptr != NULL)  {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        png_ptr = NULL;
      }

      // printf("DEBUG: Before free, address of 'row' is %p.\n", row);
      if (row != NULL)  {
        free(row);
        row = NULL;
      }
      // printf("DEBUG: After free, address of 'row' is %p.\n", row);

    }  // End of for-loop for index 'shape-type'

  }  // End of for-loop for index 'layer'
  // The above line is the end of parallel processing

  // Determine if all layers were successfully created:
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    if (returnCode[layer]) {
      // At least one layer was not successfully created, so return a non-zero code
      // from this function:
      aggregateReturnCode = 1;
      break;
    }
  }

  return(aggregateReturnCode);

}  // End of function 'makePngCongestionMaps'


//-----------------------------------------------------------------------------
// Name: makePngExplorationMaps
// Desc: Create PNG files to display the areas explored by the A* or post-
//       processing path-finding algorithms. If 'post_processed' is FALSE,
//       a cell is highlighted if the 'cellInfo[x][y][z].explored' flag is
//       set, which occurs if any path explored that cell during normal
//       A* routing. If 'post_processed' is TRUE, a cell is highlighted if
//       the 'cellInfo[x][y][z].explored_PP' flag is set, which occurs if
//       any path explored that cell during (diff-pair) post-processing.
//
//       Maps will be magnified by zoom factor 'mag' (mag = 1 or larger integer).
//
//       After using the value of 'cellInfo[x][y][z].explored' or 'explored_PP'
//       for creating the PNG file, this function resets the value to FALSE
//       (zero) for the next iteration.
//-----------------------------------------------------------------------------
static int makePngExplorationMaps(int mag, const MapInfo_t *mapInfo, char *pngExplorationFileNames[],
                                  int post_processed, CellInfo_t ***cellInfo, char *title)  {

  // printf("DEBUG: Entered makePngExplorationMaps with mag = %d, post_processed = %d, pngExplorationFileNames[0] = '%s'\n",
  //        mag, post_processed, pngExplorationFileNames[0]);

  // 'returnCode' array element contains zero for each layer if creating a PNG file was successful:
  int returnCode[mapInfo->numLayers];

  // 'aggregateReturnCode' is zero if PNG files for *ALL* layers were created successfully:
  int aggregateReturnCode = 0;

  //
  // Generate a PNG map file for exploration on each routing layer (not for via layers):
  //
  #pragma omp parallel for schedule (dynamic, 1)
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {

    returnCode[layer] = 0;  // Initialize returnCode for this layer
    int red, green, blue, opacity; // Variables for specifying color of each pixel
    int pngLayer = layer * 2;  // Routing layer in PNG space is twice the layer # in pathMap space
    // printf("    DEBUG: Generating PNG map files for explored cells on PNG layer %d ('%s').\n",
    //         pngLayer, pngExplorationFileNames[pngLayer]);

    //
    // For each routing layer, generate one exploration PNG map.
    //

    FILE *fp;
    png_structp png_ptr;
    png_infop   info_ptr;
    png_bytep   row = NULL;

    // printf("DEBUG sizeof(png_byte) is %lu.\n", sizeof(png_byte));
    // printf("DEBUG sizeof(png_bytep) is %lu.\n", sizeof(png_bytep));
    // printf("DEBUG mapInfo->mapWidth is %d.\n", mapInfo->mapWidth);
    // printf("DEBUG pngExplorationFileNames[pngLayer] is '%s'.\n", pngExplorationFileNames[pngLayer]);

    // printf("DEBUG: Before malloc, address of 'row' is %p.\n", row);

    // Allocate memory for one row (4 bytes per pixel: red, green, blue, and alpha)
    row = (png_bytep) malloc(4 * mapInfo->mapWidth * mag * sizeof(png_byte));

    // printf("DEBUG: After malloc, address of 'row' is %p.\n", row);

    // Open file for writing (binary mode)
    fp = fopen(pngExplorationFileNames[pngLayer], "wb");
    if (fp == NULL) {
      fprintf(stderr, "\nERROR: Could not open PNG file '%s' for writing\n\n", pngExplorationFileNames[pngLayer]);
      returnCode[layer] = 1;
      goto finalize_exploration;
    }

    // printf("DEBUG: Opened PNG file for writing in thread %d: '%s'\n", omp_get_thread_num(), pngExplorationFileNames[pngLayer]);

    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
      fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct.\n\n");
      returnCode[layer] = 1;
      goto finalize_exploration;
    }

    // printf("    DEBUG: Initialized write structure.\n");

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
      fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct.\n\n");
      returnCode[layer] = 1;
      goto finalize_exploration;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr))) {
      fprintf(stderr, "\nERROR during PNG creation.\n\n");
      returnCode[layer] = 1;
      goto finalize_exploration;
    }

    // printf("    DEBUG: Completed 'setjmp' for exception-handling.\n");

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    // The 'IHDR' chunk is the first chunk in a PNG file. It contains the image's width,
    // height, color type and bit depth.
    png_set_IHDR(png_ptr, info_ptr, mapInfo->mapWidth*mag, mapInfo->mapHeight*mag,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != NULL) {
      png_text title_text;
      title_text.compression = PNG_TEXT_COMPRESSION_NONE;
      title_text.key = "Title";
      title_text.text = title;
      png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    if (! post_processed) {

      // Set red/green bytes to 255, consistent with bright yellow:
      red   = 255;
      green = 255;

      // Write image data for 'normal' explored cells:
      for (int y = mapInfo->mapHeight-1 ; y >= 0; y--) {
        for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
          // printf("      DEBUG: Creating pixel at (x,y) = (%d, %d).\n", x, y);

          // Check if 'explored' bit is set in the cellInfo matrix:
          if (cellInfo[x][y][layer].explored)  {
            opacity = 0x80;  // Semi-opaque yellow if cell was explored
            blue = 0;
          }
          else  {
            opacity = 0x00;  // 100% transparent white if cell was not explored
            blue = 255;
          }  // End of if/else-block for 'explored' == TRUE

          // Reset the value of 'cellInfo[x][y][layer].explored' to FALSE (zero) so that
          // it can be used for the next iteration:
          cellInfo[x][y][layer].explored = FALSE;

          for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {
            setRGBA(&(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
          }  // end of repeat_x for-loop

        }  // End of x for-loop

        for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
          png_write_row(png_ptr, row);
        }  // End of repeat_y for-loop

      }  // End of y for-loop
    }  // End of if-block for post_processed == FALSE

    else {
      // We got here, so 'post_processed' = TRUE

      // Set color to darker yellow:
      red   = 230;
      green = 230;
      blue  =   0;

      // Write image data for post-processed explored cells:
      for (int y = mapInfo->mapHeight-1 ; y >= 0; y--) {
        for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
          // printf("      DEBUG: Creating pixel at (x,y) = (%d, %d).\n", x, y);

          // Check if 'explored_PP' bit is set in the cellInfo matrix:
          if (cellInfo[x][y][layer].explored_PP)  {
            opacity = 0x80;  // Semi-opaque yellow if cell was explored
            red     = 230;
            green   = 230;
            blue    = 0;
          }
          else  {
            opacity = 0x00;  // 100% transparent white if cell was not explored
            red     = 255;
            green   = 255;
            blue    = 255;
          }  // End of if/else-block for 'explored' == TRUE

          // Reset the value of 'cellInfo[x][y][layer].explored_PP' to FALSE (zero) so that
          // it can be used for the next iteration:
          cellInfo[x][y][layer].explored_PP = FALSE;

          for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {
            setRGBA(&(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
          }  // end of repeat_x for-loop

        }  // End of x for-loop

        for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
          png_write_row(png_ptr, row);
        }  // End of repeat_y for-loop

      }  // End of y for-loop

    }  // end of else-block (post_processed = TRUE)

    // End write
    png_write_end(png_ptr, NULL);

    finalize_exploration:
    if (fp != NULL)  {
      fclose(fp);
      fp = NULL;
    }
    // printf("DEBUG: Closed PNG file in thread %d: '%s'\n", omp_get_thread_num(), pngExplorationFileNames[pngLayer]);

    if (png_ptr != NULL)  {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      png_ptr = NULL;
    }

    // printf("DEBUG: Before free, address of 'row' is %p.\n", row);
    if (row != NULL)  {
      free(row);
      row = NULL;
    }
    // printf("DEBUG: After free, address of 'row' is %p.\n", row);

  }  // End of for-loop for index 'layer'
  // The above line is the end of parallel processing

  // Determine if all layers were successfully created:
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    if (returnCode[layer]) {
      // At least one layer was not successfully created, so return a non-zero code
      // from this function:
      aggregateReturnCode = 1;
      break;
    }
  }

  return(aggregateReturnCode);

}  // End of function 'makePngExplorationMaps'


//-----------------------------------------------------------------------------
// Name: makeHtmlIterationSummary
// Desc: Create an HTML file to display the various PNG image files showing
//       the paths, the design-rule zones, the cost-multiplier zones, and
//       statistics for the iteration. Maps will be magnified by zoom
//       factor 'mag' (mag = 1 or larger integer).
//
//       If parameter 'iteration' equals zero, then this HTML report will not
//       display or report any information related to routing. Instead, it
//       reports information only about the static map.
//-----------------------------------------------------------------------------
int makeHtmlIterationSummary(int iteration, const MapInfo_t *mapInfo,
                   CellInfo_t ***cellInfo,  const InputValues_t *user_inputs,
                   const RoutingMetrics_t *routability, char *title,
                   const DRC_details_t DRC_details[maxRecordedDRCs],
                   char *shapeTypeNames[NUM_SHAPE_TYPES])  {

  // printf("DEBUG: Entered function 'makeHtmlIterationSummary' with iteration = %d...\n", iteration);
  int returnCode = 0;

  // Adjust magnification of map so it takes up most of the width of a monitor:
  int mag = 1; // Default (and minimum) magnification factor for PNG files
  if (mag * mapInfo->mapWidth < 1000)
    mag = 1000 / mapInfo->mapWidth;

  int numPngLayers = 2 * mapInfo->numLayers - 1;
  int isViaLayer; // TRUE if an odd (via) layer; FALSE if even (routing) layer.

  //
  // Generate names of each PNG file name:
  //
  char *pngPathFileNames[numPngLayers];
  char *pngCongestionFileNames[numPngLayers][NUM_SHAPE_TYPES];
  char *pngExplorationFileNames[numPngLayers];
  char *pngPPExplorationFileNames[numPngLayers];
  for (int layer = 0; layer < numPngLayers; layer++) {

    // Generate filenames for the routing layers:
    pngPathFileNames[layer] = malloc(300 * sizeof(char));
    sprintf(pngPathFileNames[layer], "map_iter%04d_%02d_%s.png",
            iteration, layer, (*user_inputs).layer_names[layer]);

    // If this iteration is for a routed iteration, then also generate names for
    // PNG files that contain the explored cells and the congestion:
    if (iteration > 0)  {
      // Generate filenames for the A* exploration layers:
      pngExplorationFileNames[layer] = malloc(300 * sizeof(char));
      sprintf(pngExplorationFileNames[layer], "expl_iter%04d_%02d_%s.png",
              iteration, layer, (*user_inputs).layer_names[layer]);

      // Generate filenames for the post-processing exploration layers:
      pngPPExplorationFileNames[layer] = malloc(300 * sizeof(char));
      sprintf(pngPPExplorationFileNames[layer], "explPP_iter%04d_%02d_%s.png",
              iteration, layer, (*user_inputs).layer_names[layer]);

      // Generate filenames for the congestion maps. There are 3 maps for each
      // routing layer: 1 for TRACEs, 1 for upward-going vias, and 1 for downward-
      // going vias.
      for (int shape_type = 0; shape_type < NUM_SHAPE_TYPES; shape_type++)  {
        pngCongestionFileNames[layer][shape_type] = malloc(300 * sizeof(char));
        sprintf(pngCongestionFileNames[layer][shape_type], "cong_iter%04d_%02d-%1d_%s.png",
                iteration, layer, shape_type, (*user_inputs).layer_names[layer]);
      }  // End of for-loop for index 'shape_type'

    }  // End of if-block for iteration > 0

  }  // End of 'layer' for-loop


  // 
  // Open HTML output file that will contain hyperlinks to the PNG file(s):
  //

  char htmlFileName[40]; // Name of HTML file name
  if (iteration > 0)  {
    sprintf(htmlFileName, "iteration%04d.html", iteration); // Example: 'iteration0009.html'
  }
  else  {
    sprintf(htmlFileName, "preRouting_map.html");
  }  // End of if/else-blocks for creating HTML file name

  FILE *fp_html;
  fp_html = fopen(htmlFileName, "w");
  if (fp_html == NULL) {
    fprintf(stderr, "\nERROR: Could not open HTML file %s for writing\n\n", htmlFileName);
    exit(1);
  }

  // 
  // Generate HTML file that displays the overlaid PNG files:
  //
  if (iteration > 0)  {
    fprintf(fp_html, "<HTML>\n<HEAD><TITLE>Iteration %d</TITLE>\n", iteration);
  }
  else  {
    fprintf(fp_html, "<HTML>\n<HEAD><TITLE>Pre-routing Map</TITLE>\n");
  }  // End of if/else-block for iteration > 0
    
  fprintf(fp_html, "<SCRIPT language=\"javascript\" type=\"text/javascript\">\n\n");

  // Write Javascript function to make all layers visible or invisible with a single checkbox:
  fprintf(fp_html, "  function checkAll(x) {\n");
  fprintf(fp_html, "    if (x.checked == true) {\n");
  for (int layer = 0; layer < numPngLayers; layer++)  {
    fprintf(fp_html, "      document.getElementById('layer_%02d').style.visibility='visible';\n", layer);
    fprintf(fp_html, "      document.getElementById('checkbox_%02d').checked=true;\n", layer);
  }  // End of for-loop for index 'layer'
  fprintf(fp_html, "    } else {\n");
  for (int layer = 0; layer < numPngLayers; layer++)  {
    fprintf(fp_html, "      document.getElementById('layer_%02d').style.visibility='hidden';\n", layer);
    fprintf(fp_html, "      document.getElementById('checkbox_%02d').checked=false;\n", layer);
  }  // End of for-loop for index 'layer'
  fprintf(fp_html, "    }\n");
  fprintf(fp_html, "  }\n");

  fprintf(fp_html, "</SCRIPT>\n</HEAD>\n\n");
  if (iteration > 0)  {
    fprintf(fp_html, "<BODY>\n<H1>Iteration %d</H1>\n", iteration);
  }
  else  {
    fprintf(fp_html, "<BODY>\n<H1>Pre-routing Map</H1>\n");
  }

  // Print some general information at the top of the HTML page:
  if ((*user_inputs).num_routing_layers == 1)  {
    fprintf(fp_html, "Map is %6.3f mm wide by %6.3f mm high (%d layer). \n",
                     (*user_inputs).map_width_mm, (*user_inputs).map_height_mm,
                     (*user_inputs).num_routing_layers);
  }
  else  {
    fprintf(fp_html, "Map is %6.3f mm wide by %6.3f mm high (%d layers). \n",
                     (*user_inputs).map_width_mm, (*user_inputs).map_height_mm,
                     (*user_inputs).num_routing_layers);
  }
  fprintf(fp_html, "Each path-finding cell is %dx%d pixels (%.3f x %.3f microns).<BR>\n\n",
                   mag, mag, (*user_inputs).cell_size_um, (*user_inputs).cell_size_um);

  //   
  // Print HTML table to toggle visibility of each image layer.
  //
  // Start with table headers:
  fprintf(fp_html, "<!-- Hyperlinks to toggle the visibility of each image go here: -->\n");
  fprintf(fp_html, "<TABLE border=\"1\" cellpadding=\"2\">\n");
  fprintf(fp_html, "  <TR>\n    <TH rowspan=\"2\">Layer</TH>\n");
  fprintf(fp_html, "    <TH align=\"center\"><FONT size=\"1\"><B>Visibility</B></FONT></TH>\n");
  fprintf(fp_html, "    <TH rowspan=\"2\" align=\"center\"><SPAN STYLE=\"writing-mode: vertical-lr; writing-mode: tb-rl; transform: rotate(180deg);\"><FONT size=\"1\">%%&nbsp;&nbsp;DRCs</FONT></SPAN></TH>\n");
  fprintf(fp_html, "    <TH colspan=\"%d\" align=\"center\"><A href=\"designRules.html\">Design Rules</A></TH>\n",
                         user_inputs->numDesignRuleSets);

  // Print out headers that say 'Trace Cost Multipliers' and 'Via Cost Multipliers':
  if (user_inputs->numTraceMultipliersUsed)  {
    fprintf(fp_html, "    <TH colspan=\"%d\">Trace Cost<BR>Multipliers</TH>\n", user_inputs->numTraceMultipliersUsed);
  }  // End of if-block for numTraceMultipliersUsed > 0
  if (user_inputs->numViaMultipliersUsed)  {
    fprintf(fp_html, "    <TH colspan=\"%d\">Via Cost<BR>Multipliers</TH>\n", user_inputs->numViaMultipliersUsed);
  }  // End of if-block for numViaMultipliersUsed > 0

  // Print out table headers for 'Congestion' and 'Explored Cells' if this report is for a routed iteration:
  if (iteration > 0)  {
    fprintf(fp_html, "    <TH colspan=\"%d\" align=\"center\"><FONT color=\"grey\">Congestion</FONT></TH>\n", NUM_SHAPE_TYPES);

    fprintf(fp_html, "    <TH colspan=\"2\" align=\"center\"><FONT color=\"grey\">Explored Cells</FONT></TH>\n");
  }  // End of if-block for iteration > 0
  fprintf(fp_html, "  </TR>\n");  // End of first row of header-rows

  // Print out a header cell containing a checkbox to toggle visibility of *all* layers:
  fprintf(fp_html, "  <TR>\n");
  fprintf(fp_html, "    <TH><input type=\"checkbox\" name=\"check_uncheck_all\" onchange='checkAll(this);'\n");
  fprintf(fp_html, "       value=\"false\" id=\"id_check_uncheck_all\" style=\"indeterminate:true\"></TH>\n");

  // Print out the headers that list the names of design-rule sets:
  for (int DR_num = 0; DR_num < user_inputs->numDesignRuleSets; DR_num++)  {
    fprintf(fp_html, "    <TH align=\"center\"><FONT size=\"2\">%s</FONT></TH>\n", user_inputs->designRuleSetName[DR_num]);
  }

  // Print out header cells that list the used trace and via cost multipliers, in addition
  // to the associated indices for each multiplier:
  for (int i = 0; i < maxTraceCostMultipliers; i++)  {
    if (user_inputs->traceCostMultiplierUsed[i])  {
      if (i == 0)
        fprintf(fp_html, "    <TD align=\"center\"><B>%dx</B><FONT size=\"1\"><BR>#%d (default)</FONT></TD>\n",
                  user_inputs->traceCostMultiplier[i], i);
      else
        fprintf(fp_html, "    <TD align=\"center\"><B>%dx</B><FONT size=\"1\"><BR>#%d</FONT></TD>\n",
                  user_inputs->traceCostMultiplier[i], i);
    }
  }
  for (int i = 0; i < maxViaCostMultipliers; i++)  {
    if (user_inputs->viaCostMultiplierUsed[i])  {
      if (i == 0)
        fprintf(fp_html, "    <TD align=\"center\"><B>%dx</B><FONT size=\"1\"><BR>#%d (default)</FONT></TD>\n",
                  user_inputs->viaCostMultiplier[i], i);
      else
        fprintf(fp_html, "    <TD align=\"center\"><B>%dx</B><FONT size=\"1\"><BR>#%d</FONT></TD>\n",
                  user_inputs->viaCostMultiplier[i], i);
    }
  }

  // If this HTML report is for a routed iteration, then print out header cells with the names of the shape-types:
  if (iteration > 0)  {
    fprintf(fp_html, "    <TH align=\"center\"><FONT size=\"1\" color=\"grey\">Trace</FONT></TH>\n");
    fprintf(fp_html, "    <TH align=\"center\"><FONT size=\"1\" color=\"grey\">Via-Up</FONT></TH>\n");
    fprintf(fp_html, "    <TH align=\"center\"><FONT size=\"1\" color=\"grey\">Via-<BR>Down</FONT></TH>\n");
  }

  // If this HTML report is for a routed iteration, then print out 2 header cells for explored cells:
  if (iteration > 0)  {
    fprintf(fp_html, "    <TH align=\"center\"><FONT size=\"2\" color=\"grey\">Normal</FONT></TH>\n");
    fprintf(fp_html, "    <TH align=\"center\"><FONT size=\"1\" color=\"grey\">Post-<BR>Processing</FONT></TH>\n");
  }
  fprintf(fp_html, "  </TR>\n");  // End of second row of header-rows

  //
  // In the body of the HTML table, print out 1 row for each routing and via layer:
  //
  for (int layer = 0; layer < numPngLayers; layer++)  {

    isViaLayer = layer % 2; // TRUE if an odd (via) layer; FALSE if even (routing) layer.
 
    // Print out layer name:
    fprintf(fp_html, "  <TR>\n    <TD align=\"center\"><B>%s</B></TD>\n", (*user_inputs).layer_names[layer]);

    // Print out a checkbox to toggle visibility of routing or via layer. The background color for
    // each checkbox is the color of the routing/via layer:
    fprintf(fp_html, "    <TD style=\"background-color:rgba(%d,%d,%d,%3.2f)\" align=\"center\">&nbsp\n",
            RGBA[layer*4], RGBA[layer*4+1], RGBA[layer*4+2], RGBA[layer*4+3]/255.0);
    fprintf(fp_html, "      <input type=\"checkbox\" id=\"checkbox_%02d\"\n", layer);
    fprintf(fp_html, "        onclick=\"document.getElementById('layer_%02d').style.visibility=(this.checked)?'visible':'hidden';\n", layer);
    if (isViaLayer)
      fprintf(fp_html, "                 document.getElementById('id_check_uncheck_all').indeterminate=true;\">\n");
    else
      fprintf(fp_html, "                 document.getElementById('id_check_uncheck_all').indeterminate=true;\" checked>\n");
    fprintf(fp_html, "        &nbsp;\n");
    fprintf(fp_html, "    </TD>\n");

    // Print out the percentage of DRCs that are on the current layer:
    if (isViaLayer)  {
      fprintf(fp_html, "    <TD></TD>\n");  // No content for via layers
    }
    else  {
      if (routability->layer_DRC_cells[layer/2])  {
        // We got here, so this cell is for a routing-layer that contains DRCs.
        // Print out the percentage of DRCs that this layer contains:
        fprintf(fp_html, "    <TD bgcolor=\"grey\" align=\"center\"><FONT color=\"white\" size=\"1\"><SPAN STYLE=\"writing-mode: vertical-lr; writing-mode: tb-rl; transform: rotate(180deg);\"><B>%d</B></SPAN></FONT></TD>\n",
                (int)round(100 * routability->layer_DRC_cells[layer/2] / (float)routability->num_nonPseudo_DRC_cells));
      }
      else  {
        fprintf(fp_html, "    <TD></TD>\n");  // No content if routing layer contains on DRCs
      }
    }  // End of if/else-block for isViaLayer = TRUE/FALSE

    // Print hyperlinks for design-rule zones and for congestion only if layer is a routing layer (even-numbered layer):
    if (! isViaLayer)  {

      // Toggling of design-rule zones:
      char background_color[10];
      for (int DR_num = 0; DR_num < user_inputs->numDesignRuleSets; DR_num++)  {
        if (user_inputs->usedOnLayers[DR_num][layer / 2] == 0)
          // Design-rule set 'DR_num' is not used on this layer, so print 'N/A':
          fprintf(fp_html, "    <TD align=\"center\"><FONT color=\"grey\">N/A</FONT></TD>\n");
        else {
          if (user_inputs->usedOnLayers[DR_num][layer / 2] == 1)
            // 'usedOnLayers' value is 1, so no design-rule conflicts:
            strcpy(background_color, "white");
          else 
            // 'usedOnLayers' value is 2, so there's a design-rule conflict:
            strcpy(background_color, "red");
            
          fprintf(fp_html, "    <TD bgcolor=\"%s\" align=\"center\">\n", background_color);
          fprintf(fp_html, "      <input type=\"checkbox\" onclick=\"document.getElementById('DRmap_%02d-%02d').style.visibility=(this.checked)?'visible':'hidden';\"></TD>\n",
                  layer / 2, DR_num);
        }  // End of if/else-block for 'usedOnLayers == 0 or non-zero'
      }  // End of for-loop for index 'DR_num'

      // Print hyperlinks to toggle zones for trace cost multipliers
      for (int i = 0; i < maxTraceCostMultipliers; i++)  {
        if (user_inputs->traceCostMultiplierUsed[i])  {
          if (user_inputs->costUsedOnLayer[i][layer])  {
            fprintf(fp_html, "    <TD align=\"center\">\n");
            fprintf(fp_html, "      <input type=\"checkbox\" onclick=\"document.getElementById('layer%02d_cost%02d').style.visibility=(this.checked)?'visible':'hidden';\"></TD>\n",
                    layer, i);
          }
          else  {
            fprintf(fp_html, "    <TD align=\"center\"><FONT size=\"1\" color=\"grey\">Not used</FONT></TD>\n");
          }
        }
      }  // End of for-loop for iterating over routing cost-multipliers

      // Print 'N/A' for via-cost multipliers on this routing layer:
      for (int i = 0; i < maxViaCostMultipliers; i++)  {
        if (user_inputs->viaCostMultiplierUsed[i])  {
          // Layer is a routing layer, so print 'N/A':
          fprintf(fp_html, "    <TD align=\"center\"><FONT size=\"1\" color=\"grey\">N/A</FONT></TD>\n");
        }
      }  // End of for-loop for iterating over via cost-multipliers

      // If this HTML report is for a routed iteration, then print out a checkbox to toggle visibility of
      // 'Congestion' and 'Explored Cells':
      if (iteration > 0)  {
        for (int shape_type = 0; shape_type < NUM_SHAPE_TYPES; shape_type++)  {
          fprintf(fp_html, "    <TD align=\"center\">\n");
          fprintf(fp_html, "      <input type=\"checkbox\" onclick=\"document.getElementById('cong_%02d-%1d').style.visibility=(this.checked)?'visible':'hidden';\"></TD>\n",
                  layer, shape_type);
        }  // End of for-loop for index 'shape_type'

        // Print out a checkbox to toggle visibility of Explored Cells for normal A* routing:
        fprintf(fp_html, "    <TD align=\"center\">\n");
        fprintf(fp_html, "      <input type=\"checkbox\" onclick=\"document.getElementById('expl_%02d').style.visibility=(this.checked)?'visible':'hidden';\"></TD>\n",
                layer);

        // Print out a checkbox to toggle visibility of Explored Cells for post-processed routing:
        fprintf(fp_html, "    <TD align=\"center\">\n");
        fprintf(fp_html, "      <input type=\"checkbox\" onclick=\"document.getElementById('explPP_%02d').style.visibility=(this.checked)?'visible':'hidden';\"></TD>\n",
                layer);

      }  // End of if-block for iteration > 0

    }  // End of if-block for layer being a routing layer (not a via layer)
    else  {
      // Layer is a via layer, so print a blank cell with a column-span of numDesignRuleSets:
      fprintf(fp_html, "    <TD align=\"center\" colspan=\"%d\"><FONT size=\"1\" color=\"grey\">N/A</FONT></TD>\n", user_inputs->numDesignRuleSets);

      // Layer is a via layer, so print 'N/A' for trace cost multipliers:
      for (int i = 0; i < maxTraceCostMultipliers; i++)  {
        if (user_inputs->traceCostMultiplierUsed[i])  {
          fprintf(fp_html, "    <TD align=\"center\"><FONT size=\"1\" color=\"grey\">N/A</FONT></TD>\n");
        }
      }

      // Layer is a via layer, so print hyperlinks to toggle via-cost multipliers:
      for (int i = 0; i < maxViaCostMultipliers; i++)  {
        if (user_inputs->viaCostMultiplierUsed[i])  {
          if (user_inputs->costUsedOnLayer[i][layer])  {
            fprintf(fp_html, "    <TD align=\"center\">\n");
            fprintf(fp_html, "      <input type=\"checkbox\" onclick=\"document.getElementById('layer%02d_cost%02d').style.visibility=(this.checked)?'visible':'hidden';\"></TD>\n",
                    layer, i);
          }
          else  {
            fprintf(fp_html, "    <TD align=\"center\"><FONT size=\"1\" color=\"grey\">Not used</FONT></TD>\n");
          }
        }
      }  // End of for-loop for cost index 'i'

      // If this report is for a routed iteration, then print out empty cells under the
      // 'Congestion' and 'Explored Cells' headers.
      if (iteration > 0)  {
        // Layer is a via layer, so print a blank cell with a column-span of 2 * NUM_SHAPE_TYPES
        fprintf(fp_html, "    <TD align=\"center\" colspan=\"%d\"><FONT size=\"1\" color=\"grey\">N/A</FONT></TD>\n", NUM_SHAPE_TYPES);

        // Layer is a via layer, so print a blank cell with a column-span of 2 (rather than a checkbox to toggle visibility)
        fprintf(fp_html, "    <TD align=\"center\" colspan=\"2\"><FONT size=\"1\" color=\"grey\">N/A</FONT></TD>\n");
      }  // End of if-block for iteration > 0

    }  // End of else-block for layer being a via layer (not a routing layer)

    fprintf(fp_html, "  </TR>\n");
  }  // End of for-loop for index 'layer'
  fprintf(fp_html, "</TABLE>\n\n");
  
  fprintf(fp_html, "<!-- This CSS is needed to overlay multiple images: -->\n");
  fprintf(fp_html, "<STYLE type=\"text/css\">\n");
  fprintf(fp_html, "  .container_0 { float: left; position: relative; }\n");
  fprintf(fp_html, "  .container_1 { position: absolute; top: 0; right: 0; }\n");
  fprintf(fp_html, "</STYLE>\n\n");
  fprintf(fp_html, "<!-- Overlaid PNG images go here: -->\n");
  fprintf(fp_html, "<DIV class=\"container_0\">\n");

  // Write HTML for first image file
  fprintf(fp_html, "  <IMG id=\"layer_%02d\" border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                      numPngLayers-1, pngPathFileNames[numPngLayers-1], mapInfo->mapWidth * mag, 
                      mapInfo->mapHeight * mag);

  // Write HTML for subsequent image files for path maps:
  for (int layer = numPngLayers-2; layer >= 0; layer--)  {
    if (layer %2)  {
      fprintf(fp_html, "  <IMG id=\"layer_%02d\" class=\"container_1\" border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\" style=\"visibility:hidden\">\n",
                        layer, pngPathFileNames[layer], mapInfo->mapWidth * mag, mapInfo->mapHeight * mag);
    }
    else  {
      fprintf(fp_html, "  <IMG id=\"layer_%02d\" class=\"container_1\" border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                        layer, pngPathFileNames[layer], mapInfo->mapWidth * mag, mapInfo->mapHeight * mag);
    }
  }

  // If this HTML report is for a routed iteration, write HTML for PNG image files that show
  // the Explored Cells for routing layers only (not via-layers):
  if (iteration > 0)  {
    for (int layer = numPngLayers-1; layer >= 0; layer--)  {
      isViaLayer = layer % 2;  // = TRUE if via layer (odd); FALSE if routing layer (even).
      if (! isViaLayer)  {
        // Explored cells from normal A* routing:
        fprintf(fp_html, "  <IMG id=\"expl_%02d\" class=\"container_1\" border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\" style=\"visibility:hidden\">\n",
                          layer, pngExplorationFileNames[layer], mapInfo->mapWidth * mag, mapInfo->mapHeight * mag);

        // Explored cells from post-processing routing:
        fprintf(fp_html, "  <IMG id=\"explPP_%02d\" class=\"container_1\" border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\" style=\"visibility:hidden\">\n",
                          layer, pngPPExplorationFileNames[layer], mapInfo->mapWidth * mag, mapInfo->mapHeight * mag);
      }
    }
  }  // End of if-block for iteration > 0

  // Write HTML for congestion image files and design-rule zones on routing 
  // layers only (not via-layers):
  for (int layer = mapInfo->numLayers-1; layer >= 0; layer--)  {

    // Design-rule zones:
    for (int DR_num = 0; DR_num < user_inputs->numDesignRuleSets; DR_num++)  {
      fprintf(fp_html, "  <IMG id=\"DRmap_%02d-%02d\" class=\"container_1\" border=\"1\" src=\"DRmap_layer%02d_%s_DRset%02d_%s.png\" alt=\"\" width=\"%d\" height=\"%d\" style=\"visibility:hidden\">\n",
                        layer, DR_num, layer, (*user_inputs).layer_names[2 * layer], 
                        DR_num, user_inputs->designRuleSetName[DR_num],
                        mapInfo->mapWidth * mag, mapInfo->mapHeight * mag);
    }  // End of for-loop for index 'DR_num'

    // Congestion maps (only if this report is for a routed iteration):
    if (iteration > 0)  {
      for (int shape_type = 0; shape_type < NUM_SHAPE_TYPES; shape_type++)  {
        fprintf(fp_html, "  <IMG id=\"cong_%02d-%1d\" class=\"container_1\" border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\" style=\"visibility:hidden\">\n",
                          2*layer, shape_type, pngCongestionFileNames[2*layer][shape_type], mapInfo->mapWidth * mag,
                          mapInfo->mapHeight * mag);
      }  // End of for-loop for index 'shape_type'
    }  // End of if-block for iteration > 0

  }  // End of for-loop for index 'layer'

  // Iterate through PNG layers and cost indices to print out PNG cost-map
  // layers that are used in the map:
  for (int pngLayer = 2 * mapInfo->numLayers - 2; pngLayer >= 0; pngLayer--)  {

    isViaLayer = pngLayer % 2;  // = TRUE if via layer (odd); FALSE if routing layer (even).

    if (! isViaLayer)  {
      // Cycle through indices for trace-cost multipliers
      for (int i = 0; i < maxTraceCostMultipliers; i++)  {
        if (user_inputs->costUsedOnLayer[i][pngLayer])  {
          // Write HTML for subsequent image files:
          fprintf(fp_html, "  <IMG id=\"layer%02d_cost%02d\" class=\"container_1\" border=\"1\" src=\"costMap_layer%02d_%s_cost%02d_%dX.png\" alt=\"\" width=\"%d\" height=\"%d\" style=\"visibility:hidden\">\n",
                  pngLayer, i, pngLayer, user_inputs->layer_names[pngLayer], i, user_inputs->traceCostMultiplier[i],
                  mapInfo->mapWidth * mag, mapInfo->mapHeight * mag);
        }  // End of if-block for costUsedOnLayer
      }  // End of for-loop for index 'i' (0 to maxTraceCostMultipliers)
    }  // End of if-block for (! isViaLayer)
    else  {
      // Cycle through indices for via-cost multipliers
      for (int i = 0; i < maxViaCostMultipliers; i++)  {
        if (user_inputs->costUsedOnLayer[i][pngLayer])  {
          // Write HTML for subsequent image files:
          fprintf(fp_html, "  <IMG id=\"layer%02d_cost%02d\" class=\"container_1\" border=\"1\" src=\"costMap_layer%02d_%s_cost%02d_%dX.png\" alt=\"\" width=\"%d\" height=\"%d\" style=\"visibility:hidden\">\n",
                  pngLayer, i, pngLayer, user_inputs->layer_names[pngLayer], i, user_inputs->viaCostMultiplier[i],
                  mapInfo->mapWidth * mag, mapInfo->mapHeight * mag);
        }  // End of if-block for costUsedOnLayer
      }  // End of for-loop for index 'i' (0 to maxViaCostMultipliers)
    }  // End of if/else-block for (! isViaLayer)
  }  // End of for-loop for index 'pngLayer'

  fprintf(fp_html, "</DIV>\n\n");


  // Print routability metrics to HTML file if this HTML report is for a routed iteration:
  if (iteration > 0)  {
    fprintf(fp_html, "<TABLE border=\"1\" cellpadding=\"2\"><TR><TD><PRE>\n");
    // Calculate the total number of nets to report, including user-defined nets and
    // (if applicable) pseudo nets for differential pairs:
    int max_routed_nets = user_inputs->num_nets + user_inputs->num_pseudo_nets;
    printRoutabilityMetrics(fp_html, routability, user_inputs, mapInfo, max_routed_nets, 15);
    fprintf(fp_html, "\n</PRE></TD></TR></TABLE>\n");
  }  // End of if-block for iteration > 0

  fprintf(fp_html, "</BODY></HTML>\n");
  fclose(fp_html);

  //
  // Call function to create the PNG files that contains the path maps:
  //
  returnCode = makePngPathMaps(mag, numPngLayers, pngPathFileNames, mapInfo, user_inputs, cellInfo, title);
  if (returnCode) {
    printf("\nERROR: A problem occurred in function 'makePngPathMaps'. Report this issue to the software developer.\n");
    printf(  "       Program is exiting.\n\n");
    exit(returnCode);
  }

  //
  // If this HTML report is for a routed iteration, then call function to create the
  // PNG files that contains the congestion maps:
  //
  if (iteration > 0)  {
    returnCode = makePngCongestionMaps(mag, mapInfo, pngCongestionFileNames, cellInfo, title);
    if (returnCode) {
      printf("\nERROR: A problem occurred in function 'makePngCongestionMaps'. Report this issue to the software developer.\n");
      printf(  "       Program is exiting.\n\n");
      exit(returnCode);
    }
  }  // End of if-block for iteration > 0

  //
  // If this HTML report is for a routed iteration, then call function to create the PNG files
  // that contains the maps of explored cells:
  //
  if (iteration > 0)  {
    returnCode = makePngExplorationMaps(mag, mapInfo, pngExplorationFileNames, FALSE, cellInfo, title);
    if (returnCode) {
      printf("\nERROR: A problem occurred in function 'makePngExplorationMaps'. Report this issue to the software developer.\n");
      printf(  "       Program is exiting.\n\n");
      exit(returnCode);
    }

    returnCode = makePngExplorationMaps(mag, mapInfo, pngPPExplorationFileNames, TRUE, cellInfo, title);
    if (returnCode) {
      printf("\nERROR: A problem occurred in function 'makePngExplorationMaps'. Report this issue to the software developer.\n");
      printf(  "       Program is exiting.\n\n");
      exit(returnCode);
    }

  }  // End of if-block for iteration > 0


  // Free memory that was allocated for filenames:
  for (int layer = 0; layer < numPngLayers; layer++) {

    // PNG filenames for routing layers, including the maps for pre-routing:
    free(pngPathFileNames[layer]);
    pngPathFileNames[layer] = NULL; // Set pointer to NULL as a precaution
    // printf("DEBUG: Successfully free'd pngPathFileNames[%d].\n", layer);

    // If this iteration was for a routed iteration, then free the memory for
    // filenames associated with congestion and explored cells:
    if (iteration > 0)  {

      // PNG filenames for congestion:
      for (int shape_type = 0; shape_type < NUM_SHAPE_TYPES; shape_type++)  {
        free( pngCongestionFileNames[layer][shape_type] );
        pngCongestionFileNames[layer][shape_type] = NULL; // Set pointer to NULL as a precaution
        // printf("DEBUG: Successfully free'd pngCongestionFileNames[%d][%d].\n", layer, shape_type);
      }  // End of for-loop for index 'shape_type'
      free( *pngCongestionFileNames[layer] );
      *pngCongestionFileNames[layer] = NULL; // Set pointer to NULL as a precaution
      // printf("DEBUG: Successfully free'd pngCongestionFileNames[%d].\n", layer);

      // PNG filenames for exploration layers:
      free(pngExplorationFileNames[layer]);
      pngExplorationFileNames[layer] = NULL; // Set pointer to NULL as a precaution
      // printf("DEBUG: Successfully free'd pngExplorationFileNames[%d].\n", layer);

      free(pngPPExplorationFileNames[layer]);
      pngPPExplorationFileNames[layer] = NULL; // Set pointer to NULL as a precaution
      // printf("DEBUG: Successfully free'd pngPPExplorationFileNames[%d].\n", layer);

    }  // End of if-block for iteration > 0

  }  // End of for-loop for index 'layer'

  return returnCode;
}  // End of function 'makeHtmlIterationSummary'


//-----------------------------------------------------------------------------
// Name: updateHtmlTableOfContents
// Desc: Update the HTML table-of-contents file with the results of
//       iteration # 'pathFinderRun', including the generation of PNG map-files
//       and a new HTML file to display these PNG files.
//-----------------------------------------------------------------------------
void updateHtmlTableOfContents(FILE *fp_TOC, MapInfo_t *mapInfo, CellInfo_t ***cellInfo,
                               InputValues_t *user_inputs, RoutingMetrics_t *routability,
                               DRC_details_t DRC_details[maxRecordedDRCs],
                               char *shapeTypeNames[NUM_SHAPE_TYPES], int cost_multipliers_used )  {

  // printf("DEBUG: Entered function updateHtmlTableOfContents with num_nonPseudo_DRC_cells = %d, num_pseudo_DRC_cells = %d, total_num_DRC_cells = %d\n",
  //        routability->num_nonPseudo_DRC_cells, routability->num_pseudo_DRC_cells, routability->total_num_DRC_cells);

  // Declare variables for generating time-stamps later on:
  time_t tim;
  struct tm *now;

  setlocale(LC_NUMERIC, "en_US"); // Enables formatting integers like "9,876,543"

  // Write a PNG file that displays the paths in a map (a) if it's the first iteration,
  // (b) every Nth iteration (with N = runsPerPngMap), or (c) a DRC-free solution
  // is found:
  if ((mapInfo->current_iteration == 1)
      || ((*user_inputs).runsPerPngMap * (mapInfo->current_iteration / (*user_inputs).runsPerPngMap) == mapInfo->current_iteration)
      || (routability->num_nonPseudo_DRC_cells == 0))  {

    // Print a time-stamp to STDOUT:
    tim = time(NULL);
    now = localtime(&tim);
    printf("Date-stamp before generating PNG maps: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
          now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


    // Generate PNG map:
    makeHtmlIterationSummary(mapInfo->current_iteration, mapInfo, cellInfo, user_inputs, routability,
                   "Title", DRC_details, shapeTypeNames);

    // Print a time-stamp to STDOUT:
    tim = time(NULL);
    now = localtime(&tim);
    printf("Date-stamp after generating PNG maps: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
          now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

    // Add hyperlink to the the table-of-contents HTML file for the newly created
    // PNG maps:
    fprintf(fp_TOC, "  <LI><A href=\"iteration%04d.html\">Iteration %d</A>:&nbsp;",
            mapInfo->current_iteration, mapInfo->current_iteration);
    if (routability->num_nonPseudo_DRC_cells == 0)  {
      // Number of non-pseudo DRCs is zero, so use blue font color:
      fprintf(fp_TOC, "<FONT color=\"blue\">%'d cells with DRCs</FONT>",
                       routability->num_nonPseudo_DRC_cells);
    }
    else  {
      // Number of non-pseudo DRCs is non-zero, so use black font color:
      fprintf(fp_TOC, "<FONT color=\"black\">%'d cells with DRCs</FONT>",
                       routability->num_nonPseudo_DRC_cells);
    }  // End of if/else block for non-pseudo DRCs == 0

    // Print out 'disregarding user-defined cost zones' if this is the first iteration
    // and the user defined cost-zones:
    if ((mapInfo->current_iteration == 1) && (cost_multipliers_used == TRUE))  {
      fprintf(fp_TOC, "<FONT color=\"black\"> (disregarding user-defined cost zones)</FONT>");
    }

    // Print out the path length and via count:
    fprintf(fp_TOC, ", <FONT color=\"#B0B0B0\">trace length is %'.4f mm with %d vias. %d/%d nets have DRCs. (%'lu cells explored in ",
                       routability->total_lateral_nonPseudo_length_mm, routability->total_nonPseudo_vias, routability->num_paths_with_DRCs,
                       routability->num_DRCfree_paths + routability->num_paths_with_DRCs, routability->iteration_explored_cells[mapInfo->current_iteration]);
    if (routability->iteration_elapsed_time[mapInfo->current_iteration] > 1)
      fprintf(fp_TOC, "%'d seconds).</FONT>\n", routability->iteration_elapsed_time[mapInfo->current_iteration]);
    else if (routability->iteration_elapsed_time[mapInfo->current_iteration] == 1)
      fprintf(fp_TOC, "~%'d second).</FONT>\n", routability->iteration_elapsed_time[mapInfo->current_iteration]);
    else
      fprintf(fp_TOC, "< 1 second).</FONT>\n");


    // If the number of cells with non-pseudo DRCs is less than 'maxRecordedDRCs' (typically 10),
    // then also display the  names of the nets involved in the crossings and the
    // locations of the overlaps.
    if ((routability->num_nonPseudo_DRC_cells > 0) && (routability->num_nonPseudo_DRC_cells <= maxRecordedDRCs))  {
      fprintf(fp_TOC, " <input type=\"button\" onclick=\"return toggleMe('showHide%d')\" value=\"Display/hide DRC info\" style=\"height:15px; width:130px; font-family: sans-serif; font-size: 10px;\"><BR>\n",
              mapInfo->current_iteration);
      fprintf(fp_TOC, " <UL id=\"showHide%d\" style=\"display:none\">", mapInfo->current_iteration);

      for (int DRC_index = 0; DRC_index < routability->num_nonPseudo_DRC_cells; DRC_index++)  {

        // printf("DEBUG: Printing DRC #%d to HTML file...\n", DRC_index);
        fprintf(fp_TOC, "  <LI>DRC on layer %s at location (%.0f, %.0f) microns between %s of net %s and the center of a %s in net %s (min spacing = %.2f; min dist = %.2f microns).\n",
                user_inputs->layer_names[2 * DRC_details[DRC_index].z], // Layer name of DRC location
                DRC_details[DRC_index].x * user_inputs->cell_size_um,   // X-coordinate (in um) of DRC location
                DRC_details[DRC_index].y * user_inputs->cell_size_um,   // Y-coordinate (in um) of DRC location
                shapeTypeNames[DRC_details[DRC_index].shapeType],       // Shape-type name at location of DRC violation
                user_inputs->net_name[DRC_details[DRC_index].pathNum],  // Net name at location of DRC violation
                shapeTypeNames[DRC_details[DRC_index].offendingShapeType],      // Shape-type name of offending shape
                user_inputs->net_name[DRC_details[DRC_index].offendingPathNum], // Net name of offending net
                DRC_details[DRC_index].minimumAllowedSpacing,           // Minimum allowed spacing from edge to edge (in um)
                DRC_details[DRC_index].minimumAllowedDistance);         // Minimum allowed distance from edge to centerline (in um)

      }  // End of for-loop for index 'DRC_index'

      fprintf(fp_TOC, "</UL>");

    }  // End of if-block for (num_nonPseudo_DRC_cells > 0) && (num_nonPseudo_DRC_cells < maxRecordedDRCs)

  }  // End of if-block for printing out a PNG map file
  else  {
    // For iterations in which a PNG map is not generated, add a simple text summary to the
    // table-of-contents HTML file for this run's metrics:
    fprintf(fp_TOC, "  <LI><FONT color=\"blue\">Iteration %d:&nbsp;</FONT>", mapInfo->current_iteration );
    fprintf(fp_TOC, "<FONT color=\"#B0B0B0\">%'d cells with DRCs, trace length is %'.4f mm with %d vias. %d/%d nets have DRCs. (%'lu cells explored in %'d seconds).</FONT>\n",
                     routability->num_nonPseudo_DRC_cells, routability->total_lateral_nonPseudo_length_mm, routability->total_nonPseudo_vias,
                     routability->num_paths_with_DRCs, routability->num_DRCfree_paths + routability->num_paths_with_DRCs,
                     routability->iteration_explored_cells[mapInfo->current_iteration], routability->iteration_elapsed_time[mapInfo->current_iteration]);

    // If the number of crossings is maxRecordedDRCs (typically 10) or less, then also 
    // display the names of the nets involved in the crossings.
    if ((routability->num_nonPseudo_DRC_cells > 0) && (routability->num_nonPseudo_DRC_cells <= maxRecordedDRCs))  {
      fprintf(fp_TOC, " <input type=\"button\" onclick=\"return toggleMe('showHide%d')\" value=\"Display/hide DRC info\" style=\"height:15px; width:130px; font-family: sans-serif; font-size: 10px;\"><BR>\n",
              mapInfo->current_iteration);
      fprintf(fp_TOC, " <UL id=\"showHide%d\" style=\"display:none\">", mapInfo->current_iteration);

      for (int DRC_index = 0; DRC_index < routability->num_nonPseudo_DRC_cells; DRC_index++)  {
        // printf("DEBUG: Printing DRC #%d to HTML file...\n", DRC_index);
        fprintf(fp_TOC, "  <LI>DRC on layer %s at location (%.0f, %.0f) microns between %s of net %s and the center of a %s in net %s (min spacing = %.2f; min dist = %.2f microns).\n",
           user_inputs->layer_names[2 * DRC_details[DRC_index].z], // Layer name of DRC location
           DRC_details[DRC_index].x * user_inputs->cell_size_um,   // X-coordinate (in um) of DRC location
           DRC_details[DRC_index].y * user_inputs->cell_size_um,   // Y-coordinate (in um) of DRC location
           shapeTypeNames[DRC_details[DRC_index].shapeType],       // Shape-type name at location of DRC violation
           user_inputs->net_name[DRC_details[DRC_index].pathNum],  // Net name at location of DRC violation
           shapeTypeNames[DRC_details[DRC_index].offendingShapeType],      // Shape-type name of offending shape
           user_inputs->net_name[DRC_details[DRC_index].offendingPathNum], // Net name of offending net
           DRC_details[DRC_index].minimumAllowedSpacing,           // Minimum allowed spacing from edge to edge (in um)
           DRC_details[DRC_index].minimumAllowedDistance);         // Minimum allowed distance from edge to centerline (in um)

      }  // End of for-loop for index 'DRC_index'

      fprintf(fp_TOC, "</UL>");
    }  // End of if-block for num_nonPseudo_DRC_cells <= maxRecordedDRCs

  }  // End of else-block for printing summary of iteration without a PNG map

  // Print a time-stamp to STDOUT:
  tim = time(NULL);
  now = localtime(&tim);
  // printf("Date-stamp at end of function 'updateHtmlTableOfContents': %02d-%02d-%d, %02d:%02d *************************\n",
  //       now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min);

}  // End of function 'updateHtmlTableOfContents'


//-----------------------------------------------------------------------------
// Name: makeDesignRulePngMaps
// Desc: Create PNG map files that display the locations of the one or more 
//       design-rule (DR) zones in grey. If there are conflicts between design 
//       rules between two layers, then these regions are colored red.
//
//       Also populate the following arrays:
//
//        user_inputs->usedOnLayers[maxDesignRuleSets][maxRoutingLayers]
//            mapInfo->maxInteractionRadiusCellsOnLayer[maxRoutingLayers]
//            mapInfo->maxInteractionRadiusSquaredOnLayer[maxRoutingLayers]
//
//       This function also defines user_inputs->designRuleUsed[DR_num]
//       to specify whether design-rule set 'DR_num' is used anywhere
//       in the map. If a design-rule set is not used anywhere in the
//       map, then this function sets to FALSE all elements of the
//       array user_inputs->DR_subsetUsed[DR_num][DR_subset_num], for
//       all values of DR_subset_num.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_makeDesignRulePngMaps' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_makeDesignRulePngMaps 1
#undef DEBUG_makeDesignRulePngMaps

int makeDesignRulePngMaps(CellInfo_t ***cellInfo, MapInfo_t *mapInfo, InputValues_t *user_inputs)  {

  #ifdef DEBUG_makeDesignRulePngMaps
  printf("\nDEBUG: Entered function 'makeDesignRulePngMaps'...\n");
  #endif

  int red, green, blue, opacity; // Variables for specifying color of each pixel

  int returnCode = 0; // Will change to 1 if conflict is found between design-rule sets

  int mag = 1; // Default (and minimum) magnification factor for PNG files
  if (mag * mapInfo->mapWidth < 1000)
    mag = 1000 / mapInfo->mapWidth;
  // printf("DEBUG: value of 'mag' is %d.\n", mag);

  // Allocate memory for one row (4 bytes per pixel: red, green, blue, and alpha)
  png_bytep row = (png_bytep) malloc(4 * mapInfo->mapWidth * mag * sizeof(png_byte));
  
  // Define string variable for the name of the PNG file. String length must accommodate
  // 29 characters plus the length of the design-rule name and layer name, with this format:
  //       DRmap_layer##_<layerName>_DRset##_<DRname>.png
  char DR_map_filename[maxDesRuleSetNameLength + maxLayerNameLength + 29]; // Name of PNG file

  // 
  // Iterate over each layer and over each design-rule set:
  // 
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    // printf("  DEBUG: Starting layer #%d...\n", layer);
    for (int DRnum = 0; DRnum < user_inputs->numDesignRuleSets; DRnum++)  {
      // printf("    DEBUG: Design-rule set #%d...\n", DRnum);

      // Set up variables for the PNG file for current combination of
      // routing layer and design-rule number:
      FILE *fp;
      png_structp png_ptr;
      png_infop info_ptr;
 
      // 
      // Create name of PNG file for this combination of 'layer' and
      // 'DRnum'. File name has following format:
      //        DRmap_layer##_<layerName>_DRset##_<DRname>.png
      // where the '##' represent the numeric values of the layer number
      // and design-rule set, respectively.
      // 
      sprintf(DR_map_filename, "DRmap_layer%02d_%s_DRset%02d_%s.png", 
               layer, (*user_inputs).layer_names[2*layer], DRnum, user_inputs->designRuleSetName[DRnum]);
      
      // Open file for writing (binary mode)
      fp = fopen(DR_map_filename, "wb");
      if (fp == NULL) {
        fprintf(stderr, "\nERROR: Could not open PNG file '%s' for writing\n\n", DR_map_filename);
        returnCode = 1;
        goto finalize_DRzones;
      }
  
      // printf("    DEBUG: Opened file for writing: '%s'\n", DR_map_filename);

      // Initialize write structure
      png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      if (png_ptr == NULL) {
        fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct.\n\n");
        returnCode = 1;
        goto finalize_DRzones;
      }
      // printf("      DEBUG: ran 'png_create_write_struct'\n");
  
      // Initialize info structure
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL) {
        fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct.\n\n");
        returnCode = 1;
        goto finalize_DRzones;
      }
      // printf("      DEBUG: ran 'png_create_info_struct'\n");

      // Setup Exception handling
      if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "\nERROR during PNG creation.\n\n");
        returnCode = 1;
        goto finalize_DRzones;
      }
      // printf("      DEBUG: ran 'setjmp'\n");

      png_init_io(png_ptr, fp);

      // Write header (8 bit colour depth)
      // The 'IHDR' chunk is the first chunk in a PNG file. It contains the image's width, 
      // height, color type and bit depth.
      png_set_IHDR(png_ptr, info_ptr, mapInfo->mapWidth*mag, mapInfo->mapHeight*mag,
                   8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
      // printf("      DEBUG: ran 'png_set_IHDR'\n");

      // Set title of PNG file
      png_text title_text;
      title_text.compression = PNG_TEXT_COMPRESSION_NONE;
      title_text.key = "Title";
      title_text.text = "Title";
      png_set_text(png_ptr, info_ptr, &title_text, 1);
      // printf("      DEBUG: ran 'set_text'\n");

      png_write_info(png_ptr, info_ptr);
      // printf("      DEBUG: ran 'png_write_info'\n");

      //
      // Write image data:
      //
      for (int y = mapInfo->mapHeight-1 ; y >= 0; y--) {
        for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
          // printf("        DEBUG: Creating pixel at (x,y) = (%d, %d) with DR number %d.\n", 
          //           x, y, cellInfo[x][y][layer].designRuleSet);
 
          // Set default color to grey. Color will be changed to RED if
          // a design-rule conflict is found for this pixel:
          red = green = blue = 0x80; // Grey color

          //
          // Check if current cell's design-rule set matches set number 'DRnum':
          //
          if (cellInfo[x][y][layer].designRuleSet == DRnum)  {
            // Set cell to semi-opaque if this cell uses design-rule 'DRnum'
            opacity = 0x80;  // Semi-opaque if this cell uses design-rule 'DRnum'

            // Set 'usedOnLayer' flag to '1' for this combination of design-rule set
            // and layer number:
            user_inputs->usedOnLayers[DRnum][layer] = 1;

            // Set 'designRuleUsed' flag to TRUE for this design-rule set to
            // indicate that it's used somewhere in the entire map.
            user_inputs->designRuleUsed[DRnum] = TRUE;

            // Set 'DR_subsetUsed' flag to TRUE for subset #0 (the default subset) of
            // this design-rule set to indicate that it's used somewhere in the entire map.
            user_inputs->DR_subsetUsed[DRnum][0] = TRUE;

          }  // End of if-block for DRnum == cell's design-rule number
          else  { 
            opacity = 0x00;  // 100% transparent if this cell does not use design-rule 'DRnum' 
          }  // End of if/else-block for for designRuleSet == DRnum

          // printf("          DEBUG: Opacity calculated to be %d.\n", opacity);
          
          for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {
            // printf("            DEBUG: About to 'setRGBA' with arguments %d, R=%d, G=%d, B=%d, and A=%d\n",
            //         &(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
            
            setRGBA(&(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
          }  // end of repeat_x for-loop
      
        }  // End of x for-loop
      
        for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
          png_write_row(png_ptr, row);
        }  // End of repeat_y for-loop
      
      }  // End of y for-loop
    
      // End write
      png_write_end(png_ptr, NULL);

      finalize_DRzones:
      if (fp != NULL)  {
        fclose(fp);
        // printf("    DEBUG: Closed file '%s'\n", DR_map_filename);
        fp = NULL;
      }
      if (png_ptr != NULL)  {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        png_ptr = NULL;
      }
  
    }  // End of for-loop for index 'DRnum'
  }  // End of for-loop for index 'layer'

  // printf("DEBUG: Before free, address of 'row' is %p.\n", row);
  if (row != NULL)  {
    free(row);
    row = NULL;
  }
  // printf("DEBUG: After free, address of 'row' is %p.\n", row);


  // Iterate through each of the design-rule sets to confirm that each is indeed
  // used somewhere in the map. If a design-rule set is *not* used, then set to
  // FALSE the Boolean flags for the usage of this design-rule's subsets:
  for (int DR_num = 0; DR_num < user_inputs->numDesignRuleSets; DR_num++)  {

    // Check if this design-rule number is used in the map:
    if (! user_inputs->designRuleUsed[DR_num])  {

      // We got here, so design-rule number 'DR_num' is not used anywhere in the map.
      // We therefore set to FALSE all the flags for this design-rule's subsets:
      for (int DR_subset = 0; DR_subset < user_inputs->numDesignRuleSubsets[DR_num]; DR_subset++)  {
        user_inputs->DR_subsetUsed[DR_num][DR_subset] = FALSE;
      }  // End of for-loop for index 'DR_subset'
    }  // End of if-block
  }  // End of for-loop for index 'DR_num'

  //
  // Update the arrays maxInteractionRadiusCellsOnLayer and
  // maxInteractionRadiusSquaredOnLayer to reflect the largest interaction
  // distance of any design-rule set used on each layer:
  //
  // Iterate over each layer and over each design-rule set:
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    for (int DRnum = 0; DRnum < user_inputs->numDesignRuleSets; DRnum++)  {
      if (user_inputs->usedOnLayers[DRnum][layer])  {
        if (user_inputs->maxInteractionRadiusCellsInDR[DRnum] > mapInfo->maxInteractionRadiusCellsOnLayer[layer])  {
          mapInfo->maxInteractionRadiusCellsOnLayer[layer] = user_inputs->maxInteractionRadiusCellsInDR[DRnum];
          mapInfo->maxInteractionRadiusSquaredOnLayer[layer] = user_inputs->maxInteractionRadiusSquaredInDR[DRnum];
        }  // End of if-block for maxInteractionRadiusCellsInDR > maxInteractionRadiusCellsOnLayer
      }  // End of if-block for usedOnLayers == TRUE
    }  // End of for-loop for index 'DRnum'

    #ifdef DEBUG_makeDesignRulePngMaps
    printf("\nDEBUG: At end of function makeDesignRulePngMaps in iteration %d:\n", mapInfo->current_iteration);
    printf(  "DEBUG:      maxInteractionRadiusCellsOnLayer[%d] = %d\n", layer, mapInfo->maxInteractionRadiusCellsOnLayer[layer]);
    printf(  "DEBUG:    maxInteractionRadiusSquaredOnLayer[%d] = %d\n\n", layer, mapInfo->maxInteractionRadiusSquaredOnLayer[layer]);
    #endif

  }  // End of for-loop for index 'layer'

  #ifdef DEBUG_makeDesignRulePngMaps
  printf("\nDEBUG: Exiting function 'makeDesignRulePngMaps'.\n\n");
  #endif

  return returnCode;

}  // End of function 'makeDesignRulePngMaps'


//-----------------------------------------------------------------------------
// Name: makeDesignRuleReport
// Desc: Create HTML report of the various design-rule sets. Also include
//       generic PNG image depicting cross-section of design rules (not to scale).
//-----------------------------------------------------------------------------
void makeDesignRuleReport(CellInfo_t ***cellInfo, InputValues_t *user_inputs, MapInfo_t *mapInfo)  {

  // Open file 'designRules.html' for writing:
  FILE *fp_HTML;
  char output_filename[20];
  strcpy(output_filename, "designRules.html");
  fp_HTML = fopen(output_filename, "w");

  // Start writing HTML to file:
  fprintf(fp_HTML, "<!DOCTYPE HTML>\n<HEAD><TITLE>Design Rules</TITLE>\n");

  // Print Javascript that's used for toggling visibility of images (layers):
  fprintf(fp_HTML, "<SCRIPT language=\"javascript\" type=\"text/javascript\">\n\n");
  fprintf(fp_HTML, "function setImageVisible(id, visible) {\n");
  fprintf(fp_HTML, "  var img = document.getElementById(id);\n");
  fprintf(fp_HTML, "  img.style.visibility = (visible ? 'visible' : 'hidden');\n}\n");
  fprintf(fp_HTML, "</SCRIPT>\n</HEAD>\n\n");

  fprintf(fp_HTML, "<BODY>\n");
  fprintf(fp_HTML, "<H1><U>Design Rules</U></H1>\n\n");
  if (user_inputs->numDesignRuleSets > 1)  {
    fprintf(fp_HTML, "<H3>%d design-rule sets are defined:</H3>\n\n", user_inputs->numDesignRuleSets);
  }
  else if (user_inputs->numDesignRuleSets == 1)  {
    fprintf(fp_HTML, "<H3>%d design-rule set is defined:</H3>\n\n", user_inputs->numDesignRuleSets);
  }
  else  {
    fprintf(fp_HTML, "<H3>No design-rule sets are defined. All design spacings, trace widths, and via-land diameters are therefore zero.</H3>\n\n");
  }

  // Determine whether design-rule sets conflict with each other. If so,
  // then issue a warning message in red font:
  int DR_conflict = FALSE;
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    for (int DR_set_num = 0; DR_set_num < user_inputs->numDesignRuleSets; DR_set_num++)  {
      if (user_inputs->usedOnLayers[DR_set_num][layer] == 2)
        DR_conflict = TRUE;
    }  // End of for-loop for index 'DR_set_num'
  }  // End of for-loop for index 'layer'

  if (DR_conflict)  {
    fprintf(fp_HTML, "<H3><FONT color=\"red\">\n");
    fprintf(fp_HTML, "Design rules conflict with each other due to differences \n");
    fprintf(fp_HTML, "in via diameters or spacings. See red regions below.</FONT></H3><BR><BR>\n");
  }

 
  // Write PNG file that depicts cross-section of traces and vias:
  FILE *fp_PNG;
  char PNG_filename[20];
  strcpy(PNG_filename, PNG_output_file_name);
  fp_PNG = fopen(PNG_filename, "wb");
  fwrite(designRulePNGdata, PNG_file_length, 1, fp_PNG);
  fclose(fp_PNG);

  // Display PNG file near top of web page:
  fprintf(fp_HTML, "  <IMG border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\"><BR><BR>\n",
               PNG_filename, PNG_image_width/2, PNG_image_height/2);

  // For each design-rule set, print the design rules to HTML file:
  for (int DR_set_num = 0; DR_set_num < user_inputs->numDesignRuleSets; DR_set_num++)  {

    fprintf(fp_HTML, "  <TABLE border=\"1\">\n");
    fprintf(fp_HTML, "    <TR><TD bgcolor=\"LightGray\">\n");  // Beginning of left column of outer-most, 3-column table
    fprintf(fp_HTML, "      <TABLE border=\"0\" cellpadding=\"1\">\n");
    fprintf(fp_HTML, "        <TR>\n");  // Row with name of design-rule set:
    fprintf(fp_HTML, "          <TH align=\"right\">Name:</TH>\n");
    if (user_inputs->designRuleUsed[DR_set_num])  {
      // Print out name of design-rule set because this design-rule set is indeed used in the map:
      fprintf(fp_HTML, "          <TD colspan=\"6\" align=\"left\">%s</TD>\n", user_inputs->designRuleSetName[DR_set_num]);
    }
    else  {
      // Print out name of design-rule set followed by (NOT USED) because this set is not used anywhere in the map:
      fprintf(fp_HTML, "          <TD colspan=\"6\" align=\"left\">%s <FONT color=\"red\"><I><B>(NOT USED)</B></I></FONT></TD>\n",
              user_inputs->designRuleSetName[DR_set_num]);
    }
    fprintf(fp_HTML, "        </TR>\n");

    fprintf(fp_HTML, "        <TR>\n");  // Row with description of design-rule set:
    fprintf(fp_HTML, "          <TH align=\"right\">Description:</TH>\n");
    fprintf(fp_HTML, "          <TD colspan=\"6\" align=\"left\">%s</TD>\n", user_inputs->designRuleSetDescription[DR_set_num]);
    fprintf(fp_HTML, "        </TR>\n");

    fprintf(fp_HTML, "        <TR>\n");  // Row with number of design-rule set
    fprintf(fp_HTML, "          <TH align=\"right\"><FONT size=\"2\">Number:</FONT></TH>\n");
    fprintf(fp_HTML, "          <TD colspan=\"6\" align=\"left\"><FONT size=\"2\">%d", DR_set_num + 1);
    if (DR_set_num == 0)
      fprintf(fp_HTML,          " (default set)");  // Add '(default)' if it's the first design-rule set
    fprintf(fp_HTML,           "</FONT></TD>\n");
    fprintf(fp_HTML, "        </TR>\n");
    fprintf(fp_HTML, "      </TABLE>\n");

    // Create new HTML table for each subset of design-rules:
    for (int DR_subset_num = 0; DR_subset_num < user_inputs->numDesignRuleSubsets[DR_set_num]; DR_subset_num++)  {
      fprintf(fp_HTML, "      <TABLE border=\"0\" cellpadding=\"1\">\n"); //
      fprintf(fp_HTML, "        <TR><TD bgcolor=\"black\" colspan=\"7\"></TD></TR>\n");  // Blank, black row to separate DR subsets
      if (user_inputs->numDesignRuleSubsets[DR_set_num] > 0)  {  // Subset #0 is the default subset for this design-rule
        if (DR_subset_num == 0)  {
          // Row stating that these are the default (non-exception) design rules
          if (user_inputs->DR_subsetUsed[DR_set_num][DR_subset_num])  {
            // Print out 'Default Rules' because this design-rule subset is indeed used in the map:
            fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Default Rules</I></U></B></TD></TR>\n");
          }
          else  {
            // Print out 'Default Rules (NOT USED)' because this design-rule subset is not used anywhere in the map:
            fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Default Rules</I></U></B> <FONT color=\"red\"><I><B>(NOT USED)</B></I></FONT></TD></TR>\n");
          }
        }
        else  {  // Subset number is greater than 0, so this subset is an exception:
          if (   (  user_inputs->designRules[DR_set_num][DR_subset_num].isDiffPairSubset)
              && (! user_inputs->designRules[DR_set_num][DR_subset_num].isPseudoNetSubset))  {

            // Check whether this design-rule subset is indeed used in the map:
            if (user_inputs->DR_subsetUsed[DR_set_num][DR_subset_num])  {
              // This subset is dedicated to diff-pairs, so print HTML row with name of design-rule exception and diff-pair pitch
              fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Exception #%d</U>: '%s' for differential pairs</I></B><BR>&nbsp;&nbsp;(%4.1f um diff-pair pitch)<BR><BR></TD></TR>\n",
                      DR_subset_num, user_inputs->designRules[DR_set_num][DR_subset_num].subsetName,
                                     user_inputs->designRules[DR_set_num][DR_subset_num].traceDiffPairPitchMicrons);
            }
            else  {
              // This subset is dedicated to diff-pairs but is not used, so print HTML row with name of design-rule exception
              // and '(NOT USED)', followed by diff-pair pitch
              fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Exception #%d</U>: '%s' for differential pairs</I></B> <FONT color=\"red\"><I><B>(NOT USED)</B></I></FONT><BR>&nbsp;&nbsp;(%4.1f um diff-pair pitch)<BR><BR></TD></TR>\n",
                      DR_subset_num, user_inputs->designRules[DR_set_num][DR_subset_num].subsetName,
                                     user_inputs->designRules[DR_set_num][DR_subset_num].traceDiffPairPitchMicrons);
            }

          }  // End of if-block for isDiffPairSubset == TRUE

          else if (user_inputs->designRules[DR_set_num][DR_subset_num].isPseudoNetSubset)  {

            // Check whether this design-rule subset is indeed used in the map:
            if (user_inputs->DR_subsetUsed[DR_set_num][DR_subset_num])  {

              // This subset is dedicated to pseudo-nets, so print HTML row with name of design-rule exception
              fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Exception #%d</U>: '%s' for pseudo-nets</I></B><BR><BR></TD></TR>\n", DR_subset_num,
                        user_inputs->designRules[DR_set_num][DR_subset_num].subsetName);
            }
            else  {
              // This subset is dedicated to pseudo-nets but is not used, so print HTML row with name of design-rule exception
              // and '(NOT USED)':
              fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Exception #%d</U>: '%s' for pseudo-nets</I></B> <FONT color=\"red\"><I><B>(NOT USED)</B></I></FONT><BR><BR></TD></TR>\n", DR_subset_num,
                        user_inputs->designRules[DR_set_num][DR_subset_num].subsetName);
            }

          }  // End of if-block for isPseudoNetSubset == TRUE

          else  {

            // This subset is an exception that's not related to diff-pairs or pseudo-nets, so
            // print HTML row with name of design-rule exception.

            // Check whether this design-rule subset is indeed used in the map:
            if (user_inputs->DR_subsetUsed[DR_set_num][DR_subset_num])  {
              // This subset is indeed used in the map, so print out its name:
              fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Exception #%d</U>: '%s'</I></B><BR><BR></TD></TR>\n", DR_subset_num,
                        user_inputs->designRules[DR_set_num][DR_subset_num].subsetName);
            }
            else  {
              // This subset is not used anywhere in the map, so print out its name followed by '(NOT USED)':
              fprintf(fp_HTML, "        <TR><TD colspan=\"7\"><B><U><I>Exception #%d</U>: '%s'</I></B> <FONT color=\"red\"><I><B>(NOT USED)</B></I></FONT><BR><BR></TD></TR>\n", DR_subset_num,
                        user_inputs->designRules[DR_set_num][DR_subset_num].subsetName);
            }

          }  // End of else-block

        }  // End of else-block for 'exception' design-rule subsets
      }  // End of if-block for numDesignRuleSubsets > 0

      fprintf(fp_HTML, "        <TR>\n");  // Row with Trace Width and title for 'Shape-to-Shape Spacings'
      fprintf(fp_HTML, "          <TH align=\"right\">Trace Width:</TH>\n");
      fprintf(fp_HTML, "          <TD align=\"left\">%5.0f um <FONT size=\"2\">(1)</FONT></TD>\n",
                                    user_inputs->designRules[DR_set_num][DR_subset_num].width_um[TRACE]);
      fprintf(fp_HTML, "          <TD>&nbsp;&nbsp;&nbsp;</TD>\n");   // Blank cell
      fprintf(fp_HTML, "          <TH align=\"center\" colspan=\"4\"><U>Shape-to-Shape Spacings (um)</U></TH>\n");
      fprintf(fp_HTML, "        </TR>\n");

      fprintf(fp_HTML, "        <TR>\n");  // Row with blank cells and a 4x4 cell for nested table:
      fprintf(fp_HTML, "          <TD colspan=\"2\">&nbsp;</TD>\n");   // 2 blank cells
      fprintf(fp_HTML, "          <TD></TD>\n");   // Blank cell

      fprintf(fp_HTML, "          <TD colspan=\"4\" rowspan=\"4\">\n");  // 4x4 cell for nested table
      fprintf(fp_HTML, "            <TABLE border=\"1\">\n");  // Beginning of nested, 4x4-cell table
      fprintf(fp_HTML, "              <TR>\n");  // Row #1 of nested table contains header cells
      fprintf(fp_HTML, "                <TH align=\"center\"></TH>\n");
      fprintf(fp_HTML, "                <TH align=\"center\">Trace</TH>\n");
      fprintf(fp_HTML, "                <TH align=\"center\">Via-Up</TH>\n");
      fprintf(fp_HTML, "                <TH align=\"center\">Via-Down</TH>\n");
      fprintf(fp_HTML, "              </TR>\n");

      fprintf(fp_HTML, "              <TR>\n");  // Row #2 of nested table contains Trace-to-X spacings
      fprintf(fp_HTML, "                <TH align=\"right\">Trace</TH>\n");
      fprintf(fp_HTML, "                <TD align=\"center\">%5.0f <FONT size=\"2\">(4)</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[TRACE][TRACE]);
      fprintf(fp_HTML, "                <TD align=\"center\">%5.0f <FONT size=\"2\">(5)</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[TRACE][VIA_UP]);
      fprintf(fp_HTML, "                <TD align=\"center\">%5.0f <FONT size=\"2\">(6)</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[TRACE][VIA_DOWN]);
      fprintf(fp_HTML, "              </TR>\n");

      fprintf(fp_HTML, "              <TR>\n");  // Row #3 of nested table contains ViaUp-to-X spacings
      fprintf(fp_HTML, "                <TH align=\"right\">Via-Up</TH>\n");
      fprintf(fp_HTML, "                <TD align=\"center\"><FONT color=\"grey\">%5.0f</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[VIA_UP][TRACE]);
      fprintf(fp_HTML, "                <TD align=\"center\">%5.0f <FONT size=\"2\">(7)</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[VIA_UP][VIA_UP]);
      fprintf(fp_HTML, "                <TD align=\"center\">%5.0f <FONT size=\"2\">(8)</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[VIA_UP][VIA_DOWN]);
      fprintf(fp_HTML, "              </TR>\n");

      fprintf(fp_HTML, "              <TR>\n");  // Row #4 of nested table contains ViaDown-to-X spacings
      fprintf(fp_HTML, "                <TH align=\"right\">Via-Down</TH>\n");
      fprintf(fp_HTML, "                <TD align=\"center\"><FONT color=\"grey\">%5.0f</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[VIA_DOWN][TRACE]);
      fprintf(fp_HTML, "                <TD align=\"center\"><FONT color=\"grey\">%5.0f</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[VIA_DOWN][VIA_UP]);
      fprintf(fp_HTML, "                <TD align=\"center\">%5.0f <FONT size=\"2\">(9)</FONT></TD>\n",
                                            user_inputs->designRules[DR_set_num][DR_subset_num].space_um[VIA_DOWN][VIA_DOWN]);
      fprintf(fp_HTML, "              </TR>\n");

      fprintf(fp_HTML, "            </TABLE>\n");  // End of nested, 4x4-cell table
      fprintf(fp_HTML, "          </TD>\n");  // End of 4x4 cell for nested table
      fprintf(fp_HTML, "        </TR>\n");

      fprintf(fp_HTML, "        <TR>\n");  // Row with Via-Up Diameter
      fprintf(fp_HTML, "          <TH align=\"right\">Via-Up Diameter:</TH>\n");
      fprintf(fp_HTML, "          <TD align=\"left\">%5.0f um <FONT size=\"2\">(2)</FONT></TD>\n",
                                      user_inputs->designRules[DR_set_num][DR_subset_num].width_um[VIA_UP]);
      fprintf(fp_HTML, "          <TD></TD>\n");   // Blank cell
      fprintf(fp_HTML, "        </TR>\n");

      fprintf(fp_HTML, "        <TR>\n");  // Blank rows to left of nested, 4x4 table
      fprintf(fp_HTML, "          <TD colspan=\"2\">&nbsp;</TD>\n");   // 2 blank cells
      fprintf(fp_HTML, "          <TD></TD>\n");   // 1 blank cell
      fprintf(fp_HTML, "        </TR>\n");

      fprintf(fp_HTML, "        <TR>\n");  // Row with Via-Down Diameter
      fprintf(fp_HTML, "          <TH align=\"right\">Via-Down Diameter:</TH>\n");
      fprintf(fp_HTML, "          <TD align=\"left\">%5.0f um <FONT size=\"2\">(3)</FONT></TD>\n",
                                    user_inputs->designRules[DR_set_num][DR_subset_num].width_um[VIA_DOWN]);
      fprintf(fp_HTML, "          <TD></TD>\n");   // 1 blank cell
      fprintf(fp_HTML, "        </TR>\n");

      fprintf(fp_HTML, "        <TR><TD colspan=\"7\" align=\"center\"><FONT size=\"2\">\n");
      fprintf(fp_HTML, "           Parenthetical values denote dimensions in <A href=\"%s\">figure</A>.</FONT></TD></TR>\n",
                                   PNG_filename);

      char * html_routing_description;
      if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == ANY)  {
        html_routing_description = "All directions<BR><center><FONT size=\"1\">(N/NxNE/NE/ExNE/E/ExSE/SE/SxSE/S/SxSW/SW/WxSW/W/WxNW/NW/NxNW/up/down)</FONT></center>";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == NONE)  {
        html_routing_description = "No routing allowed";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == MANHATTAN)  {
        html_routing_description = "Manhattan routing<BR><center><FONT size=\"1\">(N/S/E/W/up/down)</FONT></center>";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == X_ROUTING)  {
        html_routing_description = "X-routing<BR><center><FONT size=\"1\">(NE/SE/SW/NW/up/down)</FONT></center>";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == NORTH_SOUTH)  {
        html_routing_description = "North-South routing<BR><center><FONT size=\"1\">(N/S/up/down)</FONT></center>";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == EAST_WEST)  {
        html_routing_description = "East-West routing<BR><center><FONT size=\"1\">(E/W/up/down)</FONT></center>";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == MANHATTAN_X)  {
        html_routing_description = "Manhattan and X-routing<BR><center><FONT size=\"1\">(N/NE/E/SE/S/SW/W/NW/up/down)</FONT></center>";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == UP_DOWN)  {
        html_routing_description = "Up-Down routing through vias<BR><center><FONT size=\"1\">(up/down)</FONT></center>";
      }
      else if (user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections == ANY_LATERAL)  {
        html_routing_description = "Lateral routing through traces<BR><center><FONT size=\"1\">(N/NxNE/NE/ExNE/E/ExSE/SE/SxSE/S/SxSW/SW/WxSW/W/WxNW/NW/NxNW)</FONT></center>";
      }
      else  {
        printf("\n\nERROR: In function 'makeDesignRuleReport', an illegal value was detected for variable 'routeDirections': %d\n",
               user_inputs->designRules[DR_set_num][DR_subset_num].routeDirections);
        printf(    "       Please inform the software developer of this fatal error message.\n\n");
        exit(1);
      }

      fprintf(fp_HTML, "        <TR><TD colspan=\"7\" align=\"left\"><FONT size=\"3\">\n");
      fprintf(fp_HTML, "           <BR><B>Allowed routing directions:</B> %s</FONT></TD></TR>\n", html_routing_description);

      fprintf(fp_HTML, "      </TABLE>\n");
    }  // End of for-loop for index 'j' (0 to numDesignRuleExceptions)

    fprintf(fp_HTML, "    </TD>\n");  // End of left column of 3-column table

    fprintf(fp_HTML, "    <TD valign=\"middle\">\n");  // Beginning of middle column of 3-column table

    fprintf(fp_HTML, "      <TABLE border=\"1\" cellpadding=\"2\">\n");
    fprintf(fp_HTML, "        <TR>\n");
    fprintf(fp_HTML, "          <TH>Layer</TH>\n");
    fprintf(fp_HTML, "          <TH colspan=\"2\" align=\"center\">Usage</TH>\n");
    fprintf(fp_HTML, "        </TR>\n");

    //
    // In the body of the HTML table, print out 1 row for each routing layer:
    //
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
  
      // Print out layer name:
      fprintf(fp_HTML, "        <TR>\n");
      fprintf(fp_HTML, "          <TD align=\"center\"><B>%s</B></TD>\n", user_inputs->layer_names[2*layer]);

      char background_color[10];
      if (user_inputs->usedOnLayers[DR_set_num][layer] == 0)
        // Design-rule set 'DR_num' is not used on this layer, so print 'N/A':
        fprintf(fp_HTML, "          <TD colspan=\"2\" align=\"center\">N/A</TD>\n");
      else {
        if (user_inputs->usedOnLayers[DR_set_num][layer] == 1)
          // 'usedOnLayers' value is 1, so no design-rule conflicts:
          strcpy(background_color, "white");
        else
          // 'usedOnLayers' value is 2, so there's a design-rule conflict:
          strcpy(background_color, "red");

        fprintf(fp_HTML, "          <TD bgcolor=\"%s\" align=\"center\"><A href=\"javascript:setImageVisible('layer%02d_DR%02d', true)\">Show</A></TD>\n",
                background_color, layer, DR_set_num);
        fprintf(fp_HTML, "          <TD bgcolor=\"%s\" align=\"center\"><A href=\"javascript:setImageVisible('layer%02d_DR%02d', false)\">Hide</A></TD>\n",
                background_color, layer, DR_set_num);
      }  // End of if/else-block for 'usedOnLayers == 0 or non-zero'
      fprintf(fp_HTML, "        </TR>\n");

    }  // End of for-loop for index 'layer'

    fprintf(fp_HTML, "      </TABLE>\n");

    fprintf(fp_HTML, "    </TD>\n");  // End of middle column of 3-column table

    fprintf(fp_HTML, "    <TD><valign=\"middle\">\n");  // Beginning of right-most column of 3-column table
    fprintf(fp_HTML, "      <FONT size=\"2\" color=\"grey\">Usage of '%s':</FONT><BR>\n", 
                            user_inputs->designRuleSetName[DR_set_num]);

    fprintf(fp_HTML, "      <!-- This CSS is needed to overlay multiple images: -->\n");
    fprintf(fp_HTML, "      <STYLE type=\"text/css\">\n");
    fprintf(fp_HTML, "        .container_0 { float: left; position: relative; }\n");
    fprintf(fp_HTML, "        .container_1 { position: absolute; top: 0; right: 0; }\n");
    fprintf(fp_HTML, "      </STYLE>\n\n");
    fprintf(fp_HTML, "      <!-- Overlaid images go here: -->\n");
    fprintf(fp_HTML, "      <DIV class=\"container_0\">\n");
 
    // Calculate magnification factor for images. 'mag' is calculated so that image
    // height is ~300 pixels.
    float mag = (float)(300.0 / mapInfo->mapHeight);
    int image_width  = (int)(mapInfo->mapWidth  * mag);
    int image_height = (int)(mapInfo->mapHeight * mag);
  
    // Write HTML for first image file:
    fprintf(fp_HTML, "        <IMG id=\"layer%02d_DR%02d\" border=\"1\" src=\"DRmap_layer%02d_%s_DRset%02d_%s.png\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                              mapInfo->numLayers-1, DR_set_num, mapInfo->numLayers-1, user_inputs->layer_names[2*(mapInfo->numLayers-1)], 
                              DR_set_num, user_inputs->designRuleSetName[DR_set_num], image_width, image_height);

    // Write HTML for subsequent image files:
    for (int layer = mapInfo->numLayers - 2; layer >= 0; layer--)  {
      fprintf(fp_HTML, "        <IMG id=\"layer%02d_DR%02d\" class=\"container_1\" border=\"1\" src=\"DRmap_layer%02d_%s_DRset%02d_%s.png\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                                layer, DR_set_num, layer, user_inputs->layer_names[2*layer], 
                                DR_set_num, user_inputs->designRuleSetName[DR_set_num], image_width, image_height);

    }

    fprintf(fp_HTML, "      </DIV>\n\n");





    fprintf(fp_HTML, "    </TD>\n");  // End of right-most column of 3-column table
    fprintf(fp_HTML, "  </TR></TABLE><BR>\n\n");

  }  // End of for-loop for index 'DR_set_num'

  
  // Display PNG file (again) near bottom of web page if there are at least two design-rule
  // sets:
  if (user_inputs->numDesignRuleSets >= 2)
    fprintf(fp_HTML, "  <IMG border=\"1\" src=\"%s\" alt=\"\" width=\"%d\" height=\"%d\"><BR><BR>\n",
                   PNG_filename, PNG_image_width/2, PNG_image_height/2);

  // Close file 'designRules.html' 
  fprintf(fp_HTML, "</HTML>\n");
  fclose(fp_HTML);


}  // End of function 'makeDesignRuleReport'


//-----------------------------------------------------------------------------
// Name: makeCostZonePngMaps
// Desc: Create PNG map files that display the cost zones for each routing
//       and via layer.
//-----------------------------------------------------------------------------
int makeCostZonePngMaps(CellInfo_t ***cellInfo, MapInfo_t *mapInfo, InputValues_t *user_inputs)  {

  // printf("DEBUG: Entered function 'makeCostZonePngMaps'...\n");

  int cost_multipliers_used = FALSE; // Boolean value that will be returned to the calling
                                     // program. TRUE if any non-unity cost-multipliers
                                     // are used anywhere in the map for traces or vias.

  int red, green, blue, opacity; // Variables for specifying color of each pixel
  red = green = blue = 0x80; // Set RGB color to grey
  int isViaLayer = FALSE; // 1 if layer is a via layer; 0 if a routing layer.
  int mag = 1; // Default (and minimum) magnification factor for PNG files
  if (mag * mapInfo->mapWidth < 1000)
    mag = 1000 / mapInfo->mapWidth;
  // printf("DEBUG: value of 'mag' is %d.\n", mag);

  // Allocate memory for one row (4 bytes per pixel: red, green, blue, and alpha)
  png_bytep row = (png_bytep) malloc(4 * mapInfo->mapWidth * mag * sizeof(png_byte));
  
  // Define string variable for the name of the PNG file. String length must accommodate
  // 33 characters plus the length of the design-rule name and layer name, with this format:
  //       costMap_layer##_<layerName>_cost##_<multiplier>X.png
  char cost_zone_map_filename[maxDesRuleSetNameLength + maxLayerNameLength + 33]; // Name of PNG file


  // Initialize to FALSE (zero) the array user_inputs->traceCostMultiplierUsed[traceCostIndex].
  // Also initialize to FALSE the matrix user_inputs->costUsedOnLayer[traceCostIndex][pngLayer]:
  for (int traceCostIndex = 0; traceCostIndex < maxTraceCostMultipliers; traceCostIndex++)  {
    // printf("DEBUG: costIndex = %d at address %p\n", traceCostIndex, &costIndex);
    user_inputs->traceCostMultiplierUsed[traceCostIndex] = FALSE;
    // printf("DEBUG: user_inputs->traceCostMultiplierUsed[traceCostIndex=%d] initialized to %d.\n", traceCostIndex, user_inputs->traceCostMultiplierUsed[traceCostIndex]);
    for (int pngLayer = 0; pngLayer < 2*mapInfo->numLayers - 1; pngLayer++)  {
      // printf("DEBUG: traceCostIndex = %d at address %p, pngLayer = %d at address %p\n", traceCostIndex, &traceCostIndex, pngLayer, &pngLayer);
      user_inputs->costUsedOnLayer[traceCostIndex][pngLayer] = FALSE;
      // printf("DEBUG: user_inputs->costUsedOnLayer[traceCostIndex=%d][pngLayer=%d] initialized to %d at address %p.\n",
      //        traceCostIndex, pngLayer, user_inputs->costUsedOnLayer[traceCostIndex][pngLayer], &user_inputs->costUsedOnLayer[traceCostIndex][pngLayer]);
    }  // End of for-loop for index 'pngLayer'
  }  // End of for-loop for index 'traceCostIndex'

  // Initialize to FALSE (zero) the array user_inputs->viaCostMultiplierUsed[viaCostIndex].
  for (int viaCostIndex = 0; viaCostIndex < maxViaCostMultipliers; viaCostIndex++)  {
    user_inputs->viaCostMultiplierUsed[viaCostIndex] = FALSE;
  }  // End of for-loop for index 'viaCostIndex'

  // 
  // Iterate over each layer (routing layers and via layers) and 
  // over each trace- and via-cost-multiplier:
  // 
  for (int pngLayer = 0; pngLayer < 2*mapInfo->numLayers - 1; pngLayer++)  {
    // printf("  DEBUG: Starting PNG layer #%d...\n", pngLayer);

    // Determine if layer is a via layer (odd number) or routing layer (even):
    isViaLayer = pngLayer % 2;
 
    if (! isViaLayer)  {
      // 
      // Handle routing layers, i.e., even-numbered PNG layers:
      //
      for (int costIndex = 0; costIndex < maxTraceCostMultipliers; costIndex++)  {
        // printf("    DEBUG: Trace cost index #%d...\n", costIndex);

        // If this 'costIndex' is not invoked in any 'XXXXX_cost_zone' instructions,
        // then skip this index. (Don't skip index 0, which is never explicitly invoked
        // in a XXXXX_cost_zone statement.)
        if ((costIndex > 0 ) &&  (! user_inputs->traceCostMultiplierInvoked[costIndex]))  {
          // printf("DEBUG: Trace cost index '%d' is not used for any trace_cost_zone commands, so skipping it.\n",
          //         costIndex);
          continue;
        }
  
        // Set up variables for the PNG file for current combination of
        // routing layer and design-rule number:
        FILE *fp;
        png_structp png_ptr;
        png_infop info_ptr;
   
        // 
        // Create name of PNG file for this combination of 'pngLayer' and
        // 'costIndex'. File name has following format:
        //        costMap_layer##_<layerName>_cost##_<multiplier>X.png
        // where the '##' represent the numeric values of the pngLayer number
        // and cost index, respectively.
        // 
        sprintf(cost_zone_map_filename, "costMap_layer%02d_%s_cost%02d_%dX.png", 
                 pngLayer, user_inputs->layer_names[pngLayer], costIndex, (int) user_inputs->traceCostMultiplier[costIndex]); 
        
        // Open file for writing (binary mode)
        fp = fopen(cost_zone_map_filename, "wb");
        if (fp == NULL) {
          fprintf(stderr, "\nERROR: Could not open PNG file '%s' for writing\n\n", cost_zone_map_filename);
          goto finalize_trace_cost_zones;
        }
    
        // printf("    DEBUG: Opened file for writing: '%s'\n", cost_zone_map_filename);
  
        // Initialize write structure
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) {
          fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct.\n\n");
          goto finalize_trace_cost_zones;
        }
        // printf("      DEBUG: ran 'png_create_write_struct'\n");
    
        // Initialize info structure
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
          fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct.\n\n");
          goto finalize_trace_cost_zones;
        }
        // printf("      DEBUG: ran 'png_create_info_struct'\n");
  
        // Setup Exception handling
        if (setjmp(png_jmpbuf(png_ptr))) {
          fprintf(stderr, "\nERROR during PNG creation.\n\n");
          goto finalize_trace_cost_zones;
        }
        // printf("      DEBUG: ran 'setjmp'\n");
  
        png_init_io(png_ptr, fp);
  
        // Write header (8 bit colour depth)
        // The 'IHDR' chunk is the first chunk in a PNG file. It contains the image's width, 
        // height, color type and bit depth.
        png_set_IHDR(png_ptr, info_ptr, mapInfo->mapWidth*mag, mapInfo->mapHeight*mag,
                     8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        // printf("      DEBUG: ran 'png_set_IHDR'\n");
  
        // Set title of PNG file
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = "Title";
        png_set_text(png_ptr, info_ptr, &title_text, 1);
        // printf("      DEBUG: ran 'set_text'\n");
  
        png_write_info(png_ptr, info_ptr);
        // printf("      DEBUG: ran 'png_write_info'\n");
  
        //
        // Write image data:
        //
        for (int y = mapInfo->mapHeight-1 ; y >= 0; y--) {
          for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
            // printf("        DEBUG: Creating pixel at (x,y) = (%d, %d) with cost-multiplier index %d.\n", 
            //           x, y, cellInfo[x][y][pngLayer/2].traceCostMultiplierIndex);
   
            //
            // Check if current cell's cost-multiplier index matches 'costIndex':
            //
            if (cellInfo[x][y][pngLayer/2].traceCostMultiplierIndex == costIndex)  {
              // Set cell to semi-opaque if this cell uses cost-multiplier index 'costIndex':
              opacity = 0x80; 
  
              // Set 'traceCostMultiplierUsed' flag to TRUE for this cost-multiplier index:
              if (! user_inputs->traceCostMultiplierUsed[costIndex])  {
                user_inputs->traceCostMultiplierUsed[costIndex] = TRUE;
                // printf("DEBUG: In makeCostZonePngMaps, trace cost index #%d was detected in the map.\n",
                //         costIndex);
              }

              // Set 'usedOnLayer' flag to TRUE for this combination of cost-multiplier
              // index and PNG layer number:
              if (! user_inputs->costUsedOnLayer[costIndex][pngLayer])  {
                user_inputs->costUsedOnLayer[costIndex][pngLayer] = TRUE;
                // printf("DEBUG: In makeCostZonePngMaps, trace cost index #%d was detected on PNG layer %d.\n",
                //         costIndex, pngLayer);
              }

  
            }  // End of if-block for costIndex == cell's cost-multiplier index
            else  { 
              opacity = 0x00;  // 100% transparent if this cell does not use cost index 'costIndex' 
            }  // End of if/else-block for for traceCostMultiplierIndex == costIndex
  
            // printf("          DEBUG: Opacity calculated to be %d.\n", opacity);
            
            for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {
              // printf("            DEBUG: About to 'setRGBA' with arguments %d, R=%d, G=%d, B=%d, and A=%d\n",
              //         &(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
              
              setRGBA(&(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
            }  // end of repeat_x for-loop
        
          }  // End of x for-loop
        
          for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
            png_write_row(png_ptr, row);
          }  // End of repeat_y for-loop
        
        }  // End of y for-loop
      
        // End write
        png_write_end(png_ptr, NULL);
  
        finalize_trace_cost_zones:
        if (fp != NULL)  {
          fclose(fp);
          // printf("    DEBUG: Closed file '%s'\n", cost_zone_map_filename);
          fp = NULL;
        }
        if (png_ptr != NULL)  {
          png_destroy_write_struct(&png_ptr, &info_ptr);
          png_ptr = NULL;
        }
    
      }  // End of for-loop for index 'costIndex'
    }  // End of if-block for (! isViaLayer)
    else  {

      // 
      // Handle via layers, i.e., odd-numbered PNG layers:
      //
      for (int costIndex = 0; costIndex < maxViaCostMultipliers; costIndex++)  {
        // printf("    DEBUG: Via cost index #%d...\n", costIndex);
  
        // If this 'costIndex' is not invoked by any 'XXXXX_cost_zone' instructions,
        // then skip this index. (Don't skip costIndex 0, since this index is never
        // invoked explicitly in a XXXXX_cost_zone instruction.)
        if ((costIndex > 0) && (! user_inputs->viaCostMultiplierInvoked[costIndex]))  {
          // printf("DEBUG: Via cost index '%d' is not used for any via_cost_zone commands, so skipping it.\n",
          //         costIndex);
          continue;
        }
  
        // Set up variables for the PNG file for current combination of
        // routing layer and design-rule number:
        FILE *fp;
        png_structp png_ptr;
        png_infop info_ptr;
   
        // 
        // Create name of PNG file for this combination of 'pngLayer' and
        // 'costIndex'. File name has following format:
        //        costMap_layer##_<layerName>_cost##_<multiplier>X.png
        // where the '##' represent the numeric values of the pngLayer number
        // and cost index, respectively.
        // 
        sprintf(cost_zone_map_filename, "costMap_layer%02d_%s_cost%02d_%dX.png", 
                 pngLayer, user_inputs->layer_names[pngLayer], costIndex, (int) user_inputs->viaCostMultiplier[costIndex]); 
        
        // Open file for writing (binary mode)
        fp = fopen(cost_zone_map_filename, "wb");
        if (fp == NULL) {
          fprintf(stderr, "\nERROR: Could not open PNG file '%s' for writing\n\n", cost_zone_map_filename);
          goto finalize_via_cost_zones;
        }
    
        // printf("    DEBUG: Opened file for writing: '%s'\n", cost_zone_map_filename);
  
        // Initialize write structure
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) {
          fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct.\n\n");
          goto finalize_via_cost_zones;
        }
        // printf("      DEBUG: ran 'png_create_write_struct'\n");
    
        // Initialize info structure
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
          fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct.\n\n");
          goto finalize_via_cost_zones;
        }
        // printf("      DEBUG: ran 'png_create_info_struct'\n");
  
        // Setup Exception handling
        if (setjmp(png_jmpbuf(png_ptr))) {
          fprintf(stderr, "\nERROR during PNG creation.\n\n");
          goto finalize_via_cost_zones;
        }
        // printf("      DEBUG: ran 'setjmp'\n");
  
        png_init_io(png_ptr, fp);
  
        // Write header (8 bit colour depth)
        // The 'IHDR' chunk is the first chunk in a PNG file. It contains the image's width, 
        // height, color type and bit depth.
        png_set_IHDR(png_ptr, info_ptr, mapInfo->mapWidth*mag, mapInfo->mapHeight*mag,
                     8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        // printf("      DEBUG: ran 'png_set_IHDR'\n");
  
        // Set title of PNG file
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = "Title";
        png_set_text(png_ptr, info_ptr, &title_text, 1);
        // printf("      DEBUG: ran 'set_text'\n");
  
        png_write_info(png_ptr, info_ptr);
        // printf("      DEBUG: ran 'png_write_info'\n");
  
        // printf("DEBUG: *** user_inputs->costUsedOnLayer[costIndex=2][pngLayer=1] = %d. ***\n", user_inputs->costUsedOnLayer[2][1]);

        //
        // Write image data:
        //
        for (int y = mapInfo->mapHeight-1 ; y >= 0; y--) {
          for (int x = 0 ; x < mapInfo->mapWidth ; x++) {
            // printf("        DEBUG: Creating pixel at (x,y) = (%d, %d) with via cost-multiplier index %d.\n", 
            //           x, y, cellInfo[x][y][pngLayer/2].viaUpCostMultiplierIndex);
   
            //
            // Check if current cell's viaUp cost-multiplier index matches 'costIndex'. Note
            // that we calculate the routing layer number by integer-dividing 'pngLayer' by 2,
            // resulting in the routing layer *below* the via layer. That's why we look at
            // the viaUp multiplier index:
            //
            if (cellInfo[x][y][pngLayer/2].viaUpCostMultiplierIndex == costIndex)  {
              // Set cell to semi-opaque if this cell uses cost-multiplier index 'costIndex':
              opacity = 0x80; 
  
              // Set 'viaCostMultiplierUsed' flag to TRUE for this cost-multiplier index:
              if (! user_inputs->viaCostMultiplierUsed[costIndex])  {
                user_inputs->viaCostMultiplierUsed[costIndex] = TRUE;
                // printf("DEBUG: In makeCostZonePngMaps, via cost index #%d was detected in the map.\n",
                //         costIndex);

              }

              // Set 'usedOnLayer' flag to TRUE for this combination of cost-multiplier
              // index and PNG layer number:
              if (! user_inputs->costUsedOnLayer[costIndex][pngLayer])  {
                user_inputs->costUsedOnLayer[costIndex][pngLayer] = TRUE;
                // printf("DEBUG: In makeCostZonePngMaps, via cost index #%d was detected on PNG layer %d.\n",
                //         costIndex, pngLayer);
              }
  
            }  // End of if-block for costIndex == cell's cost-multiplier index
            else  { 
              opacity = 0x00;  // 100% transparent if this cell does not use cost index 'costIndex' 
            }  // End of if/else-block for for viaUpCostMultiplierIndex == costIndex
  
            // printf("          DEBUG: Opacity calculated to be %d.\n", opacity);
            
            for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {
              // printf("            DEBUG: About to 'setRGBA' with arguments %d, R=%d, G=%d, B=%d, and A=%d\n",
              //         &(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
              
              setRGBA(&(row[((x * mag) + repeat_x)*4]), red, green, blue, opacity);
            }  // end of repeat_x for-loop
        
          }  // End of x for-loop
        
          for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
            png_write_row(png_ptr, row);
          }  // End of repeat_y for-loop
        
        }  // End of y for-loop
      
        // End write
        png_write_end(png_ptr, NULL);
  
        finalize_via_cost_zones:
        if (fp != NULL)  {
          fclose(fp);
          // printf("    DEBUG: Closed file '%s'\n", cost_zone_map_filename);
          fp = NULL;
        }
        if (png_ptr != NULL)  {
          png_destroy_write_struct(&png_ptr, &info_ptr);
          png_ptr = NULL;
        }
    
      }  // End of for-loop for index 'costIndex'

    }  // End of else-block for handling via-layers
  }  // End of for-loop for index 'pngLayer'

  // printf("DEBUG: Before free, address of 'row' is %p.\n", row);
  if (row != NULL)  {
    free(row);
    row = NULL;
  }
  // printf("DEBUG: After free, address of 'row' is %p.\n", row);

  // Calculate the total number of trace- and via-cost multipliers used in the entire map:
  for (int i = 0; i < maxTraceCostMultipliers; i++)  {
    if (user_inputs->traceCostMultiplierUsed[i])  {
      user_inputs->numTraceMultipliersUsed++;
      if (user_inputs->traceCostMultiplier[i] > 1)
        // Set flag indicating that a non-unity multiplier is used in the map:
        cost_multipliers_used = TRUE;
    }
  }
  for (int i = 0; i < maxViaCostMultipliers; i++)  {
    if (user_inputs->viaCostMultiplierUsed[i])  {
      user_inputs->numViaMultipliersUsed++;
      if (user_inputs->traceCostMultiplier[i] > 1)
        // Set flag indicating that a non-unity multiplier is used in the map:
        cost_multipliers_used = TRUE;
    }
  }

  // printf("DEBUG: numTraceMultipliersUsed is %d.\n", user_inputs->numTraceMultipliersUsed);
  // printf("DEBUG: numViaMultipliersUsed is %d.\n", user_inputs->numViaMultipliersUsed);

  return(cost_multipliers_used);

}  // End of function 'makeCostZonePngMaps'


//-----------------------------------------------------------------------------
// Name: makeCostMapReport
// Desc: Create HTML report to illustrate the locations of the various cost zones.
//-----------------------------------------------------------------------------
void makeCostMapReport(CellInfo_t ***cellInfo, InputValues_t *user_inputs, MapInfo_t *mapInfo)  {

  // printf("DEBUG: Entered function 'makeCostMapReport'...\n");

  int isViaLayer; // TRUE if PNG layer is an add number; FALSE if an even number.

  // Open file 'costZones.html' for writing:
  FILE *fp_HTML;
  char output_filename[20];
  strcpy(output_filename, "costZones.html");
  fp_HTML = fopen(output_filename, "w");

  // Start writing HTML to file:
  fprintf(fp_HTML, "<!DOCTYPE HTML>\n<HEAD><TITLE>Cost Zones</TITLE>\n");

  // Print Javascript that's used for toggling visibility of images (layers):
  fprintf(fp_HTML, "<SCRIPT language=\"javascript\" type=\"text/javascript\">\n\n");
  fprintf(fp_HTML, "function setImageVisible(id, visible) {\n");
  fprintf(fp_HTML, "  var img = document.getElementById(id);\n");
  fprintf(fp_HTML, "  img.style.visibility = (visible ? 'visible' : 'hidden');\n}\n");
  fprintf(fp_HTML, "</SCRIPT>\n</HEAD>\n\n");

  fprintf(fp_HTML, "<BODY>\n");
  fprintf(fp_HTML, "<H1><U>Cost Zones</U></H1>\n\n");
  if (user_inputs->numTraceMultipliersUsed + user_inputs->numViaMultipliersUsed > 0)  {
    fprintf(fp_HTML, "<H3>%d trace cost-zones and %d via cost-zones are used in the map:</H3>\n\n", 
                      (int) user_inputs->numTraceMultipliersUsed, (int) user_inputs->numViaMultipliersUsed);
  }
  else  {
    fprintf(fp_HTML, "<H3>No trace or via cost-zones are defined</H3>\n\n");
  }

  fprintf(fp_HTML, "  <TABLE border=\"1\">\n");
  fprintf(fp_HTML, "    <TR>\n");
  fprintf(fp_HTML, "      <TD></TD>\n");

  // Print out header row that say 'Trace Cost Multipliers' and 'Via Cost Multipliers':
  if (user_inputs->numTraceMultipliersUsed)  {
    fprintf(fp_HTML, "      <TH colspan=\"%d\" bgcolor=\"lightgrey\">\n", 2 * user_inputs->numTraceMultipliersUsed);
    fprintf(fp_HTML, "        Trace Cost<BR>Multipliers\n");
    fprintf(fp_HTML, "      </TH>\n");
  }  // End of if-block for numTraceMultipliersUsed > 0
  if (user_inputs->numViaMultipliersUsed)  {
    fprintf(fp_HTML, "      <TH colspan=\"%d\" bgcolor=\"lightgrey\">\n", 2 * user_inputs->numViaMultipliersUsed);
    fprintf(fp_HTML, "        Via Cost<BR>Multipliers\n");
    fprintf(fp_HTML, "      </TH>\n");
  }  // End of if-block for numViaMultipliersUsed > 0
  fprintf(fp_HTML, "    </TR>\n");

  // Print out header row that lists the used trace and via cost multipliers, in addition
  // to the associated indices for each multiplier:
  fprintf(fp_HTML, "    <TR>\n");
  fprintf(fp_HTML, "      <TH bgcolor=\"lightgrey\">Layer</TH>\n");
  for (int i = 0; i < maxTraceCostMultipliers; i++)  {
    if (user_inputs->traceCostMultiplierUsed[i])  {
      // printf("DEBUG:    trace-cost multiplier #%d is used in the map, so including a header row for it.\n", i);
      if (i == 0)
        fprintf(fp_HTML, "      <TD colspan=\"2\" align=\"center\" bgcolor=\"lightgrey\"><B>%dx</B><FONT size=\"1\"><BR>#%d (default)</FONT></TD>\n", 
                  user_inputs->traceCostMultiplier[i], i);
      else
        fprintf(fp_HTML, "      <TD colspan=\"2\" align=\"center\" bgcolor=\"lightgrey\"><B>%dx</B><FONT size=\"1\"><BR>#%d</FONT></TD>\n", 
                  user_inputs->traceCostMultiplier[i], i);
    }
  }
  for (int i = 0; i < maxViaCostMultipliers; i++)  {
    if (user_inputs->viaCostMultiplierUsed[i])  {
      // printf("DEBUG:      via-cost multiplier #%d is used in the map, so including a header row for it.\n", i);
      if (i == 0)
        fprintf(fp_HTML, "      <TD colspan=\"2\" align=\"center\" bgcolor=\"lightgrey\"><B>%dx</B><FONT size=\"1\"><BR>#%d (default)</FONT></TD>\n", 
                  user_inputs->viaCostMultiplier[i], i);
      else
        fprintf(fp_HTML, "      <TD colspan=\"2\" align=\"center\" bgcolor=\"lightgrey\"><B>%dx</B><FONT size=\"1\"><BR>#%d</FONT></TD>\n", 
                  user_inputs->viaCostMultiplier[i], i);
    }
  }
  fprintf(fp_HTML, "    </TR>\n");

  //
  // In the body of the HTML table, print out 1 row for each PNG layer:
  //
  for (int pngLayer = 0; pngLayer < 2 * mapInfo->numLayers - 1; pngLayer++)  {

    // Print out layer name:
    fprintf(fp_HTML, "    <TR>\n");
    fprintf(fp_HTML, "      <TD align=\"center\" bgcolor=\"lightgrey\"><B>%s</B></TD>\n", user_inputs->layer_names[pngLayer]);

    // Determine if layer is a via layer (odd number) or routing layer (even):
    isViaLayer = pngLayer % 2;

    // Print hyperlinks for trace cost multipliers
    for (int i = 0; i < maxTraceCostMultipliers; i++)  {
      if (user_inputs->traceCostMultiplierUsed[i])  {
        if (isViaLayer)  {
          // Layer is a via layer, so print 'N/A':
          fprintf(fp_HTML, "      <TD align=\"center\" colspan=\"2\" bgcolor=\"black\"><FONT size=\"1\" color=\"grey\">N/A</FONT></TD>\n");
        }
        else  {
          // Layer is a routing layer, so print hyperlinks to show/hide layers if used on this layer:
          if (user_inputs->costUsedOnLayer[i][pngLayer])  {
            // printf("DEBUG:      trace-cost multiplier #%d is used on PNG layer %d, so including hyperlinks to toggle visibility.\n",
            //           i, pngLayer);
            fprintf(fp_HTML, "      <TD align=\"center\"><A href=\"javascript:setImageVisible('layer%02d_cost%02d', true)\">Show</A></TD>\n",
                            pngLayer, i);
            fprintf(fp_HTML, "      <TD align=\"center\"><A href=\"javascript:setImageVisible('layer%02d_cost%02d', false)\">Hide</A></TD>\n",
                            pngLayer, i);
          }
          else  {
            fprintf(fp_HTML, "      <TD align=\"center\" colspan=\"2\"><FONT size=\"1\" color=\"grey\">Not used</FONT></TD>\n");
          }
        }
      }
    }  // End of for-loop for iterating over routing cost-multipliers

    // Print hyperlinks for via cost multipliers
    for (int i = 0; i < maxViaCostMultipliers; i++)  {
      if (user_inputs->viaCostMultiplierUsed[i])  {
        if (! isViaLayer)  {
          // Layer is a routing layer, so print 'N/A':
          fprintf(fp_HTML, "      <TD align=\"center\" colspan=\"2\" bgcolor=\"black\"><FONT size=\"1\" color=\"grey\">N/A</FONT></TD>\n");
        }
        else  {
          // Layer is a via layer, so print hyperlinks to show/hide layers:
          if (user_inputs->costUsedOnLayer[i][pngLayer])  {
            // printf("DEBUG:      via-cost multiplier #%d is used on PNG layer %d, so including hyperlinks to toggle visibility.\n",
            //           i, pngLayer);
            fprintf(fp_HTML, "      <TD align=\"center\"><A href=\"javascript:setImageVisible('layer%02d_cost%02d', true)\">Show</A></TD>\n",
                            pngLayer, i);
            fprintf(fp_HTML, "      <TD align=\"center\"><A href=\"javascript:setImageVisible('layer%02d_cost%02d', false)\">Hide</A></TD>\n",
                            pngLayer, i);
          }
          else  {
            fprintf(fp_HTML, "      <TD align=\"center\" colspan=\"2\"><FONT size=\"1\" color=\"grey\">Not used</FONT></TD>\n");
          }
        }
      }
    }  // End of for-loop for iterating over via cost-multipliers

    fprintf(fp_HTML, "    </TR>\n");

  }  // End of for-loop for index 'layer'

  fprintf(fp_HTML, "  </TABLE>\n");

  //
  // Print out images beneath table
  //
  fprintf(fp_HTML, "      <!-- This CSS is needed to overlay multiple images: -->\n");
  fprintf(fp_HTML, "      <STYLE type=\"text/css\">\n");
  fprintf(fp_HTML, "        .container_0 { float: left; position: relative; }\n");
  fprintf(fp_HTML, "        .container_1 { position: absolute; top: 0; right: 0; }\n");
  fprintf(fp_HTML, "      </STYLE>\n\n");
  fprintf(fp_HTML, "      <!-- Overlaid images go here: -->\n");
  fprintf(fp_HTML, "      <DIV class=\"container_0\">\n");

  // Calculate magnification factor for images. 'mag' is calculated so that image
  // height is ~800 pixels.
  float mag = (float)(800.0 / mapInfo->mapHeight);
  int image_width  = (int)(mapInfo->mapWidth  * mag);
  int image_height = (int)(mapInfo->mapHeight * mag);


  // Iterate through PNG layers and cost indices to print out PNG layers that
  // are used in the map:
  char first_image_written = FALSE;  // Set to true after the first PNG file is written
  for (int pngLayer = 2 * mapInfo->numLayers - 2; pngLayer >= 0; pngLayer--)  {
 
    isViaLayer = pngLayer % 2;  // = TRUE if via layer (odd); FALSE if routing layer (even).
 
    if (! isViaLayer)  {
      // Cycle through indices for trace-cost multipliers
      for (int i = 0; i < maxTraceCostMultipliers; i++)  {
        if (user_inputs->costUsedOnLayer[i][pngLayer])  {
          if (! first_image_written)  {
  
            // Write HTML for first image file:
            fprintf(fp_HTML, "        <IMG id=\"layer%02d_cost%02d\" border=\"1\" src=\"costMap_layer%02d_%s_cost%02d_%dX.png\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                    pngLayer, i, pngLayer, user_inputs->layer_names[pngLayer], i, user_inputs->traceCostMultiplier[i],
                    image_width, image_height);
            first_image_written = TRUE;  // Set flag to TRUE so that subsequent images use 'class=container_1'
          }  // End of if-block for (! first_image_written)
          else  {
            // Write HTML for subsequent image files:
            fprintf(fp_HTML, "        <IMG id=\"layer%02d_cost%02d\" class=\"container_1\" border=\"1\" src=\"costMap_layer%02d_%s_cost%02d_%dX.png\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                    pngLayer, i, pngLayer, user_inputs->layer_names[pngLayer], i, user_inputs->traceCostMultiplier[i],
                    image_width, image_height);
          }  // End of else-block
        }  // End of if-block for costUsedOnLayer
      }  // End of for-loop for index 'i' (0 to maxTraceCostMultipliers)
    }  // End of if-block for (! isViaLayer)
    else  {
      // Cycle through indices for via-cost multipliers
      for (int i = 0; i < maxViaCostMultipliers; i++)  {
        if (user_inputs->costUsedOnLayer[i][pngLayer])  {
          if (! first_image_written)  {
  
            // Write HTML for first image file:
            fprintf(fp_HTML, "        <IMG id=\"layer%02d_cost%02d\" border=\"1\" src=\"costMap_layer%02d_%s_cost%02d_%dX.png\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                    pngLayer, i, pngLayer, user_inputs->layer_names[pngLayer], i, user_inputs->viaCostMultiplier[i],
                    image_width, image_height);
            first_image_written = TRUE;  // Set flag to TRUE so that subsequent images use 'class=container_1'
          }  // End of if-block for (! first_image_written)
          else  {
            // Write HTML for subsequent image files:
            fprintf(fp_HTML, "        <IMG id=\"layer%02d_cost%02d\" class=\"container_1\" border=\"1\" src=\"costMap_layer%02d_%s_cost%02d_%dX.png\" alt=\"\" width=\"%d\" height=\"%d\">\n",
                    pngLayer, i, pngLayer, user_inputs->layer_names[pngLayer], i, user_inputs->viaCostMultiplier[i],
                    image_width, image_height);
          }  // End of else-block
        }  // End of if-block for costUsedOnLayer
      }  // End of for-loop for index 'i' (0 to maxViaCostMultipliers)
    }  // End of if/else-block for (! isViaLayer)
  }  // End of for-loop for index 'pngLayer'

  fprintf(fp_HTML, "      </DIV>\n\n");

  // Close file 'costZones.html' 
  fprintf(fp_HTML, "</HTML>\n");
  fclose(fp_HTML);

}  // End of function 'makeCostMapReport'

