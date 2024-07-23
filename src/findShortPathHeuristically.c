#include "global_defs.h"
#include "aStarLibrary.h"

//-----------------------------------------------------------------------------
// Name: findShortPathHeuristically
// Desc: Insert extra segments within path number 'pathNum' between points
//       mapInfo->start_cells[pathNum] and mapInfo->end_cells[pathNum].
//       These points must be within 5 cells of each other, or else a fatal
//       error is issued. This function modifies the variables at addresses
//       &insertedCoords and &num_inserted_segments_in_gap. Depending on the
//       arrangement of the start- and end-cells, the function attempts to find
//       a low-cost path between them.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_findShortPathHeuristically' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_findShortPathHeuristically 1
#undef DEBUG_findShortPathHeuristically

int findShortPathHeuristically(const Coordinate_t startCoord, const Coordinate_t endCoord, CellInfo_t ***cellInfo,
                               int pathNum, Coordinate_t *insertedCoords[], int *num_inserted_segments_in_gap,
                               InputValues_t *user_inputs, const MapInfo_t *mapInfo)  {

  // Calculate delta-X, delta-Y, and delta-Z between start- and end-points:
  int deltaX = endCoord.X - startCoord.X;
  int deltaY = endCoord.Y - startCoord.Y;
  int deltaZ = endCoord.Z - startCoord.Z;


  #ifdef DEBUG_findShortPathHeuristically
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration == 10) && (abs(deltaX) == 4) && (abs(deltaY) == 0))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in findShortPathHeuristically() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE

  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Entered 'findShortPathHeuristically' to connect path %d from (%d,%d,%d) to (%d,%d,%d). Deltas: (%d, %d, %d)\n",
           omp_get_thread_num(), pathNum, startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z, deltaX, deltaY, deltaZ);
  }
  #endif

  // Check that the start- and end-coordinates are within 5 cells of
  // each other and on the same layer. If not, issue fatal error message:
  long int gap_length_squared = deltaX*deltaX + deltaY*deltaY;
  if ((gap_length_squared > 25) || (deltaZ))  {
    printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
    printf(    "       The start- and end-coordinates are either separated by more than 5 cells, or are\n");
    printf(    "       located on different layers. The start- and end-coordinates are (in cell units):\n");
    printf(    "          Start: (%d, %d, %d)       End: (%d, %d, %d)\n", startCoord.X, startCoord.Y, startCoord.Z,
           endCoord.X, endCoord.Y, endCoord.Z);
    printf(    "       Inform the software developer of this fatal error message.\n\n");
    return(0);
  }  // End of if-block for illegal gap

  // Calculate the direction of the end-cell relative to the start-cell in the X- and
  // Y-directions. The direction is +1 if the end-cell has a higher value than the
  // start-cell, and -1 if the end-cell has a lower value than the start-cell. The
  // direction is zero if the two values are equal.
  int X_dir = 0;
  int Y_dir = 0;
  if (deltaX)
    X_dir = deltaX/abs(deltaX);
  if (deltaY)
    Y_dir = deltaY/abs(deltaY);


  // Capture the Z-coordinate (routing layer) as a local variable, since it will be
  // used repeatedly:
  int Z_coordinate = startCoord.Z;


  //
  // Depending on the values of deltaX and deltaY, create the necessary
  // intermediate points to fill in the gap:
  //
  if ((abs(deltaX) == 2) && (deltaY == 0))  {
    // Insert 2 points, including the end-point:
    *num_inserted_segments_in_gap = 2;

    // If deltaX is positive, add 1 to the X start-coordinate. If negative,
    // add -1:
    (*insertedCoords)[0].X = startCoord.X  +  X_dir;
    (*insertedCoords)[0].Y = startCoord.Y;
    (*insertedCoords)[0].Z = Z_coordinate;

    // Copy the end-coordinate as the last point:
    (*insertedCoords)[1] = copyCoordinates(endCoord);
  }  // End of if-block for abs(deltaX) == 2 && deltaY == 0

  else if ((deltaX == 0) && (abs(deltaY) == 2))  {
    // Insert 2 points, including the end-point:
    *num_inserted_segments_in_gap = 2;

    // If deltaY is positive, add 1 to the Y start-coordinate. If negative,
    // add -1:
    (*insertedCoords)[0].X = startCoord.X;
    (*insertedCoords)[0].Y = startCoord.Y  +  Y_dir;
    (*insertedCoords)[0].Z = Z_coordinate;

    // Copy the end-coordinate as the last point:
    (*insertedCoords)[1] = copyCoordinates(endCoord);
  }  // End of if-block for deltaX == 0 && abs(deltaY) == 2

  else if ((abs(deltaX) == 3) && (deltaY == 0))  {
    // Insert 3 points, including the end-point:
    *num_inserted_segments_in_gap = 3;

    // If deltaX is positive, add 1 and 2 to the X start-coordinate.
    // If negative, add -1 and -2:
    (*insertedCoords)[0].X = startCoord.X  +  X_dir;
    (*insertedCoords)[0].Y = startCoord.Y;
    (*insertedCoords)[0].Z = Z_coordinate;

    (*insertedCoords)[1].X = startCoord.X  +  2 * X_dir;
    (*insertedCoords)[1].Y = startCoord.Y;
    (*insertedCoords)[1].Z = Z_coordinate;

    // Copy the end-coordinate as the last point:
    (*insertedCoords)[2] = copyCoordinates(endCoord);
  }  // End of if-block for abs(deltaX) == 3 && deltaY == 0

  else if ((deltaX == 0) && (abs(deltaY) == 3))  {
    // Insert 3 points, including the end-point:
    *num_inserted_segments_in_gap = 3;

    // If deltaY is positive, add 1 and 2 to the Y start-coordinate.
    // If negative, add -1 and -2:
    (*insertedCoords)[0].X = startCoord.X;
    (*insertedCoords)[0].Y = startCoord.Y + Y_dir;
    (*insertedCoords)[0].Z = Z_coordinate;

    (*insertedCoords)[1].X = startCoord.X;
    (*insertedCoords)[1].Y = startCoord.Y + 2 * Y_dir;
    (*insertedCoords)[1].Z = Z_coordinate;

    // Copy the end-coordinate as the last point:
    (*insertedCoords)[2] = copyCoordinates(endCoord);
  }  // End of if-block for deltaX == 0 && abs(deltaY) == 3

  else if ((abs(deltaX) == 4) && (deltaY == 0))  {

    // Evaluate the  cost of 3 different routes; choose the
    // lowest-cost route. The routes are labeled A, B, and C:
    long route_A_cost, route_B_cost, route_C_cost;
    route_A_cost = route_B_cost = route_C_cost = 0;

    // Define Boolean flags that specify whether the three routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal, route_C_legal;
    route_A_legal = route_B_legal = route_C_legal = TRUE;

    //
    // Route 'A' consists of the 3 cells directly between the start- and end-cells:
    //
    int X_A_1 = startCoord.X  +  X_dir;       // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y;                 // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A
    int X_A_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y;                 // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A
    int X_A_3 = startCoord.X  +  3 * X_dir;   // X-coord of cell 3 in route A
    int Y_A_3 = startCoord.Y;                 // Y-coord of cell 3 in route A
    int Z_A_3 = Z_coordinate;                 // Z-coord of cell 3 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_3, Y_A_3, Z_A_3, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier
             || cellInfo[X_A_3][Y_A_3][Z_A_3].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,    startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #4 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_3, Y_A_3, Z_A_3,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_3, Y_A_3, Z_A_3,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of 1 cell at a knight's jump from the start- and end-cells:
    //
    int X_B_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  -  1;           // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    // Confirm that the cell for route-B is within the map:
    if (XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cell for route-B is walkable:
    else if (cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-B is legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Route 'C' also consists of 1 cell at a knight's jump from the start- and end-cells:
    //
    int X_C_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route C
    int Y_C_1 = startCoord.Y  +  1;           // Y-coord of cell 1 in route C
    int Z_C_1 = Z_coordinate;                 // Z-coord of cell 1 in route C


    // Confirm that the cell for route-C is within the map:
    if (XYZpointIsOutsideOfMap(X_C_1, Y_C_1, Z_C_1, mapInfo))  {
      route_C_legal = FALSE;
    }
    // Confirm that the cell for route-C is walkable:
    else if (cellInfo[X_C_1][Y_C_1][Z_C_1].forbiddenTraceBarrier)  {
      route_C_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-C is legal:

      // Calculate distance cost and congestion penalty of step #1 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route C:
      route_C_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_C_1, Y_C_1, Z_C_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_C_1, Y_C_1, Z_C_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-C being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal && !route_C_legal)  // Only route-A is legal.
        || (   route_B_legal && (route_A_cost <= route_B_cost)   // Routes B and C are legal, but route A
            && route_C_legal && (route_A_cost <= route_C_cost))  // has lowest cost.
        || ( route_B_legal && !route_C_legal && (route_A_cost <= route_B_cost))   // Route B is legal. Route C is illegal. Route A has lowest cost.
        || (!route_B_legal &&  route_C_legal && (route_A_cost <= route_C_cost)))  // Route B is illegal. Route B is legal. Route A has lowest cost.
    {
      // Route A has the lowest cost, so insert 4 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 4;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      (*insertedCoords)[2].X = X_A_3;  // Include 3rd point
      (*insertedCoords)[2].Y = Y_A_3;
      (*insertedCoords)[2].Z = Z_A_3;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[3] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal && !route_C_legal)  // Only route-B is legal.
             || (   route_A_legal && (route_B_cost <= route_A_cost)   // Routes A and C are legal, but route B
                 && route_C_legal && (route_B_cost <= route_C_cost))  // has the lowest cost.
             || ( route_A_legal && !route_C_legal && (route_B_cost <= route_A_cost))   // Route A is legal. Route C is illegal. Route B has lowest cost.
             || (!route_A_legal &&  route_C_legal && (route_B_cost <= route_C_cost)))  // Route A is illegal. Route C is legal. Route B has lowest cost.
    {
      // Route B has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    else if (   (!route_A_legal && !route_B_legal && route_C_legal)  // Only route-C is legal.
             || (   route_A_legal && (route_C_cost <= route_A_cost)   // Routes A and B are legal, but route C
                 && route_B_legal && (route_C_cost <= route_B_cost))  // has the lowest cost.
             || ( route_A_legal && !route_B_legal && (route_C_cost <= route_A_cost))   // Route A is legal. Route B is illegal. Route C has lowest cost.
             || (!route_A_legal &&  route_B_legal && (route_C_cost <= route_B_cost)))  // Route A is illegal. Route B is legal. Route C has lowest cost.
    {
      // Route C has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_C_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_C_1;
      (*insertedCoords)[0].Z = Z_C_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route C having the lowest cost

    // If none of the 3 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 4 && deltaY == 0

  else if ((deltaX == 0) && (abs(deltaY) == 4))  {

    // Evaluate the congestion cost of 3 different routes; choose the
    // lowest-cost route. The routes are labeled A, B, and C:
    long route_A_cost, route_B_cost, route_C_cost;
    route_A_cost = route_B_cost = route_C_cost = 0;

    // Define Boolean flags that specify whether the three routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal, route_C_legal;
    route_A_legal = route_B_legal = route_C_legal = TRUE;

    //
    // Route 'A' consists of the 3 cells between the start- and end-cells:
    //
    int X_A_1 = startCoord.X;                 // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +  Y_dir;       // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A
    int X_A_2 = startCoord.X;                 // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A
    int X_A_3 = startCoord.X;                 // X-coord of cell 3 in route A
    int Y_A_3 = startCoord.Y  +  3 * Y_dir;   // Y-coord of cell 3 in route A
    int Z_A_3 = Z_coordinate;                 // Z-coord of cell 3 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_3, Y_A_3, Z_A_3, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier
             || cellInfo[X_A_3][Y_A_3][Z_A_3].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,    startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #4 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_3, Y_A_3, Z_A_3,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_3, Y_A_3, Z_A_3,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of 1 cell at a knight's jump from the start- and end-cells:
    //
    int X_B_1 = startCoord.X  -  1;           // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    // Confirm that the cell for route-B is within the map:
    if (XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cell for route-B is walkable:
    else if (cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-B is legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Route 'C' also consists of 1 cell at a knight's jump from the start- and end-cells:
    //
    int X_C_1 = startCoord.X  +  1;           // X-coord of cell 1 in route C
    int Y_C_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route C
    int Z_C_1 = Z_coordinate;                 // Z-coord of cell 1 in route C

    // Confirm that the cell for route-C is within the map:
    if (XYZpointIsOutsideOfMap(X_C_1, Y_C_1, Z_C_1, mapInfo))  {
      route_C_legal = FALSE;
    }
    // Confirm that the cell for route-C is walkable:
    else if (cellInfo[X_C_1][Y_C_1][Z_C_1].forbiddenTraceBarrier)  {
      route_C_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-C is legal:

      // Calculate distance cost and congestion penalty of step #1 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route C:
      route_C_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_C_1, Y_C_1, Z_C_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_C_1, Y_C_1, Z_C_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-C being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal && !route_C_legal)  // Only route-A is legal.
        || (   route_B_legal && (route_A_cost <= route_B_cost)   // Routes B and C are legal, but route A
            && route_C_legal && (route_A_cost <= route_C_cost))  // has lowest cost.
        || ( route_B_legal && !route_C_legal && (route_A_cost <= route_B_cost))   // Route B is legal. Route C is illegal. Route A has lowest cost.
		|| (!route_B_legal &&  route_C_legal && (route_A_cost <= route_C_cost)))  // Route B is illegal. Route B is legal. Route A has lowest cost.
    {
      // Route A has the lowest cost, so insert 4 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 4;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      (*insertedCoords)[2].X = X_A_3;  // Include 3rd point
      (*insertedCoords)[2].Y = Y_A_3;
      (*insertedCoords)[2].Z = Z_A_3;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[3] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal && !route_C_legal)  // Only route-B is legal.
             || (   route_A_legal && (route_B_cost <= route_A_cost)   // Routes A and C are legal, but route B
    		     && route_C_legal && (route_B_cost <= route_C_cost))  // has the lowest cost.
             || ( route_A_legal && !route_C_legal && (route_B_cost <= route_A_cost))   // Route A is legal. Route C is illegal. Route B has lowest cost.
             || (!route_A_legal &&  route_C_legal && (route_B_cost <= route_C_cost)))  // Route A is illegal. Route C is legal. Route B has lowest cost.
    {
      // Route B has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    else if (   (!route_A_legal && !route_B_legal && route_C_legal)  // Only route-C is legal.
             || (   route_A_legal && (route_C_cost <= route_A_cost)   // Routes A and B are legal, but route C
    		     && route_B_legal && (route_C_cost <= route_B_cost))  // has the lowest cost.
             || ( route_A_legal && !route_B_legal && (route_C_cost <= route_A_cost))   // Route A is legal. Route B is illegal. Route C has lowest cost.
             || (!route_A_legal &&  route_B_legal && (route_C_cost <= route_B_cost)))  // Route A is illegal. Route B is legal. Route C has lowest cost.
    {
      // Route C has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_C_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_C_1;
      (*insertedCoords)[0].Z = Z_C_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route C having the lowest cost

    // If none of the 3 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 0 && deltaY == 4

  else if ((abs(deltaX) == 5) && (deltaY == 0))  {

    // Evaluate the congestion cost of 3 different routes; choose the
    // lowest-cost route. The routes are labeled A, B, and C:
    long route_A_cost, route_B_cost, route_C_cost;
    route_A_cost = route_B_cost = route_C_cost = 0;

    // Define Boolean flags that specify whether the three routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal, route_C_legal;
    route_A_legal = route_B_legal = route_C_legal = TRUE;

    //
    // Route 'A' consists of the 4 cells between the start- and end-cells:
    //
    int X_A_1 = startCoord.X  +  X_dir;       // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y;                 // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A
    int X_A_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y;                 // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A
    int X_A_3 = startCoord.X  +  3 * X_dir;   // X-coord of cell 3 in route A
    int Y_A_3 = startCoord.Y;                 // Y-coord of cell 3 in route A
    int Z_A_3 = Z_coordinate;                 // Z-coord of cell 3 in route A
    int X_A_4 = startCoord.X  +  4 * X_dir;   // X-coord of cell 4 in route A
    int Y_A_4 = startCoord.Y;                 // Y-coord of cell 4 in route A
    int Z_A_4 = Z_coordinate;                 // Z-coord of cell 4 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_3, Y_A_3, Z_A_3, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_4, Y_A_4, Z_A_4, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier
             || cellInfo[X_A_3][Y_A_3][Z_A_3].forbiddenTraceBarrier
             || cellInfo[X_A_4][Y_A_4][Z_A_4].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,    startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #4 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_4, Y_A_4, Z_A_4,   X_A_3, Y_A_3, Z_A_3,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_4, Y_A_4, Z_A_4,   X_A_3, Y_A_3, Z_A_3,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #5 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_4, Y_A_4, Z_A_4,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_4, Y_A_4, Z_A_4,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of 1 cell at a knight's jump from the start-cell, and 1 cell at
    // a knight's jump from the end-cell:
    //
    int X_B_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  -  1;           // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    int X_B_2 = startCoord.X  +  3 * X_dir;   // X-coord of cell 2 in route B
    int Y_B_2 = startCoord.Y  -  1;           // Y-coord of cell 2 in route B
    int Z_B_2 = Z_coordinate;                 // Z-coord of cell 2 in route B


    // Confirm that the cells for route-B are within the map:
    if (   XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_B_2, Y_B_2, Z_B_2, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cells for route-B are walkable:
    else if (   cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier
             || cellInfo[X_B_2][Y_B_2][Z_B_2].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-B are legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Route 'C' also consists of 1 cell at a knight's jump from the start-cell, and 1 cell at
    // a knight's jump from the end-cell:
    //
    int X_C_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route C
    int Y_C_1 = startCoord.Y  +  1;           // Y-coord of cell 1 in route C
    int Z_C_1 = Z_coordinate;                 // Z-coord of cell 1 in route C

    int X_C_2 = startCoord.X  +  3 * X_dir;   // X-coord of cell 2 in route C
    int Y_C_2 = startCoord.Y  +  1;           // Y-coord of cell 2 in route C
    int Z_C_2 = Z_coordinate;                 // Z-coord of cell 2 in route C

    // Confirm that the cells for route-C are within the map:
    if (   XYZpointIsOutsideOfMap(X_C_1, Y_C_1, Z_C_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_C_2, Y_C_2, Z_C_2, mapInfo))  {
      route_C_legal = FALSE;
    }
    // Confirm that the cells for route-C are walkable:
    else if (   cellInfo[X_C_1][Y_C_1][Z_C_1].forbiddenTraceBarrier
             || cellInfo[X_C_2][Y_C_2][Z_C_2].forbiddenTraceBarrier)  {
      route_C_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-C are legal:

      // Calculate distance cost and congestion penalty of step #1 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route C:
      route_C_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-C being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal && !route_C_legal)  // Only route-A is legal.
        || (   route_B_legal && (route_A_cost <= route_B_cost)   // Routes B and C are legal, but route A
            && route_C_legal && (route_A_cost <= route_C_cost))  // has lowest cost.
        || ( route_B_legal && !route_C_legal && (route_A_cost <= route_B_cost))   // Route B is legal. Route C is illegal. Route A has lowest cost.
		|| (!route_B_legal &&  route_C_legal && (route_A_cost <= route_C_cost)))  // Route B is illegal. Route B is legal. Route A has lowest cost.
    {
      // Route A has the lowest cost, so insert 5 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 5;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      (*insertedCoords)[2].X = X_A_3;  // Include 3rd point
      (*insertedCoords)[2].Y = Y_A_3;
      (*insertedCoords)[2].Z = Z_A_3;

      (*insertedCoords)[3].X = X_A_4;  // Include 4th point
      (*insertedCoords)[3].Y = Y_A_4;
      (*insertedCoords)[3].Z = Z_A_4;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[4] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal && !route_C_legal)  // Only route-B is legal.
             || (   route_A_legal && (route_B_cost <= route_A_cost)   // Routes A and C are legal, but route B
    		     && route_C_legal && (route_B_cost <= route_C_cost))  // has the lowest cost.
             || ( route_A_legal && !route_C_legal && (route_B_cost <= route_A_cost))   // Route A is legal. Route C is illegal. Route B has lowest cost.
             || (!route_A_legal &&  route_C_legal && (route_B_cost <= route_C_cost)))  // Route A is illegal. Route C is legal. Route B has lowest cost.
    {
      // Route B has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      (*insertedCoords)[1].X = X_B_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_B_2;
      (*insertedCoords)[1].Z = Z_B_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    else if (   (!route_A_legal && !route_B_legal && route_C_legal)  // Only route-C is legal.
             || (   route_A_legal && (route_C_cost <= route_A_cost)   // Routes A and B are legal, but route C
    		     && route_B_legal && (route_C_cost <= route_B_cost))  // has the lowest cost.
             || ( route_A_legal && !route_B_legal && (route_C_cost <= route_A_cost))   // Route A is legal. Route B is illegal. Route C has lowest cost.
             || (!route_A_legal &&  route_B_legal && (route_C_cost <= route_B_cost)))  // Route A is illegal. Route B is legal. Route C has lowest cost.
    {
      // Route C has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_C_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_C_1;
      (*insertedCoords)[0].Z = Z_C_1;

      (*insertedCoords)[1].X = X_C_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_C_2;
      (*insertedCoords)[1].Z = Z_C_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route C having the lowest cost

    // If none of the 3 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 5 && deltaY == 0

  else if ((deltaX == 0) && (abs(deltaY) == 5))  {

    // Evaluate the congestion cost of 3 different routes; choose the
    // lowest-cost route. The routes are labeled A, B, and C:
    long route_A_cost, route_B_cost, route_C_cost;
    route_A_cost = route_B_cost = route_C_cost = 0;

    // Define Boolean flags that specify whether the three routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal, route_C_legal;
    route_A_legal = route_B_legal = route_C_legal = TRUE;

    //
    // Route 'A' consists of the 4 cells between the start- and end-cells:
    //
    int X_A_1 = startCoord.X;                 // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +  Y_dir;       // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A
    int X_A_2 = startCoord.X;                 // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A
    int X_A_3 = startCoord.X;                 // X-coord of cell 3 in route A
    int Y_A_3 = startCoord.Y  +  3 * Y_dir;   // Y-coord of cell 3 in route A
    int Z_A_3 = Z_coordinate;                 // Z-coord of cell 3 in route A
    int X_A_4 = startCoord.X;                 // X-coord of cell 4 in route A
    int Y_A_4 = startCoord.Y  +  4 * Y_dir;   // Y-coord of cell 4 in route A
    int Z_A_4 = Z_coordinate;                 // Z-coord of cell 4 in route A


    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_3, Y_A_3, Z_A_3, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_4, Y_A_4, Z_A_4, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier
             || cellInfo[X_A_3][Y_A_3][Z_A_3].forbiddenTraceBarrier
             || cellInfo[X_A_4][Y_A_4][Z_A_4].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_3, Y_A_3, Z_A_3,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #4 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_4, Y_A_4, Z_A_4,   X_A_3, Y_A_3, Z_A_3,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_4, Y_A_4, Z_A_4,   X_A_3, Y_A_3, Z_A_3,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #5 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_4, Y_A_4, Z_A_4,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_4, Y_A_4, Z_A_4,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of 1 cell at a knight's jump from the start-cell, and 1 cell at
    // a knight's jump from the end-cell:
    //
    int X_B_1 = startCoord.X  -  1;           // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    int X_B_2 = startCoord.X  -  1;           // X-coord of cell 2 in route B
    int Y_B_2 = startCoord.Y  +  3 * Y_dir;   // Y-coord of cell 2 in route B
    int Z_B_2 = Z_coordinate;                 // Z-coord of cell 2 in route B

    // Confirm that the cells for route-B are within the map:
    if (   XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_B_2, Y_B_2, Z_B_2, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cells for route-B are walkable:
    else if (   cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier
             || cellInfo[X_B_2][Y_B_2][Z_B_2].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-B are legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Route 'C' also consists of 1 cell at a knight's jump from the start-cell, and 1 cell at
    // a knight's jump from the end-cell:
    //
    int X_C_1 = startCoord.X  +  1;           // X-coord of cell 1 in route C
    int Y_C_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route C
    int Z_C_1 = Z_coordinate;                 // Z-coord of cell 1 in route C

    int X_C_2 = startCoord.X  +  1;           // X-coord of cell 2 in route C
    int Y_C_2 = startCoord.Y  +  3 * Y_dir;   // Y-coord of cell 2 in route C
    int Z_C_2 = Z_coordinate;                 // Z-coord of cell 2 in route C

    // Confirm that the cells for route-C are within the map:
    if (   XYZpointIsOutsideOfMap(X_C_1, Y_C_1, Z_C_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_C_2, Y_C_2, Z_C_2, mapInfo))  {
      route_C_legal = FALSE;
    }
    // Confirm that the cells for route-C are walkable:
    else if (   cellInfo[X_C_1][Y_C_1][Z_C_1].forbiddenTraceBarrier
             || cellInfo[X_C_2][Y_C_2][Z_C_2].forbiddenTraceBarrier)  {
      route_C_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-C are legal:

      // Calculate distance cost and congestion penalty of step #1 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route C:
      route_C_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-C being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal && !route_C_legal)  // Only route-A is legal.
        || (   route_B_legal && (route_A_cost <= route_B_cost)   // Routes B and C are legal, but route A
            && route_C_legal && (route_A_cost <= route_C_cost))  // has lowest cost.
        || ( route_B_legal && !route_C_legal && (route_A_cost <= route_B_cost))   // Route B is legal. Route C is illegal. Route A has lowest cost.
		|| (!route_B_legal &&  route_C_legal && (route_A_cost <= route_C_cost)))  // Route B is illegal. Route B is legal. Route A has lowest cost.
    {
      // Route A has the lowest cost, so insert 5 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 5;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      (*insertedCoords)[2].X = X_A_3;  // Include 3rd point
      (*insertedCoords)[2].Y = Y_A_3;
      (*insertedCoords)[2].Z = Z_A_3;

      (*insertedCoords)[3].X = X_A_4;  // Include 4th point
      (*insertedCoords)[3].Y = Y_A_4;
      (*insertedCoords)[3].Z = Z_A_4;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[4] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal && !route_C_legal)  // Only route-B is legal.
             || (   route_A_legal && (route_B_cost <= route_A_cost)   // Routes A and C are legal, but route B
    		     && route_C_legal && (route_B_cost <= route_C_cost))  // has the lowest cost.
             || ( route_A_legal && !route_C_legal && (route_B_cost <= route_A_cost))   // Route A is legal. Route C is illegal. Route B has lowest cost.
             || (!route_A_legal &&  route_C_legal && (route_B_cost <= route_C_cost)))  // Route A is illegal. Route C is legal. Route B has lowest cost.
    {
      // Route B has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      (*insertedCoords)[1].X = X_B_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_B_2;
      (*insertedCoords)[1].Z = Z_B_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    else if (   (!route_A_legal && !route_B_legal && route_C_legal)  // Only route-C is legal.
             || (   route_A_legal && (route_C_cost <= route_A_cost)   // Routes A and B are legal, but route C
    		     && route_B_legal && (route_C_cost <= route_B_cost))  // has the lowest cost.
             || ( route_A_legal && !route_B_legal && (route_C_cost <= route_A_cost))   // Route A is legal. Route B is illegal. Route C has lowest cost.
             || (!route_A_legal &&  route_B_legal && (route_C_cost <= route_B_cost)))  // Route A is illegal. Route B is legal. Route C has lowest cost.
    {
      // Route C has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_C_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_C_1;
      (*insertedCoords)[0].Z = Z_C_1;

      (*insertedCoords)[1].X = X_C_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_C_2;
      (*insertedCoords)[1].Z = Z_C_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route C having the lowest cost

    // If none of the 3 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for deltaX == 0 && abs(deltaY) == 5

  else if ((abs(deltaX) == 3) && (abs(deltaY) == 1))  {

    // Evaluate the congestion cost of 2 different routes; choose the
    // lowest-cost route. The routes are labeled A and B:
    long route_A_cost, route_B_cost;
    route_A_cost = route_B_cost = 0;

    // Define Boolean flags that specify whether the two routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal;
    route_A_legal = route_B_legal = TRUE;

    //
    // Route 'A' consists of the 1 cell adjacent to the start-cell, and a
    // knight's jump from the end-cell:
    //
    int X_A_1 = startCoord.X  +  X_dir;       // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y;                 // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    // Confirm that the cell for route-A is within the map:
    if (XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cell for route-A is walkable:
    else if (cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-A is legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of the 1 cell adjacent to the end-cell, and a
    // knight's jump from the start-cell:
    //
    int X_B_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    // Confirm that the cell for route-B is within the map:
    if (XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cell for route-B is walkable:
    else if (cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-B is legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Depending on which route has the lower congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal)                                   // Only route-A is legal.
        || (route_A_legal && route_B_legal && (route_A_cost <= route_B_cost)))  // Routes A and B are legal, but route A has lower cost.
    {
      // Route A has the lower cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal)                                   // Only route-B is legal.
             || (route_A_legal && route_B_legal && (route_B_cost <= route_A_cost)))  // Routes A and B are legal, but route B has lower cost.
    {
      // Route B has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    // If none of the 2 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 3 && abs(deltaY) == 1

  else if ((abs(deltaX) == 1) && (abs(deltaY) == 3))  {

    // Evaluate the congestion cost of 2 different routes; choose the
    // lowest-cost route. The routes are labeled A and B:
    long route_A_cost, route_B_cost;
    route_A_cost = route_B_cost = 0;

    // Define Boolean flags that specify whether the two routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal;
    route_A_legal = route_B_legal = TRUE;

    //
    // Route 'A' consists of the 1 cell adjacent to the start-cell, and a
    // knight's jump from the end-cell:
    //
    int X_A_1 = startCoord.X;                 // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +  Y_dir;       // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    // Confirm that the cell for route-A is within the map:
    if (XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cell for route-A is walkable:
    else if (cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-A is legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of the 1 cell adjacent to the end-cell, and a
    // knight's jump from the start-cell:
    //
    int X_B_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    // Confirm that the cell for route-B is within the map:
    if (XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cell for route-B is walkable:
    else if (cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-B is legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal)                                    // Only route-A is legal.
        || (route_A_legal && route_B_legal && (route_A_cost <= route_B_cost)))  // Routes A and B are legal, but route A has lower cost.
    {
     // Route A has the lower cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal)                                    // Only route-B is legal.
             || (route_A_legal && route_B_legal && (route_B_cost <= route_A_cost)))  // Routes A and B are legal, but route B has lower cost.
    {
      // Route B has the lower cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    // If none of the 2 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 1 && abs(deltaY) == 3

  else if ((abs(deltaX) == 4) && (abs(deltaY) == 1))  {

    // Evaluate the congestion cost of 2 different routes; choose the
    // lowest-cost route. The routes are labeled A and B:
    long route_A_cost, route_B_cost;
    route_A_cost = route_B_cost = 0;

    // Define Boolean flags that specify whether the two routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal;
    route_A_legal = route_B_legal = TRUE;

    //
    // Route 'A' consists of the 1st cell adjacent to the start-cell, and a
    // 2nd cell adjacent to the 1st cell:
    //
    int X_A_1 = startCoord.X  +  X_dir;       // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y;                 // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    int X_A_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y;                 // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of the 1 cell located a knight's jump from the start-cell, and a
    // 2nd cell adjacent to the end-cell:
    //
    int X_B_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +  1 * Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    int X_B_2 = startCoord.X  +  3 * X_dir;   // X-coord of cell 2 in route B
    int Y_B_2 = startCoord.Y  +  1 * Y_dir;   // Y-coord of cell 2 in route B
    int Z_B_2 = Z_coordinate;                 // Z-coord of cell 2 in route B

    // Confirm that the cells for route-B are within the map:
    if (   XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_B_2, Y_B_2, Z_B_2, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cells for route-B are walkable:
    else if (   cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier
             || cellInfo[X_B_2][Y_B_2][Z_B_2].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-B are legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal)                                    // Only route-A is legal.
        || (route_A_legal && route_B_legal && (route_A_cost <= route_B_cost)))  // Routes A and B are legal, but route A has lower cost.
    {
      // Route A has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal)                                    // Only route-B is legal.
             || (route_A_legal && route_B_legal && (route_B_cost <= route_A_cost)))  // Routes A and B are legal, but route B has lower cost.
    {
      // Route B has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      (*insertedCoords)[1].X = X_B_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_B_2;
      (*insertedCoords)[1].Z = Z_B_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    // If none of the 2 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 4 && abs(deltaY) == 1

  else if ((abs(deltaX) == 1) && (abs(deltaY) == 4))  {

    // Evaluate the congestion cost of 2 different routes; choose the
    // lowest-cost route. The routes are labeled A and B:
    long route_A_cost, route_B_cost;
    route_A_cost = route_B_cost = 0;

    // Define Boolean flags that specify whether the two routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal;
    route_A_legal = route_B_legal = TRUE;

    //
    // Route 'A' consists of the 1st cell adjacent to the start-cell, and a
    // 2nd cell adjacent to the 1st cell:
    //
    int X_A_1 = startCoord.X;                 // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +  Y_dir;       // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    int X_A_2 = startCoord.X;                 // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of the 1 cell located a knight's jump from the start-cell, and a
    // 2nd cell adjacent to the end-cell:
    //
    int X_B_1 = startCoord.X  +  1 * X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    int X_B_2 = startCoord.X  +  1 * X_dir;   // X-coord of cell 2 in route B
    int Y_B_2 = startCoord.Y  +  3 * Y_dir;   // Y-coord of cell 2 in route B
    int Z_B_2 = Z_coordinate;                 // Z-coord of cell 2 in route B

    // Confirm that the cells for route-B are within the map:
    if (   XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_B_2, Y_B_2, Z_B_2, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cells for route-B are walkable:
    else if (   cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier
             || cellInfo[X_B_2][Y_B_2][Z_B_2].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-B are legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal)                                    // Only route-A is legal.
        || (route_A_legal && route_B_legal && (route_A_cost <= route_B_cost)))  // Routes A and B are legal, but route A has lower cost.
    {
      // Route A has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal)                                    // Only route-B is legal.
             || (route_A_legal && route_B_legal && (route_B_cost <= route_A_cost)))  // Routes A and B are legal, but route B has lower cost.
    {
      // Route B has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      (*insertedCoords)[1].X = X_B_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_B_2;
      (*insertedCoords)[1].Z = Z_B_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    // If none of the 2 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message


  }  // End of if-block for abs(deltaX) == 1 && abs(deltaY) == 4

  else if ((abs(deltaX) == 2) && (abs(deltaY) == 2))  {

    // Insert 2 points, including the end-point:
    *num_inserted_segments_in_gap = 2;

    // The location of the one intermediate point is right between the
    // the start- and end-cells, diagonal from both cells:
    (*insertedCoords)[0].X = startCoord.X  +  X_dir;
    (*insertedCoords)[0].Y = startCoord.Y  +  Y_dir;
    (*insertedCoords)[0].Z = Z_coordinate;

    // Copy the end-coordinate as the last point:
    (*insertedCoords)[1] = copyCoordinates(endCoord);

  }  // End of if-block for abs(deltaX) == 2 && abs(deltaY) == 2

  else if ((abs(deltaX) == 3) && (abs(deltaY) == 2))  {

    // Evaluate the congestion cost of 2 different routes; choose the
    // lowest-cost route. The routes are labeled A and B:
    long route_A_cost, route_B_cost;
    route_A_cost = route_B_cost = 0;

    // Define Boolean flags that specify whether the two routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal;
    route_A_legal = route_B_legal = TRUE;

    //
    // Route 'A' consists of the one cell located a knight's jump from the start-cell, which is
    // also located diagonal from the end-cell:
    //
    int X_A_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    // Confirm that the cell for route-A is within the map:
    if (XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cell for route-A is walkable:
    else if (cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-A is legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal


    //
    // Route 'B' consists of the one cell located a diagonally from the start-cell, which is
    // also located a knight's jump from the end-cell:
    //
    int X_B_1 = startCoord.X  +  X_dir;       // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +  Y_dir;       // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    // Confirm that the cell for route-B is within the map:
    if (XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cell for route-B is walkable:
    else if (cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-B is legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal)                                    // Only route-A is legal.
        || (route_A_legal && route_B_legal && (route_A_cost <= route_B_cost)))  // Routes A and B are legal, but route A has lower cost.
    {
      // Route A has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal)                                    // Only route-B is legal.
             || (route_A_legal && route_B_legal && (route_B_cost <= route_A_cost)))  // Routes A and B are legal, but route B has lower cost.
    {
      // Route B has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    // If none of the 2 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 3 && abs(deltaY) == 2

  else if ((abs(deltaX) == 2) && (abs(deltaY) == 3))  {

    // Evaluate the congestion cost of 2 different routes; choose the
    // lowest-cost route. The routes are labeled A and B:
    long route_A_cost, route_B_cost;
    route_A_cost = route_B_cost = 0;

    // Define Boolean flags that specify whether the two routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal;
    route_A_legal = route_B_legal = TRUE;

    //
    // Route 'A' consists of the one cell located a knight's jump from the start-cell, which is
    // also located diagonal from the end-cell:
    //
    int X_A_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    // Confirm that the cell for route-A is within the map:
    if (XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cell for route-A is walkable:
    else if (cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-A is legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of the one cell located a diagonally from the start-cell, which is
    // also located a knight's jump from the end-cell:
    //
    int X_B_1 = startCoord.X  +  X_dir;       // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +  Y_dir;       // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    // Confirm that the cell for route-B is within the map:
    if (XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cell for route-B is walkable:
    else if (cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-B is legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal)                                    // Only route-A is legal.
        || (route_A_legal && route_B_legal && (route_A_cost <= route_B_cost)))  // Routes A and B are legal, but route A has lower cost.
    {
      // Route A has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal)                                    // Only route-B is legal.
             || (route_A_legal && route_B_legal && (route_B_cost <= route_A_cost)))  // Routes A and B are legal, but route B has lower cost.
    {
      // Route B has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    // If none of the 2 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of else-block for fatal error message

  }  // End of if-block for abs(deltaX) == 2 && abs(deltaY) == 3

  else if ((abs(deltaX) == 4) && (abs(deltaY) == 2))  {

    // Insert 2 points, including the end-point:
    *num_inserted_segments_in_gap = 2;

    // The location of the one intermediate point is a knight's jump from both
    // the start-cell and from the end-cell:
    (*insertedCoords)[0].X = startCoord.X  +  2 * X_dir;
    (*insertedCoords)[0].Y = startCoord.Y  +      Y_dir;
    (*insertedCoords)[0].Z = Z_coordinate;

    // Copy the end-coordinate as the last point:
    (*insertedCoords)[1] = copyCoordinates(endCoord);

  }  // End of if-block for abs(deltaX) == 4 && abs(deltaY) == 2

  else if ((abs(deltaX) == 2) && (abs(deltaY) == 4))  {

    // Insert 2 points, including the end-point:
    *num_inserted_segments_in_gap = 2;

    // The location of the one intermediate point is a knight's jump from both
    // the start-cell and from the end-cell:
    (*insertedCoords)[0].X = startCoord.X  +      X_dir;
    (*insertedCoords)[0].Y = startCoord.Y  +  2 * Y_dir;
    (*insertedCoords)[0].Z = Z_coordinate;

    // Copy the end-coordinate as the last point:
    (*insertedCoords)[1] = copyCoordinates(endCoord);

  }  // End of if-block for abs(deltaX) == 2 && abs(deltaY) == 4

  else if ((abs(deltaX) == 3) && (abs(deltaY) == 3))  {

    // Evaluate the congestion cost of 3 different routes; choose the
    // lowest-cost route. The routes are labeled A, B, and C:
    long route_A_cost, route_B_cost, route_C_cost;
    route_A_cost = route_B_cost = route_C_cost = 0;

    // Define Boolean flags that specify whether the three routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal, route_C_legal;
    route_A_legal = route_B_legal = route_C_legal = TRUE;

    //
    // Route 'A' consists of the 2 cells located diagonally between the start- and end-cells:
    //
    int X_A_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    int X_A_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal


    //
    // Route 'B' consists of 1 cell at a knight's jump from the start- and end-cells:
    //
    int X_B_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    // Confirm that the cell for route-B is within the map:
    if (XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cell for route-B is walkable:
    else if (cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-B is legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Route 'C' also consists of 1 cell at a knight's jump from the start- and end-cells:
    //
    int X_C_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route C
    int Y_C_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route C
    int Z_C_1 = Z_coordinate;                 // Z-coord of cell 1 in route C

    // Confirm that the cell for route-C is within the map:
    if (XYZpointIsOutsideOfMap(X_C_1, Y_C_1, Z_C_1, mapInfo))  {
      route_C_legal = FALSE;
    }
    // Confirm that the cell for route-C is walkable:
    else if (cellInfo[X_C_1][Y_C_1][Z_C_1].forbiddenTraceBarrier)  {
      route_C_legal = FALSE;
    }
    else  {
      // We got here, so the cell in path-C is legal:

      // Calculate distance cost and congestion penalty of step #1 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 (to the end-cell) for route C:
      route_C_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_C_1, Y_C_1, Z_C_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_C_1, Y_C_1, Z_C_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-C being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal && !route_C_legal)  // Only route-A is legal.
        || (   route_B_legal && (route_A_cost <= route_B_cost)   // Routes B and C are legal, but route A
            && route_C_legal && (route_A_cost <= route_C_cost))  // has lowest cost.
        || ( route_B_legal && !route_C_legal && (route_A_cost <= route_B_cost))   // Route B is legal. Route C is illegal. Route A has lowest cost.
		|| (!route_B_legal &&  route_C_legal && (route_A_cost <= route_C_cost)))  // Route B is illegal. Route B is legal. Route A has lowest cost.
    {
      // Route A has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal && !route_C_legal)  // Only route-B is legal.
             || (   route_A_legal && (route_B_cost <= route_A_cost)   // Routes A and C are legal, but route B
    		     && route_C_legal && (route_B_cost <= route_C_cost))  // has the lowest cost.
             || ( route_A_legal && !route_C_legal && (route_B_cost <= route_A_cost))   // Route A is legal. Route C is illegal. Route B has lowest cost.
             || (!route_A_legal &&  route_C_legal && (route_B_cost <= route_C_cost)))  // Route A is illegal. Route C is legal. Route B has lowest cost.
    {
      // Route B has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    else if (   (!route_A_legal && !route_B_legal && route_C_legal)  // Only route-C is legal.
             || (   route_A_legal && (route_C_cost <= route_A_cost)   // Routes A and B are legal, but route C
    		     && route_B_legal && (route_C_cost <= route_B_cost))  // has the lowest cost.
             || ( route_A_legal && !route_B_legal && (route_C_cost <= route_A_cost))   // Route A is legal. Route B is illegal. Route C has lowest cost.
             || (!route_A_legal &&  route_B_legal && (route_C_cost <= route_B_cost)))  // Route A is illegal. Route B is legal. Route C has lowest cost.
    {
      // Route C has the lowest cost, so insert 2 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 2;

      (*insertedCoords)[0].X = X_C_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_C_1;
      (*insertedCoords)[0].Z = Z_C_1;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[1] = copyCoordinates(endCoord);
    }  // End of else-block for route C having the lowest cost

    // If none of the 3 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of if-block for fatal error message

  }  // End of if-block for abs(deltaX) == 3 && abs(deltaY) == 3

  else if ((abs(deltaX) == 4) && (abs(deltaY) == 3))  {

    // Evaluate the congestion cost of 3 different routes; choose the
    // lowest-cost route. The routes are labeled A, B, and C:
    long route_A_cost, route_B_cost, route_C_cost;
    route_A_cost = route_B_cost = route_C_cost = 0;

    // Define Boolean flags that specify whether the three routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal, route_C_legal;
    route_A_legal = route_B_legal = route_C_legal = TRUE;

    //
    // Route 'A' consists of the 2 cells between the start- and end-cells: The 1st is located
    // a knight's jump from the start-cell, and the 2nd is located diagonally between the 1st
    // cell and the end-cell:
    //
    int X_A_1 = startCoord.X  +  2 * X_dir;   // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A

    int X_A_2 = startCoord.X  +  3 * X_dir;   // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of 2 cells between the start- and end-cells: the 1st is located
    // diagonally from the start-cell, and the 2nd is located a knight's jump from the 1st
    // cell (and diagonally from the end-cell):
    //
    int X_B_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    int X_B_2 = startCoord.X  +  3 * X_dir;   // X-coord of cell 2 in route B
    int Y_B_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route B
    int Z_B_2 = Z_coordinate;                 // Z-coord of cell 2 in route B

    // Confirm that the cells for route-B are within the map:
    if (   XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_B_2, Y_B_2, Z_B_2, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cells for route-B are walkable:
    else if (   cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier
             || cellInfo[X_B_2][Y_B_2][Z_B_2].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-B are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Route 'C' consists of 2 cells between the start- and end-cells: the first is located
    // diagonally from the start-cell, and the 2nd is located diagonally from the 1st cell
    // (and a knight's jump from the end-cell):
    //
    int X_C_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route C
    int Y_C_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route C
    int Z_C_1 = Z_coordinate;                 // Z-coord of cell 1 in route C

    int X_C_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route C
    int Y_C_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route C
    int Z_C_2 = Z_coordinate;                 // Z-coord of cell 2 in route C

    // Confirm that the cells for route-C are within the map:
    if (   XYZpointIsOutsideOfMap(X_C_1, Y_C_1, Z_C_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_C_2, Y_C_2, Z_C_2, mapInfo))  {
      route_C_legal = FALSE;
    }
    // Confirm that the cells for route-C are walkable:
    else if (   cellInfo[X_C_1][Y_C_1][Z_C_1].forbiddenTraceBarrier
             || cellInfo[X_C_2][Y_C_2][Z_C_2].forbiddenTraceBarrier)  {
      route_C_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-C are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route C:
      route_C_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-C being legal


    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal && !route_C_legal)  // Only route-A is legal.
        || (   route_B_legal && (route_A_cost <= route_B_cost)   // Routes B and C are legal, but route A
            && route_C_legal && (route_A_cost <= route_C_cost))  // has lowest cost.
        || ( route_B_legal && !route_C_legal && (route_A_cost <= route_B_cost))   // Route B is legal. Route C is illegal. Route A has lowest cost.
		|| (!route_B_legal &&  route_C_legal && (route_A_cost <= route_C_cost)))  // Route B is illegal. Route B is legal. Route A has lowest cost.
    {
      // Route A has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal && !route_C_legal)  // Only route-B is legal.
             || (   route_A_legal && (route_B_cost <= route_A_cost)   // Routes A and C are legal, but route B
    		     && route_C_legal && (route_B_cost <= route_C_cost))  // has the lowest cost.
             || ( route_A_legal && !route_C_legal && (route_B_cost <= route_A_cost))   // Route A is legal. Route C is illegal. Route B has lowest cost.
             || (!route_A_legal &&  route_C_legal && (route_B_cost <= route_C_cost)))  // Route A is illegal. Route C is legal. Route B has lowest cost.
    {
      // Route B has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      (*insertedCoords)[1].X = X_B_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_B_2;
      (*insertedCoords)[1].Z = Z_B_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    else if (   (!route_A_legal && !route_B_legal && route_C_legal)  // Only route-C is legal.
             || (   route_A_legal && (route_C_cost <= route_A_cost)   // Routes A and B are legal, but route C
    		     && route_B_legal && (route_C_cost <= route_B_cost))  // has the lowest cost.
             || ( route_A_legal && !route_B_legal && (route_C_cost <= route_A_cost))   // Route A is legal. Route B is illegal. Route C has lowest cost.
             || (!route_A_legal &&  route_B_legal && (route_C_cost <= route_B_cost)))  // Route A is illegal. Route B is legal. Route C has lowest cost.
    {
      // Route C has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_C_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_C_1;
      (*insertedCoords)[0].Z = Z_C_1;

      (*insertedCoords)[1].X = X_C_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_C_2;
      (*insertedCoords)[1].Z = Z_C_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route C having the lowest cost

    // If none of the 3 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of if-block for fatal error message

  }  // End of if-block for abs(deltaX) == 4 && abs(deltaY) == 3

  else if ((abs(deltaX) == 3) && (abs(deltaY) == 4))  {

    // Evaluate the congestion cost of 3 different routes; choose the
    // lowest-cost route. The routes are labeled A, B, and C:
    long route_A_cost, route_B_cost, route_C_cost;
    route_A_cost = route_B_cost = route_C_cost = 0;

    // Define Boolean flags that specify whether the three routes comprise legal
    // cells that are walkable and within the map:
    int route_A_legal, route_B_legal, route_C_legal;
    route_A_legal = route_B_legal = route_C_legal = TRUE;

    //
    // Route 'A' consists of the 2 cells between the start- and end-cells: The 1st is located
    // a knight's jump from the start-cell, and the 2nd is located diagonally between the 1st
    // cell and the end-cell:
    //
    int X_A_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route A
    int Y_A_1 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 1 in route A
    int Z_A_1 = Z_coordinate;                 // Z-coord of cell 1 in route A
    int X_A_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route A
    int Y_A_2 = startCoord.Y  +  3 * Y_dir;   // Y-coord of cell 2 in route A
    int Z_A_2 = Z_coordinate;                 // Z-coord of cell 2 in route A

    // Confirm that the cells for route-A are within the map:
    if (   XYZpointIsOutsideOfMap(X_A_1, Y_A_1, Z_A_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_A_2, Y_A_2, Z_A_2, mapInfo))  {
      route_A_legal = FALSE;
    }
    // Confirm that the cells for route-A are walkable:
    else if (   cellInfo[X_A_1][Y_A_1][Z_A_1].forbiddenTraceBarrier
             || cellInfo[X_A_2][Y_A_2][Z_A_2].forbiddenTraceBarrier)  {
      route_A_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-A are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_1, Y_A_1, Z_A_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route A:
      route_A_cost += calc_distance_G_cost(   X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(X_A_2, Y_A_2, Z_A_2,   X_A_1, Y_A_1, Z_A_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route A:
      route_A_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_A_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_A_2, Y_A_2, Z_A_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-A being legal

    //
    // Route 'B' consists of 2 cells between the start- and end-cells: the 1st is located
    // diagonally from the start-cell, and the 2nd is located a knight's jump from the 1st
    // cell (and diagonally from the end-cell):
    //
    int X_B_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route B
    int Y_B_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route B
    int Z_B_1 = Z_coordinate;                 // Z-coord of cell 1 in route B

    int X_B_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route B
    int Y_B_2 = startCoord.Y  +  3 * Y_dir;   // Y-coord of cell 2 in route B
    int Z_B_2 = Z_coordinate;                 // Z-coord of cell 2 in route B


    // Confirm that the cells for route-B are within the map:
    if (   XYZpointIsOutsideOfMap(X_B_1, Y_B_1, Z_B_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_B_2, Y_B_2, Z_B_2, mapInfo))  {
      route_B_legal = FALSE;
    }
    // Confirm that the cells for route-B are walkable:
    else if (   cellInfo[X_B_1][Y_B_1][Z_B_1].forbiddenTraceBarrier
             || cellInfo[X_B_2][Y_B_2][Z_B_2].forbiddenTraceBarrier)  {
      route_B_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-B are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_1, Y_B_1, Z_B_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route B:
      route_B_cost += calc_distance_G_cost(   X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(X_B_2, Y_B_2, Z_B_2,   X_B_1, Y_B_1, Z_B_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route B:
      route_B_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_B_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_B_2, Y_B_2, Z_B_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-B being legal

    //
    // Route 'C' consists of 2 cells between the start- and end-cells: the first is located
    // diagonally from the start-cell, and the 2nd is located diagonally from the 1st cell
    // (and a knight's jump from the end-cell):
    //
    int X_C_1 = startCoord.X  +      X_dir;   // X-coord of cell 1 in route C
    int Y_C_1 = startCoord.Y  +      Y_dir;   // Y-coord of cell 1 in route C
    int Z_C_1 = Z_coordinate;                 // Z-coord of cell 1 in route C

    int X_C_2 = startCoord.X  +  2 * X_dir;   // X-coord of cell 2 in route C
    int Y_C_2 = startCoord.Y  +  2 * Y_dir;   // Y-coord of cell 2 in route C
    int Z_C_2 = Z_coordinate;                 // Z-coord of cell 2 in route C

    // Confirm that the cells for route-C are within the map:
    if (   XYZpointIsOutsideOfMap(X_C_1, Y_C_1, Z_C_1, mapInfo)
        || XYZpointIsOutsideOfMap(X_C_2, Y_C_2, Z_C_2, mapInfo))  {
      route_C_legal = FALSE;
    }
    // Confirm that the cells for route-C are walkable:
    else if (   cellInfo[X_C_1][Y_C_1][Z_C_1].forbiddenTraceBarrier
             || cellInfo[X_C_2][Y_C_2][Z_C_2].forbiddenTraceBarrier)  {
      route_C_legal = FALSE;
    }
    else  {
      // We got here, so the cells in path-C are all legal:

      // Calculate distance cost and congestion penalty of step #1 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_1, Y_C_1, Z_C_1,   startCoord.X, startCoord.Y, Z_coordinate,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #2 for route C:
      route_C_cost += calc_distance_G_cost(   X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(X_C_2, Y_C_2, Z_C_2,   X_C_1, Y_C_1, Z_C_1,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);

      // Calculate distance cost and congestion penalty of step #3 (to the end-cell) for route C:
      route_C_cost += calc_distance_G_cost(   endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                           user_inputs, cellInfo, mapInfo, pathNum);
      route_C_cost += calc_congestion_penalty(endCoord.X, endCoord.Y, endCoord.Z,   X_C_2, Y_C_2, Z_C_2,
                                              pathNum, TRACE, cellInfo, user_inputs, mapInfo, FALSE, 0, FALSE);
    }  // End of else-block for path-C being legal

    //
    // Depending on which route has the lowest congestion cost, define which points
    // to insert into the path:
    //
    if (   (route_A_legal && !route_B_legal && !route_C_legal)  // Only route-A is legal.
        || (   route_B_legal && (route_A_cost <= route_B_cost)   // Routes B and C are legal, but route A
            && route_C_legal && (route_A_cost <= route_C_cost))  // has lowest cost.
        || ( route_B_legal && !route_C_legal && (route_A_cost <= route_B_cost))   // Route B is legal. Route C is illegal. Route A has lowest cost.
		|| (!route_B_legal &&  route_C_legal && (route_A_cost <= route_C_cost)))  // Route B is illegal. Route B is legal. Route A has lowest cost.
    {
      // Route A has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_A_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_A_1;
      (*insertedCoords)[0].Z = Z_A_1;

      (*insertedCoords)[1].X = X_A_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_A_2;
      (*insertedCoords)[1].Z = Z_A_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of if-block for route A having the lowest cost

    else if (   (!route_A_legal && route_B_legal && !route_C_legal)  // Only route-B is legal.
             || (   route_A_legal && (route_B_cost <= route_A_cost)   // Routes A and C are legal, but route B
    		     && route_C_legal && (route_B_cost <= route_C_cost))  // has the lowest cost.
             || ( route_A_legal && !route_C_legal && (route_B_cost <= route_A_cost))   // Route A is legal. Route C is illegal. Route B has lowest cost.
             || (!route_A_legal &&  route_C_legal && (route_B_cost <= route_C_cost)))  // Route A is illegal. Route C is legal. Route B has lowest cost.
    {
      // Route B has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_B_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_B_1;
      (*insertedCoords)[0].Z = Z_B_1;

      (*insertedCoords)[1].X = X_B_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_B_2;
      (*insertedCoords)[1].Z = Z_B_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route B having the lowest cost

    else if (   (!route_A_legal && !route_B_legal && route_C_legal)  // Only route-C is legal.
             || (   route_A_legal && (route_C_cost <= route_A_cost)   // Routes A and B are legal, but route C
    		     && route_B_legal && (route_C_cost <= route_B_cost))  // has the lowest cost.
             || ( route_A_legal && !route_B_legal && (route_C_cost <= route_A_cost))   // Route A is legal. Route B is illegal. Route C has lowest cost.
             || (!route_A_legal &&  route_B_legal && (route_C_cost <= route_B_cost)))  // Route A is illegal. Route B is legal. Route C has lowest cost.
    {
      // Route C has the lowest cost, so insert 3 points from this route, including the end-point:
      *num_inserted_segments_in_gap = 3;

      (*insertedCoords)[0].X = X_C_1;  // Include 1st point
      (*insertedCoords)[0].Y = Y_C_1;
      (*insertedCoords)[0].Z = Z_C_1;

      (*insertedCoords)[1].X = X_C_2;  // Include 2nd point
      (*insertedCoords)[1].Y = Y_C_2;
      (*insertedCoords)[1].Z = Z_C_2;

      // Copy the end-coordinate as the last point:
      (*insertedCoords)[2] = copyCoordinates(endCoord);
    }  // End of else-block for route C having the lowest cost

    // If none of the 3 routes are legal, then issue a fatal error message and exit:
    else {
      printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically':\n");
      printf(    "ERROR: No valid path could be found between the start and end-coordinates:\n");
      printf(    "ERROR:    Start: (%d, %d, %d)       End: (%d, %d, %d) in cell-units\n",
             startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
      printf(    "ERROR: Inform the software developer of this fatal error message.\n\n");
      return(0);
    }  // End of if-block for fatal error message

  }  // End of if-block for abs(deltaX) == 3 && abs(deltaY) == 4

  else {
    printf("\n\nERROR: An unexpected condition was found within function 'findShortPathHeuristically': The\n");
    printf(    "       delta-X value (%d) and delta-Y value (%d) between the start- and end-coordinates\n", deltaX, deltaY);
    printf(    "       were not among the expected values. The start- and end-coordinates (in cell units) were:\n");
    printf(    "          Start: (%d, %d, %d)       End: (%d, %d, %d)\n", startCoord.X, startCoord.Y, Z_coordinate,
           endCoord.X, endCoord.Y, endCoord.Z);
    printf(    "       Inform the software developer of this fatal error message.\n\n");
    return(0);
  }  // End of else-block for unexpected deltaX/deltaY values.

  #ifdef DEBUG_findShortPathHeuristically
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Exiting 'findShortPathHeuristically' to connect path %d from (%d,%d,%d) to (%d,%d,%d). Deltas: (%d, %d, %d)\n",
           omp_get_thread_num(), pathNum, startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z, deltaX, deltaY, deltaZ);
    printf("DEBUG: (thread %2d)   %d coordinates will be inserted:\n", omp_get_thread_num(), *num_inserted_segments_in_gap);
    for (int i = 0; i < *num_inserted_segments_in_gap; i++)  {
      printf("DEBUG: (thread %2d)     Inserted segment %d: (%d,%d,%d)\n", omp_get_thread_num(), i,
             (*insertedCoords)[i].X, (*insertedCoords)[i].Y, (*insertedCoords)[i].Z);
    }  // End of for-loop for index 'i'
  }  // End of if-block for DEBUG_ON
  #endif


  return(1);  // Return '1' for successful completion

}  // End of function 'findShortPathHeuristically'
