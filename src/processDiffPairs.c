#include "global_defs.h"
#include "createDiffPairs.h"
#include "pruneDiffPairs.h"
#include "optimizeDiffPairs.h"
#include "findShortPathHeuristically.h"



//-----------------------------------------------------------------------------
// Name: calcGapRoutingRestrictions
// Desc: Calculate the routing restrictions for function findPath() to use when
//       routing across a gap of a diff-pair path. The restrictions depend on
//       the number of layers spanned by the gap, and whether a pseudo-via can
//       be associated with the gap. This function modifies the variable
//       routeRestrictions.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calcGapRoutingRestrictions' and re-compile if you want
// verbose debugging print-statements enabled:
//
// #define DEBUG_calcGapRoutingRestrictions
#undef DEBUG_calcGapRoutingRestrictions

void calcGapRoutingRestrictions(RoutingRestriction_t *routeRestrictions,
                                Coordinate_t startCoordinate, Coordinate_t endCoordinate,
                                int pathNum, Coordinate_t *pathCoords[], int *pathLengths,
                                unsigned char pseudoViaKnown, int pseudoVia_X, int pseudoVia_Y,
                                CellInfo_t ***cellInfo, MapInfo_t *mapInfo, InputValues_t *user_inputs)  {

  #ifdef DEBUG_calcGapRoutingRestrictions
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  int thread_num = omp_get_thread_num();
  if ((mapInfo->current_iteration >= 1) && (mapInfo->current_iteration <= 25))  {
    printf("\n\nDEBUG: (thread %2d) In iteration %d, setting DEBUG_ON to TRUE in calcGapRoutingRestrictions() because specific requirements were met.\n\n",
           thread_num, mapInfo->current_iteration);
    DEBUG_ON = TRUE;

    printf("DEBUG: Entered 'calcGapRoutingRestrictions' within iteration %d, path number %d, startCoord (%d,%d,%d) and endCoord (%d,%d,%d).\n",
           mapInfo->current_iteration, pathNum, startCoordinate.X, startCoordinate.Y, startCoordinate.Z,
           endCoordinate.X, endCoordinate.Y, endCoordinate.Z);
    printf("DEBUG: (thread %2d)      pseudoViaKnown = %d\n", thread_num, pseudoViaKnown);
    printf("DEBUG: (thread %2d)           pseudoVia at (%d,%d)\n\n", thread_num, pseudoVia_X, pseudoVia_Y);
  }
  #endif

  // Set to TRUE the Boolean flag that specifies that routing-restrictions will be
  // used when findPath() is called:
  routeRestrictions->restrictionFlag = TRUE;

  // Initialize to FALSE the 'allowedLayers' element for each layer. Some will later
  // be converted to TRUE:
  for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
    routeRestrictions->allowedLayers[layer] = FALSE;
  }  // End of for-loop for index 'layer'

  // Determine the routing restrictions for constraining findPath():
  //
  // There are two possibilities, each with different restrictions:
  //   Case A: the start- and end-points are on the same layer, or
  //   Case B: the start- and end-points span multiple layers:
  //
  if (startCoordinate.Z == endCoordinate.Z)  {
    // Case A: The start- and end-points are on the same layer.
    //
    // For the layer on which routing takes place, define the 'allowedLayers' value as TRUE,
    // and manually calculate the allowed routing radius:
    routeRestrictions->allowedLayers[startCoordinate.Z] = TRUE;

    // For this case, there are two possibilities, each with slightly different restrictions
    // that affect the centerpoint and the routing radii:
    //    (1) the start-point is in a pin-swap zone, or
    //    (2) the start-point is not in a pin-swap zone.
    //
    if (cellInfo[startCoordinate.X][startCoordinate.Y][startCoordinate.Z].swap_zone)  {

      // For case (1), in which the previous segment was in a pin-swap zone, we assign the
      // radius to be length of the full gap (plus 1 cell to accommodate rounding errors):
      routeRestrictions->allowedRadiiCells[startCoordinate.Z] = 1.0  + calc_2D_Pythagorean_distance_ints(startCoordinate.X, startCoordinate.Y,
                                                                                                         endCoordinate.X,   endCoordinate.Y);
      // We assign the center-point to be the end-point of the gap, since this point
      // is more 'stable' than a point in a pin-swap zone:
      routeRestrictions->centerX = endCoordinate.X;
      routeRestrictions->centerY = endCoordinate.Y;

    }  // End of if-block for start-of-gap being in a swap-zone

    else  {

      // For case (2), in which neither side of the gap is in a pin-swap zone, the routing radius
      // defined to be half the gap distance, plus 1 cell to account for rounding errors. The center-
      // point is the midpoint of the gap:
      routeRestrictions->allowedRadiiCells[startCoordinate.Z] = 1.0  +  0.5 * calc_2D_Pythagorean_distance_ints(startCoordinate.X, startCoordinate.Y,
                                                                                                                endCoordinate.X,   endCoordinate.Y);

      // Define the center-point for routing restrictions as the midpoint of the gap:
      routeRestrictions->centerX = (startCoordinate.X  +  endCoordinate.X) / 2;
      routeRestrictions->centerY = (startCoordinate.Y  +  endCoordinate.Y) / 2;

    }  // End of else-block for start-of-gap *NOT* being in a swap-zone

    // Also calculate the 'allowedRadiiMicrons' element of the routing restrictions, which is
    // simply the 'allowedRadiiCells' element multiplied by the cell size:
    routeRestrictions->allowedRadiiMicrons[startCoordinate.Z] = user_inputs->cell_size_um
                                                                * routeRestrictions->allowedRadiiCells[startCoordinate.Z];

  }  // End of if-block for routing on a *single* layer

  else {  //  The gap includes multiple layers, so calculate routing radii and center-point carefully...

    // Define variables to hold the radius of the routing restrictions:
    float max_routing_radius_cells;
    float max_routing_radius_microns;

    //
    // If input parameter 'pseudoViaKnown' is TRUE, then use the (x,y) coordinates
    // (pseudoVia_X, pseudoVia_Y) as the centerpoints of the routing restrictions.
    //
    if (pseudoViaKnown)  {
      routeRestrictions->centerX = pseudoVia_X;
      routeRestrictions->centerY = pseudoVia_Y;

      //
      // Calculate the routing radius restriction to be the largest distance between
      // the pseudo-via's center and the path segments that we're trying to connect.
      // To avoid rounding issues, we add 4 cells to the radius.
      //
      max_routing_radius_cells
         = max(calc_2D_Pythagorean_distance_ints(startCoordinate.X, startCoordinate.Y, routeRestrictions->centerX, routeRestrictions->centerY),
               calc_2D_Pythagorean_distance_ints(endCoordinate.X,   endCoordinate.Y,   routeRestrictions->centerX, routeRestrictions->centerY))
               + 4.0;
    }  // End of if-block for pseudoViaKnown == TRUE
    else  {
      //
      // We got here, so the input parameter 'pseudoViaKnown' is FALSE, so we have
      // to do our best to locate the pseudo-via associated with the gap in the
      // diff-pair path.
      //

      // To find this pseudo-via, first calculate the midpoint in the X/Y-plane of the two
      // points we're trying to connect, as long as the previous segment is not in a
      // pin-swap zone. (The routing segments in a pin-swap zone should never affect
      // routing outside such zones.)
      Coordinate_t centerPoint;
      if (! cellInfo[startCoordinate.X][startCoordinate.Y][startCoordinate.Z].swap_zone)  {
        centerPoint.X = (startCoordinate.X + endCoordinate.X)/2;
        centerPoint.Y = (startCoordinate.Y + endCoordinate.Y)/2;
      }
      else  {
        // If the start of the gap is in a pin-swap zone, then just use the end of
        // the gap as the 'centerPoint' coordinate:
        centerPoint.X = endCoordinate.X;
        centerPoint.Y = endCoordinate.Y;
      }
      centerPoint.Z = endCoordinate.Z;

      // Define the center-point for via routing restrictions as the location of the pseudo-via
      // that corresponds to this diff-pair via. To find this pseudo-via, we explore the pseudo-
      // net that corresponds to this diff-pair net and locate the pseudo-via that is the closest
      // pseudo-via and which also includes the same layers as the diff-pair via:
      int pseudoPathNum = user_inputs->diffPairToPseudoNetMap[pathNum];

      #ifdef DEBUG_calcGapRoutingRestrictions
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Calling findNearbyLayerTransition_wrapper from calcGapRoutingRestrictions with pathNum = %d, startLayer = %d and endLayer = %d\n",
                thread_num, pseudoPathNum, startCoordinate.Z, endCoordinate.Z);
      }
      #endif

      // Define variable to hold the via-stack of the pseudo-path:
      ViaStack_t closestPseudoVia;

      // Search the pseudo-path for the pseudo-via that corresponds to the diff-pair via. The returned
      // variable, 'closestPseudoVia', contains the start- and end-segments of the pseudo-via.:
      closestPseudoVia = findNearbyLayerTransition_wrapper(pseudoPathNum, pathLengths, pathCoords, startCoordinate.Z,
                                                           endCoordinate.Z, centerPoint.X, centerPoint.Y,
                                                           mapInfo, user_inputs);

      //
      // Check whether findNearbyLayerTransition_wrapper() succeeded in finding a pseudo-via
      // associated with this gap's layer-transition:
      //
      if (closestPseudoVia.endShapeType != TRACE)  {

        // Now that we know the location of the nearby pseudo-via, we assign the centerX/centerY
        // routing restrictions to be the pseudo-via's location:
        routeRestrictions->centerX = pathCoords[pseudoPathNum][closestPseudoVia.endSegment].X;
        routeRestrictions->centerY = pathCoords[pseudoPathNum][closestPseudoVia.endSegment].Y;


        #ifdef DEBUG_calcGapRoutingRestrictions
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) findNearbyLayerTransition_wrapper returned with results:\n", thread_num);
          printf("DEBUG: (thread %2d)         pathNum: %d\n", thread_num, closestPseudoVia.pathNum);
          printf("DEBUG: (thread %2d)    startSegment: %d\n", thread_num, closestPseudoVia.startSegment);
          printf("DEBUG: (thread %2d)      endSegment: %d\n", thread_num, closestPseudoVia.endSegment);
          printf("DEBUG: (thread %2d)    endShapeType: %d\n", thread_num, closestPseudoVia.endShapeType);
          printf("DEBUG: (thread %2d)      startCoord: (%d,%d,%d)\n", thread_num, closestPseudoVia.startCoord.X,
                 closestPseudoVia.startCoord.Y, closestPseudoVia.startCoord.Z);
          printf("DEBUG: (thread %2d)        endCoord: (%d,%d,%d)\n", thread_num, closestPseudoVia.endCoord.X,
                 closestPseudoVia.endCoord.Y, closestPseudoVia.endCoord.Z);
          printf("DEBUG: (thread %2d) (centerX, centerY) values are (%d,%d), which will be used when calling findPath from fillGapsInDiffPairPaths.\n",
                 thread_num, routeRestrictions->centerX, routeRestrictions->centerY);
        }
        #endif

        //
        // Calculate the routing radius restriction to be the largest distance between
        // the pseudo-via's center and the path segments that we're trying to connect.
        // To avoid rounding issues, we add 4 cells to the radius.
        //
        max_routing_radius_cells
           = max(calc_2D_Pythagorean_distance_ints(startCoordinate.X, startCoordinate.Y, routeRestrictions->centerX, routeRestrictions->centerY),
                 calc_2D_Pythagorean_distance_ints(endCoordinate.X,   endCoordinate.Y,   routeRestrictions->centerX, routeRestrictions->centerY))
                 + 4.0;

      }  // End of if-block for findNearbyLayerTransition_wrapper successfully finding a pseudo-via

      else  {
        //
        // If findNearbyLayerTransition_wrapper() failed to find a pseudo-via associated with this gap's
        // layer-transition, then it could represent a corner-case, e.g., the pseudo-via is moving up
        // while the gap in the diff-pair path is moving down. (This can happen when multiple layers
        // of pin-swap zones are stacked atop each other, and the start-terminal is changing layers
        // from one iteration to another.)
        //
        // Treat these special cases similar to how gaps are handled on a single layer. The
        // routing restrictions are calculated based on which of two cases apply:
        //      (1) the start-point is in a pin-swap zone:
        //            >  Assign the radius of the routing restrictions to be length of the
        //               full gap (plus 1 cell to accommodate rounding errors).
        //            >  Assign the center-point of the routing restrictions to be the
        //               end-point of the gap, since this point is more 'stable' than a
        //               point in a pin-swap zone.
        //      (2) or... the start-point is not in a pin-swap zone.
        //            >  The radius of the routing restrictions is defined to be half the gap
        //               distance, plus 1 cell to account for rounding errors.
        //            >  The center-point of the routing restrictions is the midpoint of the gap.
        //

        if (cellInfo[startCoordinate.X][startCoordinate.Y][startCoordinate.Z].swap_zone)  {

          // For case (1), in which the start-segment was in a pin-swap zone, we assign the
          // radius to be length of the full gap (plus 1 cell to accommodate rounding errors):
          max_routing_radius_cells = 1.0  + calc_2D_Pythagorean_distance_ints(startCoordinate.X, startCoordinate.Y,
                                                                              endCoordinate.X,   endCoordinate.Y);
          // We assign the center-point to be the end-point of the gap, since this point
          // is more 'stable' than a point in a pin-swap zone:
          routeRestrictions->centerX = endCoordinate.X;
          routeRestrictions->centerY = endCoordinate.Y;

        }  // End of if-block for start-of-gap being in a swap-zone

        else  {

          // For case (2), in which neither side of the gap is in a pin-swap zone, the routing radius
          // is defined to be half the gap distance, plus 1 cell to account for rounding errors:
          max_routing_radius_cells = 1.0  +  0.5 * calc_2D_Pythagorean_distance_ints(startCoordinate.X, startCoordinate.Y,
                                                                                     endCoordinate.X,   endCoordinate.Y);

          // Define the center-point for routing restrictions as the midpoint of the gap:
          routeRestrictions->centerX = (startCoordinate.X  +  endCoordinate.X) / 2;
          routeRestrictions->centerY = (startCoordinate.Y  +  endCoordinate.Y) / 2;

        }  // End of else-block for start-of-gap *NOT* being in a swap-zone

      }  // End of else-block for findNearbyLayerTransition_wrapper failing to find a pseudo-via

    }  // End of else-block for pseudoViaKnown == FALSE

    // As a safety-factor to account for waypoints or other user-defined obstacles, the routing
    // restriction radius was increased by 25% on 15 February 2024:
    max_routing_radius_cells *= 1.25;

    // Ensure that the routing restriction radius does not exceed diagonal distance
    // of the entire map:
    max_routing_radius_cells = min(max_routing_radius_cells, mapInfo->mapDiagonal);

    // Calculate the radius of the routing restrictions in microns by simply multiplying the
    // value (in cell units) by the cell size in microns:
    max_routing_radius_microns = max_routing_radius_cells * user_inputs->cell_size_um;

    //
    // Finally, set the Boolean 'allowedLayers' element to TRUE or FALSE, and
    // record the allowed routing radius values.
    //
    int minLayerNum = min(startCoordinate.Z, endCoordinate.Z);
    int maxLayerNum = max(startCoordinate.Z, endCoordinate.Z);
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
      if ((layer >= minLayerNum) && (layer <= maxLayerNum))  {
        routeRestrictions->allowedLayers[layer] = TRUE;
        routeRestrictions->allowedRadiiCells[layer]   = max_routing_radius_cells;
        routeRestrictions->allowedRadiiMicrons[layer] = max_routing_radius_microns;
      }
      else  {
        routeRestrictions->allowedLayers[layer] = FALSE;
      }
    }  // End of for-loop for index 'layer'
  }  // End of else-block for calculating the allowed radii across multiple layers

  #ifdef DEBUG_calcGapRoutingRestrictions
  if (DEBUG_ON)  {
    printf("\nDEBUG: At end of calcGapRoutingRestrictions:\n");
    printf("DEBUG:   restrictionFlag = %d\n", routeRestrictions->restrictionFlag);
    printf("DEBUG:   centerX = %d in sub-map\n", routeRestrictions->centerX);
    printf("DEBUG:   centerY = %d in sub-map\n", routeRestrictions->centerY);
    for (int layer = 0; layer < maxRoutingLayers; layer++)  {
      printf("DEBUG:    Sub-map layer %d:       allowedLayers = %d\n", layer, routeRestrictions->allowedLayers[layer]);
      printf("DEBUG:                       allowedRadiiCells = %.3f\n", routeRestrictions->allowedRadiiCells[layer]);
      printf("DEBUG:                     allowedRadiiMicrons = %.3f\n", routeRestrictions->allowedRadiiMicrons[layer]);
    }
    printf("DEBUG: ------------- Exiting calcGapRoutingRestrictions ----------------\n\n");
  }
  #endif

}  // End of function 'calcGapRoutingRestrictions'


//-----------------------------------------------------------------------------
// Name: detectDiffPairConnections
// Desc: Populate array 'connections' so it contains information for all diff-pair
//       connections across all pseudo-paths. Array will contain enough information
//       from each connection to create sub-maps and run autorouter. This data includes
//       the x/y/z ranges and offsets from the main map, attributes of the start-
//       and end-segments of the connections, diff-pair path numbers, etc.
//
//       This function returns the maximum number of connections found across
//       all pseudo-paths.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_detectDiffPairConnections' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_detectDiffPairConnections 1
#undef DEBUG_detectDiffPairConnections

int detectDiffPairConnections(ShoulderConnections_t * connections, MapInfo_t *mapInfo,
                              InputValues_t *user_inputs, Coordinate_t *pathCoords[],
                              int pathLengths[])  {

  // Variable used for debugging:
  int DEBUG_ON = FALSE;

  #ifdef DEBUG_detectDiffPairConnections
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  if ((mapInfo->current_iteration >= 590) && (mapInfo->current_iteration <= 615))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in detectDiffPairConnections() because specific requirements were met in iteration %d.\n\n",
           mapInfo->current_iteration);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Define variable that will hold the maximum number of connections found in any single
  // pseudo-path. This value will be returned from this function:
  int maxConnectionsPerPath = 0;

  //
  // Iterate over each pseudo-path:
  //
  for (int i = 0; i < mapInfo->numPseudoPaths; i++)  {

    // Calculate the pseudo-path number, which is 'numPaths' + the index 'i':
    int pseudoPath = i + mapInfo->numPaths;

    // Make local copies of the two diff-pair path numbers, as these variables will be used frequently:
    int diff_pair_path_1 = user_inputs->pseudoNetToDiffPair_1[pseudoPath];
    int diff_pair_path_2 = user_inputs->pseudoNetToDiffPair_2[pseudoPath];

    // Trace through the current pseudo-path to count number of pseudo-via stacks:
    int numPseudoViaStacks = 0;  // Number of pseudo-via stacks
    for (int pseudoSegment = 0; pseudoSegment < pathLengths[pseudoPath] - 1; pseudoSegment++)  {

      // Check whether we've reached the beginning of a pseudo-via. This occurs if:
      //  (a) the next segment's Z-value != the current segment's Z-value, and
      //  (b) the current segment is #0, or the current segment's Z-value = previous segment's Z-value
      if (   (pathCoords[pseudoPath][pseudoSegment + 1].Z != pathCoords[pseudoPath][pseudoSegment].Z)
          && (   (pseudoSegment == 0)
              || (pathCoords[pseudoPath][pseudoSegment].Z == pathCoords[pseudoPath][pseudoSegment - 1].Z))) {
        // We got here, so the current pseudo-segment is at the beginning of a pseudo-via.
        // Increment the counter for such vias:
        numPseudoViaStacks++;
      }  // End of if-block for detecting beginning of a pseudo-via stack
    }  // End of for-loop for index 'pseudoSegment'

    #ifdef DEBUG_detectDiffPairConnections
    if (DEBUG_ON)  {
      printf("DEBUG: %d pseudo-via stack(s) were detected in pseudo-path %d in detectDiffPairConnections.\n",
             numPseudoViaStacks, pseudoPath);
    }
    #endif

    // Define variables that are common to all connections in the current pseudo-path:
    connections[i].pseudoPath = pseudoPath;
    connections[i].diffPairPath_1 = diff_pair_path_1;
    connections[i].diffPairPath_2 = diff_pair_path_2;
    connections[i].numPseudoVias = numPseudoViaStacks;
    connections[i].PN_swappable = user_inputs->isPNswappable[pseudoPath];

    // Allocate zero elements for the 'connection' element of the 'shoulderConnections' array. This
    // array of elements contains info for each trace-to-terminal and trace-to-via connection.
    // As connections are found below, the dimension of this array will be expanded with
    // 'realloc' statements.
    connections[i].connection = malloc(0 * sizeof(ShoulderConnection_t));
    if (connections[i].connection == NULL)  {
      printf("\nERROR: Memory was not successfully allocated in function 'detectDiffPairConnections' for array\n");
      printf(  "       'connections[%d].connection'. Please inform the software developer of this fatal error message.\n\n", i);
      exit(1);
    }

    // Check for the rare case in which one of the two diff-pair paths contains only one segment:
    // the end-terminal. In this rare case, we don't attempt to detect any connections, since
    // the imbalance between the two diff-pair paths would cause routing problems.
    if (   ((pathLengths[diff_pair_path_1] == 1) && (pathLengths[diff_pair_path_2] >  1))
        || ((pathLengths[diff_pair_path_1] >  1) && (pathLengths[diff_pair_path_2] == 1)))  {

      #ifdef DEBUG_detectDiffPairConnections
      if (DEBUG_ON)  {
        printf("\nDEBUG: For pseudo-path %d, diff-pair path %d has length %d, and path %d has length %d segments.\n",
               pseudoPath, diff_pair_path_1, pathLengths[diff_pair_path_1], diff_pair_path_2, pathLengths[diff_pair_path_2]);
        printf("DEBUG: Function detectDiffPairConnections will not attempt to detect connections in this imbalanced pair.\n\n");
      }
      #endif

      // Record the number of connections in the 'shouderConnections' structure:
      connections[i].numConnections = 0;

      continue;  // Skip to the next pseudo-path
    }

    //
    // Analyze the pseudoPath to record the connection information between traces and vias, and
    // between traces and terminals. Also define routing restrictions for each of these connections.
    //
    // Begin with the start-terminals:
    int num_connections = 0;  // Keep track of how many pairs of connections were found

    // Allocate memory for this connection
    connections[i].connection = realloc(connections[i].connection, (num_connections + 1) * sizeof(ShoulderConnection_t));
    if (connections[i].connection == NULL)  {
      printf("\nERROR: Memory was not successfully allocated in function 'detectDiffPairConnections' for array\n");
      printf(  "       'connections[%d].connection'. Please inform the software developer of this fatal error message.\n\n", i);
      exit(1);
    }

    // Initialize the Boolean 'DRC_free' flags for this connection to TRUE. They will be
    // changed to FALSE only if the sub-map optimization routing results in intra-pair
    // design-rule violations:
    connections[i].connection[num_connections].DRC_free[NOT_SWAPPED] = TRUE;
    connections[i].connection[num_connections].DRC_free[SWAPPED]     = TRUE;

    // Copy the diff-pair start-terminals' coordinates to the connection structure:
    connections[i].connection[num_connections].startCoord_1 = copyCoordinates(mapInfo->start_cells[diff_pair_path_1]);
    connections[i].connection[num_connections].startCoord_2 = copyCoordinates(mapInfo->start_cells[diff_pair_path_2]);

    // Copy the diff-pairs' first segments' coordinates to the connection structure:
    connections[i].connection[num_connections].endCoord_1 = copyCoordinates(pathCoords[diff_pair_path_1][0]);
    connections[i].connection[num_connections].endCoord_2 = copyCoordinates(pathCoords[diff_pair_path_2][0]);

    // Copy the segment numbers before and after the gap to the connection structure:
    connections[i].connection[num_connections].startSegment_1 = -1;  // '-1' refers to the start-terminal
    connections[i].connection[num_connections].startSegment_2 = -1;  // '-1' refers to the start-terminal
    connections[i].connection[num_connections].endSegment_1   = 0;
    connections[i].connection[num_connections].endSegment_2   = 0;

    // Initialize the path-lengths to zero for the 4 optimized paths between the
    // start- and end-terminals of this connection:
    connections[i].connection[num_connections].optimizedConnectionLength_1[NOT_SWAPPED] = 0;
    connections[i].connection[num_connections].optimizedConnectionLength_1[SWAPPED]     = 0;
    connections[i].connection[num_connections].optimizedConnectionLength_2[NOT_SWAPPED] = 0;
    connections[i].connection[num_connections].optimizedConnectionLength_2[SWAPPED]     = 0;

    #ifdef DEBUG_detectDiffPairConnections
    if (DEBUG_ON)  {
      printf("DEBUG: Found connection between start-terminal and first segment of pseudo-path #%d.\n", pseudoPath);
      printf("DEBUG:              diff_pair_path_1 = %d\n", connections[i].diffPairPath_1);
      printf("DEBUG:   pathLengths[diffPairPath_1] = %d\n", pathLengths[connections[i].diffPairPath_1]);
      printf("DEBUG:                startSegment_1 = %d\n", connections[i].connection[num_connections].startSegment_1);
      printf("DEBUG:                  endSegment_1 = %d\n", connections[i].connection[num_connections].endSegment_1);
      printf("DEBUG:                  startCoord_1 = (%d,%d,%d)\n", connections[i].connection[num_connections].startCoord_1.X,
                                                                    connections[i].connection[num_connections].startCoord_1.Y,
                                                                    connections[i].connection[num_connections].startCoord_1.Z);
      printf("DEBUG:                    endCoord_1 = (%d,%d,%d)\n", connections[i].connection[num_connections].endCoord_1.X,
                                                                    connections[i].connection[num_connections].endCoord_1.Y,
                                                                    connections[i].connection[num_connections].endCoord_1.Z);
      printf("DEBUG:              diff_pair_path_2 = %d\n", connections[i].diffPairPath_2);
      printf("DEBUG:   pathLengths[diffPairPath_2] = %d\n", pathLengths[connections[i].diffPairPath_2]);
      printf("DEBUG:                startSegment_2 = %d\n", connections[i].connection[num_connections].startSegment_2);
      printf("DEBUG:                  endSegment_2 = %d\n", connections[i].connection[num_connections].endSegment_2);
      printf("DEBUG:                  startCoord_2 = (%d,%d,%d)\n", connections[i].connection[num_connections].startCoord_2.X,
                                                                    connections[i].connection[num_connections].startCoord_2.Y,
                                                                    connections[i].connection[num_connections].startCoord_2.Z);
      printf("DEBUG:                    endCoord_2 = (%d,%d,%d)\n", connections[i].connection[num_connections].endCoord_2.X,
                                                                    connections[i].connection[num_connections].endCoord_2.Y,
                                                                    connections[i].connection[num_connections].endCoord_2.Z);
    }
    #endif

    // Define the shape-type of both start-segments to be 'TRACE', since we can't know whether
    // a via will be needed at the start-segments:
    connections[i].connection[num_connections].startShapeType_1 = TRACE;
    connections[i].connection[num_connections].startShapeType_2 = TRACE;

    // Calculate the shape-type of both end-segments:
    int next_Z;  // Routing layer number of next segment in path beyond the end-segment (if it exists):
    if (pathLengths[diff_pair_path_1] > 1)  {
      next_Z = pathCoords[diff_pair_path_1][1].Z;
    }
    else  {
      next_Z = pathCoords[diff_pair_path_1][0].Z;
    }
    if (next_Z < pathCoords[diff_pair_path_1][0].Z)  {
      connections[i].connection[num_connections].endShapeType_1 = VIA_DOWN;
    }
    else if (next_Z > pathCoords[diff_pair_path_1][0].Z)  {
      connections[i].connection[num_connections].endShapeType_1 = VIA_UP;
    }
    else  {
      connections[i].connection[num_connections].endShapeType_1 = TRACE;
    }
    // Repeat for the second diff-pair path:
    if (pathLengths[diff_pair_path_2] > 1)  {
      next_Z = pathCoords[diff_pair_path_2][1].Z;
    }
    else  {
      next_Z = pathCoords[diff_pair_path_2][0].Z;
    }
    if (next_Z < pathCoords[diff_pair_path_2][0].Z)  {
      connections[i].connection[num_connections].endShapeType_2 = VIA_DOWN;
    }
    else if (next_Z > pathCoords[diff_pair_path_2][0].Z)  {
      connections[i].connection[num_connections].endShapeType_2 = VIA_UP;
    }
    else  {
      connections[i].connection[num_connections].endShapeType_2 = TRACE;
    }

    // Increment the num_connections counter:
    num_connections++;

    // Iterate through the segments in pseudoPath to record connection information and define routing
    // restrictions for each trace connection to vias:
    for (int pseudoSegment = 0; pseudoSegment < pathLengths[pseudoPath]; pseudoSegment++)  {

      //
      // Check whether the current segment is in a via-stack that includes the start- or end-terminals.
      // If so, we don't analyze this segment and move on to the next segment:
      //
      // Compare (x,y) coordinates of start-terminal to the (x,y) coordinates of each segment
      // from the current segment back to segment #0. If a mismatch is found, then we've proven
      // that the current segment is not part of a via directly above/below the start-terminal:
      int segmentIsInTerminalVia = TRUE;
      for (int j = pseudoSegment; j >= 0; j--)  {
        if (   (pathCoords[pseudoPath][j].X != mapInfo->start_cells[pseudoPath].X)
            || (pathCoords[pseudoPath][j].Y != mapInfo->start_cells[pseudoPath].Y))  {
          segmentIsInTerminalVia = FALSE;
          break;  // Break out of for-loop for index 'j'
        }
      }
      if (segmentIsInTerminalVia)  {
        // We got here, so the current segment is directly above/below the start-terminal.
        // Skip this pseudo-segment and move on to the next segment.
        continue;
      }

      // Repeat the above calculation for the end-terminal. That is, compare (x,y) coordinates
      // of end-terminal to the (x,y) coordinates of each segment from the current segment
      // forward to the last segment. If a mismatch is found, then we've proven that the
      // current segment is not part of a via directly above/below the end-terminal:
      segmentIsInTerminalVia = TRUE;
      for (int j = pseudoSegment; j < pathLengths[pseudoPath]; j++)  {
        if (   (pathCoords[pseudoPath][j].X != mapInfo->end_cells[pseudoPath].X)
            || (pathCoords[pseudoPath][j].Y != mapInfo->end_cells[pseudoPath].Y))  {
          segmentIsInTerminalVia = FALSE;
          break;  // Break out of for-loop for index 'j'
        }
      }
      if (segmentIsInTerminalVia)  {
        // We got here, so the current segment is directly above/below the end-terminal.
        // Skip this pseudo-segment and move on to the next segment.
        continue;
      }

      //
      // Check if we've reached the beginning of a pseudo-via stack. This condition is met if the
      // following criteria are satisfied:
      //   (a) the next segment (if it exists) is on a different layer, and the previous
      //       segment (if it exists) is on the same layer.
      // OR:
      //   (b) the current segment is segment #0, and the start-terminal is on the same
      //       layer, and the next segment is on a different layer.
      if (   (   (pseudoSegment <= pathLengths[pseudoPath] - 2)                                              // Item (a) above
              && (pathCoords[pseudoPath][pseudoSegment + 1].Z != pathCoords[pseudoPath][pseudoSegment].Z)    // Item (a), continued
              && (pseudoSegment > 0)                                                                         // Item (a), continued
              && (pathCoords[pseudoPath][pseudoSegment - 1].Z == pathCoords[pseudoPath][pseudoSegment].Z))   // Item (a), continued
          || (   (pseudoSegment == 0)                                                                        // Item (b) above
              && (pathCoords[pseudoPath][pseudoSegment].Z == mapInfo->start_cells[pseudoPath].Z)             // Item (b), continued
              && (pathCoords[pseudoPath][pseudoSegment + 1].Z != pathCoords[pseudoPath][pseudoSegment].Z) )) // Item (b), continued
      {

        //
        // We got here, so we're at the gap between traces and the start of vias.
        //
        #ifdef DEBUG_detectDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: Found connection between trace and start of pseudo-via in pseudo-path #%d.\n", pseudoPath);
        }
        #endif

        // To find the diff-pair vias associated with (closest to) the current pseudo-via, we first need
        // to know which routing-layers the pseudo-via started and ended on. So we iterate forward
        // through the pseudo-path until we find the end-layer of the pseudo-via:
        int start_layer = pathCoords[pseudoPath][pseudoSegment].Z;
        int end_layer   = start_layer;
        for (int i = pseudoSegment + 1; i < pathLengths[pseudoPath]; i++)  {
          if (pathCoords[pseudoPath][i].Z == end_layer)  {
            break;
          }
          end_layer = pathCoords[pseudoPath][i].Z;
        }  // End of for-loop for index 'i'

        // We next call function 'findNearbyLayerTransition_wrapper' to locate the diff-pair
        // via-stacks that are closest to the current pseudo-via stack:
        ViaStack_t diff_pair_via_1, diff_pair_via_2;

        #ifdef DEBUG_detectDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: In detectDiffPairConnections, about to call findNearbyLayerTransition_wrapper to find diff-pair via-stacks\n");
          printf(  "DEBUG: near pseudo-via at (%d,%d) to connect trace to start of diff-pair vias:\n",
                 pathCoords[pseudoPath][pseudoSegment].X, pathCoords[pseudoPath][pseudoSegment].Y);
          printf(  "DEBUG:          start_layer = %d\n", start_layer);
          printf(  "DEBUG:            end-layer = %d.\n", end_layer);
          printf(  "DEBUG:     diff_pair_path_1 = %d.\n", diff_pair_path_1);
        }
        #endif

        diff_pair_via_1 = findNearbyLayerTransition_wrapper(diff_pair_path_1, pathLengths, pathCoords, start_layer, end_layer,
                                                            pathCoords[pseudoPath][pseudoSegment].X,
                                                            pathCoords[pseudoPath][pseudoSegment].Y,
                                                            mapInfo, user_inputs);
        #ifdef DEBUG_detectDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: In detectDiffPairConnections, about to call findNearbyLayerTransition_wrapper to find diff-pair via-stacks\n");
          printf(  "DEBUG: near pseudo-via at (%d,%d) to connect trace to start of diff-pair vias:\n",
                 pathCoords[pseudoPath][pseudoSegment].X, pathCoords[pseudoPath][pseudoSegment].Y);
          printf(  "DEBUG:          start_layer = %d\n", start_layer);
          printf(  "DEBUG:            end-layer = %d.\n", end_layer);
          printf(  "DEBUG:     diff_pair_path_2 = %d.\n", diff_pair_path_2);
        }
        #endif
        diff_pair_via_2 = findNearbyLayerTransition_wrapper(diff_pair_path_2, pathLengths, pathCoords, start_layer, end_layer,
                                                            pathCoords[pseudoPath][pseudoSegment].X,
                                                            pathCoords[pseudoPath][pseudoSegment].Y,
                                                            mapInfo, user_inputs);

        #ifdef DEBUG_detectDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: After calling findNearbyLayerTransition_wrapper for both diff-pair vias:\n");
          printf(  "DEBUG:           diff_pair_via_1.error = %d\n", diff_pair_via_1.error);
          printf(  "DEBUG:      diff_pair_via_1.isVertical = %d\n", diff_pair_via_1.isVertical);
          printf(  "DEBUG:    diff_pair_via_1.startSegment = %d (%d,%d,%d)\n", diff_pair_via_1.startSegment,
                   diff_pair_via_1.startCoord.X, diff_pair_via_1.startCoord.Y, diff_pair_via_1.startCoord.Z);
          printf(  "DEBUG:      diff_pair_via_1.endSegment = %d (%d,%d,%d)\n", diff_pair_via_1.endSegment,
                   diff_pair_via_1.endCoord.X, diff_pair_via_1.endCoord.Y, diff_pair_via_1.endCoord.Z);
          printf(  "DEBUG:           diff_pair_via_2.error = %d\n", diff_pair_via_2.error);
          printf(  "DEBUG:      diff_pair_via_2.isVertical = %d\n", diff_pair_via_2.isVertical);
          printf(  "DEBUG:    diff_pair_via_2.startSegment = %d (%d,%d,%d)\n", diff_pair_via_2.startSegment,
                   diff_pair_via_2.startCoord.X, diff_pair_via_2.startCoord.Y, diff_pair_via_2.startCoord.Z);
          printf(  "DEBUG:      diff_pair_via_2.endSegment = %d (%d,%d,%d)\n", diff_pair_via_2.endSegment,
                   diff_pair_via_2.endCoord.X, diff_pair_via_2.endCoord.Y, diff_pair_via_2.endCoord.Z);
        }
        #endif

        //
        // Confirm that:
        //   (a) diff-pair vias were indeed found, and
        //   (b) that both are vertically stacked vias, and
        //   (c) that neither diff-pair via starts at segment #-1 or #0, since these
        //       segments are already part of the terminal-to-trace connection, and
        //   (d) that the start-segments of both diff-pair vias are greater than the end-
        //       segments of the previous connection.
        if (   (! diff_pair_via_1.error)          && (! diff_pair_via_2.error)            // Item (a) above
            && (  diff_pair_via_1.isVertical)     && (  diff_pair_via_2.isVertical)       // Item (b) above
            && (diff_pair_via_1.startSegment > 0) && (diff_pair_via_2.startSegment > 0)   // Item (c) above
            && (diff_pair_via_1.startSegment > connections[i].connection[num_connections - 1].endSegment_1)  // Item (d) above
            && (diff_pair_via_2.startSegment > connections[i].connection[num_connections - 1].endSegment_2)) // Item (d), continued
        {

          // Allocate memory for this connection
          connections[i].connection = realloc(connections[i].connection, (num_connections + 1) * sizeof(ShoulderConnection_t));
          if (connections[i].connection == NULL)  {
            printf("\nERROR: Memory was not successfully allocated in function 'detectDiffPairConnections' for array\n");
            printf(  "       'connections[%d].connection'. Please inform the software developer of this fatal error message.\n\n", i);
            exit(1);
          }

          #ifdef DEBUG_detectDiffPairConnections
          if (DEBUG_ON)  {
            printf("DEBUG: About to copyCoordinates from segment before via to #%d 'connection' structure:\n", num_connections);
            printf("DEBUG:                   diff_pair_path_1 = %d\n", diff_pair_path_1);
            printf("DEBUG:   diff_pair_via_1.startSegment - 1 = %d\n", diff_pair_via_1.startSegment - 1);
            if (diff_pair_via_1.startSegment == 0)  {
              printf("DEBUG:                 copied coordinates = (%d,%d,%d) (start terminals)\n", mapInfo->start_cells[diff_pair_path_1].X,
                     mapInfo->start_cells[diff_pair_path_1].Y, mapInfo->start_cells[diff_pair_path_1].Z);
            }
            else  {
              printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment - 1].X,
                                                                                 pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment - 1].Y,
                                                                                 pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment - 1].Z);
            }
            printf("DEBUG:                   diff_pair_path_2 = %d\n", diff_pair_path_2);
            printf("DEBUG:   diff_pair_via_2.startSegment - 1 = %d\n", diff_pair_via_2.startSegment - 1);
            if (diff_pair_via_2.startSegment == 0)  {
              printf("DEBUG:                 copied coordinates = (%d,%d,%d) (start terminals)\n", mapInfo->start_cells[diff_pair_path_2].X,
                     mapInfo->start_cells[diff_pair_path_2].Y, mapInfo->start_cells[diff_pair_path_2].Z);
            }
            else  {
              printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment - 1].X,
                                                                                 pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment - 1].Y,
                                                                                 pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment - 1].Z);
            }
          }  // End of if-block for DEBUG_ON
          #endif

          // Initialize the Boolean 'DRC_free' flags for this connection to TRUE. They will be
          // changed to FALSE only if the sub-map optimization routing results in intra-pair
          // design-rule violations:
          connections[i].connection[num_connections].DRC_free[NOT_SWAPPED] = TRUE;
          connections[i].connection[num_connections].DRC_free[SWAPPED]     = TRUE;

          // Copy the coordinates of the diff-pair segments immediately before the diff-pair via to the connection
          // structure. This segment corresponds to the last TRACE segment before the start of the diff-pair via. If
          // the via's start-segment is zero, then the preceding segment is the start-terminal:
          if (diff_pair_via_1.startSegment == 0)  {
            connections[i].connection[num_connections].startCoord_1 = copyCoordinates(mapInfo->start_cells[diff_pair_path_1]);
          }
          else  {
            connections[i].connection[num_connections].startCoord_1 = copyCoordinates(pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment - 1]);
          }

          if (diff_pair_via_2.startSegment == 0)  {
            connections[i].connection[num_connections].startCoord_2 = copyCoordinates(mapInfo->start_cells[diff_pair_path_2]);
          }
          else  {
            connections[i].connection[num_connections].startCoord_2 = copyCoordinates(pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment - 1]);
          }

          #ifdef DEBUG_detectDiffPairConnections
          if (DEBUG_ON)  {
            printf("DEBUG: About to copyCoordinates from segment at start of via to #%d 'connection' structure:\n", num_connections);
            printf("DEBUG:                   diff_pair_path_1 = %d\n", diff_pair_path_1);
            printf("DEBUG:       diff_pair_via_1.startSegment = %d\n", diff_pair_via_1.startSegment);
            printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment].X,
                                                                               pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment].Y,
                                                                               pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment].Z);
            printf("DEBUG:                   diff_pair_path_2 = %d\n", diff_pair_path_2);
            printf("DEBUG:       diff_pair_via_2.startSegment = %d\n", diff_pair_via_2.startSegment);
            printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment].X,
                                                                               pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment].Y,
                                                                               pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment].Z);
          }  // End of if-block for DEBUG_ON
          #endif

          // Copy the coordinates of the diff-pair segments at the start of the diff-pair via to the connection
          // structure. This segment corresponds to the first diff-pair via segment:
          connections[i].connection[num_connections].endCoord_1 = copyCoordinates(pathCoords[diff_pair_path_1][diff_pair_via_1.startSegment]);
          connections[i].connection[num_connections].endCoord_2 = copyCoordinates(pathCoords[diff_pair_path_2][diff_pair_via_2.startSegment]);

          // Copy the segment numbers before and after the gap to the connection structure:
          connections[i].connection[num_connections].startSegment_1 = diff_pair_via_1.startSegment - 1;
          connections[i].connection[num_connections].startSegment_2 = diff_pair_via_2.startSegment - 1;
          connections[i].connection[num_connections].endSegment_1   = diff_pair_via_1.startSegment;
          connections[i].connection[num_connections].endSegment_2   = diff_pair_via_2.startSegment;

          // Calculate the shape-types of the start- and end-segments of both diff-pair paths:
          {
            // Begin with the start-segment of diff-pair path #1:
            int prev_Z;
            if (connections[i].connection[num_connections].startSegment_1 == 0)  {
              prev_Z = mapInfo->start_cells[diff_pair_path_1].Z;
            }
            else  {
              prev_Z = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].startSegment_1 - 1].Z;
            }
            int Z = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].startSegment_1].Z;
            if (Z > prev_Z)  {
              connections[i].connection[num_connections].startShapeType_1 = VIA_DOWN;
            }
            else if (Z < prev_Z)  {
              connections[i].connection[num_connections].startShapeType_1 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].startShapeType_1 = TRACE;
            }

            // Now repeat for the start-segment of diff-pair path #2:
            if (connections[i].connection[num_connections].startSegment_2 == 0)  {
              prev_Z = mapInfo->start_cells[diff_pair_path_2].Z;
            }
            else  {
              prev_Z = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].startSegment_2 - 1].Z;
            }
            Z = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].startSegment_2].Z;
            if (Z > prev_Z)  {
              connections[i].connection[num_connections].startShapeType_2 = VIA_DOWN;
            }
            else if (Z < prev_Z)  {
              connections[i].connection[num_connections].startShapeType_2 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].startShapeType_2 = TRACE;
            }

            // Now we calculate the shape-types of the end-segment of diff-pair path #1:
            Z      = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].endSegment_1].Z;
            next_Z = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].endSegment_1 + 1].Z;
            if (Z > next_Z)  {
              connections[i].connection[num_connections].endShapeType_1 = VIA_DOWN;
            }
            else if (Z < next_Z)  {
              connections[i].connection[num_connections].endShapeType_1 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].endShapeType_1 = TRACE;
            }

            // Finally, we calculate the shape-types of the end-segment of diff-pair path #2:
            Z      = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].endSegment_2].Z;
            next_Z = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].endSegment_2 + 1].Z;
            if (Z > next_Z)  {
              connections[i].connection[num_connections].endShapeType_2 = VIA_DOWN;
            }
            else if (Z < next_Z)  {
              connections[i].connection[num_connections].endShapeType_2 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].endShapeType_2 = TRACE;
            }
          }  // End of block for calculating shape-types of both start-segments

          // Initialize the path-lengths to zero for the 4 optimized paths between the
          // start- and end-terminals of this connection:
          connections[i].connection[num_connections].optimizedConnectionLength_1[NOT_SWAPPED] = 0;
          connections[i].connection[num_connections].optimizedConnectionLength_1[SWAPPED]     = 0;
          connections[i].connection[num_connections].optimizedConnectionLength_2[NOT_SWAPPED] = 0;
          connections[i].connection[num_connections].optimizedConnectionLength_2[SWAPPED]     = 0;

          // Increment the num_connections counter:
          num_connections++;

        }  // End of if-block for successfully finding both diff-pair vias
        else  {
          // We got here, which means that findNearbyLayerTransition_wrapper() failed to find vertically
          // stacked vias for one or both diff-pairs. Issue a warning message:
          printf("\nWARNING: In function 'detectDiffPairConnections', calls to function 'findNearbyLayerTransition_wrapper()\n");
          printf(  "         were not successful in locating vertically stacked diff-pair vias that were not too close\n");
          printf(  "         to the start-terminals:\n");
          printf(  "           Pseudo-path number: %d\n", pseudoPath);
          printf(  "           Pseudo-via coordinates: (%d, %d) in cell units\n", pathCoords[pseudoPath][pseudoSegment].X,
                    pathCoords[pseudoPath][pseudoSegment].Y);
          printf(  "           Pseudo-via's start-layer number: %d\n", start_layer);
          printf(  "           Pseudo-via's end-layer number: %d\n", end_layer);
          printf(  "           Diff-pair path numbers: %d and %d\n", diff_pair_path_1, diff_pair_path_2);
          printf("\n");
        }  // End of else-block for failing to find diff-pair vias

      }  // End of else/if-block for reaching the beginning of a pseudo-via stack

      // Check if we've reached the end of a pseudo-via stack which is not at the end-terminal.
      // This condition is met if the following criteria are satisfied:
      //   (a) the current segment is not the end-terminal segment, and
      //   (b) the next segment is on the same layer, and
      //   (c) the previous segment (if it exists) is on a different layer.
      else if (   (pseudoSegment != pathLengths[pseudoPath] - 1)                                            // Item (a) above
               && (pathCoords[pseudoPath][pseudoSegment + 1].Z == pathCoords[pseudoPath][pseudoSegment].Z)  // Item (b) above
               && (pseudoSegment > 0)                                                                       // Item (c) above
               && (pathCoords[pseudoPath][pseudoSegment - 1].Z != pathCoords[pseudoPath][pseudoSegment].Z)) // Item (c), continued
      {

        //
        // We got here, so we're at the gap between the end of vias and traces.
        //

        #ifdef DEBUG_detectDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: Found connection between end of pseudo-via and trace in pseudo-path #%d.\n", pseudoPath);
        }
        #endif

        // To find the diff-pair vias associated with (closest to) the current pseudo-via, we first need
        // to know which routing-layers the pseudo-via started and ended on. So we iterate backward
        // through the pseudo-path until we find the start-layer of the pseudo-via:
        int end_layer   = pathCoords[pseudoPath][pseudoSegment].Z;
        int start_layer = end_layer;
        for (int i = pseudoSegment - 1; i >= 0; i--)  {
          if (pathCoords[pseudoPath][i].Z == start_layer)  {
            break;
          }
          start_layer = pathCoords[pseudoPath][i].Z;
        }  // End of for-loop for index 'i'

        // We next call function 'findNearbyLayerTransition_wrapper' to locate the diff-pair
        // via-stacks that are closest to the current pseudo-via stack:
        ViaStack_t diff_pair_via_1, diff_pair_via_2;

        #ifdef DEBUG_detectDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: In detectDiffPairConnections, about to call findNearbyLayerTransition_wrapper to find diff-pair via-stacks\n");
          printf(  "DEBUG: near pseudo-via at (%d,%d) to connect trace to end of diff-pair vias:\n",
                 pathCoords[pseudoPath][pseudoSegment].X, pathCoords[pseudoPath][pseudoSegment].Y);
          printf(  "DEBUG:          start_layer = %d\n", start_layer);
          printf(  "DEBUG:            end-layer = %d.\n", end_layer);
          printf(  "DEBUG:     diff_pair_path_1 = %d.\n", diff_pair_path_1);
        }
        #endif

        diff_pair_via_1 = findNearbyLayerTransition_wrapper(diff_pair_path_1, pathLengths, pathCoords, start_layer, end_layer,
                                                            pathCoords[pseudoPath][pseudoSegment].X,
                                                            pathCoords[pseudoPath][pseudoSegment].Y,
                                                            mapInfo, user_inputs);

        #ifdef DEBUG_detectDiffPairConnections
        if (DEBUG_ON)  {
          printf("\nDEBUG: In detectDiffPairConnections, about to call findNearbyLayerTransition_wrapper to find diff-pair via-stacks\n");
          printf(  "DEBUG: near pseudo-via at (%d,%d) to connect trace to end of diff-pair vias:\n",
                 pathCoords[pseudoPath][pseudoSegment].X, pathCoords[pseudoPath][pseudoSegment].Y);
          printf(  "DEBUG:          start_layer = %d\n", start_layer);
          printf(  "DEBUG:            end-layer = %d.\n", end_layer);
          printf(  "DEBUG:     diff_pair_path_2 = %d.\n", diff_pair_path_2);
        }
        #endif

        diff_pair_via_2 = findNearbyLayerTransition_wrapper(diff_pair_path_2, pathLengths, pathCoords, start_layer, end_layer,
                                                            pathCoords[pseudoPath][pseudoSegment].X,
                                                            pathCoords[pseudoPath][pseudoSegment].Y,
                                                            mapInfo, user_inputs);

        //
        // Confirm that diff-pair vias:
        //   (a) were indeed found, and
        //   (b) that both are vertically stacked, and
        //   (c) that the start-segments of both vias match the end-segments of the previous
        //       diff-pair connection
        //   (d) that neither diff-pair via ends at the path's last segment (end-terminal) or the
        //       second-to-last segment, since the second-to-last segment will be part of the
        //       trace-to-end-terminal connection.
        //
        if (   (! diff_pair_via_1.error)      && (! diff_pair_via_2.error)                                   // Item (a) above
            && (  diff_pair_via_1.isVertical) && (  diff_pair_via_2.isVertical)                              // Item (b) above
            && (diff_pair_via_1.startSegment == connections[i].connection[num_connections - 1].endSegment_1) // Item (c) above
            && (diff_pair_via_2.startSegment == connections[i].connection[num_connections - 1].endSegment_2) // Item (c), continued
            && (diff_pair_via_1.endSegment < pathLengths[diff_pair_path_1] - 2)                              // Item (d) above
            && (diff_pair_via_2.endSegment < pathLengths[diff_pair_path_2] - 2) )  {                         // Item (d), continued

          // We got here, so diff-pair vias were found for both diff-pair paths.

          // Allocate memory for this connection
          connections[i].connection = realloc(connections[i].connection, (num_connections + 1) * sizeof(ShoulderConnection_t));
          if (connections[i].connection == NULL)  {
            printf("\nERROR: Memory was not successfully allocated in function 'detectDiffPairConnections' for array\n");
            printf(  "       'connections[%d].connection'. Please inform the software developer of this fatal error message.\n\n", i);
            exit(1);
          }

          #ifdef DEBUG_detectDiffPairConnections
          if (DEBUG_ON)  {
            printf("DEBUG: About to copyCoordinates from segment at end of via to #%d 'connection' structure:\n", num_connections);
            printf("DEBUG:                   diff_pair_path_1 = %d\n", diff_pair_path_1);
            printf("DEBUG:         diff_pair_via_1.endSegment = %d\n", diff_pair_via_1.endSegment);
            printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment].X,
                                                                               pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment].Y,
                                                                               pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment].Z);
            printf("DEBUG:                   diff_pair_path_2 = %d\n", diff_pair_path_2);
            printf("DEBUG:         diff_pair_via_2.endSegment = %d\n", diff_pair_via_2.endSegment);
            printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment].X,
                                                                               pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment].Y,
                                                                               pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment].Z);
          }  // End of if-block for DEBUG_ON
          #endif

          // Initialize the Boolean 'DRC_free' flags for this connection to TRUE. They will be
          // changed to FALSE only if the sub-map optimization routing results in intra-pair
          // design-rule violations:
          connections[i].connection[num_connections].DRC_free[NOT_SWAPPED] = TRUE;
          connections[i].connection[num_connections].DRC_free[SWAPPED]     = TRUE;

          // Copy the coordinates of the diff-pair segments at the end of the diff-pair via to the connection
          // structure. This segment corresponds to the last via segment:
          connections[i].connection[num_connections].startCoord_1 = copyCoordinates(pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment]);
          connections[i].connection[num_connections].startCoord_2 = copyCoordinates(pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment]);

          #ifdef DEBUG_detectDiffPairConnections
          if (DEBUG_ON)  {
            printf("DEBUG: About to copyCoordinates from segment just after the end of via to #%d 'connection' structure:\n", num_connections);
            printf("DEBUG:                   diff_pair_path_1 = %d\n", diff_pair_path_1);
            printf("DEBUG:     diff_pair_via_1.endSegment + 1 = %d\n", diff_pair_via_1.endSegment + 1);
            printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment + 1].X,
                                                                               pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment + 1].Y,
                                                                               pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment + 1].Z);
            printf("DEBUG:                   diff_pair_path_2 = %d\n", diff_pair_path_2);
            printf("DEBUG:     diff_pair_via_2.endSegment + 1 = %d\n", diff_pair_via_2.endSegment + 1);
            printf("DEBUG:                 copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment + 1].X,
                                                                               pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment + 1].Y,
                                                                               pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment + 1].Z);
          }  // End of if-block for DEBUG_ON
          #endif

          // Confirm that the 'endSegment + 1' index does not exceed the length of the path:
          if (   (diff_pair_via_1.endSegment >= pathLengths[diff_pair_path_1])
              || (diff_pair_via_2.endSegment >= pathLengths[diff_pair_path_2]))   {
            printf("\nERROR: An unexpected condition occurred in function 'detectDiffPairConnections',\n");
            printf(  "       in which the last segment of a diff-pair via was at the end-terminal of\n");
            printf(  "       the diff-pair net. Please inform the software developer of this fatal error\n");
            printf(  "       message:\n");
            printf(  "         Diff-pair path #%d: diff_pair_via_1.endSegment = %d (path length is %d)\n",
                     diff_pair_path_1, diff_pair_via_1.endSegment, pathLengths[diff_pair_path_1]);
            printf(  "         Diff-pair path #%d: diff_pair_via_1.endSegment = %d (path length is %d)\n\n",
                     diff_pair_path_2, diff_pair_via_2.endSegment, pathLengths[diff_pair_path_2]);
            exit(1);
          }

          // Copy the coordinates of the diff-pair segments just after the via to the connection
          // structure. This segment corresponds to the first segment in the TRACE after the via:
          connections[i].connection[num_connections].endCoord_1 = copyCoordinates(pathCoords[diff_pair_path_1][diff_pair_via_1.endSegment + 1]);
          connections[i].connection[num_connections].endCoord_2 = copyCoordinates(pathCoords[diff_pair_path_2][diff_pair_via_2.endSegment + 1]);

          // Copy the segment numbers before and after the gap to the connection structure:
          connections[i].connection[num_connections].startSegment_1 = diff_pair_via_1.endSegment;
          connections[i].connection[num_connections].startSegment_2 = diff_pair_via_2.endSegment;
          connections[i].connection[num_connections].endSegment_1   = diff_pair_via_1.endSegment + 1;
          connections[i].connection[num_connections].endSegment_2   = diff_pair_via_2.endSegment + 1;

          // Calculate the shape-types of the start- and end-segments of both diff-pair paths:
          {
            // Begin with the start-segment of diff-pair path #1:
            int prev_Z = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].startSegment_1 - 1].Z;
            int Z      = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].startSegment_1].Z;
            if (Z > prev_Z)  {
              connections[i].connection[num_connections].startShapeType_1 = VIA_DOWN;
            }
            else if (Z < prev_Z)  {
              connections[i].connection[num_connections].startShapeType_1 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].startShapeType_1 = TRACE;
            }

            // Now repeat for the start-segment of diff-pair path #2:
            prev_Z = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].startSegment_2 - 1].Z;
            Z      = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].startSegment_2].Z;
            if (Z > prev_Z)  {
              connections[i].connection[num_connections].startShapeType_2 = VIA_DOWN;
            }
            else if (Z < prev_Z)  {
              connections[i].connection[num_connections].startShapeType_2 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].startShapeType_2 = TRACE;
            }

            // Now we calculate the shape-types of the end-segment of diff-pair path #1:
            Z      = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].endSegment_1].Z;
            next_Z = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].endSegment_1 + 1].Z;
            if (Z > next_Z)  {
              connections[i].connection[num_connections].endShapeType_1 = VIA_DOWN;
            }
            else if (Z < next_Z)  {
              connections[i].connection[num_connections].endShapeType_1 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].endShapeType_1 = TRACE;
            }

            // Finally, we calculate the shape-types of the end-segment of diff-pair path #2:
            Z      = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].endSegment_2].Z;
            next_Z = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].endSegment_2 + 1].Z;
            if (Z > next_Z)  {
              connections[i].connection[num_connections].endShapeType_2 = VIA_DOWN;
            }
            else if (Z < next_Z)  {
              connections[i].connection[num_connections].endShapeType_2 = VIA_UP;
            }
            else  {
              connections[i].connection[num_connections].endShapeType_2 = TRACE;
            }
          }  // End of block for calculating shape-types of both start-segments

          // Initialize the path-lengths to zero for the 4 optimized paths between the
          // start- and end-terminals of this connection:
          connections[i].connection[num_connections].optimizedConnectionLength_1[NOT_SWAPPED] = 0;
          connections[i].connection[num_connections].optimizedConnectionLength_1[SWAPPED]     = 0;
          connections[i].connection[num_connections].optimizedConnectionLength_2[NOT_SWAPPED] = 0;
          connections[i].connection[num_connections].optimizedConnectionLength_2[SWAPPED]     = 0;

          // Increment the num_connections counter:
          num_connections++;

        }  // End of if-block for successfully finding both diff-pair vias
        else  {
          // We got here, which means that findNearbyLayerTransition_wrapper() failed to find vertically
          // stacked vias for both diff-pair paths. Issue a warning message:
          printf("\nWARNING: In function 'detectDiffPairConnections', calls to function 'findNearbyLayerTransition_wrapper()\n");
          printf(  "         were not successful in locating vertically stacked diff-pair vias that were not too close\n");
          printf(  "         to the end-terminals:\n");
          printf(  "           Pseudo-path number: %d\n", pseudoPath);
          printf(  "           Pseudo-via coordinates: (%d, %d) in cell units\n", pathCoords[pseudoPath][pseudoSegment].X,
                    pathCoords[pseudoPath][pseudoSegment].Y);
          printf(  "           Pseudo-via's start-layer number: %d\n", start_layer);
          printf(  "           Pseudo-via's end-layer number: %d\n", end_layer);
          printf(  "           Diff-pair path numbers: %d and %d\n", diff_pair_path_1, diff_pair_path_2);
          printf("\n");
        }  // End of else-block for failing to find diff-pair vias

      }  // End of else/if-block for reaching the beginning of a pseudo-via stack

    }  // End of for-loop for index 'pseudoSegment' (0 to pathLengths[pseudoPath])


    // We just iterated through the pseudoPath and found all the connections between diff-pair
    // traces and diff-pair vias. If the diff-pair paths' end-terminals are not already part
    // of the most recent connection (a rare occurrence), then we add the connection between
    // the diff-pair traces and their end-terminals:
    if (   (connections[i].connection[num_connections - 1].endSegment_1 != pathLengths[diff_pair_path_1] - 1)
        && (connections[i].connection[num_connections - 1].endSegment_2 != pathLengths[diff_pair_path_2] - 1))  {

      // We got here, so the end-segments of the last connection do *not* match the end-terminals of
      // the diff-pair paths. So we need to add a connection from these end-terminals to the preceding
      // segments of each diff-pair path:

      // Allocate memory for this connection
      connections[i].connection = realloc(connections[i].connection, (num_connections + 1) * sizeof(ShoulderConnection_t));
      if (connections[i].connection == NULL)  {
        printf("\nERROR: Memory was not successfully allocated in function 'detectDiffPairConnections' for array\n");
        printf(  "       'connections[%d].connection'. Please inform the software developer of this fatal error message.\n\n", i);
        exit(1);
      }

      #ifdef DEBUG_detectDiffPairConnections
      if (DEBUG_ON)  {
        printf("DEBUG: Found connection at end-terminal of pseudo-path #%d.\n", pseudoPath);

        printf("DEBUG: About to copyCoordinates from segment before the end-terminal to #%d 'connection' structure:\n", num_connections);
        printf("DEBUG:                    diff_pair_path_1 = %d\n", diff_pair_path_1);
        printf("DEBUG:   pathLengths[diff_pair_path_1] - 2 = %d\n", pathLengths[diff_pair_path_1] - 2);
        printf("DEBUG:                  copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 2].X,
                                                                            pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 2].Y,
                                                                            pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 2].Z);
        printf("DEBUG:                    diff_pair_path_2 = %d\n", diff_pair_path_2);
        printf("DEBUG:   pathLengths[diff_pair_path_2] - 2 = %d\n", pathLengths[diff_pair_path_2] - 2);
        printf("DEBUG:                  copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 2].X,
                                                                            pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 2].Y,
                                                                            pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 2].Z);
      }  // End of if-block for DEBUG_ON
      #endif

      // Initialize the Boolean 'DRC_free' flags for this connection to TRUE. They will be
      // changed to FALSE only if the sub-map optimization routing results in intra-pair
      // design-rule violations:
      connections[i].connection[num_connections].DRC_free[NOT_SWAPPED] = TRUE;
      connections[i].connection[num_connections].DRC_free[SWAPPED]     = TRUE;

      // Copy the diff-pairs' second-to-last segments' coordinates to the connection structure:
      connections[i].connection[num_connections].startCoord_1 = copyCoordinates(pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 2]);
      connections[i].connection[num_connections].startCoord_2 = copyCoordinates(pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 2]);

      #ifdef DEBUG_detectDiffPairConnections
      if (DEBUG_ON)  {
        printf("DEBUG: About to copyCoordinates from segment at the end-terminal to #%d 'connection' structure:\n", num_connections);
        printf("DEBUG:                    diff_pair_path_1 = %d\n", diff_pair_path_1);
        printf("DEBUG:   pathLengths[diff_pair_path_1] - 1 = %d\n", pathLengths[diff_pair_path_1] - 1);
        printf("DEBUG:                  copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 1].X,
                                                                            pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 1].Y,
                                                                            pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 1].Z);
        printf("DEBUG:                    diff_pair_path_2 = %d\n", diff_pair_path_2);
        printf("DEBUG:   pathLengths[diff_pair_path_2] - 1 = %d\n", pathLengths[diff_pair_path_2] - 1);
        printf("DEBUG:                  copied coordinates = (%d,%d,%d)\n", pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 1].X,
                                                                            pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 1].Y,
                                                                            pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 1].Z);
      }  // End of if-block for DEBUG_ON
      #endif

      // Copy the diff-pairs' last segments' coordinates to the connection structure:
      connections[i].connection[num_connections].endCoord_1 = copyCoordinates(pathCoords[diff_pair_path_1][pathLengths[diff_pair_path_1] - 1]);
      connections[i].connection[num_connections].endCoord_2 = copyCoordinates(pathCoords[diff_pair_path_2][pathLengths[diff_pair_path_2] - 1]);

      // Copy the segment numbers before and after the gap to the connection structure:
      connections[i].connection[num_connections].startSegment_1 = pathLengths[diff_pair_path_1] - 2;
      connections[i].connection[num_connections].startSegment_2 = pathLengths[diff_pair_path_2] - 2;
      connections[i].connection[num_connections].endSegment_1   = pathLengths[diff_pair_path_1] - 1;
      connections[i].connection[num_connections].endSegment_2   = pathLengths[diff_pair_path_2] - 1;

      // Calculate the shape-type of both start- and end-segments:
      {
        // Begin with the start-segment of diff-pair path #1:
        int prev_Z = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].startSegment_1 - 1].Z;
        int Z      = pathCoords[diff_pair_path_1][connections[i].connection[num_connections].startSegment_1].Z;
        if (Z > prev_Z)  {
          connections[i].connection[num_connections].startShapeType_1 = VIA_DOWN;
        }
        else if (Z < prev_Z)  {
          connections[i].connection[num_connections].startShapeType_1 = VIA_UP;
        }
        else  {
          connections[i].connection[num_connections].startShapeType_1 = TRACE;
        }

        // Now repeat for the start-segment of diff-pair path #2:
        prev_Z = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].startSegment_2 - 1].Z;
        Z      = pathCoords[diff_pair_path_2][connections[i].connection[num_connections].startSegment_2].Z;
        if (Z > prev_Z)  {
          connections[i].connection[num_connections].startShapeType_2 = VIA_DOWN;
        }
        else if (Z < prev_Z)  {
          connections[i].connection[num_connections].startShapeType_2 = VIA_UP;
        }
        else  {
          connections[i].connection[num_connections].startShapeType_2 = TRACE;
        }

        // Define the shape-type of both end-segments to be TRACE, since we can't know
        // whether a via will be placed at the end-terminals:
        connections[i].connection[num_connections].endShapeType_1 = TRACE;
        connections[i].connection[num_connections].endShapeType_2 = TRACE;

      }  // End of block for calculating shape-types of start- and end-segments

      // Initialize the path-lengths to zero for the 4 optimized paths between the
      // start- and end-terminals of this connection:
      connections[i].connection[num_connections].optimizedConnectionLength_1[NOT_SWAPPED] = 0;
      connections[i].connection[num_connections].optimizedConnectionLength_1[SWAPPED]     = 0;
      connections[i].connection[num_connections].optimizedConnectionLength_2[NOT_SWAPPED] = 0;
      connections[i].connection[num_connections].optimizedConnectionLength_2[SWAPPED]     = 0;

      // Increment the num_connections counter:
      num_connections++;

    }  // End of if-block for end-segments of last connection not matching the end-terminals of the diff-pair paths


    // Record the number of connections in the 'shouderConnections' structure:
    connections[i].numConnections = num_connections;

    #ifdef DEBUG_detectDiffPairConnections
    if (DEBUG_ON)  {
      printf("\nDEBUG: %d valid connections were found associated with pseudo-path #%d\n", num_connections, pseudoPath);
    }
    #endif

    // if the current pseudo-path contained more connections than the current value
    // of maxConnectionsPerPath, then update maxConnectionsPerPath:
    if (num_connections > maxConnectionsPerPath)  {
      maxConnectionsPerPath = num_connections;
    }

  }  // End of for-loop for index 'i' (0 to numPseudoPaths)

  //
  // Perform error-checking on the connections, just in case the algorithm misbehaved.
  // Also populate the Boolean flag 'sameLayerTerminals', which is TRUE if (a) the
  // connection's two start-terminals are on the same layer, and (b) the connection's
  // two end-terminals are on the same layer.
  //
  int error_found = FALSE;
  for (int i = 0; i < mapInfo->numPseudoPaths; i++)  {
    #ifdef DEBUG_detectDiffPairConnections
    if (DEBUG_ON)  {
      printf("DEBUG: Error-checking %d diff-pair connections in pseudo-path %d...\n",
             connections[i].numConnections, connections[i].pseudoPath);
    }
    #endif
    for (int j = 0; j < connections[i].numConnections; j++)  {

      #ifdef DEBUG_detectDiffPairConnections
      if (DEBUG_ON)  {
        printf("DEBUG: Error-checking connection %d of in pseudo-path %d...\n",
               j, connections[i].pseudoPath);
      }
      #endif

      // Confirm that each connection's endSegment is larger than the
      // same connection's startSegment
      if (   (connections[i].connection[j].endSegment_1 <= connections[i].connection[j].startSegment_1)
          || (connections[i].connection[j].endSegment_2 <= connections[i].connection[j].startSegment_2))  {
        error_found = TRUE;
      }

      // Confirm that each connection's startSegment and endSegment fall within the allowed
      // ranges for that diff-pair net:
      if (   (connections[i].connection[j].startSegment_1 < -1)
          || (connections[i].connection[j].startSegment_2 < -1)
          || (connections[i].connection[j].startSegment_1 > pathLengths[connections[i].diffPairPath_1] - 2)
          || (connections[i].connection[j].startSegment_2 > pathLengths[connections[i].diffPairPath_2] - 2)
          || (connections[i].connection[j].endSegment_1   < 0)
          || (connections[i].connection[j].endSegment_2   < 0)
          || (connections[i].connection[j].endSegment_1   > pathLengths[connections[i].diffPairPath_1] - 1)
          || (connections[i].connection[j].endSegment_2   > pathLengths[connections[i].diffPairPath_2] - 1))  {
        error_found = TRUE;
      }

      // Perform checks that apply to all but the 0th connection:
      if (j > 0)  {
        // Except for the 0th connection, confirm that all 'startSegment_*' values are > -1.
        if (   (connections[i].connection[j].startSegment_1 < 0)
            || (connections[i].connection[j].startSegment_2 < 0))  {
          error_found = TRUE;
        }

        // Confirm that each connection's startSegment is larger than (or equal to) the
        // previous connection's endSegment
        if (   (connections[i].connection[j].startSegment_1 < connections[i].connection[j-1].endSegment_1)
            || (connections[i].connection[j].startSegment_2 < connections[i].connection[j-1].endSegment_2))  {
          error_found = TRUE;
        }
      }  // End of if-block for (j > 0)

      // Populate the 'sameLayerTerminals' element for connection j:
      if (   (connections[i].connection[j].startCoord_1.Z == connections[i].connection[j].startCoord_2.Z)
          && (connections[i].connection[j].endCoord_1.Z   == connections[i].connection[j].endCoord_2.Z))  {
        connections[i].connection[j].sameLayerTerminals = TRUE;
      }
      else  {
        connections[i].connection[j].sameLayerTerminals = FALSE;
      }

    }  // End of for-loop for index 'j'
  }  // End of for-loop for index 'i'

  // If an error was found, issue a fatal error message:
  if (error_found)  {
    printf("\nERROR: An unexpected condition was detected in function 'detectDiffPairConnections'. Please inform the\n");
    printf(  "       software developer of this fatal error message. Additional information is listed below.\n\n");
  }

  // Print out details of each connection if (a) an error was found, or (b) if DEBUG_ON is TRUE:
  if (error_found || DEBUG_ON)  {
    printf("\nDEBUG: Summary of connections detected in detectDiffPairConnections:\n");
    for (int i = 0; i < user_inputs->num_pseudo_nets; i++)  {
      printf("\nDEBUG: %d connections found for pseudo-path #%d (diff-pair paths %d and %d):\n",
             connections[i].numConnections, connections[i].pseudoPath,
             connections[i].diffPairPath_1, connections[i].diffPairPath_2);
      for (int j = 0; j < connections[i].numConnections; j++)  {
        printf("\n  DEBUG: Connection %d:\n", j);
        printf("  DEBUG:   startSegment_1 = %d at (%d,%d,%d)        endSegment_1 = %d at (%d,%d,%d)\n",
                  connections[i].connection[j].startSegment_1, connections[i].connection[j].startCoord_1.X,
                  connections[i].connection[j].startCoord_1.Y, connections[i].connection[j].startCoord_1.Z,
                  connections[i].connection[j].endSegment_1, connections[i].connection[j].endCoord_1.X,
                  connections[i].connection[j].endCoord_1.Y, connections[i].connection[j].endCoord_1.Z);
        printf("  DEBUG:   startSegment_2 = %d at (%d,%d,%d)        endSegment_2 = %d at (%d,%d,%d)\n",
                  connections[i].connection[j].startSegment_2, connections[i].connection[j].startCoord_2.X,
                  connections[i].connection[j].startCoord_2.Y, connections[i].connection[j].startCoord_2.Z,
                  connections[i].connection[j].endSegment_2, connections[i].connection[j].endCoord_2.X,
                  connections[i].connection[j].endCoord_2.Y, connections[i].connection[j].endCoord_2.Z);
        printf("  DEBUG:\n");
      }  // End of for-loop for index 'j'
    }  // End of for-loop for index 'i'
  }  // End of if-block for DEBUG_ON or error_found

  if (error_found)  {
    printf("\n Program is exiting.\n\n");
    exit(1);
  }

  // Return the maximum number of connections found in any single pseudo-path:
  return(maxConnectionsPerPath);

}  // End of function 'detectDiffPairConnections'


//-----------------------------------------------------------------------------
// Name: deleteDuplicatePoints
// Desc: For path number 'pathNum' iterate through the segments sequentially
//       and delete any segments that are adjacent in the path and also have
//       the same coordinates. By deleting points, this function changes the
//       'pathCoords' array and the 'pathLengths' array.
//-----------------------------------------------------------------------------
void deleteDuplicatePoints(const int pathNum, Coordinate_t *pathCoords[],
                           int pathLengths[], MapInfo_t *mapInfo)  {

  // printf("DEBUG: Entered function 'deleteDuplicatePoints' for path #%d...\n", pathNum);

  // Use the starting coordinates of this path to initialize prevX/prevY/prevZ:
  int prevX = mapInfo->start_cells[pathNum].X;
  int prevY = mapInfo->start_cells[pathNum].Y;
  int prevZ = mapInfo->start_cells[pathNum].Z;
  int num_deleted_points = 0;

  // Iterate through the path:
  for (int segment = 0; segment < pathLengths[pathNum]; segment++)  {

    // printf("DEBUG:    Path %d, segment %d is (%d,%d,%d). Previous was (%d,%d,%d).\n",
    //       pathNum, segment, x, y, z, prevX, prevY, prevZ);

    // Check for duplicate coordinate:
    if (   (prevX == pathCoords[pathNum][segment].X)
        && (prevY == pathCoords[pathNum][segment].Y)
        && (prevZ == pathCoords[pathNum][segment].Z))  {

      // We found a duplicate point, so delete this point by copying
      // element #(j+1) to element #j, starting at element #segment:
      for (int j = segment; j < pathLengths[pathNum] - 1; j++)  {
        pathCoords[pathNum][j] = copyCoordinates(pathCoords[pathNum][j+1]);
      }  // End of for-loop for index 'j'

      // Now that we've removed the duplicate, we can shorten
      // the path length for this path by 1:
      pathLengths[pathNum]--;
      num_deleted_points++;

      // printf("DEBUG: In function 'deleteDuplicatePoints', we deleted segment %d of path %d at (%d,%d,%d), reducing path length from %d to %d\n",
      //        segment, pathNum, x, y, z, pathLengths[pathNum] +1, pathLengths[pathNum]);

    }  // End of if-block for finding a duplicate coordinate

    // Re-populate prevX/prevY/prevZ for the next iteration through the for-loop:
    prevX = pathCoords[pathNum][segment].X;
    prevY = pathCoords[pathNum][segment].Y;
    prevZ = pathCoords[pathNum][segment].Z;

  }  // End of for-loop for index 'segment'

  // Now that we've removed all the duplicate points, we can shorten
  // the pathCoords array for this path:
  if (num_deleted_points > 0 )
    pathCoords[pathNum] = realloc(pathCoords[pathNum], pathLengths[pathNum] * sizeof(Coordinate_t));

  // printf("DEBUG: Exiting function 'deleteDuplicatePoints' for path #%d...\n", pathNum);

}  // End of function 'deleteDuplicatePoints'


//-----------------------------------------------------------------------------
// Name: update_swapZone_startTerms
// Desc: Updates the coordinates of the start-terminal for path 'path' that
//       originates in a swap-zone. The new coordinates are those of the last
//       segment before the path exits the swap zone. This function modifies
//       the 'start_cells' element of the mapInfo variable, but does not
//       affect the coordinates originally provided by the user for the
//       start-terminals, which are saved in variable user_inputs->start_X_um
//       and user_inputs->start_Y_um.
//
//       This function also deletes all segments from the pathCoords array
//       that were in the pin-swap zone, and therefore reduces the length of
//       the path that's stored in pathLengths.
//-----------------------------------------------------------------------------
void update_swapZone_startTerms(const int path, Coordinate_t *pathCoords[], int *pathLength,
                                InputValues_t *user_inputs, CellInfo_t ***const cellInfo,
                                MapInfo_t *mapInfo)  {

  // Define variables to hold coordinates of previous and current segment:
  int prevX, prevY, prevZ, x, y, z;

  // Check whether current path originates in a swap-zone area:
  if (mapInfo->swapZone[path])  {

    // Get the coordinates of the current start-terminal:
    prevX = mapInfo->start_cells[path].X;
    prevY = mapInfo->start_cells[path].Y;
    prevZ = mapInfo->start_cells[path].Z;

    // printf("DEBUG: (thread %2d) In function update_swapZone_startTerms, start_cells for path %d are (%d,%d,%d)\n", omp_get_thread_num(), path, prevX, prevY, prevZ);

    // Iterate over the segments of this path:
    for (int i = 0; i < *pathLength; i++)  {
      // Get coordinates of segment #i:
      x = (*pathCoords)[i].X;
      y = (*pathCoords)[i].Y;
      z = (*pathCoords)[i].Z;

      // Check if (x,y,z) is in a swap-zone:
      if (! cellInfo[x][y][z].swap_zone)  {

        // This is the first segment after the path has exited the swap-zone, so define the previous segment
        // as the new starting-coordinate (if it's different than the current starting-coordinate):

        // Check if we need to change the start-coordinate:
        if ((prevX != mapInfo->start_cells[path].X) || (prevY != mapInfo->start_cells[path].Y) || (prevZ != mapInfo->start_cells[path].Z))  {
          // printf("\nDEBUG: (thread %2d) In function 'update_swapZone_startTerms', start-terminal for path %d was updated from\n", omp_get_thread_num(), path);
          // printf(  "       (thread %2d) (%d,%d,%d) to (%d,%d,%d), which is the last coordinate before the path exited a swap-zone.\n",
          //         omp_get_thread_num(), mapInfo->start_cells[path].X, mapInfo->start_cells[path].Y, mapInfo->start_cells[path].Z, prevX, prevY, prevZ);

          // Update the starting coordinate in the mapInfo->start_cells variable:
          mapInfo->start_cells[path].X = prevX;
          mapInfo->start_cells[path].Y = prevY;
          mapInfo->start_cells[path].Z = prevZ;

          // If i > 0, then delete the i segments between the new start-cell (in a swap-zone)
          // and the first segment in a non-swap-zone area.
          if (i > 0)  {
            // Delete the first i segments by over-writing them with segments located
            // i segments farther in the path:
            for (int j = 0; j < *pathLength - i; j++)  {
              (*pathCoords)[j] = copyCoordinates((*pathCoords)[j+i]);
            }  // End of for-loop for index 'j'

            // Update the pathLengths array to reflect the shorter path:
            *pathLength = *pathLength - i;

            // Re-allocate memory for the shorter pathCoords array:
            *pathCoords = realloc(*pathCoords, *pathLength * sizeof(Coordinate_t));
            if (*pathCoords == 0)  {
              printf("\n\nERROR: Failed to reallocate memory for pathCoords[%d] in function update_swapZone_startTerms.\n\n", path);
              exit(1);
            }  // End of if-block for failed re-allocate
          }  // End of if-block for (i > 0)
        }  // End of if-block for changes in the starting-coordinate

        // Break out of the for-loop for the current path
        break;

      }  // End of if-block for when (x,y,z) is NOT in a swap-zone
      else  {
        // Segment #i is still in a swap-zone, so update the values of the prevX/prevY/prevZ
        // and move on to the next segment:
        prevX = x;
        prevY = y;
        prevZ = z;
      }  // End of else-block for when (x,y,z) is in a swap-zone

    }  // End of for-loop for index 'i'

  }  // End of if-block for swapZone[path]

}  // End of function 'update_swapZone_startTerms'


//-----------------------------------------------------------------------------
// Name: fillGapsInDiffPairPaths
// Desc: For each diff-pair path, check whether the distance between each segment
//       is not one of the legal jumps from the normal autorouter algorithm. If
//       the gap is an illegal jump, then insert extra segments until we have a legal
//       set of jumps. For illegal gaps of 5 cells or less in length, use
//       heuristics to fill in the gap. For longer gaps, run the path-finding
//       algorithm to find a legal path to fill in this gap. This function modifies
//       the arrays 'pathLength' and 'pathCoords'.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_fillGapsInDiffPairPaths' and re-compile if you want verbose debugging
// print-statements enabled:
//
// #define DEBUG_fillGapsInDiffPairPaths 1
#undef DEBUG_fillGapsInDiffPairPaths

static void fillGapsInDiffPairPaths(Coordinate_t *pathCoords[], int pathLengths[], InputValues_t *user_inputs,
                                    CellInfo_t ***cellInfo, MapInfo_t *mapInfo, RoutingMetrics_t *routability,
                                    PathFinding_t *pathFinding, int num_threads)  {

  //
  // Iterate through all user-defined paths:
  //
  printf("INFO: Entered function fillGapsInDiffPairPaths to fill gaps in diff-pair paths using %d threads...\n", num_threads);

  #ifdef DEBUG_fillGapsInDiffPairPaths
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration >= 84) && (mapInfo->current_iteration <= 84))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in fillGapsInDiffPairPaths() because specific requirements were met.\n\n");
    DEBUG_ON = TRUE;

    // printf("\nDEBUG: Iteration %d: At beginning of fillGapsInDiffPairPaths, congestion at (284,53,2) is:\n", mapInfo->current_iteration);
    // print_cell_congestion(&(cellInfo[284][53][2]));
    // printf("\nDEBUG: Iteration %d: At beginning of fillGapsInDiffPairPaths, congestion at (284,53,3) is:\n", mapInfo->current_iteration);
    // print_cell_congestion(&(cellInfo[284][53][3]));

  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Calculate the diagonal size of the map (in cell-units). If the routing radius exceeds this value
  // and findPath() still cannot fill the gap, then this program will exit with a fatal error message.
  float map_diagonal_size = sqrt(mapInfo->mapWidth * mapInfo->mapWidth   +   mapInfo->mapHeight * mapInfo->mapHeight);

  #ifdef DEBUG_fillGapsInDiffPairPaths
  #pragma omp parallel for if (! DEBUG_ON) schedule(dynamic, 1)
  #else
  #pragma omp parallel for schedule(dynamic, 1)
  #endif
  for (int pathNum = 0; pathNum < user_inputs->num_nets; pathNum++)  {

    // Skip paths that are not diff-pair paths:
    if (! user_inputs->isDiffPair[pathNum])  {
      continue;
    }

    // Get the thread number for the current for-loop, so we can use this number
    // to access the appropriate pathFinding array slice:
    int thread_num = omp_get_thread_num();

    #ifdef DEBUG_fillGapsInDiffPairPaths
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Analyzing path %d in fillGapsInDiffPairPaths...\n", thread_num, pathNum);
    }
    #endif

    // Initialize variable to hold the number of segments that will be
    // inserted within a given gap and within a given path:
    int num_inserted_segments_in_gap  = 0;
    int num_inserted_segments_in_path = 0;

    // Create an array to hold coordinates of segments that will be inserted
    // within a given gap. Allocate memory for 20 segments. (This can expand,
    // if needed.)
    Coordinate_t * insertedCoords;
    insertedCoords = malloc(20 * sizeof(Coordinate_t));

    #ifdef DEBUG_fillGapsInDiffPairPaths
    if (DEBUG_ON)  {
      printf("\n\nDEBUG: (thread %2d) Near top of function 'fillGapsInDiffPairPaths', each path's coordinates are:\n", thread_num);
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, pathNum);
      printf("DEBUG: (thread %2d)   Start-terminal: (%d,%d,%d)\n", thread_num, mapInfo->start_cells[pathNum].X,
             mapInfo->start_cells[pathNum].Y, mapInfo->start_cells[pathNum].Z);
      for (int segment = 0; segment < pathLengths[pathNum]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d)\n", thread_num, pathNum, segment,
                pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z);
      }  // End of for-loop for index 'segment'
    }
    #endif

    // Copy the starting terminal's coordinates into 'prev_segment' variable:
    Coordinate_t prev_segment = copyCoordinates(mapInfo->start_cells[pathNum]);

    #ifdef DEBUG_fillGapsInDiffPairPaths
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) ========= In fillGapsInDiffPairPaths, initial pathLengths[%d] = %d ==============\n",
             thread_num, pathNum, pathLengths[pathNum]);
    }
    #endif

    //
    // Iterate through each segment of the path:
    //
    for (int segment = 0; segment < pathLengths[pathNum]; segment++)  {

      #ifdef DEBUG_fillGapsInDiffPairPaths
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) ============== Analyzing segment %d at (%d,%d,%d)  ================\n", thread_num, segment,
                pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z);
      }
      #endif

      // Calculate delta-X, delta-Y, and delta-Z between current and previous segments:
      int deltaX = abs(pathCoords[pathNum][segment].X - prev_segment.X);
      int deltaY = abs(pathCoords[pathNum][segment].Y - prev_segment.Y);
      int deltaZ = abs(pathCoords[pathNum][segment].Z - prev_segment.Z);

      // Check if inter-segment distance is not a legal distance. Legal distances are any of the following 5:
      //     1) delta-X + delta-Y == 1, with delta-Z = 0    (a north/south/east/west jump)
      //     2) delta-X == 1  and  delta-Y == 1, with delta-Z = 0  (a diagonal jump)
      //     3) delta-X == 1  and  delta-Y == 2, with delta-Z = 0  (a knight's jump)
      //     4) delta-X == 2  and  delta-Y == 1, with delta-Z = 0  (a knight's jump)
      //     5) delta-X + delta-Y == 0 and delta-Z = 1   (an inter-layer jump)
      if (! (   ((deltaZ == 0) && ( (deltaX + deltaY == 1)         // Manhattan jump
             || ((deltaX == 1) && (deltaY == 1))                   // Diagonal jump
             ||   ((deltaX == 1) && (deltaY == 2))                 // Knight's jump
             ||   ((deltaX == 2) && (deltaY == 1))))               // Knight's jump
             || ((deltaX + deltaY == 0) && (deltaZ == 1)) ) )  {   // Via jump

        #ifdef DEBUG_fillGapsInDiffPairPaths
        if (DEBUG_ON)  {
          printf("\nDEBUG: (thread %2d) In path %d, the gap between segment %d at (%d,%d,%d) and segment %d at (%d,%d,%d) has a delta-X of %d and delta-Y of %d, which is illegal.\n",
                  thread_num, pathNum, segment - 1, prev_segment.X, prev_segment.Y, prev_segment.Z,
                  segment, pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z,
                  deltaX, deltaY);
        }
        #endif

        // Re-initialize to zero the number of segments that will be inserted into the current gap:
        num_inserted_segments_in_gap  = 0;

        // Calculate the length of the gap to be filled in. If the gap is 5 cells or less,
        // then we use heuristic algorithms to insert extra segments. For longer gaps, we
        // use the autorouting algorithm in function 'findPath()':
        long int gap_length_squared = deltaX*deltaX + deltaY*deltaY;
        int findShortPathResult = 0;  // Return-code from function 'findShortPathHeuristically()'
        if ((gap_length_squared <= 25) && (deltaZ == 0))  {
          //
          // For this short gap, call the heuristic path-finding function:
          //

          #ifdef DEBUG_fillGapsInDiffPairPaths
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Function 'findShortPathHeuristically' will be run to fill gap in path %d between (%d,%d,%d) and (%d,%d,%d).\n",
                    thread_num, pathNum, prev_segment.X, prev_segment.Y, prev_segment.Z, pathCoords[pathNum][segment].X,
                    pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z);
          }
          #endif

          findShortPathResult = findShortPathHeuristically(prev_segment, pathCoords[pathNum][segment], cellInfo, pathNum,
                                                           &insertedCoords, &num_inserted_segments_in_gap, user_inputs, mapInfo);

          #ifdef DEBUG_fillGapsInDiffPairPaths
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Function 'findShortPathHeuristically' returned %d. The %d inserted segments are:\n",
                    thread_num, findShortPathResult, num_inserted_segments_in_gap);
            for (int i = 0; i < num_inserted_segments_in_gap; i++)  {
              printf("DEBUG: (thread %2d)    (%d,%d,%d)\n", thread_num, insertedCoords[i].X, insertedCoords[i].Y, insertedCoords[i].Z);
            }
          }
          #endif

          // If no path was found, issue fatal error message and exit:
          if (findShortPathResult != 1)  {
            printf("\n\nERROR: No path was found by function 'findShortPathHeuristically' for diff-pair path %d ('%s') to fill in\n",
                    pathNum, user_inputs->net_name[pathNum]);
            printf(    "       gaps between location (%d,%d,%d) and (%d,%d,%d) in function 'fillGapsInDiffPairPaths'.  Inform the\n",
                    prev_segment.X,                 prev_segment.Y,                 prev_segment.Z,
                    pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z);
            printf("       software developer of this fatal error message.\n\n");
            exit(findShortPathResult);
          }  // End of if-block for fatal error message

        }  // End of if-block for (gap_length_squared <= 25)

        else  {  // Gap length is longer than 5 cells, so we use findPath():
          long unsigned pathCost = 0;    // Return-code from function 'findPath()'
          int routing_radius_multiplier = 1;

          // Determine the lowest and highest routing layer numbers that the gap's routing
          // must traverse:
          int minLayerNum = min(prev_segment.Z, pathCoords[pathNum][segment].Z);
          int maxLayerNum = max(prev_segment.Z, pathCoords[pathNum][segment].Z);

          // Keep running 'findPath()' until a solution is found:
          while (pathCost == 0)  {

            #ifdef DEBUG_fillGapsInDiffPairPaths
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Function 'findPath' will be run to fill in gap in path %d between (%d,%d,%d) and (%d,%d,%d)\n",
                      thread_num, pathNum, prev_segment.X, prev_segment.Y, prev_segment.Z, pathCoords[pathNum][segment].X,
                      pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z);
              int prev_DR_num    = cellInfo[prev_segment.X][prev_segment.Y][prev_segment.Z].designRuleSet;
              int current_DR_num = cellInfo[pathCoords[pathNum][segment].X][pathCoords[pathNum][segment].Y][pathCoords[pathNum][segment].Z].designRuleSet;
              int prev_DR_subset    = user_inputs->designRuleSubsetMap[pathNum][prev_DR_num];
              int current_DR_subset = user_inputs->designRuleSubsetMap[pathNum][current_DR_num];
              int prevAllowedRoutingDirections    = user_inputs->designRules[prev_DR_num][prev_DR_subset].routeDirections;
              int currentAllowedRoutingDirections = user_inputs->designRules[current_DR_num][current_DR_subset].routeDirections;

              printf("       (thread %2d) with route-directions 0x%05X and 0x%05X, respectively.\n", thread_num,
                      prevAllowedRoutingDirections, currentAllowedRoutingDirections);
            }
            #endif

            // For this long gap, we need to invoke the compute-intensive path-finding function. First, we need to
            // populate the 'routingRestrictions' structure to restrict routing to a small region around the gap.
            RoutingRestriction_t gapFillRestrictions;

            // Calculate the routing restrictions for this gap:
            calcGapRoutingRestrictions(&gapFillRestrictions, prev_segment, pathCoords[pathNum][segment], pathNum,
                                       pathCoords, pathLengths, FALSE, 0, 0, cellInfo, mapInfo, user_inputs);


            // Scale the routing restriction radii by the factor 'routing_radius_multiplier', which starts at 1 but
            // increases if findPath() fails to find a solution:
            for (int layer = minLayerNum; layer <= maxLayerNum; layer++)  {
              gapFillRestrictions.allowedRadiiMicrons[layer] *= routing_radius_multiplier;
              gapFillRestrictions.allowedRadiiCells[layer]   *= routing_radius_multiplier;

              // Check if the allowed radius has exceeded the size of the map after increasing the
              // 'routing_radius_multiplier'. This is not expected, and triggers a fatal error:
              if (gapFillRestrictions.allowedRadiiCells[layer] > mapInfo->mapDiagonal)  {
                printf("\n\nERROR: The allowed routing radius on layer #%d (%6.3f cells) exceeded the size of the map (%6.3f cells) in function 'fillGapsInDiffPairPaths'\n",
                       layer, gapFillRestrictions.allowedRadiiCells[layer], map_diagonal_size);
                printf(    "       for path #%d ('%s') between points (%d,%d,%d) cells and (%d,%d,%d) cells on attempt #%d. Inform the software\n",
                        pathNum, user_inputs->net_name[pathNum], prev_segment.X, prev_segment.Y, prev_segment.Z, pathCoords[pathNum][segment].X,
                        pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z, routing_radius_multiplier);
                printf(    "       developer of this fatal error message.\n\n");
                exit(1);
              }  // End of if-block for fatal error

            }  // End of for-loop for index 'layer'

            #ifdef DEBUG_fillGapsInDiffPairPaths
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) About to call findPath from fillGapsInDiffPairPaths with following routing restrictions:\n", thread_num);
              printf("DEBUG: (thread %2d)              restrictionFlag = %d\n", thread_num, gapFillRestrictions.restrictionFlag);
              printf("DEBUG: (thread %2d)           (centerX, centerY) = (%d,%d)\n", thread_num, gapFillRestrictions.centerX, gapFillRestrictions.centerY);
              for (int layer = min(prev_segment.Z, pathCoords[pathNum][segment].Z); layer <= max(prev_segment.Z, pathCoords[pathNum][segment].Z); layer++)  {
                printf("DEBUG: (thread %2d)    Layer %d:\n", thread_num, layer);
                printf("DEBUG: (thread %2d)             allowedLayers[%d] = %d\n", thread_num, layer, gapFillRestrictions.allowedLayers[layer]);
                printf("DEBUG: (thread %2d)       allowedRadiiMicrons[%d] = %6.3f\n", thread_num, layer, gapFillRestrictions.allowedRadiiMicrons[layer]);
                printf("DEBUG: (thread %2d)         allowedRadiiCells[%d] = %6.3f\n", thread_num, layer, gapFillRestrictions.allowedRadiiCells[layer]);
              }  // End of for-loop for index 'layer'
            }
            #endif

            pathCost = findPath(mapInfo, cellInfo, pathNum, prev_segment, pathCoords[pathNum][segment], &insertedCoords,
                                &num_inserted_segments_in_gap, user_inputs, routability, &pathFinding[thread_num],
                                2, FALSE, TRUE, &gapFillRestrictions, FALSE, FALSE);

            //
            // If no path was found, increase the value of 'routing_radius_multiplier' and try again in
            // the next pass through this while-loop:
            //
            if (pathCost == 0)  {
              printf("\n\nWARNING: No path was found by function 'findPath' for diff-pair path %d ('%s') to fill in gaps between\n",
                      pathNum, user_inputs->net_name[pathNum]);
              printf(    "         location (%d,%d,%d) and (%d,%d,%d) in function 'fillGapsInDiffPairPaths' with a routing radius\n",
                      prev_segment.X,                 prev_segment.Y,                 prev_segment.Z,
                      pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z);
              printf(    "         multiplier of %d. Unsuccessful routing radii about (%d,%d) were:\n",
                      routing_radius_multiplier, gapFillRestrictions.centerX, gapFillRestrictions.centerY);
              for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
                if (gapFillRestrictions.allowedLayers[layer])  {
                  printf(    "           Layer %d: %6.3f microns (%6.3f cells)\n", layer,
                         gapFillRestrictions.allowedRadiiMicrons[layer], gapFillRestrictions.allowedRadiiCells[layer]);
                }  // End of if-block for 'allowedLayers[layer]'
              }  // End of for-loop for index 'layer'
              printf(    "         Routing will be attempted again with a multiplier of %d.\n", routing_radius_multiplier + 1);

              routing_radius_multiplier++;
            }  // End of if-block for pathCost == 0
            else  {
              #ifdef DEBUG_fillGapsInDiffPairPaths
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d)    Done filling in gap along diff-pair net %d. G-cost was %'lu.\n\n", thread_num, pathNum, pathCost);
              }
              #endif
            }  // End of else-block

          }  // End of while-loop for (pathCost == 0)

        }  // End of else-block for distance > sqrt(5)

        // Decrease the 'num_inserted_segments_in_gap' by 1 because the last inserted point is already
        // one of the points in the original path:
        num_inserted_segments_in_gap--;

        // Add the number of segments from the current gap to the segments that will be
        // inserted into the entire path:
        num_inserted_segments_in_path += num_inserted_segments_in_gap;

        #ifdef DEBUG_fillGapsInDiffPairPaths
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) For path %d, the %d inserted points between (%d,%d,%d) and (%d,%d,%d) follow:\n", thread_num,
                 pathNum, num_inserted_segments_in_gap, prev_segment.X, prev_segment.Y, prev_segment.Z,
                 pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z);
          for (int i = 0; i < num_inserted_segments_in_gap; i++)  {
            printf("DEBUG: (thread %2d)   (%d,%d,%d)\n", thread_num, insertedCoords[i].X, insertedCoords[i].Y, insertedCoords[i].Z);
          }
        }
        #endif

        // Insert the new segments into the path by moving all segments after the gap by
        // 'num_inserted_segments_in_gap' places. Start by re-sizing the pathCoords array:
        pathLengths[pathNum] += num_inserted_segments_in_gap;

        #ifdef DEBUG_fillGapsInDiffPairPaths
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) pathLengths[%d] increased from %d to %d in function 'fillGapsInDiffPairPaths'\n",
                  thread_num, pathNum, pathLengths[pathNum] - num_inserted_segments_in_gap, pathLengths[pathNum]);
        }
        #endif

        pathCoords[pathNum] = realloc(pathCoords[pathNum], pathLengths[pathNum] * sizeof(Coordinate_t));

        // Next, we move each segment forward after the gap:
        for (int i = pathLengths[pathNum] - 1; i >= segment + num_inserted_segments_in_gap; i--)  {
          pathCoords[pathNum][i] = copyCoordinates(pathCoords[pathNum][i - num_inserted_segments_in_gap]);

          #ifdef DEBUG_fillGapsInDiffPairPaths
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) In path %d, segment %d at (%d,%d,%d) was copied forward to segment %d.\n",
                    thread_num, pathNum, i - num_inserted_segments_in_gap,
                    pathCoords[pathNum][i - num_inserted_segments_in_gap].X,
                    pathCoords[pathNum][i - num_inserted_segments_in_gap].Y,
                    pathCoords[pathNum][i - num_inserted_segments_in_gap].Z, i);
          }
          #endif

        }  // End of for-loop for index 'i'

        // Finally, we insert the new segments by overwriting the segments that were just
        // copied forward in the array:
        for (int j = segment; j < segment + num_inserted_segments_in_gap; j++)  {
          pathCoords[pathNum][j] = copyCoordinates(insertedCoords[j - segment]);
        }  // End of for-loop for index 'j'

      }  // End of if-block for an illegal jump between segments

      // Re-populate the 'prev_segment' variable for the next pass through this loop:
      prev_segment = copyCoordinates(pathCoords[pathNum][segment]);

    }  // End of for-loop for index 'segment'

    #ifdef DEBUG_fillGapsInDiffPairPaths
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Path %d requires %d segments to be inserted in function 'fillGapsInDiffPairPaths.\n",
             thread_num, pathNum, num_inserted_segments_in_path);
    }
    #endif

    // Free the memory allocated for the inserted coordinates:
    free(insertedCoords);
    insertedCoords = NULL;

    // If the path starts in a pin-swap zone, then update the start-terminal so that it's
    // the last path-segment before the path exits the pin-swap zone:
    if (mapInfo->swapZone[pathNum])  {
      update_swapZone_startTerms(pathNum, &(pathCoords[pathNum]), &pathLengths[pathNum],
                                 user_inputs, cellInfo, mapInfo);
    }  // End of if-block for net starting in a pin-swap zone

  }  // End of for-loop for index 'i' (0 to num_pseudo_nets)
}  // End of function 'fillGapsInDiffPairPaths'


//-----------------------------------------------------------------------------
// Name: postProcessDiffPairs
// Desc: Create diff-pair nets using the pseudo-nets routed by the auto-router.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_postProcess' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_postProcess 1
#undef DEBUG_postProcess

void postProcessDiffPairs(Coordinate_t *pathCoords[], int pathLengths[], InputValues_t *user_inputs,
                          CellInfo_t ***cellInfo, MapInfo_t *mapInfo, RoutingMetrics_t *routability,
                          PathFinding_t *pathFinding, RoutingMetrics_t subMapRoutability[2],
                          RoutingRestriction_t *noRoutingRestrictions, int num_threads)  {

  // Variables for time-stamps:
  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);

  #ifdef DEBUG_postProcess
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration >= 84) && (mapInfo->current_iteration <= 84))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in postProcessDiffPairs() because specific requirements were met in iteration %d.\n\n",
           mapInfo->current_iteration);
    DEBUG_ON = TRUE;

    // printf("DEBUG: Entered function 'postProcessDiffPairs' with pathLengths[0] = %d, pathLengths[1] = %d, pathLengths[2] = %d\n",
    //         pathLengths[0], pathLengths[1], pathLengths[2]);
    // printf("DEBUG: pathCoords[2][0].X = %d, pathCoords[2][0].Y = %d, pathCoords[2][0].Z = %d\n", pathCoords[2][0].X, pathCoords[2][0].Y, pathCoords[2][0].Z);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE

  // Calculate the total number of nets to check, including user-defined nets and
  // (if applicable) pseudo nets for differential pairs:
  int max_routed_nets = user_inputs->num_nets + user_inputs->num_pseudo_nets;
  #endif

  //
  // Post-process the routed paths that represent diff-pair nets.
  //
  #ifdef DEBUG_postProcess
  #pragma omp parallel for if (! DEBUG_ON) schedule(dynamic, 1)
  #else
  #pragma omp parallel for schedule(dynamic, 1)
  #endif
  for (int i = 0; i < user_inputs->num_pseudo_nets; i++)  {

    // Calculate the path number for the current pseudo-path:
    int pathNum = user_inputs->num_nets + i;

    // Capture the thread number, which will be used to pass the appropriate arrays
    // into functions:
    int thread_num = omp_get_thread_num();

    // We found a pseudo-net. Get the path numbers of the two nets that this pseudo-net
    // corresponds to:
    int path_1_number = user_inputs->pseudoNetToDiffPair_1[pathNum];
    int path_2_number = user_inputs->pseudoNetToDiffPair_2[pathNum];

    tim = time(NULL);
    now = localtime(&tim);
    printf("\nINFO: Post-processing diff-pair nets #%d and #%d from pseudo-net #%d in thread %d at %02d-%02d-%d, %02d:%02d:%02d.\n",
            path_1_number, path_2_number, pathNum, thread_num, now->tm_mon+1, now->tm_mday,
            now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

    #ifdef DEBUG_postProcess
    if (DEBUG_ON)  {
      printf("  DEBUG: (thread %2d) Net %d starts at (%d,%d,%d) and ends at (%d,%d,%d).\n", omp_get_thread_num(), path_1_number,
              mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y, mapInfo->start_cells[path_1_number].Z,
              mapInfo->end_cells[path_1_number].X,   mapInfo->end_cells[path_1_number].Y,   mapInfo->end_cells[path_1_number].Z);
      printf("  DEBUG: (thread %2d) Net %d starts at (%d,%d,%d) and ends at (%d,%d,%d).\n\n", omp_get_thread_num(), path_2_number,
              mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y, mapInfo->start_cells[path_2_number].Z,
              mapInfo->end_cells[path_2_number].X,   mapInfo->end_cells[path_2_number].Y,   mapInfo->end_cells[path_2_number].Z);
    }  // End of if-block for DEBUG_ON
    #endif

    // Create preliminary paths on the left and right 'shoulders' of the pseudo-net:
    createDiffPairShoulderPoints(pathNum, pathCoords, pathLengths, user_inputs, cellInfo, mapInfo);

    #ifdef DEBUG_postProcess
    if (DEBUG_ON)  {
      tim = time(NULL);
      now = localtime(&tim);
      printf("  DEBUG: (thread %2d) Successfully returned from function 'createDiffPairShoulderPoints' at %02d-%02d-%d, %02d:%02d:%02d.\n",
              thread_num, now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

      printf("\n\nDEBUG: (thread %2d) After createDiffPairShoulderPoints...\n", thread_num);
      printf("\nDEBUG: (thread %2d) Net %d starts at (%d,%d,%d) and ends at (%d,%d,%d).\n", thread_num, path_1_number,
              mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y, mapInfo->start_cells[path_1_number].Z,
              mapInfo->end_cells[path_1_number].X,   mapInfo->end_cells[path_1_number].Y,   mapInfo->end_cells[path_1_number].Z);
      printf(  "DEBUG: (thread %2d) Net %d starts at (%d,%d,%d) and ends at (%d,%d,%d).\n\n", thread_num, path_2_number,
              mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y, mapInfo->start_cells[path_2_number].Z,
              mapInfo->end_cells[path_2_number].X,   mapInfo->end_cells[path_2_number].Y,   mapInfo->end_cells[path_2_number].Z);

      printf("\n\nDEBUG: (thread %2d) After function 'createDiffPairShoulderPoints' in function 'postProcessDiffPairs', each path's coordinates are:\n", thread_num);
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_1_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_1_number, mapInfo->start_cells[path_1_number].X,
              mapInfo->start_cells[path_1_number].Y, mapInfo->start_cells[path_1_number].Z, mapInfo->start_cells[path_1_number].flag);
      for (int segment = 0; segment < pathLengths[path_1_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_1_number, segment,
                pathCoords[path_1_number][segment].X, pathCoords[path_1_number][segment].Y, pathCoords[path_1_number][segment].Z,
                pathCoords[path_1_number][segment].flag);
      }  // End of for-loop for index 'segment'
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_2_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_2_number,
             mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y,
             mapInfo->start_cells[path_2_number].Z, mapInfo->start_cells[path_2_number].flag);
      for (int segment = 0; segment < pathLengths[path_2_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_2_number, segment,
                pathCoords[path_2_number][segment].X, pathCoords[path_2_number][segment].Y,
                pathCoords[path_2_number][segment].Z, pathCoords[path_2_number][segment].flag);
      }  // End of for-loop for index 'segment'

      printf("\nDEBUG: (thread %2d) Pseudo-path number %d:\n", thread_num, pathNum);
      printf("DEBUG: (thread %2d)   Pseudo-path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), pathNum,
             mapInfo->start_cells[pathNum].X, mapInfo->start_cells[pathNum].Y,
             mapInfo->start_cells[pathNum].Z, mapInfo->start_cells[pathNum].flag);
      for (int segment = 0; segment < pathLengths[pathNum]; segment++)  {
        printf("DEBUG: (thread %2d)   Pseudo-path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, pathNum, segment,
               pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y,
               pathCoords[pathNum][segment].Z, pathCoords[pathNum][segment].flag);
      }  // End of for-loop for index 'segment'
    }  // End of if-block for DEBUG_ON
    #endif


    //
    // It's possible that function 'createDiffPairShoulderPoints()' can insert points into a
    // path that duplicate a neighboring point. To fix this, run function
    // 'deleteDuplicatePoints()' on both diff-pair nets:
    //
    deleteDuplicatePoints(path_1_number, pathCoords, pathLengths, mapInfo);
    deleteDuplicatePoints(path_2_number, pathCoords, pathLengths, mapInfo);

    #ifdef DEBUG_postProcess
    if (DEBUG_ON)  {
      tim = time(NULL);
      now = localtime(&tim);
      printf("  DEBUG: (thread %2d) Successfully returned from both calls to function 'deleteDuplicatePoints' after 'createDiffPairShoulderPoints' at %02d-%02d-%d, %02d:%02d:%02d.\n",
              thread_num, now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
      printf("\n\nDEBUG: (thread %2d) After both calls to function 'deleteDuplicatePoints' in function 'postProcessDiffPairs' after 'createDiffPairShoulderPoints', each path's coordinates are:\n", thread_num);
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_1_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_1_number,
             mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y,
             mapInfo->start_cells[path_1_number].Z, mapInfo->start_cells[path_1_number].flag);
      for (int segment = 0; segment < pathLengths[path_1_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_1_number, segment,
                pathCoords[path_1_number][segment].X, pathCoords[path_1_number][segment].Y,
                pathCoords[path_1_number][segment].Z, pathCoords[path_1_number][segment].flag);
      }  // End of for-loop for index 'segment'
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_2_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_2_number,
              mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y,
              mapInfo->start_cells[path_2_number].Z, mapInfo->start_cells[path_2_number].flag);
      for (int segment = 0; segment < pathLengths[path_2_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_2_number, segment,
                pathCoords[path_2_number][segment].X, pathCoords[path_2_number][segment].Y,
                pathCoords[path_2_number][segment].Z, pathCoords[path_2_number][segment].flag);
      }  // End of for-loop for index 'segment'
    }  // End of if-block for DEBUG_ON
    #endif


    // Add vias to the two shoulder paths beside pseudo-net 'pathNum':
    createDiffPairVias(pathNum, path_1_number, path_2_number, pathCoords, pathLengths, user_inputs, cellInfo, mapInfo);

    #ifdef DEBUG_postProcess
    if (DEBUG_ON)  {
      tim = time(NULL);
      now = localtime(&tim);
      printf("  DEBUG: (thread %2d) Successfully returned from function 'createDiffPairVias' at %02d-%02d-%d, %02d:%02d:%02d.\n",
              thread_num, now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

      printf("\n\nDEBUG: (thread %2d) After function 'createDiffPairVias' in function 'postProcessDiffPairs', each path's coordinates are:\n", thread_num);
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_1_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_1_number,
              mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y,
              mapInfo->start_cells[path_1_number].Z, mapInfo->start_cells[path_1_number].flag);
      for (int segment = 0; segment < pathLengths[path_1_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_1_number, segment,
                pathCoords[path_1_number][segment].X, pathCoords[path_1_number][segment].Y,
                pathCoords[path_1_number][segment].Z, pathCoords[path_1_number][segment].flag);
      }  // End of for-loop for index 'segment'
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_2_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_2_number,
              mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y,
              mapInfo->start_cells[path_2_number].Z, mapInfo->start_cells[path_2_number].flag);
      for (int segment = 0; segment < pathLengths[path_2_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_2_number, segment,
                pathCoords[path_2_number][segment].X, pathCoords[path_2_number][segment].Y,
                pathCoords[path_2_number][segment].Z, pathCoords[path_2_number][segment].flag);
      }  // End of for-loop for index 'segment'
    }  // End of if-block for DEBUG_ON
    #endif

    //
    // It's possible that function 'createDiffPairVias()' can insert points into a
    // path that duplicate a neighboring point. To fix this, run function
    // 'deleteDuplicatePoints()' on both diff-pair nets:
    //
    deleteDuplicatePoints(path_1_number, pathCoords, pathLengths, mapInfo);
    deleteDuplicatePoints(path_2_number, pathCoords, pathLengths, mapInfo);

    #ifdef DEBUG_postProcess
    if (DEBUG_ON)  {
      tim = time(NULL);
      now = localtime(&tim);
      printf("  DEBUG: (thread %2d) Successfully returned from both calls to function 'deleteDuplicatePoints' after 'createDiffPairVias' at %02d-%02d-%d, %02d:%02d:%02d.\n",
              thread_num, now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

      printf("\n\nDEBUG: (thread %2d) After both calls to function 'deleteDuplicatePoints' in function 'postProcessDiffPairs' after 'createDiffPairVias', each path's coordinates are:\n", thread_num);
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_1_number);
      for (int segment = 0; segment < pathLengths[path_1_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_1_number, segment,
                pathCoords[path_1_number][segment].X, pathCoords[path_1_number][segment].Y,
                pathCoords[path_1_number][segment].Z, pathCoords[path_1_number][segment].flag);
      }  // End of for-loop for index 'segment'
      printf("\nDEBUG: (thread %2d) Path number %d:\n", thread_num, path_2_number);
      for (int segment = 0; segment < pathLengths[path_2_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_2_number, segment,
                pathCoords[path_2_number][segment].X, pathCoords[path_2_number][segment].Y,
                pathCoords[path_2_number][segment].Z, pathCoords[path_2_number][segment].flag);
      }  // End of for-loop for index 'segment'
    }  // End of if-block for DEBUG_ON
    #endif

    // Delete selected diff-pair segments that are (a) near pseudo-vias,
    // (b) near design-rule boundaries, and (c) near terminals:
    deleteSelectedDiffPairSegments(pathNum, pathCoords, pathLengths, user_inputs, cellInfo, mapInfo);

    #ifdef DEBUG_postProcess
    if (DEBUG_ON)  {
      tim = time(NULL);
      now = localtime(&tim);
      printf("  DEBUG: (thread %2d) Successfully returned from function 'deleteSelectedDiffPairSegments' at %02d-%02d-%d, %02d:%02d:%02d.\n",
             thread_num, now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

      printf("\n\nDEBUG: (thread %2d) After function 'deleteSelectedDiffPairSegments' in function 'postProcessDiffPairs', each path's coordinates are:\n", thread_num);
      printf("\nDEBUG: (thread %2d) First path number %d:\n", thread_num, path_1_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_1_number,
             mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y,
             mapInfo->start_cells[path_1_number].Z, mapInfo->start_cells[path_1_number].flag);
      for (int segment = 0; segment < pathLengths[path_1_number]; segment++)  {
        printf("DEBUG: (thread %2d)   First path #%d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_1_number, segment,
               pathCoords[path_1_number][segment].X, pathCoords[path_1_number][segment].Y,
               pathCoords[path_1_number][segment].Z, pathCoords[path_1_number][segment].flag);
      }  // End of for-loop for index 'segment'
      printf("\nDEBUG: (thread %2d) Second path number %d:\n", thread_num, path_2_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_2_number,
             mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y,
             mapInfo->start_cells[path_2_number].Z, mapInfo->start_cells[path_2_number].flag);
      for (int segment = 0; segment < pathLengths[path_2_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Second path #%d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_2_number, segment,
               pathCoords[path_2_number][segment].X, pathCoords[path_2_number][segment].Y,
               pathCoords[path_2_number][segment].Z, pathCoords[path_2_number][segment].flag);
      }  // End of for-loop for index 'segment'
    }  // End of if-block for DEBUG_ON
    #endif


    //
    // It's possible that function 'deleteSelectedDiffPairSegments()' can cause non-adjacent,
    // duplicate coordinates to become adjacent, so that we have two neighboring points with
    // identical coordinates. To fix this, run function 'deleteDuplicatePoints()' on both diff-pair nets:
    //
    deleteDuplicatePoints(path_1_number, pathCoords, pathLengths, mapInfo);
    deleteDuplicatePoints(path_2_number, pathCoords, pathLengths, mapInfo);

    #ifdef DEBUG_postProcess
    if (DEBUG_ON)  {
      printf("\n\nDEBUG: (thread %2d) After both calls to 'deleteDuplicatePoints' after function 'deleteSelectedDiffPairSegments' in function 'postProcessDiffPairs', each path's coordinates are:\n", thread_num);
      printf("\nDEBUG: (thread %2d) First path number %d:\n", thread_num, path_1_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_1_number,
             mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y,
             mapInfo->start_cells[path_1_number].Z, mapInfo->start_cells[path_1_number].flag);
      for (int segment = 0; segment < pathLengths[path_1_number]; segment++)  {
        printf("DEBUG: (thread %2d)   First path #%d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_1_number, segment,
               pathCoords[path_1_number][segment].X, pathCoords[path_1_number][segment].Y,
               pathCoords[path_1_number][segment].Z, pathCoords[path_1_number][segment].flag);
      }  // End of for-loop for index 'segment'
      printf("\nDEBUG: (thread %2d) Second path number %d:\n", thread_num, path_2_number);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_2_number,
             mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y,
             mapInfo->start_cells[path_2_number].Z, mapInfo->start_cells[path_2_number].flag);
      for (int segment = 0; segment < pathLengths[path_2_number]; segment++)  {
        printf("DEBUG: (thread %2d)   Second path #%d, segment %d: (%d,%d,%d), flag = %d\n", thread_num, path_2_number, segment,
               pathCoords[path_2_number][segment].X, pathCoords[path_2_number][segment].Y,
               pathCoords[path_2_number][segment].Z, pathCoords[path_2_number][segment].flag);
      }  // End of for-loop for index 'segment'

    }  // End of if-block for DEBUG_ON
    #endif

  }  // End of for-loop for index 'pathNum'
  //
  // The above line marks the end of parallel processing for creating and pruning
  // diff-pair shoulder-paths.
  //


  //
  // Call function to optimize trace-to-via and trace-to-terminal connections
  // of the diff-pair paths:
  //
  #ifdef DEBUG_postProcess
  if (DEBUG_ON)  {
    tim = time(NULL);
    now = localtime(&tim);
    printf("\nDEBUG: (thread %2d) About to start 'optimizeDiffPairConnections' after deleteDuplicatePoints() at %02d-%02d-%d, %02d:%02d:%02d, \n",
           omp_get_thread_num(), now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  }  // End of if-block for DEBUG_ON
  #endif

  // Call function to optimize the connections between diff-pair traces and the corresponding
  // terminals and vias:
  // printf("DEBUG: Before calling optimizeDiffPairConnections, omp_get_num_threads = %d\n", omp_get_num_threads());
  optimizeDiffPairConnections(pathCoords, pathLengths, cellInfo, mapInfo, user_inputs, routability, subMapRoutability,
                              noRoutingRestrictions, num_threads);

  #ifdef DEBUG_postProcess
  if (DEBUG_ON)  {
    tim = time(NULL);
    now = localtime(&tim);
    printf("DEBUG: (thread %2d) Successfully returned from function 'optimizeDiffPairConnections' at %02d-%02d-%d, %02d:%02d:%02d.\n",
            omp_get_thread_num(), now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

    printf("\n\nDEBUG: (thread %2d) After function 'optimizeDiffPairConnections' in function 'postProcessDiffPairs', each path's coordinates are:\n",
           omp_get_thread_num());
    for (int pathNum = 0; pathNum < max_routed_nets; pathNum++)  {
      printf("\nDEBUG: (thread %2d) Path number %d:\n", omp_get_thread_num(), pathNum);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), pathNum, mapInfo->start_cells[pathNum].X,
              mapInfo->start_cells[pathNum].Y, mapInfo->start_cells[pathNum].Z, mapInfo->start_cells[pathNum].flag);
      for (int segment = 0; segment < pathLengths[pathNum]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", omp_get_thread_num(), pathNum, segment, pathCoords[pathNum][segment].X,
               pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z, pathCoords[pathNum][segment].flag);
      }  // End of for-loop for index 'segment'
    }  // End of for-loop for index 'pathNum'
  }  // End of if-block for DEBUG_ON
  #endif

  //
  // Call function to add points within gaps of the diff-pair paths:
  //
  #ifdef DEBUG_postProcess
  if (DEBUG_ON)  {
    tim = time(NULL);
    now = localtime(&tim);
    printf("DEBUG: (thread %2d) About to start 'fillGapsInDiffPairPaths' after optimizeDiffPairConnections() at %02d-%02d-%d, %02d:%02d:%02d, \n",
           omp_get_thread_num(), now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  }  // End of if-block for DEBUG_ON
  #endif

  fillGapsInDiffPairPaths(pathCoords, pathLengths, user_inputs, cellInfo, mapInfo, routability, pathFinding, num_threads);

  #ifdef DEBUG_postProcess
  if (DEBUG_ON)  {
    tim = time(NULL);
    now = localtime(&tim);
    printf("DEBUG: (thread %2d) Successfully returned from function 'fillGapsInDiffPairPaths' at %02d-%02d-%d, %02d:%02d:%02d.\n",
            omp_get_thread_num(), now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

    printf("\n\nDEBUG: (thread %2d) After function 'fillGapsInDiffPairPaths' in function 'postProcessDiffPairs', each path's coordinates are:\n", omp_get_thread_num());
    for (int pathNum = 0; pathNum < max_routed_nets; pathNum++)  {
      printf("\nDEBUG: (thread %2d) Path number %d:\n", omp_get_thread_num(), pathNum);
      printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), pathNum, mapInfo->start_cells[pathNum].X,
              mapInfo->start_cells[pathNum].Y, mapInfo->start_cells[pathNum].Z, mapInfo->start_cells[pathNum].flag);
      for (int segment = 0; segment < pathLengths[pathNum]; segment++)  {
        printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", omp_get_thread_num(), pathNum, segment, pathCoords[pathNum][segment].X,
               pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z, pathCoords[pathNum][segment].flag);
      }  // End of for-loop for index 'segment'
    }  // End of for-loop for index 'pathNum'
  }  // End of if-block for DEBUG_ON
  #endif

}  // End of function 'postProcessDiffPairs'
