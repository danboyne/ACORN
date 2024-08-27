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
  fprintf(fp_TOC, "      <LI>Input file: <FONT size=\"2\"><A href=\"%s\">%s</A></FONT></LI>\n", base_input_filename, base_input_filename);
  fprintf(fp_TOC, "      <LI><A href=\"preRouting_map.html\">Pre-routing map</A></LI>\n");
  fprintf(fp_TOC, "      <LI><A href=\"designRules.html\">Design rules</A></LI>\n");
  fprintf(fp_TOC, "      <LI><A href=\"costZones.html\">Cost zones</A></LI>\n");
  fprintf(fp_TOC, "    </UL>\n");
  fprintf(fp_TOC, "  </TD>\n");
 
  // Create a blank column to add horizontal spacing:
  fprintf(fp_TOC, "  <TD width=\"200px\">&nbsp;</TD>\n");

  // Write the value of key parameters to the output HTML file:
  fprintf(fp_TOC, "  <TD valign=\"top\">\n");
  fprintf(fp_TOC, "    <FONT size=\"1\" color=\"#B0B0B0\">Key parameters:\n");
  fprintf(fp_TOC, "    <UL>\n");
  fprintf(fp_TOC, "      <LI>grid_resolution: %.2f um</LI>\n", user_inputs->cell_size_um);
  fprintf(fp_TOC, "      <LI>maxIterations: %d</LI>\n", user_inputs->maxIterations);
  fprintf(fp_TOC, "      <LI>violationFreeThreshold: %d</LI>\n", user_inputs->userDRCfreeThreshold);
  fprintf(fp_TOC, "      <LI>DRC_free_threshold: %d</LI>\n", DRC_free_threshold);
  fprintf(fp_TOC, "      <LI>baseVertCostMicrons: %6.1f um</LI>\n", user_inputs->baseVertCostMicrons);
  fprintf(fp_TOC, "      <LI>baseVertCostCells: %'d cells</LI>\n", user_inputs->baseVertCostCells);
  fprintf(fp_TOC, "      <LI>baseVertCost: %'lu</LI>\n", user_inputs->baseVertCost);
  fprintf(fp_TOC, "      <LI>preEvaporationIterations: %d</LI>\n", user_inputs->preEvaporationIterations);
  fprintf(fp_TOC, "      <LI>runsPerPngMap: %d</LI>\n", user_inputs->runsPerPngMap);
  fprintf(fp_TOC, "      <LI>baseCellCost: %'lu</LI>\n", user_inputs->baseCellCost);
  fprintf(fp_TOC, "      <LI>baseDiagCost: %'lu</LI>\n", user_inputs->baseDiagCost);
  fprintf(fp_TOC, "      <LI>baseKnightCost: %'lu</LI>\n", user_inputs->baseKnightCost);
  fprintf(fp_TOC, "    </UL></FONT>\n");
  fprintf(fp_TOC, "  </TD>\n");
  fprintf(fp_TOC, "</TR></TABLE>\n\n");


  // Write the value of key parameters to STDOUT:
  printf("\n\nKey parameters:\n");
  printf("---------------\n");
  printf("  grid_resolution = %.2f\n", user_inputs->cell_size_um);
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
// Name: makeCompositePngPathMap
// Desc: Create a single PNG file that overlays all the routing and via layers
//       into a single image with the same height and width as the original
//       PNG images. Images are skipped if their entry is FALSE in the array
//       user_inputs->include_layer_in_composite_images.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_makeCompositePngPathMap' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_makeCompositePngPathMap 1
#undef DEBUG_makeCompositePngPathMap

int makeCompositePngPathMap(char *compositeFileName, const MapInfo_t *mapInfo,
                            const InputValues_t *user_inputs, CellInfo_t ***cellInfo,
                            unsigned char ***pathTerminals, char *title)  {

  #ifdef DEBUG_makeCompositePngPathMap
  // DEBUG code follows:
  printf("\nDEBUG: Entered function makeCompositePngPathMap\n\n");
  #endif

  // Adjust magnification of map so it takes up most of the width of a monitor:
  int mag = 1; // Default (and minimum) magnification factor for PNG files
  if (mag * mapInfo->mapWidth < 1000)
    mag = 1000 / mapInfo->mapWidth;

  int returnCode = 0;  // Return-code to indicate errors from this function

  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep row = NULL;

  // Allocate memory for one row (4 bytes per pixel: red, green, blue, and alpha)
  row = (png_bytep) malloc(4 * mapInfo->mapWidth * mag * sizeof(png_byte));

  // Open file for writing (binary mode)
  fp = fopen(compositeFileName, "wb");
  if (fp == NULL) {
    fprintf(stderr, "\nERROR: Could not open PNG composite file '%s' for writing.\n\n", compositeFileName);
    returnCode = 1;
    goto finalize_composite;
  }

  // Initialize write structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fprintf(stderr, "\nERROR: Could not allocate memory for PNG write struct for thumbnail image.\n\n");
    returnCode = 1;
    goto finalize_composite;
  }

  // Initialize info structure
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fprintf(stderr, "\nERROR: Could not allocate memory for PNG info struct for thumbnail image.\n\n");
    returnCode = 1;
    goto finalize_composite;
  }

  // Setup Exception handling
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "\nERROR during PNG creation for thumbnail image.\n\n");
    returnCode = 1;
    goto finalize_composite;
  }

  png_init_io(png_ptr, fp);


  // Write header (8 bit colour depth).  The 'IHDR' chunk is the first chunk in a PNG file. It
  // contains the image's width, height, color type and bit depth.
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
  #ifdef DEBUG_makeCompositePngPathMap
  printf("DEBUG: About to raster over all cells in the composite file: %d cells X %d cells X %d image-layers.\n",
         mapInfo->mapWidth, mapInfo->mapHeight, 2 * mapInfo->numLayers - 1);
  #endif
  for (int y = mapInfo->mapHeight - 1; y >= 0; y--) {
    for (int x = 0; x < mapInfo->mapWidth; x++) {

      // Variables for specifying color of each pixel from each layer:
      int red_map, green_map, blue_map, opacity_map;

      // Determine the color and opacity at the (x, y) coordinate of the back-most layer,
      // i.e., the layer with the highest layer index number:
      get_RGBA_values_for_pixel(x, y, 2 * mapInfo->numLayers - 2, (2 * mapInfo->numLayers - 2)/2,
                                (2 * mapInfo->numLayers - 2) % 2, cellInfo, mapInfo, pathTerminals,
                                &red_map, &green_map, &blue_map, &opacity_map);

      //
      // Define RGBA variables for the composite pixel, i.e., after merging the pixels at (x,y)
      // from each layer. We initialize the values to those from the back-most layer, i.e.,
      // the PNG layer with the largest index:
      //
      float red_composite   = (float)red_map;              // Allowed range: 0.0 to 255.0
      float green_composite = (float)green_map;            // Allowed range: 0.0 to 255.0
      float blue_composite  = (float)blue_map;             // Allowed range: 0.0 to 255.0
      float alpha_composite = (float)opacity_map / 255.0;  // Allowed range: 0.0 to 1.0

      //
      // Iterate over the trace and via layers at the current X/Y pixel, starting from
      // the second-to-largest layer number, which is the layer 'behind' all layers except
      // the back-most layer:
      //
      for (int traceViaLayer = 2 * mapInfo->numLayers - 3; traceViaLayer >= 0; traceViaLayer--)  {

        //
        // Skip this layer if the user-supplied array-element 'include_layer_in_composite_images' is FALSE:
        //
        if (! user_inputs->include_layer_in_composite_images[traceViaLayer])  {
          #ifdef DEBUG_makeCompositePngPathMap
          printf("DEBUG: Skipping pixel (%d,%d,%d) in makeCompositePngPathMap because user wants to skip this layer...\n", x, y, traceViaLayer);
          #endif

          continue;  // Move on to next layer
        }
        else  {
          #ifdef DEBUG_makeCompositePngPathMap
          printf("DEBUG: Analyzing pixel (%d,%d,%d) in makeCompositePngPathMap...\n", x, y, traceViaLayer);
          #endif
        }

        int mapLayer   = traceViaLayer / 2;  // mapLayer is the z-value to be used in the cellInfo[x][y][z] matrix
        int isViaLayer = traceViaLayer % 2;  // (traceViaLayer % 2) = TRUE if layer is a via layer (FALSE if routing layer)

        // Determine the color and opacity at the (x, y, traceViaLayer) coordinate:
        get_RGBA_values_for_pixel(x, y, traceViaLayer, mapLayer, isViaLayer, cellInfo, mapInfo, pathTerminals,
                                  &red_map, &green_map, &blue_map, &opacity_map);

        #ifdef DEBUG_makeCompositePngPathMap
        printf("DEBUG: Pixel data from (%d,%d) on PNG layer %d is RGBA = (%d,%d,%d,%d) in makeCompositePngPathMap.\n", x, y, traceViaLayer,
               red_map, green_map, blue_map, opacity_map);
        #endif

        // Normalize the alpha to 1.0 for the current layer:
        float current_layer_alpha = (float)opacity_map / 255.0;

        //
        // Use the 'pre-multiplied compositing' methodology for overlaying multiple PNG pixels,
        // based on Wikipedia explanation located at the following URL (as of July 2024):
        //            https://en.wikipedia.org/wiki/Alpha_compositing
        //
        alpha_composite = current_layer_alpha   +   alpha_composite * (1.0 - current_layer_alpha);
        red_composite   = red_map               +   red_composite   * (1.0 - current_layer_alpha);
        green_composite = green_map             +   green_composite * (1.0 - current_layer_alpha);
        blue_composite  = blue_map              +   blue_composite  * (1.0 - current_layer_alpha);


        #ifdef DEBUG_makeCompositePngPathMap
        printf("DEBUG: After analyzing pixel (%d,%d) on PNG layer %d, RGBA = (%.2f, %.2f, %.2f, %.2f) in makeCompositePngPathMap.\n",
               x, y, traceViaLayer, red_composite, green_composite, blue_composite, alpha_composite * 255);
        #endif

      }  // End of for-loop for index 'traceViaLayer'


      #ifdef DEBUG_makeCompositePngPathMap
      printf("\nDEBUG: For composite pixel (%d,%d):\n", x, y);
      printf(  "DEBUG:            red_composite = %.2f\n", red_composite);
      printf(  "DEBUG:          green_composite = %.2f\n", green_composite);
      printf(  "DEBUG:           blue_composite = %.2f\n", blue_composite);
      printf(  "DEBUG:          alpha_composite = %.2f\n", alpha_composite * 255.0);
      printf(  "DEBUG:   RGBA = (%d,%d,%d,%d)\n\n", (int)red_composite, (int)green_composite, (int)blue_composite, (int)(alpha_composite * 255.0));
      #endif


      // Account for the magnification factor 'mag' in the x-direction using the 'repeat_x' variable:
      for (int repeat_x = 0; repeat_x < mag; repeat_x++)  {

        //
        // Write the RGBA data to the pixel:
        //
        setRGBA(&(row[((x * mag) + repeat_x) * 4]), (int)red_composite, (int)green_composite, (int)blue_composite, (int)(alpha_composite * 255.0));
      }  // end of repeat_x for-loop

    }  // End of for-loop for index 'x'

    // Account for the magnification factor 'mag' in the y-direction:
    for (int repeat_y = 0; repeat_y < mag; repeat_y++)  {
      png_write_row(png_ptr, row);
    }  // End of repeat_y for-loop

  }  // End of for-loop for index 'y'

  // End write
  png_write_end(png_ptr, NULL);

  finalize_composite:
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

  return(returnCode);
}  // End of function 'makeCompositePngPathMap'


//-----------------------------------------------------------------------------
// Name: makePngPathMaps
// Desc: Create PNG files for routing and via layers to display the paths of
//       each routed net. Maps will be magnified by zoom factor 'mag'
//       (mag = 1 or larger integer). Also create a composite map in a
//       single PNG file.
//-----------------------------------------------------------------------------
static int makePngPathMaps(int mag, int numPngLayers, char *pngPathFileNames[], char *compositeFileName,
                           const MapInfo_t *mapInfo, const InputValues_t *user_inputs, CellInfo_t ***cellInfo, char *title)  {

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

    // Write header (8 bit colour depth).  The 'IHDR' chunk is the first chunk in a PNG file. It
    // contains the image's width, height, color type and bit depth.
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


  // Create a single PNG file that overlays this iteration's routing/via layers. Layers with a
  // 'FALSE' in array user_inputs->include_layer_in_composite_images will be excluded from the composite:
  // printf("DEBUG: About to call makeCompositePngPathMap from makePngPathMaps after creating PNG path maps for all layers.\n");
  makeCompositePngPathMap(compositeFileName, mapInfo, user_inputs, cellInfo, pathTerminals, "Title");


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
  // Generate name of composite PNG file, which overlays the routing of all routing/via layers:
  //
  char *compositeFileName;
  compositeFileName = malloc (300 * sizeof(char));
  sprintf(compositeFileName, "map_composite_iter%04d.png", iteration);


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
  returnCode = makePngPathMaps(mag, numPngLayers, pngPathFileNames, compositeFileName, mapInfo, user_inputs, cellInfo, title);
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
  free(compositeFileName); compositeFileName = NULL;
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

  return(returnCode);

}  // End of function 'makeHtmlIterationSummary'


//-----------------------------------------------------------------------------
// Name: updateHtmlTableOfContents
// Desc: Update the HTML table-of-contents file with the results of
//       iteration # 'pathFinderRun', including the generation of PNG map-files
//       and a new HTML file to display these PNG files.
//-----------------------------------------------------------------------------
void updateHtmlTableOfContents(FILE *fp_TOC, MapInfo_t *mapInfo, CellInfo_t ***cellInfo,
                               InputValues_t *user_inputs, RoutingMetrics_t *routability,
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
  if ((mapInfo->current_iteration < 2)
      || ((*user_inputs).runsPerPngMap * (mapInfo->current_iteration / (*user_inputs).runsPerPngMap) == mapInfo->current_iteration)
      || (routability->num_nonPseudo_DRC_cells == 0))  {

    // Print a time-stamp to STDOUT:
    tim = time(NULL);
    now = localtime(&tim);
    printf("Date-stamp before generating PNG maps: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
          now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


    // Generate PNG map:
//  makeHtmlIterationSummary(mapInfo->current_iteration, mapInfo, cellInfo, user_inputs, routability,
//                 "Title", DRC_details, shapeTypeNames);
    makeHtmlIterationSummary(mapInfo->current_iteration, mapInfo, cellInfo, user_inputs, routability,
                   "Title", shapeTypeNames);

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
    if (routability->iteration_cumulative_time[mapInfo->current_iteration] - routability->iteration_cumulative_time[mapInfo->current_iteration - 1] > 1)
      fprintf(fp_TOC, "%'d seconds).</FONT>\n", routability->iteration_cumulative_time[mapInfo->current_iteration] - routability->iteration_cumulative_time[mapInfo->current_iteration - 1]);
    else if (routability->iteration_cumulative_time[mapInfo->current_iteration] - routability->iteration_cumulative_time[mapInfo->current_iteration - 1] == 1)
      fprintf(fp_TOC, "~%'d second).</FONT>\n", routability->iteration_cumulative_time[mapInfo->current_iteration] - routability->iteration_cumulative_time[mapInfo->current_iteration - 1]);
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
                user_inputs->layer_names[2 * routability->DRC_details[mapInfo->current_iteration][DRC_index].z], // Layer name of DRC location
                routability->DRC_details[mapInfo->current_iteration][DRC_index].x * user_inputs->cell_size_um,   // X-coordinate (in um) of DRC location
                routability->DRC_details[mapInfo->current_iteration][DRC_index].y * user_inputs->cell_size_um,   // Y-coordinate (in um) of DRC location
                shapeTypeNames[routability->DRC_details[mapInfo->current_iteration][DRC_index].shapeType],       // Shape-type name at location of DRC violation
                user_inputs->net_name[routability->DRC_details[mapInfo->current_iteration][DRC_index].pathNum],  // Net name at location of DRC violation
                shapeTypeNames[routability->DRC_details[mapInfo->current_iteration][DRC_index].offendingShapeType],      // Shape-type name of offending shape
                user_inputs->net_name[routability->DRC_details[mapInfo->current_iteration][DRC_index].offendingPathNum], // Net name of offending net
                routability->DRC_details[mapInfo->current_iteration][DRC_index].minimumAllowedSpacing,           // Minimum allowed spacing from edge to edge (in um)
                routability->DRC_details[mapInfo->current_iteration][DRC_index].minimumAllowedDistance);         // Minimum allowed distance from edge to centerline (in um)

      }  // End of for-loop for index 'DRC_index'

      fprintf(fp_TOC, " </UL>\n");

    }  // End of if-block for (num_nonPseudo_DRC_cells > 0) && (num_nonPseudo_DRC_cells < maxRecordedDRCs)

  }  // End of if-block for printing out a PNG map file
  else  {
    // For iterations in which a PNG map is not generated, add a simple text summary to the
    // table-of-contents HTML file for this run's metrics:
    fprintf(fp_TOC, "  <LI><FONT color=\"blue\">Iteration %d:&nbsp;</FONT>", mapInfo->current_iteration );
    fprintf(fp_TOC, "<FONT color=\"#B0B0B0\">%'d cells with DRCs, trace length is %'.4f mm with %d vias. %d/%d nets have DRCs. (%'lu cells explored in %'d seconds).</FONT>\n",
                     routability->num_nonPseudo_DRC_cells, routability->total_lateral_nonPseudo_length_mm, routability->total_nonPseudo_vias,
                     routability->num_paths_with_DRCs, routability->num_DRCfree_paths + routability->num_paths_with_DRCs,
                     routability->iteration_explored_cells[mapInfo->current_iteration],
                     routability->iteration_cumulative_time[mapInfo->current_iteration] - routability->iteration_cumulative_time[mapInfo->current_iteration - 1]);

    // If the number of crossings is maxRecordedDRCs (typically 10) or less, then also 
    // display the names of the nets involved in the crossings.
    if ((routability->num_nonPseudo_DRC_cells > 0) && (routability->num_nonPseudo_DRC_cells <= maxRecordedDRCs))  {
      fprintf(fp_TOC, " <input type=\"button\" onclick=\"return toggleMe('showHide%d')\" value=\"Display/hide DRC info\" style=\"height:15px; width:130px; font-family: sans-serif; font-size: 10px;\"><BR>\n",
              mapInfo->current_iteration);
      fprintf(fp_TOC, " <UL id=\"showHide%d\" style=\"display:none\">", mapInfo->current_iteration);

      for (int DRC_index = 0; DRC_index < routability->num_nonPseudo_DRC_cells; DRC_index++)  {
        // printf("DEBUG: Printing DRC #%d to HTML file...\n", DRC_index);
        fprintf(fp_TOC, "  <LI>DRC on layer %s at location (%.0f, %.0f) microns between %s of net %s and the center of a %s in net %s (min spacing = %.2f; min dist = %.2f microns).\n",
           user_inputs->layer_names[2 * routability->DRC_details[mapInfo->current_iteration][DRC_index].z], // Layer name of DRC location
           routability->DRC_details[mapInfo->current_iteration][DRC_index].x * user_inputs->cell_size_um,   // X-coordinate (in um) of DRC location
           routability->DRC_details[mapInfo->current_iteration][DRC_index].y * user_inputs->cell_size_um,   // Y-coordinate (in um) of DRC location
           shapeTypeNames[routability->DRC_details[mapInfo->current_iteration][DRC_index].shapeType],       // Shape-type name at location of DRC violation
           user_inputs->net_name[routability->DRC_details[mapInfo->current_iteration][DRC_index].pathNum],  // Net name at location of DRC violation
           shapeTypeNames[routability->DRC_details[mapInfo->current_iteration][DRC_index].offendingShapeType],      // Shape-type name of offending shape
           user_inputs->net_name[routability->DRC_details[mapInfo->current_iteration][DRC_index].offendingPathNum], // Net name of offending net
           routability->DRC_details[mapInfo->current_iteration][DRC_index].minimumAllowedSpacing,           // Minimum allowed spacing from edge to edge (in um)
           routability->DRC_details[mapInfo->current_iteration][DRC_index].minimumAllowedDistance);         // Minimum allowed distance from edge to centerline (in um)

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


//-----------------------------------------------------------------------------
// Name: createRoutingMetricsGraph
// Desc: Create PNG file that plots routing metrics versus the iteration count.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_createRoutingMetricsGraph' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_createRoutingMetricsGraph 1
#undef DEBUG_createRoutingMetricsGraph

void createRoutingMetricsGraph(const char *input_text_filename, const char *output_PNG_filename,
                               const MapInfo_t *mapInfo, const RoutingMetrics_t *routability,
                               InputValues_t *user_inputs, int adequateSolutionFound,
                               int start_iteration, int end_iteration)  {

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Entered function createRoutingMetricsGraph with:\n");
  printf("DEBUG:      start_iteration = %d       end_iteration = %d\n", start_iteration, end_iteration);
  printf("DEBUG:      input_text_filename = '%s'\n", input_text_filename);
  #endif

  // If 'end_iteration' is zero, return without creating any graph:
  if (end_iteration == 0)  {
    return;
  }

  // Do some error-checking on 'start_iteration' and 'end_iteration':
  if (start_iteration > mapInfo->current_iteration)  {
    start_iteration = mapInfo->current_iteration;
  }
  if (end_iteration > mapInfo->current_iteration)  {
    end_iteration = mapInfo->current_iteration;
  }
  if (start_iteration > end_iteration)  {
    end_iteration = start_iteration;
  }

  // Calculate the number of iterations to be plotted:
  int num_iterations = end_iteration - start_iteration + 1;


  // Delete the file if a previous version already exists:
  remove(output_PNG_filename);

  FILE *PNG_out;  // Pointer to output PNG file
  gdImagePtr PNG_image;  //

  // Assign constants that define dimensions of the map and the enclosed graph:
  const int x_image_size_pixels = 900;
  const int y_image_size_pixels = 650;
  const int image_border_offset = 2;  // Distance from edge of image where border is located
  const int graph_offset_from_top_pixels    =  75;  // Distance between graph and top of image
  const int graph_offset_from_bottom_pixels = 100;  // Distance between graph and bottom of image
  const int graph_offset_from_left_pixels   =  75;  // Distance between graph and left of image
  const int graph_offset_from_right_pixels  =  75;  // Distance between graph and right of image

  // Define the horizontal and vertical extents of the graph region:
  const int graph_x_size_pixels = x_image_size_pixels - graph_offset_from_left_pixels - graph_offset_from_right_pixels;
  const int graph_y_size_pixels = y_image_size_pixels - graph_offset_from_top_pixels  - graph_offset_from_bottom_pixels;

  // Define coordinates of the left, right, top, and bottom of graph region:
  const int graph_leftSide_pixels   = graph_offset_from_left_pixels;
  const int graph_rightSide_pixels  = x_image_size_pixels - graph_offset_from_right_pixels;
  const int graph_topSide_pixels    = graph_offset_from_top_pixels;
  const int graph_bottomSide_pixels = y_image_size_pixels - graph_offset_from_bottom_pixels;

  // Define length of major and minor tick-marks:
  const int major_tick_length_pixels = 10;
  const int minor_tick_length_pixels =  5;

  // Define spacing between tick-marks and numeric axis labels:
  const int tickMark_to_labelSpacing_pixels = 3;

  // Define size of standard data points. 'Special' data points might be scaled larger than this:
  const int dataPoint_size_pixels = 2;

  // Define coordinates and sizes associated with the legend on left side of graph:
  const int left_legend_width_pixels  = x_image_size_pixels / 3;
  const int left_legend_height_pixels = 45;
  const int left_legend_lowerLeft_X_pixels = graph_offset_from_left_pixels / 10;
  const int left_legend_lowerLeft_Y_pixels = y_image_size_pixels - graph_offset_from_bottom_pixels / 15;

  // Define coordinates and sizes associated with the legend on right side of graph:
  const int right_legend_width_pixels  = x_image_size_pixels / 3;
  const int right_legend_height_pixels = 45;
  const int right_legend_lowerLeft_X_pixels = x_image_size_pixels - right_legend_width_pixels - graph_offset_from_right_pixels / 10;
  const int right_legend_lowerLeft_Y_pixels = y_image_size_pixels - graph_offset_from_bottom_pixels / 15;


  //
  // Create the image:
  //
  PNG_image = gdImageCreate(x_image_size_pixels, y_image_size_pixels);

  // Allocate colors. The first allocated color will be the background color:
  int white        = gdImageColorAllocate(PNG_image, 255, 255, 255);
  int black        = gdImageColorAllocate(PNG_image,   0,   0,   0);
  int light_grey   = gdImageColorAllocate(PNG_image, 192, 192, 192);
  int grey         = gdImageColorAllocate(PNG_image, 128, 128, 128);
  int red          = gdImageColorAllocate(PNG_image, 255,   0,   0);
  int green        = gdImageColorAllocate(PNG_image,   0, 255,   0);
  int blue         = gdImageColorAllocate(PNG_image,   0,   0, 255);
  int lime_green   = gdImageColorAllocate(PNG_image, 118, 253,  79);
  int aqua         = gdImageColorAllocate(PNG_image, 194, 251, 249);
  int burnt_orange = gdImageColorAllocate(PNG_image, 253, 198,  79);
  int light_red    = gdImageColorAllocate(PNG_image, 252, 109, 109);

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Created PNG_image and allocated colors in function createRoutingMetricsGraph.\n");
  #endif

  // Get pointers to fonts:
  gdFont *tinyFont_ptr;
  gdFont *medFont_ptr;
  gdFont *giantFont_ptr;
  tinyFont_ptr  = gdFontGetTiny();
  medFont_ptr   = gdFontGetMediumBold();
  giantFont_ptr = gdFontGetGiant();

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Got pointers to fonts in function createRoutingMetricsGraph.\n");
  #endif

  //
  // Determine the minimum and maximum values for the y-values that will be plotted,
  // which are the routing cost and the number of design-rule violations (DRCs):
  //
  unsigned int min_DRC_cells = UINT_MAX;
  unsigned int max_DRC_cells = 0;
  unsigned long min_cost = ULONG_MAX;
  unsigned long max_cost = 0;
  for (int iteration = start_iteration; iteration <= end_iteration; iteration++)  {
    if (routability->nonPseudo_num_DRC_cells[iteration] < min_DRC_cells)  {
      min_DRC_cells = routability->nonPseudo_num_DRC_cells[iteration];
    }

    if (routability->nonPseudo_num_DRC_cells[iteration] > max_DRC_cells)  {
      max_DRC_cells = routability->nonPseudo_num_DRC_cells[iteration];
    }

    if (routability->nonPseudoPathCosts[iteration] < min_cost)  {
      min_cost = routability->nonPseudoPathCosts[iteration];
    }

    if (routability->nonPseudoPathCosts[iteration] > max_cost)  {
      max_cost = routability->nonPseudoPathCosts[iteration];
    }
  }  // End of for-loop for index 'iteration'

  // Define a scaling factor between the Acorn-supplied cost and the effective
  // length, in meters, that this cost is equivalent to:
  float cost_per_meter = 10 * pow(2.0, NON_PIN_SWAP_EXPONENT) * 1.0E6 / user_inputs->cell_size_um;

  // Define scaling factors between Acorn data and the pixel coordinate space, in addition
  // to the minimum value in the plotting space for the two y-axes:
  //
  // Determine the x-axis scaling such that iteration #(start_iteration - 1) is at the
  // origin and iteration #(end_iteration + 1) is at the right-hand axis:
  float x_axis_pixels_per_iteration = (float) graph_x_size_pixels / (num_iterations + 1);

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Determined min/max y-values in createRoutingMetricsGraph:\n");
  printf("DEBUG:   min_DRC_cells = %d          max_DRC_cells = %d\n", min_DRC_cells, max_DRC_cells);
  printf("DEBUG:   min_cost      = %lu         max_cost = %lu\n", min_cost, max_cost);
  printf("DEBUG:   cost_per_meter = %f     x_axis_pixels_per_iteration = %f\n", cost_per_meter, x_axis_pixels_per_iteration);
  #endif

  { //
    // This block writes a title for the graph:
    //
      char text_label[100];
      strcpy(text_label, "Routing Metrics");

      // Estimate the width of the title so it can be positioned correctly. The width of 'giantFont'
      // font is 9 pixels (15 pixels tall), per https://libgd.github.io/manuals/2.3.3/files/gdfontg-c.html.
      int text_width_pixels = 9 * strlen(text_label);

      // Write the title of the graph: "Routing Metrics":
      gdImageString(PNG_image, giantFont_ptr, (x_image_size_pixels - text_width_pixels) / 2,
                    graph_topSide_pixels * 0.3,
                    (unsigned char*) text_label, black);

      // Write the name of the test-case beneath the "Routing Metrics" title:
      strcpy(text_label, input_text_filename);
      // Estimate the width of the input filename so it can be positioned correctly. The width of 'tinyFont'
      // font is 5 pixels, per https://libgd.github.io/manuals/2.3.3/files/gdfontg-c.html.
      text_width_pixels = 5 * strlen(text_label);
      gdImageString(PNG_image, tinyFont_ptr, (x_image_size_pixels - text_width_pixels) / 2,
                    graph_topSide_pixels * 0.3 + 18,
                    (unsigned char*) text_label, black);

  }  // End of block for writing a title to the graph

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Finished writing a title to the graph in function createRoutingMetricsGraph.\n");
  #endif


  { // This block creates vertical lines and background colors for iterations at which the routing algorithm
    // was modified:

    // If the congestion sensitivities are increasing linearly from 20% to 100% anywhere
    // between iterations #start_iteration and #end_iteration, then create a light blue
    // background where the congestion sensitivities are increasing:
    if (   (end_iteration   >=     mapInfo->time_constant_iterations)
        && (start_iteration <= 5 * mapInfo->time_constant_iterations))  {

      int box_min_y_pixels = graph_bottomSide_pixels;
      int box_max_y_pixels = graph_topSide_pixels;
      int box_min_x_pixels = graph_leftSide_pixels
                                 + x_axis_pixels_per_iteration * (max(start_iteration, mapInfo->time_constant_iterations) - start_iteration + 1);
      int box_max_x_pixels = graph_leftSide_pixels
                                 + x_axis_pixels_per_iteration * (min(end_iteration, 5 * mapInfo->time_constant_iterations) - start_iteration + 1);

      // Draw the light-blue rectangle using the 4 points calculated above:
      gdImageFilledRectangle(PNG_image,
                             box_min_x_pixels, box_min_y_pixels,
                             box_max_x_pixels, box_max_y_pixels,
                             aqua);

      // Label the lower-left corner of the light-blue rectangle with 'Congestion sensitivity increasing from x%',
      // where x is the percentage at the iteration where the blue box starts (between 20% and 100%):
      char blue_box_label[60];
      int initial_percentage = 100 * max(start_iteration, mapInfo->time_constant_iterations)
                                                        / (5 * mapInfo->time_constant_iterations);
      if (start_iteration < mapInfo->time_constant_iterations)  {
        sprintf(blue_box_label, "Congestion sensitivity starts increasing from %d%%", initial_percentage); // Should print '20%'
      }
      else  {
        sprintf(blue_box_label, "Congestion sensitivity increasing from %d%%", initial_percentage);  // Should print value greater than 20%
      }
      gdImageStringUp(PNG_image, tinyFont_ptr, box_min_x_pixels - 8, box_min_y_pixels - major_tick_length_pixels,
                      (unsigned char*) blue_box_label, black);

      // Label the lower-right corner of the light-blue rectangle with 'Congestion sensitivity reached x%',
      // where x is the percentage that was reached at the iteration where the blue box ended (<= 100%):
      int final_percentage = 100 * min(end_iteration, 5 * mapInfo->time_constant_iterations)
                                                  / ( 5 * mapInfo->time_constant_iterations);
      sprintf(blue_box_label, "Congestion sensitivity reached %d%%", final_percentage);
      gdImageStringUp(PNG_image, tinyFont_ptr, max(box_min_x_pixels, box_max_x_pixels - 8), box_min_y_pixels - major_tick_length_pixels,
                      (unsigned char*) blue_box_label, black);

    }  // End of if-block for start_iteration or end_iteration falling between time_constant_iterations and 5*time_constant_iterations

    //
    // Iterate over the iterations that have HTML-encoded messages and draw a vertical
    // line wherever there is a 'HTML_message_categories' value that is not 'NO_ANNOTATION'.
    //
    #ifdef DEBUG_createRoutingMetricsGraph
    printf("\nDEBUG:In createRoutingMetricsGraph at iteration %d, %d HTML-encoded messages were found:\n",
            mapInfo->current_iteration, routability->num_HTML_messages);
    #endif

    for (int HTML_message_index = 0; HTML_message_index < routability->num_HTML_messages; HTML_message_index++)  {

      #ifdef DEBUG_createRoutingMetricsGraph
      printf("DEBUG:   HTML message %d for iteration %d and category %d.\n", HTML_message_index,
             routability->HTML_message_iter_nums[HTML_message_index], routability->HTML_message_categories[HTML_message_index]);
      #endif

      // Check whether the HTML-encoded message contains a category different from 'NO_ANNOTATION':
      if (routability->HTML_message_categories[HTML_message_index] != NO_ANNOTATION)  {

        // We got here, so a vertical line should be drawn in the graph. Calculate the x-axis value for
        // the vertical line, based on the 'HTML_message_iter_nums' value for this HTML-encoded message:
        int x_pixels = graph_leftSide_pixels
                         +   x_axis_pixels_per_iteration * (routability->HTML_message_iter_nums[HTML_message_index] - start_iteration + 1) ;

        int line_color;
        // Based on the category of the HTML-encoded message, select a color:
        if (routability->HTML_message_categories[HTML_message_index] == SWAP_TERMS)  {
          line_color = lime_green;
        }
        else if (   (routability->HTML_message_categories[HTML_message_index] == TR_CONG_SENS_UP)
                 || (routability->HTML_message_categories[HTML_message_index] == VIA_CONG_SENS_UP))  {
          line_color = aqua;
        }
        else if (   (routability->HTML_message_categories[HTML_message_index] == TR_CONG_SENS_DOWN)
                 || (routability->HTML_message_categories[HTML_message_index] == VIA_CONG_SENS_DOWN))  {
          line_color = burnt_orange;
        }
        else if (routability->HTML_message_categories[HTML_message_index] == TR_CONG_SENS_DOWN)  {
          line_color = light_red;
        }
        else  {
          line_color = light_grey;  // We should never get here, but assign light grey as the default.
        }

        #ifdef DEBUG_createRoutingMetricsGraph
        printf("DEBUG:       Drawing a vertical line at x_pixels = %d and color = %d\n", x_pixels, line_color);
        #endif

        // Draw a light-grey vertical line:
        gdImageLine(PNG_image, x_pixels, graph_bottomSide_pixels,
                               x_pixels, graph_topSide_pixels, line_color);

      }  // End of if-block for HTML_message_categories != 'NO_ANNOTATION'
    }  // End of for-loop for index 'HTML_message_index'

  }  // End of block for creating background graphics to indicate changes in the routing algorithm

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Finished creating background graphics in function createRoutingMetricsGraph.\n");
  #endif

  float y_axis_pixels_per_cost      = 1.0;
  float y_axis_pixels_per_DRC       = 1.0;
  long int graph_min_cost = 0;
  int graph_min_DRCs = 0;

  // 'DRC_axis_is_linear' is a Boolean flag to indicate whether the right-hand vertical axis
  // is linear (TRUE) or logarithmic (FALSE):
  unsigned char DRC_axis_is_linear = TRUE;


  {// Block for drawing graph's outline and legends.

    // Draw a short white line to avoid compile-time warnings for unused variable 'white':
    gdImageLine(PNG_image, 0, 0, 0, 1, white);

    // Draw a black rectangle to serve as a border for the image:
    gdImageRectangle(PNG_image, image_border_offset - 1, image_border_offset - 1,
                     x_image_size_pixels - image_border_offset, y_image_size_pixels - image_border_offset, black);

    // Draw a grey rectangle to define the border of the graph. The sides of the graph are located
    // 75 pixels from the bottom, left, and right edges of the image, and 50 pixels from the top.
    gdImageRectangle(PNG_image, graph_offset_from_left_pixels, graph_offset_from_top_pixels,
                     graph_rightSide_pixels, graph_bottomSide_pixels, light_grey);

    // Draw a black horizontal axis (x-axis) along the bottom:
    gdImageLine(PNG_image, graph_leftSide_pixels, graph_bottomSide_pixels,
                graph_rightSide_pixels, graph_bottomSide_pixels, black);

    // Draw a black vertical axis (y-axis) along the left side of the graph:
    gdImageLine(PNG_image, graph_leftSide_pixels, graph_bottomSide_pixels,
                graph_leftSide_pixels, graph_topSide_pixels, black);

    // Draw a red vertical axis (y-axis) along the right side of the graph:
    gdImageLine(PNG_image, graph_rightSide_pixels, graph_bottomSide_pixels,
                graph_rightSide_pixels, graph_topSide_pixels, red);

    //
    // Legend on left starts here:
    //
    // Draw a grey rectangular box for the left legend:
    gdImageRectangle(PNG_image, left_legend_lowerLeft_X_pixels, left_legend_lowerLeft_Y_pixels,
                                left_legend_lowerLeft_X_pixels + left_legend_width_pixels,
                                left_legend_lowerLeft_Y_pixels - left_legend_height_pixels, grey);

    // Write 'Routing Cost:' in upper-left corner of left legend box:
    char text_string[80];
    int string_length_pixels;
    sprintf(text_string, "Routing Cost:");
    gdImageString(PNG_image, tinyFont_ptr, left_legend_lowerLeft_X_pixels + 3,
                                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 2,
                                           (unsigned char*)text_string, black);

    // Write 'With violations:' below 'Routing Cost:' line:
    sprintf(text_string, "With violations:");
    // Calculate the width of the string, in pixels, assuming each character is 5 pixels wide and 8 pixels high.
    // This is based on following libgd website: https://libgd.github.io/manuals/2.3.3/files/gdfontt-c.html
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 - string_length_pixels,
                                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 12,
                                           (unsigned char*)text_string, black);

    // Beside 'With violations:', draw a black horizontal line:
    gdImageLine(PNG_image, left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 5,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 16,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 35,    // Line is 30 pixels long
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 16, black);

    // Beside 'With violations:', draw a single black, rectangular data point through the black horizontal line:
    gdImageFilledRectangle(PNG_image,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 20 - dataPoint_size_pixels/2,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 16 - dataPoint_size_pixels/2,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 20 + dataPoint_size_pixels/2,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 16 + dataPoint_size_pixels/2,
                           black);

    // Write 'Without violations:' below 'With violations:' line:
    sprintf(text_string, "Without violations:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 - string_length_pixels,
                                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 23,
                                           (unsigned char*)text_string, black);

    // Beside 'Without violations:', draw a black horizontal line:
    gdImageLine(PNG_image, left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 5,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 27,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 35,    // Line is 30 pixels long
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 27, black);

    // Beside 'Without violations:', draw a single large blue, rectangular data point through the black horizontal line:
    gdImageFilledRectangle(PNG_image,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 20 - dataPoint_size_pixels,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 27 - dataPoint_size_pixels,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 20 + dataPoint_size_pixels,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 27 + dataPoint_size_pixels,
                           blue);

    // Write 'Lowest cost:' below 'Without violations:' line:
    sprintf(text_string, "Lowest cost:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 - string_length_pixels,
                                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 34,
                                           (unsigned char*)text_string, black);

    // Beside 'Lowest cost:', draw a black horizontal line:
    gdImageLine(PNG_image, left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 5,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 38,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 35,    // Line is 30 pixels long
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 38, black);

    // Beside 'Lowest cost:', draw a single black, rectangular data point through the black line:
    gdImageFilledRectangle(PNG_image,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 20 - dataPoint_size_pixels/2,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 38 - dataPoint_size_pixels/2,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 20 + dataPoint_size_pixels/2,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 38 + dataPoint_size_pixels/2,
                           black);

    // Beside 'Lowest cost:', draw a blue circle around the black, rectangular data point:
    gdImageArc(PNG_image, left_legend_lowerLeft_X_pixels + left_legend_width_pixels/3 + 20,
                          left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 38,
                          10, 10, 0, 360, blue);

    // Write 'Design-rule Violations:' in upper-right corner of legend box:
    sprintf(text_string, "Design-rule Violations:");
    gdImageString(PNG_image, tinyFont_ptr, left_legend_lowerLeft_X_pixels + left_legend_width_pixels * 0.6,
                                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 2,
                                           (unsigned char*)text_string, red);

    // Write 'Violations:' below 'Design-rule Violations:' line:
    sprintf(text_string, "Violations:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, left_legend_lowerLeft_X_pixels + left_legend_width_pixels - string_length_pixels - 40,
                                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 16,
                                           (unsigned char*)text_string, red);

    // Beside 'Violations:', draw a red horizontal line:
    gdImageLine(PNG_image, left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 35,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 20,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 5,    // Line is 30 pixels long
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 20, red);

    // Beside 'Violations:', draw a single red, rectangular data point through the red horizontal line:
    gdImageFilledRectangle(PNG_image,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 20 - dataPoint_size_pixels/2,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 20 - dataPoint_size_pixels/2,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 20 + dataPoint_size_pixels/2,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 20 + dataPoint_size_pixels/2,
                           red);

    // Write 'No violations:' below 'Violations:' line:
    sprintf(text_string, "No violations:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, left_legend_lowerLeft_X_pixels + left_legend_width_pixels - string_length_pixels - 40,
                                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 31,
                                           (unsigned char*)text_string, red);

    // Beside 'No violations:', draw a black horizontal line:
    gdImageLine(PNG_image, left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 35,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 35,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 5,    // Line is 30 pixels long
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 35, black);

    // Beside 'No violations:', draw a single large green rectangular data point through the black horizontal line:
    gdImageFilledRectangle(PNG_image,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 20 - dataPoint_size_pixels,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 35 - dataPoint_size_pixels,
                           left_legend_lowerLeft_X_pixels + left_legend_width_pixels - 20 + dataPoint_size_pixels,
                           left_legend_lowerLeft_Y_pixels - left_legend_height_pixels + 35 + dataPoint_size_pixels,
                           green);

    //
    // Legend on right starts here:
    //
    // Draw a grey rectangular box for the right legend:
    gdImageRectangle(PNG_image, right_legend_lowerLeft_X_pixels, right_legend_lowerLeft_Y_pixels,
                                right_legend_lowerLeft_X_pixels + right_legend_width_pixels,
                                right_legend_lowerLeft_Y_pixels - right_legend_height_pixels, grey);

    // Vertically write 'Routing' along left side of legend box:
    sprintf(text_string, "Routing");
    string_length_pixels = strlen(text_string) * 5;
    gdImageStringUp(PNG_image, tinyFont_ptr, right_legend_lowerLeft_X_pixels + 3,
                                             (2 * right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + string_length_pixels)/2,
                                             (unsigned char*)text_string, black);

    // Vertically write 'changes' along left side of legend box, to right of 'Algorithm':
    sprintf(text_string, "changes:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageStringUp(PNG_image, tinyFont_ptr, right_legend_lowerLeft_X_pixels + 13,
                                             (2 * right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + string_length_pixels)/2,
                                             (unsigned char*)text_string, black);

    // Write 'Swap start & end-terminals of selected nets' along top of right legend box:
    sprintf(text_string, "Swap start & end-terminals of selected nets:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, right_legend_lowerLeft_X_pixels + right_legend_width_pixels - string_length_pixels - 43,
                                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 2,
                                           (unsigned char*)text_string, black);

    // Draw two horizontal lime-green lines (double width) to the right of the above text label:
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 5,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 5,
                           lime_green);
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 6,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 6,
                           lime_green);

    // Write 'Increase trace or via congestion sensitivity' below 'Swap start & end-terminals of selected nets':
    sprintf(text_string, "Increase trace or via congestion sensitivity:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, right_legend_lowerLeft_X_pixels + right_legend_width_pixels - string_length_pixels - 43,
                                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 13,
                                           (unsigned char*)text_string, black);

    // Draw three horizontal aqua lines (triple width) to the right of the above text label:
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 16,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  -  4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 16,
                           aqua);
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 17,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  -  4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 17,
                           aqua);
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 18,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  -  4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 18,
                           aqua);

    // Write 'Decrease trace or via congestion sensitivity' below 'Increase trace or via congestion sensitivity':
    sprintf(text_string, "Decrease trace or via congestion sensitivity:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, right_legend_lowerLeft_X_pixels + right_legend_width_pixels - string_length_pixels - 43,
                                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 24,
                                           (unsigned char*)text_string, black);

    // Draw two horizontal burnt-orange lines (double width) to the right of the above text label:
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 27,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  -  4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 27,
                           burnt_orange);
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 28,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  -  4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 28,
                           burnt_orange);

    // Write 'Add pseudo-congestion near crowded pseudo-vias' below 'Decrease trace or via congestion sensitivity':
    sprintf(text_string, "Add pseudo-congestion at crowded pseudo-vias:");
    string_length_pixels = strlen(text_string) * 5;
    gdImageString(PNG_image, tinyFont_ptr, right_legend_lowerLeft_X_pixels + right_legend_width_pixels - string_length_pixels - 43,
                                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 35,
                                           (unsigned char*)text_string, black);

    // Draw two horizontal light-red lines (double width) to the right of the above text label:
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 38,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  -  4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 38,
                           light_red);
    gdImageLine(PNG_image, right_legend_lowerLeft_X_pixels + right_legend_width_pixels  - 39,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 39,
                           right_legend_lowerLeft_X_pixels + right_legend_width_pixels  -  4,
                           right_legend_lowerLeft_Y_pixels - right_legend_height_pixels + 39,
                           light_red);

  }  // End of block for creating graph outline and legends

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Finished creating the graph outline and legends in function createRoutingMetricsGraph.\n");
  #endif

  { //
    // This block labels the x-axis for iterations:
    //

    { // Label the x-axis:
      char x_axis_label[] = "Iteration No.\0";

      // Estimate the width of the x-axis label so it can be positioned correctly. The width of 'Medium
      // Bold' font is 7 pixels (13 pixels high), per https://libgd.github.io/manuals/2.3.3/files/gdfontmb-c.html.
      int label_width_pixels = 7 * strlen(x_axis_label);

      gdImageString(PNG_image, giantFont_ptr, (graph_leftSide_pixels + graph_rightSide_pixels)/2 - label_width_pixels/2,
                    graph_bottomSide_pixels + major_tick_length_pixels + 20,
                    (unsigned char*) x_axis_label, black);
    }  // End of block for adding x-axis label

    // Determine the number of iterations between major tick-marks on the x-axis:
    int major_x_tick_mark = 1;
    int minor_x_tick_mark = 1;
    {
      if (num_iterations <= 10)  {
        major_x_tick_mark = 1;
        minor_x_tick_mark = 1;
      }
      else if (num_iterations <= 20)  {
        major_x_tick_mark = 2;
        minor_x_tick_mark = 1;
      }
      else if (num_iterations <= 50)  {
        major_x_tick_mark = 5;
        minor_x_tick_mark = 1;
      }
      else if (num_iterations <= 100)  {
        major_x_tick_mark = 10;
        minor_x_tick_mark =  5;
      }
      else if (num_iterations <= 200)  {
        major_x_tick_mark = 20;
        minor_x_tick_mark = 10;
      }
      else if (num_iterations <= 500)  {
        major_x_tick_mark = 50;
        minor_x_tick_mark = 10;
      }
      else if (num_iterations <= 1000)  {
        major_x_tick_mark = 100;
        minor_x_tick_mark =  50;
      }
      else if (num_iterations <= 2000)  {
        major_x_tick_mark = 200;
        minor_x_tick_mark = 100;
      }
      else if (num_iterations <= 5000)  {
        major_x_tick_mark = 500;
        minor_x_tick_mark = 100;
      }
      else if (num_iterations < 10000)  {
        major_x_tick_mark = 1000;
        minor_x_tick_mark =  500;
      }
    }  // End of block for calculating major and minor x tick-marks

    // Place grey minor tick-marks along the inside of the x-axis:
    for (int iteration = start_iteration - 1 + minor_x_tick_mark; iteration <= end_iteration; iteration = iteration + minor_x_tick_mark)  {

      int x_pixels = graph_leftSide_pixels + (iteration - start_iteration + 1) * x_axis_pixels_per_iteration;

      gdImageLine(PNG_image,
                  x_pixels, graph_bottomSide_pixels,
                  x_pixels, graph_bottomSide_pixels - minor_tick_length_pixels,
                  grey);
    }  // End of for-loop for generating minor tick-marks along x-axis

    //
    // Place numeric labels and black major tick-marks along the x-axis:
    //
    for (int iteration = start_iteration - 1; iteration <= end_iteration + 1; iteration = iteration + major_x_tick_mark)  {

      int x_pixels = graph_leftSide_pixels + (iteration - start_iteration + 1) * x_axis_pixels_per_iteration;

      // Major tick-mark:
      gdImageLine(PNG_image,
                  x_pixels, graph_bottomSide_pixels + major_tick_length_pixels/2,
                  x_pixels, graph_bottomSide_pixels - major_tick_length_pixels/2,
                  black);

      // Numeric label:
      char num_label[50];  // Character string to hold the numeric label
      sprintf(num_label, "%d", iteration);  // Convert the iteration number to a text string

      // Estimate the width of the numeric label so it can be positioned correctly. The width of 'Medium
      // Bold' font is 7 pixels, per https://libgd.github.io/manuals/2.3.3/files/gdfontmb-c.html.
      int label_width_pixels = 7 * strlen(num_label);
      gdImageString(PNG_image, medFont_ptr, x_pixels - label_width_pixels/2,
                    graph_bottomSide_pixels + major_tick_length_pixels/2 + tickMark_to_labelSpacing_pixels,
                    (unsigned char*) num_label, black);

    }  // End of for-loop for generating major tick-marks and numeric labels along x-axis

     //
     //
  }  // End of block for labeling the x-axis

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Finished labeling the x-axis for iterations in function createRoutingMetricsGraph.\n");
  #endif

  { //
    // This block labels the left-hand y-axis, which is used for plotting costs:
    //

    // Convert the costs to meters, accounting for (a) the factor of 10 used for each cell,
    // (b) the factor of 2^30 used for non-pin-swap zones, (c) the size of each cell (in microns),
    // and (d) the 10^6 conversion from microns to meters:
    const float min_cost_meters = (float) min_cost / cost_per_meter;
    const float max_cost_meters = (float) max_cost / cost_per_meter;

    //
    // Determine the appropriate units to plot the cost-lengths in: microns, millimeters, or meters. Select the unit such that
    // most of the range of the cost-data fall between 10 and 9999. The units are stored in the variable 'unit_multiplier',
    // which is 1 for meters, 1000 for millimeters, and 1,000,000 for microns. The unit string is stored in 'unit_string':
    //
    int unit_multiplier = 1;
    char unit_string[10];
/// if ((min_cost_meters >= 10.0) && (max_cost_meters < 1.0E4))  {
    if (min_cost_meters >= 10.0)  {
      unit_multiplier = 1;  // Plotted data should be presented in meters
      strcpy(unit_string, "m\0");
    }
    else if ((min_cost_meters >= 0.01) && (max_cost_meters < 10.0))  {
      unit_multiplier = 1000;  // Plotted data should be presented in millimeters
      strcpy(unit_string, "mm\0");
    }
/// else if ((min_cost_meters >= 0.000001) && (max_cost_meters < 0.01))  {
    else  {
      unit_multiplier = 1000000;  // Plotted data should be presented in microns
      strcpy(unit_string, "microns\0");
    }
/// else  {
///   printf("\nERROR: An unexpected condition occurred in function createRoutingMetricsGraph, which could not determine\n");
///   printf(  "       the appropriate units (meters, millimeters, or microns) to use for plotting routing costs. Please\n");
///   printf(  "       report this fatal error message to the software developer, including the following diagnostic\n");
///   printf(  "       information: unit_multiplier = '%'d', unit_string = '%s'\n", unit_multiplier, unit_string);
///   printf(  "                    min_cost_meters = '%f', max_cost_meters = '%f'\n", min_cost_meters, max_cost_meters);
///   printf(  "                    min_cost = '%'lu', max_cost = '%'lu'\n\n", min_cost, max_cost);
///   printf(  "       Program is exiting.\n\n");
///   exit(1);
/// }  // End of else-block for fatal error

    #ifdef DEBUG_createRoutingMetricsGraph
    printf("DEBUG: In createRoutingMetricsGraph, right vertical axis for cost has following attributes:\n");
    printf("DEBUG:    min_cost_meters = %f\n", min_cost_meters);
    printf("DEBUG:    max_cost_meters = %f\n", max_cost_meters);
    printf("DEBUG:    unit_multiplier = %d\n", unit_multiplier);
    printf("DEBUG:        unit_string = '%s'\n", unit_string);
    #endif


    { // Add a label to the left-hand y-axis:
      char y_axis_label[100];
      sprintf(y_axis_label, "Total Routing Cost (%s)", unit_string);

      // Estimate the height of the left y-axis label so it can be positioned correctly. The width of 'Medium
      // Bold' font is 7 pixels, per https://libgd.github.io/manuals/2.3.3/files/gdfontmb-c.html.
      int label_height_pixels = 8 * strlen(y_axis_label);

      gdImageStringUp(PNG_image, giantFont_ptr, graph_leftSide_pixels - graph_offset_from_left_pixels * 0.9,
                      (graph_topSide_pixels + graph_bottomSide_pixels)/2 + label_height_pixels/2,
                      (unsigned char*) y_axis_label, black);
    }

    // Determine the minimum, maximum, and range for the plotted cost data, in units determined above:
    float min_cost_units = min_cost_meters * unit_multiplier;
    float max_cost_units = max_cost_meters * unit_multiplier;
    float cost_range_units = max_cost_units - min_cost_units;

    // To handle the case where the minimum and maximum are identical:
    if (cost_range_units < 0.01)  {
      min_cost_units = min_cost_units - 0.8;
      max_cost_units = max_cost_units + 0.8;
      cost_range_units = max_cost_units - min_cost_units;
    }

    // Determine the y-axis range for the graph of the cost data. This
    // range is ~10% larger than the range of the actual data points:
    float graph_cost_range_units = 1.1 * cost_range_units;

    // Determine the number of units between major and minor tick-marks on the
    // vertical cost-axis
    float major_cost_tick_mark = 1.0;
    float minor_cost_tick_mark = 1.0;

    if (graph_cost_range_units <= 5)  {
      // Note that this case covers situations where the range of y-axis data is zero
      major_cost_tick_mark = 1.0;
      minor_cost_tick_mark = 0.1;
    }
    else if (graph_cost_range_units <= 10)  {
      major_cost_tick_mark = 1.0;
      minor_cost_tick_mark = 0.5;
    }
    else if (graph_cost_range_units <= 20)  {
      major_cost_tick_mark = 2.0;
      minor_cost_tick_mark = 1.0;
    }
    else if (graph_cost_range_units <= 50)  {
      major_cost_tick_mark = 5.0;
      minor_cost_tick_mark = 1.0;
    }
    else if (graph_cost_range_units <= 100)  {
      major_cost_tick_mark = 10.0;
      minor_cost_tick_mark =  5.0;
    }
    else if (graph_cost_range_units <= 200)  {
      major_cost_tick_mark = 20.0;
      minor_cost_tick_mark = 10.0;
    }
    else if (graph_cost_range_units <= 500)  {
      major_cost_tick_mark = 50.0;
      minor_cost_tick_mark = 10.0;
    }
    else if (graph_cost_range_units <= 1000)  {
      major_cost_tick_mark = 100.0;
      minor_cost_tick_mark =  50.0;
    }
    else if (graph_cost_range_units <= 2000)  {
      major_cost_tick_mark = 200.0;
      minor_cost_tick_mark = 100.0;
    }
    else if (graph_cost_range_units <= 5000)  {
      major_cost_tick_mark = 500.0;
      minor_cost_tick_mark = 100.0;
    }
    else if (graph_cost_range_units < 10000)  {
      major_cost_tick_mark = 1000.0;
      minor_cost_tick_mark =  500.0;
    }  // End of if/else-block for calculating major and minor cost tick-marks

    // Calculate the graph_min_cost_units and graph_max_cost_units so that they
    // will align to major tick-marks:
    float graph_min_cost_units = major_cost_tick_mark * floor((float)min_cost_units / major_cost_tick_mark);
    float graph_max_cost_units = major_cost_tick_mark *  ceil((float)max_cost_units / major_cost_tick_mark);

    // Also calculate the graph_min_cost variable, which is the minimum Acorn-supplied cost in
    // the graphing space:
    graph_min_cost = graph_min_cost_units * cost_per_meter / unit_multiplier;

    // Refine the graph_cost_range_units so it accurately reflects the updated min and
    // max values for the graph's upper and lower limits for the cost:
    graph_cost_range_units = graph_max_cost_units - graph_min_cost_units;

    // Calculate the scaling factor between pixels and cost (in units defined above):
    float y_axis_pixels_per_cost_unit =  (float)graph_y_size_pixels / graph_cost_range_units;

    // Also calculate the scaling  between pixels and the cost reported by Acorn:
    y_axis_pixels_per_cost = y_axis_pixels_per_cost_unit / cost_per_meter * unit_multiplier;

    #ifdef DEBUG_createRoutingMetricsGraph
    printf("\nDEBUG: In createRoutingMetricsGraph:\n");
    printf(  "DEBUG:      graph_bottomSide_pixels = %d\n", graph_bottomSide_pixels);
    printf(  "DEBUG:                     min_cost = %'lu\n", min_cost);
    printf(  "DEBUG:                     max_cost = %'lu\n", max_cost);
    printf(  "DEBUG:               cost_per_meter = %'f\n", cost_per_meter);
    printf(  "DEBUG:              min_cost_meters = %'f\n", min_cost_meters);
    printf(  "DEBUG:              max_cost_meters = %'f\n", max_cost_meters);
    printf(  "DEBUG:               graph_min_cost = %'lu\n", graph_min_cost);
    printf(  "DEBUG:         graph_min_cost_units = %'d\n", graph_min_cost_units);
    printf(  "DEBUG:         graph_max_cost_units = %'d\n", graph_max_cost_units);
    printf(  "DEBUG:  x_axis_pixels_per_iteration = %'f\n", x_axis_pixels_per_iteration);
    printf(  "DEBUG:          graph_y_size_pixels = %d\n", graph_y_size_pixels);
    printf(  "DEBUG:              unit_multiplier = %'d\n", unit_multiplier);
    printf(  "DEBUG:  y_axis_pixels_per_cost_unit = %'f\n", y_axis_pixels_per_cost_unit);
    printf(  "DEBUG:       y_axis_pixels_per_cost = %e\n\n", y_axis_pixels_per_cost);
    #endif

    //
    // Note: To calculate the y-axis pixel number from the cost (in units determined above), we use
    //       a linear equation of the form: y = mx + b, where:
    //         m = -y_axis_pixels_per_cost_unit (note the negative sign!), and
    //         b = graph_bottomSide_pixels + y_axis_pixels_per_cost_unit * graph_min_cost_units
    //

    // Place grey minor tick-marks along the inside of the left y-axis:
    for (float cost = graph_min_cost_units + minor_cost_tick_mark; cost <= graph_max_cost_units - minor_cost_tick_mark; cost = cost + minor_cost_tick_mark)  {
      int y_value_pixels = -cost*y_axis_pixels_per_cost_unit + graph_bottomSide_pixels + y_axis_pixels_per_cost_unit * graph_min_cost_units;
      gdImageLine(PNG_image, graph_leftSide_pixels, y_value_pixels, graph_leftSide_pixels + minor_tick_length_pixels, y_value_pixels, grey);
    }  // End of for-loop for generating minor tick-marks along left y-axis

    //
    // Place numeric labels and black major tick-marks along the left-hand y-axis:
    //
    for (float cost = graph_min_cost_units; cost <= graph_max_cost_units; cost = cost + major_cost_tick_mark)  {

      int y_value_pixels = -cost * y_axis_pixels_per_cost_unit
                             + graph_bottomSide_pixels
                             + y_axis_pixels_per_cost_unit * graph_min_cost_units;

      // Major tick-mark:
      gdImageLine(PNG_image, graph_leftSide_pixels - major_tick_length_pixels/2, y_value_pixels,
                             graph_leftSide_pixels + major_tick_length_pixels/2, y_value_pixels, black);

      // Numeric label:
      char num_label[50];  // Character string to hold the numeric label
      if (cost - floor(cost) < 0.001)  {
        sprintf(num_label, "%.0f", cost);  // Convert the cost value to a text string with no decimal places
      }
      else  {
        sprintf(num_label, "%.1f", cost);  // Convert the cost value to a text string with 1 decimal place
      }

      // Estimate the width of the numeric label so it can be positioned correctly. The width of 'Medium
      // Bold' font is 7 pixels and the height is 13 pixels, per https://libgd.github.io/manuals/2.3.3/files/gdfontmb-c.html.
      int label_width_pixels = 7 * strlen(num_label);
      gdImageString(PNG_image, medFont_ptr,
                    graph_leftSide_pixels - major_tick_length_pixels/2 - label_width_pixels - tickMark_to_labelSpacing_pixels,
                    y_value_pixels - 13/2, (unsigned char*) num_label, black);

    }  // End of for-loop for generating major tick-marks and numeric labels along left-hand y-axis

    //
    //
  } // End of block for labeling the left-hand y-axis

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Finished labeling the left-hand y-axis for costs in function createRoutingMetricsGraph.\n");
  #endif

  { //
    // This block labels the right-hand y-axis, which is used for plotting the number of DRC-cells:
    //

    { // Add a label to the right-hand y-axis:
      char y_axis_label[] = "Cells with Design-rule Violations";

      // Estimate the height of the left y-axis label so it can be positioned correctly. The width of 'Medium
      // Bold' font is 7 pixels, per https://libgd.github.io/manuals/2.3.3/files/gdfontmb-c.html.
      int label_height_pixels = 8 * strlen(y_axis_label);

      gdImageStringUp(PNG_image, giantFont_ptr, graph_rightSide_pixels + graph_offset_from_right_pixels * 0.65,
                      (graph_topSide_pixels + graph_bottomSide_pixels)/2 + label_height_pixels/2,
                      (unsigned char*) y_axis_label, red);
    }  // End of block for adding right-hand y-axis

    //
    // For the right-hand y-axis, we determine whether to use a linear axis or a logarithmic
    // axis. If the ratio of the largest DRC-count to the smallest is 100 or less, then we use
    // a linear axis. If it's greater than 100, we use logarithmic:
    //
    graph_min_DRCs      = min_DRC_cells;  // Minimum plottable value for DRCs.
    int graph_max_DRCs  = max_DRC_cells;  // Maximum plottable value for DRCs.
    int graph_DRC_range = graph_max_DRCs - graph_min_DRCs;
    float graph_DRC_maxMin_ratio = (float)max_DRC_cells / max(1, min_DRC_cells);

    // Determine the number of units between major and minor tick-marks on the
    // vertical DRC-axis
    int major_DRC_tick_mark = 1;
    int minor_DRC_tick_mark = 1;

    if (graph_DRC_maxMin_ratio > 100.0)  {

      //
      // We got here, so we'll use a logarithmic axis to plot the number of DRC-cells:
      //
      DRC_axis_is_linear = FALSE;
      #ifdef DEBUG_createRoutingMetricsGraph
      printf("DEBUG: In createRoutingMetricsGraph, right vertical axis for DRCs is logarithmic.\n");
      #endif

      // Set the minimum and maximum scale-values for the logarithmic DRC axis. Note that
      // these values represent the logarithm of the number of DRC-cells. For DRC-counts
      // of zero, we substitute 0.1, whose log is -1. This is clearly not realistic. But
      // for graphing purposes, this value is a place-holder for zero DRCs, which cannot
      // be plotted on a logarithmic axis.
      graph_min_DRCs = (int)floor(log10(max(0.1, min_DRC_cells)));
      graph_max_DRCs = (int)ceil(log10(max_DRC_cells));

      // Redefine the graph_DRC_range so it represents the log of the plotted range:
      graph_DRC_range = graph_max_DRCs - graph_min_DRCs;

      #ifdef DEBUG_createRoutingMetricsGraph
      printf("DEBUG: In createRoutingMetricsGraph, after adjusting min, max, and range:\n");
      printf("DEBUG:    Adjusted graph_min_DRCs = %d\n", graph_min_DRCs);
      printf("DEBUG:    Adjusted graph_max_DRCs = %d\n", graph_max_DRCs);
      printf("DEBUG:   Adjusted graph_DRC_range = %d\n", graph_DRC_range);
      #endif

    }  // End of if-block for graph_DRC_range > 100 (logarithmic axis)
    else  {
      //
      // We got here, so we'll use a linear axis to plot the number of DRC-cells:
      //

      #ifdef DEBUG_createRoutingMetricsGraph
      printf("DEBUG: In createRoutingMetricsGraph, right vertical axis for DRCs is linear.\n");
      printf("DEBUG:           Original graph_min_DRCs = %d\n", graph_min_DRCs);
      printf("DEBUG:           Original graph_max_DRCs = %d\n", graph_max_DRCs);
      printf("DEBUG:          Original graph_DRC_range = %d\n", graph_DRC_range);
      printf("DEBUG:   Original graph_DRC_maxMin_ratio = %.2f\n", graph_DRC_maxMin_ratio);
      #endif

      // First, handle the case where the minimum and maximum are identical:
      if (graph_DRC_range == 0)  {
        graph_min_DRCs = max(0, (int)min_DRC_cells - 1);
        graph_max_DRCs = graph_min_DRCs + 2;
        graph_DRC_range = graph_max_DRCs - graph_min_DRCs;
      }  // End of if-block for graph_DRC_range == 0

      // Determine the number of units between major and minor tick-marks on the
      // vertical (linear) DRC-axis
      if (graph_DRC_range <= 10)  {
        major_DRC_tick_mark = 1;
        minor_DRC_tick_mark = 1;
      }
      else if (graph_DRC_range <= 20)  {
        major_DRC_tick_mark = 2;
        minor_DRC_tick_mark = 1;
      }
      else if (graph_DRC_range <= 50)  {
        major_DRC_tick_mark = 5;
        minor_DRC_tick_mark = 1;
      }
      else if (graph_DRC_range <= 100)  {
        major_DRC_tick_mark = 10;
        minor_DRC_tick_mark =  5;
      }
      else if (graph_DRC_range <= 200)  {
        major_DRC_tick_mark = 20;
        minor_DRC_tick_mark = 10;
      }
      else if (graph_DRC_range <= 500)  {
        major_DRC_tick_mark = 50;
        minor_DRC_tick_mark = 10;
      }
      else if (graph_DRC_range <= 1000)  {
        major_DRC_tick_mark = 100;
        minor_DRC_tick_mark =  50;
      }
      else if (graph_DRC_range <= 2000)  {
        major_DRC_tick_mark = 200;
        minor_DRC_tick_mark = 100;
      }
      else if (graph_DRC_range <= 5000)  {
        major_DRC_tick_mark = 500;
        minor_DRC_tick_mark = 100;
      }
      else if (graph_DRC_range < 10000)  {
        major_DRC_tick_mark = 1000;
        minor_DRC_tick_mark =  500;
      }
      else if (graph_DRC_range <= 20000)  {
        major_DRC_tick_mark = 2000;
        minor_DRC_tick_mark = 1000;
      }
      else if (graph_DRC_range <= 50000)  {
        major_DRC_tick_mark = 5000;
        minor_DRC_tick_mark = 1000;
      }
      else if (graph_DRC_range < 100000)  {
        major_DRC_tick_mark = 10000;
        minor_DRC_tick_mark =  5000;
      }
      else if (graph_DRC_range <= 200000)  {
        major_DRC_tick_mark = 20000;
        minor_DRC_tick_mark = 10000;
      }
      else if (graph_DRC_range <= 500000)  {
        major_DRC_tick_mark = 50000;
        minor_DRC_tick_mark = 10000;
      }
      else if (graph_DRC_range < 1000000)  {
        major_DRC_tick_mark = 100000;
        minor_DRC_tick_mark =  50000;
      }  // End of if/else-block for calculating major and minor DRC tick-marks


      // Calculate the graph_min_DRC_units and graph_max_cost_units so that they
      // will align to major tick-marks:
      graph_min_DRCs = major_DRC_tick_mark * floor((float)graph_min_DRCs / major_DRC_tick_mark);
      graph_max_DRCs = major_DRC_tick_mark *  ceil((float)graph_max_DRCs / major_DRC_tick_mark);

      // Refine the graph_DRC_range so it accurately reflects the updated min and
      // max values for the graph's upper and lower limits for the DRCs:
      graph_DRC_range = graph_max_DRCs - graph_min_DRCs;


      #ifdef DEBUG_createRoutingMetricsGraph
      printf("DEBUG: In createRoutingMetricsGraph, after adjusting min, max, and range:\n");
      printf("DEBUG:    Adjusted graph_min_DRCs = %d\n", graph_min_DRCs);
      printf("DEBUG:    Adjusted graph_max_DRCs = %d\n", graph_max_DRCs);
      printf("DEBUG:   Adjusted graph_DRC_range = %d\n", graph_DRC_range);
      printf("DEBUG:        major_DRC_tick_mark = %d\n", major_DRC_tick_mark);
      printf("DEBUG:        minor_DRC_tick_mark = %d\n", major_DRC_tick_mark);
      #endif

    }  // End of else-block for labeling a DRC-axis that's linear

    // Calculate the scaling factor between pixels and DRCs. For a linear axis, this scaling
    // factor has the normal (linear) meaning. For a logarithmic axis, this scaling factor
    // should be interpreted as the number of pixels divided by the log of the DRC-count:
    y_axis_pixels_per_DRC =  (float)graph_y_size_pixels / graph_DRC_range;

    //
    // Note: To calculate the y-axis pixel number from the DRC-count (for linear or logarithmic axes),
    //       we use a linear equation of the form: y = mx + b, where:
    //         m = -y_axis_pixels_per_DRC (note the negative sign!), and
    //         b = graph_bottomSide_pixels + y_axis_pixels_per_DRC * graph_min_DRCs
    //

    // Place grey minor tick-marks along the inside of the right y-axis:
    for (int DRC_count = graph_min_DRCs + minor_DRC_tick_mark; DRC_count <= graph_max_DRCs - minor_DRC_tick_mark; DRC_count = DRC_count + minor_DRC_tick_mark)  {
      int y_value_pixels = -DRC_count*y_axis_pixels_per_DRC + graph_bottomSide_pixels + y_axis_pixels_per_DRC * graph_min_DRCs;
      gdImageLine(PNG_image, graph_rightSide_pixels - minor_tick_length_pixels, y_value_pixels, graph_rightSide_pixels, y_value_pixels, grey);
    }  // End of for-loop for generating minor tick-marks right-hand y-axis

    //
    // Place red numeric labels and red major tick-marks along the right-hand y-axis, with the exception of the
    // tick-mark and label for zero design-rule violations, which is black (not red):
    //
    for (int DRC_count = graph_min_DRCs; DRC_count <= graph_max_DRCs; DRC_count = DRC_count + major_DRC_tick_mark)  {

      int y_value_pixels = -DRC_count*y_axis_pixels_per_DRC + graph_bottomSide_pixels + y_axis_pixels_per_DRC * graph_min_DRCs;

      // Define the default color as red. But if the DRC_count is the minimum value, then use black instead:
      int label_tickMark_color = red;
      if (   (   DRC_axis_is_linear  && (DRC_count == 0))
          || ((! DRC_axis_is_linear) && (DRC_count == -1)))  {
        label_tickMark_color = black;
      }

      // Major tick-mark:
      gdImageLine(PNG_image, graph_rightSide_pixels - major_tick_length_pixels/2, y_value_pixels,
                             graph_rightSide_pixels + major_tick_length_pixels/2, y_value_pixels, label_tickMark_color);

      // Numeric label:
      char num_label[50];  // Character string to hold the numeric label
      if (DRC_axis_is_linear)  {
        sprintf(num_label, "%d", DRC_count);  // Convert the DRC-count to a text string (for linear axis)
      }
      else  {
        // Convert the DRC-count to a text-string, accounting for the logarithmic axis. The DRC count of
        // of '-1' will be converted to a numeric lable of '0' because (int){10 ^ (-1)} = (int){0.1} = 0.
        sprintf(num_label, "%'d", (int)pow(10, DRC_count));
      }

      gdImageString(PNG_image, medFont_ptr,
                    graph_rightSide_pixels + major_tick_length_pixels/2 + tickMark_to_labelSpacing_pixels,
                    y_value_pixels - 13/2, (unsigned char*) num_label, label_tickMark_color);

    }  // End of for-loop for generating major tick-marks and numeric labels along right-hand y-axis
    //
    //
  } // End of block for labeling the right-hand y-axis for DRC-cells.

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: Finished labeling the right-hand y-axis for DRC cells in function createRoutingMetricsGraph.\n");
  #endif

  //
  // Plot the cost data points in black, substituting a blue point for iterations that have
  // zero DRC-cells. Plot the DRC-count in red, substituting a green point for iterations
  // that have zero DRC-cells:
  //
  // Define variables for 'previous' coordinates. Initialize these to zero to avoid compile-time warnings:
  int prev_x_pixels      = 0;
  int prev_y_cost_pixels = 0;
  int prev_y_DRC_pixels  = 0;
  for (int iteration = start_iteration; iteration <= end_iteration; iteration++)  {
    // Define coordinates in the pixel coordinate system:
    int x_pixels = graph_leftSide_pixels + (iteration - start_iteration + 1) * x_axis_pixels_per_iteration;

    int y_cost_pixels = (int)(-(float)routability->nonPseudoPathCosts[iteration] * y_axis_pixels_per_cost
                              + graph_bottomSide_pixels
                              + (float)graph_min_cost * y_axis_pixels_per_cost);
    int y_DRC_pixels;
    if (DRC_axis_is_linear)  {
      y_DRC_pixels = (int)(-(float)routability->nonPseudo_num_DRC_cells[iteration] * y_axis_pixels_per_DRC
                            + graph_bottomSide_pixels
                            + (float)graph_min_DRCs * y_axis_pixels_per_DRC);
    }  // End of if-block for a linear axis
    else  {
      float log_of_DRCs;
      if (routability->nonPseudo_num_DRC_cells[iteration])  {
        // We got here, so this iteration has a non-zero number of DRCs. So we
        // can take the log of this value:
        log_of_DRCs = log10(routability->nonPseudo_num_DRC_cells[iteration]);
      }
      else  {
        // We got here, so this iteration has zero DRCs. Since we can't take the log of zero,
        // we assign the log to be -1 (as if we had 0.1 DRCs instead of zero DRCs):
        log_of_DRCs = -1;
      }
      y_DRC_pixels = (int)(-log_of_DRCs * y_axis_pixels_per_DRC
                            + graph_bottomSide_pixels
                            + (float)graph_min_DRCs * y_axis_pixels_per_DRC);
    }  // End of if-block for a logarithmic axis


    #ifdef DEBUG_createRoutingMetricsGraph
    printf("\nDEBUG: In createRoutingMetricsGraph for iteration = %d:\n", iteration);
    printf(  "DEBUG:      routability->nonPseudoPathCosts[iteration] = %'lu\n", routability->nonPseudoPathCosts[iteration]);
    printf(  "DEBUG:                          y_axis_pixels_per_cost = %e\n", y_axis_pixels_per_cost);
    printf(  "DEBUG:                         graph_bottomSide_pixels = %d\n", graph_bottomSide_pixels);
    printf(  "DEBUG:                                  graph_min_cost = %'lu\n", graph_min_cost);
    printf(  "DEBUG:   -((float)routability->nonPseudoPathCosts[iteration]) * y_axis_pixels_per_cost = %d\n",
            (int)(-((float)routability->nonPseudoPathCosts[iteration]) * y_axis_pixels_per_cost));
    printf(  "DEBUG:                                  graph_min_cost * y_axis_pixels_per_cost = %d\n",
            (int)(graph_min_cost * y_axis_pixels_per_cost));

    printf("\nDEBUG: In function createRoutingMetricsGraph, plotting (%d, %d) in pixels\n", x_pixels, y_cost_pixels);
    printf(  "DEBUG: for raw data point (%d iterations, %'lu cost)\n\n", iteration, routability->nonPseudoPathCosts[iteration]);
    #endif

    // Plot a point for the cost in the graph. If the iteration is DRC-free, then the point is a larger
    // blue solid square. If the iteration has DRCs, it's a smaller black solid square:
    int point_size_pixels;
    int point_color;
    if (routability->nonPseudo_num_DRC_cells[iteration] > 0)  {
      point_size_pixels = dataPoint_size_pixels;
      point_color = black;
    }
    else {
      point_size_pixels = 2 * dataPoint_size_pixels;
      point_color = blue;
    }

    // Draw the cost point in the graph:
    gdImageFilledRectangle(PNG_image,
                           x_pixels - point_size_pixels/2, y_cost_pixels - point_size_pixels/2,
                           x_pixels + point_size_pixels/2, y_cost_pixels + point_size_pixels/2,
                           point_color);

    // Draw a black line from the previous cost point to the current cost point:
    if (iteration > start_iteration)  {
      gdImageLine(PNG_image, prev_x_pixels, prev_y_cost_pixels,
                             x_pixels,      y_cost_pixels, black);
    }

    // If the iteration is the lowest-cost iteration, then draw a blue circle around the cost point:
    if (iteration == routability->lowest_cost_iteration)  {
      gdImageArc(PNG_image, x_pixels, y_cost_pixels, 10, 10, 0, 360, blue);
    }

    //
    // Plot the DRC data:
    //
    // Draw a red line from the previous DRC point to the current DRC point, as long as
    // at least one of these points is non-zero:
    if (   (iteration > start_iteration)
        && (routability->nonPseudo_num_DRC_cells[iteration] || routability->nonPseudo_num_DRC_cells[iteration - 1])) {
      gdImageLine(PNG_image, prev_x_pixels, prev_y_DRC_pixels,
                             x_pixels,      y_DRC_pixels,       red);
    }

    // Plot a point for the DRC-count in the graph. If the iteration is DRC-free, then the point is a
    // larger green solid square. If the iteration has DRCs, it's a smaller red solid square:
    if (routability->nonPseudo_num_DRC_cells[iteration] > 0)  {
      point_size_pixels = dataPoint_size_pixels;
      point_color = red;
    }
    else {
      point_size_pixels = 2 * dataPoint_size_pixels;
      point_color = green;
    }

    // Draw the DRC point in the graph:
    gdImageFilledRectangle(PNG_image,
                           x_pixels - point_size_pixels/2, y_DRC_pixels - point_size_pixels/2,
                           x_pixels + point_size_pixels/2, y_DRC_pixels + point_size_pixels/2,
                           point_color);


    // In anticipation for the next time through this for-loop, save the current x/y data points:
    prev_x_pixels = x_pixels;
    prev_y_cost_pixels = y_cost_pixels;
    prev_y_DRC_pixels  = y_DRC_pixels;

  }  // End of for-loop for index 'iteration' for plotting cost data

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: About to open file '%s' for writing in function createRoutingMetricsGraph.\n", output_PNG_filename);
  #endif

  // Open a file for writing:
  PNG_out = fopen(output_PNG_filename, "wb");
  if (! PNG_out) {
    printf("\nERROR: Can't open output file '%s' for writing.\n\n", output_PNG_filename);
    exit(1);
  }

  //Output the image to the file in PNG format:
  gdImagePng(PNG_image, PNG_out);

  // Close the file:
  fclose(PNG_out);

  // Destroy the image in memory:
  gdImageDestroy(PNG_image);

  #ifdef DEBUG_createRoutingMetricsGraph
  printf("DEBUG: About to exit createRoutingMetricsGraph.\n");
  #endif

}  // End of function 'createRoutingMetricsGraph'


//-----------------------------------------------------------------------------
// Name: create_animation_HTML_files
// Desc: Create HTML files with animation of the routing evolution. If there is
//       more than 1 routing layer, this function creates 2*(N-1) HTML files, where
//       N is the number of routing layers. For each adjacent pair of routing
//       layers, this function creates an animated HTML file with all iterations,
//       and a second animated HTML file with only the last 30 iterations. (If
//       there are fewer than 31 iterations, then only (N-1) HTML files are
//       created.)
//
//       If there is only 1 routing layer, then this function creates an
//       animated HTML file for all iterations, and a second animated HTML file
//       for only the last 30 iterations (assuming there are more than 30
//       iterations).
//
//       If the function is not able to delete old HTML files, or is not able to
//       create new HTML files, then it returns with a non-zero return code.
//-----------------------------------------------------------------------------
static int create_animation_HTML_files(const MapInfo_t *mapInfo, const InputValues_t *user_inputs, int num_threads)  {

  printf("DEBUG: Entered function create_animation_HTML_files...\n");


  int return_status = 0;

  // 'create_last30' is a Boolean flag that, if TRUE, instructs this function to
  // create animated HTML files for the last 30 iterations. Otherwise, the function
  // will create animated HTML files only for *all* iterations.
  unsigned char create_last30 = FALSE;
  if (mapInfo->current_iteration > 30)  {
    create_last30 = TRUE;
  }

  // 'multiLayer' is a Boolean flag that, if TRUE, instructs this function to
  // create animated HTML files for each adjacent pair of routing layers. Otherwise,
  // the function creates animated HTML files only for the single routing layer
  // in the map.
  unsigned char multiLayer = FALSE;
  if (mapInfo->numLayers > 1)  {
    multiLayer = TRUE;
  }

  //
  // Iterate over all the pairs of adjacent routing layers in the map:
  //
  for (int layer = 0; layer < max(1, mapInfo->numLayers - 1); layer++)  {

    // Create unique JavaScript ID's for the layer(s) that will be displayed. The unique ID for each
    // layer is 'layer_xx', where xx is the Acorn layer number:
    int startLayerID = 2*layer;
    int endLayerID;
    if (multiLayer)  {
      endLayerID = startLayerID + 2;  // Include a via layer and one additional routing layer
    }
    else  {
      endLayerID = startLayerID;  // There's only a single layer in the entire map!
    }


    //
    // Iterate over both 'durations' of HTML animation:
    //   Duration #0 is the animated HTML file for *all* iterations
    //   Duration #1 is the animated HTML file for only the last 30 iterations:
    //
    for (int duration = 0; duration <= 1; duration++)  {

      if ((duration == 1) && (! create_last30)) {
        // Break out of for-loop for index 'duration', thereby skipping the 'last 30' HTML file:
        break;
      }

      // Define and set the name of the PNG file that contains the routing metrics, which
      // will be displayed in the HTML file created by this function:
      char metrics_file_name[80];
      if (duration == 0)  {
        sprintf(metrics_file_name, "metricsGraphAll.png");
      }
      else  {
        sprintf(metrics_file_name, "metricsGraphLast30.png");
      }


      // Define variables that will contain the first and the last iterations for the current animation:
      int first_iteration;
      int last_iteration = mapInfo->current_iteration;

      //
      // For this set of routing layers and duration of animation, create/calculate the
      // (1) first iteration number, (2) the name of the HTML file, (3) the string
      // used for the HTML title, and (4) the string used for the header in the body
      // of the HTML file:
      //
      char fileName[80];
      strncpy(fileName, "\0", 1);   // Initialize fileName text string.
      char HTML_title[80];
      strncpy(HTML_title, "\0", 1);   // Initialize HTML_title text string.
      char HTML_body_header[400];
      strncpy(HTML_body_header, "\0", 1);   // Initialize HTML_body_header text string.
      if (multiLayer)  {
        if (duration == 0)  {
          first_iteration = 1;
          sprintf(fileName, "animation_allIter_%s_and_%s.html", user_inputs->layer_names[2*layer], user_inputs->layer_names[2*(layer + 1)]);
          sprintf(HTML_title, "%s/%s Animation (all)", user_inputs->layer_names[2*layer], user_inputs->layer_names[2*(layer + 1)]);
        }
        else  {
          first_iteration = last_iteration - 29;  // Subtract 29 to get the first iteration to include in the last 30 iterations
          sprintf(fileName, "animation_last30iter_%s_and_%s.html", user_inputs->layer_names[2*layer], user_inputs->layer_names[2*(layer + 1)]);
          sprintf(HTML_title, "%s/%s Animation (30)", user_inputs->layer_names[2*layer], user_inputs->layer_names[2*(layer + 1)]);
        }
        // Create the HTML text string that will be displayed in the body of the HTML file:
        sprintf(HTML_body_header, "Animated Routing Evolution for Layers <B><FONT color=\"blue\">%s</FONT></B> and <B><FONT color=\"blue\">%s</FONT></B> from Iteration <B><FONT color=\"blue\">%d</FONT></B> Through <B><FONT color=\"blue\">%d</FONT></B>",
                user_inputs->layer_names[2*layer], user_inputs->layer_names[2*(layer + 1)], first_iteration, last_iteration);
      }  // End of if-block for 'multiLayer'
      else {
        if (duration == 0)  {
          first_iteration = 1;
          sprintf(fileName, "animation_allIter_%s.html", user_inputs->layer_names[2*layer]);
          sprintf(HTML_title, "%s Animation (all)", user_inputs->layer_names[2*layer]);
        }
        else  {
          first_iteration = last_iteration - 30;
          sprintf(fileName, "animation_last30iter_%s.html", user_inputs->layer_names[2*layer]);
          sprintf(HTML_title, "%s Animation (30)", user_inputs->layer_names[2*layer]);
        }
        // Create the HTML text string that will be displayed in the body of the HTML file:
        sprintf(HTML_body_header, "Animated Routing Evolution for Layer <B><FONT color=\"blue\">%s</FONT></B> from Iteration <B><FONT color=\"blue\">%d</FONT></B> Through <B><FONT color=\"blue\">%d</FONT></B>",
                user_inputs->layer_names[2*layer], first_iteration, last_iteration);
      }  // End of if-block for NOT 'multiLayer'

      // printf("DEBUG: In create_animation_HTML_files after iteration %d, layer=%d, duration=%d, fileName='%s'\n",
      //        mapInfo->current_iteration, layer, duration, fileName);


      // Open the HTML file for writing:
      FILE *fp_HTMLout;
      fp_HTMLout = fopen(fileName, "w");
      if (fp_HTMLout == NULL)  {
        return_status = 1;
        return(return_status);
      }

      // Create title of HTML page that contains the layer name(s) being animated and the duration:
      fprintf(fp_HTMLout, "<!DOCTYPE HTML>\n<HTML>\n<HEAD>\n");
      fprintf(fp_HTMLout, "<!-- Filename '%s' -->\n\n", fileName);
      fprintf(fp_HTMLout, "  <TITLE>%s</TITLE>\n\n", HTML_title);

      // Create CSS to enable overlay of images:
      fprintf(fp_HTMLout, "  <STYLE>\n");
      fprintf(fp_HTMLout, "    .overlay-container {\n");
      fprintf(fp_HTMLout, "      float: left;\n");
      fprintf(fp_HTMLout, "      position: relative;\n");
      fprintf(fp_HTMLout, "    }\n");
      fprintf(fp_HTMLout, "    .overlay-image {\n");
      fprintf(fp_HTMLout, "      position: absolute;\n");
      fprintf(fp_HTMLout, "      top: 0;\n");
      fprintf(fp_HTMLout, "      right: 0;\n");
      fprintf(fp_HTMLout, "    }\n");
      fprintf(fp_HTMLout, "  </STYLE>\n\n");


      // Create JavaScript that enables animation and changes in visibility:
      fprintf(fp_HTMLout, "  <SCRIPT>\n");
      fprintf(fp_HTMLout, "    const firstImage = %d;\n", first_iteration);
      fprintf(fp_HTMLout, "    const lastImage = %d;\n", last_iteration);
      fprintf(fp_HTMLout, "    const imageCount = %d; // Total images from %d through %d\n", last_iteration - first_iteration + 1,
              first_iteration, last_iteration);

      // Calculate the time for each frame of the animated evolution. For fewer than 30 iterations, each frame takes 1 second.
      // For 30 to 120 iterations, frame rate is adjusted so that entire show takes 30 seconds. For more than 120 iterations,
      // each frame takes 250 milliseconds.
      int milliseconds_per_frame = min(1000, max(250, 30000/(last_iteration - first_iteration + 1)));
      fprintf(fp_HTMLout, "    const displayTime = %d; // %d milliseconds for each image\n", milliseconds_per_frame, milliseconds_per_frame);
      fprintf(fp_HTMLout, "    const specialDisplayTime = 3000; // 3000 milliseconds for the first and last images\n");
      fprintf(fp_HTMLout, "    let currentIndex = firstImage;\n\n");
      fprintf(fp_HTMLout, "    // State variable 'intervalId' contains the ID of the current setTimeout, allowing you to pause/resume the slideshow:\n");
      fprintf(fp_HTMLout, "    let intervalId = null;\n\n");
      fprintf(fp_HTMLout, "    // State variable 'isReversed' tracks the direction of the slideshow:\n");
      fprintf(fp_HTMLout, "    let isReversed = false;\n\n");

      fprintf(fp_HTMLout, "    // Function showNextImage does the following:\n");
      fprintf(fp_HTMLout, "    //   o  Checks isReversed to determine whether to increment or decrement currentIndex.\n");
      fprintf(fp_HTMLout, "    //   o  Updates the images and the imageInfo text.\n");
      fprintf(fp_HTMLout, "    //   o  Sets the timeout for the next image display based on whether the current image is the first or last.\n");
      fprintf(fp_HTMLout, "    function showNextImage() {\n");
      fprintf(fp_HTMLout, "      const imgA = document.getElementById('layer_%02d');\n", 2*layer);
      if (multiLayer)  {
        fprintf(fp_HTMLout, "      const imgB = document.getElementById('layer_%02d');\n", 2*layer + 1);
        fprintf(fp_HTMLout, "      const imgC = document.getElementById('layer_%02d');\n", 2*layer + 2);
      }
      fprintf(fp_HTMLout, "      \n");
      fprintf(fp_HTMLout, "      const imageInfoTop    = document.getElementById('imageInfoTop');\n");
      fprintf(fp_HTMLout, "      const imageInfoBottom = document.getElementById('imageInfoBottom');\n");
      fprintf(fp_HTMLout, "      \n");
      fprintf(fp_HTMLout, "      const baseName = `map_iter${String(currentIndex).padStart(4, '0')}_`;\n");
      fprintf(fp_HTMLout, "      imgA.src = baseName + '%02d_%s.png';\n", 2*layer,     user_inputs->layer_names[2*layer]);
      if (multiLayer)  {
        fprintf(fp_HTMLout, "      imgB.src = baseName + '%02d_%s.png';\n", 2*layer + 1, user_inputs->layer_names[2*layer + 1]);
        fprintf(fp_HTMLout, "      imgC.src = baseName + '%02d_%s.png';\n", 2*layer + 2, user_inputs->layer_names[2*layer + 2]);
      }
      fprintf(fp_HTMLout, "      \n");
      fprintf(fp_HTMLout, "      if (currentIndex === 0)  {\n");
      fprintf(fp_HTMLout, "        imageInfoTop.textContent    = `Pre-routing configuration`;\n");
      fprintf(fp_HTMLout, "        imageInfoBottom.textContent = `Pre-routing configuration`;\n");
      fprintf(fp_HTMLout, "      } else {\n");
      fprintf(fp_HTMLout, "        imageInfoTop.textContent    = `Iteration ${currentIndex} of ${lastImage}`;\n");
      fprintf(fp_HTMLout, "        imageInfoBottom.textContent = `Iteration ${currentIndex} of ${lastImage}`;\n");
      fprintf(fp_HTMLout, "      }\n\n");

      // Specify the time for each frame. The 0th, first, and last frames are shown for longer durations
      // than all the other frames:
      fprintf(fp_HTMLout, "      let currentDisplayTime = 1000;  // Placeholder value\n");
      fprintf(fp_HTMLout, "      if (currentIndex === 0 || currentIndex === firstImage || currentIndex === lastImage) {\n");
      fprintf(fp_HTMLout, "        currentDisplayTime = specialDisplayTime;\n");
      fprintf(fp_HTMLout, "      }\n");
      fprintf(fp_HTMLout, "      else {\n");
      fprintf(fp_HTMLout, "        currentDisplayTime = displayTime;\n");
      fprintf(fp_HTMLout, "      }\n\n");
      fprintf(fp_HTMLout, "      intervalId = setTimeout(showNextImage, currentDisplayTime);\n");

      // Increment the 'currentIndex' JavaScript variable, ensuring that it cycles around after reaching first_imag or last_image:
      fprintf(fp_HTMLout, "      // Update the 'currentIndex' based on the 'isReversed' and previous 'currentIndex' values:\n");
      fprintf(fp_HTMLout, "      if (currentIndex === 0)  {\n");
      fprintf(fp_HTMLout, "        if (! isReversed)  {\n");
      fprintf(fp_HTMLout, "          currentIndex = firstImage;\n");
      fprintf(fp_HTMLout, "        } else {\n");
      fprintf(fp_HTMLout, "          currentIndex = lastImage;\n");
      fprintf(fp_HTMLout, "        }\n");
      fprintf(fp_HTMLout, "      }\n");

      fprintf(fp_HTMLout, "      else if (currentIndex === lastImage && ! isReversed) {\n");
      fprintf(fp_HTMLout, "        currentIndex = 0;\n");
      fprintf(fp_HTMLout, "      }\n");

      fprintf(fp_HTMLout, "      else if (currentIndex === firstImage && isReversed) {\n");
      fprintf(fp_HTMLout, "        currentIndex = 0;\n");
      fprintf(fp_HTMLout, "      }\n");

      fprintf(fp_HTMLout, "      else  {\n");
      fprintf(fp_HTMLout, "        if (! isReversed)  {\n");
      fprintf(fp_HTMLout, "          currentIndex = currentIndex + 1;\n");
      fprintf(fp_HTMLout, "        } else {\n");
      fprintf(fp_HTMLout, "          currentIndex = currentIndex - 1;\n");
      fprintf(fp_HTMLout, "        }\n");
      fprintf(fp_HTMLout, "      }\n");

      fprintf(fp_HTMLout, "    }  // End of function 'showNextImage'\n\n");

      fprintf(fp_HTMLout, "    // Function pauseSlideshow uses clearTimeout(intervalId) to pause the slideshow:\n");
      fprintf(fp_HTMLout, "    function pauseSlideshow() {\n");
      fprintf(fp_HTMLout, "      clearTimeout(intervalId);\n");
      fprintf(fp_HTMLout, "    }\n\n");

      fprintf(fp_HTMLout, "    // Function resumeSlideshow calls showNextImage to resume the slideshow.\n");
      fprintf(fp_HTMLout, "    function resumeSlideshow() {\n");
      fprintf(fp_HTMLout, "      showNextImage();\n");
      fprintf(fp_HTMLout, "    }\n\n");

      fprintf(fp_HTMLout, "    // Function reverseSlideshow does the following:\n");
      fprintf(fp_HTMLout, "    //   o  Toggles isReversed to change the direction of the slideshow.\n");
      fprintf(fp_HTMLout, "    //   o  Calls showNextImage to start the slideshow in the new direction.\n");
      fprintf(fp_HTMLout, "    function reverseSlideshow() {\n");
      fprintf(fp_HTMLout, "      isReversed = !isReversed;\n");
      fprintf(fp_HTMLout, "      showNextImage();\n");
      fprintf(fp_HTMLout, "    }\n\n");

      fprintf(fp_HTMLout, "    intervalId = setTimeout(showNextImage, specialDisplayTime);\n\n");

      // Create JavaScript to enable visibility/invisibility of the layer(s).
      fprintf(fp_HTMLout, "    function checkAll(x) {\n");
      fprintf(fp_HTMLout, "      if (x.checked == true) {\n");
      for (int legendLayer = startLayerID; legendLayer <= endLayerID; legendLayer++)  {
        fprintf(fp_HTMLout, "        document.getElementById('layer_%02d').style.visibility='visible';\n", legendLayer);
        fprintf(fp_HTMLout, "        document.getElementById('checkbox_%02d').checked=true;\n", legendLayer);
      }
      fprintf(fp_HTMLout, "      } else {\n");
      for (int legendLayer = startLayerID; legendLayer <= endLayerID; legendLayer++)  {
        fprintf(fp_HTMLout, "        document.getElementById('layer_%02d').style.visibility='hidden';\n", legendLayer);
        fprintf(fp_HTMLout, "        document.getElementById('checkbox_%02d').checked=false;\n", legendLayer);
      }


      fprintf(fp_HTMLout, "      }\n");
      fprintf(fp_HTMLout, "    }  // End of function checkAll\n\n");

      fprintf(fp_HTMLout, "  </SCRIPT>\n\n");
      fprintf(fp_HTMLout, "</HEAD>\n\n");

      fprintf(fp_HTMLout, "<BODY>\n");
      fprintf(fp_HTMLout, "  <H1>%s</FONT></H1>\n", HTML_body_header);

      // Create a table with 2 columns and 3 rows:
      fprintf(fp_HTMLout, "  <TABLE border=\"0\">\n");

      // Caption for routing evolution goes in top-left cell:
      fprintf(fp_HTMLout, "    <TR>\n");
      fprintf(fp_HTMLout, "      <TD valign=\"bottom\">\n");
      fprintf(fp_HTMLout, "        <DIV id=\"imageInfoTop\">\n");
      fprintf(fp_HTMLout, "          Pre-routing configuration\n");
      fprintf(fp_HTMLout, "        </DIV>\n");
      fprintf(fp_HTMLout, "        <button onclick=\"pauseSlideshow()\">Pause</button>\n");
      fprintf(fp_HTMLout, "        <button onclick=\"resumeSlideshow()\">Resume</button>\n");
      fprintf(fp_HTMLout, "        <button onclick=\"reverseSlideshow()\">Reverse</button>\n");
      fprintf(fp_HTMLout, "      </TD>\n");

      // Create a blank cell in the top-right corner:
      fprintf(fp_HTMLout, "      <TD>&nbsp;</TD>\n");
      fprintf(fp_HTMLout, "    </TR>\n");

      // Second row starts here, and contains the overlaid images in left column:
      fprintf(fp_HTMLout, "    <TR>\n");
      fprintf(fp_HTMLout, "      <TD valign=\"top\">\n");

      fprintf(fp_HTMLout, "        <DIV class=\"overlay-container\">\n");
      fprintf(fp_HTMLout, "          <IMG id=\"layer_%02d\" border=\"1\" src=\"map_iter0000_%02d_%s.png\" alt=\"Layer %s\">\n",
              2*layer, 2*layer, user_inputs->layer_names[2*layer], user_inputs->layer_names[2*layer]);
      if (multiLayer)  {
        fprintf(fp_HTMLout, "          <IMG id=\"layer_%02d\" class=\"overlay-image\" border=\"1\" src=\"map_iter0000_%02d_%s.png\" alt=\"Layer %s\" style=\"visibility:hidden\">\n",
                2*layer + 1, 2*layer + 1, user_inputs->layer_names[2*layer + 1], user_inputs->layer_names[2*layer + 1]);
        fprintf(fp_HTMLout, "          <IMG id=\"layer_%02d\" class=\"overlay-image\" border=\"1\" src=\"map_iter0000_%02d_%s.png\" alt=\"Layer %s\">\n",
                2*layer + 2, 2*layer + 2, user_inputs->layer_names[2*layer + 2], user_inputs->layer_names[2*layer + 2]);
      }
      fprintf(fp_HTMLout, "        </DIV>\n");
      fprintf(fp_HTMLout, "      </TD>\n");

      // Right-hand column contains a legend for the colors of the routing/via layers:
      fprintf(fp_HTMLout, "      <TD valign=\"middle\">\n");
      fprintf(fp_HTMLout, "        <TABLE border=\"1\">\n");
      fprintf(fp_HTMLout, "          <TR>\n");
      fprintf(fp_HTMLout, "            <TH rowspan=\"2\">Layer</TH>\n");
      fprintf(fp_HTMLout, "            <TH>Visibility</TH>\n");
      fprintf(fp_HTMLout, "          </TR>\n");
      fprintf(fp_HTMLout, "          <TR>\n");
      fprintf(fp_HTMLout, "            <TH><input type=\"checkbox\" name=\"check_uncheck_all\" onchange='checkAll(this);'\n");
      fprintf(fp_HTMLout, "                value=\"false\" id=\"id_check_uncheck_all\" style=\"indeterminate:true\"></TH>\n");
      fprintf(fp_HTMLout, "          </TR>\n");

      // In the body of the legend table, print out 1 row for each routing and via layer in this page's map.
      for (int legendLayer = startLayerID; legendLayer <= endLayerID; legendLayer++)  {
        // Print out layer name:
        fprintf(fp_HTMLout, "          <TR>\n");
        fprintf(fp_HTMLout, "            <TD align=\"center\"><B>%s</B></TD>\n", user_inputs->layer_names[legendLayer]);

        // Print a cell with a checkbox (for visibility) whose background color matches the color of the routing or via layer:
        fprintf(fp_HTMLout, "            <TD style=\"background-color:rgba(%d,%d,%d,%3.2f)\" align=\"center\">&nbsp;\n",
                RGBA[legendLayer*4], RGBA[legendLayer*4+1], RGBA[legendLayer*4+2], RGBA[legendLayer*4+3]/255.0);
        fprintf(fp_HTMLout, "              <input type=\"checkbox\" id=\"checkbox_%02d\" onclick=\"document.getElementById('layer_%02d').style.visibility=(this.checked)?'visible':'hidden';\n",
                legendLayer, legendLayer);
        fprintf(fp_HTMLout, "              document.getElementById('id_check_uncheck_all').indeterminate=true;\"");

        // If Acorn layer number 'legendLayer' is even, then initially check the checkbox because it's a routing layer.
        // Otherwise, keep the checkbox unchecked:
        if (legendLayer % 2)  {
          fprintf(fp_HTMLout, ">\n");
        }
        else  {
          fprintf(fp_HTMLout, " checked>\n");
        }
        fprintf(fp_HTMLout, "              &nbsp;\n");
        fprintf(fp_HTMLout, "            </TD>\n");
        fprintf(fp_HTMLout, "          </TR>\n");
      }  // End of for-loop for index 'legendLayer'
      fprintf(fp_HTMLout, "        </TABLE>\n");
      fprintf(fp_HTMLout, "      </TD>\n");

      fprintf(fp_HTMLout, "    </TR>\n");

      // Caption for routing evolution also goes in bottom cell::
      fprintf(fp_HTMLout, "    <TR>\n");
      fprintf(fp_HTMLout, "      <TD valign=\"top\">\n");
      fprintf(fp_HTMLout, "        <DIV id=\"imageInfoBottom\">\n");
      fprintf(fp_HTMLout, "          Pre-routing configuration\n");
      fprintf(fp_HTMLout, "        </DIV>\n");
      fprintf(fp_HTMLout, "        <button onclick=\"pauseSlideshow()\">Pause</button>\n");
      fprintf(fp_HTMLout, "        <button onclick=\"resumeSlideshow()\">Resume</button>\n");
      fprintf(fp_HTMLout, "        <button onclick=\"reverseSlideshow()\">Reverse</button>\n");
      fprintf(fp_HTMLout, "      </TD>\n");

      // Create a blank cell in the lower-right corner:
      fprintf(fp_HTMLout, "      <TD>&nbsp;</TD>\n");

      fprintf(fp_HTMLout, "    </TR>\n");
      fprintf(fp_HTMLout, "  </TABLE>\n\n");

      fprintf(fp_HTMLout, "  <HR>\n\n");

      // Create a table with 1 column and 3 rows that will contain the routing metrics for
      // the most recent 30 iterations:
      fprintf(fp_HTMLout, "  <TABLE border=\"0\">\n");
      fprintf(fp_HTMLout, "    <TR>\n");
      fprintf(fp_HTMLout, "      <TD valign=\"bottom\">\n");
      fprintf(fp_HTMLout, "        <FONT size=\"6\"><B>Routing metrics:</B></FONT>\n");
      fprintf(fp_HTMLout, "      </TD>\n");
      fprintf(fp_HTMLout, "    </TR>\n");
      fprintf(fp_HTMLout, "    <TR>\n");
      fprintf(fp_HTMLout, "      <TD width=\"900px\" valign=\"top\">\n");
      fprintf(fp_HTMLout, "        <IMG border=\"1\" src=\"%s\" alt=\"Graph of routing metrics\">\n", metrics_file_name);
      fprintf(fp_HTMLout, "      </TD>\n");
      fprintf(fp_HTMLout, "    </TR>\n");
      fprintf(fp_HTMLout, "    <TR>\n");
      fprintf(fp_HTMLout, "      <TD valign=\"top\">\n");
      fprintf(fp_HTMLout, "        Aggregate routing cost, including lateral traces and vertical vias, and accounting for user-defined\n");
      fprintf(fp_HTMLout, "        cost-zones. The vertical axis on the right shows the number of square cells involved with design-rule\n");
      fprintf(fp_HTMLout, "        violations. Each cell is %.2f by %.2f microns in size, as defined by the 'grid_resolution' parameter\n",
              user_inputs->cell_size_um, user_inputs->cell_size_um);
      fprintf(fp_HTMLout, "        in the input file.\n");
      fprintf(fp_HTMLout, "      </TD>\n");
      fprintf(fp_HTMLout, "    </TR>\n");
      fprintf(fp_HTMLout, "  </TABLE>\n\n");

      fprintf(fp_HTMLout, "  <BR><HR>\n");
      fprintf(fp_HTMLout, "  \n");

      // Write time-stamp and ACORN version to bottom of HTML file. Note that 'VERSION' is extracted from GIT repository at compile-time.
      {  // Beginning of block for writing time-stamp.
        time_t tim = time(NULL);
        struct tm *now = localtime(&tim);
        if (num_threads > 1)  {
          // Acorn is using multiple threads, so use the plural ('threads'):
          fprintf(fp_HTMLout, "  <FONT size=\"2\">Updated at %02d:%02d on %02d-%02d-%d from Acorn version '%s' using %d threads.</FONT><BR><BR>\n",
                            now->tm_hour, now->tm_min, now->tm_mon+1, now->tm_mday, now->tm_year+1900, VERSION, num_threads);
        }
        else  {
          // Acorn is using a single thread, so use the singular ('thread'):
          fprintf(fp_HTMLout, "  <FONT size=\"2\">Updated at %02d:%02d on %02d-%02d-%d from Acorn version '%s' using %d thread.</FONT><BR><BR>\n",
                            now->tm_hour, now->tm_min, now->tm_mon+1, now->tm_mday, now->tm_year+1900, VERSION, num_threads);
        }
      }  // End of block for writing time-stamp


      fprintf(fp_HTMLout, "</BODY>\n");
      fprintf(fp_HTMLout, "</HTML>\n");


      // Close the output HTML file
      int close_result = fclose(fp_HTMLout);
      if (close_result != 0)  {
        return_status = 1;
        return(return_status);
      }

    }  // End of for-loop for index 'duration'

  }  // End of for-loop for index 'layer'

  return(return_status);
}


//-----------------------------------------------------------------------------
// Name: create_routingStatus_HTML_file
// Desc: Create HTML report that summarizes the entire Acorn run. If the function
//       is not able to create the output file successfully, it returns a non-zero
//       return-code.
//-----------------------------------------------------------------------------
int create_routingStatus_HTML_file(const char *input_text_filename, const char *output_HTML_filename,
                                   const MapInfo_t *mapInfo, const RoutingMetrics_t *routability,
                                   InputValues_t *user_inputs, char *shapeTypeNames[NUM_SHAPE_TYPES],
                                   int adequateSolutionFound, int DRC_free_threshold, int num_threads)  {

  printf("DEBUG: Entered function create_routingStatus_HTML_file...\n");

  // The return_status is zero if function completes successfully, and is 1 otherwise:
  int return_status = 0;

  char *temp_input_filename;
  temp_input_filename = malloc(300 * sizeof(char));
  strcpy(temp_input_filename, input_text_filename);  // Create local copy of input_filename

  // Create a pointer to the base filename. This does not need its own memory allocation;
  // the pointer will point to memory that's already allocated in the variable
  // 'temp_input_filename' (above).
  char *base_input_filename;
  base_input_filename = basename(temp_input_filename);   // Calculate the base filename, removing the directory path

  // printf("DEBUG: The base name of input_filename is '%s'\n", base_input_filename);
  // printf("DEBUG: output_filename is '%s'\n", output_filename);
  FILE *fp_HTMLout;
  fp_HTMLout = fopen(output_HTML_filename, "w");
  if (fp_HTMLout == NULL)  {
    return_status = 1;
    return(return_status);
  }

  //
  // Call function that creates a PNG file containing a graph of the routing metrics
  // versus the iteration count starting with iteration #1:
  //
  createRoutingMetricsGraph(base_input_filename, "metricsGraphAll.png", mapInfo, routability, user_inputs,
                            adequateSolutionFound, 1, mapInfo->current_iteration);

  //
  // If Acorn has proceeded beyond iteration #30, then call the same function as above to create a PNG file
  // containing a graph of the routing metrics versus the iteration count for only the last 30 iterations:
  //
  if (mapInfo->current_iteration > 30)  {
    createRoutingMetricsGraph(base_input_filename, "metricsGraphLast30.png", mapInfo, routability, user_inputs,
                              adequateSolutionFound, max(1, mapInfo->current_iteration - 29), mapInfo->current_iteration);
  }  // End of if-block for current_iteration > 30


  fprintf(fp_HTMLout, "<!DOCTYPE HTML>\n<HTML lang=\"en\">\n");
  fprintf(fp_HTMLout, "<HEAD>\n");
  fprintf(fp_HTMLout, "  <meta charset=\"UTF-8\">\n");
  fprintf(fp_HTMLout, "  <TITLE>Routing Status</TITLE>\n\n");

  fprintf(fp_HTMLout, "  <SCRIPT type=\"text/javascript\">\n");

  // JavaScript to toggle the visibility of a row in an HTML table:
  fprintf(fp_HTMLout, "    function toggleMe(a){\n");
  fprintf(fp_HTMLout, "      var e=document.getElementById(a);\n");
  fprintf(fp_HTMLout, "      if(!e)return true;\n");
  fprintf(fp_HTMLout, "      if(e.style.display==\"none\"){\n");
  fprintf(fp_HTMLout, "        e.style.display=\"table-row\"\n");
  fprintf(fp_HTMLout, "      }\n");
  fprintf(fp_HTMLout, "      else{\n");
  fprintf(fp_HTMLout, "        e.style.display=\"none\"\n");
  fprintf(fp_HTMLout, "      }\n");
  fprintf(fp_HTMLout, "      return true;\n");
  fprintf(fp_HTMLout, "    }  // End of function 'toggleMe'\n\n");

  // JavaScript to animate the evolution of the routing (if the current iteration is not zero):
  if (mapInfo->current_iteration > 0)  {
    fprintf(fp_HTMLout, "    const imageCount = %d; // Total images from 0 to %d\n", mapInfo->current_iteration + 1, mapInfo->current_iteration);

    // Calculate the time for each frame of the animated evolution. For fewer than 30 iterations, each frame takes 1 second.
    // For 30 to 120 iterations, frame rate is adjusted so that entire show takes 30 seconds. For more than 120 iterations,
    // each frame takes 250 milliseconds.
    int milliseconds_per_frame = min(1000, max(250, 30000/mapInfo->current_iteration));
    fprintf(fp_HTMLout, "    const displayTime = %d; // %d milliseconds for each image\n", milliseconds_per_frame, milliseconds_per_frame);
    fprintf(fp_HTMLout, "    const specialDisplayTime = 3000; // 3000 milliseconds for the first and last images\n");
    fprintf(fp_HTMLout, "    let currentIndex = 0;\n\n");
    fprintf(fp_HTMLout, "    // State variable 'intervalId' contains the ID of the current setTimeout, allowing you to pause/resume the slideshow:\n");
    fprintf(fp_HTMLout, "    let intervalId = null;\n\n");
    fprintf(fp_HTMLout, "    // State variable 'isReversed' tracks the direction of the slideshow:\n");
    fprintf(fp_HTMLout, "    let isReversed = false;\n\n");

    fprintf(fp_HTMLout, "    // Function showNextImage does the following:\n");
    fprintf(fp_HTMLout, "    //   o  Checks isReversed to determine whether to increment or decrement currentIndex.\n");
    fprintf(fp_HTMLout, "    //   o  Updates the images and the imageInfo text.\n");
    fprintf(fp_HTMLout, "    //   o  Sets the timeout for the next image display based on whether the current image is the first or last.\n");
    fprintf(fp_HTMLout, "    function showNextImage() {\n");
    fprintf(fp_HTMLout, "      const img = document.getElementById('slideshow');\n");
    fprintf(fp_HTMLout, "      const imageInfoTop    = document.getElementById('imageInfoTop');\n");
    fprintf(fp_HTMLout, "      const imageInfoBottom = document.getElementById('imageInfoBottom');\n\n");

    fprintf(fp_HTMLout, "      if (isReversed) {\n");
    fprintf(fp_HTMLout, "        currentIndex = (currentIndex - 1 + imageCount) %% imageCount;\n");
    fprintf(fp_HTMLout, "      } else {\n");
    fprintf(fp_HTMLout, "        currentIndex = (currentIndex + 1) %% imageCount;\n");
    fprintf(fp_HTMLout, "      }\n\n");

    fprintf(fp_HTMLout, "      const nextImage = `map_composite_iter${String(currentIndex).padStart(4, '0')}.png`;\n");
    fprintf(fp_HTMLout, "      img.src = nextImage;\n\n");

    fprintf(fp_HTMLout, "      if (currentIndex === 0)  {\n");
    fprintf(fp_HTMLout, "        imageInfoTop.textContent    = `Pre-routing configuration`;\n");
    fprintf(fp_HTMLout, "        imageInfoBottom.textContent = `Pre-routing configuration`;\n");
    fprintf(fp_HTMLout, "      } else {\n");
    fprintf(fp_HTMLout, "        imageInfoTop.textContent    = `Iteration ${currentIndex} of ${imageCount - 1}`;\n");
    fprintf(fp_HTMLout, "        imageInfoBottom.textContent = `Iteration ${currentIndex} of ${imageCount - 1}`;\n");
    fprintf(fp_HTMLout, "      }\n\n");

    // Extend the duration of the image's visibility for three 'special' frames of the slideshow:
    // (1) the pre-routing image, (2) the first iteration, and (3) the final iteration:
    fprintf(fp_HTMLout, "      let currentDisplayTime = 1000;  // Placeholder value\n");
    fprintf(fp_HTMLout, "      if (currentIndex === 0 || currentIndex === 1 || currentIndex === imageCount - 1) {\n");
    fprintf(fp_HTMLout, "        currentDisplayTime = specialDisplayTime;\n");
    fprintf(fp_HTMLout, "      } else {\n");
    fprintf(fp_HTMLout, "        currentDisplayTime = displayTime;\n");
    fprintf(fp_HTMLout, "      }\n\n");

    fprintf(fp_HTMLout, "      intervalId = setTimeout(showNextImage, currentDisplayTime);\n");

    fprintf(fp_HTMLout, "    }  // End of function 'showNextImage'\n\n");

    fprintf(fp_HTMLout, "    // Function pauseSlideshow uses clearTimeout(intervalId) to pause the slideshow:\n");
    fprintf(fp_HTMLout, "    function pauseSlideshow() {\n");
    fprintf(fp_HTMLout, "      clearTimeout(intervalId);\n");
    fprintf(fp_HTMLout, "    }\n\n");

    fprintf(fp_HTMLout, "    // Function resumeSlideshow calls showNextImage to resume the slideshow.\n");
    fprintf(fp_HTMLout, "    function resumeSlideshow() {\n");
    fprintf(fp_HTMLout, "      showNextImage();\n");
    fprintf(fp_HTMLout, "    }\n\n");

    fprintf(fp_HTMLout, "    // Function reverseSlideshow does the following:\n");
    fprintf(fp_HTMLout, "    //   o  Toggles isReversed to change the direction of the slideshow.\n");
    fprintf(fp_HTMLout, "    //   o  Calls showNextImage to start the slideshow in the new direction.\n");
    fprintf(fp_HTMLout, "    function reverseSlideshow() {\n");
    fprintf(fp_HTMLout, "      isReversed = !isReversed;\n");
    fprintf(fp_HTMLout, "      showNextImage();\n");
    fprintf(fp_HTMLout, "    }\n\n");

    fprintf(fp_HTMLout, "    intervalId = setTimeout(showNextImage, specialDisplayTime);\n\n");
  }  // End of if-block for current_iteration > 0

  fprintf(fp_HTMLout, "  </SCRIPT>\n");
  fprintf(fp_HTMLout, "</HEAD>\n\n");

  fprintf(fp_HTMLout, "<BODY>\n");
  if (adequateSolutionFound)  {
    fprintf(fp_HTMLout, "  <H1>Routing Status: <FONT color=\"blue\">Successfully Completed</FONT></H1>\n");
  }
  // The 'adequateSolutionFound' flag is FALSE, so check whether it's FALSE because Acorn (a) has not yet
  // started iterating, or (b) is still iterating to improve the routing, or (c) because the maximum number
  // of iterations has been reached:
  else if (mapInfo->current_iteration == 0)  {
    fprintf(fp_HTMLout, "  <H1>Routing Status: Preparing to Iterate</H1>\n");
  }
  else if (mapInfo->current_iteration < user_inputs->maxIterations)  {
    if (mapInfo->current_iteration == 1)  {
      fprintf(fp_HTMLout, "  <H1>Routing Status: In Progress <FONT color=\"#B0B0B0\">(1 iteration)</FONT></H1>\n");
    }
    else  {
      fprintf(fp_HTMLout, "  <H1>Routing Status: In Progress <FONT color=\"#B0B0B0\">(%d iterations)</FONT></H1>\n", mapInfo->current_iteration);
    }
  }  // End of if-block for current_iteration < maxIterations
  else  {
    if (user_inputs->maxIterations > 1)  {
      // Plural 'iterations':
      fprintf(fp_HTMLout, "  <H1>Routing Status: <FONT color=\"red\">Failed after %d iterations</FONT></H1>\n", user_inputs->maxIterations);
    }
    else  {
      // Singular 'iteration':
      fprintf(fp_HTMLout, "  <H1>Routing Status: <FONT color=\"red\">Failed after %d iteration</FONT></H1>\n", user_inputs->maxIterations);
    }
  }

  // If the program has completed, then create a single-cell table that summarizes the results:
  if (adequateSolutionFound || (mapInfo->current_iteration == user_inputs->maxIterations))  {
    fprintf(fp_HTMLout, "  <TABLE border=\"1\" cellpadding=\"10\">\n");

    // If Acorn was successful, then print out the following summary:
    if (adequateSolutionFound)  {
      fprintf(fp_HTMLout, "    <TR><TD width=\"900\" bgcolor=\"#00FF66\">\n");  // Cell has green background if Acorn was successful
      fprintf(fp_HTMLout, "      Program completed successfully after %d iterations in %'d seconds, exploring %'lu cells. \n",
              mapInfo->current_iteration, routability->iteration_cumulative_time[mapInfo->current_iteration], routability->total_explored_cells);

      fprintf(fp_HTMLout, "      %d violation-free iterations were found; at least %d were required. \n",
              routability->cumulative_DRCfree_iterations[mapInfo->current_iteration], DRC_free_threshold);

      fprintf(fp_HTMLout, "      The lowest-cost routing results are in <A href=\"iteration%04d.html\">iteration %d</A>.\n",
              routability->lowest_cost_iteration, routability->lowest_cost_iteration);

    }  // End if if-block for adequateSolutionFound == TRUE

    // If Acorn was not successful, then print out the following message:
    else  {
      fprintf(fp_HTMLout, "    <TR><TD width=\"900\" bgcolor=\"#FFF9C4\">\n");  // Cell has yellow background if Acorn was not successful
      if (routability->cumulative_DRCfree_iterations[mapInfo->current_iteration] > 0)  {
        fprintf(fp_HTMLout, "      An insufficient number of violation-free iterations (%d) were achieved before reaching the \n",
                routability->cumulative_DRCfree_iterations[mapInfo->current_iteration]);
        fprintf(fp_HTMLout, "      maximum allowed number of iterations (%d) in %'d seconds, exploring %'lu cells. \n", user_inputs->maxIterations,
                routability->iteration_cumulative_time[mapInfo->current_iteration], routability->total_explored_cells);
      }
      else  {
        fprintf(fp_HTMLout, "      No violation-free iterations were achieved before reaching the maximum allowed number of iterations (%d) \n",
                user_inputs->maxIterations);
        fprintf(fp_HTMLout, "      in %'d seconds, exploring %'lu cells. \n", routability->iteration_cumulative_time[mapInfo->current_iteration],
                routability->total_explored_cells);
      }

      fprintf(fp_HTMLout, "      At least %d violation-free iterations were required. \n", DRC_free_threshold);

      fprintf(fp_HTMLout, "      The lowest-cost routing results are in <A href=\"iteration%04d.html\">iteration %d</A>.\n",
              routability->lowest_cost_iteration, routability->lowest_cost_iteration);

    }  // End of else-block for adequateSolutionFound == FALSE

    fprintf(fp_HTMLout, "    </TD></TR>\n");
    fprintf(fp_HTMLout, "  </TABLE>\n");  // End of HTML table for end-of-run summary
    fprintf(fp_HTMLout, "  <HR>\n");      // Horizontal line
  }


  // If at least 1 iteration has completed, then create a table with 1 row and 3 columns. The left column will
  // hold the graph of routing metrics. The third column will hold the animated evolution of the routing. The second
  // column will be empty, and is used for spacing.
  if (mapInfo->current_iteration > 0)  {

    fprintf(fp_HTMLout, "  <TABLE border=\"0\">\n");
    fprintf(fp_HTMLout, "    <TR>\n");

    // In left-hand cell, create a sub-table with 1 column and 3 rows to hold the graph of the routing
    // metrics, plus a title (on top) and a caption (at bottom):
    fprintf(fp_HTMLout, "      <TD valign=\"top\">\n");
    fprintf(fp_HTMLout, "        <TABLE border=\"0\">\n");
    fprintf(fp_HTMLout, "          <TR>\n");
    fprintf(fp_HTMLout, "            <TD valign=\"bottom\">\n");
    fprintf(fp_HTMLout, "              <FONT size=\"6\"><B>Routing metrics:</B></FONT>\n");
    fprintf(fp_HTMLout, "            </TD>\n");
    fprintf(fp_HTMLout, "          </TR>\n");
    fprintf(fp_HTMLout, "          <TR>\n");
    fprintf(fp_HTMLout, "            <TD width=\"900px\" valign=\"top\">\n");
    fprintf(fp_HTMLout, "              <IMG border=\"1\" src=\"metricsGraphAll.png\" alt=\"Graph of routing metrics\">\n");
    fprintf(fp_HTMLout, "            </TD>\n");
    fprintf(fp_HTMLout, "          </TR>\n");
    fprintf(fp_HTMLout, "          <TR>\n");
    fprintf(fp_HTMLout, "            <TD valign=\"top\">\n");
    fprintf(fp_HTMLout, "              Aggregate routing cost, including lateral traces and vertical vias, and accounting for user-defined\n");
    fprintf(fp_HTMLout, "              cost-zones. The vertical axis on the right shows the number of square cells involved with design-rule\n");
    fprintf(fp_HTMLout, "              violations. Each cell is %.2f by %.2f microns in size, as defined by the 'grid_resolution' parameter\n",
            user_inputs->cell_size_um, user_inputs->cell_size_um);
    fprintf(fp_HTMLout, "              in the input file.\n");
    fprintf(fp_HTMLout, "            </TD>\n");
    fprintf(fp_HTMLout, "          </TR>\n");
    fprintf(fp_HTMLout, "        </TABLE>\n");
    fprintf(fp_HTMLout, "      </TD>\n");

    // The middle cell is used only for spacing:
    fprintf(fp_HTMLout, "      <TD width=\"25px\">&nbsp;</TD>\n");


    // In right-hand cell, create a table with 2 column and 3 rows to hold the animated routing and
    // dynamic captions above and below the routing. Also include a legend that shows the colors of
    // each routing layer:
    fprintf(fp_HTMLout, "      <TD valign=\"top\">\n");
    fprintf(fp_HTMLout, "        <TABLE border=\"0\">\n");

    // Create a cell with the number of the image, in addition to buttons to Pause, Resume,
    // or Reverse the animation:
    fprintf(fp_HTMLout, "          <TR>\n");
    fprintf(fp_HTMLout, "            <TD valign=\"bottom\">\n");
    fprintf(fp_HTMLout, "              <DIV id=\"imageInfoTop\">\n");
    fprintf(fp_HTMLout, "                Pre-routing configuration\n");
    fprintf(fp_HTMLout, "              </DIV>\n");
    fprintf(fp_HTMLout, "              <button onclick=\"pauseSlideshow()\">Pause</button>\n");
    fprintf(fp_HTMLout, "              <button onclick=\"resumeSlideshow()\">Resume</button>\n");
    fprintf(fp_HTMLout, "              <button onclick=\"reverseSlideshow()\">Reverse</button>\n");
    fprintf(fp_HTMLout, "            </TD>\n");

    // Create empty cell in upper-right of current table:
    fprintf(fp_HTMLout, "            <TD>&nbsp;</TD>\n");
    fprintf(fp_HTMLout, "          </TR>\n");

    // Create a cell with the animated routing:
    fprintf(fp_HTMLout, "          <TR>\n");
    fprintf(fp_HTMLout, "            <TD valign=\"top\">\n");
    fprintf(fp_HTMLout, "              <IMG id=\"slideshow\" border=\"1\" src=\"map_composite_iter0000.png\" alt=\"Animated routing evolution\">\n");
    fprintf(fp_HTMLout, "            </TD>\n");

    // Create a cell to hold a sub-table with the legend of layer colors:
    fprintf(fp_HTMLout, "            <TD valign=\"middle\">\n");
    fprintf(fp_HTMLout, "              <TABLE border=\"1\">\n");
    fprintf(fp_HTMLout, "                <TR>\n");
    fprintf(fp_HTMLout, "                  <TH>Layer</TH>\n");
    fprintf(fp_HTMLout, "                  <TH>Color</TH>\n");
    fprintf(fp_HTMLout, "                </TR>\n");

    // In the body of the legend table, print out 1 row for each routing and via layer:
    for (int layer = 0; layer < 2 * mapInfo->numLayers - 1; layer++)  {

      // Print out layer name:
      fprintf(fp_HTMLout, "                <TR>\n");
      fprintf(fp_HTMLout, "                  <TD align=\"center\"><B>%s</B></TD>\n", user_inputs->layer_names[layer]);

      // Print a blank cell whose background color matches the color of the routing or via layer:
      fprintf(fp_HTMLout, "                  <TD style=\"background-color:rgba(%d,%d,%d,%3.2f)\" align=\"center\">&nbsp;</TD>\n",
              RGBA[layer*4], RGBA[layer*4+1], RGBA[layer*4+2], RGBA[layer*4+3]/255.0);

      fprintf(fp_HTMLout, "                </TR>\n");
    }  // End of for-loop for index 'layer'
    fprintf(fp_HTMLout, "              </TABLE>\n");
    fprintf(fp_HTMLout, "            </TD>\n");
    fprintf(fp_HTMLout, "          </TR>\n");


    // Create a cell with the number of the image, in addition to buttons to Pause, Resume,
    // or Reverse the animation:
    fprintf(fp_HTMLout, "          <TR>\n");
    fprintf(fp_HTMLout, "            <TD valign=\"top\">\n");
    fprintf(fp_HTMLout, "              <DIV id=\"imageInfoBottom\">\n");
    fprintf(fp_HTMLout, "                Pre-routing configuration\n");
    fprintf(fp_HTMLout, "              </DIV>\n");
    fprintf(fp_HTMLout, "              <button onclick=\"pauseSlideshow()\">Pause</button>\n");
    fprintf(fp_HTMLout, "              <button onclick=\"resumeSlideshow()\">Resume</button>\n");
    fprintf(fp_HTMLout, "              <button onclick=\"reverseSlideshow()\">Reverse</button>\n");
    fprintf(fp_HTMLout, "            </TD>\n");

    // Create empty cell in lower-right of current table:
    fprintf(fp_HTMLout, "            <TD>&nbsp;</TD>\n");

    fprintf(fp_HTMLout, "          </TR>\n");
    fprintf(fp_HTMLout, "        </TABLE>\n");

    fprintf(fp_HTMLout, "      </TD>\n");
    fprintf(fp_HTMLout, "    </TR>\n");
    fprintf(fp_HTMLout, "  </TABLE>\n");

    fprintf(fp_HTMLout, "  \n");
    fprintf(fp_HTMLout, "  <BR><HR><BR>\n");
    fprintf(fp_HTMLout, "  \n");

    //
    // Create an HTML table with 3 columns and 1 row. The left-hand cell will contain an HTML table that lists
    // the routing metrics for each iteration. (This embedded table can be quite long!)
    //
    // The middle cell will be empty, and is created for spacing.
    //
    // The right-hand cell will contain an embedded HTML table with hyperlinks to animated
    // evolutions of routing metrics.
    //
    fprintf(fp_HTMLout, "  <TABLE border=\"0\">\n");
    fprintf(fp_HTMLout, "    <TR>\n");
    fprintf(fp_HTMLout, "      <TD valign=\"top\">\n");

    //
    // Left-hand cell contains the metrics for each iteration:
    //
    fprintf(fp_HTMLout, "        <B><FONT size=\"5\">Metrics by Iteration:</FONT></B>\n");

    //
    // Create a table with 7 or 8 columns and enough rows to describe all iterations.
    //
    fprintf(fp_HTMLout, "        <TABLE border=\"1\">\n");

    // Create a header for the table:
    fprintf(fp_HTMLout, "          <TR>\n");
    fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\" \"padding-left: 20px;\">Iteration</TH>\n");
    fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\">Nets with<BR>Violations</TH>\n");
    fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\">Cells with<BR>Violations</TH>\n");
    fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\">Aggregate Path<BR>Length (mm)</TH>\n");
    if (mapInfo->numLayers > 1)  {
      fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\">Via<BR>Count</TH>\n");
    }
    fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\"><FONT size=\"1\">Explored<BR>Cells</FONT></TH>\n");
    fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\"><FONT size=\"1\">Elapsed<BR>Time (seconds)</FONT></TH>\n");
    fprintf(fp_HTMLout, "           <TH align=\"center\" bgcolor=\"#CCCCCC\"><FONT size=\"1\">Cumulative<BR>Time (seconds)</FONT></TH>\n");
    fprintf(fp_HTMLout, "          </TR>\n");

    // Initialize index to HTML-encoded messages before entering for-loop that iterates over all iterations:
    int HTML_message_index = routability->num_HTML_messages - 1;

    // Create a row for each iteration, starting with the most recent iteration:
    for (int i = mapInfo->current_iteration; i > 0; i--)  {

      // If the current iteration ('i') matches the iteration in one or more routability->HTML_message_iter_nums elements, then print out
      // the HTML-encoded message in routability->HTML_message_strings. The HTML-encoded string is created in a row above the row
      // that shows the current iteration's metrics:
      while ((HTML_message_index >= 0) && (i == routability->HTML_message_iter_nums[HTML_message_index]))  {
        fprintf(fp_HTMLout, "    <TR>\n");
        if (mapInfo->numLayers > 1)  {
          fprintf(fp_HTMLout, "            <TD colspan=\"8\">\n");
        }
        else  {
          fprintf(fp_HTMLout, "            <TD colspan=\"7\">\n");
        }

        fprintf(fp_HTMLout, "              %s\n", routability->HTML_message_strings[HTML_message_index]);

        fprintf(fp_HTMLout, "            </TD>\n");
        fprintf(fp_HTMLout, "          </TR>\n");

        // Decrement the 'HTML_message_index' since its message has already been printed:
        HTML_message_index--;

      }  // End of while-loop for iteration == HTML-message_iter_nums

      //
      // Print out a row that contains the routing metrics for iteration number 'i':
      //
      fprintf(fp_HTMLout, "          <TR>\n");

      // If the current iteration 'i' has the fewest DRCs or lowest-cost routing, then highlight the cell with a
      // green background and add a comment in small font:
      if (i == routability->lowest_cost_iteration)  {
        if (routability->nonPseudo_num_DRC_cells[i] == 0)  {
          // We got here, so this DRC-free iteration has the lowest routing cost. Print out the
          // iteration number, green background, including hyperlink, with 'Lowest cost' comment
          fprintf(fp_HTMLout, "            <TD align=\"center\" bgcolor=\"#00FF66\"><A href=\"iteration%04d.html\">&nbsp;<B>%d</B>&nbsp;</A><FONT size=\"1\"><BR>Lowest cost</FONT></TD>\n",
                  i, i);
        }  // End of if-block for nonPseudo_num_DRC_cells == 0
        else  {
          // We got here, so this iteration has the fewest number of DRC-cells. Print out the
          // iteration number, green background, including hyperlink, with 'Fewest cells with violations' comment
          fprintf(fp_HTMLout, "            <TD align=\"center\" bgcolor=\"#00FF66\"><A href=\"iteration%04d.html\">&nbsp;<B>%d</B>&nbsp;</A><FONT size=\"1\"><BR>Fewest cells<BR>with violations</FONT></TD>\n",
                  i, i);
        }  // End of else-block for nonPseudo_num_DRC_cells > 0
      }
      else  {
        fprintf(fp_HTMLout, "            <TD align=\"center\"><A href=\"iteration%04d.html\">&nbsp;%d&nbsp;</A></TD>\n", i, i);   // Iteration number, including hyperlink
      }

      //
      // If the current iteration is the one with the fewest number of DRC-nets, then highlight this cell
      // with a green background color and informative comment:
      //
      if (i == routability->fewest_DRCnets_iteration)  {
        // Number of nets with DRCs, highlighted in green because it's the lowest number in the Acorn run:
        fprintf(fp_HTMLout, "            <TD align=\"center\" bgcolor=\"#00FF66\"><B> %d / %d </B><FONT size=\"1\"><BR>Fewest nets</FONT></TD>\n",
               routability->numNonPseudoDRCnets[i], mapInfo->numPaths);
      }
      else  {
        // Number of nets with DRCs, with a standard (white) background:
        fprintf(fp_HTMLout, "            <TD align=\"center\"> %d / %d </TD>\n", routability->numNonPseudoDRCnets[i], mapInfo->numPaths);
      }

      //
      // The cell with the number of DRC-cells requires some calculations:
      //
      if (routability->nonPseudo_num_DRC_cells[i] > 0)  {
        // If the number of cells with DRCs is non-zero, then print out the number without any colored background:
        fprintf(fp_HTMLout, "            <TD align=\"center\">&nbsp;%'d <FONT size=\"1\">cells</FONT>\n", routability->nonPseudo_num_DRC_cells[i]);
      }  // End of if-block (nonPseudo_num_DRC_cells > 0)
      else {
        // We got here, so this iteration has zero DRC-cells. Color the cell blue if the required number of DRC-free
        // iterations has not yet been reached, and color it green if the threshold has indeed been reached:
        if (routability->cumulative_DRCfree_iterations[i] < DRC_free_threshold)  {
          fprintf(fp_HTMLout, "      <TD align=\"center\" bgcolor=\"#7DF9FF\">&nbsp;<B>%'d <FONT size=\"1\">cells</FONT></B>\n",   // Zero cells with DRCs, with BLUE background
                  routability->nonPseudo_num_DRC_cells[i]);
        }
        else  {
          fprintf(fp_HTMLout, "            <TD align=\"center\" bgcolor=\"#00FF66\">&nbsp;<B>%'d <FONT size=\"1\">cells</FONT></B>\n",   // Zero cells with DRCs, with GREEN background
                  routability->nonPseudo_num_DRC_cells[i]);
        }
      }  // End of else-block (nonPseudo_num_DRC_cells == 0)


      // If this iteration has zero DRC-cells, then report how many DRC-free iterations have been achieved, as well as
      // the number required:
      if (routability->nonPseudo_num_DRC_cells[i] == 0)  {
        fprintf(fp_HTMLout, "              <FONT size=\"1\"><BR>(#%d / %d)</FONT>\n",
                routability->cumulative_DRCfree_iterations[i], DRC_free_threshold);
      }  // End of if-block for nonPseudo_num_DRC_cells == zero

      // If the number of cells with non-pseudo DRCs is greater than zero but less than 'maxRecordedDRCs'
      // (typically 10), then also display a button that, when pressed, will display an additional row
      // beneath the current row with details of the nets involved in the violations:
      if ((routability->nonPseudo_num_DRC_cells[i] > 0) && (routability->nonPseudo_num_DRC_cells[i] <= maxRecordedDRCs))  {
        fprintf(fp_HTMLout, "              <BR><input type=\"button\" onclick=\"return toggleMe('showHide%d')\" value=\"Details\" style=\"height:15px; width:50px; font-family: sans-serif; font-size: 8px;\">\n", i);
      }  // End of if-block for (num_nonPseudo_DRC_cells > 0) && (num_nonPseudo_DRC_cells < maxRecordedDRCs)
      fprintf(fp_HTMLout, "            </TD>\n");  // Close out the HTML table cell with the number of DRC-cells
      //
      // Above line is the end of the HTML cell that contains the number of DRC-cells.
      //

      // If the current iteration has the shortest aggregate path-length, then highlight this cell with a green
      // background color and add an informative note:
      if (i == routability->shortest_path_iteration)  {
        fprintf(fp_HTMLout, "            <TD align=\"center\" bgcolor=\"#00FF66\"><B> %'.4f <FONT size=\"1\">mm</B><BR>", routability->nonPseudoPathLengths[i]);   // Aggregate path length (in mm)

        if (routability->nonPseudo_num_DRC_cells[i] == 0)  {
          fprintf(fp_HTMLout, "Shortest without<BR>violations</FONT></TD>\n");
        }
        else  {
          fprintf(fp_HTMLout, "Shortest with<BR>fewest violations</FONT></TD>\n");
        }
      }  // End of if-block for iteration being the one with the shortest aggregate path-length
      else  {
        fprintf(fp_HTMLout, "            <TD align=\"center\"> %'.4f <FONT size=\"1\">mm</FONT></TD>\n", routability->nonPseudoPathLengths[i]);   // Aggregate path length (in mm)
      }

      if (mapInfo->numLayers > 1)  {
        fprintf(fp_HTMLout, "            <TD align=\"center\"> %'d <FONT size=\"1\">vias</FONT></TD>\n", routability->nonPseudoViaCounts[i]);   // Number of vias
      }
      fprintf(fp_HTMLout, "            <TD align=\"center\"><FONT size=\"1\"> %'lu </FONT></TD>\n", routability->iteration_explored_cells[i]);   // Number of explored cells
      fprintf(fp_HTMLout, "            <TD align=\"center\"> %'d <FONT size=\"1\">s</FONT></TD>\n",
              routability->iteration_cumulative_time[i] - routability->iteration_cumulative_time[i -1 ]);   // Elapsed time in seconds
      fprintf(fp_HTMLout, "            <TD align=\"center\"> %'d <FONT size=\"1\">s</FONT></TD>\n", routability->iteration_cumulative_time[i]);   // Cumulative time in seconds
      fprintf(fp_HTMLout, "          </TR>\n");

      // If the number of cells with non-pseudo DRCs is less than 'maxRecordedDRCs' (typically 10),
      // then create a row in the table that contains details of the design-rule violations. By default,
      // this row will be hidden, but will appear if the user clicks the button in the previous row.
      if ((routability->nonPseudo_num_DRC_cells[i] > 0) && (routability->nonPseudo_num_DRC_cells[i] <= maxRecordedDRCs))  {
        fprintf(fp_HTMLout, "          <TR id=\"showHide%d\" style=\"display:none\">\n", i);
        if (mapInfo->numLayers > 1)  {
          fprintf(fp_HTMLout, "            <TD colspan=\"8\">\n");
        }
        else  {
          fprintf(fp_HTMLout, "            <TD colspan=\"7\">\n");
        }
        fprintf(fp_HTMLout, "              <B><U>%d cells with design-rule violations for iteration %d:</U></B>\n", routability->nonPseudo_num_DRC_cells[i], i);
        fprintf(fp_HTMLout, "              <FONT size=\"1\"><OL>\n");
        for (int DRC_index = 0; DRC_index < routability->nonPseudo_num_DRC_cells[i]; DRC_index++)  {

          // printf("DEBUG: Printing DRC #%d to HTML file...\n", DRC_index);
          fprintf(fp_HTMLout, "                <LI>Layer %s at location (%.0f, %.0f) microns between %s of net %s and<BR>the center of a %s in net %s (min spacing = %.2f; min dist = %.2f microns).</LI>\n",
                  user_inputs->layer_names[2 * routability->DRC_details[i][DRC_index].z], // Layer name of DRC location
                  routability->DRC_details[i][DRC_index].x * user_inputs->cell_size_um,   // X-coordinate (in um) of DRC location
                  routability->DRC_details[i][DRC_index].y * user_inputs->cell_size_um,   // Y-coordinate (in um) of DRC location
                  shapeTypeNames[routability->DRC_details[i][DRC_index].shapeType],       // Shape-type name at location of DRC violation
                  user_inputs->net_name[routability->DRC_details[i][DRC_index].pathNum],  // Net name at location of DRC violation
                  shapeTypeNames[routability->DRC_details[i][DRC_index].offendingShapeType],      // Shape-type name of offending shape
                  user_inputs->net_name[routability->DRC_details[i][DRC_index].offendingPathNum], // Net name of offending net
                  routability->DRC_details[i][DRC_index].minimumAllowedSpacing,           // Minimum allowed spacing from edge to edge (in um)
                  routability->DRC_details[i][DRC_index].minimumAllowedDistance);         // Minimum allowed distance from edge to centerline (in um)
        }  // End of for-loop for index 'DRC_index'
        fprintf(fp_HTMLout, "              </OL></FONT>\n");
        fprintf(fp_HTMLout, "            </TD>\n");
        fprintf(fp_HTMLout, "          </TR>\n");
      }

    }  // End of for-loop for index 'i' (from mapInfo->current_iteration down to 1)

    fprintf(fp_HTMLout, "        </TABLE>\n\n");

    fprintf(fp_HTMLout, "      </TD>\n");  // End of left-hand cell of 3-cell table.


    //
    // Middle cell of 3-cell table is used only for spacing:
    //
    fprintf(fp_HTMLout, "      <TD width=\"50px\">&nbsp;</TD>\n");


    //
    // Right-hand cell of 3-cell table contains an embedded table with hyperlinks to
    // other HTML files:
    //
    fprintf(fp_HTMLout, "      <TD valign=\"top\">\n");

    // Only populate this cell of the 3-cell table if at least 2 iterations have completed.
    // (Otherwise, there is little point in providing the user with animations of the
    // routing evolution.)
    if (mapInfo->current_iteration >= 2)  {


      // Call function that creates the animated HTML files that are hyperlinked in the
      // current cell:
      if (create_animation_HTML_files(mapInfo, user_inputs, num_threads))  {
        printf("\nERROR: Function 'create_animation_HTML_files' returned with an error, indicating that it could not delete or\n");
        printf(  "       create HTML files that contain animation of the routing evolution. This error is not expected.\n");
        printf(  "       Please inform the software developer of this fatal error message.\n\n");
        exit(1);
      }


      fprintf(fp_HTMLout, "        <B><FONT size=\"5\">Other Animations:</FONT></B>\n");
      fprintf(fp_HTMLout, "        <TABLE border=\"1\" cellpadding=\"5\">\n");

      // Print out a header-row:
      fprintf(fp_HTMLout, "          <TR>\n");
      if (mapInfo->numLayers > 1)  {
        fprintf(fp_HTMLout, "            <TH align=\"center\" bgcolor=\"#CCCCCC\" \"padding-left: 20px;\">Routing<BR>Layer Pairs</TH>\n");
      }
      else  {
        fprintf(fp_HTMLout, "            <TH align=\"center\" bgcolor=\"#CCCCCC\" \"padding-left: 20px;\">Routing<BR>Layer</TH>\n");
      }
      fprintf(fp_HTMLout, "            <TH align=\"center\" bgcolor=\"#CCCCCC\" \"padding-left: 20px;\">All<BR>Iterations</TH>\n");
      if (mapInfo->current_iteration > 30)  {
        fprintf(fp_HTMLout, "            <TH align=\"center\" bgcolor=\"#CCCCCC\" \"padding-left: 20px;\">Last 30<BR>Iterations</TH>\n");
      }
      fprintf(fp_HTMLout, "          </TR>\n");

      // If the map contains more than 1 routing layer, then print out a row for each pair of adjacent routing layers:
      if (mapInfo->numLayers > 1)  {
        for (int layer = 0; layer < mapInfo->numLayers - 1; layer++)  {
          fprintf(fp_HTMLout, "          <TR>\n");
          // Note: In the following line, HTML code '&#38;' is an ampersand ('&')
          fprintf(fp_HTMLout, "            <TD align=\"center\">%s<FONT size=\"1\"><BR>&#38;<BR></FONT>%s</TD>\n",
                  user_inputs->layer_names[2*layer], user_inputs->layer_names[2*layer + 2]);
          fprintf(fp_HTMLout, "            <TD align=\"center\"><A href=\"animation_allIter_%s_and_%s.html\" target=\"_all_%s_%s\">LINK</A></TD>\n",
                  user_inputs->layer_names[2*layer], user_inputs->layer_names[2*layer + 2],
                  user_inputs->layer_names[2*layer], user_inputs->layer_names[2*layer + 2]);
          if (mapInfo->current_iteration > 30)  {
            fprintf(fp_HTMLout, "            <TD align=\"center\"><A href=\"animation_last30iter_%s_and_%s.html\" target=\"_last30_%s_%s\">LINK</A></TD>\n",
                    user_inputs->layer_names[2*layer], user_inputs->layer_names[2*layer + 2],
                    user_inputs->layer_names[2*layer], user_inputs->layer_names[2*layer + 2]);
          }
          fprintf(fp_HTMLout, "          <TR>\n");
        }  // End of for-loop for index 'layer'
      }  // End of if-block for numLayers > 1
      else  {
        // We got here, so the map has only a single routing layer. Print out a single row:
        fprintf(fp_HTMLout, "          <TR>\n");
        fprintf(fp_HTMLout, "            <TD align=\"center\">%s</TD>\n", user_inputs->layer_names[0]);
        fprintf(fp_HTMLout, "            <TD align=\"center\"><A href=\"animation_allIter_%s.html\" target=\"_all_%s\">LINK</A></TD>\n",
                user_inputs->layer_names[0], user_inputs->layer_names[0]);
        if (mapInfo->current_iteration > 30)  {
          fprintf(fp_HTMLout, "            <TD align=\"center\"><A href=\"animation_last30iter_%s.html\" target=\"_last30_%s\">LINK</A></TD>\n",
                  user_inputs->layer_names[0], user_inputs->layer_names[0]);
        }
        fprintf(fp_HTMLout, "          </TR>\n");
      }  // End of else-block for numLayers == 1

      fprintf(fp_HTMLout, "        </TABLE>\n");

    }  // End of if-block for current_iteration >= 2


    fprintf(fp_HTMLout, "      </TD>\n");
    fprintf(fp_HTMLout, "    </TR>\n");
    fprintf(fp_HTMLout, "  </TABLE>\n");  // End of 3-cell table

    fprintf(fp_HTMLout, "  <BR><HR>\n");

  }  // End of if-block block for current_iteration > 0


  //
  // Print out information to HTML file about pre-routing information and key parameters. This information
  // is organized in an HTML table that contains 1 row and 3 columns:
  //
  fprintf(fp_HTMLout, "  <TABLE><TR>\n");
  fprintf(fp_HTMLout, "    <TD valign=\"top\">\n");
  // Create links to input file, to pre-routing map, to HTML file showing design
  // rules, and to HTML file showing cost-zones:
  fprintf(fp_HTMLout, "      <B><U>Pre-routing Information:</U></B>\n");
  fprintf(fp_HTMLout, "      <UL>\n");
  fprintf(fp_HTMLout, "        <LI>Input file: <FONT size=\"2\"><A href=\"%s\">%s</A></FONT></LI>\n", base_input_filename, base_input_filename);
  fprintf(fp_HTMLout, "        <LI><A href=\"preRouting_map.html\">Pre-routing map</A></LI>\n");
  fprintf(fp_HTMLout, "        <LI><A href=\"designRules.html\">Design rules</A></LI>\n");
  fprintf(fp_HTMLout, "        <LI><A href=\"costZones.html\">Cost zones</A></LI>\n");
  fprintf(fp_HTMLout, "      </UL>\n");
  fprintf(fp_HTMLout, "    </TD>\n");

  // Create a blank column to add horizontal spacing:
  fprintf(fp_HTMLout, "    <TD width=\"200px\">&nbsp;</TD>\n");

  // Write the value of key parameters to the output HTML file:
  fprintf(fp_HTMLout, "    <TD valign=\"top\">\n");
  fprintf(fp_HTMLout, "      <FONT size=\"1\" color=\"#B0B0B0\">Key parameters:\n");
  fprintf(fp_HTMLout, "      <UL>\n");
  fprintf(fp_HTMLout, "        <LI>grid_resolution: %.2f um</LI>\n", user_inputs->cell_size_um);
  fprintf(fp_HTMLout, "        <LI>maxIterations: %d</LI>\n", user_inputs->maxIterations);
  fprintf(fp_HTMLout, "        <LI>violationFreeThreshold: %d</LI>\n", user_inputs->userDRCfreeThreshold);
  fprintf(fp_HTMLout, "        <LI>DRC_free_threshold: %d</LI>\n", DRC_free_threshold);
  fprintf(fp_HTMLout, "        <LI>baseVertCostMicrons: %6.1f um</LI>\n", user_inputs->baseVertCostMicrons);
  fprintf(fp_HTMLout, "        <LI>baseVertCostCells: %'d cells</LI>\n", user_inputs->baseVertCostCells);
  fprintf(fp_HTMLout, "        <LI>baseVertCost: %'lu</LI>\n", user_inputs->baseVertCost);
  fprintf(fp_HTMLout, "        <LI>preEvaporationIterations: %d</LI>\n", user_inputs->preEvaporationIterations);
  fprintf(fp_HTMLout, "        <LI>runsPerPngMap: %d</LI>\n", user_inputs->runsPerPngMap);
  fprintf(fp_HTMLout, "        <LI>baseCellCost: %'lu</LI>\n", user_inputs->baseCellCost);
  fprintf(fp_HTMLout, "        <LI>baseDiagCost: %'lu</LI>\n", user_inputs->baseDiagCost);
  fprintf(fp_HTMLout, "        <LI>baseKnightCost: %'lu</LI>\n", user_inputs->baseKnightCost);
  fprintf(fp_HTMLout, "      </UL></FONT>\n");
  fprintf(fp_HTMLout, "    </TD>\n");
  fprintf(fp_HTMLout, "  </TR></TABLE>\n\n");

  fprintf(fp_HTMLout, "  <HR>\n");  // Generate a horizontal line

  // Write time-stamp and ACORN version to bottom of HTML file. Note that 'VERSION' is extracted from GIT repository at compile-time.
  {  // Beginning of block for writing time-stamp.
    time_t tim = time(NULL);
    struct tm *now = localtime(&tim);
    if (num_threads > 1)  {
      // Acorn is using multiple threads, so use the plural ('threads'):
      fprintf(fp_HTMLout, "  <FONT size=\"2\">Updated at %02d:%02d on %02d-%02d-%d from Acorn version '%s' using %d threads.</FONT><BR><BR>\n",
                        now->tm_hour, now->tm_min, now->tm_mon+1, now->tm_mday, now->tm_year+1900, VERSION, num_threads);
    }
    else  {
      // Acorn is using a single thread, so use the singular ('thread'):
      fprintf(fp_HTMLout, "  <FONT size=\"2\">Updated at %02d:%02d on %02d-%02d-%d from Acorn version '%s' using %d thread.</FONT><BR><BR>\n",
                        now->tm_hour, now->tm_min, now->tm_mon+1, now->tm_mday, now->tm_year+1900, VERSION, num_threads);
    }
  }  // End of block for writing time-stamp

  fprintf(fp_HTMLout, "</BODY>\n");
  fprintf(fp_HTMLout, "</HTML>\n");

  int close_result = fclose(fp_HTMLout); // Close the output HTML file
  if (close_result != 0)  {
    return_status = 1;
    return(return_status);
  }

  // Free the memory that was allocated in this function:
  free(temp_input_filename);    temp_input_filename = NULL;

  // Return the status of this program to the calling routine:
  return(return_status);

}  // End of function 'create_routingStatus_HTML_file'

