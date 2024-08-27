#include "global_defs.h"
#include "processDiffPairs.h"
#include "aStarLibrary.h"




//-----------------------------------------------------------------------------
// Name: calcRoutingRadiiAtTerminal
// Desc: Calculate radius of allowed routing for path 'diffairPathNum' around the
//       coordinates of the terminal 'pseudoTerm'. This function modifies the
//       elements 'allowedLayers[]', 'allowedRadiiCells[]' and
//       'allowedRadiiMicrons[]' of variable 'terminalRestrictions'.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calcRoutingRadiiAtTerminal' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_calcRoutingRadiiAtTerminal 1
#undef DEBUG_calcRoutingRadiiAtTerminal

static void calcRoutingRadiiAtTerminal(RoutingRestriction_t *terminalRestrictions, int diffPairPathNum,
                                       float diffPairTerminalPitchMicrons, int currentLayer,
                                       Coordinate_t pseudoTerm, InputValues_t *user_inputs,
                                       CellInfo_t ***cellInfo, const MapInfo_t *mapInfo)  {

  #ifdef DEBUG_calcRoutingRadiiAtTerminal
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((diffPairPathNum == 1) && (mapInfo->current_iteration == 26))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in calcRoutingRadiiAtTerminal() because specific requirements were met.\n\n");
    DEBUG_ON = TRUE;

    printf("\nDEBUG: Entered function calcRoutingRadiiAtTerminal with following input parameters:\n");
    printf("DEBUG:                diffpairPathNum: %d\n", diffPairPathNum);
    printf("DEBUG:             pseudo-path number: %d\n", user_inputs->diffPairToPseudoNetMap[diffPairPathNum]);
    printf("DEBUG:   diffPairTerminalPitchMicrons: %6.3f microns\n", diffPairTerminalPitchMicrons);
    printf("DEBUG:                   currentLayer: %d\n", currentLayer);
    printf("DEBUG:           pseudo-path terminal: (%d,%d,%d)\n", pseudoTerm.X, pseudoTerm.Y, pseudoTerm.Z);
    printf("DEBUG:       diff-pair start-terminal: (%d,%d,%d)\n", mapInfo->start_cells[diffPairPathNum].X,
                                                                  mapInfo->start_cells[diffPairPathNum].Y,
                                                                  mapInfo->start_cells[diffPairPathNum].Z);
    printf("DEBUG:         diff-pair end-terminal: (%d,%d,%d)\n\n", mapInfo->end_cells[diffPairPathNum].X,
                                                                    mapInfo->end_cells[diffPairPathNum].Y,
                                                                    mapInfo->end_cells[diffPairPathNum].Z);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Determine the lowest and highest routing layer numbers that the routing
  // must traverse:
  int minLayerNum = min(pseudoTerm.Z, currentLayer);
  int maxLayerNum = max(pseudoTerm.Z, currentLayer);

  // Ensure that the 'allowedLayers' Boolean variable is set to TRUE for each
  // layer from the minLayerNum to the maxLayerNum:
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    if ((layer >= minLayerNum) && (layer <= maxLayerNum))  {
      terminalRestrictions->allowedLayers[layer] = TRUE;
    }
    else  {
      terminalRestrictions->allowedLayers[layer] = FALSE;
    }
  }  // End of for-loop for index 'layer'


  // For each layer on which routing can occur, calculate the radius from the
  // mid-point that routing will be allowed.
  for (int routableLayer = minLayerNum; routableLayer <= maxLayerNum; routableLayer++)  {

    float routingRadiusMicrons = 0.0;

    // Get design-rule number for this layer:
    int DR_num = cellInfo[pseudoTerm.X][pseudoTerm.Y][routableLayer].designRuleSet;

    // Get the design-rule subset number for this diff-pair path number and design-rule set:
    int DR_subset = user_inputs->designRuleSubsetMap[diffPairPathNum][DR_num];

    #ifdef DEBUG_calcRoutingRadiiAtTerminal
    if (DEBUG_ON)  {
      printf("DEBUG: **** Analyzing layer %d, with design-rule set %d and design-rule subset = %d.\n", routableLayer, DR_num, DR_subset);
    }
    #endif

    // The routing radius depends on a number of factors. There are 3 cases (A, B, and C), as described below:
    //   Case A: Routing is restricted to a single layer, which includes the terminal.
    //   Case B: There are multiple, routable layers allowed.
    //   Case C: There are multiple, routable layers allowed. One of these layers contains the terminal.
    //
    // Case A: routing is restricted to a single layer, which includes the terminals.
    if (minLayerNum == maxLayerNum)  {
      // The radius is the sum of these two quantities:
      //     (A) (Half of the diff-pair terminal pitch) + (linewidth of diff-pair net)
      //     (B) Half of the diff-pair trace pitch
      // Note that if any of the design-rule values are zero, then we substitute the cell size.
      float A = 0.5 * diffPairTerminalPitchMicrons
                                 +  max(user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons, user_inputs->cell_size_um);
      float B = 0.5 * user_inputs->diffPairPitchMicrons[diffPairPathNum][DR_num]
                                 +  max(user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons, user_inputs->cell_size_um);

      routingRadiusMicrons = A + B;

      #ifdef DEBUG_calcRoutingRadiiAtTerminal
      if (DEBUG_ON)  {
        printf("DEBUG: Case 'A' detected on layer %d. Routing radius = %6.3f microns\n", routableLayer, routingRadiusMicrons);
        printf("DEBUG:    This is sum of %6.3f (termPitch/2 + lineWidth) and %6.3f (diffPairPitch/2 + lineWidth)\n", A, B);
      }
      #endif

    }  // End of if-block for a single routable layer (Case 'A')

    // Cases B & C: there are multiple, routable layers allowed:
    else {

      // For Case 'B', the radius is the the sum of these two quantities:
      //     (1) (Half of the diff-pair trace pitch) + (linewidth of diff-pair net)
      //     (2) The maximum of the following two via-related quantities (a) and (b):
      //          (a) If routing needs to go 'up' to get to the shoulder path, the maximum of:
      //             (i)  Half of (Dvu + Svu) for the via-up ('vu') layer, and
      //             (ii) Half of (Dvd + Svd) for the layer 'above'
      //          (b) If routing needs to go 'down' to get to the shoulder path, the maximum of:
      //             (i)  Half of (Dvd + Svd) for the via-down ('vd') layer, and
      //             (ii) Half of (Dvu + Svu) for layer 'below'
      // If any of the design rules are zero, then we substitute the size of one cell.
      float trace_limited_radius = 0.5 * user_inputs->diffPairPitchMicrons[diffPairPathNum][DR_num]
                                    +  max(user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons, user_inputs->cell_size_um);

      #ifdef DEBUG_calcRoutingRadiiAtTerminal
      if (DEBUG_ON)  {
        printf("DEBUG: On layer %d, trace_limited_radius = %6.3f, consisting of 0.5 * diffPairPitchMicrons (%6.3f) plus lineWidthMicrons (%6.3f).\n",
                  routableLayer, trace_limited_radius, user_inputs->diffPairPitchMicrons[diffPairPathNum][DR_num],
                  user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons);
      }
      #endif

      // Define variables to calculating the radius that's limited by via diameter/spacing:
      float viaUp_limited_radius   = 0.0;
      float viaDown_limited_radius = 0.0;
      float via_limited_radius     = 0.0;

      // Check whether routing on the next higher routing layer is allowed:
      if ((routableLayer < mapInfo->numLayers - 1) &&  terminalRestrictions->allowedLayers[routableLayer + 1])  {

        // Calculate radius of allowable routing if it were limited by the up-via dimensions on the current
        // layer. This radius is half the sum of the via diameter plus spacing. If a design-rule value is zero,
        // then we substitute the size of one cell.
        viaUp_limited_radius   = 0.5 * (max(user_inputs->designRules[DR_num][DR_subset].width_um[VIA_UP], user_inputs->cell_size_um)
                                              + max(max(user_inputs->designRules[DR_num][DR_subset].space_um[VIA_UP][VIA_UP],
                                                        user_inputs->designRules[DR_num][DR_subset].space_um[TRACE][VIA_UP]),
                                                    user_inputs->cell_size_um) );

        // Calculate radius of allowable routing if it were limited by the down-via dimensions
        // on layer above. First get the design-rule number and subset for the layer above:
        int DR_num_above = cellInfo[pseudoTerm.X][pseudoTerm.Y][routableLayer + 1].designRuleSet;
        int DR_subset_above = user_inputs->designRuleSubsetMap[diffPairPathNum][DR_num_above];

        // Calculate half the sum of the via diameter plus spacing for layer above:
        viaDown_limited_radius = 0.5 * (max(user_inputs->designRules[DR_num_above][DR_subset_above].width_um[VIA_DOWN], user_inputs->cell_size_um)
                                              + max(max(user_inputs->designRules[DR_num_above][DR_subset_above].space_um[VIA_DOWN][VIA_DOWN],
                                                        user_inputs->designRules[DR_num_above][DR_subset_above].space_um[TRACE][VIA_DOWN]),
                                                    user_inputs->cell_size_um) );

        via_limited_radius = max(viaUp_limited_radius, viaDown_limited_radius);

        #ifdef DEBUG_calcRoutingRadiiAtTerminal
        if (DEBUG_ON)  {
          printf("DEBUG: Case 'B/C' detected on layer %d. trace_limited_radius = %6.3f, viaUp-limited radius = %6.3f, viaDown-limited radius = %6.3f microns\n",
                  routableLayer, trace_limited_radius, viaUp_limited_radius, viaDown_limited_radius);
        }
        #endif

      }  // End of if-block for checking whether next higher routing layer is routable

      // Check whether routing on the next lower routing layer is allowed:
      if ((routableLayer > 0) &&  terminalRestrictions->allowedLayers[routableLayer - 1])  {

        // Calculate radius of allowable routing if it were limited by the down-via dimensions
        // on the current layer. This radius is half the sum of the via diameter plus spacing:
        viaDown_limited_radius   =  0.5 * (max(user_inputs->designRules[DR_num][DR_subset].width_um[VIA_DOWN], user_inputs->cell_size_um)
                                                 + max(max(user_inputs->designRules[DR_num][DR_subset].space_um[VIA_DOWN][VIA_DOWN],
                                                           user_inputs->designRules[DR_num][DR_subset].space_um[TRACE][VIA_DOWN]),
                                                       user_inputs->cell_size_um));

        // If above variable is larger than other via-related diameters, capture the value in variable 'via_limited_radius':
        via_limited_radius = max(via_limited_radius, viaDown_limited_radius);

        // Calculate radius of allowable routing if it were limited by the up-via dimensions
        // on layer below. First get the design-rule number and subset for the layer below:
        int DR_num_below = cellInfo[pseudoTerm.X][pseudoTerm.Y][routableLayer - 1].designRuleSet;
        int DR_subset_below = user_inputs->designRuleSubsetMap[diffPairPathNum][DR_num_below];

        // Calculate half the sum of the via diameter plus spacing for layer below:
        viaUp_limited_radius   = 0.5 * (max(user_inputs->designRules[DR_num_below][DR_subset_below].width_um[VIA_UP], user_inputs->cell_size_um)
                                              + max(max(user_inputs->designRules[DR_num_below][DR_subset_below].space_um[VIA_UP][VIA_UP],
                                                        user_inputs->designRules[DR_num_below][DR_subset_below].space_um[TRACE][VIA_UP]),
                                                    user_inputs->cell_size_um) );

        // If above variable is larger than other via-related diameters, capture the value in variable 'via_limited_radius':
        via_limited_radius = max(via_limited_radius, viaUp_limited_radius);

        #ifdef DEBUG_calcRoutingRadiiAtTerminal
        if (DEBUG_ON)  {
          printf("DEBUG: Case 'B/C' detected on layer %d. trace_limited_radius = %6.3f, viaUp-limited radius = %6.3f, viaDown-limited radius = %6.3f microns\n",
                  routableLayer, trace_limited_radius, viaUp_limited_radius, viaDown_limited_radius);
        }
        #endif

      }  // End of if-block for checking whether next higher routing layer is routable

      // Calculate the routing radius as the sum of the trace-limited radius
      // and the via-limited radius:
      routingRadiusMicrons = trace_limited_radius  +  via_limited_radius;

      // Check if current layer contains the terminals. This differentiates Case 'B'
      // (calculated above) from Case 'C':
      if (routableLayer == pseudoTerm.Z) {
        // Case 'C': layer contains the terminals, and vias to other layers are required
        // Update the 'routingRadiusMicrons' value from Case 'B' (above) to also account
        // for the diff-pair terminal pitch:

        float A = routingRadiusMicrons;
        float B = 0.5 * diffPairTerminalPitchMicrons  +  max(user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons, user_inputs->cell_size_um);

        routingRadiusMicrons = A + B;

        #ifdef DEBUG_calcRoutingRadiiAtTerminal
        if (DEBUG_ON)  {
          printf("DEBUG: Case 'C' detected on layer %d. trace_limited_radius = %6.3f, via_limited_radius = %6.3f, routing radius = %6.3f microns\n",
                 routableLayer, trace_limited_radius, via_limited_radius, routingRadiusMicrons);
          printf("DEBUG:    This is sum of %6.3f microns and %6.3f microns\n", A, B);
        }
        #endif

      }  // End of if-block for Case 'C'

    }  // End of if-block for a routable layers (cases 'B' and 'C')

    // Add a safety buffer to the routing radius to account for rounding/pixilation
    // errors. The safety buffer is 5% or 1 cell, whichever is larger:
    routingRadiusMicrons = max(1.05 * routingRadiusMicrons, routingRadiusMicrons + user_inputs->cell_size_um);

    // Assign the routing radius to the array element in 'allowedRadiiMicrons[]' and
    // 'allowedRadiiCells[]'.
    terminalRestrictions->allowedRadiiMicrons[routableLayer] = routingRadiusMicrons;

    terminalRestrictions->allowedRadiiCells[routableLayer]   = routingRadiusMicrons / user_inputs->cell_size_um;

  }  // End of for-loop for index 'routableLayer'

  // Experimental code follows:
  //
  // Re-define the 'allowedRadii' values as the maximum radius calculated on any
  // of the allowed routing layers:
  //
  float max_allowed_radius_cells   = 0.0;
  float max_allowed_radius_microns = 0.0;
  for (int layer = minLayerNum; layer <= maxLayerNum; layer++)  {
    if (terminalRestrictions->allowedRadiiMicrons[layer] > max_allowed_radius_microns)  {
      max_allowed_radius_microns = terminalRestrictions->allowedRadiiMicrons[layer];
      max_allowed_radius_cells   = terminalRestrictions->allowedRadiiCells[layer];
    }
  }

  #ifdef DEBUG_calcRoutingRadiiAtTerminal
  if (DEBUG_ON)  {
    printf("DEBUG: Setting max_allowed_radius_microns to %6.3f and max_allowed_radius_cells to %6.3f on all layers.\n",
           max_allowed_radius_microns, max_allowed_radius_cells);
  }
  #endif

  for (int layer = minLayerNum; layer <= maxLayerNum; layer++)  {
    terminalRestrictions->allowedRadiiMicrons[layer] = max_allowed_radius_microns;
    terminalRestrictions->allowedRadiiCells[layer]   = max_allowed_radius_cells;
  }
  // End of experimental code


  #ifdef DEBUG_calcRoutingRadiiAtTerminal
  if (DEBUG_ON)  {
    printf("DEBUG: At end of function calcRoutingRadiiAtTerminal...\n");
    for (int i = 0; i < mapInfo->numLayers; i++)  {
      if (terminalRestrictions->allowedLayers[i]) {
        printf("DEBUG:      Layer: %d radius = %6.3f microns (%6.3f cells)\n", i,
                terminalRestrictions->allowedRadiiMicrons[i],
                terminalRestrictions->allowedRadiiCells[i]);
      }
      else {
        printf("DEBUG:      Layer %d: Routing is not allowed.\n", i);
      }
    }
    printf("DEBUG: Exiting function calcRoutingRadiiAtTerminal.\n\n");
  }
  #endif

}  // End of function 'calcRoutingRadiiAtTerminal'


//-----------------------------------------------------------------------------
// Name: swap_PN_congestion
// Desc: Swap the congestion around start terminals associated with pseudo-path
//       'pseudoPathNum', with associated diff-pair paths 'path_1_num' and
//       'path_2_num'. This function is intended to be called from within
//       function matchShoulderPathsToTerminals() if said function swaps the
//       start-terminals of pin-swappable diff-pair nets. This function changes
//       the values of 'pathNum' in the Congestion_t elements of the 'cellInfo'
//       3D matrix.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_swap_PN_congestion' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_swap_PN_congestion 1
#undef DEBUG_swap_PN_congestion

static void swap_PN_congestion(int pseudoPathNum, int path_1_num, int path_2_num, InputValues_t *user_inputs,
                               CellInfo_t ***cellInfo, MapInfo_t *mapInfo)  {

  #ifdef DEBUG_swap_PN_congestion
  // Variable used for debugging:
  int DEBUG_ON = FALSE;

  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  if (   (mapInfo->current_iteration >= 186) && (mapInfo->current_iteration <= 190)
      && (pseudoPathNum == 35))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in swap_PN_congestion() because specific requirements were met in iteration %d.\n\n",
           mapInfo->current_iteration);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE

  printf("DEBUG: Checking for congestion to swap between diff-pair paths %d and %d (pseudo-path: %d)\n",
         path_1_num, path_2_num, pseudoPathNum);
  #endif

  //
  // Swap the congestion in the map for diff-pair children of pseudo-path:
  //

  // Get the coordinates of the start-terminal of the pseudo-path:
  Coordinate_t pseudoStartTerm = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);

  // Define a 'swapRestrictions' structure to restrict congestion-swapping to a
  // small region around the pseudo-path's start-terminal:
  RoutingRestriction_t swapRestrictions;

  //
  // Iterate over each layer in the map:
  //
  for (int z = 0; z < mapInfo->numLayers; z++)  {

    // Call function 'calcRoutingRadiiAtTerminal' to determine how far around this
    // pseudo-net's start-terminal that findPath() would be used for routing the nets.
    // Use this radius to determine where congestion needs to be swapped:
    calcRoutingRadiiAtTerminal(&swapRestrictions, path_1_num, mapInfo->diffPairStartTermPitchMicrons[path_1_num],
                               z, pseudoStartTerm, user_inputs, cellInfo, mapInfo);

    #ifdef DEBUG_swap_PN_congestion
    if (DEBUG_ON)  {
      printf("DEBUG: For layer %d, about to swap congestion between paths %d and %d within %.3f cells of pseudo-terminal (%d,%d,%d)\n",
           z, path_1_num, path_2_num, swapRestrictions.allowedRadiiCells[z],
           pseudoStartTerm.X, pseudoStartTerm.Y, pseudoStartTerm.Z);
    }
    #endif

    int swap_radius = (int)(swapRestrictions.allowedRadiiCells[z] + 1);
    int swap_radius_squared = swap_radius * swap_radius;

    // Iterate over the X/Y coordinates within 'swap_radius' of the pseudo-path's start-terminal:
    int min_X = pseudoStartTerm.X - swap_radius;
    int max_X = pseudoStartTerm.X + swap_radius;
    int min_Y = pseudoStartTerm.Y - swap_radius;
    int max_Y = pseudoStartTerm.Y + swap_radius;

    for (int x = min_X; x <= max_X; x++)  {
      for (int y = min_Y; y <= max_Y; y++)  {

        // Skip this point if it's outside of the map:
        if(XY_coords_are_outside_of_map(x, y, mapInfo))  {
          continue;
        }

        // Skip this point if it's farther from the pseudo-terminal than 'swap_radius':
        int distance_squared = (x - pseudoStartTerm.X)*(x - pseudoStartTerm.X) + (y - pseudoStartTerm.Y)*(y - pseudoStartTerm.Y);
        if (distance_squared > swap_radius_squared)  {
          continue;
        }  // End of if-block for

        // We got here, so this cell is near the pseudo-path's start-terminal. Iterate over the
        // congestion elements at (x,y,z) and swap the congestion between the two diff-pair paths:
        for (int path_index = 0; path_index < cellInfo[x][y][z].numTraversingPaths; path_index++)  {

          int congestion_path_number = cellInfo[x][y][z].congestion[path_index].pathNum;

          // Compare the path number of the congestion to the path number of diff-pair #1.
          // If they're equal, then re-assign the congestion path number to that of
          // diff-pair path #2:
          if (congestion_path_number == path_1_num)  {
            cellInfo[x][y][z].congestion[path_index].pathNum = path_2_num;
          }  // End of if-block for congestion matching path_1_num

          // Compare the path number of the congestion to the path number of diff-pair #2.
          // If they're equal, then re-assign the congestion path number to that of
          // diff-pair path #1:
          if (congestion_path_number == path_2_num)  {
            cellInfo[x][y][z].congestion[path_index].pathNum = path_1_num;
          }  // End of if-block for congestion matching path_2_num

        }  // End of for-loop for index 'path_index'

      }  // End of for-loop for index 'y'
    }  // End of for-loop for index 'x'
  }  // End of for-loop for index 'layer'

}  // End of function 'swap_PN_congestion'


//-----------------------------------------------------------------------------
// Name: getTraceCongestionAtConnectionTerminal
// Desc: For diff-pair path-number 'pathNum', determine whether the (x,y,z)
//       'coordinate' is surrounded by more TRACE congestion from 'pathNum'
//       (with same design-rule subset as 'pathNum), or from its diff-pair
//       partner 'partnerPathNum'. This function returns the path number
//       that has more congestion at the (x,y,z) coordinate. If there is an
//       approximately equal amount of congestion (within 2%) from both paths
//       at the coordinate, this function returns '-1'. The amounts of
//       detected congestion from the path and its partner are stored in
//       variables pathTraversalsTimes100_path and
//       pathTraversalsTimes100_partner.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_getTraceCongestionAtConnectionTerminal' and re-compile if
// you want verbose debugging print-statements enabled:
//
// #define DEBUG_getTraceCongestionAtConnectionTerminal 1
#undef DEBUG_getTraceCongestionAtConnectionTerminal

static int getTraceCongestionAtConnectionTerminal(int pathNum, int partnerPathNum, Coordinate_t coordinate,
                                                  int *pathTraversalsTimes100_path, int *pathTraversalsTimes100_partner,
                                                  MapInfo_t *mapInfo, CellInfo_t ***cellInfo, InputValues_t *user_inputs)  {

  // Get the x/y/z coordinates of the point, and get the design-rule set
  // and subset for coordinate and path number:
  int center_DR_num, center_DR_subset, centerX, centerY, centerZ, radius;
  centerX = coordinate.X;
  centerY = coordinate.Y;
  centerZ = coordinate.Z;
  center_DR_num    = cellInfo[centerX][centerY][centerZ].designRuleSet;
  center_DR_subset = user_inputs->designRuleSubsetMap[pathNum][center_DR_num];

  // Calculate the radius as half the diff-pair pitch (in cell units):
  radius = user_inputs->diffPairPitchCells[pathNum][center_DR_num] / 2;

  #ifdef DEBUG_getTraceCongestionAtConnectionTerminal
  int DEBUG_ON = FALSE;
  if (   (mapInfo->current_iteration >= 1) && (mapInfo->current_iteration <= 44)
      && ((pathNum == 2) || (pathNum == 3) || (partnerPathNum == 2) || (partnerPathNum == 3)))  {
    DEBUG_ON = TRUE;
    printf("\nDEBUG: (thread %2d) Entered getTraceCongestionAtConnectionTerminal for path %d and partner %d. Radius = %d cells (%d/2) around (%d,%d,%d) in iter %d.\n\n",
           omp_get_thread_num(), pathNum, partnerPathNum, radius, user_inputs->diffPairPitchCells[pathNum][center_DR_num],
           centerX, centerY, centerZ, mapInfo->current_iteration);
  }
  #endif

  // Initialize variables that will contain the amount of congestion ('path traversals X 100')
  // from 'pathNum' and 'partnerPathNum':
  *pathTraversalsTimes100_path    = 0;
  *pathTraversalsTimes100_partner = 0;

  // Iterate over the cells around 'coordinate':
  for (int x = centerX - radius; x <= centerX + radius; x++)  {
    for (int y = centerY - radius; y <= centerY + radius; y++)  {
      // Confirm that the (x,y) location is within the map:
      if (! XY_coords_are_outside_of_map(x, y, mapInfo))  {

        // Determine how many paths traverse the cell:
        const unsigned int path_count = cellInfo[x][y][centerZ].numTraversingPaths;

        // Determine the design-rule set and subsets at the cell. These will likely be the
        // same design-rule set and subset as at the center-point, but this isn't guaranteed:
        int point_DR_num    = cellInfo[x][y][centerZ].designRuleSet;
        int point_DR_subset;
        int center_DR_subset_mapped_to_point;
        if (point_DR_num != center_DR_num)  {
          point_DR_subset = user_inputs->designRuleSubsetMap[pathNum][point_DR_num];
          center_DR_subset_mapped_to_point = user_inputs->foreign_DR_subset[center_DR_num][center_DR_subset][point_DR_num];
        }
        else  {
          center_DR_subset_mapped_to_point = point_DR_subset = center_DR_subset;
        }

        // Check all paths that traverse the cell:
        for (int path_index = 0; path_index < path_count; path_index++)  {

          // Recognize only the congestion with the same design-rule subset as the diff-pairs'
          // subset, and only with shape-type 'TRACE', excluding VIA_UP and VIA_DOWN congestion:
          if (   (cellInfo[x][y][centerZ].congestion[path_index].shapeType == TRACE)
              && (cellInfo[x][y][centerZ].congestion[path_index].DR_subset == center_DR_subset_mapped_to_point))  {

            // Get the path number of the congestion for the current path index:
            int congestion_pathNum   = cellInfo[x][y][centerZ].congestion[path_index].pathNum;

            // Add the 'congestion' (pathTraversalsTimes100) to the appropriate summation variables:
            if (congestion_pathNum == pathNum)  {
              *pathTraversalsTimes100_path += cellInfo[x][y][centerZ].congestion[path_index].pathTraversalsTimes100;
            }
            else if (congestion_pathNum == partnerPathNum)  {
              *pathTraversalsTimes100_partner += cellInfo[x][y][centerZ].congestion[path_index].pathTraversalsTimes100;
            }
          }  // End of if-block for shapeType == TRACE
        }  // End of for-loop for index 'path_index'
      }  // End of if-block for cell being within the map
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'x'

  // Calculate the appropriate return-value based on the relative congestion found from the two
  // diff-pair paths. The factors of 1.5 in the inequalities ensure there's at least a 50% difference
  // in the congestion values for this function to conclude that there's more congestion from
  // one path than the other:
  int return_value = -1;  // Default value is -1 if both paths have approximately the same congestion
  if (*pathTraversalsTimes100_path > *pathTraversalsTimes100_partner * 1.5)  {
    return_value = pathNum;
  } else if (*pathTraversalsTimes100_partner > *pathTraversalsTimes100_path * 1.5)  {
    return_value = partnerPathNum;
  }

  #ifdef DEBUG_getTraceCongestionAtConnectionTerminal
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) At end of getTraceCongestionAtConnectionTerminal during iteration %d:\n", omp_get_thread_num(), mapInfo->current_iteration);
    printf("DEBUG: (thread %2d)                          pathNum = %d\n", omp_get_thread_num(), pathNum);
    printf("DEBUG: (thread %2d)                   partnerPathNum = %d\n", omp_get_thread_num(), partnerPathNum);
    printf("DEBUG: (thread %2d)                       coordinate = (%d,%d,%d)\n", omp_get_thread_num(), centerX, centerY, centerZ);
    printf("DEBUG: (thread %2d)                           radius = %d cells\n", omp_get_thread_num(), radius);
    printf("DEBUG: (thread %2d)     *pathTraversalsTimes100_path = %'d\n", omp_get_thread_num(), *pathTraversalsTimes100_path);
    printf("DEBUG: (thread %2d)  *pathTraversalsTimes100_partner = %'d\n", omp_get_thread_num(), *pathTraversalsTimes100_partner);
    printf("DEBUG: (thread %2d)                     return_value = %d\n\n", omp_get_thread_num(), return_value);
  }
  #endif

  // Return the return-value to the calling program:
  return(return_value);

}  // End of function 'getTraceCongestionAtConnectionTerminal'


//-----------------------------------------------------------------------------
// Name: optimize_using_geometry
// Desc: For a given diff-pair connection, this function reads the coordinates
//       of the four start/end terminals. If all four coordinates are on the
//       same routing layer, then it calculates a 'symmetryRatio' for the
//       shoulder connection structure 'connection'. If the symmetryRatio is
//       less than 0.45 or greater than 0.55, the function updates the 'swap'
//       value in the shoulder connection structure and also returns 'TRUE' to
//       indicate that it found a conclusive result. If the symmetryRatio is
//       between 0.45 and 0.55, then the function will not modify the 'swap'
//       value, but will return FALSE, indicating that it did not find a
//       conclusive result.
//-----------------------------------------------------------------------------
static int optimize_using_geometry(ShoulderConnection_t * connection)  {

  // 'success_flag' is Boolean flag indicating whether this function found a
  // conclusive result.
  int success_flag;

  // If the connection spans more than a single routing layer, then return
  // from this function with a value of FALSE:
  if (   (connection->startCoord_1.Z != connection->endCoord_1.Z)
      || (connection->startCoord_1.Z != connection->endCoord_2.Z)
      || (connection->startCoord_1.Z != connection->startCoord_2.Z))  {
    success_flag = FALSE;
  }  // End of if-block for coordinates not being on same Z-layer
  else  {
    // We got here, so all four coordinates are on the same routing layer.

    // Calculate the total lateral straight-line distance if start-coordinate #1
    // were wired to end-coordinate #1, and if start-coordinate #2 were wired to
    // end-coordinate #2. This is the 'unswapped' distance:
    double unswapped_distance
        =   calc_2D_Pythagorean_distance_ints(connection->startCoord_1.X, connection->startCoord_1.Y,
                                               connection->endCoord_1.X,   connection->endCoord_1.Y)
          + calc_2D_Pythagorean_distance_ints(connection->startCoord_2.X, connection->startCoord_2.Y,
                                               connection->endCoord_2.X,   connection->endCoord_2.Y);

    // Calculate the total lateral straight-line distance if start-coordinate #1
    // were wired to end-coordinate #2, and if start-coordinate #2 were wired to
    // end-coordinate #1. This is the 'swapped' distance:
    double swapped_distance
        =   calc_2D_Pythagorean_distance_ints(connection->startCoord_1.X, connection->startCoord_1.Y,
                                               connection->endCoord_2.X,   connection->endCoord_2.Y)
          + calc_2D_Pythagorean_distance_ints(connection->startCoord_2.X, connection->startCoord_2.Y,
                                               connection->endCoord_1.X,   connection->endCoord_1.Y);

    // Calculate a symmetry ratio:
    connection->symmetryRatio = unswapped_distance / (unswapped_distance + swapped_distance);

    // Based on the symmetryRatio value, set the 'swap' value and the return value:
    if (connection->symmetryRatio <= 0.45)  {
      connection->swap = FALSE;
      success_flag = TRUE;
    }
    else if (connection->symmetryRatio >= 0.55)  {
      connection->swap = TRUE;
      success_flag = TRUE;
    }
    else  {
      success_flag = FALSE;
    }  // End of if/else-if/else block

  }  // End of else-block in which all 4 coordinates are on same routing layer

  return(success_flag);
}  // End of function 'optimize_using_geometry'

//-----------------------------------------------------------------------------
// Name: optimize_using_congestion
// Desc: For a given diff-pair connection, this function reads the amount of
//       congestion around each of the four connection terminals to determine
//       which connection-terminals were wired together in previous iterations.
//       If this determination can be conclusively made, then this function
//       returns TRUE after setting the 'swap' and 'symmetryRatio' elements in
//       the 'connection' structure. If the previous wiring cannot be
//       conclusively determined from the congestion, then this function
//       returns FALSE.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_optimize_using_congestion' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_optimize_using_congestion 1
#undef DEBUG_optimize_using_congestion

static int optimize_using_congestion(int path_1, int path_2, ShoulderConnection_t * connection,
                                     MapInfo_t *mapInfo, CellInfo_t ***cellInfo,
                                     InputValues_t *user_inputs)  {

  #ifdef DEBUG_optimize_using_congestion
  int DEBUG_ON = FALSE;
  if (   (mapInfo->current_iteration >= 59) && (mapInfo->current_iteration <= 60)
      && ((path_1 == 0) || (path_2 == 0) || (path_1 == 1) || (path_2 == 1)))  {
    DEBUG_ON = TRUE;
    printf("\nDEBUG: (thread %2d) Entered optimize_using_congestion in iteration %d for path %d and partner %d.\n\n",
           omp_get_thread_num(), mapInfo->current_iteration, path_1, path_2);
  }
  #endif

  // 'success_flag' is Boolean flag indicating whether this function found a
  // conclusive result.
  int success_flag = FALSE;

  // 'symmetry_ratio' is <0.5 if the congestion is consistent with an unswapped
  // wiring configuration, and >0.5 for a swapped wiring configuration.
  double symmetry_ratio;

  //
  // Get the congestion in the map near the start- and end-terminals of the connection:
  //
  int path_1_cong_near_start_1, path_1_cong_near_start_2, path_1_cong_near_end_1, path_1_cong_near_end_2,
      path_2_cong_near_start_1, path_2_cong_near_start_2, path_2_cong_near_end_1, path_2_cong_near_end_2;
  int congNearStart_1
      = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->startCoord_1,
                                               &path_1_cong_near_start_1, &path_2_cong_near_start_1,
                                               mapInfo, cellInfo, user_inputs);
  int congNearStart_2
      = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->startCoord_2,
                                               &path_1_cong_near_start_2, &path_2_cong_near_start_2,
                                               mapInfo, cellInfo, user_inputs);
  int congNearEnd_1
      = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->endCoord_1,
                                               &path_1_cong_near_end_1, &path_2_cong_near_end_1,
                                               mapInfo, cellInfo, user_inputs);
  int congNearEnd_2
      = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->endCoord_2,
                                               &path_1_cong_near_end_2, &path_2_cong_near_end_2,
                                               mapInfo, cellInfo, user_inputs);

  // Assuming that the routing has been stable for many iterations, the amounts of congestion near
  // each terminal could take one of the following four configurations, labeled A, B, C, and D in the
  // table below. In this table, Sab refers to the congestion near start-terminal 'a' from path 'b'.
  // Eab refers to the congestion near end-terminal 'a' from path 'b'. Tx refers to the wire path
  // number that has the higher congestion near a terminal.
  //--------------------------------------------------------|---------------------|---------------------|-----------------|
  //                                                        |    Start-terminals  |     End-terminals   |    Symmetry     |
  //                                                        |S11 S12 S21 S22 T1 T2|E11 E12 E21 E22 T1 T2|      Ratio      |
  //--------------------------------------------------------|---------------------|---------------------|-----------------|
  // Wiring configuration A:                                |                     |                     |                 |
  //  Start-terminal #1 o-----Path#1-----o End-terminal #1  |                     |                     | S12+S21+E12+E21 |
  //                                                        | hi low low  hi  1  2| hi low low  hi  1  2| --------------- |
  //  Start-terminal #2 o-----Path#2-----o End-terminal #2  |                     |                     |  (sum of all 8) |
  //--------------------------------------------------------|---------------------|---------------------|-----------------|
  // Wiring configuration B:                                |                     |                     |                 |
  //  Start-terminal #1 o-----Path#2-----o End-terminal #1  |                     |                     | S11+S22+E11+E22 |
  //                                                        |low  hi  hi low  2  1|low  hi  hi low  2  1| --------------- |
  //  Start-terminal #2 o-----Path#1-----o End-terminal #2  |                     |                     |  (sum of all 8) |
  //--------------------------------------------------------|---------------------|---------------------|-----------------|
  // Wiring configuration C:                                |                     |                     |                 |
  //  Start-terminal #1 o--Path#1--\ /---o End-terminal #1  |                     |                     | S11+S22+E12+E21 |
  //                                X                       | hi low low  hi  1  2|low  hi  hi low  2  1| --------------- |
  //  Start-terminal #2 o--Path#2--/ \---o End-terminal #2  |                     |                     |  (sum of all 8) |
  //--------------------------------------------------------|---------------------|---------------------|-----------------|
  // Wiring configuration D:                                |                     |                     |                 |
  //  Start-terminal #1 o--Path#2--\ /---o End-terminal #1  |                     |                     | S12+S21+E11+E22 |
  //                                X                       |low  hi  hi low  2  1| hi low low  hi  1  2| --------------- |
  //  Start-terminal #2 o--Path#1--/ \---o End-terminal #2  |                     |                     |  (sum of all 8) |
  //--------------------------------------------------------|---------------------|---------------------|-----------------|

  // Sum the congestion amounts associated with an unswapped wiring configuration and
  // a swapped wiring configuration at each terminal:
  int unswapped_startTerm_congestion = path_1_cong_near_start_1 + path_2_cong_near_start_2;
  int unswapped_endTerm_congestion   = path_1_cong_near_end_1   + path_2_cong_near_end_2;
  int swapped_startTerm_congestion   = path_1_cong_near_start_2 + path_2_cong_near_start_1;
  int swapped_endTerm_congestion     = path_1_cong_near_end_2   + path_2_cong_near_end_1;
  int total_congestion  =   unswapped_startTerm_congestion + unswapped_endTerm_congestion
                          +   swapped_startTerm_congestion +   swapped_endTerm_congestion;

  #ifdef DEBUG_optimize_using_congestion
  if (DEBUG_ON)  {
    printf("\nDEBUG: In optimize_using_congestion in iteration %d\n", mapInfo->current_iteration);
    printf(  "DEBUG:          path_1_cong_near_start_1 = %d\n", path_1_cong_near_start_1);
    printf(  "DEBUG:            path_1_cong_near_end_1 = %d\n", path_1_cong_near_end_1);
    printf(  "DEBUG:          path_2_cong_near_start_2 = %d\n", path_2_cong_near_start_2);
    printf(  "DEBUG:            path_2_cong_near_end_2 = %d\n", path_2_cong_near_end_2);
    printf(  "DEBUG:          path_1_cong_near_start_2 = %d\n", path_1_cong_near_start_2);
    printf(  "DEBUG:            path_1_cong_near_end_2 = %d\n", path_1_cong_near_end_2);
    printf(  "DEBUG:          path_2_cong_near_start_1 = %d\n", path_2_cong_near_start_1);
    printf(  "DEBUG:            path_2_cong_near_end_1 = %d\n\n", path_2_cong_near_end_1);
  }
  #endif

  // If the congestion near any of the four connection-terminals was the same between
  // the two diff-pair paths, then return FALSE from this function.
  if ((congNearStart_1 == -1) || (congNearStart_2 == -1) || (congNearEnd_1 == -1) || (congNearEnd_2 == -1))  {
    success_flag = FALSE;

    #ifdef DEBUG_optimize_using_congestion
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In optimize_using_congestion, congestion for paths %d & %d was comparable for at least 1 terminal.\n",
             omp_get_thread_num(), path_1, path_2);
    }
    #endif
  }
  else  {
    // We got here, so there are differences in the congestion amount at each of
    // the four terminals. Determine if the congestion amounts are consistent
    // with an unswapped or swapped wiring configuration:
    //
    // Determine which of the 4 wiring configurations (if any) best matches the congestion
    // pattern:
    //
    if ((congNearStart_1 == congNearEnd_1) && (congNearStart_2 == congNearEnd_2))  {
      // We got here, so the congestion amounts at the four connection-terminals are
      // consistent with an UNSWAPPED connection. We next calculate a symmetry ratio,
      // based on which of two possible wiring configurations (A or B from table above)
      // resulted in the unswapped state:
      if ((congNearStart_1 == path_1) && (congNearStart_2 == path_2))  {
        // We got here, so the configuration matches 'A' from table above.
        symmetry_ratio = (double)(swapped_startTerm_congestion + swapped_endTerm_congestion) / total_congestion;
        if (symmetry_ratio <= 0.45)  {
          success_flag              = TRUE;
          connection->swap          = NOT_SWAPPED;
          connection->symmetryRatio = symmetry_ratio;
        }
        else  {
          success_flag              = FALSE;
        }
      }  // End of if-block for wiring configuration 'A' from table above
      else if ((congNearStart_1 == path_2) && (congNearStart_2 == path_1))  {
        // We got here, so the configuration matches 'B' from table above.
        symmetry_ratio = (double)(unswapped_startTerm_congestion + unswapped_endTerm_congestion) / total_congestion;
        if (symmetry_ratio <= 0.45)  {
          success_flag              = TRUE;
          connection->swap          = NOT_SWAPPED;
          connection->symmetryRatio = symmetry_ratio;
        }
        else  {
          success_flag              = FALSE;
        }
      }  // End of if-block for wiring configuration 'B' from table above
      else  {
        // We got here, so the connection's congestion does not conclusively point to either a swapped or non-swapped
        // wiring configuration. Issue an informational message:
        printf("\nINFO: (thread %2d) Function optimize_using_congestion could not conclusively determine the wiring configuration in iteration %d\n",
               omp_get_thread_num(), mapInfo->current_iteration);
        printf(  "INFO: (thread %2d) of diff-pair path %d and partner %d. Other methods will be used to optimize this diff-pair connection.\n",
               omp_get_thread_num(), path_1, path_2);
        success_flag = FALSE;
      }
    }  // End of if-block for congestion being consistent with unswapped connection
    else if ((congNearStart_1 == congNearEnd_2) && (congNearStart_2 == congNearEnd_1))  {
      // We got here, so the congestion amounts at the four connection-terminals are
      // consistent with a SWAPPED connection. We next calculate a symmetry ratio,
      // based on which of two possible wiring configurations (C or D from table above)
      // resulted in the swapped state:
      if ((congNearStart_1 == path_1) && (congNearStart_2 == path_2))  {
        // We got here, so the configuration matches 'C' from table above.
        symmetry_ratio = (double)(unswapped_startTerm_congestion + swapped_endTerm_congestion) / total_congestion;
        if (symmetry_ratio >= 0.55)  {
          success_flag              = TRUE;
          connection->swap          = SWAPPED;
          connection->symmetryRatio = symmetry_ratio;
        }
        else  {
          success_flag              = FALSE;
        }
      }  // End of if-block for wiring configuration 'C' from table above
      else if ((congNearStart_1 == path_2) && (congNearStart_2 == path_1))  {
        // We got here, so the configuration matches 'D' from table above.
        symmetry_ratio = (double)(swapped_startTerm_congestion + unswapped_endTerm_congestion) / total_congestion;
        if (symmetry_ratio >= 0.55)  {
          success_flag              = TRUE;
          connection->swap          = SWAPPED;
          connection->symmetryRatio = symmetry_ratio;
        }
        else  {
          success_flag              = FALSE;
        }
      }  // End of if-block for wiring configuration 'D' from table above
      else  {
        // We got here, so the connection's congestion does not conclusively point to either a swapped or non-swapped
        // wiring configuration. Issue an informational message:
        printf("\nINFO: (thread %2d) Function optimize_using_congestion could not conclusively determine the wiring configuration in iteration %d\n",
               omp_get_thread_num(), mapInfo->current_iteration);
        printf(  "INFO: (thread %2d) of diff-pair path %d and partner %d. Other methods will be used to optimize this diff-pair connection.\n",
               omp_get_thread_num(), path_1, path_2);
        success_flag = FALSE;
      }
    }  // End of if-block for congestion being consistent with swapped connection
  }  // End of else-block for congestion being different at each connection-terminal

  //
  // DEBUG code follows:
  //
  #ifdef DEBUG_optimize_using_congestion
  if (DEBUG_ON)  {
  if (success_flag)  {
      printf("DEBUG: (thread %d) Exiting optimize_using_congestion with success_flag = %d, symmetryRatio = %.3f, swap = %d.\n",
             omp_get_thread_num(), success_flag, connection->symmetryRatio, connection->swap);
    }
    else  {
      printf("DEBUG: (thread %d) Exiting optimize_using_congestion with success_flag = %d.\n",
             omp_get_thread_num(), success_flag);
    }
  }
  #endif

  return(success_flag);
}  // End of function 'optimize_using_congestion'


//-----------------------------------------------------------------------------
// Name: connection_has_DRCs_in_previous_iteration
// Desc: For a given diff-pair connection, this function determines whether
//       any of the routing layers associated with the connection had design-
//       rule violations in the previous iteration on the two diff-pair paths
//       of the connection. The function returns TRUE if the previous iteration
//       had 1 or more DRC cells, and FALSE otherwise.
//-----------------------------------------------------------------------------
static int connection_has_DRCs_in_previous_iteration(int path_1, int path_2, ShoulderConnection_t connection,
                                                     RoutingMetrics_t *routability)  {

  // Define Boolean variable that will be returned from this function:
  int result = FALSE;

  // Get the minimum layer number of all four connection terminals:
  int min_layer_num = connection.startCoord_1.Z;
  min_layer_num = min(min_layer_num, connection.startCoord_2.Z);
  min_layer_num = min(min_layer_num, connection.endCoord_1.Z);
  min_layer_num = min(min_layer_num, connection.endCoord_2.Z);

  // Get the maximum layer number of all four connection terminals:
  int max_layer_num = connection.startCoord_1.Z;
  max_layer_num = max(max_layer_num, connection.startCoord_2.Z);
  max_layer_num = max(max_layer_num, connection.endCoord_1.Z);
  max_layer_num = max(max_layer_num, connection.endCoord_2.Z);

  // Cycle through all layers between the minimum and maximum connection
  // layers (inclusive) to determine whether any have design-rule
  // violations on either of the two diff-pair paths:
  for (int layer = min_layer_num; layer <= max_layer_num; layer++)  {
    if (   routability->path_DRC_cells_by_layer[path_1][layer]
        || routability->path_DRC_cells_by_layer[path_2][layer])  {
      // We got here, so one or both of the diff-pair paths has a design-rule
      // rule violation on layer 'layer'. Set the 'result' variable to TRUE
      // and break out of the for-loop:
      result = TRUE;
      break;
    }  // End of if-block for detecting a DRC
  }  // End of for-loop for index 'layer'

  return(result);
}  // End of function 'connection_has_DRCs_in_previous_iteration'


//-----------------------------------------------------------------------------
// Name: decideWhetherToSwapConnection
// Desc: Based on the G-cost metrics for the shoulder-connection 'connection',
//       decide whether it's best to swap the two diff-pair connections. This
//       function modifies the Boolean element connection->swap and also
//       calculates the floating-point value of connection->symmetryRatio.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_decideWhetherToSwapConnection' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_decideWhetherToSwapConnection 1
#undef DEBUG_decideWhetherToSwapConnection

static void decideWhetherToSwapConnection(unsigned long Gcost[2][2], ShoulderConnection_t * connection,
                                          int path_1, int path_2, MapInfo_t *mapInfo, CellInfo_t ***cellInfo,
                                          InputValues_t *user_inputs)  {

  #ifdef DEBUG_decideWhetherToSwapConnection
  // DEBUG code follows:
  //
  // Variable used for debugging:
  int DEBUG_ON = FALSE;

  // Get the current thread number, for use in print/DEBUG statements:
  int thread_num = omp_get_thread_num();

  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  if (   (mapInfo->current_iteration >= 59) && (mapInfo->current_iteration <= 60)
      && ((path_1 == 0) || (path_2 == 0) || (path_1 == 1) || (path_2 == 1)))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in decideWhetherToSwapConnection() because specific requirements were met in iteration %d.\n",
           mapInfo->current_iteration);
    printf("DEBUG: Analyzing metrics for diff-pair paths %d and %d.\n\n", path_1, path_2);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif


  // Calculate the 'symmetryRatio', which is <0.5 if unswapped connection has better metrics:
  unsigned long unswapped_Gcost = Gcost[NOT_SWAPPED][0] + Gcost[NOT_SWAPPED][1];
  unsigned long swapped_Gcost   = Gcost[SWAPPED][0]     + Gcost[SWAPPED][1];
  connection->symmetryRatio = (double) unswapped_Gcost / (unswapped_Gcost + swapped_Gcost);

  #ifdef DEBUG_decideWhetherToSwapConnection
  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) In decideWhetherToSwapConnection after running findPath in iteration %d:\n",
           thread_num, mapInfo->current_iteration);
    printf("DEBUG: (thread %2d) unswapped_Gcost = %'lu   swapped_Gcost = %'lu  symmetryRatio = %.5f\n", thread_num,
           unswapped_Gcost, swapped_Gcost, connection->symmetryRatio);
  }
  #endif

  //
  // If the symmetry ratio deviates significantly from 0.5, then assign the 'swap' value
  // based solely on this ratio:
  //
  if (connection->symmetryRatio <= 0.45)  {
    connection->swap = FALSE;

    #ifdef DEBUG_decideWhetherToSwapConnection
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Assigned swap to FALSE because symmetryRatio <= 0.45.\n", thread_num);
    }
    #endif
  }
  else if (connection->symmetryRatio >= 0.55)  {
    connection->swap = TRUE;

    #ifdef DEBUG_decideWhetherToSwapConnection
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Assigned swap to TRUE because symmetryRatio >= 0.55.\n", thread_num);
    }
    #endif

  }
  else  {
    // We got here, so the G-costs of the unswapped and swapped configurations are within
    // ~20% of each other, i.e., the symmetryRatio is between 0.45 and 0.55. So we determine
    // the new swap-configuration to be consistent with previous configurations, as inferred
    // from congestion in the map near the start- and end-terminals of the connection:
    int pathCongestion, partnerCongestion;
    int congNearStart_1
        = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->startCoord_1,
                                                 &pathCongestion, &partnerCongestion,
                                                 mapInfo, cellInfo, user_inputs);
    int congNearStart_2
        = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->startCoord_2,
                                                 &pathCongestion, &partnerCongestion,
                                                 mapInfo, cellInfo, user_inputs);
    int congNearEnd_1
        = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->endCoord_1,
                                                 &pathCongestion, &partnerCongestion,
                                                 mapInfo, cellInfo, user_inputs);
    int congNearEnd_2
        = getTraceCongestionAtConnectionTerminal(path_1, path_2, connection->endCoord_2,
                                                 &pathCongestion, &partnerCongestion,
                                                 mapInfo, cellInfo, user_inputs);

    #ifdef DEBUG_decideWhetherToSwapConnection
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) congNearStart_1 = %d, congNearStart_2 = %d, congNearEnd_1 = %d, congNearEnd_2 = %d\n\n",
             thread_num, congNearStart_1, congNearStart_2, congNearEnd_1, congNearEnd_2);
    }
    #endif

    // Check for evidence of an unswapped connection:
    //     (a) All 4 connection-terminals are consistent with unswapped connection and
    //         all 4 terminals have valid congestion readings.
    // or  (b) 2 of the connection-terminals, start_1 and end_1, are consistent with unswapped
    //         connections, and at least one of the other 2 have invalid congestion readings.
    // or  (c) 2 of the connection-terminals, start_2 and end_2, are consistent with unswapped
    //         connections, and at least one of the other 2 have invalid congestion readings.
    //
    if (   (   (congNearStart_1 == congNearEnd_1) && (congNearStart_1 != -1) && (congNearEnd_1 != -1)    // Item (a) above
            && (congNearStart_2 == congNearEnd_2) && (congNearStart_2 != -1) && (congNearEnd_2 != -1) )  // Item (a), continued

        || (   (congNearStart_1 == congNearEnd_1) && (congNearStart_1 != -1) && (congNearEnd_1 != -1)    // Item (b) above
            && ((congNearStart_2 == -1) || (congNearEnd_2 == -1)))                                       // Item (b), continued

        || (   (congNearStart_2 == congNearEnd_2) && (congNearStart_2 != -1) && (congNearEnd_2 != -1)    // Item (c) above
            && ((congNearStart_1 == -1) || (congNearEnd_1 == -1))))                                      // Item (c), continued
    {
      connection->swap = FALSE;

      #ifdef DEBUG_decideWhetherToSwapConnection
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Assigned swap to FALSE because connection was previously unswapped.\n", thread_num);
      }
      #endif
    }

    // Otherwise, check for evidence of a swapped connection:
    //     (a) All 4 connection-terminals are consistent with swapped connection and
    //         all 4 terminals have valid congestion readings.
    // or  (b) 2 of the connection-terminals, start_1 and end_2, are consistent with swapped
    //         connections, and at least one of the other 2 have invalid congestion readings.
    // or  (c) 2 of the connection-terminals, start_2 and end_1, are consistent with swapped
    //         connections, and at least one of the other 2 have invalid congestion readings.
    //
    else if (   (   (congNearStart_1 == congNearEnd_2) && (congNearStart_1 != -1) && (congNearEnd_2 != -1)   // Item (a) above
                 && (congNearStart_2 == congNearEnd_1) && (congNearStart_2 != -1) && (congNearEnd_1 != -1) ) // Item (a), continued

             || (   (congNearStart_1 == congNearEnd_2) && (congNearStart_1 != -1) && (congNearEnd_2 != -1)   // Item (b) above
                 && ((congNearStart_2 == -1) || (congNearEnd_1 == -1)))                                      // Item (b), continued

             || (   (congNearStart_2 == congNearEnd_1) && (congNearStart_2 != -1) && (congNearEnd_1 != -1)   // Item (c) above
                 && ((congNearStart_1 == -1) || (congNearEnd_2 == -1))))                                     // Item (c), continued
    {
      connection->swap = TRUE;

      #ifdef DEBUG_decideWhetherToSwapConnection
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Assigned swap to TRUE because connection was previously swapped.\n", thread_num);
      }
      #endif
    }
    else  {
      // We got here, so the congestion from previous iterations does not definitively
      // show whether the connection was previously unswapped or swapped. So we
      // simply use the symmetryRatio to decide:
      if (connection->symmetryRatio <= 0.50)  {
        connection->swap = FALSE;

        #ifdef DEBUG_decideWhetherToSwapConnection
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Assigned swap to FALSE because connection's previous state is ambiguous, but symmetryRatio is <= 0.5.\n", thread_num);
        }
        #endif
      }
      else  {
        connection->swap = TRUE;

        #ifdef DEBUG_decideWhetherToSwapConnection
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Assigned swap to TRUE because connection's previous state is ambiguous, but symmetryRatio is > 0.5.\n", thread_num);
        }
        #endif
      }
    }  // End of else-block for inconclusive congestion in map
  }  // End of else-block for unswapped & swapped configurations have similar G-costs

}  // End of function 'decideWhetherToSwapConnection'


//-----------------------------------------------------------------------------
// Name: calcSubMapDimensions
// Desc: For a given diff-pair connection, calculate the minimum and
//       maximum (x,y,z) coordinates from the main map. These coordinates
//       are placed in elements minCoord and maxCoord. The coordinates
//       will be used as bounds for the auto-router to optimize the
//       connection's wiring.
//
//       The minimum and maximum coordinates are chosen such that they define
//       a square region in the X/Y plane that is centered at the midpoint of
//       a rectangle that circumscribes the 4 terminals of the connection. If
//       the input value of 'scaleFactor' is set to 1.0, then the sides of
//       the resulting square region are equal to twice the maximum distance
//       between the coordinates in either the X- or Y-direction. The size of
//       this square can be increased or decreased by increasing/decreasing
//       the 'scaleFactor' input value.
//
//       The minimum and maximum Z-coordinates are simply the min/max
//       Z-coordinates of the two points.
//
//       This function modifies the following 6 elements of the 'connection'
//       variable:
//             minCoord.X           maxCoord.X
//             minCoord.Y           maxCoord.Y
//             minCoord.Z           maxCoord.Z
//-----------------------------------------------------------------------------
static void calcSubMapDimensions(ShoulderConnection_t * connection, MapInfo_t *mapInfo,
                                 float scaleFactor)  {

  // Temporary variables to capture minimum, maximum, and mid-point values:
  int min_X, max_X, min_Y, max_Y, min_Z, max_Z, mid_X, mid_Y;

  // Find the minimum x-value of the 4 start/end-segments for this connection:
  min_X = min(connection->startCoord_1.X, connection->startCoord_2.X);
  min_X = min(min_X, connection->endCoord_1.X);
  min_X = min(min_X, connection->endCoord_2.X);

  // Find the maximum x-value of the 4 start/end-segments for this connection:
  max_X = max(connection->startCoord_1.X, connection->startCoord_2.X);
  max_X = max(max_X, connection->endCoord_1.X);
  max_X = max(max_X, connection->endCoord_2.X);

  // Find the minimum y-value of the 4 start/end-segments for this connection:
  min_Y = min(connection->startCoord_1.Y, connection->startCoord_2.Y);
  min_Y = min(min_Y, connection->endCoord_1.Y);
  min_Y = min(min_Y, connection->endCoord_2.Y);

  // Find the maximum y-value of the 4 start/end-segments for this connection:
  max_Y = max(connection->startCoord_1.Y, connection->startCoord_2.Y);
  max_Y = max(max_Y, connection->endCoord_1.Y);
  max_Y = max(max_Y, connection->endCoord_2.Y);

  // Find the minimum z-value of the 4 start/end-segments for this connection:
  min_Z = min(connection->startCoord_1.Z, connection->startCoord_2.Z);
  min_Z = min(min_Z, connection->endCoord_1.Z);
  min_Z = min(min_Z, connection->endCoord_2.Z);

  // Find the maximum z-value of the 4 start/end-segments for this connection:
  max_Z = max(connection->startCoord_1.Z, connection->startCoord_2.Z);
  max_Z = max(max_Z, connection->endCoord_1.Z);
  max_Z = max(max_Z, connection->endCoord_2.Z);

  // Calculate the lateral (x,y) midpoint:
  mid_X = (min_X + max_X) / 2;
  mid_Y = (min_Y + max_Y) / 2;

  // Calculate the extents in the x- and y-directions:
  int span_X, span_Y;
  span_X = max_X - min_X;
  span_Y = max_Y - min_Y;

  // Calculate the maximum lateral span in the X- and Y-directions. We use this
  // maximum span to make a square- (not rectangular-) shaped sub-map. Scale the
  // size of the square by 'scaleFactor':
  int max_span = scaleFactor * max(span_X, span_Y);

  // If the 'scaleFactor' is so large that it would make the sub-map as large as the
  // original map (in the X- and Y-directions), then issue a fatal error message
  // and die:
  if (   (scaleFactor > 2.000001)
      && (mid_X - max_span < 0)
      && (mid_X + max_span >= mapInfo->mapWidth)
      && (mid_Y - max_span < 0)
      && (mid_Y + max_span >= mapInfo->mapHeight))  {
    printf("\nERROR: Function calcSubMapDimensions attempted to create a sub-map during iteration %d whose\n",
           mapInfo->current_iteration);
    printf(  "       lateral size would match that of the parent map (%d cells wide by %d cells high).\n",
           mapInfo->mapWidth, mapInfo->mapHeight);
    printf(  "       Please inform the software developer of this fatal error message.\n\n");
    exit(1);
  }


  // Calculate the minimum and maximum coordinates of the map
  // that will be used for auto-routing the connection:
  connection->minCoord.X = max(0, mid_X - max_span);
  connection->minCoord.Y = max(0, mid_Y - max_span);
  connection->minCoord.Z = min_Z;
  connection->maxCoord.X = min(mapInfo->mapWidth - 1,  mid_X + max_span);
  connection->maxCoord.Y = min(mapInfo->mapHeight - 1, mid_Y + max_span);
  connection->maxCoord.Z = max_Z;

}  // End of function 'calcSubMapDimensions'


//-----------------------------------------------------------------------------
// Name: populate_subMapInfo
// Desc: Populate a 'subMapInfo' variable of type MapInfo_t that contains the
//       sub-map's width, height, and number of routing layers for a single diff-pair
//       connection that is described by variable 'connection'. The (x,y,z) range
//       starts at (0,0,0) and extend to the size necessary to connect the diff-pair
//       connections. The start_cells[] and end_cells[] coordinates can contain
//       coordinates that are outside of the submap's boundaries, except for the two
//       diff-pairs' start- and end-coordinates; these are changed to the start- and
//       end-coordinates of the specific diff-pair connection. This function
//       modifies the elements of variable 'subMapInfo'.
//-----------------------------------------------------------------------------
static void populate_subMapInfo(MapInfo_t *subMapInfo, const ShoulderConnection_t * connection,
                                const MapInfo_t *mapInfo, RoutingMetrics_t *routability)  {

  // Define the height and width of the submap, along with the the number of layers
  // and diagonal distance.
  subMapInfo->mapWidth  = 1 + connection->maxCoord.X - connection->minCoord.X;
  subMapInfo->mapHeight = 1 + connection->maxCoord.Y - connection->minCoord.Y;
  subMapInfo->numLayers = 1 + connection->maxCoord.Z - connection->minCoord.Z;
  subMapInfo->mapDiagonal = sqrt(subMapInfo->mapWidth * subMapInfo->mapWidth
                                  +   subMapInfo->mapHeight * subMapInfo->mapHeight);

  // Copy variables from the main mapInfo variable that remain unchanged in the
  // subMapInfo variable:
  subMapInfo->numPaths = mapInfo->numPaths;
  subMapInfo->numPseudoPaths = mapInfo->numPseudoPaths;

  // Initialize the iteration count to zero; it will be incremented prior to each
  // call to findPath():
  subMapInfo->current_iteration = 0;

  // Initialize the trace and via congestion sensitivities and congestion multipliers. The congestion
  // multipliers are initialized to their nominal values (100%) and never subsequently changed, unlike
  // in the main map, in which these multiplier starts out smaller and increases as a function of the
  // iteration count.
  subMapInfo->currentTraceCongSensIndex = 0;
  subMapInfo->traceCongestionMultiplier = (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter / 100.0)
                                          * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
  subMapInfo->currentViaCongSensIndex   = 0;
  subMapInfo->viaCongestionMultiplier   = (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter / 100.0)
                                          * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;

  // Initialize the 'iterationDependentRatio' value to 1.00 for the sub-map. Unlike in the main map,
  // this multiplier will remain constant through the small number of iterations:
  subMapInfo->iterationDependentRatio = 1.00;

  subMapInfo->max_iterations = subMap_maxIterations;

  // Allocate memory from the heap for arrays in the new 'subMapInfo' variable:
  allocateMapInfo(subMapInfo, mapInfo->numPaths, mapInfo->numPseudoPaths, subMapInfo->numLayers);

  // Initialize arrays in the new 'subMapInfo' variable by (generally)
  // copying the array-elements from the main 'mapInfo' variable:
  for (int path = 0; path < mapInfo->numPaths + mapInfo->numPseudoPaths; path++)  {
    subMapInfo->start_cells[path]                   = mapInfo->start_cells[path];
    subMapInfo->end_cells[path]                     = mapInfo->end_cells[path];
    subMapInfo->diffPairStartTermPitchMicrons[path] = mapInfo->diffPairStartTermPitchMicrons[path];
    subMapInfo->diffPairEndTermPitchMicrons[path]   = mapInfo->diffPairEndTermPitchMicrons[path];
    subMapInfo->swapZone[path]                      = mapInfo->swapZone[path];
    subMapInfo->diff_pair_terms_swapped[path]       = mapInfo->diff_pair_terms_swapped[path];
    subMapInfo->start_end_terms_swapped[path]       = mapInfo->start_end_terms_swapped[path];
    for (int layer = 0; layer < subMapInfo->numLayers; layer++)  {
      subMapInfo->addPseudoTraceCongestionNearVias[path][layer] = FALSE;
    }  // End of for-loop for index 'layer' (0 to numLayers of sub-map)
  }  // End of for-loop for index 'path' (0 to numPaths+numPseudoPaths)

  // For each routing layer, copy the maximum interaction radius values from the main
  // map to the sub-map:
  for (int layer = 0; layer < subMapInfo->numLayers; layer++)  {
    subMapInfo->maxInteractionRadiusCellsOnLayer[layer]   = mapInfo->maxInteractionRadiusCellsOnLayer[layer + connection->minCoord.Z];
    subMapInfo->maxInteractionRadiusSquaredOnLayer[layer] = mapInfo->maxInteractionRadiusSquaredOnLayer[layer + connection->minCoord.Z];
  }

}  // End of function 'populate_subMapInfo'


//-----------------------------------------------------------------------------
// Name: copyCellInfo
// Desc: Copy information from the larger cellInfo 3D matrix to the smaller
//       subMap_cellInfo 3D matrix using the spatial offsets x_offset, y_offset,
//       and z_offset. This function allocates additional memory within the
//       subMap_cellInfo[][][] elements, as necessary, to accommodate the
//       data in the cellInfo[][][] matrix. Congestion and path-center info
//       from path numbers excludePath_1 and excludePath_2 is NOT copied over
//       to the subMap_cellInfo matrix. But wherever these two paths were
//       routed, the map is populated with 'forbidden' cells that are
//       off-limits for routing.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_copyCellInfo' and re-compile if you want verbose debugging
// print-statements enabled:
//
// #define DEBUG_copyCellInfo 1
#undef DEBUG_copyCellInfo

static void copyCellInfo(CellInfo_t ***cellInfo, CellInfo_t ***subMap_cellInfo,
                         MapInfo_t *subMapInfo, int x_offset, int y_offset, int z_offset,
                         int excludePath_1, int excludePath_2)  {

  #ifdef DEBUG_copyCellInfo
  printf("\nDEBUG: (thread %2d) Entered copyCellInfo during sub-iteration %d with sub-map dimensions %d x %d x %d layers, \n",
         omp_get_thread_num(), subMapInfo->current_iteration, subMapInfo->mapWidth, subMapInfo->mapHeight, subMapInfo->numLayers);
  printf(  "DEBUG: (thread %2d) offsets of (%d,%d,%d), and exclude-nets of %d, and %d\n", omp_get_thread_num(),
         -x_offset, -y_offset, -z_offset, excludePath_1, excludePath_2);
  int DEBUG_ON = FALSE;
  // For very large maps, define here the 3-dimensional window in (x,y,z) for which you want
  // detailed information sent to the log file. Without these constraints, the log file can
  // grow very large. The coordinates refer to the main map (not the sub-map):
  int x_window_min = 143;
  int x_window_max = 143;
  int y_window_min =  16;
  int y_window_max =  16;
  int z_window_min =   3;
  int z_window_max =   3;
  printf("DEBUG: (thread %2d) Detailed info will be displayed for coordinates from (%d,%d,%d) to (%d,%d,%d), inclusive,\n",
         omp_get_thread_num(), x_window_min, y_window_min, z_window_min, x_window_max, y_window_max, z_window_max);
  printf("DEBUG: (thread %2d) in the main map, which is the following range in the sub-map: (%d,%d,%d) to (%d,%d,%d).\n\n",
         omp_get_thread_num(), x_window_min - x_offset, y_window_min - y_offset, z_window_min - z_offset,
         x_window_max - x_offset, y_window_max - y_offset, z_window_max - z_offset);
  #endif

  // Iterate over the (i,j,k) coordinates of the smaller sub-map to copy data
  // from the (x,y,z) coordinates of the larger, main map:

  for (int i = 0; i < subMapInfo->mapWidth; i++)  {

    // Calculate the x-coordinate from the main 'CellInfo' map:
    int x = i + x_offset;

    for (int j = 0; j < subMapInfo->mapHeight; j++)  {

      // Calculate the y-coordinate from the main 'CellInfo' map:
      int y = j + y_offset;

      for (int k = 0; k < subMapInfo->numLayers; k++)  {

        // Calculate the z-coordinate from the main 'cellInfo' map:
        int z = k + z_offset;

        #ifdef DEBUG_copyCellInfo
        if (   (x >= x_window_min) && (x <= x_window_max)
            && (y >= y_window_min) && (y <= y_window_max)
            && (z >= z_window_min) && (z <= z_window_max))  {
          DEBUG_ON = TRUE;
        }
        else  {
          DEBUG_ON = FALSE;
        }
        #endif


        // Copy elements from coordinate (x,y,z) of the main 'cellInfo' map  to
        // coordinate (i,j,k) of the smaller sub-map:
        subMap_cellInfo[i][j][k].forbiddenTraceBarrier           = cellInfo[x][y][z].forbiddenTraceBarrier;
        subMap_cellInfo[i][j][k].forbiddenUpViaBarrier           = cellInfo[x][y][z].forbiddenUpViaBarrier;
        subMap_cellInfo[i][j][k].forbiddenDownViaBarrier         = cellInfo[x][y][z].forbiddenDownViaBarrier;
        subMap_cellInfo[i][j][k].forbiddenProximityBarrier       = cellInfo[x][y][z].forbiddenProximityBarrier;
        subMap_cellInfo[i][j][k].forbiddenProximityPinSwap       = cellInfo[x][y][z].forbiddenProximityPinSwap;
        subMap_cellInfo[i][j][k].designRuleSet                   = cellInfo[x][y][z].designRuleSet;
        subMap_cellInfo[i][j][k].traceCostMultiplierIndex        = cellInfo[x][y][z].traceCostMultiplierIndex;
        subMap_cellInfo[i][j][k].viaUpCostMultiplierIndex        = cellInfo[x][y][z].viaUpCostMultiplierIndex;
        subMap_cellInfo[i][j][k].viaDownCostMultiplierIndex      = cellInfo[x][y][z].viaDownCostMultiplierIndex;
        subMap_cellInfo[i][j][k].routing_layer_metal_fill        = cellInfo[x][y][z].routing_layer_metal_fill;
        subMap_cellInfo[i][j][k].pseudo_routing_layer_metal_fill = cellInfo[x][y][z].pseudo_routing_layer_metal_fill;
        subMap_cellInfo[i][j][k].DRC_flag                        = cellInfo[x][y][z].DRC_flag;
        subMap_cellInfo[i][j][k].via_above_metal_fill            = cellInfo[x][y][z].via_above_metal_fill;
        subMap_cellInfo[i][j][k].via_below_metal_fill            = cellInfo[x][y][z].via_below_metal_fill;
        subMap_cellInfo[i][j][k].pseudo_via_above_metal_fill     = cellInfo[x][y][z].pseudo_via_above_metal_fill;
        subMap_cellInfo[i][j][k].pseudo_via_below_metal_fill     = cellInfo[x][y][z].pseudo_via_below_metal_fill;
        subMap_cellInfo[i][j][k].via_above_DRC_flag              = cellInfo[x][y][z].via_above_DRC_flag;
        subMap_cellInfo[i][j][k].center_line_flag                = cellInfo[x][y][z].center_line_flag;
        subMap_cellInfo[i][j][k].center_viaUp_flag               = cellInfo[x][y][z].center_viaUp_flag;
        subMap_cellInfo[i][j][k].center_viaDown_flag             = cellInfo[x][y][z].center_viaDown_flag;
        subMap_cellInfo[i][j][k].near_a_net                      = cellInfo[x][y][z].near_a_net;
        subMap_cellInfo[i][j][k].swap_zone                       = cellInfo[x][y][z].swap_zone;
        subMap_cellInfo[i][j][k].explored                        = cellInfo[x][y][z].explored;
        subMap_cellInfo[i][j][k].explored_PP                     = cellInfo[x][y][z].explored_PP;
        subMap_cellInfo[i][j][k].flag                            = cellInfo[x][y][z].flag;


        //
        // Iterate over the paths that traverse the larger 'cellInfo' map at (x,y,z). If the path number
        // is not equal to excludePath_1 or excludePath_2, then copy the path's congestion to the sub-map
        // matrix at (i,j,k):
        //
        subMap_cellInfo[i][j][k].numTraversingPaths = 0;
        for (int pathIndex = 0; pathIndex < cellInfo[x][y][z].numTraversingPaths; pathIndex++)  {

          // Check whether the path-number of the traversing path is one of the two
          // diff-pair paths that should *not* be copied:
          if (   (cellInfo[x][y][z].congestion[pathIndex].pathNum != excludePath_1)
              && (cellInfo[x][y][z].congestion[pathIndex].pathNum != excludePath_2))  {

            // Allocate memory for the additional element in the 'congestion' array :
            if (subMap_cellInfo[i][j][k].numTraversingPaths == 0)  {

              // Allocate memory for the first element in the 'congestion' array in the sub-map:
              subMap_cellInfo[i][j][k].congestion = malloc(sizeof(Congestion_t));
              if (subMap_cellInfo[i][j][k].congestion == 0)  {
                printf("\n\nERROR: Failed to allocate memory for congestion in 'subMap_cellInfo' matrix at location (%d,%d,%d) in function 'copyCellInfo.\n",
                       i, j, k);
                printf(    "       Please inform the software developer of this fatal error message.\n\n");
                exit(1);
              }  // End of if-block
            }
            else  {
              // Re-allocate memory to grow the 'congestion' array in the sub-map:
              subMap_cellInfo[i][j][k].congestion = realloc(subMap_cellInfo[i][j][k].congestion,
                                                   (subMap_cellInfo[i][j][k].numTraversingPaths + 1) * sizeof(Congestion_t));
              if (subMap_cellInfo[i][j][k].congestion == 0)  {
                printf("\n\nERROR: Failed to re-allocate memory for congestion in 'subMap_cellInfo' matrix at location (%d,%d,%d) in function 'copyCellInfo.\n",
                       i, j, k);
                printf(    "       Please inform the software developer of this fatal error message.\n\n");
                exit(1);
              }  // End of if-block
            }  // End of if/else block for allocating memory for additional 'congestion' array element

            //
            // Copy 'congestion' elements from congestion-index 'pathIndex' of the main cellInfo 3D
            // matrix to congestion-index 'subMap_cellInfo[i][j][k].numTraversingPaths' of the sub-map:
            //
            subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].pathNum
                   = cellInfo[x][y][z].congestion[pathIndex].pathNum;

            subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].DR_subset
                   = cellInfo[x][y][z].congestion[pathIndex].DR_subset;

            subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].shapeType
                   = cellInfo[x][y][z].congestion[pathIndex].shapeType;

            subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].pathTraversalsTimes100
                   = cellInfo[x][y][z].congestion[pathIndex].pathTraversalsTimes100;

            #ifdef DEBUG_copyCellInfo
            if (DEBUG_ON)  {
              printf("\nDEBUG: (thread %2d) Copied the following congestion information from main map to sub-map:\n", omp_get_thread_num());
              printf(  "DEBUG: (thread %2d)          Source: (%d,%d,%d), index %d of main map\n", omp_get_thread_num(), x, y, z, pathIndex);
              printf(  "DEBUG: (thread %2d)     Destination: (%d,%d,%d), index %d of sub-map\n", omp_get_thread_num(), i, j, k,
                     subMap_cellInfo[i][j][k].numTraversingPaths);
              printf(  "DEBUG: (thread %2d)                     congestion[%d].pathNum: %d\n", omp_get_thread_num(), subMap_cellInfo[i][j][k].numTraversingPaths,
                     subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].pathNum);
              printf(  "DEBUG: (thread %2d)      congestion[%d].pathTraversalsTimes100: %d\n", omp_get_thread_num(), subMap_cellInfo[i][j][k].numTraversingPaths,
                     subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].pathTraversalsTimes100);
              printf(  "DEBUG: (thread %2d)                   congestion[%d].DR_subset: %d\n", omp_get_thread_num(), subMap_cellInfo[i][j][k].numTraversingPaths,
                     subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].DR_subset);
              printf(  "DEBUG: (thread %2d)                   congestion[%d].shapeType: %d\n", omp_get_thread_num(), subMap_cellInfo[i][j][k].numTraversingPaths,
                     subMap_cellInfo[i][j][k].congestion[subMap_cellInfo[i][j][k].numTraversingPaths].shapeType);
            }
            #endif

            // Increment the number of paths that traverse coordinate (i,j,k) in the sub-map:
            subMap_cellInfo[i][j][k].numTraversingPaths++;

          }  // End of if-block for path number *not* being one of the two excluded paths
          #ifdef DEBUG_copyCellInfo
          else  {
            if (DEBUG_ON)  {
              printf("\nDEBUG: (thread %2d) Did _NOT_ copy the following congestion information from main map to sub-map:\n", omp_get_thread_num());
              printf(  "DEBUG: (thread %2d)          Source: (%d,%d,%d), index %d of main map\n", omp_get_thread_num(), x, y, z, pathIndex);
              printf(  "DEBUG: (thread %2d)     Destination: (%d,%d,%d) of sub-map\n", omp_get_thread_num(), i, j, k);
              printf(  "DEBUG: (thread %2d)                     congestion[%d].pathNum: %d\n", omp_get_thread_num(), pathIndex,
                     cellInfo[x][y][z].congestion[pathIndex].pathNum);
              printf(  "DEBUG: (thread %2d)      congestion[%d].pathTraversalsTimes100: %d\n", omp_get_thread_num(), pathIndex,
                     cellInfo[x][y][z].congestion[pathIndex].pathTraversalsTimes100);
              printf(  "DEBUG: (thread %2d)                   congestion[%d].DR_subset: %d\n", omp_get_thread_num(), pathIndex,
                     cellInfo[x][y][z].congestion[pathIndex].DR_subset);
              printf(  "DEBUG: (thread %2d)                   congestion[%d].shapeType: %d\n", omp_get_thread_num(), pathIndex,
                     cellInfo[x][y][z].congestion[pathIndex].shapeType);
            }
          }  // End of else-block for path number being one of the two excluded paths
          #endif

        }  // End of for-loop for index 'pathIndex' (0 to numTraversingPaths)

        //
        // Iterate over the path-centers that traverse the larger 'cellInfo' map at (x,y,z). If the path number
        // is not equal to excludePath_1 or excludePath_2, then copy it to the sub-map matrix at (i,j,k):
        //
        subMap_cellInfo[i][j][k].numTraversingPathCenters = 0;
        for (int pathCenterIndex = 0; pathCenterIndex < cellInfo[x][y][z].numTraversingPathCenters; pathCenterIndex++)  {

          // Check whether the path-number of the traversing path-center is one of the two
          // diff-pair paths that should *not* be copied:
          if (   (cellInfo[x][y][z].pathCenters[pathCenterIndex].pathNum != excludePath_1)
              && (cellInfo[x][y][z].pathCenters[pathCenterIndex].pathNum != excludePath_2))  {

            add_path_center_info(&(subMap_cellInfo[i][j][k]), cellInfo[x][y][z].pathCenters[pathCenterIndex].pathNum,
                                 cellInfo[x][y][z].pathCenters[pathCenterIndex].shapeType);

            #ifdef DEBUG_copyCellInfo
            if (DEBUG_ON)  {
              printf("\nDEBUG: (thread %2d) Copied the following path-center information from main map to sub-map:\n", omp_get_thread_num());
              printf(  "DEBUG: (thread %2d)          Source: (%d,%d,%d), index %d of main map\n", omp_get_thread_num(), x, y, z, pathCenterIndex);
              printf(  "DEBUG: (thread %2d)     Destination: (%d,%d,%d), index %d of sub-map\n", omp_get_thread_num(), i, j, k,
                     subMap_cellInfo[i][j][k].numTraversingPathCenters - 1);
              printf(  "DEBUG: (thread %2d)             pathCenters[%d].pathNum: %d\n", omp_get_thread_num(), subMap_cellInfo[i][j][k].numTraversingPathCenters,
                     subMap_cellInfo[i][j][k].pathCenters[subMap_cellInfo[i][j][k].numTraversingPathCenters - 1].pathNum);
              printf(  "DEBUG: (thread %2d)           pathCenters[%d].shapeType: %d\n", omp_get_thread_num(), subMap_cellInfo[i][j][k].numTraversingPathCenters,
                     subMap_cellInfo[i][j][k].pathCenters[subMap_cellInfo[i][j][k].numTraversingPathCenters - 1].shapeType);
            }
            #endif

          }  // End of if-block for path-center number *not* being one of the two excluded paths
          #ifdef DEBUG_copyCellInfo
          else  {

            if (DEBUG_ON)  {
              printf("\nDEBUG: (thread %2d) Did _NOT_ copy the following path-center information from main map to sub-map:\n", omp_get_thread_num());
              printf(  "DEBUG: (thread %2d)          Source: (%d,%d,%d), index %d of main map\n", omp_get_thread_num(), x, y, z, pathCenterIndex);
              printf(  "DEBUG: (thread %2d)     Destination: (%d,%d,%d) of sub-map\n", omp_get_thread_num(), i, j, k);
              printf(  "DEBUG: (thread %2d)                     pathCenters[%d].pathNum: %d\n", omp_get_thread_num(), pathCenterIndex,
                     cellInfo[x][y][z].pathCenters[pathCenterIndex].pathNum);
              printf(  "DEBUG: (thread %2d)                   pathCenters[%d].shapeType: %d\n", omp_get_thread_num(), pathCenterIndex,
                     cellInfo[x][y][z].pathCenters[pathCenterIndex].shapeType);
            }
          }  // End of else-block for path-center number being one of the two excluded paths
          #endif

        }  // End of for-loop for index 'pathCenterIndex' (0 to numTraversingPathCenters)

        #ifdef DEBUG_copyCellInfo
        if (DEBUG_ON)  {
          printf("\nDEBUG: (thread %2d) For submap cell at (%d,%d,%d), numTraversingPaths = %d, numTraversingPathCenters = %d\n\n",
                 omp_get_thread_num(), i, j, k, subMap_cellInfo[i][j][k].numTraversingPaths, subMap_cellInfo[i][j][k].numTraversingPathCenters);
        }
        #endif

      }  // End of for-loop for index 'k' (0 to number of layers in sub-map)
    }  // End of for-loop for index 'j' (0 to sub-map's height)
  }  // End of for-loop for index 'i' (0 to sub-map's width)

}  // End of function 'copyCellInfo'


//-----------------------------------------------------------------------------
// Name: evaporateDiffPairCongestion
// Desc: At each cell in the cellInfo matrix, reduce the congestion value of
//       paths pathNum_1 and pathNum_2 by the percentage specified by
//       'evaporationRate' at that cell. Valid values of 'evaporationRate'
//       could range from 0 to 100. The resulting congestion is always rounded
//       down, so it could reach zero. If the result is indeed zero congestion,
//       then eliminate the traversing path from the cell.
//
//       This function is similar to evaporateCongestion() with the following
//       differences:
//         o  This function does not reduce congestion of all paths, but only
//            reduces the congestion of pathNum_1 and pathNum_2 (diff-pair paths).
//         o  This function does not use parallel processing, which
//            significantly simplifies the function.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_evaporateDiffPairCongestion' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_evaporateDiffPairCongestion 1
#undef DEBUG_evaporateDiffPairCongestion

static void evaporateDiffPairCongestion(MapInfo_t *mapInfo, CellInfo_t ***cellInfo,
                                        const float evaporationRate, int pathNum_1, int pathNum_2) {

  const float retainFactor = 1 - (evaporationRate/100.0); // Factor by which to multiply congestion
                                                          // in order to reduce congestion by
                                                          // evaporationRate percent.

  int num_cells_resized = 0;

  #ifdef DEBUG_evaporateDiffPairCongestion
  printf("\nDEBUG: (thread %2d) Entered evaporateDiffPairCongestion for paths %d and %d with retainFactor = %.4f\n",
         omp_get_thread_num(), pathNum_1, pathNum_2, retainFactor);
  #endif

  // Iterate over all X/Y/Z locations and reduce the congestion values at each
  // cell. For congestion values that become zero, re-allocate (reduce) the
  // memory for such arrays.
  for (int z = 0; z < mapInfo->numLayers; z++)  {
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      for (int x = 0; x < mapInfo->mapWidth; x++ )  {

        // If cell is not walkable, then skip it:
        if (cellInfo[x][y][z].forbiddenTraceBarrier)  {
          continue;
        }

        // Save the original number of traversing paths before any evaporation takes place.
        // We'll use this saved value later to decide whether to re-allocate (shrink) memory
        // for the 'congestion[]' array:
        int origNumTraversingPaths = cellInfo[x][y][z].numTraversingPaths;

        #ifdef DEBUG_evaporateDiffPairCongestion
        printf("DEBUG: Cell (%d,%d,%d) has %d congestion elements.\n", x, y, z, origNumTraversingPaths);
        #endif

        unsigned int original_congestion, new_congestion;

        // For each traversing path, get the associated congestion and reduce it by 'retainFactor':
        int pathIndex = 0;
        while (pathIndex < cellInfo[x][y][z].numTraversingPaths)  {

          // Check if the congestion is from pathNum_1 or pathNum_2:
          if (   (cellInfo[x][y][z].congestion[pathIndex].pathNum == pathNum_1)
              || (cellInfo[x][y][z].congestion[pathIndex].pathNum == pathNum_2))  {

            // Get current congestion, i.e., number of path-traversals multiplied by 100:
            original_congestion = cellInfo[x][y][z].congestion[pathIndex].pathTraversalsTimes100;

            // Calculate new congestion for this cell by multiplying the current
            // congestion by 'retainFactor', and truncating to the next lowest integer:
            new_congestion = (unsigned) (original_congestion * retainFactor);

            // printf("DEBUG:    (thread %2d) Index %d: Congestion reduced from %d to %d at (%d,%d,%d)\n",
            //        thread_num, pathIndex, original_congestion, new_congestion, x, y, z);

            // Depending on whether the congestion has evaporated to zero, either reduce
            // the congestion to a non-zero value, or else eliminate the zero-congestion
            // element from the array:
            if (new_congestion != 0)  {
              // Assign the new congestion to the appropriate location in the cellInfo matrix:
              assignCongestionByPathIndex(&(cellInfo[x][y][z]), pathIndex, new_congestion);
            }
            else  {
              // We got here, so the congestion evaporated to zero. For the subsequent paths
              // that traverse this cell, move the path number and its congestion 'down' by 1.
              // That is, reduce the index values for these subsequent paths:
              for (int oldPathIndex = pathIndex + 1; oldPathIndex < cellInfo[x][y][z].numTraversingPaths; oldPathIndex++)  {

                // Assign path number, DR_subset, shape type, and congestion to (oldPathIndex - 1):
                cellInfo[x][y][z].congestion[oldPathIndex - 1].pathNum                = cellInfo[x][y][z].congestion[oldPathIndex].pathNum;
                cellInfo[x][y][z].congestion[oldPathIndex - 1].DR_subset              = cellInfo[x][y][z].congestion[oldPathIndex].DR_subset;
                cellInfo[x][y][z].congestion[oldPathIndex - 1].shapeType              = cellInfo[x][y][z].congestion[oldPathIndex].shapeType;
                cellInfo[x][y][z].congestion[oldPathIndex - 1].pathTraversalsTimes100 = cellInfo[x][y][z].congestion[oldPathIndex].pathTraversalsTimes100;

              }  // End of for-loop for index 'oldPathIndex'

              // Decrease 'pathIndex' by 1 because we just decremented all the indices
              // of the traversing paths. So we need to process the same pathIndex value
              // now that it contains different data.
              pathIndex--;

              // Reduce the new number of congestion indices by 1. Note that this might reduce it to zero.
              cellInfo[x][y][z].numTraversingPaths--;

            }  // End of else-block for new_congestion == 0

          }  // End of if-block for the congestion belonging to either pathNum_1 or pathNum_2

          // Increment 'pathIndex' so we can move on to the next traversing path at the cell.
          pathIndex++;

        }  // End of while-block for pathIndex < cellInfo[x][y][z].numTraversingPaths

        // If any paths' congestion evaporated to zero, then re-size (or free) the array
        // for 'cellInfo[x][y][z].congestion' array so the array requires fewer elements:
        if (cellInfo[x][y][z].numTraversingPaths != origNumTraversingPaths)  {
          num_cells_resized++;
          if (cellInfo[x][y][z].numTraversingPaths)  {
            cellInfo[x][y][z].congestion = realloc(cellInfo[x][y][z].congestion,
                                                   cellInfo[x][y][z].numTraversingPaths * sizeof(Congestion_t));
          }  // End of if-block for having non-zero number of elements in congestion[] array
          else  {
            // There is no longer any congestion at (x,y,z), so free the
            // memory in the 1-dimensional 'congestion' matrix for this
            // (x,y,z) location:
            free(cellInfo[x][y][z].congestion);
            cellInfo[x][y][z].congestion = NULL; // Set pointer to NULL as a precaution
          }  // End of else-block for having zero elements in congestion[] array
        }  // End of if-block for the (new) number of traversing paths being different (less than) the original

      }  // End of loop for index 'x'
    }  // End of loop for index 'y'
  }  // End of loop for index 'z'

  #ifdef DEBUG_evaporateDiffPairCongestion
  printf("DEBUG: (thread %2d) %d cells were resized in evaporateDiffPairCongestion for paths %d and %d with retainFactor = %5.3f\n\n",
         omp_get_thread_num(), num_cells_resized, pathNum_1, pathNum_2, retainFactor);
  #endif

}  // End of function 'evaporateDiffPairCongestion'


//-----------------------------------------------------------------------------
// Name: xyzIsOutsideOfSubMap
// Desc: Check if the point (x,y,z) is within the sub-map described by the
//       the diff-pair connection 'connection'. If not, return TRUE.
//       Return FALSE otherwise. Note that all coordinates are relative to
//       the main map, and not to the smaller sub-map.
//-----------------------------------------------------------------------------
static inline int xyzIsOutsideOfSubMap(int x, int y, int z, ShoulderConnection_t connection)  {

  // Check if point's coordinates are outside of the sub-map's range:
  if (  (x < connection.minCoord.X) || (x > connection.maxCoord.X)
     || (y < connection.minCoord.Y) || (y > connection.maxCoord.Y)
     || (z < connection.minCoord.Z) || (z > connection.maxCoord.Z))  {

    return(TRUE);  // Point is outside of sub-map, so return TRUE
  }
  else {
    return(FALSE);  // Point is not outside of sub-map, so return FALSE
  }

}  // End of function 'xyzIsOutsideOfSubMap'


//-----------------------------------------------------------------------------
// Name: isFarFromConnectionTerminals
// Desc: Return TRUE if the coordinate (x,y,z) from the main map is at least
//       'distance' number of cells from each of the four connection-terminals
//       in diff-pair connection 'connection', whose coordinates are also in the
//       main map. Return FALSE if (x,y,z) is within 'distance' cells of any
//       one of the four terminals and on the same layer.
//-----------------------------------------------------------------------------
static int isFarFromConnectionTerminals(int x, int y, int z, ShoulderConnection_t connection, float distance)  {

  // Check the start-terminal for path #1 of the diff-pair connection:
  if (   (z == connection.startCoord_1.Z)
      && (calc_2D_Pythagorean_distance_ints(x, y, connection.startCoord_1.X, connection.startCoord_1.Y) <= distance))  {
    return(FALSE);
  }

  // Check the start-terminal for path #2 of the diff-pair connection:
  if (   (z == connection.startCoord_2.Z)
      && (calc_2D_Pythagorean_distance_ints(x, y, connection.startCoord_2.X, connection.startCoord_2.Y) <= distance))  {
    return(FALSE);
  }

  // Check the end-terminal for path #1 of the diff-pair connection:
  if (   (z == connection.endCoord_1.Z)
      && (calc_2D_Pythagorean_distance_ints(x, y, connection.endCoord_1.X, connection.endCoord_1.Y) <= distance))  {
    return(FALSE);
  }

  // Check the end-terminal for path #2 of the diff-pair connection:
  if (   (z == connection.endCoord_2.Z)
      && (calc_2D_Pythagorean_distance_ints(x, y, connection.endCoord_2.X, connection.endCoord_2.Y) <= distance))  {
    return(FALSE);
  }

  // We got here, so none of the above 4 compound if-statements were satisfied. This means that
  // point (x,y,z) is *not* close to any one of the four terminals in the diff-pair connection.
  // So we return 'TRUE':
  return(TRUE);

}  // End of function 'isFarFromConnectionTerminals'


//-----------------------------------------------------------------------------
// Name: makeContiguousForbiddenCells
// Desc: In sub-map 'subMap_cellInfo', iterate over diff-pair path number
//       'path_num' from segment 'start_seg' to segment 'end_segment'. At
//       each segment, make the path-center un-routable. Also make intermediate
//       cells un-routable, so that there's a contiguous un-routable path.
//       Cells located within 2 cell-units from any of the diff-pair
//       connection's four start/end-terminals will not be made un-
//       routable.  If the path exits the sub-map for two consecutive segments,
//       then stop iterating over the segment. If 'start_seg' is greater than
//       'end_segment', iterate over the path in reverse.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_makeContiguousForbiddenCells' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_makeContiguousForbiddenCells 1
#undef DEBUG_makeContiguousForbiddenCells

static void makeContiguousForbiddenCells(int path_num, int start_seg, int end_seg,
                                         CellInfo_t ***subMap_cellInfo,
                                         ShoulderConnection_t connection,
                                         Coordinate_t *pathCoords[], MapInfo_t *mapInfo)  {

  #ifdef DEBUG_makeContiguousForbiddenCells
  // DEBUG code follows:
  //
  // Variable used for debugging:
  int DEBUG_ON = FALSE;

  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  if (   (mapInfo->current_iteration >= 55) && (mapInfo->current_iteration <= 55)
      && ((path_num == 10) || (path_num == 11)))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in makeContiguousForbiddenCells() because specific requirements were met in iteration %d.\n\n",
           mapInfo->current_iteration);
    DEBUG_ON = TRUE;

    printf("DEBUG: makeContiguousForbiddenCells was entered with:\n");
    printf("DEBUG:       path_num = %d\n", path_num);
    printf("DEBUG:      start_seg = %d\n", start_seg);
    printf("DEBUG:        end_seg = %d\n", end_seg);
    printf("DEBUG:  Connection attributes:\n");
    printf("DEBUG:     Start-coordinate #1 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.startSegment_1,
           connection.startCoord_1.X, connection.startCoord_1.Y, connection.startCoord_1.Z,
           connection.startCoord_1.X - connection.minCoord.X, connection.startCoord_1.Y - connection.minCoord.Y,
           connection.startCoord_1.Z - connection.minCoord.Z);
    printf("DEBUG:     Start-coordinate #2 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.startSegment_2,
           connection.startCoord_2.X, connection.startCoord_2.Y, connection.startCoord_2.Z,
           connection.startCoord_2.X - connection.minCoord.X, connection.startCoord_2.Y - connection.minCoord.Y,
           connection.startCoord_2.Z - connection.minCoord.Z);
    printf("DEBUG:       End-coordinate #1 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.endSegment_1,
               connection.endCoord_1.X, connection.endCoord_1.Y, connection.endCoord_1.Z,
               connection.endCoord_1.X - connection.minCoord.X, connection.endCoord_1.Y - connection.minCoord.Y,
               connection.endCoord_1.Z - connection.minCoord.Z);
    printf("DEBUG:       End-coordinate #2 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.endSegment_2,
           connection.endCoord_2.X, connection.endCoord_2.Y, connection.endCoord_2.Z,
           connection.endCoord_2.X - connection.minCoord.X, connection.endCoord_2.Y - connection.minCoord.Y,
           connection.endCoord_2.Z - connection.minCoord.Z);
    printf("DEBUG:       Min coordinate: (%d,%d,%d)\n", connection.minCoord.X, connection.minCoord.Y, connection.minCoord.Z);
    printf("DEBUG:       Max coordinate: (%d,%d,%d)\n\n", connection.maxCoord.X, connection.maxCoord.Y, connection.maxCoord.Z);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Based on the values of 'start_seg' and 'end_seg', determine what direction
  // this function should iterate over the path:
  int step_direction;
  if (start_seg > end_seg)  {
    step_direction = -1;
  }
  else  if (start_seg < end_seg)  {
    step_direction = 1;
  }
  else  {
    // We got here, so start_seg is equal to end_seg. This function should do nothing.
    #ifdef DEBUG_makeContiguousForbiddenCells
    if (DEBUG_ON)  {
      printf("DEBUG: Returning from makeContiguousForbiddenCells because start_seg is equal to end_seg\n\n");
    }
    #endif

    return;
  }
  #ifdef DEBUG_makeContiguousForbiddenCells
  if (DEBUG_ON)  {
    printf("DEBUG: step_direction = %d\n\n", step_direction);
  }
  #endif

  // Handle the case where the start- or end-segment is -1, which is the start-terminal of
  // the entire path. Replace this segment number with '0', which is close enough for
  // the goal of this function, and simplifies the coding:
  if (start_seg == -1)  {
    start_seg = 0;
  }

  Coordinate_t prev_segment;
  Coordinate_t current_segment;
  prev_segment.X = pathCoords[path_num][start_seg].X;
  prev_segment.Y = pathCoords[path_num][start_seg].Y;
  prev_segment.Z = pathCoords[path_num][start_seg].Z;

  for (int segment = start_seg + step_direction; segment != end_seg; segment += step_direction)  {

    // Get the coordinates of this segment in the main map:
    current_segment.X = pathCoords[path_num][segment].X;
    current_segment.Y = pathCoords[path_num][segment].Y;
    current_segment.Z = pathCoords[path_num][segment].Z;

    #ifdef DEBUG_makeContiguousForbiddenCells
    if (DEBUG_ON)  {
      printf("DEBUG: Iterating over segment %d of path %d at (%d,%d,%d) in main map. Previous segment was at (%d,%d,%d)\n",
             segment, path_num, current_segment.X, current_segment.Y, current_segment.Z,
             prev_segment.X, prev_segment.Y, prev_segment.Z);
    }
    #endif

    // If the previous and current segments are both outside of the sub-map,
    // then it means that we've traced this path beyond the boundaries of the
    // sub-map. In this case, break out of this for-loop:
    if (   xyzIsOutsideOfSubMap(current_segment.X, current_segment.Y, current_segment.Z, connection)
        && xyzIsOutsideOfSubMap(prev_segment.X,    prev_segment.Y,    prev_segment.Z,    connection))  {

      #ifdef DEBUG_makeContiguousForbiddenCells
      if (DEBUG_ON)  {
        printf("DEBUG: Stopped iteration of path %d because we exited the sub-map.\n\n", path_num);
      }
      #endif

      break;  // Break out of the for-loop of index 'segment'
    }

    // Calculate the lateral distance between the previous and current segments:
    float lateral_dist = calc_2D_Pythagorean_distance_ints(prev_segment.X, prev_segment.Y, current_segment.X, current_segment.Y);

    // Calculate the delta-X and delta-Y of the current segment relative to
    // the previous segment:
    int delta_X = current_segment.X - prev_segment.X;
    int delta_Y = current_segment.Y - prev_segment.Y;
    int delta_Z = current_segment.Z - prev_segment.Z;

    // Iterate in units of 1.0 cells over a straight line that goes from the
    // previous segment to the current segment:
    for (float t = 0.0; t < lateral_dist; t = t + 1.0)  {
      float param = t / lateral_dist;  // 'param' value ranges from 0.0 to 1.0

      // At the current point along the parameterized line, calculate three
      // coordinates:
      // (X_a, Y_a) is a point on the parameterized line,
      // (X_b, Y_b) is a point that's one cell east of the parameterized line, and
      // (X_c, Y_c) is a point that's one cell north of the parameterized line.
      //
      // Note that each of these coordinates is in the coordinat-space of the original,
      // main map, and not the smaller sub-map!
      //
      int X_a = prev_segment.X + (int)(delta_X * param);
      int Y_a = prev_segment.Y + (int)(delta_Y * param);
      int Z_a = prev_segment.Z + (int)(delta_Z * param);

      int X_b = X_a + 1;  // Just east of point (Xa, Ya)
      int Y_b = Y_a;
      int Z_b = Z_a;

      int X_c = X_a;
      int Y_c = Y_a + 1;  // Just north of point (Xa, Ya)
      int Z_c = Z_a;

      // Make each of the above three cells un-routable if the cell meets the
      // following criteria:
      //   (a) Cell is at least 2.1 cells away from any of the four
      //       terminals associated with the current diff-pair connection, and
      //   (b) Cell is within the boundaries of the sub-map, and
      //   (c) Cell is not in a pin-swap zone.
      // Start with point (X_a, Y_a, Z_a):
      if (   isFarFromConnectionTerminals(X_a, Y_a, Z_a, connection, 2.1)  // Criterion (a) above
          && (! xyzIsOutsideOfSubMap(X_a, Y_a, Z_a, connection))           // Criterion (b) above
          && (0 == subMap_cellInfo[X_a - connection.minCoord.X][Y_a - connection.minCoord.Y][Z_a - connection.minCoord.Z].swap_zone))  // Criterion (c) above
      {

        subMap_cellInfo[X_a - connection.minCoord.X][Y_a - connection.minCoord.Y][Z_a - connection.minCoord.Z].forbiddenTraceBarrier = TRUE;
        #ifdef DEBUG_makeContiguousForbiddenCells
        if (DEBUG_ON)  {
          printf("DEBUG: Making (%d,%d,%d) in sub-map un-routable for path %d, which was (%d,%d,%d) in main map.\n",
                 X_a - connection.minCoord.X, Y_a - connection.minCoord.Y, Z_a - connection.minCoord.Z, path_num, X_a, Y_a, Z_a);
        }
        #endif
      }

      // Next, evaluate point (X_b, Y_b, Z_b):
      if (   isFarFromConnectionTerminals(X_b, Y_b, Z_b, connection, 2.1)  // Criterion (a) above
          && (! xyzIsOutsideOfSubMap(X_b, Y_b, Z_b, connection))           // Criterion (b) above
          && (0 == subMap_cellInfo[X_b - connection.minCoord.X][Y_b - connection.minCoord.Y][Z_b - connection.minCoord.Z].swap_zone))  // Criterion (c) above
      {

        subMap_cellInfo[X_b - connection.minCoord.X][Y_b - connection.minCoord.Y][Z_b - connection.minCoord.Z].forbiddenTraceBarrier = TRUE;
        #ifdef DEBUG_makeContiguousForbiddenCells
        if (DEBUG_ON)  {
          printf("DEBUG: Making (%d,%d,%d) in sub-map un-routable for path %d, which was (%d,%d,%d) in main map.\n",
                 X_b - connection.minCoord.X, Y_b - connection.minCoord.Y, Z_b - connection.minCoord.Z, path_num, X_b, Y_b, Z_b);
        }
        #endif
      }

      // Finally, evaluate point (X_c, Y_c, Z_c):
      if (   isFarFromConnectionTerminals(X_c, Y_c, Z_c, connection, 2.1)  // Criterion (a) above
          && (! xyzIsOutsideOfSubMap(X_c, Y_c, Z_c, connection))          // Criterion (b) above
          && (0 == subMap_cellInfo[X_c - connection.minCoord.X][Y_c - connection.minCoord.Y][Z_c - connection.minCoord.Z].swap_zone))  // Criterion (c) above
      {

        subMap_cellInfo[X_c - connection.minCoord.X][Y_c - connection.minCoord.Y][Z_c - connection.minCoord.Z].forbiddenTraceBarrier = TRUE;
        #ifdef DEBUG_makeContiguousForbiddenCells
        if (DEBUG_ON)  {
          printf("DEBUG: Making (%d,%d,%d) in sub-map un-routable for path %d, which was (%d,%d,%d) in main map.\n",
                 X_c - connection.minCoord.X, Y_c - connection.minCoord.Y, Z_c - connection.minCoord.Z, path_num, X_c, Y_c, Z_c);
        }
        #endif
      }
    }  // End of for-loop for floating-point index 't'

    // In anticipation of the next iteration through this for-loop copy the current segment's
    // coordinates into the 'prev_segment' variable:
    prev_segment = copyCoordinates(current_segment);

  }  // End of for-loop to iterate over path segments

}  // End of function 'makeContiguousForbiddenCells'


//-----------------------------------------------------------------------------
// Name: addDiffPairPathCentersToSubMap
// Desc: In sub-map 'subMap_cellInfo', iterate over diff-pair path number
//       'path_num' from segment 'start_seg' to segment 'end_segment'. At
//       each segment, add the path-center information for the associated path
//       including the shape-type. If the path exits the sub-map for two
//       consecutive segments, then stop iterating over the segment. If
//       'start_seg' is greater than 'end_segment', iterate over the path in
//       reverse.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_addDiffPairPathCentersToSubMap' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_addDiffPairPathCentersToSubMap 1
#undef DEBUG_addDiffPairPathCentersToSubMap

static void addDiffPairPathCentersToSubMap(int path_num, int start_seg, int end_seg,
                                           CellInfo_t ***subMap_cellInfo,
                                           ShoulderConnection_t connection, Coordinate_t *pathCoords[],
                                           MapInfo_t *mapInfo)  {

  #ifdef DEBUG_addDiffPairPathCentersToSubMap
  // DEBUG code follows:
  //
  // Variable used for debugging:
  int DEBUG_ON = FALSE;

  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  if (   (mapInfo->current_iteration >= 84) && (mapInfo->current_iteration <= 84)
      && ((path_num == 0) || (path_num == 1)))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in addDiffPairPathCentersToSubMap() because specific requirements were met in iteration %d.\n\n",
           mapInfo->current_iteration);
    DEBUG_ON = TRUE;

    printf("DEBUG: addDiffPairPathCentersToSubMap was entered with:\n");
    printf("DEBUG:       path_num = %d\n", path_num);
    printf("DEBUG:      start_seg = %d\n", start_seg);
    printf("DEBUG:        end_seg = %d\n", end_seg);
    printf("DEBUG:  Connection attributes:\n");
    printf("DEBUG:     Start-coordinate #1 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.startSegment_1,
           connection.startCoord_1.X, connection.startCoord_1.Y, connection.startCoord_1.Z,
           connection.startCoord_1.X - connection.minCoord.X, connection.startCoord_1.Y - connection.minCoord.Y,
           connection.startCoord_1.Z - connection.minCoord.Z);
    printf("DEBUG:     Start-coordinate #2 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.startSegment_2,
           connection.startCoord_2.X, connection.startCoord_2.Y, connection.startCoord_2.Z,
           connection.startCoord_2.X - connection.minCoord.X, connection.startCoord_2.Y - connection.minCoord.Y,
           connection.startCoord_2.Z - connection.minCoord.Z);
    printf("DEBUG:       End-coordinate #1 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.endSegment_1,
               connection.endCoord_1.X, connection.endCoord_1.Y, connection.endCoord_1.Z,
               connection.endCoord_1.X - connection.minCoord.X, connection.endCoord_1.Y - connection.minCoord.Y,
               connection.endCoord_1.Z - connection.minCoord.Z);
    printf("DEBUG:       End-coordinate #2 at segment %d: (%d,%d,%d) ==> (%d,%d,%d) in sub-map\n", connection.endSegment_2,
           connection.endCoord_2.X, connection.endCoord_2.Y, connection.endCoord_2.Z,
           connection.endCoord_2.X - connection.minCoord.X, connection.endCoord_2.Y - connection.minCoord.Y,
           connection.endCoord_2.Z - connection.minCoord.Z);
    printf("DEBUG:       Min coordinate: (%d,%d,%d)\n", connection.minCoord.X, connection.minCoord.Y, connection.minCoord.Z);
    printf("DEBUG:       Max coordinate: (%d,%d,%d)\n\n", connection.maxCoord.X, connection.maxCoord.Y, connection.maxCoord.Z);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Based on the values of 'start_seg' and 'end_seg', determine what direction
  // this function should iterate over the path:
  int step_direction;
  if (start_seg > end_seg)  {
    step_direction = -1;
  }
  else  if (start_seg < end_seg)  {
    step_direction = 1;
  }
  else  {
    // We got here, so start_seg is equal to end_seg. This function should do nothing.
    #ifdef DEBUG_addDiffPairPathCentersToSubMap
    if (DEBUG_ON)  {
      printf("DEBUG: Returning from addDiffPairPathCentersToSubMap because start_seg is equal to end_seg\n\n");
    }
    #endif

    return;
  }
  #ifdef DEBUG_addDiffPairPathCentersToSubMap
  if (DEBUG_ON)  {
    printf("DEBUG: step_direction = %d\n\n", step_direction);
  }
  #endif

  // Handle the case where the start- or end-segment is -1, which is the start-terminal of
  // the entire path. Replace this segment number with '0', which is close enough for
  // the goal of this function, and simplifies the coding:
  if (start_seg == -1)  {
    start_seg = 0;
  }

  // Assign the 'previous' coordinates to the start-segment:
  int prev_x_mainMap = pathCoords[path_num][start_seg].X;
  int prev_y_mainMap = pathCoords[path_num][start_seg].Y;
  int prev_z_mainMap = pathCoords[path_num][start_seg].Z;

  int prev_x_subMap = prev_x_mainMap - connection.minCoord.X;
  int prev_y_subMap = prev_y_mainMap - connection.minCoord.Y;
  int prev_z_subMap = prev_z_mainMap - connection.minCoord.Z;

  for (int segment = start_seg + step_direction; segment != end_seg; segment += step_direction)  {

    // Get the coordinates of this segment in the main map:
    int x_mainMap = pathCoords[path_num][segment].X;
    int y_mainMap = pathCoords[path_num][segment].Y;
    int z_mainMap = pathCoords[path_num][segment].Z;

    // Calculate the coordinates of this segment in the sub-map:
    int x_subMap = x_mainMap - connection.minCoord.X;
    int y_subMap = y_mainMap - connection.minCoord.Y;
    int z_subMap = z_mainMap - connection.minCoord.Z;

    #ifdef DEBUG_addDiffPairPathCentersToSubMap
    if (DEBUG_ON)  {
      printf("DEBUG: Iterating over segment %d of path %d at (%d,%d,%d) in main map, or (%d,%d,%d) in sub-map.\n",
             segment, path_num, x_mainMap, y_mainMap, z_mainMap, x_subMap, y_subMap, z_subMap);
      printf("DEBUG:   Previous segment was at (%d,%d,%d) in main map, or (%d,%d,%d) in sub-map.\n",
             prev_x_mainMap, prev_y_mainMap, prev_z_mainMap, prev_x_subMap, prev_y_subMap, prev_z_subMap);
    }
    #endif

    // If the previous and current segments are both outside of the sub-map,
    // then it means that we've traced this path beyond the boundaries of the
    // sub-map. In this case, break out of this for-loop:
    if (   xyzIsOutsideOfSubMap(x_mainMap,      y_mainMap,      z_mainMap,      connection)
        && xyzIsOutsideOfSubMap(prev_x_mainMap, prev_y_mainMap, prev_z_mainMap, connection))  {

      #ifdef DEBUG_addDiffPairPathCentersToSubMap
      if (DEBUG_ON)  {
        printf("DEBUG: Stopped iteration of path %d because we exited the sub-map.\n\n", path_num);
      }
      #endif

      break;  // Break out of the for-loop of index 'segment'
    }

    // Add the path-center information to the sub-map, including the path-number and its shape-type
    // if the cell meets the following criteria:
    //   (a) Cell is within the boundaries of the sub-map, and
    //   (b) Cell is not in a pin-swap zone.
    //
    if (   ( ! xyzIsOutsideOfSubMap(x_mainMap, y_mainMap, z_mainMap, connection))          // Criterion (a) above
        && (0 == subMap_cellInfo[x_subMap][y_subMap][z_subMap].swap_zone))                 // Criterion (b) above
    {
      // For the current segment of the path, add the path-center information to the cell in the
      // sub-map, including the path-number and its (TRACE) shape-type:
      add_path_center_info(&(subMap_cellInfo[x_subMap][y_subMap][z_subMap]), path_num, TRACE);

      #ifdef DEBUG_addDiffPairPathCentersToSubMap
      if (DEBUG_ON)  {
        printf("DEBUG:   Added TRACE path-center for path #%d at (%d,%d,%d) in sub-map, or (%d,%d,%d) in main map.\n",
               path_num, x_subMap, y_subMap, z_subMap, x_mainMap, y_mainMap, z_mainMap);
      }
      #endif

      // If the previous segment has a lower Z-value than the current segment, then the previous segment
      // has a shape-type of VIA_UP, and the current segment has a shape-type of VIA_DOWN. Add these
      // shape-types to the cell in the sub-map:
      if (prev_z_mainMap < z_mainMap)  {

        // Before adding info to submap, confirm that coordinate resides within the submap:
        if ( ! xyzIsOutsideOfSubMap(prev_x_mainMap, prev_y_mainMap, prev_z_mainMap, connection))  {
          add_path_center_info(&(subMap_cellInfo[prev_x_subMap][prev_y_subMap][prev_z_subMap]), path_num, VIA_UP);
        }

        // Before adding info to submap, confirm that coordinate resides within the submap:
        if ( ! xyzIsOutsideOfSubMap(x_mainMap, y_mainMap, z_mainMap, connection))  {
          add_path_center_info(&(subMap_cellInfo[x_subMap][y_subMap][z_subMap]), path_num, VIA_DOWN);
        }

        #ifdef DEBUG_addDiffPairPathCentersToSubMap
        if (DEBUG_ON)  {
          printf("DEBUG:   Added VIA_UP path-center for path #%d at (%d,%d,%d) in sub-map, or (%d,%d,%d) in main map.\n",
                 path_num, prev_x_subMap, prev_y_subMap, prev_z_subMap, prev_x_mainMap, prev_y_mainMap, prev_z_mainMap);
          printf("DEBUG:   Added VIA_DOWN path-center for path #%d at (%d,%d,%d) in sub-map, or (%d,%d,%d) in main map.\n",
                 path_num, x_subMap, y_subMap, z_subMap, x_mainMap, y_mainMap, z_mainMap);
        }
        #endif
      }
      // If the previous segment has a higher Z-value than the current segment, then the previous segment
      // has a shape-type of VIA_DOWN, and the current segment has a shape-type of VIA_UP. Add these
      // shape-types to the cell in the sub-map:
      else if (prev_z_mainMap > z_mainMap)  {

        // Before adding info to submap, confirm that coordinate resides within the submap:
        if ( ! xyzIsOutsideOfSubMap(prev_x_mainMap, prev_y_mainMap, prev_z_mainMap, connection))  {
          add_path_center_info(&(subMap_cellInfo[prev_x_subMap][prev_y_subMap][prev_z_subMap]), path_num, VIA_DOWN);
        }

        // Before adding info to submap, confirm that coordinate resides within the submap:
        if ( ! xyzIsOutsideOfSubMap(x_mainMap, y_mainMap, z_mainMap, connection))  {
          add_path_center_info(&(subMap_cellInfo[x_subMap][y_subMap][z_subMap]), path_num, VIA_UP);
        }


        #ifdef DEBUG_addDiffPairPathCentersToSubMap
        if (DEBUG_ON)  {
          printf("DEBUG:   Added VIA_DOWN path-center for path #%d at (%d,%d,%d) in sub-map, or (%d,%d,%d) in main map.\n",
                 path_num, prev_x_subMap, prev_y_subMap, prev_z_subMap, prev_x_mainMap, prev_y_mainMap, prev_z_mainMap);
          printf("DEBUG:   Added VIA_UP path-center for path #%d at (%d,%d,%d) in sub-map, or (%d,%d,%d) in main map.\n",
                 path_num, x_subMap, y_subMap, z_subMap, x_mainMap, y_mainMap, z_mainMap);
        }
        #endif
      }
    }

    // In anticipation of the next iteration through this for-loop, copy the current segment's
    // coordinates into the 'prev segment' variables:
    prev_x_mainMap = x_mainMap;
    prev_y_mainMap = y_mainMap;
    prev_z_mainMap = z_mainMap;

    prev_x_subMap = x_subMap;
    prev_y_subMap = y_subMap;
    prev_z_subMap = z_subMap;

  }  // End of for-loop to iterate over path segments

}  // End of function 'addDiffPairPathCentersToSubMap'


//-----------------------------------------------------------------------------
// Name: convertCongestionAtCell
// Desc: Modify the congestion at a cell of type CellInfo_t such
//       that all congestion from path 'partnerPath' and shape-type 'shapeType'
//       is converted to path 'pathNum'. If the cell is in a swap-zone, then
//       this function performs no actions, since there should never be any
//       congestion in a swap-zone.
//-----------------------------------------------------------------------------
// Define 'DEBUG_addCongestion' and re-compile if you want verbose debugging
// of functions that add congestion
// #define DEBUG_addCongestion
#undef DEBUG_addCongestion

static void convertCongestionAtCell(int pathNum, int partnerPath, int shapeType, CellInfo_t *cellInfo)  {

  // Save the original number of traversing paths before any changes are made.
  // We'll use this saved value later to decide whether to re-allocate (shrink) memory
  // for the 'congestion[]' array:
  int origNumTraversingPaths = cellInfo->numTraversingPaths;

  // Iterate over the congestion at the cell:
  int pathIndex = 0;
  while (pathIndex < cellInfo->numTraversingPaths)  {

    // Get the path number and shape-type of the congestion for the current path index:
    int congestion_pathNum   = cellInfo->congestion[pathIndex].pathNum;
    int congestion_shapeType = cellInfo->congestion[pathIndex].shapeType;

    // If the congestion satisfies all of the following three criteria, then convert this congestion to
    // path 'pathNum' with the same shape-type and design-rule subset:
    //   (a) path-number matches 'partnerPath', and
    //   (b) shape-type matches 'shapeType', and
    //   (c) the congestion amount is non-zero.
    if (   (congestion_pathNum == partnerPath)                            // Criterion (a) in above list
        && (congestion_shapeType == shapeType)                            // Criterion (b) in above list
        && (cellInfo->congestion[pathIndex].pathTraversalsTimes100 > 0))  // Criterion (c) in above list
    {
      int congestion_DRsubset  = cellInfo->congestion[pathIndex].DR_subset;

      // Check whether the cell already contains congestion with the same
      // shape-type and design-rule subset, but with path number 'pathNum':
      #ifdef DEBUG_addCongestion
      printf("\nDEBUG: About to call getIndexOfTraversingPath with path=%d, DRsubset=%d, and shapeType=%d\n",
             pathNum, congestion_DRsubset, congestion_shapeType);
      int found_path_index = getIndexOfTraversingPath(cellInfo, pathNum, congestion_DRsubset, congestion_shapeType, FALSE);
      #endif
      int found_path_index = getIndexOfTraversingPath(cellInfo, pathNum, congestion_DRsubset, congestion_shapeType);
      if (found_path_index == -1)  {
        // We got here, so the cell at (x,y,z) does not contain congestion from path 'pathNum' with
        // shape-type 'congestion_shapeType' and design-rule subset 'congestion_DRsubset'. We therefore
        // can simply convert the information at this pathIndex from path 'partnerPath' to
        // path 'pathNum':
        cellInfo->congestion[pathIndex].pathNum = pathNum;

      }  // End of if-block for found_path_index == -1
      else  {
        // We got here, so the cell already contains congestion from path 'pathNum' with
        // shape-type 'congestion_shapeType' and design-rule subset 'congestion_DRsubset'.
        // Add the congestion from path 'partnerPath' to the congestion for path 'pathNum':

        #ifdef DEBUG_addCongestion
        printf("\nDEBUG: getIndexOfTraversingPath returned with found_path_index=%d for path=%d, DRsubset=%d, and shapeType=%d\n",
               found_path_index, pathNum, congestion_DRsubset, congestion_shapeType);
        #endif

        unsigned int old_congestion = cellInfo->congestion[found_path_index].pathTraversalsTimes100;
        unsigned int new_congestion = old_congestion + cellInfo->congestion[pathIndex].pathTraversalsTimes100;
        assignCongestionByPathIndex(cellInfo, found_path_index, new_congestion);

        // For the subsequent paths that traverse the cell, move the path number and its congestion 'down' by 1.
        // That is, reduce the index values for these subsequent paths, thereby overwriting the information
        // at the current pathIndex value:
        for (int oldPathIndex = pathIndex + 1; oldPathIndex < cellInfo->numTraversingPaths; oldPathIndex++)  {

          // Assign path number, DR_subset, shape type, and congestion to (oldPathIndex - 1):
          cellInfo->congestion[oldPathIndex - 1].pathNum                = cellInfo->congestion[oldPathIndex].pathNum;
          cellInfo->congestion[oldPathIndex - 1].DR_subset              = cellInfo->congestion[oldPathIndex].DR_subset;
          cellInfo->congestion[oldPathIndex - 1].shapeType              = cellInfo->congestion[oldPathIndex].shapeType;
          cellInfo->congestion[oldPathIndex - 1].pathTraversalsTimes100 = cellInfo->congestion[oldPathIndex].pathTraversalsTimes100;

        }  // End of for-loop for index 'oldPathIndex'

        // Decrease 'pathIndex' by 1 because we just decremented all the indices
        // of the traversing paths. So we need to process the same pathIndex value
        // now that it contains different data.
        pathIndex--;

        // Reduce the new number of congestion indices by 1. Note that this might reduce it to zero.
        cellInfo->numTraversingPaths--;

      }  // End of else-block for found_path_index is *not* -1
    }  // End of if-block for congestion_pathNum == partnerPath

    // Increment 'pathIndex' so we can move on to the next traversing path at the cell.
    pathIndex++;

  }  // End of while-loop for index 'pathIndex' (0 to pathCount)

  // If any paths' congestion was zero'd out, then re-size (or free) the array
  // for 'congestion->congestion' array so the array requires fewer elements:
  if (cellInfo->numTraversingPaths != origNumTraversingPaths)  {
    if (cellInfo->numTraversingPaths)  {
      cellInfo->congestion = realloc(cellInfo->congestion,
                                             cellInfo->numTraversingPaths * sizeof(Congestion_t));

    }  // End of if-block for having non-zero number of elements in congestion[] array
    else  {
      // There is no longer any congestion at the cell, so free the
      // memory in the 1-dimensional 'congestion' matrix for this cell:
      free(cellInfo->congestion);
      cellInfo->congestion = NULL; // Set pointer to NULL as a precaution
    }  // End of else-block for having zero elements in congestion[] array
  }  // End of if-block for the (new) number of traversing paths being different (less than) the original

}  // End of function 'convertCongestionAtCell'


//-----------------------------------------------------------------------------
// Name: convertCongestionAlongPath
// Desc: Modify the congestion along the path given by 'pathCoords' with path
//       length 'pathLength' such that these cells provide a low-cost path
//       for path number 'pathNum'. This is accomplished by converting all
//       congestion of path-number 'partnerPath' to that of 'pathNum'. This
//       function assumes that 'pathCoords' consists only of legal jumps. The
//       congestion changes are made along 'pathCoords', in addition to
//       intermediate cells in the case of diagonal and knight's jumps.
//
//       This function assumes that all segments in 'pathCoords' are within
//       the map defined by mapInfo and cellInfo.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_convertCongestionAlongPath' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_convertCongestionAlongPath 1
#undef DEBUG_convertCongestionAlongPath

static void convertCongestionAlongPath(int pathNum, int partnerPath, int pathLength,
                                       Coordinate_t *pathCoords, CellInfo_t ***cellInfo,
                                       MapInfo_t *mapInfo)  {

  #ifdef DEBUG_convertCongestionAlongPath
  int DEBUG_ON = FALSE;
  if (   (mapInfo->current_iteration >= 84) && (mapInfo->current_iteration <= 84)
      && ((pathNum >= 0) || (pathNum <= 1)))  {
    DEBUG_ON = TRUE;
    printf("\nDEBUG: Entered function convertCongestionAlongPath in iteration %d with:\n", mapInfo->current_iteration);
    printf(  "DEBUG:           pathNum = %d\n", pathNum);
    printf(  "DEBUG:       partnerPath = %d\n", partnerPath);
    printf(  "DEBUG:        pathLength = %d\n", pathLength);

    // printf("\nDEBUG: Iteration %d: At beginning of convertCongestionAlongPath, congestion at (284,53,2) is:\n", mapInfo->current_iteration);
    // print_cell_congestion(&(cellInfo[284][53][2]));
    // printf("\nDEBUG: Iteration %d: At beginning of convertCongestionAlongPath, congestion at (284,53,3) is:\n", mapInfo->current_iteration);
    // print_cell_congestion(&(cellInfo[284][53][3]));

    if (pathLength > 0)  {
      printf("DEBUG:  first coordinate = (%d,%d,%d)\n", pathCoords[0].X, pathCoords[0].Y, pathCoords[0].Z);
      printf("DEBUG:   last coordinate = (%d,%d,%d)\n\n", pathCoords[pathLength - 1].X, pathCoords[pathLength - 1].Y, pathCoords[pathLength - 1].Z);

      printf("DEBUG: Congestion will be optimized for path %d along the following %d coordinates:\n", pathNum, pathLength);
      printf("DEBUG:   ");
      for (int seg = 0; seg < pathLength; seg++)  {
        printf(  "(%d,%d,%d)   ", pathCoords[seg].X, pathCoords[seg].Y, pathCoords[seg].Z);
      }  // End of for-loop for index 'seg' (segment)
      printf("\n");
    }  // End of if-block for pathLength > 0
    else  {
      printf("\nDEBUG: Congestion will NOT be optimized for path %d because no path was defined (pathLength = %d).\n", pathNum, pathLength);
    }
  }  // End of if-block for specific criteria being satisfied
  #endif

  //
  // Check of the path length is non-zero. If it's zero, this this function performs no action:
  //
  if (pathLength > 0)  {
    //
    // Iterate over the non-contiguous segments of the path:
    //
    // Start with the first segment:
    Coordinate_t prev_segment = copyCoordinates(pathCoords[0]);

    // Convert congestion at start-segment if it's not in a swap-zone:
    if (! cellInfo[prev_segment.X][prev_segment.Y][prev_segment.Z].swap_zone)  {

      #ifdef DEBUG_convertCongestionAlongPath
      if (DEBUG_ON)  {
        printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
               prev_segment.X, prev_segment.Y, prev_segment.Z, partnerPath, pathNum, TRACE);
      }
      #endif
      convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[prev_segment.X][prev_segment.Y][prev_segment.Z]));

      // Determine the direction of routing from the first segment to the next segment (if there is a second segment). If
      // it's a via, then convert the congestion for the appropriate via-type:
      if (pathLength >=2)  {
        if (pathCoords[1].Z > pathCoords[0].Z)  {

          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   prev_segment.X, prev_segment.Y, prev_segment.Z, partnerPath, pathNum, VIA_UP);
          }
          #endif
          convertCongestionAtCell(pathNum, partnerPath, VIA_UP, &(cellInfo[prev_segment.X][prev_segment.Y][prev_segment.Z]));
        }
        else if (pathCoords[1].Z < pathCoords[0].Z)  {

          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   prev_segment.X, prev_segment.Y, prev_segment.Z, partnerPath, pathNum, VIA_DOWN);
          }
          #endif
          convertCongestionAtCell(pathNum, partnerPath, VIA_DOWN, &(cellInfo[prev_segment.X][prev_segment.Y][prev_segment.Z]));
        }
      }  // End of if-block for pathLength >= 2
    }  // End of if-block for cell not being in a swap-zone

    // Now iterate over all subsequent segments:
    for (int seg = 1; seg < pathLength; seg++)  {

      // Get the x/y/z coordinates of the current and previous segments:
      int x      = pathCoords[seg].X;
      int y      = pathCoords[seg].Y;
      int z      = pathCoords[seg].Z;
      int prev_x = prev_segment.X;
      int prev_y = prev_segment.Y;
      int prev_z = prev_segment.Z;

      // Convert congestion at (x,y,z) if it's not in a swap-zone:
      if (! cellInfo[x][y][z].swap_zone)  {
        #ifdef DEBUG_convertCongestionAlongPath
        if (DEBUG_ON)  {
          printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                 x, y, z, partnerPath, pathNum, TRACE);
        }
        #endif
        convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[x][y][z]));

        // Determine the direction of routing from the previous segment to the current segment. If
        // it's a via, then convert the congestion for the appropriate via-type of the current and
        // previous segments:
        if (z < prev_z)  {

          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   prev_x, prev_y, prev_z, partnerPath, pathNum, VIA_DOWN);
          }
          #endif

          // Convert previous segment's VIA_DOWN congestion:
          convertCongestionAtCell(pathNum, partnerPath, VIA_DOWN, &(cellInfo[prev_x][prev_y][prev_z]));

          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   x, y, z, partnerPath, pathNum, VIA_UP);
          }
          #endif

          // Convert current segment's VIA_UP congestion:
          convertCongestionAtCell(pathNum, partnerPath, VIA_UP, &(cellInfo[x][y][z]));
        }
        else if (z > prev_z)  {

          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   prev_x, prev_y, prev_z, partnerPath, pathNum, VIA_UP);
          }
          #endif

          // Convert previous segment's VIA_UP congestion:
          convertCongestionAtCell(pathNum, partnerPath, VIA_UP, &(cellInfo[prev_x][prev_y][prev_z]));


          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   x, y, z, partnerPath, pathNum, VIA_DOWN);
          }
          #endif

          // Convert current segment's VIA_DOWN congestion:
          convertCongestionAtCell(pathNum, partnerPath, VIA_DOWN, &(cellInfo[x][y][z]));
        }

      }  // End of if-block for cell not being in a swap-zone

      // Calculate the direction and magnitude of the jump between the previous segment
      // and the current segment:
      int delta_x = x - prev_x;
      int delta_y = y - prev_y;

      // If the current segment represents a diagonal jump from the previous
      // segment, then convert the congestion at the two corner cells:
      if (abs(delta_x) + abs(delta_y) == 2)  {

        // Convert congestion at (prev_x + delta_x, prev_y, z) if it's not in a swap-zone:
        if (! cellInfo[prev_x + delta_x][prev_y][z].swap_zone)  {
          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at diagonal corner-cell (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   prev_x + delta_x, prev_y, z, partnerPath, pathNum, TRACE);
          }
          #endif
          convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[prev_x + delta_x][prev_y][z]));
        }

        // Convert congestion at (prev_x, prev_y + delta_y, z) if it's not in a swap-zone:
        if (! cellInfo[prev_x][prev_y + delta_y][z].swap_zone)  {
          #ifdef DEBUG_convertCongestionAlongPath
          if (DEBUG_ON)  {
            printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at diagonal corner-cell (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                   prev_x, prev_y + delta_y, z, partnerPath, pathNum, TRACE);
          }
          #endif
          convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[prev_x][prev_y + delta_y][z]));
        }
      }  // End of if-block for a diagonal jump

      else if (abs(delta_x) + abs(delta_y) == 3)  {
        // We got here, so the current segment is a knight's jump from the previous segment.
        if (abs(delta_x) == 1)  {
          // We got here, so the knight's jump has a delta_x of +/-1 and a
          // delta_y of +/-2. Convert the congestion at the two corner cells:

          // Convert congestion at (prev_x, prev_y + delta_y/2, z) if it's not in a swap-zone:
          if (! cellInfo[prev_x][prev_y + delta_y/2][z].swap_zone)  {
            #ifdef DEBUG_convertCongestionAlongPath
            if (DEBUG_ON)  {
              printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at knight's corner-cell (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                     prev_x, prev_y + delta_y/2, z, partnerPath, pathNum, TRACE);
            }
            #endif
            convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[prev_x][prev_y + delta_y/2][z]));
          }

          // Convert congestion at (prev_x + delta_x, prev_y + delta_y/2, z) if it's not in a swap-zone:
          if (! cellInfo[prev_x + delta_x][prev_y + delta_y/2][z].swap_zone)  {
            #ifdef DEBUG_convertCongestionAlongPath
            if (DEBUG_ON)  {
              printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at knight's corner-cell (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                     prev_x + delta_x, prev_y + delta_y/2, z, partnerPath, pathNum, TRACE);
            }
            #endif
            convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[prev_x + delta_x][prev_y + delta_y/2][z]));
          }
        }
        else  {
          // We got here, so the knight's jump has a delta_x of +/-2 and a
          // delta_y of +/-1. Convert the congestion at the two corner cells:

          // Convert congestion at (prev_x + delta_x/2, prev_y, z) if it's not in a swap-zone:
          if (! cellInfo[prev_x + delta_x/2][prev_y][z].swap_zone)  {
            #ifdef DEBUG_convertCongestionAlongPath
            if (DEBUG_ON)  {
              printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at knight's corner-cell (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                     prev_x + delta_x/2, prev_y, z, partnerPath, pathNum, TRACE);
            }
            #endif
            convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[prev_x + delta_x/2][prev_y][z]));
          }

          // Convert congestion at (prev_x + delta_x/2, prev_y + delta_y, z) if it's not in a swap-zone:
          if (! cellInfo[prev_x + delta_x/2][prev_y + delta_y][z].swap_zone)  {
            #ifdef DEBUG_convertCongestionAlongPath
            if (DEBUG_ON)  {
              printf("DEBUG: In convertCongestionAlongPath, about to convert congestion at knight's corner-cell (%d,%d,%d) from path %d to %d for shape-type %d.\n",
                     prev_x + delta_x/2, prev_y + delta_y, z, partnerPath, pathNum, TRACE);
            }
            #endif
            convertCongestionAtCell(pathNum, partnerPath, TRACE, &(cellInfo[prev_x + delta_x/2][prev_y + delta_y][z]));
          }

        }  // End of else-block for knight's jump with delta_x of +/-2
      }  // End of if-block for a knight's jump

      // In preparation for the next iteration through this loop, copy the current
      // segment's coordinates to the 'prev_segment' variable:
      prev_segment = copyCoordinates(pathCoords[seg]);

    }  // End of for-loop for index 'seg' (0 to pathLength)
  }  // End of if-block for pathLength > 0

  #ifdef DEBUG_convertCongestionAlongPath
  if (DEBUG_ON)  {
    // printf("\nDEBUG: Iteration %d: At end of convertCongestionAlongPath, congestion at (284,53,2) is:\n", mapInfo->current_iteration);
    // print_cell_congestion(&(cellInfo[284][53][2]));
    // printf("\nDEBUG: Iteration %d: At end of convertCongestionAlongPath, congestion at (284,53,3) is:\n", mapInfo->current_iteration);
    // print_cell_congestion(&(cellInfo[284][53][3]));

    printf("\nDEBUG: Exiting function convertCongestionAlongPath.\n\n");
  }
  #endif

}  // End of function 'convertCongestionAlongPath'


//-----------------------------------------------------------------------------
// Name: optimizeDiffPairConnections
// Desc: Optimize the connections between diff-pair shoulder-paths and the
//       corresponding diff-pair vias and terminals, respecting whether the
//       diff-pair is P/N-swappable. This function modifies the pathCoords[][]
//       array and the pathLengths[] array. For P/N-swappable diff-pairs, this
//       function may also modify mapInfo->start_cells[] and
//       mapInfo->diff_pair_terms_swapped[].
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_optimizeDiffPairConnections' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_optimizeDiffPairConnections 1
#undef DEBUG_optimizeDiffPairConnections

// Define 'DEBUG_addCongestion' and re-compile if you want verbose debugging
// of functions that add congestion
// #define DEBUG_addCongestion
#undef DEBUG_addCongestion

void optimizeDiffPairConnections(Coordinate_t *pathCoords[], int pathLengths[], CellInfo_t ***cellInfo,
                                 MapInfo_t *mapInfo, InputValues_t *user_inputs, RoutingMetrics_t *routability,
                                 RoutingMetrics_t subMapRoutability[2], RoutingRestriction_t *noRoutingRestrictions,
                                 int num_threads)  {

  // Define the number of sub-iterations required to have the same 'swap' value for diff-pair
  // connections before this function stops running additional sub-iterations:
  const int numIterationsWithStableSwapValue = 5;

  // Define the number of sub-iterations required to have nearly identical
  // 'symmetryRatio' values for diff-pair connections before this function
  // stops running additional sub-iterations:
  const int numIterationsWithStableSymmetryRatio = 3;

  // Define the allowed deviation in symmetryRatio from sub-iteration to sub-iteration
  // that's allowed in order to categorize the symmetryRatio as stable:
  const float symmetryRatio_stability_tolerance = 0.0001;

  // Define lower and upper thresholds for the symmetryRatio to be considered
  // too close to 0.500 to stop sub-iterations:
  const float symmetryRatio_low_threshold  = 0.495;
  const float symmetryRatio_high_threshold = 0.505;

  // Define variables that are used to print time-stamps:
  time_t tim;
  struct tm *now;

  // printf("DEBUG: Upon entering optimizeDiffPairConnections, omp_get_num_threads = %d\n", omp_get_num_threads());

  #ifdef DEBUG_optimizeDiffPairConnections
  // DEBUG code follows:
  //
  // Variable used for debugging:
  int DEBUG_ON = FALSE;

  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  if ((mapInfo->current_iteration >= 55) && (mapInfo->current_iteration <= 55))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in optimizeDiffPairConnections() because specific requirements were met in iteration %d.\n\n",
           mapInfo->current_iteration);
    DEBUG_ON = TRUE;

    printf("DEBUG: optimizeDiffPairConnections was entered with num_threads = %d\n\n", num_threads);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE

  #endif

  // Make a local copy of the number of pseudo-paths in the map:
  int numPseudoPaths = user_inputs->num_pseudo_nets;

  // Make a local copy of the total number of routed paths in the entire map:
  int num_routed_nets = user_inputs->num_nets + user_inputs->num_pseudo_nets;
  // printf("DEBUG: In function optimizeDiffPairConnections, num_routed_nets = %d (sum of %d and %d).\n",
  //        num_routed_nets, user_inputs->num_nets, user_inputs->num_pseudo_nets);


  // Create 1st dimension of 2D array that will contain info for each trace-to-terminal
  // and trace-to-via connection for all pseudo-paths:
  ShoulderConnections_t *shoulderConnections;
  shoulderConnections = malloc(numPseudoPaths * sizeof(ShoulderConnections_t));
  if (shoulderConnections == NULL)  {
    printf("\nERROR: Memory was not successfully allocated in function 'optimizeDiffPairConnections' for array\n");
    printf(  "       'shoulderConnections'. Please inform the software developer of this fatal error message.\n\n");
    exit(1);
  }

  //
  // Analyze each pseudoPath to record the connection information between traces and vias, and
  // between traces and terminals. The 'maxConnectionsPerPath' variable holds the maximum number
  // of connections per path for all of the pseudo-paths.
  //
  int maxConnectionsPerPath = detectDiffPairConnections(shoulderConnections, mapInfo, user_inputs, pathCoords, pathLengths);

  // Print the detected connections to the log file:
  if (maxConnectionsPerPath)  {  // Check if any connections were detected
    printf("\nINFO: The following diff-pair connections were detected:\n");
    for (int i = 0; i < numPseudoPaths; i++)  {
      printf("INFO:   %d connections for pseudo-path %d, with %d pseudo-vias and PN_swappable = %d:\n",
             shoulderConnections[i].numConnections, shoulderConnections[i].pseudoPath,
             shoulderConnections[i].numPseudoVias, shoulderConnections[i].PN_swappable);
      printf("INFO:      Diff-pair path #1 = %d               Diff-pair path #2 = %d\n",
             shoulderConnections[i].diffPairPath_1, shoulderConnections[i].diffPairPath_2);

      for (int j = 0; j < shoulderConnections[i].numConnections; j++)  {
        printf("INFO:          Connection #%d, with same-layer terminals = %d:\n", j, shoulderConnections[i].connection[j].sameLayerTerminals);
        printf("INFO:                Path #1 (%d): Segment %d (%d,%d,%d) of type %d to segment %d (%d,%d,%d) of type %d\n",
               shoulderConnections[i].diffPairPath_1, shoulderConnections[i].connection[j].startSegment_1,
               shoulderConnections[i].connection[j].startCoord_1.X, shoulderConnections[i].connection[j].startCoord_1.Y,
               shoulderConnections[i].connection[j].startCoord_1.Z, shoulderConnections[i].connection[j].startShapeType_1,
               shoulderConnections[i].connection[j].endSegment_1, shoulderConnections[i].connection[j].endCoord_1.X,
               shoulderConnections[i].connection[j].endCoord_1.Y, shoulderConnections[i].connection[j].endCoord_1.Z,
               shoulderConnections[i].connection[j].endShapeType_1);
        printf("INFO:                Path #2 (%d): Segment %d (%d,%d,%d) of type %d to segment %d (%d,%d,%d) of type %d\n",
               shoulderConnections[i].diffPairPath_2, shoulderConnections[i].connection[j].startSegment_2,
               shoulderConnections[i].connection[j].startCoord_2.X, shoulderConnections[i].connection[j].startCoord_2.Y,
               shoulderConnections[i].connection[j].startCoord_2.Z, shoulderConnections[i].connection[j].startShapeType_2,
               shoulderConnections[i].connection[j].endSegment_2, shoulderConnections[i].connection[j].endCoord_2.X,
               shoulderConnections[i].connection[j].endCoord_2.Y, shoulderConnections[i].connection[j].endCoord_2.Z,
               shoulderConnections[i].connection[j].endShapeType_2);
      }  // End of for-loop for index 'j'
    }  // End of for-loop for index 'i'
    printf("\n");
  }  // End of if-block for maxConnectionsPerPath > 0

  // For each diff-pair connection, we analyze two configurations to determine
  // which is better:
  //   o  A non-swapped configuration, in which path_1's trace connects to
  //      path_1's terminal or via, and path_2's trace connects to path_2's
  //      terminal or via.
  //   o  A swapped configuration, in which path_1's trace connects to
  //      path_2's terminal or via, and path_2's trace connects to path_1's
  //      terminal or via.
  // As a reminder, the non-swapped configuration corresponds to index 0
  // (NOT_SWAPPED), and the swapped configuration as index 1 ('SWAPPED'):

  //
  // Analyze all connections in all pseudo-paths to calculate routing
  // and associated metrics for all possible connections of traces to
  // terminals and traces to vias:
  for (int i = 0; i < numPseudoPaths; i++)  {
    for (int j = 0; j < maxConnectionsPerPath; j++)  {
      // Check if 'j' is less than the number of connections for path 'i'. If so,
      // then process this connection:
      if (j < shoulderConnections[i].numConnections)  {

        // Capture the thread number, which will be used to pass the appropriate arrays
        // into functions:
        int thread_num = omp_get_thread_num();

        // Determine whether this connection's start-terminals are in a swap-zone, which will affect how we
        // handle the wiring configurations for this diff-pair connections.
        int startTermsInSwapZone;
        if ((j == 0) && mapInfo->swapZone[shoulderConnections[i].pseudoPath])  {
          startTermsInSwapZone = TRUE;
        }
        else  {
          startTermsInSwapZone = FALSE;
        }

        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: (thread %2d) -----------------------------------------------------------------------------------\n", thread_num);
          printf(  "DEBUG: (thread %2d) In optimizeDiffPairConnections, analyzing pseudo-path %d, connection %d (startTermsInSwapZone = %d)\n",
                 thread_num, shoulderConnections[i].pseudoPath, j, startTermsInSwapZone);
          printf(  "DEBUG: (thread %2d) -----------------------------------------------------------------------------------\n", thread_num);
        }
        #endif

        // Make local copies of the diff-pair path numbers:
        int pathNums[2];
        pathNums[0] = shoulderConnections[i].diffPairPath_1;
        pathNums[1] = shoulderConnections[i].diffPairPath_2;

        //
        // Check for the rare situation in which a start-terminal of the connection has the same
        // x/y/z coordinates as an end-terminal of the same connection. If this is found, then
        // set the 'swap' parameter to ensure that the start- and end-terminals with identical
        // x/y/z coordinates are connected to the same diff-pair path:
        //
        // Check if startCoord_1 matches endCoord_1. (This should NEVER happen, but we check just in case.)
        if (   (shoulderConnections[i].connection[j].startCoord_1.X == shoulderConnections[i].connection[j].endCoord_1.X)
            && (shoulderConnections[i].connection[j].startCoord_1.Y == shoulderConnections[i].connection[j].endCoord_1.Y)
            && (shoulderConnections[i].connection[j].startCoord_1.Z == shoulderConnections[i].connection[j].endCoord_1.Z))  {
          // We got here, so start-coordinate #1 has the same x/y/z coordinates as end-coordinate #1. Set the 'swap'
          // element to 'FALSE' and the symmetryRatio to its minimum possible value of 0.0:
          shoulderConnections[i].connection[j].swap = FALSE;
          shoulderConnections[i].connection[j].symmetryRatio = 0.0;
          continue; // Continue on to the next connection
        }
        // Check if startCoord_2 matches endCoord_2. (This should NEVER happen, but we check just in case.)
        else if (  (shoulderConnections[i].connection[j].startCoord_2.X == shoulderConnections[i].connection[j].endCoord_2.X)
                && (shoulderConnections[i].connection[j].startCoord_2.Y == shoulderConnections[i].connection[j].endCoord_2.Y)
                && (shoulderConnections[i].connection[j].startCoord_2.Z == shoulderConnections[i].connection[j].endCoord_2.Z))  {
          // We got here, so start-coordinate #2 has the same x/y/z coordinates as end-coordinate #2. Set the 'swap'
          // element to 'FALSE' and the symmetryRatio to its minimum possible value of 0.0:
          shoulderConnections[i].connection[j].swap = FALSE;
          shoulderConnections[i].connection[j].symmetryRatio = 0.0;
          continue; // Continue on to the next connection
        }
        // Check if startCoord_1 matches endCoord_2. (This is possible, but should be rare.)
        else if (  (shoulderConnections[i].connection[j].startCoord_1.X == shoulderConnections[i].connection[j].endCoord_2.X)
                && (shoulderConnections[i].connection[j].startCoord_1.Y == shoulderConnections[i].connection[j].endCoord_2.Y)
                && (shoulderConnections[i].connection[j].startCoord_1.Z == shoulderConnections[i].connection[j].endCoord_2.Z))  {
          // We got here, so start-coordinate #1 has the same x/y/z coordinates as end-coordinate #2. Set the 'swap'
          // element to 'TRUE' and the symmetryRatio to its maximum possible value of 1.0:
          shoulderConnections[i].connection[j].swap = TRUE;
          shoulderConnections[i].connection[j].symmetryRatio = 1.0;
          continue; // Continue on to the next connection
        }
        // Check if startCoord_2 matches endCoord_1. (This is possible, but should be rare.)
        else if (  (shoulderConnections[i].connection[j].startCoord_2.X == shoulderConnections[i].connection[j].endCoord_1.X)
                && (shoulderConnections[i].connection[j].startCoord_2.Y == shoulderConnections[i].connection[j].endCoord_1.Y)
                && (shoulderConnections[i].connection[j].startCoord_2.Z == shoulderConnections[i].connection[j].endCoord_1.Z))  {
          // We got here, so start-coordinate #2 has the same x/y/z coordinates as end-coordinate #1. Set the 'swap'
          // element to 'TRUE' and the symmetryRatio to its maximum possible value of 1.0:
          shoulderConnections[i].connection[j].swap = TRUE;
          shoulderConnections[i].connection[j].symmetryRatio = 1.0;
          continue; // Continue on to the next connection
        }


        //
        // Check whether the current connection had design-rule violations on the most recent iteration.
        // If the connection doesn't start in a swap-zone and if the connection was previously DRC-clean, then
        // attempt to use heuristics to determine whether the wiring configuration should/shouldn't be swapped
        // for this connection:
        if (   (! startTermsInSwapZone)
            && (! connection_has_DRCs_in_previous_iteration(pathNums[0], pathNums[1], shoulderConnections[i].connection[j], routability)))  {

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("DEBUG: In optimizeDiffPairConnections during iteration %d, connection %d of pseudo-path %d had no DRCs in previous iteration.\n",
                   mapInfo->current_iteration, j, shoulderConnections[i].pseudoPath);
          }
          #endif

          // We got here, so the current connection had no DRCs on the previous iteration. Check
          // whether the current connection can be optimized using simple geometric considerations.
          // If so, then update the 'symmetryRatio' and 'swap' elements of the ShoulderConnection_t
          // structure and move on to the next connection:
          //
          if (optimize_using_geometry(&shoulderConnections[i].connection[j]))  {
            // We got here, so the result from function 'optimize_using_geometry' was TRUE, indicating that
            // it successfully optimized the connection using geometric considerations.

            tim = time(NULL);
            now = localtime(&tim);
            printf("INFO: (thread %2d) Optimized connection %d of pseudo-path %d in iteration %d using lateral distances at %02d-%02d-%d, %02d:%02d:%02d\n",
                   thread_num, j, shoulderConnections[i].pseudoPath, mapInfo->current_iteration,
                   now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

            continue;  // Continue on to the next connection
          }  // End of if-block for optimize_using_geometry() == TRUE
          else  {
            #ifdef DEBUG_optimizeDiffPairConnections
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) In optimizeDiffPairConnections, geometric heuristics could not conclusively determine the optimized wiring for connection %d of pseudo-path %d.\n",
                     thread_num, j, shoulderConnections[i].pseudoPath);
            }
            #endif
          }  // End of else-block

          // We got here, so the current connection was DRC-clean on the previous iteration, but the wiring
          // configuration could not be conclusively calculated based solely on geometric considerations. We
          // next check whether the previous, DRC-clean wiring configuration can be inferred from
          // the congestion in the map.
          if (optimize_using_congestion(pathNums[0], pathNums[1], &shoulderConnections[i].connection[j], mapInfo, cellInfo, user_inputs))  {
            // printf("\nDEBUG: (thread %2d) In optimizeDiffPairConnections, swap = %d based on congestion heuristics for connection %d of pseudo-path %d (symmetryRatio = %.3f\n",
            //        thread_num, shoulderConnections[i].connection[j].swap, j, shoulderConnections[i].pseudoPath, shoulderConnections[i].connection[j].symmetryRatio);

            tim = time(NULL);
            now = localtime(&tim);
            printf("INFO: (thread %2d) Optimized connection %d of pseudo-path %d in iteration %d using previous congestion at %02d-%02d-%d, %02d:%02d:%02d\n",
                   thread_num, j, shoulderConnections[i].pseudoPath, mapInfo->current_iteration,
                   now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

            continue;  // Continue on to the next connection
          }  // End of if-block for optimize_using_congestion == TRUE
          else  {
            #ifdef DEBUG_optimizeDiffPairConnections
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) In optimizeDiffPairConnections, congestion heuristics could not conclusively determine the optimized wiring for connection %d of pseudo-path %d.\n",
                     thread_num, j, shoulderConnections[i].pseudoPath);
            }
            #endif
          }  // End of else-block
        }  // End of if-block for connection having no DRCs in previous iteration
        else  {
          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("DEBUG: In optimizeDiffPairConnections during iteration %d, connection %d of pseudo-path %d had DRCs in previous iteration.\n",
                   mapInfo->current_iteration, j, shoulderConnections[i].pseudoPath);
          }
          #endif
        }

        //
        // We got here, so the diff-pair paths involved with this connection had DRCs
        // on the previous iteration on the layers associated with this connection, or
        // the above geometric algorithms could not conclusively calculate the best
        // wiring configuration, or the connection starts in a swap-zone. We therefore
        // use the compute-intesive algorithm of path-finding the best wiring configuration.
        //

        // 'pathFound' is TRUE only after function findPath() successfully finds paths
        // between all the terminals within the current connection.
        unsigned char pathFound = FALSE;

        // 'mapSizeMultiplier' is a scaling factor that increases the size of the sub-map
        // if findPath() fails to find a path between any of the terminals within the
        // current connection:
        int mapSizeMultiplier = 0;

        tim = time(NULL);
        now = localtime(&tim);
        printf("INFO: (thread %2d) Starting path-finding to optimize connection %d of pseudo-path %d in iteration %d at %02d-%02d-%d, %02d:%02d:%02d\n",
               thread_num, j, shoulderConnections[i].pseudoPath, mapInfo->current_iteration,
               now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

        while (! pathFound)  {
          mapSizeMultiplier++;  // Starts at 1, then increases to 2, 3, ..., if sub-map size needs to be enlarged

          //
          // Analyze the current connection to determine the x-, y-, and z-extents of the
          // sub-map that will be used for auto-routing the wires in this connection
          //
          calcSubMapDimensions(&shoulderConnections[i].connection[j], mapInfo, mapSizeMultiplier);

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) After calcSubMapDimensions:\n", thread_num);
            printf("DEBUG: (thread %2d)              shoulderConnections[%d].connection[%d].minCoord = (%d,%d,%d)\n",
                   thread_num, i, j, shoulderConnections[i].connection[j].minCoord.X,
                                     shoulderConnections[i].connection[j].minCoord.Y,
                                     shoulderConnections[i].connection[j].minCoord.Z);
            printf("DEBUG: (thread %2d)              shoulderConnections[%d].connection[%d].maxCoord = (%d,%d,%d)\n",
                   thread_num, i, j, shoulderConnections[i].connection[j].maxCoord.X,
                                     shoulderConnections[i].connection[j].maxCoord.Y,
                                     shoulderConnections[i].connection[j].maxCoord.Z);
          }
          #endif

          // Define the x-/y-/z-offsets between the main map and the sub-map:
          int X_offset = shoulderConnections[i].connection[j].minCoord.X;
          int Y_offset = shoulderConnections[i].connection[j].minCoord.Y;
          int Z_offset = shoulderConnections[i].connection[j].minCoord.Z;

          // Define the start- and end-coordinates in the sub-map for the unswapped and
          // swapped configurations. The start-coordinates are the same between the
          // unswapped and swapped configurations, but the end-coordiantes are swapped.
          //  o  startCoord[pathIndex] is the start-coordinate for path number 'pathNums[pathIndex]'.
          //  o  endCoord[config][pathIndex] is the end-coordinate in wiring configuration
          //                                 'config' for path number 'pathNums[pathIndex]'.
          Coordinate_t startCoord[2];
          Coordinate_t endCoord[2][2];
          startCoord[0].X = shoulderConnections[i].connection[j].startCoord_1.X - X_offset;
          startCoord[0].Y = shoulderConnections[i].connection[j].startCoord_1.Y - Y_offset;
          startCoord[0].Z = shoulderConnections[i].connection[j].startCoord_1.Z - Z_offset;
          startCoord[1].X = shoulderConnections[i].connection[j].startCoord_2.X - X_offset;
          startCoord[1].Y = shoulderConnections[i].connection[j].startCoord_2.Y - Y_offset;
          startCoord[1].Z = shoulderConnections[i].connection[j].startCoord_2.Z - Z_offset;

          endCoord[NOT_SWAPPED][0].X  = shoulderConnections[i].connection[j].endCoord_1.X - X_offset;
          endCoord[NOT_SWAPPED][0].Y  = shoulderConnections[i].connection[j].endCoord_1.Y - Y_offset;
          endCoord[NOT_SWAPPED][0].Z  = shoulderConnections[i].connection[j].endCoord_1.Z - Z_offset;
          endCoord[NOT_SWAPPED][1].X  = shoulderConnections[i].connection[j].endCoord_2.X - X_offset;
          endCoord[NOT_SWAPPED][1].Y  = shoulderConnections[i].connection[j].endCoord_2.Y - Y_offset;
          endCoord[NOT_SWAPPED][1].Z  = shoulderConnections[i].connection[j].endCoord_2.Z - Z_offset;

          endCoord[SWAPPED][0].X = endCoord[NOT_SWAPPED][1].X;
          endCoord[SWAPPED][0].Y = endCoord[NOT_SWAPPED][1].Y;
          endCoord[SWAPPED][0].Z = endCoord[NOT_SWAPPED][1].Z;
          endCoord[SWAPPED][1].X = endCoord[NOT_SWAPPED][0].X;
          endCoord[SWAPPED][1].Y = endCoord[NOT_SWAPPED][0].Y;
          endCoord[SWAPPED][1].Z = endCoord[NOT_SWAPPED][0].Z;

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: (thread %2d) For pseudo-path %d, connection %d, in iteration %d:\n",
                   thread_num, shoulderConnections[i].pseudoPath, j, mapInfo->current_iteration);
            printf(  "DEBUG: (thread %2d)                         Submap offset: (%d,%d,%d)\n", thread_num, -X_offset, -Y_offset, -Z_offset);
            printf(  "DEBUG: (thread %2d)                 Original startCoord_1: (%d,%d,%d)\n", thread_num, shoulderConnections[i].connection[j].startCoord_1.X,
                   shoulderConnections[i].connection[j].startCoord_1.Y, shoulderConnections[i].connection[j].startCoord_1.Z);
            printf(  "DEBUG: (thread %2d)                 Original startCoord_2: (%d,%d,%d)\n\n", thread_num, shoulderConnections[i].connection[j].startCoord_2.X,
                   shoulderConnections[i].connection[j].startCoord_2.Y, shoulderConnections[i].connection[j].startCoord_2.Z);
            printf(  "DEBUG: (thread %2d)                 Sub-map startCoord[0]: (%d,%d,%d)\n", thread_num, startCoord[0].X, startCoord[0].Y, startCoord[0].Z);
            printf(  "DEBUG: (thread %2d)                 Sub-map startCoord[1]: (%d,%d,%d)\n\n", thread_num, startCoord[1].X, startCoord[1].Y, startCoord[1].Z);

            printf(  "DEBUG: (thread %2d)                   Original endCoord_1: (%d,%d,%d)\n", thread_num, shoulderConnections[i].connection[j].endCoord_1.X,
                   shoulderConnections[i].connection[j].endCoord_1.Y, shoulderConnections[i].connection[j].endCoord_1.Z);
            printf(  "DEBUG: (thread %2d)                   Original endCoord_2: (%d,%d,%d)\n\n", thread_num, shoulderConnections[i].connection[j].endCoord_2.X,
                   shoulderConnections[i].connection[j].endCoord_2.Y, shoulderConnections[i].connection[j].endCoord_2.Z);

            printf(  "DEBUG: (thread %2d)      Sub-map endCoord[NOT_SWAPPED][0]: (%d,%d,%d)\n", thread_num,
                   endCoord[NOT_SWAPPED][0].X, endCoord[NOT_SWAPPED][0].Y, endCoord[NOT_SWAPPED][0].Z);
            printf(  "DEBUG: (thread %2d)      Sub-map endCoord[NOT_SWAPPED][1]: (%d,%d,%d)\n\n", thread_num,
                   endCoord[NOT_SWAPPED][1].X, endCoord[NOT_SWAPPED][1].Y, endCoord[NOT_SWAPPED][1].Z);

            printf(  "DEBUG: (thread %2d)          Sub-map endCoord[SWAPPED][0]: (%d,%d,%d)\n", thread_num,
                   endCoord[SWAPPED][0].X, endCoord[SWAPPED][0].Y, endCoord[SWAPPED][0].Z);
            printf(  "DEBUG: (thread %2d)          Sub-map endCoord[SWAPPED][1]: (%d,%d,%d)\n\n", thread_num,
                   endCoord[SWAPPED][1].X, endCoord[SWAPPED][1].Y, endCoord[SWAPPED][1].Z);
          }
          #endif

          //
          // For the current connection, create a variable of type MapInfo_t and populate
          // its contents:
          //
          MapInfo_t subMapInfo;
          populate_subMapInfo(&subMapInfo, &(shoulderConnections[i].connection[j]), mapInfo, routability);

          // Create variables that will hold the coordinates and lengths of paths in the sub-map for
          // each of the two wiring configurations:
          int *subMapPathLengths[2];
          int *subMapContiguousPathLengths[2];
          Coordinate_t **subMapPathCoords[2];
          Coordinate_t **subMapContigPathCoords[2];

          // Create 3D arrays for each of 2 wire-configurations. These will be used in path-finding.
          CellInfo_t ***subMap_cellInfo[2];

          // For each of the 2 diff-pair paths, create pointers to the large arrays used by
          // the path-finding function, findPath().
          PathFinding_t subMapPathFinding[2];

          //
          // Iterate over both wire-configurations to allocate/initialize memory for arrays
          // that will be used by findPath():
          //
          for (int wire_config = NOT_SWAPPED; wire_config <= SWAPPED; wire_config++)  {

            // If this connection starts in a swap-zone, there is no need to check for swapped
            // and non-swapped wiring configurations, since they'll be virtually identical.
            // So we skip the SWAPPED configuration altogether.
            if (startTermsInSwapZone && (wire_config == SWAPPED))  {
              continue;
            }

            // For the current wire configuration, allocate memory from the heap for the array that
            // holds the lengths of the paths in the sub-map:
            subMapPathLengths[wire_config] = malloc(num_routed_nets * sizeof(int));
            if (subMapPathLengths[wire_config] == 0)  {
              printf("\nERROR: Unable to allocate %d elements for array subMapPathLengths for wire-configuration %d.\n\n",
                     num_routed_nets, wire_config);
              exit(1);
            }

            // For the current wire configuration, allocate memory from the heap for the array that
            // holds the lengths of the contiguous paths in the sub-map:
            subMapContiguousPathLengths[wire_config] = malloc(num_routed_nets * sizeof(int));
            if (subMapContiguousPathLengths[wire_config] == 0)  {
              printf("\nERROR: Unable to allocate %d elements for array subMapContiguousPathLengths for wire-configuration %d.\n\n",
                     num_routed_nets, wire_config);
              exit(1);
            }

            // For the current wire configuration, allocate memory from the heap for the 2-dimensional
            // array that holds the coordinates of each segment of each path in the sub-map:
            subMapPathCoords[wire_config] = malloc(num_routed_nets * sizeof(Coordinate_t *));
            if (subMapPathCoords[wire_config] == 0)  {
              printf("\nERROR: Unable to allocate %d elements for array subMapPathCoords for wire-configuration %d.\n\n",
                     num_routed_nets, wire_config);
              exit(1);
            }

            // For the current wire configuration, allocate memory from the heap for the 2-dimensional
            // array that holds the coordinates of each segment of each contiguous path in the sub-map:
            subMapContigPathCoords[wire_config] = malloc(num_routed_nets * sizeof(Coordinate_t *));
            if (subMapContigPathCoords[wire_config] == 0)  {
              printf("\nERROR: Unable to allocate %d elements for array subMapContigPathCoords for wire-configuration %d.\n\n",
                     num_routed_nets, wire_config);
              exit(1);
            }

            // For the current wire-configuration, dynamically allocate small amount of memory from heap
            // for 'subMapPathCoords' and 'subMapContigPathCoords' arrays of arrays. Also initialize to
            // zero the elements of subMapPathLengths and subMapContiguousPathLengths arrays:
            initializePathfinder(num_routed_nets, subMapPathLengths[wire_config], subMapPathCoords[wire_config],
                                 subMapContiguousPathLengths[wire_config], subMapContigPathCoords[wire_config]);

            // For the current wire-configuration, allocate memory for the subMap_cellInfo 3D matrix
            // that will be used during path-finding:
            subMap_cellInfo[wire_config] = allocateCellInfo(&subMapInfo);

            // For the current wire-configuratino, initialize to zero the elements of the sub-map
            // cellInfo 3D matrix before starting the iterative path-finding:
            initializeCellInfo(subMap_cellInfo[wire_config], &subMapInfo);

            // Copy the congestion and path-centers from the main map into the sub-map for the current wire-
            // configuration, excluding the congestion from the two diff-pair paths of the current connection:
            #ifdef DEBUG_optimizeDiffPairConnections
            if (DEBUG_ON)  {
              printf("\nDEBUG: (thread %2d) About to enter copyCellInfo in iteration %d, pseudo-path %d, connection %d, config %d...\n",
                     omp_get_thread_num(), mapInfo->current_iteration, shoulderConnections[i].pseudoPath, j, wire_config);
            }
            #endif
            copyCellInfo(cellInfo, subMap_cellInfo[wire_config], &subMapInfo, X_offset, Y_offset, Z_offset,
                         pathNums[0], pathNums[1]);

            //
            // Make the locations of both diff-pair paths' path-centers forbidden in both directions
            // (except for the start- and end-terminals of the current connection).
            //
            // Create forbidden cells from path 1's end-segment to the end of this path:
            makeContiguousForbiddenCells(pathNums[0], shoulderConnections[i].connection[j].endSegment_1, pathLengths[pathNums[0]],
                                         subMap_cellInfo[wire_config], shoulderConnections[i].connection[j], pathCoords, mapInfo);
            //
            // Create forbidden cells from path 1's start-segment to the beginning of this path:
            makeContiguousForbiddenCells(pathNums[0], shoulderConnections[i].connection[j].startSegment_1, -1,
                                         subMap_cellInfo[wire_config], shoulderConnections[i].connection[j], pathCoords, mapInfo);
            //
            // Create forbidden cells from path 2's end-segment to the end of this path:
            makeContiguousForbiddenCells(pathNums[1], shoulderConnections[i].connection[j].endSegment_2, pathLengths[pathNums[1]],
                                         subMap_cellInfo[wire_config], shoulderConnections[i].connection[j], pathCoords, mapInfo);
            //
            // Create forbidden cells from path 2's start-segment to the beginning of this path:
            makeContiguousForbiddenCells(pathNums[1], shoulderConnections[i].connection[j].startSegment_2, -1,
                                         subMap_cellInfo[wire_config], shoulderConnections[i].connection[j], pathCoords, mapInfo);

            // Initialize the elements in subMapRoutability[thread_num][wire_config) for
            // the current wiring configuration, eliminating the 'memory' of routing metrics
            // from other diff-pair connections that happened to use the same thread:
            initializeRoutability(&(subMapRoutability[wire_config]), &subMapInfo, TRUE);

          }  // End of for-loop for index 'wire_config'

          // For each of the two diff-pair paths in the connection allocate memory for the large arrays
          // used by the path-finding function, findPath(). These arrays get re-initialized within
          // findPath() itself.
          for (int pathIndex = 0; pathIndex < 2; pathIndex++) {
            allocatePathFindingArrays(&subMapPathFinding[pathIndex], &subMapInfo);
          }


          // Except for the two diff-pair paths associated with the current connection, copy
          // the path coordinates to the 'subMapPathCoords[][]' arrays for each of the two
          // wire-configurations. Coordinates beyond the sub-map's boundaries will not be
          // included. The 'subMapPathLengths[]' array will contain only the number of
          // segments within the sub-map's boundaries.
          // printf("\nDEBUG: About to copy path coordinates from main map to sub-map in optimizeDiffPairConnections, excluding path %d and %d...\n",
          //        pathNums[0], pathNums[1]);
          for (int path = 0; path < num_routed_nets; path++)  {
            // printf("DEBUG: ------------------- Copying segments for path #%d. -------------------\n", path);

            //
            // Iterate over both wiring configuration (NOT_SWAPPED and SWAPPED):
            //
            for (int wire_config = NOT_SWAPPED; wire_config <= SWAPPED; wire_config++)  {

              // If this connection starts in a swap-zone, there is no need to check for swapped
              // and non-swapped wiring configurations, since they'll be virtually identical.
              // So we skip the SWAPPED configuration altogether.
              if (startTermsInSwapZone && (wire_config == SWAPPED))  {
                continue;
              }

              // Initialize the path-lengths to zero for each path in each wiring configuration
              subMapPathLengths[wire_config][path] = 0;

              // Check if the path is not equal to either of the diff-pair paths:
              if ((path != pathNums[0]) && (path != pathNums[1]))  {

                // We got here, so 'path' is not one of the two diff-pair paths
                // associated with the current connection. Iterate over the original
                // segments from this path and copy those to the path coordinates that
                // are within the sub-map's boundaries:
                for (int origSeg = 0; origSeg < pathLengths[path]; origSeg++)  {
                  if (   (pathCoords[path][origSeg].X >= shoulderConnections[i].connection[j].minCoord.X)
                      && (pathCoords[path][origSeg].X <= shoulderConnections[i].connection[j].maxCoord.X)
                      && (pathCoords[path][origSeg].Y >= shoulderConnections[i].connection[j].minCoord.Y)
                      && (pathCoords[path][origSeg].Y <= shoulderConnections[i].connection[j].maxCoord.Y)
                      && (pathCoords[path][origSeg].Z >= shoulderConnections[i].connection[j].minCoord.Z)
                      && (pathCoords[path][origSeg].Z <= shoulderConnections[i].connection[j].maxCoord.Z))  {

                    // printf("DEBUG: For path #%d, about to copy original segment #%d at (%d,%d,%d) from main map to segment #%d of sub-map with offsets (%d,%d,%d)...\n",
                    //        path, origSeg, pathCoords[path][origSeg].X, pathCoords[path][origSeg].Y, pathCoords[path][origSeg].Z,
                    //        subMapPathLengths[path], -X_offset, -Y_offset, -Z_offset);

                    // For each wire-configurations, expand the memory allocated for 'subMapPathCoords'
                    // to accommodate another segment from 'pathCoords[][]':
                    subMapPathCoords[wire_config][path]
                       = realloc(subMapPathCoords[wire_config][path],  sizeof(Coordinate_t) * (1 + subMapPathLengths[wire_config][path]) );

                    // Copy the segment to the 'subMapPathCoords' arrays (with the appropriate X/Y/Z-offsets):
                    subMapPathCoords[wire_config][path][subMapPathLengths[wire_config][path]].X
                          = pathCoords[path][origSeg].X  -  X_offset;
                    subMapPathCoords[wire_config][path][subMapPathLengths[wire_config][path]].Y
                          = pathCoords[path][origSeg].Y  -  Y_offset;
                    subMapPathCoords[wire_config][path][subMapPathLengths[wire_config][path]].Z
                          = pathCoords[path][origSeg].Z  -  Z_offset;

                    // printf("DEBUG: For path #%d, copied original segment #%d at (%d,%d,%d) in main map to segment #%d at (%d,%d,%d) in sub-map.\n",
                    //        path, origSeg, pathCoords[path][origSeg].X, pathCoords[path][origSeg].Y, pathCoords[path][origSeg].Z,
                    //        subMapPathLengths[path], subMapPathCoords[path][subMapPathLengths[path]].X,
                    //        subMapPathCoords[path][subMapPathLengths[path]].Y, subMapPathCoords[path][subMapPathLengths[path]].Z);

                    // Increment the corresponding 'subMapPathLengths' element for each wire-configuration:
                    subMapPathLengths[wire_config][path]++;
                  }  // End of if-block for original segment being within the sub-map's boundary
                  else  {
                    // printf("DEBUG:   For path #%d, original segment #%d at (%d,%d,%d) in main map is outside of sub-map's boundaries.\n",
                    //        path, origSeg, pathCoords[path][origSeg].X, pathCoords[path][origSeg].Y, pathCoords[path][origSeg].Z);
                  }
                }  // End of for-loop for index 'origSeg'
              }  // End of if-block for 'path' NOT being one of the two diff-pair paths in the current connection
            }  // End of for-loop for index 'wire_config'
          }  // End of for-loop for index 'path' (0 to max number of paths in main map)

          // Create contiguous paths for the sub-map from the non-contiguous paths created above. It's understood
          // that function createContiguousPaths() will not generate perfectly contiguous paths if the non-contiguous
          // shoulder-paths do not consist entirely of legal jumps. But these moderately contiguous paths should
          // suffice for the purposes of adding congestion.
          for (int wire_config = NOT_SWAPPED; wire_config <= SWAPPED; wire_config++)  {

            // If this connection starts in a swap-zone, there is no need to check for swapped
            // and non-swapped wiring configurations, since they'll be virtually identical.
            // So we skip the SWAPPED configuration altogether.
            if (startTermsInSwapZone && (wire_config == SWAPPED))  {
              continue;
            }

            // printf("\nDEBUG: Before calling createContiguousPaths from optimizeDiffPairConnections, num_routed_nets = %d\n", num_routed_nets);
            // for (int i = 0; i < num_routed_nets; i++)  {
            //   printf("DEBUG:    subMapPathLengths[wire_config = %d][i = %d] = %d\n", wire_config, i, subMapPathLengths[wire_config][i]);
            // }

            createContiguousPaths(num_routed_nets, subMapPathLengths[wire_config], &subMapInfo, subMapPathCoords[wire_config],
                                  subMapContigPathCoords[wire_config], subMapContiguousPathLengths[wire_config], user_inputs,
                                  subMap_cellInfo[wire_config]);
          }  // End of for-loop for index 'wire_config'

          // Gcost holds the G-cost of each path in each wiring configuration:
          //   Gcost[config][pathIndex] = cost of path number 'pathNums[pathIndex]'
          //                              in wiring configuration 'config'
          unsigned long Gcost[2][2] = { 0 };


          // Define Boolean flag that is FALSE until 4 consecutive sub-iterations result in
          // stable routing results:
          unsigned char stable_subIterations = FALSE;

          // Define Boolean flag that is FALSE until 4 consecutive sub iterations result in the
          // same value for 'swap':
          unsigned char stable_swap_result = FALSE;

          // Define Boolean flag that is FALSE until 4 consecutive sub iterations result in
          // essentially the same value for 'symmetryRatio' (within 0.00001):
          unsigned char stable_symmetryRatio = FALSE;

          // Initialize the Boolean 'DRC_free' flags for this connection to TRUE. They will be
          // changed to FALSE only if the sub-map optimization routing results in intra-pair
          // design-rule violations:
          shoulderConnections[i].connection[j].DRC_free[NOT_SWAPPED] = TRUE;
          shoulderConnections[i].connection[j].DRC_free[SWAPPED]     = TRUE;

          // Define array of Boolean values that captures the 'swap' decision after each
          // sub-iteration (up to a maximum of 'subMap_maxIterations':
          unsigned char swap_decisions[subMap_maxIterations + 1] = { 0 };

          // Define array of floating-point values that captures the 'symmetryRatio' value
          // after each sub-iteration (up to a maximum of 'subMap_maxIterations':
          double subIteration_symmetryRatios[subMap_maxIterations + 1] = { 0.0 };

          //
          // Run iterations ('subMap_maxIterations') to find the G-cost of
          // of routing path numbers 'pathNums[0]' and 'pathNums[1]' in the
          // unswapped wiring configuration (0) and swapped wiring config (1):
          //
          subMapInfo.current_iteration = 0;  // Counter for number of path-finding iterations in sub-map
          while ((subMapInfo.current_iteration < subMap_maxIterations) && (stable_subIterations == FALSE))  {

            // Increment the counter for the sub-iteration:
            subMapInfo.current_iteration++;

            // Iterate over both wiring configurations (unswapped and swapped):
            for (int config = NOT_SWAPPED; config <= SWAPPED; config++)  {

              // If this connection starts in a swap-zone, there is no need to check for swapped
              // and non-swapped wiring configurations, since they'll be virtually identical.
              // So we skip the SWAPPED configuration altogether.
              if (startTermsInSwapZone && (config == SWAPPED))  {
                continue;
              }

              #ifdef DEBUG_optimizeDiffPairConnections
              if (DEBUG_ON)  {
                printf("DEBUG: Entered loop for wiring configurations, with config = %d\n", config);
              }
              #endif

              tim = time(NULL);
              now = localtime(&tim);
              printf("INFO: (thread %2d)   Starting sub-iteration #%d for wiring-config %d for connection %d of pseudo-path %d in parent-iteration %d at %02d-%02d-%d, %02d:%02d:%02d\n",
                     thread_num, subMapInfo.current_iteration, config, j, shoulderConnections[i].pseudoPath, mapInfo->current_iteration,
                     now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


              // For any of the 4 connection-terminals in the main map that were at via-sites, add congestion
              // around these terminals. This is especially important if the sub-map is a single layer,
              // but the connection-terminals represent vias whose congestion could potentially extend far
              // from the terminals and influence routing in the sub-map.

              #ifdef DEBUG_addCongestion
              //
              // The following DEBUG code is used for debugging the addition of congestion:
              //
              int DEBUG_FLAG = FALSE;
              if (   (mapInfo->current_iteration == 3) && (subMapInfo.current_iteration == 1)
                  && (shoulderConnections[i].pseudoPath == 31) && (config == 0))  {

                DEBUG_FLAG = TRUE;

                printf("DEBUG: ------------------------------------------------------------------------------------------------\n");
                printf("DEBUG: In optimizeDiffPairConnections during iteration %d, sub-iteration %d, pseudo-path %d, connection %d, config %d,\n",
                       mapInfo->current_iteration, subMapInfo.current_iteration, shoulderConnections[i].pseudoPath, j, config);
                printf("DEBUG: about to call addCongestionAroundTerminal 4 times with following inputs:\n");
                printf("DEBUG:   1. pathNum = %d, centerPoint = (%d,%d,%d), centerShapeType = %d,\n", pathNums[0],
                       startCoord[0].X, startCoord[0].Y, startCoord[0].Z, shoulderConnections[i].connection[j].startShapeType_1);
                printf("DEBUG:      user_inputs = %p, &mapInfo = %p, cellInfo = %p\n", user_inputs, &subMapInfo, subMap_cellInfo[config]);

                printf("DEBUG:   2. pathNum = %d, centerPoint = (%d,%d,%d), centerShapeType = %d,\n", pathNums[1],
                       startCoord[1].X, startCoord[1].Y, startCoord[1].Z, shoulderConnections[i].connection[j].startShapeType_2);
                printf("DEBUG:      user_inputs = %p, &mapInfo = %p, cellInfo = %p\n", user_inputs, &subMapInfo, subMap_cellInfo[config]);

                printf("DEBUG:   3. pathNum = %d, centerPoint = (%d,%d,%d), centerShapeType = %d,\n", pathNums[0],
                       endCoord[config][0].X, endCoord[config][0].Y, endCoord[config][0].Z, shoulderConnections[i].connection[j].endShapeType_1);
                printf("DEBUG:      user_inputs = %p, &mapInfo = %p, cellInfo = %p\n", user_inputs, &subMapInfo, subMap_cellInfo[config]);

                printf("DEBUG:   4. pathNum = %d, centerPoint = (%d,%d,%d), centerShapeType = %d,\n", pathNums[1],
                       endCoord[config][1].X, endCoord[config][1].Y, endCoord[config][1].Z, shoulderConnections[i].connection[j].endShapeType_2);
                printf("DEBUG:      user_inputs = %p, &mapInfo = %p, cellInfo = %p\n", user_inputs, &subMapInfo, subMap_cellInfo[config]);
                printf("DEBUG:\n");
                printf("DEBUG:  mapInfo.mapWidth = %d, mapInfo.mapHeight = %d, mapInfo.numLayers = %d\n",
                       subMapInfo.mapWidth, subMapInfo.mapHeight, subMapInfo.numLayers);
                printf("DEBUG: ------------------------------------------------------------------------------------------------\n");
              }

              if (shoulderConnections[i].connection[j].startShapeType_1 != TRACE)  {
                addCongestionAroundTerminal(pathNums[0], startCoord[0], shoulderConnections[i].connection[j].startShapeType_1,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config], DEBUG_FLAG);
              }
              if (shoulderConnections[i].connection[j].startShapeType_2 != TRACE)  {
                addCongestionAroundTerminal(pathNums[1], startCoord[1], shoulderConnections[i].connection[j].startShapeType_2,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config], DEBUG_FLAG);
              }
              if (shoulderConnections[i].connection[j].endShapeType_1 != TRACE)  {
                addCongestionAroundTerminal(pathNums[0], endCoord[config][0], shoulderConnections[i].connection[j].endShapeType_1,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config], DEBUG_FLAG);
              }
              if (shoulderConnections[i].connection[j].endShapeType_2 != TRACE)  {
                addCongestionAroundTerminal(pathNums[1], endCoord[config][1], shoulderConnections[i].connection[j].endShapeType_2,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config], DEBUG_FLAG);
              }
              // End of DEBUG code

              #else
              if (shoulderConnections[i].connection[j].startShapeType_1 != TRACE)  {
                addCongestionAroundTerminal(pathNums[0], startCoord[0], shoulderConnections[i].connection[j].startShapeType_1,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config]);
              }
              if (shoulderConnections[i].connection[j].startShapeType_2 != TRACE)  {
                addCongestionAroundTerminal(pathNums[1], startCoord[1], shoulderConnections[i].connection[j].startShapeType_2,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config]);
              }
              if (shoulderConnections[i].connection[j].endShapeType_1 != TRACE)  {
                addCongestionAroundTerminal(pathNums[0], endCoord[config][0], shoulderConnections[i].connection[j].endShapeType_1,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config]);
              }
              if (shoulderConnections[i].connection[j].endShapeType_2 != TRACE)  {
                addCongestionAroundTerminal(pathNums[1], endCoord[config][1], shoulderConnections[i].connection[j].endShapeType_2,
                                            user_inputs, &subMapInfo, subMap_cellInfo[config]);
              }

              #endif

              #ifdef DEBUG_addCongestion
              //
              // The following DEBUG code was temporarily added 11/27/2023:
              //
              if (   (mapInfo->current_iteration == 3) && (subMapInfo.current_iteration == 1)
                  && (shoulderConnections[i].pseudoPath == 31) && (j == 1) && (config == 0))  {
                printf("\nDEBUG: Exiting due to meeting criteria in DEBUG statement.\n\n");
                exit(1);
              }
              //
              // End of DEBUG code
              //
              #endif


              // Evaporate 10% of the congestion from the sub-map for only the two diff-pair paths.
              evaporateDiffPairCongestion(&subMapInfo, subMap_cellInfo[config], 10.0, pathNums[0], pathNums[1]);


              // Define 2-element array of Boolean variables to indicate whether a path was successfully found
              // for each of the diff-pair nets:
              int parallel_pathFound[2] = { FALSE };

              //
              // Iterate over both nets in the diff-pair:
              //
              // Note that the following '#pragma omp' statement might cause more delay than
              // acceleration, given the small loop-size.
              #ifdef DEBUG_optimizeDiffPairConnections
              #pragma omp parallel for if (! DEBUG_ON)
              #else
              #pragma omp parallel for
              #endif
              for (int pathIndex = 0; pathIndex <= 1; pathIndex++)  {

                #ifdef DEBUG_optimizeDiffPairConnections
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)    Routing path %d in connection %d of pseudo-path %d in sub-iteration %d of parent-iteration %d.\n",
                         omp_get_thread_num(), pathNums[pathIndex], j, shoulderConnections[i].pseudoPath,
                         subMapInfo.current_iteration, mapInfo->current_iteration);
                }
                #endif

                // For this connection, we need to invoke the compute-intensive path-finding function. If we have not yet
                // had to expand the sub-map's size to find a connection between terminals (mapSizeMultiplier is 1), then
                // we  populate the 'routingRestrictions' structure to restrict routing to a small region around the
                // connection. This is the same restriction that will be used in function fillGapsInDiffPairPaths().
                //
                // If we've had to increase the sub-map size because findPath() could not find a connection between
                // start- and end-terminals (i.e., mapSizeMultiplier >= 2), then we disable all routing restrictions.
                RoutingRestriction_t connectionRouteRestrictions;
                if (mapSizeMultiplier == 1)  {
                  connectionRouteRestrictions.restrictionFlag = TRUE;
                  for (int layer = 0; layer < maxRoutingLayers; layer++)  {
                    connectionRouteRestrictions.allowedLayers[layer] = FALSE;
                    connectionRouteRestrictions.allowedRadiiMicrons[layer] = 0.0;
                    connectionRouteRestrictions.allowedRadiiCells[layer] = 0.0;
                  }
                }
                else  {
                  connectionRouteRestrictions.restrictionFlag = FALSE;
                }

                // Calculate the routing restrictions for this connection:
                Coordinate_t start;
                Coordinate_t end;

                if ((config == NOT_SWAPPED) && (pathIndex == 0))  {
                  start.X =  shoulderConnections[i].connection[j].startCoord_1.X;
                  start.Y =  shoulderConnections[i].connection[j].startCoord_1.Y;
                  start.Z =  shoulderConnections[i].connection[j].startCoord_1.Z;
                  end.X   =  shoulderConnections[i].connection[j].endCoord_1.X;
                  end.Y   =  shoulderConnections[i].connection[j].endCoord_1.Y;
                  end.Z   =  shoulderConnections[i].connection[j].endCoord_1.Z;
                }
                else if ((config == NOT_SWAPPED) && (pathIndex == 1))  {
                  start.X =  shoulderConnections[i].connection[j].startCoord_2.X;
                  start.Y =  shoulderConnections[i].connection[j].startCoord_2.Y;
                  start.Z =  shoulderConnections[i].connection[j].startCoord_2.Z;
                  end.X   =  shoulderConnections[i].connection[j].endCoord_2.X;
                  end.Y   =  shoulderConnections[i].connection[j].endCoord_2.Y;
                  end.Z   =  shoulderConnections[i].connection[j].endCoord_2.Z;
                }
                else if ((config == SWAPPED) && (pathIndex == 0))  {
                  start.X =  shoulderConnections[i].connection[j].startCoord_1.X;
                  start.Y =  shoulderConnections[i].connection[j].startCoord_1.Y;
                  start.Z =  shoulderConnections[i].connection[j].startCoord_1.Z;
                  end.X   =  shoulderConnections[i].connection[j].endCoord_2.X;
                  end.Y   =  shoulderConnections[i].connection[j].endCoord_2.Y;
                  end.Z   =  shoulderConnections[i].connection[j].endCoord_2.Z;
                }
                else if ((config == SWAPPED) && (pathIndex == 1))  {
                  start.X =  shoulderConnections[i].connection[j].startCoord_2.X;
                  start.Y =  shoulderConnections[i].connection[j].startCoord_2.Y;
                  start.Z =  shoulderConnections[i].connection[j].startCoord_2.Z;
                  end.X   =  shoulderConnections[i].connection[j].endCoord_1.X;
                  end.Y   =  shoulderConnections[i].connection[j].endCoord_1.Y;
                  end.Z   =  shoulderConnections[i].connection[j].endCoord_1.Z;
                }

                // If we have not yet had to expand the sub-map's size to find a connection between terminals
                // (mapSizeMultiplier is 1), then call function calcGapRoutingRestrictions() to calculate the
                // allowed radii to restrict the routing when we call findPath() below. 'calcGapRoutingRestrictions'
                // needs access to the main map. So give it the main map, and then we'll translate
                // the resulting routing restrictions to the sub-map:
                if (mapSizeMultiplier == 1)  {
                  calcGapRoutingRestrictions(&connectionRouteRestrictions, start, end, pathNums[pathIndex],
                                             pathCoords, pathLengths,
                                             FALSE, 0, 0, cellInfo, mapInfo, user_inputs);

                  // Modify the output of the above calcGapRoutingRestrictions() function so we
                  // can use it for the sub-map, which likely has fewer layers than the main map:
                  connectionRouteRestrictions.centerX = connectionRouteRestrictions.centerX - X_offset;
                  connectionRouteRestrictions.centerY = connectionRouteRestrictions.centerY - Y_offset;

                  for (int subMapLayer = 0; subMapLayer < maxRoutingLayers; subMapLayer++)  {
                    if (subMapLayer < subMapInfo.numLayers)  {
                      #ifdef DEBUG_optimizeDiffPairConnections
                      if (DEBUG_ON)  {
                        printf("DEBUG: Mapping routing restrictions from main map layer %d to sub-map layer %d\n", subMapLayer + Z_offset, subMapLayer);
                      }
                      #endif
                      connectionRouteRestrictions.allowedLayers[subMapLayer]       = connectionRouteRestrictions.allowedLayers[subMapLayer + Z_offset];
                      connectionRouteRestrictions.allowedRadiiCells[subMapLayer]   = connectionRouteRestrictions.allowedRadiiCells[subMapLayer + Z_offset];
                      connectionRouteRestrictions.allowedRadiiMicrons[subMapLayer] = connectionRouteRestrictions.allowedRadiiMicrons[subMapLayer + Z_offset];
                    }
                    else  {
                      #ifdef DEBUG_optimizeDiffPairConnections
                      if (DEBUG_ON)  {
                        printf("DEBUG: Zero'ing out the routing restrictions for sub-map layer %d\n", subMapLayer);
                      }
                      #endif
                      connectionRouteRestrictions.allowedLayers[subMapLayer] = FALSE;
                      connectionRouteRestrictions.allowedRadiiCells[subMapLayer]   = 0.0;
                      connectionRouteRestrictions.allowedRadiiMicrons[subMapLayer] = 0.0;
                    }
                  }  // End of for-loop with index 'subMapLayer'

                  // Scale the routing restriction radii by the factor 'mapSizeMultiplier', which starts at 1 but
                  // increases if findPath() fails to find a solution:
                  for (int subMapLayer = 0; subMapLayer < subMapInfo.numLayers; subMapLayer++)  {
                    connectionRouteRestrictions.allowedRadiiMicrons[subMapLayer] *= mapSizeMultiplier;
                    connectionRouteRestrictions.allowedRadiiCells[subMapLayer]   *= mapSizeMultiplier;

                    // Check if the allowed radius has significantly exceeded the size of the sub-map after
                    // increasing the 'mapSizeMultiplier'. This is not expected, and triggers a fatal error:
                    if (   (mapSizeMultiplier > 1)
                        && (connectionRouteRestrictions.allowedRadiiCells[subMapLayer] > 2 * subMapInfo.mapDiagonal))  {
                      printf("\n\nERROR: The allowed routing radius on sub-map layer #%d (%6.3f cells) exceeded twice the diagonal size of the sub-map (%d x %d cells)\n",
                             subMapLayer, connectionRouteRestrictions.allowedRadiiCells[subMapLayer], subMapInfo.mapWidth, subMapInfo.mapHeight);
                      printf(    "       in function 'optimizeDiffPairConnections' for path #%d ('%s') between points (%d,%d,%d) cells and (%d,%d,%d) cells\n",
                              pathNums[pathIndex], user_inputs->net_name[pathNums[pathIndex]], start.X, start.Y, start.Z,
                              end.X, end.Y, end.Z);
                      printf(    "       on attempt #%d. Inform the software developer of this fatal error message.\n\n", mapSizeMultiplier);
                      exit(1);
                    }  // End of if-block for fatal error

                  }  // End of for-loop for index 'subMapLayer'

                  #ifdef DEBUG_optimizeDiffPairConnections
                  if (DEBUG_ON)  {
                    printf("\nDEBUG: About to call findPath for path %d in sub-iteration %d of iteration %d for connection %d with routing restrictions:\n",
                           pathNums[pathIndex], subMapInfo.current_iteration, mapInfo->current_iteration, j);
                    printf("DEBUG:   centerX = %d in sub-map\n", connectionRouteRestrictions.centerX);
                    printf("DEBUG:   centerY = %d in sub-map\n", connectionRouteRestrictions.centerY);
                    for (int layer = 0; layer < maxRoutingLayers; layer++)  {
                      printf("DEBUG:    Sub-map layer %d:       allowedLayers = %d\n", layer, connectionRouteRestrictions.allowedLayers[layer]);
                      printf("DEBUG:    Sub-map layer %d:   allowedRadiiCells = %.3f\n", layer, connectionRouteRestrictions.allowedRadiiCells[layer]);
                      printf("DEBUG:    Sub-map layer %d: allowedRadiiMicrons = %.3f\n", layer, connectionRouteRestrictions.allowedRadiiMicrons[layer]);
                    }
                    printf("DEBUG:\n");
                  }
                  #endif
                }  // End of if-block for mapSizeMultiplier == 1
                else  {
                  // We got here, so mapSizeMultiplier > 1, i.e., findPath() was not able to route
                  // from the start- to the end-terminal on the previous attempt. So we now
                  // disable routing restrictions:
                  connectionRouteRestrictions.restrictionFlag = FALSE;
                }  // End of else-block for mapSizeMultiplier != 1


                // Call findPath() to route path 'pathNums[pathIndex]' in an unswapped wiring configuration (config = 0)
                // or swapped wiring configuration (config = 1):
                Gcost[config][pathIndex] = findPath(&subMapInfo, subMap_cellInfo[config], pathNums[pathIndex], startCoord[pathIndex],
                                                    endCoord[config][pathIndex], &(subMapPathCoords[config][pathNums[pathIndex]]),
                                                    &subMapPathLengths[config][pathNums[pathIndex]], user_inputs,
                                                    &(subMapRoutability[config]), &subMapPathFinding[pathIndex],
                                                    0, FALSE, FALSE, &connectionRouteRestrictions, TRUE, FALSE);


                #ifdef DEBUG_optimizeDiffPairConnections
                if (DEBUG_ON)  {
                  printf("\nDEBUG: (thread %2d)   In iteration %d, sub-iteration %d, pseudo-path %d, connection %d, G-cost for path %d in config %d was %'lu between (%d,%d,%d) and (%d,%d,%d), with %d segments.\n\n",
                         thread_num, mapInfo->current_iteration, subMapInfo.current_iteration, shoulderConnections[i].pseudoPath, j,
                         pathNums[pathIndex], config, Gcost[config][pathIndex],
                         startCoord[pathIndex].X, startCoord[pathIndex].Y, startCoord[pathIndex].Z,
                         endCoord[config][pathIndex].X, endCoord[config][pathIndex].Y, endCoord[config][pathIndex].Z,
                         subMapPathLengths[config][pathNums[pathIndex]]);

                  printf("DEBUG: (thread %2d)      Coordinates:", thread_num);
                  for (int segment_num = 0; segment_num < subMapPathLengths[config][pathNums[pathIndex]]; segment_num++)  {
                    printf("  %d:(%d,%d,%d)", segment_num, subMapPathCoords[config][pathNums[pathIndex]][segment_num].X,
                                                           subMapPathCoords[config][pathNums[pathIndex]][segment_num].Y,
                                                           subMapPathCoords[config][pathNums[pathIndex]][segment_num].Z);
                  }
                  printf("\nDEBUG: (thread %2d)\n", thread_num);

                  if (subMapInfo.current_iteration == subMap_maxIterations)  {
                    printf("\nDEBUG: (thread %2d) Path %d coordinates for config %d after sub-iteration %d:\n", omp_get_thread_num(),
                           pathNums[pathIndex], config, subMapInfo.current_iteration);
                    for (int ii = 0; ii < subMapPathLengths[config][pathNums[pathIndex]]; ii++)  {
                      printf("DEBUG: (thread %2d)   Segment %d: (%d,%d,%d)\n", omp_get_thread_num(), ii, subMapPathCoords[config][pathNums[pathIndex]][ii].X,
                             subMapPathCoords[config][pathNums[pathIndex]][ii].Y, subMapPathCoords[config][pathNums[pathIndex]][ii].Z);
                    }
                    printf("DEBUG: (thread %2d) ----------------------------------------------\n", omp_get_thread_num());
                  }
                }
                #endif

                // If a path was successfully found, set the 'parallel_pathFound' flag to TRUE. But if no path was found,
                // set this flag to FALSE, issue a warning message, and break out of the current loop so that the
                // size of the sub-map can be enlarged before re-trying findPath():
                if (Gcost[config][pathIndex])  {
                  parallel_pathFound[pathIndex] = TRUE;

                  // For the current diff-pair path, create a contiguous version of the path from the most recent iteration:
                  createOneContiguousPath(pathNums[pathIndex], startCoord[pathIndex], &subMapInfo, subMapPathLengths[config][pathNums[pathIndex]],
                                          subMapPathCoords[config][pathNums[pathIndex]], &(subMapContigPathCoords[config][pathNums[pathIndex]]),
                                          &(subMapContiguousPathLengths[config][pathNums[pathIndex]]), user_inputs, subMap_cellInfo[config]);

                  #ifdef DEBUG_optimizeDiffPairConnections
                  if (DEBUG_ON)  {
                    printf("DEBUG: (thread %2d) findPath returned a G-cost of %'lu for path %d during iteration %d, sub-iteration %d\n",
                           omp_get_thread_num(), Gcost[config][pathIndex], pathNums[pathIndex], mapInfo->current_iteration, subMapInfo.current_iteration);
                    printf("DEBUG: (thread %2d) pseudo-path #%d, connection #%d, wire-configuration #%d between the following sub-map\n",
                           omp_get_thread_num(), shoulderConnections[i].pseudoPath, j, config);
                    printf("DEBUG: (thread %2d) coordinates:  (%d,%d,%d)  and  (%d,%d,%d)\n", omp_get_thread_num(),
                           startCoord[pathIndex].X,       startCoord[pathIndex].Y,       startCoord[pathIndex].Z,
                           endCoord[config][pathIndex].X, endCoord[config][pathIndex].Y, endCoord[config][pathIndex].Z);
                  }
                  #endif
                }  // End of if-block for Gcost > 0
                else {
                  // We got here, so findPath() failed to find a path between the two terminals.
                  printf("\nWARNING: (thread %2d) No path was found for path %d ('%s') during iteration %d, sub-iteration %d,\n",
                         omp_get_thread_num(), pathNums[pathIndex], user_inputs->net_name[pathNums[pathIndex]],
                         mapInfo->current_iteration, subMapInfo.current_iteration);
                  printf(  "         (thread %2d) pseudo-path #%d, connection #%d, wire-configuration #%d. The path cost = %lu\n",
                         omp_get_thread_num(), shoulderConnections[i].pseudoPath, j, config, Gcost[config][pathIndex]);
                  printf(  "         (thread %2d) between sub-map coordinates (%d,%d,%d) and (%d,%d,%d). These correspond to \n",
                         omp_get_thread_num(), startCoord[pathIndex].X, startCoord[pathIndex].Y, startCoord[pathIndex].Z,
                         endCoord[config][pathIndex].X, endCoord[config][pathIndex].Y, endCoord[config][pathIndex].Z);
                  printf(  "         (thread %2d) coordinates (%d,%d,%d) and (%d,%d,%d) in the main map. The size of the sub-map\n",
                         omp_get_thread_num(), startCoord[pathIndex].X + X_offset, startCoord[pathIndex].Y + Y_offset, startCoord[pathIndex].Z + Z_offset,
                         endCoord[config][pathIndex].X + X_offset, endCoord[config][pathIndex].Y + Y_offset, endCoord[config][pathIndex].Z + Z_offset);
                  printf(  "         (thread %2d) will be enlarged, and routing restrictions will be eliminated, before\n", omp_get_thread_num());
                  printf(  "         (thread %2d) re-trying to find a path.\n\n", omp_get_thread_num());

                  // Reset parallel_pathFound to FALSE so that the subsequent non-parallized code below can
                  // detect the failed path-finding
                  parallel_pathFound[pathIndex] = FALSE;

                }  // End of else-block for Gcost == 0

              }  // End of for-loop for index 'pathIndex' (0 to 1)
              //
              // The above line is the end of parallel processing to run findPath() on both
              // diff-pair nets in parallel.
              //

              // Confirm that paths were successfully found for both diff-pair nets. If not, then break out of
              // the current for-loop so that the sub-map size can be enlarged before re-trying findPath():
              if (parallel_pathFound[0] && parallel_pathFound[1])  {
                pathFound = TRUE;
              }
              else  {
                pathFound = FALSE;
                break;  // Break out of for-loop for index 'config'
              }

              // Re-initialize the cellInfo:
              reInitializeCellInfo(&subMapInfo, subMap_cellInfo[config]);

              // Now that both diff-pair paths have been routed through the sub-map, add
              // path-center information in each sub-map at the 4 connection-terminals so that
              // intra-pair design-rule violations can be detected. This is only needed if the
              // connection-terminals were at vias in the main map:
              //
              // Connection start-terminal for first diff-pair path:
              if (shoulderConnections[i].connection[j].startShapeType_1 != TRACE)  {

                add_path_center_info(&(subMap_cellInfo[config][startCoord[0].X][startCoord[0].Y][startCoord[0].Z]),
                                     pathNums[0], shoulderConnections[i].connection[j].startShapeType_1);

              }  // End of if-block for startShapeType_1 != TRACE

              // Connection start-terminal for second diff-pair path:
              if (shoulderConnections[i].connection[j].startShapeType_2 != TRACE)  {

                add_path_center_info(&(subMap_cellInfo[config][startCoord[1].X][startCoord[1].Y][startCoord[1].Z]),
                                     pathNums[1], shoulderConnections[i].connection[j].startShapeType_2);

              }  // End of if-block for startShapeType_2 != TRACE

              // Connection end-terminal for first diff-pair path:
              if (shoulderConnections[i].connection[j].endShapeType_1 != TRACE)  {

                add_path_center_info(&(subMap_cellInfo[config][endCoord[config][0].X][endCoord[config][0].Y][endCoord[config][0].Z]),
                                     pathNums[0], shoulderConnections[i].connection[j].endShapeType_1);

              }  // End of if-block for endShapeType_1 != TRACE

              // Connection end-terminal for second diff-pair path:
              if (shoulderConnections[i].connection[j].endShapeType_2 != TRACE)  {

                add_path_center_info(&(subMap_cellInfo[config][endCoord[config][1].X][endCoord[config][1].Y][endCoord[config][1].Z]),
                                     pathNums[1], shoulderConnections[i].connection[j].endShapeType_1);

              }  // End of if-block for endShapeType_2 != TRACE

              //
              // Add path-center information to the sub-map all along the diff-pair paths that are routed
              // in the main map. This allows calcRoutabilityMetrics() to detect design-rule violations
              // between the newly routed diff-pair paths in the sub-map and those that were routed in
              // the main map.
              //
              // Add path-center info to cells from path 1's end-segment to the end of this path:
              addDiffPairPathCentersToSubMap(pathNums[0], shoulderConnections[i].connection[j].endSegment_1, pathLengths[pathNums[0]],
                                           subMap_cellInfo[config], shoulderConnections[i].connection[j], pathCoords, mapInfo);
              //
              // Add path-center info to cells from path 1's start-segment to the beginning of this path:
              addDiffPairPathCentersToSubMap(pathNums[0], shoulderConnections[i].connection[j].startSegment_1, -1,
                                           subMap_cellInfo[config], shoulderConnections[i].connection[j], pathCoords, mapInfo);
              //
              // Add path-center info to cells from path 2's end-segment to the end of this path:
              addDiffPairPathCentersToSubMap(pathNums[1], shoulderConnections[i].connection[j].endSegment_2, pathLengths[pathNums[1]],
                                           subMap_cellInfo[config], shoulderConnections[i].connection[j], pathCoords, mapInfo);
              //
              // Add path-center info to cells from path 2's start-segment to the beginning of this path:
              addDiffPairPathCentersToSubMap(pathNums[1], shoulderConnections[i].connection[j].startSegment_2, -1,
                                           subMap_cellInfo[config], shoulderConnections[i].connection[j], pathCoords, mapInfo);


              //
              // Now that both diff-pair paths have been routed through the sub-map, run 'calcRoutabilityMetrics'
              // to assess the routing before the next sub-iteration. We deposit congestion only for the diff-pair
              // paths associated with the pseudo-path for the current connection.
              //
              unsigned char doNotPrint_DRCs = TRUE;
              #ifdef DEBUG_optimizeDiffPairConnections
              if (DEBUG_ON)  {
                doNotPrint_DRCs = FALSE;
              }
              #endif
              calcRoutabilityMetrics(&subMapInfo, subMapPathLengths[config], subMapPathCoords[config],
                                     subMapContiguousPathLengths[config], subMapContigPathCoords[config], &(subMapRoutability[config]),
                                     user_inputs, subMap_cellInfo[config], TRUE, shoulderConnections[i].pseudoPath, FALSE,
                                     doNotPrint_DRCs, TRUE);


              // For this most recent sub-iteration, record whether there were any intra-pair design-rule violations:
              if (subMapRoutability[config].crossing_matrix[pathNums[0]][pathNums[1]])  {

                shoulderConnections[i].connection[j].DRC_free[config] = FALSE;
              }
              else  {
                shoulderConnections[i].connection[j].DRC_free[config] = TRUE;
              }


              #ifdef DEBUG_optimizeDiffPairConnections
              if (DEBUG_ON)  {
                printf("\nDEBUG: (thread %2d) In optimizeDiffPairConnections after running findPath %d times in iteration %d, pseudo-path %d, connection %d, config %d:\n",
                       omp_get_thread_num(), subMapInfo.current_iteration, mapInfo->current_iteration, shoulderConnections[i].pseudoPath, j, config);
                printf(  "DEBUG: (thread %2d)   Gcost[%d][0] = %'lu\n",   omp_get_thread_num(), config, Gcost[config][0]);
                printf(  "DEBUG: (thread %2d)   Gcost[%d][1] = %'lu\n", omp_get_thread_num(), config, Gcost[config][1]);
                printf(  "DEBUG: (thread %2d)   crossing_matrix[%d][%d] = %d DRC-cells for config %d.\n\n", omp_get_thread_num(),
                       pathNums[0], pathNums[1], subMapRoutability[config].crossing_matrix[pathNums[0]][pathNums[1]], config );
              }
              #endif

            }  // End of for-loop for index 'config' (NOT_SWAPPED to SWAPPED)

            // If no path was found, then break out of the current while-loop so that the sub-map size
            // can be enlarged before re-trying findPath():
            if (! pathFound)  {
              break;  // Break out of while-loop (for current_iteration < subMap_maxIterations) && (stable_subIterations == FALSE)
            }

            // If this connection starts in a swap-zone, then assign the value of 'swap' to FALSE and
            // assign the value of 'symmetryRatio' to a value less than 0.5 (arbitrarily chosen as 0.1):
            if (startTermsInSwapZone)  {
              shoulderConnections[i].connection[j].swap = FALSE;
              shoulderConnections[i].connection[j].symmetryRatio = 0.1;
            }
            else  {
              // If paths were found, then decide whether the connection should be swapped by determining which
              // wiring configuration has the lower G-cost. The function calculates the floating-point value of
              // connection->symmetryRatio and also modifies the Boolean element connection->swap.
              //
              decideWhetherToSwapConnection(Gcost, &shoulderConnections[i].connection[j],
                                            pathNums[0], pathNums[1], mapInfo, cellInfo, user_inputs);
            }

            // Save the 'symmetryRatio' and 'swap' values to arrays so that we can later check if last 4 iterations
            // had the same swap value and approximately the same symmetryRatio value.
            subIteration_symmetryRatios[subMapInfo.current_iteration] = shoulderConnections[i].connection[j].symmetryRatio;
            swap_decisions[subMapInfo.current_iteration] = shoulderConnections[i].connection[j].swap;

            #ifdef DEBUG_optimizeDiffPairConnections
            if (DEBUG_ON)  {
              printf("\nDEBUG: (thread %2d) swap = %d for sub-iteration %d. All swap-values:\n", omp_get_thread_num(),
                     swap_decisions[subMapInfo.current_iteration], subMapInfo.current_iteration);
              printf(  "DEBUG: (thread %2d)       ", omp_get_thread_num());
              for (int iter = 1; iter <= subMapInfo.current_iteration; iter++)  {
                printf("%d: %d    ", iter, swap_decisions[iter]);
              }
              printf("\n\n");

              printf("\nDEBUG: (thread %2d) symmetryRatio = %.4f for sub-iteration %d. All symmetryRatio-values:\n", omp_get_thread_num(),
                     subIteration_symmetryRatios[subMapInfo.current_iteration], subMapInfo.current_iteration);
              printf(  "DEBUG: (thread %2d)       ", omp_get_thread_num());
              for (int iter = 1; iter <= subMapInfo.current_iteration; iter++)  {
                printf("%d: %.4f    ", iter, subIteration_symmetryRatios[iter]);
              }
              printf("\n\n");
            }
            #endif

            //
            // Set the stable_subIterations Boolean flag to TRUE so we can exit the while-loop and not do
            // any more sub-iterations if both of the following conditions are satisfied:
            //   a. The 'swap' value has been consistent for the last N number of sub-iterations, where
            //      N = numIterationsWithStableSwapValue, and
            //   b. The latest symmetryRatio is far from 0.500 (<0.495 or >0.505), or the symmetryRatio
            //      has been nearly constant (within 0.0001) for the last N sub-iterations, where N is
            //      numIterationsWithStableSymmetryRatio (defined as 5).
            //   c. There were no DRC's between the two diff-pair nets in the most recent sub-iteration
            //
            // First, check for criterion (a):
            if (subMapInfo.current_iteration >= numIterationsWithStableSwapValue)  {
              stable_swap_result = TRUE;
              for (int iter = 1 + subMapInfo.current_iteration - numIterationsWithStableSwapValue; iter < subMapInfo.current_iteration; iter++)  {
                if (swap_decisions[iter] != swap_decisions[subMapInfo.current_iteration])  {
                  stable_swap_result = FALSE;
                  break;
                }  // End of if-block for detecting a different swap value
              }  // End of for-loop for index 'iter'
            }  // End of if-block for current_iteration >= numIterationsWithStableSwapValue
            //
            // Next, check for criterion (b) in the above list:
            if (stable_swap_result)  {
              if (   (shoulderConnections[i].connection[j].symmetryRatio >= symmetryRatio_low_threshold )
                  && (shoulderConnections[i].connection[j].symmetryRatio <= symmetryRatio_high_threshold))  {

                // We got here, so the symmetry ratio is very close to 0.500.  We next check whether the
                // previous few sub-iterations' symmetryRatios were within 0.0001 of the current
                // sub-iteration's symmetryRatio:
                stable_symmetryRatio = TRUE;
                for (int iter = 1 + subMapInfo.current_iteration - numIterationsWithStableSymmetryRatio; iter < subMapInfo.current_iteration; iter++)  {
                  if (fabs(subIteration_symmetryRatios[iter] - subIteration_symmetryRatios[subMapInfo.current_iteration]) > symmetryRatio_stability_tolerance)  {
                    // We got here, so one of the recent sub-iterations had a symmetryRatio that differed
                    // significantly from the current sub-iteration's symmetryRatio. So set the Boolean
                    // 'stable_symmetryRatio' variable to FALSE and exit this while-loop.
                    stable_symmetryRatio = FALSE;

                    #ifdef DEBUG_optimizeDiffPairConnections
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d)   symmetryRatio is too close to 0.5 (%.3f) after sub-iteration %d of iteration %d, so more sub-iterations will be run for connection %d.\n",
                             omp_get_thread_num(), shoulderConnections[i].connection[j].symmetryRatio, subMapInfo.current_iteration, mapInfo->current_iteration, j);
                    }
                    #endif

                    break;  // Break out of while-loop (subMapInfo.current_iteration < subMap_maxIterations) && (stable_subIterations == FALSE)
                  }  // End of if-block for detecting a significantly different symmetryRatio value
                }  // End of for-loop for index 'iter'
              }  // End of if-block for symmetryRatio being close to 0.5
              else  {
                #ifdef DEBUG_optimizeDiffPairConnections
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)   symmetryRatio is NOT close to 0.5 (%.3f) after sub-iteration %d of iteration %d, so no more sub-iterations will be run for connection %d.\n",
                         omp_get_thread_num(), shoulderConnections[i].connection[j].symmetryRatio, subMapInfo.current_iteration, mapInfo->current_iteration, j);
                }
                #endif
              }  // End of else-block (symmetryRatio is not close to 0.5)
            }  // End of if-block for stable_swap_result == TRUE

            // Determine whether the sub-iterations' results have stabilized based on the Boolean value of
            // 'stable_swap_results', the most recent symmetryRatio value, (if necessary) the Boolean
            // value of 'stable_symmetryRatio', and the DRC results from the most recent sub-iteration:
            if (   (stable_swap_result)
                && (   (shoulderConnections[i].connection[j].symmetryRatio < 0.495)
                    || (shoulderConnections[i].connection[j].symmetryRatio > 0.505)
                    || (stable_symmetryRatio))
                && (shoulderConnections[i].connection[j].DRC_free[shoulderConnections[i].connection[j].swap]))  {

              // We got here, thereby satisfying the conditions to conclude that the sub-iterations'
              // routing results are stable (so no more sub-iterations are necessary):
              stable_subIterations = TRUE;

              #ifdef DEBUG_optimizeDiffPairConnections
              if (DEBUG_ON)  {
                printf("\nDEBUG: (thread %2d) In iteration %d, sub-iteration %d, pseudo-path %d, connection %d, stable_subIterations set to TRUE because:\n",
                       omp_get_thread_num(), mapInfo->current_iteration, subMapInfo.current_iteration, shoulderConnections[i].pseudoPath, j);
              }
              #endif
            }  // End of if-block
            else  {
              #ifdef DEBUG_optimizeDiffPairConnections
              if (DEBUG_ON)  {
                printf("\nDEBUG: (thread %2d) In iteration %d, sub-iteration %d, pseudo-path %d, connection %d, stable_subIterations remains FALSE because:\n",
                       omp_get_thread_num(), mapInfo->current_iteration, subMapInfo.current_iteration, shoulderConnections[i].pseudoPath, j);
              }
              #endif
            }  // End of else-block

            #ifdef DEBUG_optimizeDiffPairConnections
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d)       stable_swap_result = %d\n", omp_get_thread_num(), stable_swap_result);
              printf("DEBUG: (thread %2d)            symmetryRatio = %.5f\n", omp_get_thread_num(), shoulderConnections[i].connection[j].symmetryRatio);
              printf("DEBUG: (thread %2d)     stable_symmetryRatio = %d\n\n", omp_get_thread_num(), stable_symmetryRatio);
              printf("DEBUG: (thread %2d)    crossing_matrix[%d][%d] = %d\n\n", omp_get_thread_num(), pathNums[0], pathNums[1],
                     subMapRoutability[shoulderConnections[i].connection[j].swap].crossing_matrix[pathNums[0]][pathNums[1]]);
            }
            #endif

            //
            // DEBUG code follows:
            //
            // if ((mapInfo->current_iteration == 1) && (shoulderConnections[i].pseudoPath == 8) && (j == 1) && (config == 1))  {
            //   printf("DEBUG: (thread %2d) Iteration %d, sub-iteration %d, pseudo-path %d, connection %d, config %d:\n",
            //          omp_get_thread_num(), mapInfo->current_iteration, subMapInfo.current_iteration, shoulderConnections[i].pseudoPath, j, config);
            //   printf("DEBUG: (thread %2d)     subMap_cellInfo[14][24][0].numTraversingPaths = %d after running calcRoutabilityMetrics\n", omp_get_thread_num(),
            //          subMap_cellInfo[14][24][0].numTraversingPaths);
            //   if (subMap_cellInfo[14][24][0].numTraversingPaths >= 4)  {
            //
            //     printf("DEBUG: (thread %2d)     subMap_cellInfo[14][24][0].congestion[3].pathTraversalsTimes100 = %d\n", omp_get_thread_num(),
            //            subMap_cellInfo[14][24][0].congestion[3].pathTraversalsTimes100);
            //   }
            // }

          }  // End of while-loop for (current_iteration <= subMap_maxIterations) && (stable_subIterations == FALSE)

          // We got here, so we now have four routes that have been optimized for this diff-pair connection:
          // two routes for an unswapped configuration, and two for a swapped configuration. Save all four routes
          // in the current connection's ShoulderConnection_t variable while translating the coordinates back to
          // the main map:
          //
          for (int wire_config = NOT_SWAPPED; wire_config <= SWAPPED; wire_config++)  {

            // If this connection starts in a swap-zone, there is no need to check for swapped
            // and non-swapped wiring configurations, since they'll be virtually identical.
            // So we skip the SWAPPED configuration altogether.
            if (startTermsInSwapZone && (wire_config == SWAPPED))  {
              continue;
            }

            // Path #1 of 2:
            shoulderConnections[i].connection[j].optimizedConnectionLength_1[wire_config] = subMapPathLengths[wire_config][pathNums[0]];
            shoulderConnections[i].connection[j].optimizedConnectionCoords_1[wire_config] = malloc(subMapPathLengths[wire_config][pathNums[0]] * sizeof(Coordinate_t));
            if (shoulderConnections[i].connection[j].optimizedConnectionCoords_1[wire_config] == 0)  {
              printf("\nERROR: Failed to allocate memory for optimized diff-pair connection in function 'optimizeDiffPairConnections'\n");
              printf(  "       Please inform software developer of this fatal error message.\n\n");
              exit(1);
            }
            for (int segment = 0; segment < subMapPathLengths[wire_config][pathNums[0]]; segment++)  {
              shoulderConnections[i].connection[j].optimizedConnectionCoords_1[wire_config][segment].X = subMapPathCoords[wire_config][pathNums[0]][segment].X + X_offset;
              shoulderConnections[i].connection[j].optimizedConnectionCoords_1[wire_config][segment].Y = subMapPathCoords[wire_config][pathNums[0]][segment].Y + Y_offset;
              shoulderConnections[i].connection[j].optimizedConnectionCoords_1[wire_config][segment].Z = subMapPathCoords[wire_config][pathNums[0]][segment].Z + Z_offset;
            }

            // Path #2 of 2:
            shoulderConnections[i].connection[j].optimizedConnectionLength_2[wire_config] = subMapPathLengths[wire_config][pathNums[1]];
            shoulderConnections[i].connection[j].optimizedConnectionCoords_2[wire_config] = malloc(subMapPathLengths[wire_config][pathNums[1]] * sizeof(Coordinate_t));
            if (shoulderConnections[i].connection[j].optimizedConnectionCoords_2[wire_config] == 0)  {
              printf("\nERROR: Failed to allocate memory for optimized diff-pair connection in function 'optimizeDiffPairConnections'\n");
              printf(  "       Please inform software developer of this fatal error message.\n\n");
              exit(1);
            }
            for (int segment = 0; segment < subMapPathLengths[wire_config][pathNums[1]]; segment++)  {
              shoulderConnections[i].connection[j].optimizedConnectionCoords_2[wire_config][segment].X = subMapPathCoords[wire_config][pathNums[1]][segment].X + X_offset;
              shoulderConnections[i].connection[j].optimizedConnectionCoords_2[wire_config][segment].Y = subMapPathCoords[wire_config][pathNums[1]][segment].Y + Y_offset;
              shoulderConnections[i].connection[j].optimizedConnectionCoords_2[wire_config][segment].Z = subMapPathCoords[wire_config][pathNums[1]][segment].Z + Z_offset;
            }

          }  // End of for-loop for index 'wire_config'


          //
          // Iterate over both wire-configurations to free memory for arrays
          // that were created in this scope and used by findPath():
          //
          for (int wire_config = NOT_SWAPPED; wire_config <= SWAPPED; wire_config++)  {

            // If this connection starts in a swap-zone, then arrays were not allocated for
            // the SWAPPED wiring configuration. So we free memory associated only with
            // the NOT_SWAPPED configuration:
            if (startTermsInSwapZone && (wire_config == SWAPPED))  {
              continue;
            }

            // Free memory used by 'pathCoords' and 'contigPathCoords' array of arrays:
            endPathfinder(num_routed_nets, subMapPathCoords[wire_config], subMapContigPathCoords[wire_config]);

            // Free the memory that was allocated above for the subMap_cellInfo 3D matrix:
            freeMemory_cellInfo(&subMapInfo, subMap_cellInfo[wire_config]);

            // Free arrays for paths that were allocated within this block:
            free(subMapPathLengths[wire_config]);             subMapPathLengths[wire_config]           = NULL;
            free(subMapPathCoords[wire_config]);              subMapPathCoords[wire_config]            = NULL;
            free(subMapContiguousPathLengths[wire_config]);   subMapContiguousPathLengths[wire_config] = NULL;
            free(subMapContigPathCoords[wire_config]);        subMapContigPathCoords[wire_config]      = NULL;

          }  // End of for-loop for index 'wire_config'


          // For each of the two diff-pair paths in the connection, free memory for the large arrays
          // used by the path-finding function, findPath():
          for (int pathIndex = 0; pathIndex < 2; pathIndex++) {
            freePathFindingArrays(&subMapPathFinding[pathIndex], &subMapInfo);
          }

          // Free memory from the heap for variable 'subMapInfo' that was allocated
          // earlier in this block:
          freeMemory_mapInfo(&subMapInfo);

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            if (! pathFound)  {
              printf("\nDEBUG: (thread %2d) pathFound is FALSE, so mapSizeMultiplier will be incremented from its current value (%d)\n\n",
                     thread_num, mapSizeMultiplier);
            }  // End of if-block for pathFound == FALSE
            else  {
              printf("\nDEBUG: (thread %2d) In optimizeDiffPairConnections after running findPath %d times in iteration %d, pseudo-path %d, connection %d:\n",
                     thread_num, subMapInfo.current_iteration, mapInfo->current_iteration, shoulderConnections[i].pseudoPath, j);
              printf(  "DEBUG: (thread %2d)     pathFound = %d\n", thread_num, pathFound);
              printf(  "DEBUG: (thread %2d)    Gcost[NOT_SWAPPED][0] = %'lu for path %d\n", thread_num, Gcost[NOT_SWAPPED][0], pathNums[0]);
              printf(  "DEBUG: (thread %2d)    Gcost[NOT_SWAPPED][1] = %'lu for path %d\n", thread_num, Gcost[NOT_SWAPPED][1], pathNums[1]);
              if (! startTermsInSwapZone)  {
                printf(  "DEBUG: (thread %2d)        Gcost[SWAPPED][0] = %'lu for path %d\n", thread_num, Gcost[SWAPPED][0],     pathNums[0]);
                printf(  "DEBUG: (thread %2d)        Gcost[SWAPPED][1] = %'lu for path %d\n", thread_num, Gcost[SWAPPED][1],     pathNums[1]);
              }
              printf(  "DEBUG: (thread %2d) Based on the above, the shoulderConnections[%d].connection[%d].symmetryRatio = %.3f\n",
                     thread_num, i, j, shoulderConnections[i].connection[j].symmetryRatio);
              printf(  "DEBUG: (thread %2d) Based on the above, the shoulderConnections[%d].connection[%d].swap is set to %d\n\n",
                     thread_num, i, j, shoulderConnections[i].connection[j].swap);
            }  // End of else-block for pathFound == TRUE
          }  // End of if-block for DEBUG_ON == TRUE
          #endif

        }  // End of while-loop for pathFound == FALSE

        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: (thread %2d) Done with connection %d of pseudo-path %d in iteration %d.\n",
                 thread_num, j, shoulderConnections[i].pseudoPath, mapInfo->current_iteration);
        }
        #endif

      }  // End of if-block for j < the number of connections for the current pseudo-path
    }  // End of for-loop for index 'j'
  }  // End of for-loop for index 'i'
  //
  // The above line represents the end of parallel processing
  //

  //
  // Iterate over all pseudo-paths to find the optimal set of connections at each
  // trace-to-terminal and trace-to-via connection:
  //
  for (int i = 0; i < numPseudoPaths; i++)  {
    int num_swaps = 0;

    // Iterate over all connections in pseudo-path 'i':
    for (int j = 0; j < shoulderConnections[i].numConnections; j++)  {

      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("\nDEBUG: For pseudo-path %d, connection %d, in iteration %d:\n", shoulderConnections[i].pseudoPath, j, mapInfo->current_iteration);

        printf("  DEBUG:   startSegment_1 = %d at (%d,%d,%d)        endSegment_1 = %d at (%d,%d,%d)\n",
                  shoulderConnections[i].connection[j].startSegment_1, shoulderConnections[i].connection[j].startCoord_1.X,
                  shoulderConnections[i].connection[j].startCoord_1.Y, shoulderConnections[i].connection[j].startCoord_1.Z,
                  shoulderConnections[i].connection[j].endSegment_1, shoulderConnections[i].connection[j].endCoord_1.X,
                  shoulderConnections[i].connection[j].endCoord_1.Y, shoulderConnections[i].connection[j].endCoord_1.Z);
        printf("  DEBUG:   startSegment_2 = %d at (%d,%d,%d)        endSegment_2 = %d at (%d,%d,%d)\n",
                  shoulderConnections[i].connection[j].startSegment_2, shoulderConnections[i].connection[j].startCoord_2.X,
                  shoulderConnections[i].connection[j].startCoord_2.Y, shoulderConnections[i].connection[j].startCoord_2.Z,
                  shoulderConnections[i].connection[j].endSegment_2, shoulderConnections[i].connection[j].endCoord_2.X,
                  shoulderConnections[i].connection[j].endCoord_2.Y, shoulderConnections[i].connection[j].endCoord_2.Z);
        printf("  DEBUG:   swap = %d\n\n", shoulderConnections[i].connection[j].swap);
      }
      #endif

      // Based on the metrics for the current connection, decide whether it's best to
      // swap the connections:
      if (shoulderConnections[i].connection[j].swap)  {
        // Increment the number of swapped connections for the current pseudo-path:
        num_swaps++;
      }


      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("\nDEBUG: (thread %2d) Pseudo-path %d, connection %d, in iteration %d:\n", omp_get_thread_num(), shoulderConnections[i].pseudoPath, j,
               mapInfo->current_iteration);
        printf(  "DEBUG: (thread %2d)   Preliminary symmetryRatio = %3.5f\n", omp_get_thread_num(), shoulderConnections[i].connection[j].symmetryRatio);
        printf(  "DEBUG: (thread %2d)    Preliminary 'swap' value = %d\n\n", omp_get_thread_num(), shoulderConnections[i].connection[j].swap);
      }
      #endif

    }  // End of for-loop for index 'j' (0 to numConnections)

    #ifdef DEBUG_optimizeDiffPairConnections
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Pseudo-path #%d has %d connections to be swapped, based on G-costs.\n",
             omp_get_thread_num(), shoulderConnections[i].pseudoPath, num_swaps);
    }
    #endif

    //
    // Check if a non-P/N-swappable diff-pair has an odd number of swaps:
    //
    if ((shoulderConnections[i].PN_swappable == FALSE) && (num_swaps % 2))  {
      // Path is not PN-swappable and has an odd-number of swaps. So we evaluate the start-
      // and end-terminals to find which of these two connections would be least-impacted
      // if it were swapped or un-swapped. This connection is that whose ratio
      //
      //    (Gcost_1to1 + Gcost_2to2)/(Gcost_1to1 + Gcost_2to2 + Gcost_1to2 + Gcost_2to1)
      //
      // is closest to 0.5. We call the above ratio the 'impact ratio'.

      // Calculate the 'impact ratio' for the start- and end-terminals:
      float startTerm_impactRatio = fabs( shoulderConnections[i].connection[0].symmetryRatio - 0.5 );
      float endTerm_impactRatio   = fabs( shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].symmetryRatio - 0.5 );

      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("\nDEBUG: The diff-pair paths #%d and %d are not P/N-swappable, and there's an odd number of connection-swaps.\n",
               shoulderConnections[i].diffPairPath_1, shoulderConnections[i].diffPairPath_2);
        printf("DEBUG:   Connection at start-terminal has an impact of %2.4f.\n", startTerm_impactRatio);
        printf("DEBUG:   Connection at end-terminal has an impact of   %2.4f.\n", endTerm_impactRatio);
      }
      #endif

      // 'min_impact_connection' refers to the connection number that would have the least
      // impact of swapped/unswapped. Allowed values are zero (the start-terminal) or the
      // connection number of the end-terminal (numConnections - 1):
      int min_impact_connection = 0;
      if (endTerm_impactRatio < startTerm_impactRatio)  {
        min_impact_connection = shoulderConnections[i].numConnections - 1;
        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("DEBUG:     Swapping/unswapping the end-terminal connection would have the least impact, so it will be swapped/unswapped.\n\n");
        }
        #endif
      }
      #ifdef DEBUG_optimizeDiffPairConnections
      else  {
        if (DEBUG_ON)  {
          printf("DEBUG:     Swapping/unswapping the start-terminal connection would have the least impact, so it will be swapped/unswapped.\n\n");
        }
      }
      #endif

      // Now that we know which terminal-connection would be least impacted if it were
      // swapped/unswapped, we toggle the swap status of that connection:
      if (shoulderConnections[i].connection[min_impact_connection].swap)  {
        shoulderConnections[i].connection[min_impact_connection].swap = NOT_SWAPPED;
        shoulderConnections[i].numSwaps = num_swaps - 1;
      }
      else  {
        shoulderConnections[i].connection[min_impact_connection].swap = SWAPPED;
        shoulderConnections[i].numSwaps = num_swaps + 1;
      }  // End of if/else block
    }  // End of if-block for non-P/N-swappable diff-pair having odd number of swaps
    else  {
      // We got here, so PN_swappable is TRUE and/or num_swaps is even.
      shoulderConnections[i].numSwaps = num_swaps;

      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("\nDEBUG: (thread %2d) The diff-pair paths #%d and %d are either P/N-swappable, or there's an even number of connection-swaps (%d).\n",
               omp_get_thread_num(), shoulderConnections[i].diffPairPath_1, shoulderConnections[i].diffPairPath_2, shoulderConnections[i].numSwaps);
        printf(  "DEBUG: (thread %2d)   So there is no need to toggle any connections\n\n", omp_get_thread_num());
      }
      #endif

    }  // End of else-block
  }  // End of for-loop for index 'i' (0 to numPseudoPaths)

  //
  // Iterate over all pseudo-paths to stitch the trace-segments to the appropriate terminals and vias:
  //
  for (int i = 0; i < numPseudoPaths; i++)  {

    // If zero connections were detected for the current pseudo-path, then skip
    // to the next pseudo-path:
    if (shoulderConnections[i].numConnections == 0)  {

      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("\nDEBUG: (thread %2d) No diff-pair connections were detected for pseudo-path %d, so no need to stitch together the paths.\n\n",
               omp_get_thread_num(), shoulderConnections[i].pseudoPath);
      }
      #endif

      continue;
    }

    // Make local copies of the diff-pair path numbers:
    int path_1 = shoulderConnections[i].diffPairPath_1;
    int path_2 = shoulderConnections[i].diffPairPath_2;

    // Create arrays and allocate memory for 2 new paths. Initially, allocate as many elements as
    // the sum of the segments of both shoulder-paths. Later, we'll re-allocate memory to the
    // actual lengths of each new shoulder-path:
    Coordinate_t *newPath_1, *newPath_2;
    newPath_1 = malloc((pathLengths[path_1] + pathLengths[path_2]) * sizeof(Coordinate_t));
    newPath_2 = malloc((pathLengths[path_1] + pathLengths[path_2]) * sizeof(Coordinate_t));
    if ((newPath_1 == NULL) || (newPath_2 == NULL))  {
      printf("\nERROR: Memory could not be allocated for one or both of the following arrays in function 'optimizeDiffPairConnections':\n");
      printf(  "         newPath_1, newPath_2\n");
      printf(  "       Each array was initially intended to hold %d elements. Please inform the software developer of this fatal error message.\n\n",
                pathLengths[path_1] + pathLengths[path_2]);
      exit(1);
    }  // End of fatal error condition

    // 'swapPaths' is a Boolean variable that specifies whether to swap the segments
    // between the two diff-pair paths. Value is based on the 'swap' Boolean variable
    // at each connection
    int swapPaths = FALSE;

    // If all of the following three criteria are satisfied, then swap the start-terminals
    // so that the end-terminals don't need to be swapped as a result of an odd number of
    // swaps along the pseudo-path:
    //   (a) pseudo-path 'i' is P/N-swappable, and
    //   (b) the number of swaps is odd, and
    //   (c) the start-terminals are not in a pin-swap zone
    if (   (shoulderConnections[i].PN_swappable)                         // Criterion (a) above
        && (shoulderConnections[i].numSwaps % 2))                        // Criterion (b) above
    {

      // Regardless of whether the start-terminals are in a pin-swap zone,
      // toggle/complement the Boolean 'swapPaths' variable, which specifies
      // whether to swap the segments between diff-pair paths:
      swapPaths ^= TRUE;   // Using XOR to flip the Boolean value of variable.

      if  (mapInfo->swapZone[shoulderConnections[i].pseudoPath] == 0)  {  // Criterion (c) above
        Coordinate_t tempCoord;
        tempCoord                    = mapInfo->start_cells[path_1];
        mapInfo->start_cells[path_1] = mapInfo->start_cells[path_2];
        mapInfo->start_cells[path_2] = tempCoord;

        // Also swap the local copies of the start-coordinates, which can be different than the
        // global copies (above) if the global start-coordinates are in a swap-zone:
        tempCoord                                         = shoulderConnections[i].connection[0].startCoord_1;
        shoulderConnections[i].connection[0].startCoord_1 = shoulderConnections[i].connection[0].startCoord_2;
        shoulderConnections[i].connection[0].startCoord_2 = tempCoord;

        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: Swapping the start-terminals because paths %d and %d are P/N-swappable\n", path_1, path_2);
          printf("DEBUG: and there's an odd number of connections to be swapped (%d).\n\n", shoulderConnections[i].numSwaps);
          printf("DEBUG: Also swapping the congestion around the pseudo-start-terminal (%d,%d,%d) for paths %d and %d.\n\n",
                 mapInfo->start_cells[shoulderConnections[i].pseudoPath].X, mapInfo->start_cells[shoulderConnections[i].pseudoPath].Y,
                 mapInfo->start_cells[shoulderConnections[i].pseudoPath].Z, path_1, path_2);
        }
        #endif

        //
        // Swap the congestion around the pseudo-start-terminal between the two diff-pair paths:
        //
        swap_PN_congestion(shoulderConnections[i].pseudoPath, path_1, path_2, user_inputs, cellInfo, mapInfo);

        // Before toggling the Boolean flags that indicate whether the start-coordinates
        // have been swapped, confirm that the flags for both diff-pair nets are identical,
        // along with their parent pseudo-path. If not, issue a fatal error message:
        if (   (mapInfo->diff_pair_terms_swapped[path_1] != mapInfo->diff_pair_terms_swapped[path_2])
            || (mapInfo->diff_pair_terms_swapped[path_1] != mapInfo->diff_pair_terms_swapped[shoulderConnections[i].pseudoPath]))  {
          printf("\n\nERROR: Function 'matchShoulderPathsToTerminals' detected an unexpected condition in which the 'diff_pair_terms_swapped'\n");
          printf(    "       Boolean flags were different for two differential-pair nets. They should always be identical:\n");
          printf(    "            Flag = %d for diff-pair net '%s'\n", mapInfo->diff_pair_terms_swapped[path_1], user_inputs->net_name[path_1]);
          printf(    "            Flag = %d for diff-pair net '%s'\n", mapInfo->diff_pair_terms_swapped[path_2], user_inputs->net_name[path_2]);
          printf(    "            Flag = %d for pseudo-net '%s'\n", mapInfo->diff_pair_terms_swapped[shoulderConnections[i].pseudoPath],
                     user_inputs->net_name[shoulderConnections[i].pseudoPath]);
          printf(    "       Please inform the software developer of this fatal error.\n\n");
          exit(1);
        }  // End of if-block for detecting different Boolean flag values for two diff-pair nets

        // Toggle the Boolean flag that indicates whether the start-coordinates
        // have been swapped:
        if (mapInfo->diff_pair_terms_swapped[shoulderConnections[i].pseudoPath])  {
          // Flags were TRUE, so now set them to FALSE:
          mapInfo->diff_pair_terms_swapped[shoulderConnections[i].pseudoPath] = FALSE;
          mapInfo->diff_pair_terms_swapped[path_1] = FALSE;
          mapInfo->diff_pair_terms_swapped[path_2] = FALSE;
        }  // End of if-block for diff_pair_terms_swapped[pseudoPath] == TRUE
        else  {
          // Flags were FALSE, so now set them to TRUE:
          mapInfo->diff_pair_terms_swapped[shoulderConnections[i].pseudoPath] = TRUE;
          mapInfo->diff_pair_terms_swapped[path_1] = TRUE;
          mapInfo->diff_pair_terms_swapped[path_2] = TRUE;
        }  // End of else-block for diff_pair_terms_swapped[pseudoPath] == FALSE
      }  // End of if-block for path's start-terminals not being in a swap-zone
    }  // End of if-block for needing to swap the start-terminals.

    else  {

      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("\nDEBUG: No need to swap the start-terminals for paths %d and %d because they're either not \n", path_1, path_2);
        printf("DEBUG: P/N-swappable or there's an even number of connections to be swapped (%d)\n", shoulderConnections[i].numSwaps);
        printf("DEBUG: or the terminals are in a swap-zone (%d)\n\n", mapInfo->swapZone[shoulderConnections[i].pseudoPath]);
      }
      #endif
    }  // End of else-block


    //
    // Stitch together each new shoulder-path using the original shoulder-paths and the
    // information in the 'connections' array:
    //
    // Start at the beginning of the existing pseudo-path and visit each connection (each terminal
    // and the top and bottom of each via):
    int numNewSegments_1 = 0;  // Number of segments in new shoulder-path #1
    int numNewSegments_2 = 0;  // Number of segments in new shoulder-path #2
    #ifdef DEBUG_optimizeDiffPairConnections
    int prevNumNewSegments_1 = 0;  // Used for debugging only
    int prevNumNewSegments_2 = 0;  // Used for debugging only
    #endif

    // Iterate over all connections in pseudo-path 'i' except the last one. The segments
    // near the end-terminal will be handled immediately after this for-loop:
    for (int j = 0; j < shoulderConnections[i].numConnections - 1; j++)  {

      // 'swapConnection' is the current connection's value of the 'swap' Boolean variable:
      //    0 = NOT SWAPPED
      //    1 = SWAPPED
      int swapConnection = shoulderConnections[i].connection[j].swap;



      // Modify the congestion in the main map to ensure that there's a congestion-free
      // path from the start-terminal of the connection to its end-terminal if the
      // following criteria are satisfied:
      //   (a) DRC-free routing was found in the sub-maps, AND
      //   (b) the connection's two start-terminals are on the same layer, and
      //       the connection's two end-terminals are on the same layer.
      if (   shoulderConnections[i].connection[j].DRC_free[swapConnection]   // Criterion (a) in above list
          && shoulderConnections[i].connection[j].sameLayerTerminals)        // Criterion (b) in above list
      {
        if (swapPaths)  {
          // We got here, so the current value of Boolean value 'swapPaths' is TRUE, which means that the start-terminals
          // for the current connection j have been swapped. We therefore swap the congestion along the optimized path:
          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, j=%d, swapPaths=%d, swapConnection=%d,\n",
                   i, j, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_1[%d] = %d\n", path_2, path_1, swapConnection,
                   shoulderConnections[i].connection[j].optimizedConnectionLength_1[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_2, path_1,
                                     shoulderConnections[i].connection[j].optimizedConnectionLength_1[swapConnection],
                                     shoulderConnections[i].connection[j].optimizedConnectionCoords_1[swapConnection],
                                     cellInfo, mapInfo);

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, j=%d, swapPaths=%d, swapConnection=%d,\n",
                   i, j, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_2[%d] = %d\n", path_1, path_2, swapConnection,
                   shoulderConnections[i].connection[j].optimizedConnectionLength_2[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_1, path_2,
                                     shoulderConnections[i].connection[j].optimizedConnectionLength_2[swapConnection],
                                     shoulderConnections[i].connection[j].optimizedConnectionCoords_2[swapConnection],
                                     cellInfo, mapInfo);
        } // End of if-block for swapPaths == TRUE

        else  {
          // We got here, so the current value of Boolean value 'swapPaths' is FALSE, which means that the start-terminals
          // for the current connection j have not been swapped. We therefore do not swap the congestion along the optimized path:
          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, j=%d, swapPaths=%d, swapConnection=%d,\n",
                   i, j, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_1[%d] = %d\n", path_1, path_2, swapConnection,
                   shoulderConnections[i].connection[j].optimizedConnectionLength_1[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_1, path_2,
                                     shoulderConnections[i].connection[j].optimizedConnectionLength_1[swapConnection],
                                     shoulderConnections[i].connection[j].optimizedConnectionCoords_1[swapConnection],
                                     cellInfo, mapInfo);

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, j=%d, swapPaths=%d, swapConnection=%d,\n",
                   i, j, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_2[%d] = %d\n", path_2, path_1, swapConnection,
                   shoulderConnections[i].connection[j].optimizedConnectionLength_2[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_2, path_1,
                                     shoulderConnections[i].connection[j].optimizedConnectionLength_2[swapConnection],
                                     shoulderConnections[i].connection[j].optimizedConnectionCoords_2[swapConnection],
                                     cellInfo, mapInfo);

        }  // End of if-block for swapPaths == FALSE
      }  // End of if-block for DRC_free == TRUE
      else  {
        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: DRC_free[%d] = FALSE, so congestion will not be swapped between paths %d and %d for connection %d.\n\n",
                 swapConnection, path_1, path_2, j);
        }
        #endif
      }  // End of else-block for DRC_free == FALSE


      // Check the current connection's value of the 'swap' Boolean variable:
      if (swapConnection == SWAPPED)  {
        // We got here, so the current connection needs to be swapped. Toggle/complement
        // the Boolean 'swapPaths' variable, which specifies whether to swap the segments
        // between diff-pair paths:
        if (swapPaths)  {
          swapPaths = FALSE;
          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: Toggling 'swapPaths' from TRUE to FALSE because connection #%d's 'swap' value was TRUE.\n\n", j);
          }
          #endif
        }
        else  {
          swapPaths = TRUE;
          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: Toggling 'swapPaths' from FALSE to TRUE because connection #%d's 'swap' value was TRUE.\n\n", j);
          }
          #endif
        }
      }  // End of if-block for 'swap' == TRUE

      //
      // Based on the current value of 'swapPaths', copy segments from the old diff-pair
      // paths into the new 'newPath_1' and 'newPath_2' path-arrays:
      //
      if (swapPaths == TRUE)  {

        #ifdef DEBUG_optimizeDiffPairConnections
        prevNumNewSegments_1 = numNewSegments_1;  // Make temporary copy for debugging purposes
        prevNumNewSegments_2 = numNewSegments_2;  // Make temporary copy for debugging purposes
        #endif

        // Copy path-1 segments to path #2 from segment shoulderConnections[i].connection[j].endSegment_1
        // up to and including segment shoulderConnections[i].connection[j+1].startSegment_1. Increment
        // 'numNewSegments_2' for each new segment.
        for (int pathSegment_1  = shoulderConnections[i].connection[j].endSegment_1;
                 pathSegment_1 <= shoulderConnections[i].connection[j+1].startSegment_1; pathSegment_1++)  {
          newPath_2[numNewSegments_2] = copyCoordinates(pathCoords[path_1][pathSegment_1]);
          numNewSegments_2++;
        }  // End of for-loop for index 'pathSegment_1'

        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("DEBUG: Copied %d segments from original path #%d to new path #%d, from segment #%d (%d,%d,%d) to segment #%d (%d,%d,%d), inclusive.\n",
                  numNewSegments_2 - prevNumNewSegments_2, path_1, path_2,
                  shoulderConnections[i].connection[j].endSegment_1,
                  pathCoords[path_1][shoulderConnections[i].connection[j].endSegment_1].X,
                  pathCoords[path_1][shoulderConnections[i].connection[j].endSegment_1].Y,
                  pathCoords[path_1][shoulderConnections[i].connection[j].endSegment_1].Z,
                  shoulderConnections[i].connection[j+1].startSegment_1,
                  pathCoords[path_1][shoulderConnections[i].connection[j+1].startSegment_1].X,
                  pathCoords[path_1][shoulderConnections[i].connection[j+1].startSegment_1].Y,
                  pathCoords[path_1][shoulderConnections[i].connection[j+1].startSegment_1].Z);
        }  // End of if-block for (DEBUG_ON == TRUE)
        #endif

        // Copy path-2 segments to path #1 from segment shoulderConnections[i].connection[j].endSegment_2
        // up to and including segment shoulderConnections[i].connection[j+1].startSegment_2. Increment
        // 'numNewSegments_1' for each new segment.
        for (int pathSegment_2  = shoulderConnections[i].connection[j].endSegment_2;
                 pathSegment_2 <= shoulderConnections[i].connection[j+1].startSegment_2; pathSegment_2++)  {
          newPath_1[numNewSegments_1] = copyCoordinates(pathCoords[path_2][pathSegment_2]);
          numNewSegments_1++;
        }  // End of for-loop for index 'pathSegment_2'

        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("DEBUG: Copied %d segments from original path #%d to new path #%d, from segment #%d (%d,%d,%d) to segment #%d (%d,%d,%d), inclusive.\n",
                  numNewSegments_1 - prevNumNewSegments_1, path_2, path_1,
                  shoulderConnections[i].connection[j].endSegment_2,
                  pathCoords[path_2][shoulderConnections[i].connection[j].endSegment_2].X,
                  pathCoords[path_2][shoulderConnections[i].connection[j].endSegment_2].Y,
                  pathCoords[path_2][shoulderConnections[i].connection[j].endSegment_2].Z,
                  shoulderConnections[i].connection[j+1].startSegment_2,
                  pathCoords[path_2][shoulderConnections[i].connection[j+1].startSegment_2].X,
                  pathCoords[path_2][shoulderConnections[i].connection[j+1].startSegment_2].Y,
                  pathCoords[path_2][shoulderConnections[i].connection[j+1].startSegment_2].Z);
        }  // End of if-block for (DEBUG_ON == TRUE)
        #endif

      }  // End of if-block for (swapPaths == TRUE)
      else  {
        // We got here, so we should *not* swap the paths.
        //

        #ifdef DEBUG_optimizeDiffPairConnections
        prevNumNewSegments_1 = numNewSegments_1;  // Make temporary copy for debugging purposes
        prevNumNewSegments_2 = numNewSegments_2;  // Make temporary copy for debugging purposes
        #endif

        // Copy path-1 segments to path #1 from segment shoulderConnections[i].connection[j].endSegment_1
        // up to and including segment shoulderConnections[i].connection[j+1].startSegment_1. Increment
        // 'numNewSegments_1' for each new segment.
        for (int pathSegment_1  = shoulderConnections[i].connection[j].endSegment_1;
                 pathSegment_1 <= shoulderConnections[i].connection[j+1].startSegment_1; pathSegment_1++)  {
          newPath_1[numNewSegments_1] = copyCoordinates(pathCoords[path_1][pathSegment_1]);
          numNewSegments_1++;
        }  // End of for-loop for index 'pathSegment_1'

        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("DEBUG: Copied %d segments from original path #%d to new path #%d, from segment #%d (%d,%d,%d) to segment #%d (%d,%d,%d), inclusive.\n",
                  numNewSegments_1 - prevNumNewSegments_1, path_1, path_1,
                  shoulderConnections[i].connection[j].endSegment_1,
                  pathCoords[path_1][shoulderConnections[i].connection[j].endSegment_1].X,
                  pathCoords[path_1][shoulderConnections[i].connection[j].endSegment_1].Y,
                  pathCoords[path_1][shoulderConnections[i].connection[j].endSegment_1].Z,
                  shoulderConnections[i].connection[j+1].startSegment_1,
                  pathCoords[path_1][shoulderConnections[i].connection[j+1].startSegment_1].X,
                  pathCoords[path_1][shoulderConnections[i].connection[j+1].startSegment_1].Y,
                  pathCoords[path_1][shoulderConnections[i].connection[j+1].startSegment_1].Z);
        }  // End of if-block for (DEBUG_ON == TRUE)
        #endif

        // Copy path-2 segments to path #2 from segment shoulderConnections[i].connection[j].endSegment_2
        // up to and including segment shoulderConnections[i].connection[j+1].startSegment_2. Increment
        // 'numNewSegments_2' for each new segment.
        for (int pathSegment_2  = shoulderConnections[i].connection[j].endSegment_2;
                 pathSegment_2 <= shoulderConnections[i].connection[j+1].startSegment_2; pathSegment_2++)  {
          newPath_2[numNewSegments_2] = copyCoordinates(pathCoords[path_2][pathSegment_2]);
          numNewSegments_2++;
        }  // End of for-loop for index 'pathSegment_2'

        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("DEBUG: Copied %d segments from original path #%d to new path #%d, from segment #%d (%d,%d,%d) to segment #%d (%d,%d,%d), inclusive.\n",
                  numNewSegments_2 - prevNumNewSegments_2, path_2, path_2,
                  shoulderConnections[i].connection[j].endSegment_2,
                  pathCoords[path_2][shoulderConnections[i].connection[j].endSegment_2].X,
                  pathCoords[path_2][shoulderConnections[i].connection[j].endSegment_2].Y,
                  pathCoords[path_2][shoulderConnections[i].connection[j].endSegment_2].Z,
                  shoulderConnections[i].connection[j+1].startSegment_2,
                  pathCoords[path_2][shoulderConnections[i].connection[j+1].startSegment_2].X,
                  pathCoords[path_2][shoulderConnections[i].connection[j+1].startSegment_2].Y,
                  pathCoords[path_2][shoulderConnections[i].connection[j+1].startSegment_2].Z);
        }  // End of if-block for (DEBUG_ON == TRUE)
        #endif

      }  // End of else-block for (swapPaths == FALSE)

    }  // End of for-loop for index 'j' (from 0 to numConnections - 1)


    //
    // Copy the remaining segments from endSegment_1 of the final connection to the end of the original
    // diff-pair path. Do the same for segments between endSegment_2 and the end of that diff-pair path.
    // Because the end-terminals must *never* be swapped, it is not allowed for 'swapConnection' XOR 'swapPaths'
    // to be TRUE. In other words, 'swapConnection' and 'swapPaths' must *both* be TRUE or must *both* be FALSE.
    //
    // 'swapConnection' is the last connection's value of the 'swap' Boolean variable:
    //    0 = NOT SWAPPED
    //    1 = SWAPPED
    int swapConnection = shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].swap;
    if (   (   (swapConnection == SWAPPED)
            && (swapPaths == FALSE))
        || (   (swapConnection == NOT_SWAPPED)
                && (swapPaths == TRUE)))  {

      // Issue a fatal error because the program should never have gotten to this if-block. If we
      // got here, it would mean that the end-terminals should be swapped, which must never happen.
      printf("\nERROR: Function optimizeDiffPairConnections encountered an unexpected condition in which the\n");
      printf(  "       end-terminals of pseudo-path %d were swapped. (End-terminals should NEVER be swapped.)\n", i);
      printf(  "       Please report this fatal error message to the software developer.\n\n");
      exit(1);
    }  // End of if-block
    else  {

      #ifdef DEBUG_optimizeDiffPairConnections
      prevNumNewSegments_1 = numNewSegments_1;  // Make temporary copy for debugging purposes
      prevNumNewSegments_2 = numNewSegments_2;  // Make temporary copy for debugging purposes
      #endif

      // Copy path-1 segments to path #1 from segment:
      // shoulderConnections[i].connection[shoulderConnections[i].numConnections – 1].endSegment_1
      // up to and including the last segment in path #1. Increment 'numNewSegments_1' for each new segment.
      for (int pathSegment_1 = shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_1;
               pathSegment_1 < pathLengths[path_1]; pathSegment_1++)  {
        newPath_1[numNewSegments_1] = copyCoordinates(pathCoords[path_1][pathSegment_1]);
        numNewSegments_1++;
      }  // End of for-loop for index 'pathSegment_1'

      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("DEBUG: Copied %d segments from original path #%d to new path #%d, from segment #%d (%d,%d,%d) to segment #%d (%d,%d,%d), inclusive.\n",
                numNewSegments_1 - prevNumNewSegments_1, path_1, path_1,
                shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_1,
                pathCoords[path_1][shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_1].X,
                pathCoords[path_1][shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_1].Y,
                pathCoords[path_1][shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_1].Z,
                pathLengths[path_1] - 1,
                pathCoords[path_1][pathLengths[path_1] - 1].X,
                pathCoords[path_1][pathLengths[path_1] - 1].Y,
                pathCoords[path_1][pathLengths[path_1] - 1].Z);
      }  // End of if-block for (DEBUG_ON == TRUE)
      #endif


      // Copy path-2 segments to path #2 from segment:
      // shoulderConnections[i].connection[shoulderConnections[i].numConnections – 1].endSegment_2
      // up to and including the last segment in path #2. Increment 'numNewSegments_2' for each new segment.
      for (int pathSegment_2 = shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_2;
               pathSegment_2 < pathLengths[path_2]; pathSegment_2++)  {
        newPath_2[numNewSegments_2] = copyCoordinates(pathCoords[path_2][pathSegment_2]);
        numNewSegments_2++;
      }  // End of for-loop for index 'pathSegment_2'


      #ifdef DEBUG_optimizeDiffPairConnections
      if (DEBUG_ON)  {
        printf("DEBUG: Copied %d segments from original path #%d to new path #%d, from segment #%d (%d,%d,%d) to segment #%d (%d,%d,%d), inclusive.\n",
                numNewSegments_2 - prevNumNewSegments_2, path_2, path_2,
                shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_2,
                pathCoords[path_2][shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_2].X,
                pathCoords[path_2][shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_2].Y,
                pathCoords[path_2][shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].endSegment_2].Z,
                pathLengths[path_2] - 1,
                pathCoords[path_2][pathLengths[path_2] - 1].X,
                pathCoords[path_2][pathLengths[path_2] - 1].Y,
                pathCoords[path_2][pathLengths[path_2] - 1].Z);
      }  // End of if-block for (DEBUG_ON == TRUE)
      #endif

      // If compute-intensive path-finding was performed to optimize the routing for the
      // current connection, and if DRC-free routing was found in the sub-maps, then
      // modify the congestion in the main map to ensure that there's a congestion-free
      // path from the start-terminal of the connection to its end-terminal:


      // Modify the congestion in the main map to ensure that there's a congestion-free
      // path from the start-terminal of the connection to its end-terminal if the
      // following criteria are satisfied:
      //   (a) DRC-free routing was found in the sub-maps, AND
      //   (b) the connection's two start-terminals are on the same layer, and
      //       the connection's two end-terminals are on the same layer.

      if (   shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].DRC_free[swapConnection]  // Criterion (a) in above list
          && shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].sameLayerTerminals)       // Criterion (b) in above list
      {
        if (swapPaths == FALSE)  {

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, (numConnections - 1) = %d, swapPaths=%d, swapConnection=%d,\n",
                   i, shoulderConnections[i].numConnections - 1, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_1[%d] = %d\n", path_1, path_2, swapConnection,
                   shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_1[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_1, path_2,
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_1[swapConnection],
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionCoords_1[swapConnection],
                                     cellInfo, mapInfo);

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, (numConnections - 1) = %d, swapPaths=%d, swapConnection=%d,\n",
                   i, shoulderConnections[i].numConnections - 1, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_2[%d] = %d\n", path_2, path_1, swapConnection,
                   shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_2[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_2, path_1,
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_2[swapConnection],
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionCoords_2[swapConnection],
                                     cellInfo, mapInfo);
        }  // End of if-block (swapPaths == FALSE)
        else  {

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, (numConnections - 1) = %d, swapPaths=%d, swapConnection=%d,\n",
                   i, shoulderConnections[i].numConnections - 1, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_1[%d] = %d\n", path_2, path_1, swapConnection,
                   shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_1[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_2, path_1,
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_1[swapConnection],
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionCoords_1[swapConnection],
                                     cellInfo, mapInfo);

          #ifdef DEBUG_optimizeDiffPairConnections
          if (DEBUG_ON)  {
            printf("\nDEBUG: About to call convertCongestionAlongPath with i=%d, (numConnections - 1) = %d, swapPaths=%d, swapConnection=%d,\n",
                   i, shoulderConnections[i].numConnections - 1, swapPaths, swapConnection);
            printf(  "DEBUG: optimized path=%d, partner path=%d, optimizedConnectionLength_2[%d] = %d\n", path_1, path_2, swapConnection,
                   shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_2[swapConnection]);
          }
          #endif
          convertCongestionAlongPath(path_1, path_2,
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionLength_2[swapConnection],
                                     shoulderConnections[i].connection[shoulderConnections[i].numConnections - 1].optimizedConnectionCoords_2[swapConnection],
                                     cellInfo, mapInfo);
        }  // End of else-block (swapPaths == TRUE)
      }  // End of if-block for DRC_free == TRUE
      else  {
        #ifdef DEBUG_optimizeDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: DRC_free[%d] = FALSE, so congestion will not be swapped between paths %d and %d for connection %d.\n\n",
                 swapConnection, path_1, path_2, shoulderConnections[i].numConnections - 1);
        }
        #endif
      }  // End of else-block for DRC_free == FALSE
    }  // End of else-block


    //
    // Now that we've stitched together the optimized diff-pair paths, we need to copy them over
    // from arrays 'newPath_1' and 'newPath_2' to arrays pathCoords[path_1] and pathCoords[path_2].
    // We also must adjust the lengths of these arrays in the pathLengths[] variable.
    //
    // Start with path #1: Re-allocate the length of pathCoords[path_1] to match the length
    // of the new diff-pair path:
    pathCoords[path_1] = realloc(pathCoords[path_1], numNewSegments_1 * sizeof(Coordinate_t));
    pathLengths[path_1] = numNewSegments_1;
    for (int pathSegment_1 = 0; pathSegment_1 < numNewSegments_1; pathSegment_1++)  {
      pathCoords[path_1][pathSegment_1] = copyCoordinates(newPath_1[pathSegment_1]);
    }  // End of for-loop for index 'pathSegment_1'

    // Repeat the process for path #2: Re-allocate the length of pathCoords[path_2] to
    // match the length of the new diff-pair path:
    pathCoords[path_2] = realloc(pathCoords[path_2], numNewSegments_2 * sizeof(Coordinate_t));
    pathLengths[path_2] = numNewSegments_2;
    for (int pathSegment_2 = 0; pathSegment_2 < numNewSegments_2; pathSegment_2++)  {
      pathCoords[path_2][pathSegment_2] = copyCoordinates(newPath_2[pathSegment_2]);
    }  // End of for-loop for index 'pathSegment_2'

    // Free the memory allocated in arrays 'newPath_1' and 'newPath_2':
    free(newPath_1);    newPath_1 = NULL;
    free(newPath_2);    newPath_2 = NULL;

    // In rare cases, stitching together the diff-pair paths can result in two consecutive segments
    // that have the same x/y/z coordinates. So we run 'deleteDuplicatePoints()' on both diff-pair paths:
    deleteDuplicatePoints(path_1, pathCoords, pathLengths, mapInfo);
    deleteDuplicatePoints(path_2, pathCoords, pathLengths, mapInfo);

  }  // End of for-loop for index 'i'

  // Free memory that was allocated within this function for the connections:
  for (int i = 0; i < numPseudoPaths; i++)  {
    for (int j = 0; j < shoulderConnections[i].numConnections; j++)  {
      if (shoulderConnections[i].connection[j].optimizedConnectionLength_1[NOT_SWAPPED] > 0)  {
        free(shoulderConnections[i].connection[j].optimizedConnectionCoords_1[NOT_SWAPPED]);
        shoulderConnections[i].connection[j].optimizedConnectionCoords_1[NOT_SWAPPED] = NULL;
      }

      if (shoulderConnections[i].connection[j].optimizedConnectionLength_1[SWAPPED] > 0)  {
        free(shoulderConnections[i].connection[j].optimizedConnectionCoords_1[SWAPPED]);
        shoulderConnections[i].connection[j].optimizedConnectionCoords_1[SWAPPED] = NULL;
      }

      if (shoulderConnections[i].connection[j].optimizedConnectionLength_2[NOT_SWAPPED] > 0)  {
        free(shoulderConnections[i].connection[j].optimizedConnectionCoords_2[NOT_SWAPPED]);
        shoulderConnections[i].connection[j].optimizedConnectionCoords_2[NOT_SWAPPED] = NULL;
      }

      if (shoulderConnections[i].connection[j].optimizedConnectionLength_2[SWAPPED] > 0)  {
        free(shoulderConnections[i].connection[j].optimizedConnectionCoords_2[SWAPPED]);
        shoulderConnections[i].connection[j].optimizedConnectionCoords_2[SWAPPED] = NULL;
      }
    }  // End of for-loop for index 'j' (0 to numConnections)
    free(shoulderConnections[i].connection);
    shoulderConnections[i].connection = NULL;
  }
  free(shoulderConnections);
  shoulderConnections = NULL;

}  // End of function 'optimizeDiffPairConnections'



