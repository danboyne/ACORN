#include "global_defs.h"


//-----------------------------------------------------------------------------
// Name: createNoRoutingRestrictions
// Desc: Initialize elements of variable 'routingRestrictions' such that
//       this variable can be fed into function 'findPath()' for general routing
//       without any restrictions.
//-----------------------------------------------------------------------------
void createNoRoutingRestrictions(RoutingRestriction_t * routingRestrictions)  {

  routingRestrictions->restrictionFlag = FALSE;  // FALSE means no routing restriction
  routingRestrictions->centerX = 0;
  routingRestrictions->centerY = 0;

  // Iterate over each routing layer:
  for (int layer = 0; layer < maxRoutingLayers; layer++)  {
    routingRestrictions->allowedLayers[layer]       = TRUE;  // TRUE means that routing is allowed on layer
    routingRestrictions->allowedRadiiMicrons[layer] = 0.0;   // Zero implies an infinite radius on the layer
    routingRestrictions->allowedRadiiCells[layer]   = 0.0;   // Zero implies an infinite radius on the layer
  }  // End of for-loop for index 'layer'

}  // End of function 'void initializeAllowedRoutingRadii'


//-----------------------------------------------------------------------------
// Name: evaporateCongestion
// Desc: At each cell in the cellInfo matrix, reduce the congestion value by
//       the percentage specified by 'evaporationRate' at that cell. Valid values
//       of 'evaporationRate' could range from 0 to 100. The resulting 
//       congestion is always rounded down, so it could reach zero. If the
//       result is indeed zero congestion, then eliminate the traversing path
//       from the cell.
//
//       Congestion from path-number N is not evaporated, where N is the
//       path-number of the universal repellent. The value of N given by:
//              N  =  mapInfo->numPaths + mapInfo->numPseudoPaths
//-----------------------------------------------------------------------------
void evaporateCongestion(MapInfo_t *mapInfo, CellInfo_t ***cellInfo,
                         const float evaporationRate, int num_threads) {

  const float retainFactor = 1 - (evaporationRate/100.0); // Factor by which to multiply congestion
                                                          // in order to reduce congestion by
                                                          // evaporationRate percent.

  // Define the path number of the universal repellent net:
  const int universal_repellent_pathNum = mapInfo->numPaths + mapInfo->numPseudoPaths;

  // printf("DEBUG: In function 'evaporateCongestion', retainFactor = %5.3f\n", retainFactor);

  // Define an array that will hold the number of cells whose congestion memory will
  // later need to be re-allocated because they contain zero-congestion values:
  int numZeroCongestionCells[num_threads];

  // Define an array that holds Boolean flags indicating whether a warning message has
  // been printed that tells user that there are more zero-congestion elements than this
  // function could delete due to memory constraints.
  char warningMessagePrinted[num_threads];

  // Define a 2-dimensional array that holds the locations of congestion values
  // that evaporate to zero. The first dimension is necessary because each
  // thread needs its own array to avoid memory conflicts:
  Coordinate_t * zeroCongestionCells[num_threads];

  // Allocate memory for the 2nd dimension of each of the arrays defined above. The number
  // of array elements is 20% of the number of cells in the map, divided by the number of
  // threads. If this number is not sufficient to flag each zero-congestion position,
  // then such elements will not be eliminated from element in this call to
  // evaporateCongestion. But they're likely to be eliminated in subsequent calls after
  // future Acorn iterations.
  int maxDeletionsPerThread = (int)(0.20 * mapInfo->mapWidth * mapInfo->mapHeight * mapInfo->numLayers / num_threads);
  for (int i = 0; i < num_threads; i++)  {
    zeroCongestionCells[i] = malloc(maxDeletionsPerThread * sizeof(Coordinate_t));
    numZeroCongestionCells[i] = 0;     // Initialize array elements to zero
    warningMessagePrinted[i]  = FALSE; // Initialize array elements to FALSE
  }  // End of for-loop for index 'i'


  // printf("DEBUG: In function 'evaporateCongestion', retainFactor = %5.3f\n", retainFactor);

  // Iterate over all X/Y/Z locations and reduce the congestion values at each
  // cell. For congestion values that become zero, store their X/Y/Z coordinates
  // so that we can later eliminate these values and re-allocate (reduce)
  // the memory for such arrays. (Note that we cannot re-allocate memory in
  // the multi-threaded for-loops below because it would corrupt the memory.)
  #pragma omp parallel for collapse(3) schedule(dynamic, 1) if (num_threads > 1)
  for (int z = 0; z < mapInfo->numLayers; z++)  {
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      for (int x = 0; x < mapInfo->mapWidth; x++ )  {
        int thread_num = omp_get_thread_num();

        // If cell is not walkable, then skip it:
        if (cellInfo[x][y][z].forbiddenTraceBarrier)  {
          continue;
        }

        // At cell (x, y, z), determine how many paths have congestion at this cell:
        unsigned int num_traversing_paths;
        num_traversing_paths = cellInfo[x][y][z].numTraversingPaths;

        // printf("DEBUG: (thread %2d) Cell (%d,%d,%d) has %d congestion elements.\n", thread_num, x, y, z, num_traversing_paths);

        unsigned int original_congestion, new_congestion;

        // Define a Boolean flag that, if TRUE, flags the current (x,y,z) cell as having at
        // least one congestion index that has evaporated to zero.
        char cell_contains_zeroCongestion_element = FALSE;

        // For each traversing path, get the associated congestion and reduce it by 'retainFactor':
        for (int pathIndex = 0; pathIndex < num_traversing_paths; pathIndex++)  {


          // Check that the path-number of the congestion is *not* that of
          // the the universal repellent, which we don't want to evaporate:
          if (cellInfo[x][y][z].congestion[pathIndex].pathNum != universal_repellent_pathNum)  {

            // Get current congestion, i.e., number path-traversals multiplied by 100:
            original_congestion = cellInfo[x][y][z].congestion[pathIndex].pathTraversalsTimes100;
  
            // Calculate new congestion for this cell by multiplying the current
            // congestion by 'retainFactor', and truncating to the next lowest integer:
            new_congestion = (unsigned) (original_congestion * retainFactor);

            // printf("DEBUG:    (thread %2d) Index %d: Congestion reduced from %d to %d at (%d,%d,%d)\n",
            //        thread_num, pathIndex, original_congestion, new_congestion, x, y, z);

            // Assign the new congestion to the appropriate location in the cellInfo matrix:
            assignCongestionByPathIndex(&(cellInfo[x][y][z]), pathIndex, new_congestion);


            // If the congestion has evaporated to zero, then flag this cell
            // so we can later eliminate it from memory:
            if (new_congestion == 0)  {
              cell_contains_zeroCongestion_element = TRUE;
              // printf("DEBUG:       (thread %2d) For index %d at (%d,%d,%d), original_congestion = %d, new_congestion = %d\n",
              //         thread_num, pathIndex, x, y, z, original_congestion, new_congestion);
            }  // End of if-block for new_congestion == 0

          }  // End of if-block for pathNum != universal repellent

        }  // End of for-block for index 'pathIndex'

        //
        // If the cell at (x,y,z) was found to contain at least one element with
        // zero congestion, then record the x/y/z values:
        //
        if (cell_contains_zeroCongestion_element)  {
          // printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) has at least one zero-congestion element.\n", thread_num, x, y, z);

          // Confirm that we haven't flagged more cells than we've allocated memory for:
          if (numZeroCongestionCells[thread_num] < maxDeletionsPerThread)  {

            // Store the x/y/z/index parameters that have zero congestion so we can later
            // eliminate them from memory:
            zeroCongestionCells[thread_num][numZeroCongestionCells[thread_num]].X = x;
            zeroCongestionCells[thread_num][numZeroCongestionCells[thread_num]].Y = y;
            zeroCongestionCells[thread_num][numZeroCongestionCells[thread_num]].Z = z;

            // Increment the number of cells detected by this thread that contain
            // zero-congestion elements (and which we'll later eliminate):
            numZeroCongestionCells[thread_num]++;

            // printf("DEBUG:   (thread %2d) Cell at (%d,%d,%d) was flagged for deletion (#%d in this thread).\n",
            //         thread_num, x, y, z, numZeroCongestionCells[thread_num]);
          }  // End of if-block for numZeroCongestionCells < maxDeletionsPerThread

          else if (! warningMessagePrinted[thread_num])  {
            printf("\nWARNING: (thread %2d) The number of cells with congestion that evaporated to zero reached %'d in thread %d\n",
                     thread_num, maxDeletionsPerThread, thread_num);
            printf(  "WARNING: (thread %2d) which is the maximum number that can be deleted in a single iteration. More\n", thread_num);
            printf(  "WARNING: (thread %2d) zero-congestion values will naturally be deleted in subsequent iterations\n", thread_num);
            printf(  "WARNING: (thread %2d) to save memory/RAM.\n\n", thread_num);

            // Set the Boolean flag to TRUE to avoid printing out more of the above warnings.
            warningMessagePrinted[thread_num] = TRUE;
          }  // End of else/if block to print warning message
        }  // End of if-block for (cell_contains_zeroCongestion_element == TRUE)

      }  // End of loop for index 'x'
    }  // End of loop for index 'y'
  }  // End of loop for index 'z'
  //
  // The above line represents the end of parallel processing
  //

  //
  // In single-threaded mode, iterate over the two-dimensional
  // zeroCongestionCells[][] array and re-allocate memory for
  // cells and indices that are zero.
  //
  int x, y, z;  // Coordinates of cells that contain zero-congestion values
  for (int thread = 0; thread < num_threads; thread++)  {
    // printf("DEBUG: Processing the %d cells from thread %d that contain zero-congestion elements...\n",
    //         numZeroCongestionCells[thread], thread);

    for (int i = 0; i < numZeroCongestionCells[thread]; i++)  {

      // Get the x/y/z/index values corresponding to zero congestion:
      x = zeroCongestionCells[thread][i].X;
      y = zeroCongestionCells[thread][i].Y;
      z = zeroCongestionCells[thread][i].Z;

      // At cell (x,y,z), determine how many congestion indices exist at this cell:
      int old_num_indices = cellInfo[x][y][z].numTraversingPaths;

      // printf("DEBUG: Processing cell #%d from thread %d at (%d,%d,%d). old_num_indices = %d.\n",
      //         i, thread, x, y, z, old_num_indices);

      // Define variable that will contain the new (but reduced) number of congestion
      // indices for this (x,y,z) cell. Initially, this value is set to the old value
      // (but it will later be decremented when we find congestion values of zero).
      int new_num_indices = old_num_indices;

      // Iterate over the congestion indices to find zero-values:
      int pathIndex = 0;
      while (pathIndex < new_num_indices)  {

        // Get the new (post-evaporation) congestion value:
        int new_congestion = cellInfo[x][y][z].congestion[pathIndex].pathTraversalsTimes100;

        // Check whether the new congestion is zero or non-zero:
        if (new_congestion != 0)  {
          // Congestion is not zero, so increment pathIndex so we can move on to
          // the next traversing path:
          pathIndex++;
        }
        else  {
          // New congestion is zero, so eliminate this traversing path from this cell:

          // printf("DEBUG: Congestion at (%d,%d,%d) for path index %d evaporated to zero, so eliminating this traversing path\n",
          //        x, y, z, pathIndex);
          // printf("       and moving subsequent path indices 'down' by one...\n");

          // For each of the subsequent paths that traverse this cell, move the
          // path number and its congestion 'down' by 1. That is, reduce the index
          // values for these subsequent paths:
          for (int oldPathIndex = pathIndex + 1; oldPathIndex < new_num_indices; oldPathIndex++)  {

            // Assign path number, DR_subset, shape type, and congestion to (oldPathIndex - 1):
            cellInfo[x][y][z].congestion[oldPathIndex - 1].pathNum                = cellInfo[x][y][z].congestion[oldPathIndex].pathNum;
            cellInfo[x][y][z].congestion[oldPathIndex - 1].DR_subset              = cellInfo[x][y][z].congestion[oldPathIndex].DR_subset;
            cellInfo[x][y][z].congestion[oldPathIndex - 1].shapeType              = cellInfo[x][y][z].congestion[oldPathIndex].shapeType;
            cellInfo[x][y][z].congestion[oldPathIndex - 1].pathTraversalsTimes100 = cellInfo[x][y][z].congestion[oldPathIndex].pathTraversalsTimes100;

            // printf("DEBUG:     Moved path number %d and congestion %d from path index %d to %d at (%d,%d,%d).\n",
            //         temp_path_number, temp_congestion, oldPathIndex, oldPathIndex-1, x, y, z);

          }  // End of for-loop for index 'oldPathIndex'
          // printf("DEBUG:     Done moving subsequent path indices 'down' by one.\n");

          // Reduce the new number of congestion indices by 1:
          new_num_indices--;

          // Update the 'cellInfo' matrix with the new (reduced) number of congestion
          // indices for this cell:
          cellInfo[x][y][z].numTraversingPaths = new_num_indices;

          // Re-allocate (or free) memory for 'congestion[x][y][z]' array so the array
          // requires 1 fewer element:
          if (new_num_indices)  {
            cellInfo[x][y][z].congestion = realloc(cellInfo[x][y][z].congestion,
                                            new_num_indices * sizeof(Congestion_t));
          }
          else  {
            // There is no longer any congestion at (x,y,z), so free the
            // memory in the 1-dimensional 'congestion' matrix for this
            // (x,y,z) location:
            free(cellInfo[x][y][z].congestion);
            cellInfo[x][y][z].congestion = NULL; // Set pointer to NULL as a precaution
          }

        }  // End of else-block to handle case where new_congestion is zero

      }  // End of while-loop for (pathIndex < new_num_indices)

    }  // End of for-loop for index 'i'
  }  // End of for-loop for index 'thread'

  // Free the memory that was allocated in this function:
  for (int i = 0; i < num_threads; i++)  {
    free(zeroCongestionCells[i]);
    zeroCongestionCells[i] = NULL;
  }  // End of for-loop for index 'i'

}  // End of function 'evaporateCongestion'


//-----------------------------------------------------------------------------
// Name: directionToText
// Desc: Given a direction, routeDir, this function generates a text string
//       that corresponds to that routing direction. See the definitions of
//       routing directions in file 'global_defs.h' for which text string is
//       appropriate for which value of the routing direction. The text string
//       is written to the character buffer starting at routeDescription.
//-----------------------------------------------------------------------------
void directionToText(const int routeDir, char * routeDescription)  {

  if (routeDir == ANY)  {
    strcpy(routeDescription, "ANY");
  }
  else if (routeDir == NONE)  {
    strcpy(routeDescription, "NONE");
  }
  else if (routeDir == ANY_LATERAL)  {
    strcpy(routeDescription, "ANY_LATERAL");
  }
  else if (routeDir == MANHATTAN)  {
    strcpy(routeDescription, "MANHATTAN");
  }
  else if (routeDir == X_ROUTING)  {
    strcpy(routeDescription, "X_ROUTING");
  }
  else if (routeDir == NORTH_SOUTH)  {
    strcpy(routeDescription, "NORTH_SOUTH");
  }
  else if (routeDir == EAST_WEST)  {
    strcpy(routeDescription, "EAST_WEST");
  }
  else if (routeDir == MANHATTAN_X)  {
    strcpy(routeDescription, "MANHATTAN_X");
  }
  else if (routeDir == UP_DOWN)  {
    strcpy(routeDescription, "UP_DOWN");
  }
  else if (routeDir <= 0x03FFFF)  {
    sprintf(routeDescription, "Custom route direction: 0x%06X", routeDir);  // Print route-direction in hexadecimal format
  }
  else  {
    printf("\n\nERROR: In function 'directionToText', an illegal value was received for the variable 'routeDir': %d\n", routeDir);
    printf(    "       Please inform the software developer of this fatal error message.\n\n");
    exit(1);
  }

}  // End of function 'directionToText'

//-----------------------------------------------------------------------------
// Name: addCongestion
// Desc: Add congestion for path number 'pathNum' to the 'cellInfo' cell with
//       with shape-type 'shapeType' and design-rule subset 'DR_subset'.
//       If pathNum with shapeType and DR_subset already traverses current cell,
//       then simply add the congestionPenalty associated with this
//       pathNum/DR_subset/shapeType.
//
//       If an entry with 'pathNum', 'shapeType', and 'DR_subset' does not
//       already exist in this cell, then re-allocate memory to increase the
//       array length of cellInfo[x][y][z].congestion[i].
//-----------------------------------------------------------------------------
void addCongestion(CellInfo_t *cellInfo, int pathNum, unsigned short DR_subset,
                   unsigned short shapeType, unsigned congestionPenalty)  {

  if (congestionPenalty == 0)  {
    printf("\n\nWARNING: Function 'addCongestion' was called to add zero congestion for path %d\n", pathNum);
    printf(    "         Function will return without action.\n\n");
    return;
  }

  // Check whether the 'pathNum' path with shape-type 'shapeType' and design-rule
  // subset 'DR_subset' already traverses the current cell.
  const int pathIndex = getIndexOfTraversingPath(cellInfo, pathNum, DR_subset, shapeType);

  if (pathIndex != -1)  {

    // 'pathNum' path already traverses current cell, so simply augment the
    // congestion associated with this path:
    const unsigned original_congestion = cellInfo->congestion[pathIndex].pathTraversalsTimes100;

    // printf("  DEBUG: Path number %d with shape-type %d and DR_subset %d already traverses (%d,%d,%d) with pathTraversalsTimes100 = %d,\n",
    //        pathNum, shapeType, DR_subset, x, y, z, original_congestion);
    // printf("         so we'll augment the congestion associated with this path by adding %d.\n", congestionPenalty);

    // printf("  DEBUG: Assigning congestion %d to pathIndex %d at (%d, %d, %d).\n",
    //         original_congestion + congestionPenalty, pathIndex, x, y, z);
    assignCongestionByPathIndex(cellInfo, pathIndex, original_congestion + congestionPenalty);

    // printf("DEBUG: Function 'addCongestion' exited with:\n");
    // printf("            (x,y,z): (%d,%d,%d)\n", x, y, z);
    // printf("            pathNum: %d\n", pathNum);
    // printf("          shapeType: %d\n", shapeType);
    // printf("          DR_subset: %d\n", DR_subset);
    // printf("  congestionPenalty: %d\n", original_congestion + congestionPenalty);

    return;

  }  // End of if-block for pathIndex != -1

  // Get the initial number of paths that traverse this cell:
  const unsigned original_num_paths = cellInfo->numTraversingPaths;

  // printf("  DEBUG: original_num_paths is %d at (%d,%d,%d)\n", original_num_paths, x, y, z);

  // Increment current number of paths; this is the new number of paths:
  const unsigned new_num_paths = original_num_paths + 1;

  // printf("  DEBUG: new_num_paths is %d at (%d,%d,%d)\n", new_num_paths, x, y, z);

  // Increment the value of traversing paths in the 'cellInfo' variable
  // and add the path number and associated congestion value to the
  // 'congestion' variable:
  if (new_num_paths <= maxTraversingShapes)  {
    cellInfo->numTraversingPaths = new_num_paths;
  }
  else  {
    printf("ERROR: Function 'addCongestion' attempted to increase the 'numTraversingPaths' variable\n");
    printf("       to %d, which equals/exceeds the maximum allowed value (%d).\n",
            new_num_paths, maxTraversingShapes);
    printf("       This reflects an error in the software algorithm. The program is exiting.\n\n");
    exit(1);
  }

  // If this is the first congestion added at this cell), then allocate
  // memory to accommodate 1 element. If this location already
  // contained congestion from other path-crossings, then
  // re-allocate memory of the 'congestion[x][y][z]' array so it can
  // printf("DEBUG: Address of cellInfo[%d][%d][%d].congestion is %p.\n", x, y, z, cellInfo[x][y][z].congestion);
  // printf("DEBUG: sizeof(Congestion_t) is %d.\n", (int) sizeof(Congestion_t));
  // printf("DEBUG: About to re-allocate memory in 'cellInfo[][][].congestion' matrix...\n");
  if (original_num_paths == 0)  {
    cellInfo->congestion = malloc(new_num_paths * sizeof(Congestion_t));
    // printf("DEBUG: Memory allocated for cellInfo[%d][%d][%d].congestion at address %p\n", x, y, z, cellInfo[x][y][z].congestion);
    if (cellInfo->congestion == 0)  {
      printf("\n\nERROR: Failed to allocate memory for congestion in 'cellInfo' matrix in function 'addCongestion'.\n\n");
      exit(1);
    }  // End of if-block
  }  // End of if-block
  else  {
    cellInfo->congestion = realloc(cellInfo->congestion, new_num_paths * sizeof(Congestion_t));
    // printf("DEBUG: Memory reallocated for cellInfo[%d][%d][%d].congestion at address %p\n", x, y, z, cellInfo[x][y][z].congestion);
    if (cellInfo->congestion == 0)  {
      printf("\n\nERROR: Failed to re-allocate memory for congestion in 'cellInfo' matrix in function 'addCongestion'.\n\n");
      exit(1);
    }  // End of if-block
  }  // End of if/else block
  // printf("DEBUG: In function 'addCongestion', successfully re-allocated memory for congestion in the 'cellInfo' matrix for path %d at (%d,%d,%d), with new_num_paths=%d.\n", pathNum, x, y, z, new_num_paths);

  // Add congestionPenalty, path number, and path type  to 'cellInfo' matrix. Note that the path index
  // is equal to 'original_num_paths', since the index range is zero to (new_num_paths-1),
  // and original_num_paths is equal to new_num_paths-1.
  cellInfo->congestion[original_num_paths].pathTraversalsTimes100 = congestionPenalty;
  cellInfo->congestion[original_num_paths].pathNum                = pathNum;
  cellInfo->congestion[original_num_paths].DR_subset              = DR_subset;
  cellInfo->congestion[original_num_paths].shapeType              = shapeType;

  // printf("  DEBUG: cellInfo[%d][%d][%d].congestion[%d].pathTraversalsTimes100 set to %d.\n",
  //         x, y, z, original_num_paths, congestionPenalty);
  // printf("  DEBUG: cellInfo[%d][%d][%d].congestion[%d].pathNum    set to %d.\n",
  //         x, y, z, original_num_paths, pathNum);
  // printf("  DEBUG: cellInfo[%d][%d][%d].congestion[%d].DR_subset  set to %d.\n",
  //         x, y, z, original_num_paths, DR_subset);
  // printf("  DEBUG: cellInfo[%d][%d][%d].congestion[%d].shapeType  set to %d.\n",
  //         x, y, z, original_num_paths, shapeType);


  // printf("DEBUG: Function 'addCongestion' exited with:\n");
  // printf("            (x,y,z): (%d,%d,%d)\n", x, y, z);
  // printf("            pathNum: %d\n", pathNum);
  // printf("          shapeType: %d\n", shapeType);
  // printf("          DR_subset: %d\n", DR_subset);
  // printf("  congestionPenalty: %d\n", congestionPenalty);

  return;

}  // End of function 'addCongestion'


//-----------------------------------------------------------------------------
// Name: initializePathFindingArrays
// Desc: Initialize all elements in the various path-finding arrays to values
//       appropriate for the beginning of function 'findPath()'.
//-----------------------------------------------------------------------------
void initializePathFindingArrays(PathFinding_t *pathFinding, const MapInfo_t *mapInfo)  {

  const char notOpenOrClosedList = -1;  // Denotes that cell is not yet on the Open or Closed List

  // printf("DEBUG: Entered 'initializePathFindingArrays' with pathFinding address = %p\n", pathFinding);
  int i, j, k;

  for (i = 0; i < (mapInfo->mapWidth+1) ; i++)  {
    for (j = 0; j < (mapInfo->mapHeight+1); j++)  {
      for (k = 0; k < (mapInfo->numLayers+1); k++)  {
        // Initialize whichList elements so all cells are neither on the Open
        // nor the Closed list.

        // printf("DEBUG: About to set whichList[%d][%d][%d] to %d\n", i, j, k, notOpenOrClosedList);

        pathFinding->whichList[i][j][k] = notOpenOrClosedList;
        pathFinding->sortNumber[i][j][k] = 0;
      }  // End of for-loop for index 'k'
    }  // End of for-loop for index 'j'
   }  // End of for-loop for index 'i'

}  // End of function 'initializePathFindingArrays'


//-----------------------------------------------------------------------------
// Name: calcMinimumAllowedDirection
// Desc: Given two routing directions, routeDir_1 and routeDir_2, this function
//       returns the more restrictive routing direction. For cases where there
//       is zero overlap between the two directions, we return the logical OR
//       of the two directions (the superset) -- unless one of the directions
//       is 'NONE', in which case we return 'NONE'.
//-----------------------------------------------------------------------------
int calcMinimumAllowedDirection(const int routeDir_1, const int routeDir_2)  {

  // Logically AND the two directions together to find the direction that
  // describes the intersection of the two directions:
  int dir1_AND_dir2 = routeDir_1 & routeDir_2;

  // If the logical AND of the two route directions is non-zero, then return
  // the result:
  if (dir1_AND_dir2)  {
    return(dir1_AND_dir2);
  }  // End of if-block for dir1_AND_dir2 > 0

  else if ((! routeDir_1) || (! routeDir_2))  {
    // One of the routing directions is 'NONE', so we should return 'NONE':
    return(NONE);
  }  // End of if/else block for case where 1 of the directions is NONE

  else  {
    // We got here, so the two routing directions have no overlap. We therefore
    // return the logical OR of the two routing directions:

    // printf("DEBUG: Function calcMinimumAllowedDirections received two non-overlapping directions:\n");
    // printf("        '%s' and '%s'\n", directionToText(routeDir_1), directionToText(routeDir_2));
    // printf("       The result will be the logical OR of these directions:\n");
    // printf("        '%s'\n\n", directionToText(routeDir_1 | routeDir_2));

    return(routeDir_1 | routeDir_2);
  }  // End of else-block for route directions having zero overlap

}  // End of function 'calcMinimumAllowedDirection'


//-----------------------------------------------------------------------------
// Name: allowedDirection
// Desc: Determine whether the routing direction represented by
//       (deltaX, deltaY, deltaZ) is an allowed routing direction, based on the
//       value of the parameter 'allowedDir'. The variable allowedDir is a
//       binary-encoded value with the following bit-field definitions:
//
//            Bit-fields: | Up   Dn | N    S    E    W  | NE   SE   SW   NW |NxNE ExNE ExSE SxSE|SxSW WxSW WxNW NxNW|
//                        |---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|
// ANY         = 0x03FFFF | 1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  |
// NONE        = 0x000000 | 0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// ANY_LATERAL = 0x00FFFF | 0    0  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  |
// MANHATTAN   = 0x03F000 | 1    1  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// X_ROUTING   = 0x030F00 | 1    1  | 0    0    0    0  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  |
// NORTH_SOUTH = 0x03C000 | 1    1  | 1    1    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// EAST_WEST   = 0x033000 | 1    1  | 0    0    1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// MANHATTAN_X = 0x03FF00 | 1    1  | 1    1    1    1  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  |
// UP_DOWN     = 0x030000 | 1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
//
//-----------------------------------------------------------------------------
int allowedDirection(const int deltaX, const int deltaY, const int deltaZ, const int allowedDir)  {

  // Initialize the return_value to FALSE. It will be changed to TRUE if the transition is
  // not prohibited:
  int return_value = FALSE;

  //
  // Check all possible transitions:
  //
  if (deltaZ == 1) {
    // We got here, so transition is to a different layer.
    if (deltaX + deltaY == 0)  {
      // We got here, so cell (a,b,c) is directly above or below cell (x,y,z):

      // Set the return_value to TRUE if allowedDir contains 1's in the 'Up'
      // or 'Down' bit-fields:
      if (allowedDir & 0x030000)  {
        return_value = TRUE;
      }  // End of if-block for allowedDir containing 1's in Up/Down bit-fields
    }  // End of if-block for (delX,delY,delZ) = (0,0,1)
  }  // End of if-block for deltaZ == 1

  else  {
    // We got here, so transition is on the same layer (deltaZ is zero):
    // Check for condition in which (a,b,c) is located one cell east/west from (x,y,z):
    if ((deltaX == 1) && (deltaY == 0) && (allowedDir & 0x003000))  {
      return_value = TRUE;
    }  // End of if/else block for (delX,delY,delZ) = (1,0,0)

    // Check for condition in which (a,b,c) is located one cell north/south from (x,y,z):
    else if ((deltaX == 0) && (deltaY == 1) && (allowedDir & 0x00C000))  {
      return_value = TRUE;
    }  // End of if/else block for (delX,delY,delZ) = (0,1,0)

    // Check for condition in which (a,b,c) is located one cell diagonally from (x,y,z):
    else if ((deltaX == 1) && (deltaY == 1) && (allowedDir & 0x000F00))  {
      return_value = TRUE;
    }  // End of if/else block for (delX,delY,delZ) = (1,1,0)

    // Check for condition in which (a,b,c) is located a knight's move from (x,y,z):
    else if ((deltaX == 2) && (deltaY == 1) && (allowedDir & 0x0000FF))  {
      return_value = TRUE;
    }  // End of if/else block for (delX,delY,delZ) = (2,1,0)

    // Check for condition in which (a,b,c) is located a knight's move from (x,y,z):
    else if ((deltaX == 1) && (deltaY == 2) && (allowedDir & 0x0000FF))  {
      return_value = TRUE;
    }  // End of if/else block for (delX,delY,delZ) = (1,2,0)
  }  // End of

  // printf("DEBUG: Returned %d from function allowedDirection with allowedDir = %d, (x,y,z) = (%d,%d,%d),  and (delX,delY,delZ) = (%d,%d,%d)\n",
  //         return_value, allowedDir, x, y, z, deltaX, deltaY, deltaZ);

  // Return either TRUE or FALSE to calling routine:
  return(return_value);

}  // End of function 'allowedDirection'


//-----------------------------------------------------------------------------
// Name: calc_heuristic
// Desc: Calculate the heuristic function, H, which is an estimated cost between
//       the current position, (currentX, currentY, currentZ), and the target
//       position, (targetX, targetY, targetZ). Account for whether the cell
//       is in a pin-swap zone, where the heuristic is much smaller. The variable
//       routeDirections is a binary-encoded value with the following bit-field
//       definitions:
//
//            Bit-fields: | Up   Dn | N    S    E    W  | NE   SE   SW   NW |NxNE ExNE ExSE SxSE|SxSW WxSW WxNW NxNW|
//                        |---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|
// ANY         = 0x03FFFF | 1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  |
// NONE        = 0x000000 | 0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// ANY_LATERAL = 0x00FFFF | 0    0  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  |
// MANHATTAN   = 0x03F000 | 1    1  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// X_ROUTING   = 0x030F00 | 1    1  | 0    0    0    0  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  |
// NORTH_SOUTH = 0x03C000 | 1    1  | 1    1    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// EAST_WEST   = 0x033000 | 1    1  | 0    0    1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
// MANHATTAN_X = 0x03FF00 | 1    1  | 1    1    1    1  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  |
// UP_DOWN     = 0x030000 | 1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
//
//-----------------------------------------------------------------------------
unsigned long int calc_heuristic(const int currentX, const int currentY, const int currentZ,
                                 const int targetX, const int targetY, const int targetZ,
                                 const int routeDirections, InputValues_t *user_inputs,
                                 CellInfo_t ***const  cellInfo)  {

// As an experiment, the Dijkstra algorithm can be used (rather than A*) by returning
// a value of zero for the heuristic (for the H-value):
// return(0);

  int xDistance = abs(targetX - currentX);
  int yDistance = abs(targetY - currentY);
  int zDistance = abs(targetZ - currentZ);

  unsigned long int H_value;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // The Horizontal Pythagorean heuristic below is computationally expensive, but provides very good results.
  // Use this heuristic if any of the least significant 8 bits are set in the 'routeDirections' variable. In
  // other words, use this heuristic if any type of knight's move is allowed.
  //
  if (routeDirections & 0xFF)  {  // If routeDirections == ANY or ANY_LATERAL
    if (cellInfo[currentX][currentY][currentZ].swap_zone)  {
      H_value = (unsigned long)(sqrt(xDistance * xDistance  +  yDistance * yDistance) * user_inputs->pinSwapCellCost
                   + user_inputs->pinSwapCellCost * zDistance);
    }
    else  {
      float horizontal_distance = sqrt(xDistance * xDistance  +  yDistance * yDistance) * user_inputs->baseCellCost;
      unsigned long vertical_distance = zDistance * user_inputs->baseVertCost;

      H_value = (unsigned long)(horizontal_distance + vertical_distance);
    }  // End of if/else block for Pythagorean heuristic
  }  // End of else/if-block for 'ANY' routing direction

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // The octile heuristic below works well when N/S/E/W *and* NE/SE/SW/NW moves are allowed, but doesn't work well
  // for knight's moves:
  //
  else if ((routeDirections & 0xF000) && (routeDirections & 0x0F00))  {
    if (xDistance > yDistance)  {  // xDistance is greater than yDistance
      if (cellInfo[currentX][currentY][currentZ].swap_zone)  {
        // Octile heuristic for xDistance > yDistance in pin-swap zone:
        H_value = (unsigned long)(user_inputs->pinSwapDiagCost * yDistance
                   + user_inputs->pinSwapCellCost * (xDistance-yDistance)
                   + user_inputs->pinSwapCellCost * zDistance);
      }  // End of if-block for pin-swappable cell
      else  {
        // Octile heuristic for xDistance > yDistance in non-pin-swap zone:
        H_value = (unsigned long)(user_inputs->baseDiagCost * yDistance
                   + user_inputs->baseCellCost * (xDistance-yDistance)
                   + user_inputs->baseVertCost * zDistance);
      }  // End of else-block for non-pin-swapable zone
    }  // End of if-block for (xDistance > yDistance)

    else  {  // yDistance is greater than xDistance
      if (cellInfo[currentX][currentY][currentZ].swap_zone)  {
        // Octile heuristic for yDistance > xDistance in pin-swap zone:
        H_value = (unsigned long)(user_inputs->pinSwapDiagCost * xDistance
                   + user_inputs->pinSwapCellCost * (yDistance-xDistance)
                   + user_inputs->pinSwapCellCost * zDistance);
      }  // End of if-block for pin-swappable cell
      else  {
        //  Octile heuristic for yDistance > xDistance in non-pin-swap zone:
        H_value = (unsigned long)(user_inputs->baseDiagCost * xDistance
                   + user_inputs->baseCellCost * (yDistance-xDistance)
                   + user_inputs->baseVertCost * zDistance);
      }  // End of if/else-block for non-pin-swappable zone
    }  // End of if/else-block for octile heuristic
  }  // End of else/if-block for 'X_ROUTING' or 'MANHATTAN_X' routing directions

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // The rotated-Manhattan heuristic below works well for NE/SE/SW/NW moves:
  //
  else if (routeDirections & 0x0F00)  {
    if (cellInfo[currentX][currentY][currentZ].swap_zone)  {
      // Rotated-Manhattan heuristic in pin-swap zone:
      H_value = (unsigned long)(user_inputs->pinSwapDiagCost * max(xDistance, yDistance) + user_inputs->pinSwapCellCost * zDistance);
    }  // End of if-block for pin-swappable cell
    else  {
      // Rotated-Manhattan heuristic in non-pin-swap zone:
      H_value = (unsigned long)(user_inputs->baseDiagCost * max(xDistance, yDistance) + user_inputs->baseVertCost * zDistance);
    }  // End of if/else-block for non-pin-swappable zone
  }  // End of else/if-block for 'X_ROUTING' routing directions

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // The Manhattan heuristic below works well for north/south/east/west moves, and for up/down moves. Use this
  // heuristic if the least significant 12 bits are ZERO in the 'routeDirections' variable. (This heuristic is also
  // used if routeDirections == zero.
  //
  else if (! (routeDirections & 0x00FFF))  {  // routeDirections == MANHATTAN or NORTH_SOUTH or EAST_WEST or UP_DOWN
    if (cellInfo[currentX][currentY][currentZ].swap_zone)  {
      // Manhattan heuristic in pin-swap zone:
      H_value = (unsigned long)(user_inputs->pinSwapCellCost * (xDistance + yDistance) + user_inputs->pinSwapCellCost * zDistance);
    }  // End of if-block for pin-swappable cell
    else  {
      // Manhattan heuristic in non-pin-swap zone:
      H_value = (unsigned long)(user_inputs->baseCellCost * (xDistance + yDistance) + user_inputs->baseVertCost * zDistance);
    }  // End of if/else-block for non-pin-swappable zone
  }  // End of else/if-block for 'MANHATTAN' or 'NORTH_SOUTH' or 'EAST_WEST' or 'UP_DOWN' routing directions

  //
  // We got here, which is unexpected. Issue a fatal error message and exit:
  //
  else {
    printf("\n\nERROR: In function 'calc_heuristic', an illegal value of %d was encountered for variable 'routeDirections'\n",
           routeDirections);
    printf(    "       Please inform the software developer of this fatal error message.\n\n");
    exit(1);
  }


  return(H_value);

}  // End of function 'calc_heuristic'


//-----------------------------------------------------------------------------
// Name: record_DRC_by_index
// Desc: Set bit number 'index' in the stream of bytes that make up character
//       array 'DRCs[]'. Bit #0 is the least-significant bit in the first
//       element/byte of the array, bit #1 is the 2nd least significant bit,
//       bit #8 is the least-significant bit of the 2nd element/byte, etc, etc.
//-----------------------------------------------------------------------------
static void record_DRC_by_index(char DRCs[], int index)  {

  // printf("DEBUG: Entered function 'record_DRC_by_index' with index = %d.\n", index);

  int byte_number = index / 8;
  short remainder = index % 8;

  // Create a 1-byte mask based on the remainder value:
  //  remainder value       mask value (binary)
  //  ---------------       -------------------
  //         0         (msb)  0 0 0 0 0 0 0 1  (lsb)
  //         1                0 0 0 0 0 0 1 0
  //         2                0 0 0 0 0 1 0 0
  //         3                0 0 0 0 1 0 0 0
  //         4                0 0 0 1 0 0 0 0
  //         5                0 0 1 0 0 0 0 0
  //         6                0 1 0 0 0 0 0 0
  //         7                1 0 0 0 0 0 0 0
  unsigned char mask = 1 << remainder;

  // Set the appropriate bit of byte number 'byte_number' by
  // doing a binary OR operation of this byte's previous value
  // and the 'mask' value:
  // printf("DEBUG: Writing to element #%d of array DRCs[]. mask = %d\n", byte_number, mask);
  DRCs[byte_number] = DRCs[byte_number] | mask;

}  // End of function 'record_DRC_by_index'


//-----------------------------------------------------------------------------
// Name: calc_DRC_bit_index
// Desc: Calculate the index number for a 1-dimensional array of bits, given
//       a path number (path_1) and its shape-type (shapeType_1), plus an
//       offending path number (path_2) and its shape-type (shapeType_2).
//       Each combination of these 4 input variables is mapped to a unique
//       bit 'index', which is returned by this function.
//-----------------------------------------------------------------------------
static int calc_DRC_bit_index(unsigned int numPaths, int path_1, int shapeType_1,
                              int path_2, int shapeType_2)  {

  // printf("DEBUG: Entered function calc_DRC_bit_index with numPaths=%d, path_1=%d, shapeType_1=%d, path_2=%d, shape_type_2=%d\n",
  //        numPaths, path_1, shapeType_1, path_2, shapeType_2);

  // If path_1 is equal to path_2, then issue fatal error message and die:
  if (path_1 == path_2)  {
    printf("ERROR: The function 'calc_DRC_bit_index' was asked to evaluate a design-rule violation\n");
    printf("       between net #%d and net #%d. DRC violations between the same net are not possible,\n", path_1, path_2);
    printf("       so this represents a serious error in the software. Please report this message\n");
    printf("       to the software developer.\n\n");
    exit(1);
  }  // End of if-block for (path_1 == path_2)

  // If path_2 is greater than path_1, reverse the paths and corresponding shape-types:
  if (path_2 > path_1)  {
    int temp = path_1;
    path_1 = path_2;
    path_2 = temp;

    temp = shapeType_1;
    shapeType_1 = shapeType_2;
    shapeType_2 = temp;
  }  // End of if-block for (path_2 > path_1)

  // Calculate the index value:
  int index = 0;  // Initialize the value that will be returned.
  // The following for-loop calculates the index values in a 2x2 symmetric matrix above
  // the row corresponding to the DRC interaction:
  for (int row = 0; row <= path_2*NUM_SHAPE_TYPES + shapeType_2 - 1; row++)  {
    // Add to 'index' the number of cells that we skip before getting to the
    // cell of interest:
    index = index + NUM_SHAPE_TYPES * (numPaths - (row/3) - 1);
  }  // End of for-loop

  // Calculate the offset in the final row and add to 'index':
  index = index + NUM_SHAPE_TYPES * (path_1 - path_2 - 1)    +    shapeType_1;

  // printf("DEBUG: Exiting function calc_DRC_bit_index with index = %d\n", index);

  // Return 'index' to calling routine:
  return(index);

}  // End of function 'calc_DRC_bit_index'


//-----------------------------------------------------------------------------
// Name: record_DRC_by_paths
// Desc: Record the design-rule violation between path number (path_1) and its
//       shape-type (shapeType_1) and offending path number (path_2) and its
//       shape-type (shapeType_2).
//-----------------------------------------------------------------------------
void record_DRC_by_paths(int numPaths, char DRCs[], int path_1, int shapeType_1,
                         int path_2, int shapeType_2)  {

  // Determine the index based on the path numbers and shape-types:
  int index = calc_DRC_bit_index(numPaths, path_1, shapeType_1, path_2, shapeType_2);

  // Set the bit to '1' in the 'DRCs[]' array corresponding to the path numbers
  // and shape-types:
  record_DRC_by_index(DRCs, index);

}  // End of function 'record_DRC_by_paths'


//-----------------------------------------------------------------------------
// Name: read_DRC_by_index
// Desc: Read bit number 'index' in the stream of bytes that make up character
//       array 'DRCs[]'. Bit #0 is the least-significant bit in the first
//       element/byte of the array, bit #1 is the 2nd least significant bit,
//       bit #8 is the least-significant bit of the 2nd element/byte, etc, etc.
//-----------------------------------------------------------------------------
static unsigned char read_DRC_by_index(char DRCs[], int index)  {

  int byte_number = index / 8;
  short remainder = index % 8;

  // Create a 1-byte mask based on the remainder value:
  //  remainder value       mask value (binary)
  //  ---------------       -------------------
  //         0         (msb)  0 0 0 0 0 0 0 1  (lsb)
  //         1                0 0 0 0 0 0 1 0
  //         2                0 0 0 0 0 1 0 0
  //         3                0 0 0 0 1 0 0 0
  //         4                0 0 0 1 0 0 0 0
  //         5                0 0 1 0 0 0 0 0
  //         6                0 1 0 0 0 0 0 0
  //         7                1 0 0 0 0 0 0 0
  unsigned char mask = 1 << remainder;

  // Extract the appropriate bit of byte number 'byte_number' by
  // doing a binary AND operation of this byte's  value and
  // the 'mask' value. This AND operation returns a '1' if the
  // requested bit is '1', and returns '0' if the requested bit
  // is zero.
  // printf("DEBUG: In function 'read_DRC_by_index(),' accessing element %d of array DRCs[]. mask=%d\n", byte_number, mask);
  unsigned char result = (DRCs[byte_number] & mask) >> remainder;
  return(result);

}  // End of function 'read_DRC_by_index'


//-----------------------------------------------------------------------------
// Name: check_for_DRC
// Desc: Read the 'DRCs[]' array to determine whether a design-rule violation
//       has previously been recorded between path number (path_1) and its
//       shape-type (shapeType_1), and offending path number (path_2) and its
//       shape-type (shapeType_2).
//-----------------------------------------------------------------------------
unsigned char check_for_DRC(char DRCs[], unsigned int numPaths, int path_1, int shapeType_1,
                            int path_2, int shapeType_2)  {

  unsigned char result;

  // Determine the index based on the path numbers and shape-types:
  int index = calc_DRC_bit_index(numPaths, path_1, shapeType_1, path_2, shapeType_2);

  // Read the bit in the 'DRCs[]' array corresponding to the path numbers
  // and shape-types:
  result = read_DRC_by_index(DRCs, index);

  return(result);

}  // End of function 'check_for_DRC'


//-----------------------------------------------------------------------------
// Name: calcPathMetrics
// Desc: Calculate path-specific metrics like path length, via count, etc. Also
//       mark the centerlines of each path and via. If Boolean flag
//       'exitIfInvalidJump' is TRUE, the program will die if an invalid
//       jump is detected between segments. This should be the default behavior
//       when checking most paths, but not for sub-maps of diff-pair connections,
//       in which paths may exit and re-enter the sub-map.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calcPathMetrics' and re-compile if you want verbose debugging
// print-statements enabled:
//
// #define DEBUG_calcPathMetrics 1
#undef DEBUG_calcPathMetrics

void calcPathMetrics(const int total_nets, const InputValues_t *user_inputs,
                     const MapInfo_t *mapInfo, const int pathLengths[],
                     Coordinate_t *pathCoords[], CellInfo_t ***cellInfo,
                     RoutingMetrics_t *routability, int exitIfInvalidJump)  {

  #ifdef DEBUG_calcPathMetrics
  printf("\nDEBUG: In calcPathMetrics in iteration %d, map extends from (0,0,0) to (%d,%d,%d)\n",
         mapInfo->current_iteration, mapInfo->mapWidth - 1, mapInfo->mapHeight - 1, mapInfo->numLayers - 1);
  #endif

  //
  // Using the (non-contiguous) path array, calculate the length of each path.
  // Areas in pin-swappable zones do not count towards the length. Also, mark the
  // cells of each path as center-line cells.
  //
  for (int path = 0; path < total_nets; path++)  {  // Iterate through each path

    #ifdef DEBUG_calcPathMetrics
    printf("DEBUG: Calculating length of path %d in calcPathMetrics.\n", path);
    #endif

    // Initialize prevX/prevY/prevZ values to starting cell in current path:
    Coordinate_t prevCoord = copyCoordinates(mapInfo->start_cells[path]);

    // Check if the start-terminal is outside of the map's boundary, which can happen when
    // analyzing sub-maps. If the terminal is outside the map, then re-define the 'previous'
    // (x,y,z) coordinates as the first segment in the path that is within the map.
    int first_segment_to_analyze = 0;  // First segment to analyze that's inside the map
    for (int i = 0; i < pathLengths[path]; i++)  {
      if (pointIsOutsideOfMap(prevCoord, mapInfo))  {

        #ifdef DEBUG_calcPathMetrics
        printf("DEBUG: Near beginning of path %d, skipping coordinate at (%d,%d,%d) because it's outside the map.\n",
               path, prevCoord.X, prevCoord.Y, prevCoord.Z);
        #endif

        prevCoord = copyCoordinates(pathCoords[path][i]);

        // Increment the first segment that's found inside the map:
        first_segment_to_analyze++;
      }  // End if if-block for 'prevCoord' being outside the map
      else  {
        // We got here, so the 'prevCoord' is within the map.

        // Flag the first segment of the path as part of the path's center-line so it can
        // be displayed properly in PNG maps:
        cellInfo[prevCoord.X][prevCoord.Y][prevCoord.Z].center_line_flag = TRUE;  // <<<=== VALGRIND ERROR HERE!!!

        // Break out of the for-loop so we can use the 'prevCoord' as the initial
        // coordinate in the next for-loop.
        break;
      }
    }  // End of for-loop for index 'i' (0 to pathLength)

    #ifdef DEBUG_calcPathMetrics
    printf("DEBUG: After searching path %d for the first segment within the map, prevCoord = (%d,%d,%d)\n",
           path, prevCoord.X, prevCoord.Y, prevCoord.Z);
    printf("DEBUG:   Value of first_segment_to_analyze is %d. pathLengths[%d] is %d.\n",
           first_segment_to_analyze, path, pathLengths[path]);
    #endif


    //
    // Iterate through each segment of path 'path':
    //
    for (int i = first_segment_to_analyze; i < pathLengths[path]; i++)  {

      #ifdef DEBUG_calcPathMetrics
      printf("DEBUG: Iterating through segment #%d of path %d, with total path length %d.\n", i, path, pathLengths[path]);
      #endif

      // Extract X/Y/Z coordinates for this segment:
      int x = pathCoords[path][i].X;
      int y = pathCoords[path][i].Y;
      int z = pathCoords[path][i].Z;

      #ifdef DEBUG_calcPathMetrics
      printf("DEBUG:   Segment #%d of path %d has coordinates (%d,%d,%d).\n", i, path, x, y, z);
      #endif

      // Check if the path-segment is outside of the map, which can happen if this function is called
      // to assess paths for sub-maps. If the path-segment is outside of the map, then skip this segment:
      if (pointIsOutsideOfMap(pathCoords[path][i], mapInfo))  {
        // Re-define prevX/prevY/prevZ values to current cell and move on to next segment:
        #ifdef DEBUG_calcPathMetrics
        printf("DEBUG: SKIPPING segment %d of path %d because its coordinates (%d,%d,%d) are outside of the map.\n", i, path, x, y, z);
        #endif
        continue;
      }  // End of if-block for (x,y,z) being outside of the map

      // Flag the (x,y,z) location as the center of a trace/via so it can
      // be displayed correctly in maps:
      cellInfo[x][y][z].center_line_flag = TRUE;
      // printf("DEBUG: Set center_line_flag to TRUE at (%d,%d,%d)\n", x, y, z);

      // Also flag the locations of vias:
      if (z > prevCoord.Z)  {
        cellInfo[x][y][prevCoord.Z].center_viaUp_flag   = TRUE;
        cellInfo[x][y][     z     ].center_viaDown_flag = TRUE;
        #ifdef DEBUG_calcPathMetrics
        printf("DEBUG: Set center_viaUp_flag to TRUE at (%d,%d,%d)\n",   x, y, prevCoord.Z);
        printf("DEBUG: Set center_viaDown_flag to TRUE at (%d,%d,%d)\n", x, y, z);
        #endif
      }
      else if (z < prevCoord.Z)  {
        cellInfo[x][y][prevCoord.Z].center_viaDown_flag = TRUE;
        cellInfo[x][y][     z     ].center_viaUp_flag = TRUE;
        #ifdef DEBUG_calcPathMetrics
        printf("DEBUG: Set center_viaDown_flag to TRUE at (%d,%d,%d)\n", x, y, prevCoord.Z);
        printf("DEBUG: Set center_viaUp_flag to TRUE at (%d,%d,%d)\n", x, y, z);
        #endif
      }  // End of if/else/else block

      // Get indices of cost-multipliers for this cell:
      int trace_cost_multiplier_index   = cellInfo[x][y][z].traceCostMultiplierIndex;
      int viaUp_cost_multiplier_index   = cellInfo[x][y][z].viaUpCostMultiplierIndex;
      int viaDown_cost_multiplier_index = cellInfo[x][y][z].viaDownCostMultiplierIndex;

      // If cell is not in a pin-swappable zone, then count the length and
      // cost associated with this cell:
      if (! cellInfo[x][y][z].swap_zone)  {
        if (z > prevCoord.Z)  {
          // Cost of going 1 cell up (vertically):
          routability->path_cost[path] += user_inputs->vertCost[viaUp_cost_multiplier_index];
          routability->num_vias[path]++;            // Increment number of vias for this path
        }

        else if (z < prevCoord.Z)  {
          // Cost of going 1 cell down (vertically):
          routability->path_cost[path] += user_inputs->vertCost[viaDown_cost_multiplier_index];
          routability->num_vias[path]++;           // Increment number of vias for this path
        }

        else if ((abs(x - prevCoord.X) + abs(y - prevCoord.Y)) == 1)  {
          // Cost of going 1 cell north/S/E/W
          routability->path_cost[path] += user_inputs->cellCost[trace_cost_multiplier_index];
          routability->num_adjacent_steps[path]++ ;   // Increment number of 'adjacent' steps

          // printf("DEBUG:   Added %'lu cell cost to path %d. trace_cost_multiplier_index is %d.\n",
          //        user_inputs->cellCost[trace_cost_multiplier_index], path, trace_cost_multiplier_index);
          // printf("         New value of routability->path_cost[%d] is %'lu\n", path, routability->path_cost[path]);
          // printf("DEBUG:   routability->num_adjacent_steps[%d] is now %d\n", path, routability->num_adjacent_steps[path]);
        }

        else if ((abs(x - prevCoord.X) == 1) && (abs(y - prevCoord.Y) == 1))  {
          // Cost of going 1 cell diagonally
          routability->path_cost[path] += user_inputs->diagCost[trace_cost_multiplier_index];
          routability->num_diagonal_steps[path]++;   // Increment number of 'diagonal' steps

          // printf("DEBUG:   Added %'lu cell cost to path %d. trace_cost_multiplier_index is %d.\n",
          //        user_inputs->diagCost[trace_cost_multiplier_index], path, trace_cost_multiplier_index);
          // printf("         New value of routability->path_cost[%d] is %'lu\n", path, routability->path_cost[path]);
          // printf("DEBUG:   routability->num_diagonal_steps[%d] is now %d\n", path, routability->num_diagonal_steps[path]);
        }

        else if (   ((abs(x - prevCoord.X) == 2) && (abs(y - prevCoord.Y) == 1))
                 || ((abs(x - prevCoord.X) == 1) && (abs(y - prevCoord.Y) == 2)))  {
          // Cost of going 2 cells in one direction and 1 cell sideways.
          routability->path_cost[path] += user_inputs->knightCost[trace_cost_multiplier_index];
          routability->num_knights_steps[path]++;       // Increment number of 'knights' steps

          // printf("DEBUG:   Added %'lu cell cost to path %d. trace_cost_multiplier_index is %d.\n",
          //        user_inputs->knightCost[trace_cost_multiplier_index], path, trace_cost_multiplier_index);
          // printf("         New value of routability->path_cost[%d] is %'lu\n", path, routability->path_cost[path]);
          // printf("DEBUG:   routability->num_knights_steps[%d] is now %d\n", path, routability->num_knights_steps[path]);
        }

        else if (exitIfInvalidJump) {
          printf("\n\nERROR: For path %d, two adjacent points (#%d and #%d) are separated by an illegal distance. The two points\n",
                 path, i-1, i);
          printf(    "       have coordinates (%d,%d,%d) and (%d,%d,%d). Please inform the software developer\n",
                 prevCoord.X, prevCoord.Y, prevCoord.Z, x, y, z);
          printf(    "       of this fatal error message.\n\n");
          exit(1);
        }  // End of if/else/else/else block

      }  // End of if-block for (! swap_zone)

      // Re-define prevX/prevY/prevZ values to current cell:
      prevCoord = copyCoordinates(pathCoords[path][i]);

    }  // End of for-loop for i (0 to pathLength)

    #ifdef DEBUG_calcPathMetrics
    printf("DEBUG: In 'calcPathMetrics,' path %d has cost %'lu.\n",
                          path, routability->path_cost[path]);
    #endif

    // Calculate the path length in millimeters for the current path:
    routability->lateral_path_lengths_mm[path] = user_inputs->cell_size_um *
                                          (routability->num_adjacent_steps[path]
                                            + routability->num_diagonal_steps[path] * sqrt(2)
                                             + routability->num_knights_steps[path] * sqrt(5)) / 1000;

    #ifdef DEBUG_calcPathMetrics
    printf("DEBUG: For path %d, lateral_path_length_mm is %7.3f mm\n", path,
           routability->lateral_path_lengths_mm[path]);
    printf("DEBUG: This is based on 'cell_size_um' (%7.3f), num_adjacent_steps[%d] (%d)\n",
           user_inputs->cell_size_um, path, routability->num_adjacent_steps[path]);
    printf("DEBUG: num_diagonal_steps[%d] (%d), num_knights_steps[%d] (%d), sqrt(2) (%7.3f), and squrt(5) (%7.3f).\n",
           path, routability->num_diagonal_steps[path], path, routability->num_knights_steps[path], sqrt(2), sqrt(5));
    #endif

    // Sum the lengths of each path together:
    if (user_inputs->isPseudoNet[path])  {
      routability->total_lateral_pseudo_length_mm += routability->lateral_path_lengths_mm[path];
      routability->total_pseudo_cost              += routability->path_cost[path];
      routability->total_pseudo_vias              += routability->num_vias[path];
    }
    else  {
      routability->total_lateral_nonPseudo_length_mm += routability->lateral_path_lengths_mm[path];
      routability->total_nonPseudo_cost              += routability->path_cost[path];
      routability->total_nonPseudo_vias              += routability->num_vias[path];
    }

    routability->total_cost              += routability->path_cost[path];
    routability->total_lateral_length_mm += routability->lateral_path_lengths_mm[path];
    routability->total_vias              += routability->num_vias[path];

  }  // End of for-loop for variable 'path' (0 to total_nets)


  #ifdef DEBUG_calcPathMetrics
  printf("DEBUG: At end of calcPathMetrics in iteration %d, total_cost is %'lu.\n", mapInfo->current_iteration, routability->total_cost);
  printf("DEBUG:           total_lateral_length_mm is %7.3f mm.\n", routability->total_lateral_length_mm);
  printf("DEBUG:    total_lateral_pseudo_length_mm is %7.3f mm.\n", routability->total_lateral_pseudo_length_mm);
  printf("DEBUG: total_lateral_nonPseudo_length_mm is %7.3f mm.\n", routability->total_lateral_nonPseudo_length_mm);
  printf("DEBUG:                        total_vias is %d vias.\n", routability->total_vias);
  printf("DEBUG:                 total_pseudo_vias is %d vias.\n", routability->total_pseudo_vias);
  printf("DEBUG:              total_nonPseudo_vias is %d vias.\n", routability->total_nonPseudo_vias);
  #endif

}  // End of function 'calcPathMetrics'


//-----------------------------------------------------------------------------
// Name: markCellsNearCenterlinesInMap
// Desc: Flag cells that are near the centers of (contiguous) paths, so we can avoid
//       other cells when checking design rules. The 'near_a_net' element in the
//       cellInfo 3D matrix is set to TRUE for cells that are near a path-center.
//
//       Note: this function can be parallelized into multiple threads if the CPU
//       architecture allows atomic writes to single-bit elements of a structure.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_markCellsNearCenterlinesInMap' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_markCellsNearCenterlinesInMap 1
#undef DEBUG_markCellsNearCenterlinesInMap

void markCellsNearCenterlinesInMap(const int total_nets, const MapInfo_t *mapInfo, int contiguousPathLength[],
                                   Coordinate_t *contigPathCoords[], const InputValues_t *user_inputs,
                                   CellInfo_t ***cellInfo)  {



  #ifdef DEBUG_markCellsNearCenterlinesInMap
  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);
  printf("Date-stamp before flagging path centerlines and cells near paths: %02d-%02d-%d, %02d:%02d:%02d *************\n",
         now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

  // In DEBUG mode, keep track of how many cells are flagged by this function:
  int num_cells_near_centerlines = 0;
  #endif

  // Iterate over all paths:
  for (int pathNum = 0; pathNum < total_nets; pathNum++)  {
    // printf("  DEBUG: Contiguous path number %d with length %d segments...\n", pathNum, contiguousPathLength[pathNum]);
    // Iterate through each segment of path  #pathNum

    // Iterate over all segments of the contiguous path:
    for (int pathSegment = 0; pathSegment < contiguousPathLength[pathNum]; pathSegment++)  {
      // printf("    DEBUG: segment number %d...\n", pathSegment);

      // Check if the path-segment is outside of the map, which can happen if this function is called
      // for sub-maps of diff-pair connections. If the path-segment is outside of the map, then skip
      // this segment:
      if (pointIsOutsideOfMap(contigPathCoords[pathNum][pathSegment], mapInfo))  {
        // Move on to next segment:
        continue;
      }  // End of if-block for (x,y,z) being outside of the map

      // Get x/y/z locations of current segment of current path:
      int x     = contigPathCoords[pathNum][pathSegment].X;
      int y     = contigPathCoords[pathNum][pathSegment].Y;
      int layer = contigPathCoords[pathNum][pathSegment].Z;

      //
      // If cell is not in a pin-swappable region, flag cells that are
      // within 'maxInteractionRadiusCellsonLayer' of the center of the contiguous
      // path. Any cell that lacks this flag will not be checked for
      // design-rule violations, thereby saving time during design-rule checking.
      //
      // Check if path-segment is in a pin-swap zone. If so, then skip this segment:
      if (! cellInfo[x][y][layer].swap_zone)  {
        int interactionRadius        = max(1, mapInfo->maxInteractionRadiusCellsOnLayer[layer]);
        int interactionRadiusSquared = max(1, mapInfo->maxInteractionRadiusSquaredOnLayer[layer]);

        // Raster over a square centered at (x,y,z), with a maximum distance from the center
        // being 'maxInteractionRadiusCellsOnLayer[layer_num]':
        for (int x_prime = x - interactionRadius; x_prime <= x + interactionRadius; x_prime++)  {

          const int delta_x_squared = (x - x_prime) * (x - x_prime);

          for (int y_prime = y - interactionRadius; y_prime <= y + interactionRadius; y_prime++)  {

            // If x_prime or y_prime is outside of the map or unwalkable, then move on to next point:
            if (XY_coords_are_outside_of_map(x_prime, y_prime, mapInfo) || cellInfo[x_prime][y_prime][layer].forbiddenTraceBarrier)
              continue;

            int distance_squared = delta_x_squared   +   (y - y_prime) * (y - y_prime);

            // Check whether path-segement is within distance 'maxInteractionRadius' that's unique for current
            // layer:
            if (distance_squared <= interactionRadiusSquared)  {

              // We got here, so cell at (x',y',z) is within 'maxInteractionRadius' of the path's
              // center-line for layer 'layer'. Set the 'near_a_net' flag for this cell:
              #ifdef DEBUG_markCellsNearCenterlinesInMap
              // In DEBUG mode, count how many cells are flagged by this function:
              if (! cellInfo[x_prime][y_prime][layer].near_a_net)  {
                num_cells_near_centerlines++;
              }
              #endif

              cellInfo[x_prime][y_prime][layer].near_a_net = TRUE;

            }  // End of if-block for distance^2 <= radius^2
          }  // End of for-loop for index y_prime
        }  // End of for-loop for index x_prime
      }  // End of if-block for (! swap_zone)
    }  // End of for-loop for pathSegment = 0 to contiguousPathLength
  }  // End of for-loop for pathNum = 0 to total_nets

  #ifdef DEBUG_markCellsNearCenterlinesInMap
  // In DEBUG mode, report how many cells are flagged by this function:
  printf("\nDEBUG: In function markCellsNearCenterlinesInMap in iteration %d, the number of cells\n", mapInfo->current_iteration);
  printf(  "DEBUG: flagged for being close to path center-lines is %'d.\n\n", num_cells_near_centerlines);

  tim = time(NULL); now = localtime(&tim);
  printf("Date-stamp after flagging path centerlines and cells near paths: %02d-%02d-%d, %02d:%02d:%02d *************\n",
          now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  #endif

}  // End of function 'markCellsNearCenterlinesInMap'


//-----------------------------------------------------------------------------
// Name: markPathCenterlinesInMap
// Desc: Use the contiguous path array to mark the x/y/z locations of every path
//       and via. The path-number and shape-type are stored in each cell that
//       the path traverses.
//
//       Also add extra congestion to the path-center cells in order
//       to repel foreign nets from crossing the path-centers.
//
//       Note: this function cannot be parallelized into multiple threads (one
//       for each path) because function 'add_path_center_info' re-allocates
//       memory on the heap, and multiple paths might access the same cell
//       at the same (x,y,z) coordinate.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_markPathCenterlinesInMap' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_markPathCenterlinesInMap 1
#undef DEBUG_markPathCenterlinesInMap

void markPathCenterlinesInMap(const int total_nets, int contiguousPathLength[],
                              Coordinate_t *contigPathCoords[], CellInfo_t ***cellInfo,
                              const MapInfo_t *mapInfo, const RoutingMetrics_t *routability,
                              const InputValues_t *user_inputs)  {

  #ifdef DEBUG_markPathCenterlinesInMap
  printf("\nDEBUG: Entered function markPathCenterlinesInMap in iteration %d...\n", mapInfo->current_iteration);
  #endif

  // Iterate over all path numbers:
  for (int pathNum = 0; pathNum < total_nets; pathNum++)  {
    #ifdef DEBUG_markPathCenterlinesInMap
    printf("  DEBUG: Contiguous path number %d with length %d segments...\n", pathNum, contiguousPathLength[pathNum]);
    #endif

    // Define the amount of *additional* congestion to deposit in cells with traversing path-centers.
    // The amount is higher than a standard amount by a factor of 2, or 2^1:

//// The following assignment statement was commented out 6/22/2024 and replaced with the subsequent statement:
////unsigned pathCenter_congestion_amount = routability->one_path_traversal[pathNum] << 1;
////unsigned pathCenter_congestion_amount = (int)(routability->one_path_traversal[pathNum]
////                                               * 2 * mapInfo->iterationDependentRatio);
////unsigned pathCenter_congestion_amount = (int)(0.1 * ONE_TRAVERSAL * mapInfo->iterationDependentRatio);
////unsigned pathCenter_congestion_amount = (int)(0.05 * ONE_TRAVERSAL);

    unsigned pathCenter_congestion_amount = (int)(0.10 * ONE_TRAVERSAL);


////unsigned pathCenter_congestion_amount = 1;  // This is only 1% of a typical congestion amount!!



    // Define variables to hold previous/next coordinates. Initialize the previous
    // coordinates to nonsense values to avoid compile-time warnings:
    int prev_X = -99, prev_Y = -99;
    int prev_Z= -99, next_Z;

    // Iterate over all segments of the contiguous path:
    for (int pathSegment = 0; pathSegment < contiguousPathLength[pathNum]; pathSegment++)  {
      #ifdef DEBUG_markPathCenterlinesInMap
      printf("    DEBUG: segment number %d...\n", pathSegment);
      #endif

      // Check if the path-segment is outside of the map, which can happen if this function is called
      // for sub-maps of diff-pair connections. If the path-segment is outside of the map, then skip
      // this segment:
      // printf("DEBUG: In function markPathCenterlinesInMap, checking if (%d,%d,%d) is within the map with dimensions %d x %d x %d cells\n",
      //        contigPathCoords[pathNum][pathSegment].X, contigPathCoords[pathNum][pathSegment].Y, contigPathCoords[pathNum][pathSegment].Z,
      //        mapInfo->mapWidth, mapInfo->mapHeight, mapInfo->numLayers);
      if (pointIsOutsideOfMap(contigPathCoords[pathNum][pathSegment], mapInfo))  {
        // Move on to next segment:
        continue;
      }  // End of if-block for (x,y,z) being outside of the map

      // Get x/y/z locations of current segment of current path:
      int x = contigPathCoords[pathNum][pathSegment].X;
      int y = contigPathCoords[pathNum][pathSegment].Y;
      int z = contigPathCoords[pathNum][pathSegment].Z;

      // Initialize the Boolean variables that record whether the current segment at (x,y,z)
      // is a TRACE, VIA_UP, and/or VIA_DOWN. (These are not mutually exclusive.)
      int trace   = FALSE;
      int upVia   = FALSE;
      int downVia = FALSE;

      // Determine whether the segment is a TRACE, a VIA-UP, and/or a VIA-DOWN. The
      // first and last segments are special cases because they lack segments on
      // both sides. We use the __builtin_expect compiler directive to tell compiler
      // that it's unlikely that pathSegment will equal 0 or equal the last segment
      // of path. The logic below must accommodate paths that are only 1 segment long,
      // because sub-maps will have only their start-terminals.
      #ifdef DEBUG_markPathCenterlinesInMap
      printf("DEBUG: In markPathCenterlinesInMap: pathNum = %d, pathSegment = %d before calculating next_Z, prev_Z\n", pathNum, pathSegment);
      printf("DEBUG:                            : contiguousPathLength[pathNum] = %d\n", contiguousPathLength[pathNum]);
      #endif
      if (__builtin_expect(contiguousPathLength[pathNum] == 1, 0))  {
        // Handle the rare case of paths that contain only 1 segment:
        trace   = TRUE;
        upVia   = FALSE;
        downVia = FALSE;
        #ifdef DEBUG_markPathCenterlinesInMap
        printf("      DEBUG: Segment is TRACE because path has only 1 segment.\n");
        #endif
      }  // End of if-block for pathLength == 1
      else if (__builtin_expect(pathSegment == 0, 0))  {
        // Handle the initial segment in the array, for which there is no previous segment:
        next_Z = contigPathCoords[pathNum][pathSegment+1].Z;
        if (z == next_Z) {
          trace = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is TRACE because it's the first segment and next segment is on same layer.\n");
          #endif
        }
        else if (z < next_Z)  {
          upVia = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is upVia because it's the first segment and next segment is on higher layer.\n");
          #endif
        }
        else  {
          downVia = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is downVia because it's the first segment and next segment is on lower layer.\n");
          #endif
        }
      }  // End of if-block for pathSegment == 0
      else if (__builtin_expect(pathSegment == contiguousPathLength[pathNum] - 1, 0))  {
        // Handle the final segment in the array, for which there is no subsequent segment:
        prev_Z = contigPathCoords[pathNum][pathSegment-1].Z;
        if (z == prev_Z) {
          trace = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is TRACE because it's the last segment and previous segment is on same layer.\n");
          #endif
        }
        else if (z < prev_Z)  {
          upVia = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is upVia because it's the last segment and previous segment is on higher layer.\n");
          #endif
        }
        else  {
          downVia = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is downVia because it's the last segment and previous segment is on lower layer.\n");
          #endif
        }
      }  // End of if-block for pathSegment being the last segment in path
      else  {
        // We got here, so we're somewhere in the middle of the path, with
        // segments on either side of (or above or below) the current segment.
        prev_Z = contigPathCoords[pathNum][pathSegment-1].Z;
        next_Z = contigPathCoords[pathNum][pathSegment+1].Z;
        if ((z == prev_Z) || (z == next_Z))  {
          trace = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is TRACE because it's in path's middle and at least one adjacent segment is on same layer.\n");
          #endif
        }
        if ((z < prev_Z) || (z < next_Z))  {
          upVia = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is upVia because it's in path's middle and at least one adjacent segment is on higher layer.\n");
          #endif
        }
        if ((z > prev_Z) || (z > next_Z))  {
          downVia = TRUE;
          #ifdef DEBUG_markPathCenterlinesInMap
          printf("      DEBUG: Segment is downVia because it's in path's middle and at least one adjacent segment is on lower layer.\n");
          #endif
        }
      }  // End of else-block for being in middle of path

      #ifdef DEBUG_markPathCenterlinesInMap
      printf("      DEBUG: Final result: trace = %d. upVia = %d. downVia = %d\n", trace, upVia, downVia);
      #endif


      //
      // In anticipation of adding congestion to path-center cells, get the design-rule set
      // and subset for this path at the current x/y/z location:
      int pathCenter_DR_set = cellInfo[x][y][z].designRuleSet;
      int pathCenter_DR_subset = user_inputs->designRuleSubsetMap[pathNum][pathCenter_DR_set];

      //
      // If the current segment is on the same routing layer as the previous segment, but is
      // not contiguous to the previous segment, then add congestion to the intervening 2 cells:
      //
      if ((pathSegment > 0) && (z == prev_Z))  {
        if (abs(x - prev_X) + abs(y - prev_Y) == 2)  {
          // Current (x,y,z) is diagonal from previous (x,y,z). Add congestion to cell at
          // location (x, prev_Y, z), which is north/south of original point at (x,y,z):
          addCongestion(&(cellInfo[x][prev_Y][z]), pathNum, pathCenter_DR_subset, TRACE,
                                                        pathCenter_congestion_amount);
        }  // End of if-block for diagonal cell
        else if ((abs(x - prev_X) == 2) && (abs(y - prev_Y) == 1))  {
          // Current (x,y,z) is a knight's move from previous (x,y,z),
          // with deltaX = 2 and deltaY = 1. Add 2 intermediate points:
          //
          //      ----------   s = start = previous (pX,pY)
          //   y  |  |i2| e|   e = end   = (x, y)
          //      ----------  i1 = 1st intermediate point = ((pX+x)/2,pY)
          //   pY |s |i1|  |  i2 = 2nd intermediate point = (x from i1, y)
          //      ----------
          //       pX    x

          // Add congestion to first intermediate point east/west of previous point (pX,pY):
          int temp_X = (x + prev_X) / 2;
          addCongestion(&(cellInfo[temp_X][prev_Y][z]), pathNum, pathCenter_DR_subset, TRACE,
                                                        pathCenter_congestion_amount);

          // Add congestion to second intermediate point east/west from current point (x,y):
          addCongestion(&(cellInfo[temp_X][y][z]), pathNum, pathCenter_DR_subset, TRACE,
                                                        pathCenter_congestion_amount);
        }  // End of else-block for knight's move left/right


        else if ((abs(x - prev_X) == 1) && (abs(y - prev_Y) == 2))  {
          // Current (x,y,z) is a knight's move from previous (x,y,z),
          // with deltaX = 1 and deltaY = 2. Add 2 intermediate points:
          //
          //      -------
          //   y  |  | e|    s = start = previous (pX,pY)
          //      -------    e = end   = (x, y)
          //      |i1|i2|   i1 = 1st intermediate point = (pX,(y+pY)/2)
          //      -------   i2 = 2nd intermediate point = (x, y from i1)
          //   pY |s |  |
          //      -------
          //       pX  x

          // Add congestion to first intermediate point north/south of previous point (pX,pY):
          int temp_Y = (y + prev_Y) / 2;
          addCongestion(&(cellInfo[prev_X][temp_Y][z]), pathNum, pathCenter_DR_subset, TRACE,
                                                        pathCenter_congestion_amount);

          // Add congestion to second intermediate point north/south from current point (x,y):
          addCongestion(&(cellInfo[x][temp_Y][z]), pathNum, pathCenter_DR_subset, TRACE,
                                                        pathCenter_congestion_amount);
        }  // End of else-block for knight's move up/down
      }  // End of if-block for (z == prev_Z)

      //
      // For the path-center at this x/y/z location, add the path and shape-type information to the pathCenters array.
      // Also add additional congestion to the path-center cell, making foreign paths less likely to cross this path-center:
      //
      if (trace)  {
        // Add path-number and shape-type to path-center array:
        add_path_center_info(&(cellInfo[x][y][z]), pathNum, TRACE);

        // Add congestion along the centerline of the trace:
        addCongestion(&(cellInfo[x][y][z]), pathNum, pathCenter_DR_subset, TRACE,
                                                      pathCenter_congestion_amount);

        #ifdef DEBUG_markPathCenterlinesInMap
        printf("        DEBUG: Added path-center information to (%d,%d,%d): path #%d for TRACE\n", x, y, z, pathNum);
        #endif
      }
      if (upVia) {
        // Add path-number and shape-type to path-center array:
        add_path_center_info(&(cellInfo[x][y][z]), pathNum, VIA_UP);

//// The following line was added 7/4/2024 as a temporary experiment:
////    // Add congestion for via:
        addCongestion(&(cellInfo[x][y][z]), pathNum, pathCenter_DR_subset, VIA_UP,
                                                      pathCenter_congestion_amount);

        #ifdef DEBUG_markPathCenterlinesInMap
        printf("        DEBUG: Added path-center information to (%d,%d,%d): path #%d for VIA_UP\n", x, y, z, pathNum);
        #endif
      }
      if (downVia)  {
        // Add path-number and shape-type to path-center array:
        add_path_center_info(&(cellInfo[x][y][z]), pathNum, VIA_DOWN);


//// The following line was added 7/4/2024 as a temporary experiment:
////    // Add congestion for via:
        addCongestion(&(cellInfo[x][y][z]), pathNum, pathCenter_DR_subset, VIA_DOWN,
                                                      pathCenter_congestion_amount);

        #ifdef DEBUG_markPathCenterlinesInMap
        printf("        DEBUG: Added path-center information to (%d,%d,%d): path #%d for VIA_DOWN\n", x, y, z, pathNum);
        #endif
      }

      // In anticipation of the next iteration through this for-loop, assign the previous (x,y,z)
      // coordinates as the current coordinates for the current iteration:
      prev_X = x;
      prev_Y = y;
      prev_Z = z;

    }  // End of for-loop for pathSegment = 0 to contiguousPathLength
  }  // End of for-loop for pathNum = 0 to total_nets

  #ifdef DEBUG_markPathCenterlinesInMap
  printf("\nDEBUG: Exiting function markPathCenterlinesInMap in iteration %d...\n\n", mapInfo->current_iteration);
  #endif

}  // End of function 'markPathCenterlinesInMap'


//-----------------------------------------------------------------------------
// Name: calc_fraction_of_recent_iterations_with_DRCs
// Desc: Return the floating-point fraction (from 0 to 1.0) of the number of
//       recent iterations that contained any design-rule violations for
//       the path whose 'recent_path_DRC_cells' array is given among the input
//       parameters over 'num_iterations' iterations. The maximum allowed
//       value for 'num_iterations' is numIterationsToReEquilibrate.
//-----------------------------------------------------------------------------
float calc_fraction_of_recent_iterations_with_DRCs(unsigned int * recent_path_DRC_cells,
                                                   int num_iterations)  {
  float fraction_iterations_with_DRCs = 0.0;

  // Check if 'num_iterations' is too big:
  if (num_iterations > numIterationsToReEquilibrate)  {
    printf("\nERROR: Function calc_fraction_of_recent_iterations_with_DRCs detected an error in its\n");
    printf(  "       input values: the 'num_iterations' parameter has a value of %d, but the allowed\n", num_iterations);
    printf(  "       range is 0 to %d, inclusive. Please notify the software developer of this fatal\n", numIterationsToReEquilibrate);
    printf(  "       error message.\n\n");
    exit(1);
  }

  // Iterate over the recent iterations to determine how many had design-rule violations:
  int num_iterations_with_DRCs = 0;
  for (int iter = 0; iter < num_iterations; iter++)  {
    if (recent_path_DRC_cells[iter])  {
      // We got here, so the path had at least one DRC during iteration 'iter':
      num_iterations_with_DRCs++;
    }  // End of if-block
  }  // End of for-loop

  // Calculate the fraction of recent iterations with DRCs, and return the
  // fraction to the calling routine:
  fraction_iterations_with_DRCs = (float) num_iterations_with_DRCs / num_iterations;
  return(fraction_iterations_with_DRCs);

}  // End of function 'calc_fraction_of_recent_iterations_with_DRCs'


//-----------------------------------------------------------------------------
// Name: determineIfMetricsPlateaued
// Desc: Determine whether the routing metrics reached a plateau. Such a
//       plateau is defined if the following criteria are satisfied:
//       (1) The slope and standard deviation are both exactly zero for the
//           non-pseudo path costs over the 10 most recent iterations
//
//        or:
//
//       (2a) The standard deviation of the non-pseudo path costs over the 10 most
//            recent iterations is less than 2x the standard deviation of
//            iteration i - 10, and
//       (2b) The absolute value of the slope of the non-pseudo path costs over
//            the 10 most recent iterations is <= 0.1%/iteration, and is
//            <= 0.2%/iteration at iteration i - 10.
//
//       This function modifies the following variable
//            routability->inMetricsPlateau[mapInfo->current_iteration]
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_determineIfMetricsPlateaued' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_determineIfMetricsPlateaued 1
#undef DEBUG_determineIfMetricsPlateaued

void determineIfMetricsPlateaued(const MapInfo_t *mapInfo, RoutingMetrics_t *routability)  {

  #ifdef DEBUG_determineIfMetricsPlateaued
  // DEBUG code follows:
  //
  // Variable used for debugging:
  int DEBUG_ON = FALSE;

  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  if ((mapInfo->current_iteration >= 1) && (mapInfo->current_iteration <= 2))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in determineIfMetricsPlateaued() because specific requirements were met in iteration %d.\n\n",
           mapInfo->current_iteration);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif


  //
  // Calculate the slope and standard deviation of the non-pseudo path costs for the 10 most recent iterations:
  //
  if (mapInfo->current_iteration >= 10)  {

    // For the 10 most recent iterations, create the sums that are necessary to calculate
    // the standard deviation and the slope:
    double sum_x         = 0.0;  // Sum of the X-values (iteration numbers)
    double sum_x_squared = 0.0;  // Sum of the square of the X-values
    double sum_y         = 0.0;  // Sum of the Y-values (routing cost)
    double sum_y_squared = 0.0;  // Sum of the square of the Y-values
    double sum_xy        = 0.0;  // Sum of the product of the X- and Y-values
    char   all_y_values_are_same = TRUE;  // Boolean flag that's TRUE only if all
                                          // 10 Y-values are identical.
    for (int i = mapInfo->current_iteration - 9; i <= mapInfo->current_iteration; i++)  {
      sum_x         += (double)i;
      sum_x_squared += (double)(i * i);
      sum_y         += (double)routability->nonPseudoPathCosts[i];
      sum_y_squared += (double)routability->nonPseudoPathCosts[i] * (double)routability->nonPseudoPathCosts[i];
      sum_xy        += (double)i * (double)routability->nonPseudoPathCosts[i];
      if (   (all_y_values_are_same)
          && (routability->nonPseudoPathCosts[i] != routability->nonPseudoPathCosts[mapInfo->current_iteration]))  {
        all_y_values_are_same = FALSE;
      }
      // printf("DEBUG:     (%d, %'lu) = (iteration, cost)\n", i, routability->nonPseudoPathCosts[i]);
    }  // End of for-loop for index 'i'

    // If all the y-values are identical, then set the standard deviation and the slope to zero:
    if (all_y_values_are_same)  {
      routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration] = 0.0;
      routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration]  = 0.0;
    }
    else  {
      // Use the above sums to calculate the standard deviation (in units of routing cost):
      routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration]
                                        = sqrt((sum_y_squared - sum_y * sum_y / 10.0) / 10.0);

      // Use the above sums to calculate the slope, in units of 'routing cost per iteration':
      routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration]
                                        = (10.0 * sum_xy  -  sum_x * sum_y) / (10.0 * sum_x_squared - sum_x * sum_x);

      // Normalize the slope by dividing by the average routing cost from the most recent
      // 10 iterations. The resulting, normalized slope has units of 'per iteration':
      routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration]
                                     /= (sum_y / 10.0);

      // If either of the above two calculated values is 'NaN' (not a number), then define the parameter as zero:
      if (isnan(routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration]))  {
        routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration] = 0.0;
      }
      if (isnan(routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration]))  {
        routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration] = 0.0;
      }

    }  // End of if/else-block for all_y_values_are_same == TRUE

    // printf("\nDEBUG: For iteration %d:\n", mapInfo->current_iteration);
    // printf("DEBUG:         sum_y_squared = %.8E,    sum_y * sum_y / 10 = %.8E\n\n", sum_y_squared, sum_y * sum_y / 10.0);
  }  // End of if-block for current_iteration >= 10
  else  {
    // We got here, so the current iteration has not yet reached 10. We therefore cannot calculate
    // statistics for the trailing iteration, and we therefore set the standard deviation and slope
    // to zero:
    routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration] = 0.0;
    routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration]  = 0.0;
  }  // End of else block for current_iteration < 10

  #ifdef DEBUG_determineIfMetricsPlateaued
  if (DEBUG_ON)  {
    printf("\nDEBUG: For iteration %d:\n", mapInfo->current_iteration);
    printf("DEBUG:    nonPseudoPathCosts_stdDev_trailing_10_iterations = %.4E\n", routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration]);

    if (mapInfo->current_iteration >= 30)  {
      printf("DEBUG:                                   10 iterations ago = %.4E\n", routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration - 10]);
    }

    printf("DEBUG:    nonPseudoPathCosts_slope_trailing_10_iterations = %.4E\n",   routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration]);
    if (mapInfo->current_iteration >= 30)  {
      printf("DEBUG:                                  10 iterations ago = %.4E\n",   routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration - 10]);
    }
  }
  #endif

  // Initialize the 'inMetricsPlateau' to FALSE for the current iteration. This might
  // be changed to TRUE in the subsequent code, if the necessary criteria are met:
  routability->inMetricsPlateau[mapInfo->current_iteration] = FALSE;

  //
  // Determine whether we've reached a plateau in the routing metrics for iteration 'i'. Such a
  // plateau is defined if the following criteria are satisfied:
  //   (1) The slope and standard deviation are both exactly zero for the
  //       non-pseudo path costs over the 10 most recent iterations
  //
  //    or:
  //
  //  (2a) The standard deviation of the non-pseudo path costs over the 10 most
  //       recent iterations is less than 2x the standard deviation of
  //       iteration i - 10, and
  //  (2b) The absolute value of the slope of the non-pseudo path costs over
  //       the 10 most recent iterations is <= 0.1%/iteration, and is
  //       <= 0.2%/iteration at iteration i - 10.
  //
  if (   ( (mapInfo->current_iteration >= 10)
            && (routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration] == 0.0)
            && (routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration] == 0.0))
      || ( (mapInfo->current_iteration >= 20)
          && (          routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration]
               <= 2.0 * routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[mapInfo->current_iteration - 10])
          && (fabs(routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration])      <= 0.001)
          && (fabs(routability->nonPseudoPathCosts_slope_trailing_10_iterations[mapInfo->current_iteration - 10]) <= 0.002)))  {

    routability->inMetricsPlateau[mapInfo->current_iteration] = TRUE;

  }  // End of if-block for satisfying all criteria of being in a metric plateau

  #ifdef DEBUG_determineIfMetricsPlateaued
  if (DEBUG_ON)  {
    printf("\nDEBUG: For iteration %d, inMetricsPlateau = %d\n\n", mapInfo->current_iteration, routability->inMetricsPlateau[mapInfo->current_iteration]);
  }
  #endif

}  // End of function 'determineIfMetricsPlateaued'


//-----------------------------------------------------------------------------
// Name: calc_distance_G_cost
// Desc: Calculate the distance component of the G-cost between target
//       point (x,y,z) and parent point (parentX, parentY, parentZ). Include
//       effects of cost-multipliers if (x,y,z) is not in a pin-swappable zone.
//       The parent- and target-points must both be walkable. If there are
//       corner-cells between the parent and target, these cell must also
//       be walkable.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calc_distance_G_cost' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_calc_distance_G_cost 1
#undef DEBUG_calc_distance_G_cost

unsigned long calc_distance_G_cost(int x, int y, int z, int parentX, int parentY, int parentZ,
                                   InputValues_t *user_inputs, CellInfo_t ***const  cellInfo,
                                   const MapInfo_t *mapInfo, int pathNum)  {

  #ifdef DEBUG_calc_distance_G_cost
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;

  int x_window_min =  72;
  int x_window_max =  74;
  int y_window_min = 104;
  int y_window_max = 106;
  int z_window_min =   0;
  int z_window_max =   2;

  if (   (pathNum == 13)  && (mapInfo->current_iteration >= 285) && (mapInfo->current_iteration <= 288)
      && (parentX >= x_window_min) && (parentY >= y_window_min) && (parentZ >= z_window_min)
      && (parentX <= x_window_max) && (parentY <= y_window_max) && (parentZ <= z_window_max))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in calc_distance_G_cost() because specific requirements were met.\n\n");
    printf("DEBUG: Parent cell: (%d,%d,%d)     Target cell: (%d,%d,%d)\n", parentX, parentY, parentZ, x, y, z);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif


  unsigned long addedGCost = 0;  // G-cost value to be returned from this function

  // 'target_cell_pin_swappable_zone' is 0 if not in a swap-zone, or non-zero if in a swap-zone:
  int target_cell_pin_swappable_zone = cellInfo[x][y][z].swap_zone;
  #ifdef DEBUG_calc_distance_G_cost
  // DEBUG code follows:
  if (DEBUG_ON)  {
    printf("DEBUG: cellInfo[%d][%d][%d].swap_zone = %d at target cell.\n", x, y, z, cellInfo[x][y][z].swap_zone);
  }
  #endif

  // Get indices of cost-multipliers for the target cell at (x,y,z).
  int target_trace_cost_multiplier_index = cellInfo[x][y][z].traceCostMultiplierIndex;
  int viaUp_cost_multiplier_index        = cellInfo[x][y][z].viaUpCostMultiplierIndex;
  int viaDown_cost_multiplier_index      = cellInfo[x][y][z].viaDownCostMultiplierIndex;

  #ifdef DEBUG_calc_distance_G_cost
  // DEBUG code follows:
  if (DEBUG_ON)  {
    printf("DEBUG: In function calc_distance_G_cost:\n");
    printf("         swap_zone is %d\n", target_cell_pin_swappable_zone);
    printf("         user_inputs->pinSwapCellCost   = %'lu\n", user_inputs->pinSwapCellCost);
    printf("         user_inputs->cellCost[%d]       = %'lu\n", target_trace_cost_multiplier_index, user_inputs->cellCost[target_trace_cost_multiplier_index]);
    printf("         user_inputs->pinSwapDiagCost   = %'lu\n", user_inputs->pinSwapDiagCost);
    printf("         user_inputs->diagCost[%d]       = %'lu\n", target_trace_cost_multiplier_index, user_inputs->diagCost[target_trace_cost_multiplier_index]);
    printf("         user_inputs->pinSwapKnightCost = %'lu\n", user_inputs->pinSwapKnightCost);
    printf("         user_inputs->knightCost[%d]     = %'lu\n", target_trace_cost_multiplier_index, user_inputs->knightCost[target_trace_cost_multiplier_index]);
    printf("         user_inputs->pinSwapVertCost   = %'lu\n", user_inputs->pinSwapVertCost);
    printf("         user_inputs->baseVertCost      = %'lu\n", user_inputs->baseVertCost);
    printf("         user_inputs->vertCost[%d]       = %'lu\n", viaUp_cost_multiplier_index,   user_inputs->vertCost[viaUp_cost_multiplier_index]);
    printf("         user_inputs->vertCost[%d]       = %'lu\n", viaDown_cost_multiplier_index, user_inputs->vertCost[viaDown_cost_multiplier_index]);
  }
  #endif

  // Cost of going to adjacent, diagonal cells:
  if ((abs(x-parentX) == 1) && (abs(y-parentY) == 1))  {
    if (target_cell_pin_swappable_zone)
      addedGCost = user_inputs->pinSwapDiagCost;
    else
      addedGCost = user_inputs->diagCost[target_trace_cost_multiplier_index];
  }

  // Cost of going to adjacent, non-diagonal cells:
  else if (((abs(x-parentX) == 1) && (y == parentY))
          || ((x == parentX) && (abs(y-parentY) == 1)))  {
    if (target_cell_pin_swappable_zone)
      addedGCost = user_inputs->pinSwapCellCost;
    else
      addedGCost = user_inputs->cellCost[target_trace_cost_multiplier_index];
  }

  // Cost of going 2 cells in one direction and 1 cell in an orthogonal direction:
  else if (abs(x-parentX) + abs(y-parentY) == 3)  {
    // We've found a knight's move. First calculate the coordinates of the two
    // intermediate points that are 'jumped over':
    int x1 = 0; int y1 = 0; int z1 = 0;   // Coordinates of one intermediate cell
    int x2 = 0; int y2 = 0; int z2 = 0;   // Coordinates of other intermediate cell
    if ((abs(x-parentX) == 2) && (abs(y-parentY) == 1))  {
      // Current (x,y,z) is a knight's move from parent (x,y,z),
      // with absolute values of deltaX = 2 and deltaY = 1. Calculate locations
      // of 2 intermediate cells, labeled i1 and i2 in the diagram below:
      //
      //      ----------   s = start = parent (pX,pY)
      //   y  |  |i2| e|   e = end   = (x, y)
      //      ----------  i1 = 1st intermediate cell = ((pX+x)/2,pY)
      //   pY |s |i1|  |  i2 = 2nd intermediate cell = ((pX+x)/2, y)
      //      ----------
      //       pX     x
      // Add first intermediate cell 'i1' east/west of parent cell:
      x1 = (x + parentX) / 2;
      y1 = parentY;
      z1 = parentZ;

      // Add second intermediate cell 'i2' diagonal from parent cell:
      x2 = x1;  // X-value of cell 'i2' is same as X-value of cell 'i1'
      y2 = y;
      z2 = parentZ;
    }  // End of if-block for deltaX=2 and deltaY=1

    else {
      // Current (x,y,z) is a knight's move from parent (x,y,z),
      // with absolute values of deltaX = 1 and deltaY = 2. Calculate locations
      // of 2 intermediate cells, labeled i1 and i2 in the diagram below:
      //      -------
      //   y  |  | e|    s = start = parent (x,y)
      //      -------    e = end   = (x, y)
      //      |i1|i2|   i1 = 1st intermediate point = (pX,(y+pY)/2)
      //      -------   i2 = 2nd intermediate point = (x, (y+pY)/2)
      //   pY |s |  |
      //      -------
      //       pX  x
      //
      // Add first intermediate cell 'i1' north/south of parent cell:
      x1 = parentX;
      y1 = (y + parentY) / 2;
      z1 = parentZ;

      // Add second intermediate cell 'i2' diagonal from parent cell:
      x2 = x;
      y2 = y1;  // Y-value of cell 'i2' is same as Y-value of cell 'i1'
      z2 = parentZ;
    }  // End of else-block in which deltaX=1 and deltaY=2

    // Get cost-multiplier indices for the two intermediate cells:
    int cell1_cost_multiplier_index = cellInfo[x1][y1][z1].traceCostMultiplierIndex;
    int cell2_cost_multiplier_index = cellInfo[x2][y2][z2].traceCostMultiplierIndex;


    // Calculate G-costs for various combinations of knight's moves. In each case, the G-cost is calculated thus:
    //     G-cost = [knightCost for target cell  +   max(knightCost for 2 intermediate cells) ] / 2,
    // in which the 'knightCost' value includes the effects of cost-zone multipliers.
    //

    // Calculate knightCost for target cell:
    unsigned long target_cell_cost_contribution = 0;
    target_cell_cost_contribution = user_inputs->knightCost[target_trace_cost_multiplier_index];

    // Calculate knightCost for 2 intermediate cells:
    unsigned long intermediate_cell1_cost_contribution = 0;
    unsigned long intermediate_cell2_cost_contribution = 0;

    // Contribution from intermediate cell #1:
    intermediate_cell1_cost_contribution = user_inputs->knightCost[cell1_cost_multiplier_index];

    // Contribution from intermediate cell #2:
    intermediate_cell2_cost_contribution = user_inputs->knightCost[cell2_cost_multiplier_index];

    // Calculate total G-cost for the knight's move, which is the maximum of the two intermediate
    // cells' cost:
    addedGCost =  (target_cell_cost_contribution + max(intermediate_cell1_cost_contribution, intermediate_cell2_cost_contribution))/2;

  }  // End of if-block for a knight's move, in which abs(x-parentX) + abs(y-parentY) == 3


  // Cost of going through a via:
  else if ((x == parentX) && (y == parentY))  {
    if (target_cell_pin_swappable_zone)  {
      addedGCost = user_inputs->pinSwapVertCost;
      #ifdef DEBUG_calc_distance_G_cost
      // DEBUG code follows:
      if (DEBUG_ON)  {
         printf("DEBUG:   In calc_distance_G_cost, added pinSwapVertCost of %'lu because we're in pin-swap zone.\n", addedGCost);
      }
      #endif
    }
    else  {
      // Target cell is above parent cell, so use the vertCost for down-going vias
      // at cell (x,y,z). This is the same as the up-going via cost for cell
      // (x,y,parentZ).
      if (z > parentZ)  {
        addedGCost = user_inputs->vertCost[viaDown_cost_multiplier_index];
        #ifdef DEBUG_calc_distance_G_cost
        // DEBUG code follows:
        if (DEBUG_ON)  {
          printf("DEBUG:   In calc_distance_G_cost, added vertCost of %'lu for via-down\n", addedGCost);
        }
        #endif
      }
      else if (z < parentZ) {
        // Target cell is below parent cell, so use the vertCost for up-going vias
        // at cell (x,y,z). This is the same as the down-going via cost for cell
        // (x,y,parentZ).
        addedGCost = user_inputs->vertCost[viaUp_cost_multiplier_index];
        #ifdef DEBUG_calc_distance_G_cost
        // DEBUG code follows:
        if (DEBUG_ON)  {
          printf("DEBUG:   In calc_distance_G_cost, added vertCost of %'lu for via-up\n", addedGCost);
        }
        #endif
      }
      else  {
        // If program gets to here, it's because there's a bug in our logic:
        printf("Error: An unexpected error occurred in function 'calc_distance_G_cost.'\n");
        printf("                              (x,y,z) = (%d, %d, %d)\n", x, y, z);
        printf("          (parentX, parentY, parentZ) = (%d, %d, %d)\n", parentX, parentY, parentZ);
        printf("       Program will terminate.\n\n");
        exit(1);
      }
    }  // End of else-block for (target_cell_pin_swappable_zone)
  }  // End of else-block for (x==parentZ) and (y==parentY)
  else {
    // If program gets to here, it's because there's a bug in our logic:
    printf("Error: An unexpected error occurred in function 'calc_distance_G_cost.'\n");
    printf("                              (x,y,z) = (%d, %d, %d)\n", x, y, z);
    printf("          (parentX, parentY, parentZ) = (%d, %d, %d)\n", parentX, parentY, parentZ);
    printf("       Program will terminate.\n\n");
    exit(1);
  }

  #ifdef DEBUG_calc_distance_G_cost
  // DEBUG code follows:
  if (DEBUG_ON)  {
    printf("DEBUG: Returning distance-related G-cost of %'lu between parent (%d,%d,%d) and child (%d,%d,%d)\n",
           addedGCost, parentX, parentY, parentZ, x, y, z);
  }
  #endif

  return addedGCost;

}  // End of function 'calc_distance_G_cost'


//-----------------------------------------------------------------------------
// Name: calc_via_congestion
// Desc: Calculate the congestion penalty for the via between parent cell at
//       (x,y,parentZ) and target cell at (x,y,targetZ), taking into account
//       the up-via and down-via congestion of both cells. Congestion is recognized
//       consistent with the following table:
//
//==============================================================================================
// congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
// ---------------  |  ----------------------  ----------------------  ----------------------
//      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
//                  |
//   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
//                  |                                                    if not related
//                  |
//      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
//==============================================================================================
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calc_via_congestion' and re-compile if you want verbose debugging
// print-statements enabled:
//
// #define DEBUG_calc_via_congestion 1
#undef DEBUG_calc_via_congestion

int calc_via_congestion(const int path, const unsigned short target_DR_num, const unsigned short target_DR_subset,
                        const unsigned short parent_DR_num, const unsigned short parent_DR_subset,
                        CellInfo_t ***const  cellInfo, const InputValues_t *user_inputs, const MapInfo_t *mapInfo,
                        const int x, const int y, const int parentZ, const int targetZ,
                        const int excludeCongestion, const int excludePathNum, const int recognizeSelfCongestion)  {

  #ifdef DEBUG_calc_via_congestion
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;

  int x_window_min =  72;
  int x_window_max =  74;
  int y_window_min = 104;
  int y_window_max = 106;
  int z_window_min =   0;
  int z_window_max =   2;

  if (   (path == 13) && (mapInfo->current_iteration >= 285) && (mapInfo->current_iteration <= 288)
      && (x >= x_window_min) && (y >= y_window_min) && (parentZ >= z_window_min)
      && (x <= x_window_max) && (y <= y_window_max) && (parentZ <= z_window_max))  {
    printf("\nDEBUG: Setting DEBUG_ON to TRUE in calc_via_congestion() because specific requirements were met.\n");
    printf("DEBUG: Iteration = %d, path = %d, Parent cell = (%d,%d,%d), Target cell = (%d,%d,%d)\n\n", mapInfo->current_iteration,
           path, x, y, parentZ, x, y, targetZ);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  int congestion_penalty = 0;  // Congestion penalty to be returned from this function

  // Get trace-cost multipliers for the parent and target cells, i.e.,
  // above and below the via:
  int parent_via_cost_multiplier;
  int target_via_cost_multiplier;

  // Types of vias: VIA_UP (= 1) or VIA_DOWN (= 2)
  short parentToTargetViaType; // Type of via when moving from parent to target cell
  short targetToParentViaType; // Type of via when moving from target to parent cell
  int path_index;          // Temporary variable for path index

  // Determine whether via is an upward-going via (targetZ > parentZ) or a
  // downward-going via (targetZ < parentZ):
  if (targetZ > parentZ)  {
    parentToTargetViaType = VIA_UP;
    targetToParentViaType = VIA_DOWN;

    parent_via_cost_multiplier = user_inputs->viaCostMultiplier[cellInfo[x][y][parentZ].viaUpCostMultiplierIndex];
    target_via_cost_multiplier = user_inputs->viaCostMultiplier[cellInfo[x][y][targetZ].viaDownCostMultiplierIndex];
  }
  else if (targetZ < parentZ)  {
    parentToTargetViaType = VIA_DOWN;
    targetToParentViaType = VIA_UP;

    parent_via_cost_multiplier = user_inputs->viaCostMultiplier[cellInfo[x][y][parentZ].viaDownCostMultiplierIndex];
    target_via_cost_multiplier = user_inputs->viaCostMultiplier[cellInfo[x][y][targetZ].viaUpCostMultiplierIndex];
  }
  else  {
    printf("ERROR: In function 'calc_via_congestion', the 'targetZ' and 'parentZ' values are identical\n");
    printf("       from source location (%d, %d, %d) to target location (%d, %d, %d).\n",
            x, y, parentZ, x, y, targetZ);
    printf("       Program will terminate.\n\n");
    exit(1);
  }

  // Determine whether any paths traverse the parent and target cells:
  const int parent_path_count = cellInfo[x][y][parentZ].numTraversingPaths; // Number of paths that traverse the parent cell
  const int target_path_count = cellInfo[x][y][targetZ].numTraversingPaths; // Number of paths that traverse the target cell

  #ifdef DEBUG_calc_via_congestion
  if (DEBUG_ON)  {
    printf("DEBUG: In calc_via_congestion, parentToTargetViaType is '%d'. targetToParentViaType is '%d'.\n",
           parentToTargetViaType, targetToParentViaType);
    printf("DEBUG: %d paths traverse the parent cell at (%d,%d,%d)\n", parent_path_count, x, y, parentZ);
    printf("DEBUG: %d paths traverse the target cell at (%d,%d,%d)\n\n", target_path_count, x, y, targetZ);
  }
  #endif


  // Calculate the subset/shapeType index for the routed path. This index is simply
  // the subset x 3   +   shapeType:
  int routing_subset_shapeType = parent_DR_subset * NUM_SHAPE_TYPES   +   parentToTargetViaType;


  // Iterate through the paths that traverse the parent cell and determine the
  // congestion from that cell. Only include via-related congestion with the
  // same shape-type as 'parentToTargetViaType':
  for (path_index = 0; path_index < parent_path_count; path_index++)  {

    // Check whether the congestion should be considered. Congestion should be considered if:
    //  (a) recognizeSelfCongestion is TRUE or the congestion is not from 'path', AND
    //  (b) excludeCongestion is FALSE, OR the congestion is not from 'excludePathNum':
    int congestion_pathNum = cellInfo[x][y][parentZ].congestion[path_index].pathNum;
    #ifdef DEBUG_calc_via_congestion
    if (DEBUG_ON)  {
      printf("DEBUG:   %3d: Parent cell (%d,%d,%d) contains congestion from path=%d, subset=%d, shapeType=%d, pathTraversalsTimes100=%'d.\n",
             path_index, x, y, parentZ, congestion_pathNum, cellInfo[x][y][parentZ].congestion[path_index].DR_subset,
             cellInfo[x][y][parentZ].congestion[path_index].shapeType, cellInfo[x][y][parentZ].congestion[path_index].pathTraversalsTimes100);
    }
    #endif
    if (   (recognizeSelfCongestion || (congestion_pathNum != path))           // Item (a) above
        && ((! excludeCongestion) || (congestion_pathNum != excludePathNum)))  // Item (b) above
    {
      int congestion_DR_subset = cellInfo[x][y][parentZ].congestion[path_index].DR_subset;
      int congestion_shapeType = cellInfo[x][y][parentZ].congestion[path_index].shapeType;

      #ifdef DEBUG_calc_via_congestion
      if (DEBUG_ON)  {
        // printf("DEBUG:     congestion_DR_subset = %d,  congestion_shapeType = %d\n", congestion_DR_subset, congestion_shapeType);
        printf("DEBUG:          Comparing to parent_DR_subset=%d, parentToTargetViaType=%d\n", parent_DR_subset, parentToTargetViaType);
      }
      #endif

      // Check whether the congestion has the same shape-type and design-rule subset as the target:
      if ((parent_DR_subset == congestion_DR_subset) && (parentToTargetViaType == congestion_shapeType))  {

        // We got here, so we found congestion from a foreign path of the same design-rule
        // subset and shape-type. Recognize congestion consistent with the following table
        //==============================================================================================
        // congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
        // ---------------  |  ----------------------  ----------------------  ----------------------
        //      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
        //                  |
        //   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
        //                  |                                                    if not related
        //                  |
        //      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
        //==============================================================================================
        // Based on the above table, we recognize congestion only if:
        //   (1) 'congestion_pathNum' is not a pseudo-path and 'path' is not the parent pseudo-path of 'congestion_pathNum'
        // or
        //   (2) 'path' is a pseudo-path and 'congestion_pathNum' is a pseudo-path
        //
        if (   (( ! user_inputs->isPseudoNet[congestion_pathNum])                            // Item #1 in list above
                    && (user_inputs->diffPairToPseudoNetMap[congestion_pathNum] != path))    // Item #1 in list above (continued)
            || ( user_inputs->isPseudoNet[path]                                              // Item #2 in list above
                    && user_inputs->isPseudoNet[congestion_pathNum]) )  {                    // Item #2 in list above (continued)

          // We got here, so it's appropriate to recognize congestion. We now calculate the
          // subset/shapeType index for the congestion. This index is simply
          // the subset x 3   +   shapeType:
          int congestion_subset_shapeType = congestion_DR_subset * NUM_SHAPE_TYPES   +   congestion_shapeType;

          // Amount of congestion added by the current path_index:
          int added_congestion = 0;

          // Next, check whether the routed net and the congestion net are diff-pair partners.
          // If so, then the congestion-related G-cost should be higher because such partners
          // have less freedom to move around:
          if (user_inputs->isDiffPair[path] && (user_inputs->diffPairPartner[path] == congestion_pathNum))  {

            // 'pathNum' is a diff-pair path, and the 'congestion_pathNum' is its diff-pair
            // partner, so add more congestion from the foreign path (by a factor of VIA_CONGESTION_FACTOR):
            added_congestion = DIFF_PAIR_PARTNER_VIA_CONGESTION_FACTOR
                                    * (int)(cellInfo[x][y][parentZ].congestion[path_index].pathTraversalsTimes100
                                    * mapInfo->viaCongestionMultiplier
                                    * user_inputs->detour_distance[parent_DR_num][routing_subset_shapeType][parent_DR_num][congestion_subset_shapeType]
                                    * (parent_via_cost_multiplier + target_via_cost_multiplier)/2);
          }
          else  {
            // 'pathNum' and 'congestion_pathNum' are not diff-pair partners, so add the
            // normal amount of congestion-related G-cost:
            added_congestion = (int)(cellInfo[x][y][parentZ].congestion[path_index].pathTraversalsTimes100
                                    * mapInfo->viaCongestionMultiplier
                                    * user_inputs->detour_distance[parent_DR_num][routing_subset_shapeType][parent_DR_num][congestion_subset_shapeType]
                                    * (parent_via_cost_multiplier + target_via_cost_multiplier)/2);

          }  // End of if/else block for routed path and congestion path being diff-pair partners


          // Add the congestion to 'congestion_penalty', which is the variable that will be
          // returned from this function:
          congestion_penalty += added_congestion;

          #ifdef DEBUG_calc_via_congestion
          if (DEBUG_ON)  {
            printf("DEBUG: In calc_via_congestion: Path index %d that traverses parent cell (%d,%d,%d) is path number %d (subset %d, shapeType %d)\n",
                    path_index, x, y, parentZ, congestion_pathNum, congestion_DR_subset, congestion_shapeType);
            printf("DEBUG:    cellInfo[%d][%d][%d].congestion[%d].pathTraversalsTimes100 = %d\n", x, y, parentZ, path_index,
                   cellInfo[x][y][parentZ].congestion[path_index].pathTraversalsTimes100);
            printf("DEBUG:    congestionMultiplier = %.7f\n", mapInfo->congestionMultiplier);
            printf("DEBUG:    detour_distance[%d][%d][%d][%d] = %6.3f\n", parent_DR_num, routing_subset_shapeType, parent_DR_num,
                   congestion_subset_shapeType, user_inputs->detour_distance[parent_DR_num][routing_subset_shapeType][parent_DR_num][congestion_subset_shapeType]);
            printf("DEBUG:    parent_via_cost_multiplier = %d\n", parent_via_cost_multiplier);
            printf("DEBUG:    target_via_cost_multiplier = %d\n\n", target_via_cost_multiplier);
          }
          #endif

        }  // End of if-block for pathNum and congestion_pathNum not being related diff-pair nets.
      }  // End of if-block for congestion having the same shape-type and design-rule subset as the target
    }  // End of if-block for foreign congestion
  }  // End of for-loop for index 'path_index'

  #ifdef DEBUG_calc_via_congestion
  if (DEBUG_ON)  {
    printf("\nDEBUG: Finished processing parent cell at (%d,%d,%d). Starting target cell at (%d,%d,%d)...\n\n",
           x, y, parentZ, x, y, targetZ);
  }
  #endif

  // Re-calculate the subset/shapeType index for the routed path. This index is simply
  // the subset x 3   +   shapeType:
  routing_subset_shapeType = target_DR_subset * NUM_SHAPE_TYPES   +   targetToParentViaType;

  // Now do the same for the target cell: Iterate through the paths that traverse the
  // target cell and determine the congestion from that cell. Only include via-related
  // congestion with the same shape-type as 'targetToParentViaType'.
  for (path_index = 0; path_index < target_path_count; path_index++)  {

    // Check whether the congestion should be considered. Congestion should be considered if:
    //  (a) recognizeSelfCongestion is TRUE or the congestion is not from 'path', AND
    //  (b) excludeCongestion is FALSE, OR the congestion is not from 'excludePathNum':
    int congestion_pathNum = cellInfo[x][y][targetZ].congestion[path_index].pathNum;
    #ifdef DEBUG_calc_via_congestion
    if (DEBUG_ON)  {
      printf("DEBUG:   %3d: Target cell (%d,%d,%d) contains congestion from path=%d, subset=%d, shapeType=%d, pathTraversalsTimes100=%'d.\n",
             path_index, x, y, targetZ, congestion_pathNum, cellInfo[x][y][targetZ].congestion[path_index].DR_subset,
             cellInfo[x][y][targetZ].congestion[path_index].shapeType, cellInfo[x][y][targetZ].congestion[path_index].pathTraversalsTimes100);
    }
    #endif
    if (   (recognizeSelfCongestion || (congestion_pathNum != path))           // Item (a) above
        && ((! excludeCongestion) || (congestion_pathNum != excludePathNum)))  // Item (b) above
    {

      // Check whether the congestion has the same shape-type and design-rule subset as the target:
      int congestion_DR_subset = cellInfo[x][y][targetZ].congestion[path_index].DR_subset;
      int congestion_shapeType = cellInfo[x][y][targetZ].congestion[path_index].shapeType;

      #ifdef DEBUG_calc_via_congestion
      if (DEBUG_ON)  {
        // printf("DEBUG:     congestion_DR_subset = %d,  congestion_shapeType = %d\n", congestion_DR_subset, congestion_shapeType);
        printf("DEBUG:          Comparing to target_DR_subset=%d, targetToParentViaType=%d\n", target_DR_subset, targetToParentViaType);
      }
      #endif

      if ((target_DR_subset == congestion_DR_subset) && (targetToParentViaType == congestion_shapeType))  {

        // We got here, so we found congestion from a foreign path of the same design-rule
        // subset and shape-type. Recognize congestion consistent with the following table
        //==============================================================================================
        // congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
        // ---------------  |  ----------------------  ----------------------  ----------------------
        //      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
        //                  |
        //   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
        //                  |                                                    if not related
        //                  |
        //      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
        //==============================================================================================
        // Based on the above table, we recognize congestion only if:
        //   (1) 'congestion_pathNum' is not a pseudo-path and 'path' is not the parent pseudo-path of 'congestion_pathNum'
        // or
        //   (2) 'path' is a pseudo-path and 'congestion_pathNum' is a pseudo-path
        //
        if (   (( ! user_inputs->isPseudoNet[congestion_pathNum])                            // Item #1 in list above
                    && (user_inputs->diffPairToPseudoNetMap[congestion_pathNum] != path))    // Item #1 in list above (continued)
            || ( user_inputs->isPseudoNet[path]                                              // Item #2 in list above
                    && user_inputs->isPseudoNet[congestion_pathNum]) )  {                    // Item #2 in list above (continued)

          // We got here, so it's appropriate to recognize congestion. We now calculate the
          // subset/shapeType index for the the congestion. This index is simply
          // the subset x 3   +   shapeType:
          int congestion_subset_shapeType = congestion_DR_subset * NUM_SHAPE_TYPES   +   congestion_shapeType;

          // Amount of congestion added by the current path_index:
          int added_congestion = 0;

          // Next, check whether the routed net and the congestion net are diff-pair partners.
          // If so, then the congestion-related G-cost should be higher because such partners
          // have less freedom to move around:
          if (user_inputs->isDiffPair[path] && (user_inputs->diffPairPartner[path] == congestion_pathNum))  {

            // 'pathNum' is a diff-pair path, and the 'congestion_pathNum' is its diff-pair
            // partner, so add more congestion from the foreign path (by a factor of VIA_CONGESTION_FACTOR):
            added_congestion = DIFF_PAIR_PARTNER_VIA_CONGESTION_FACTOR
                                    * (int)(cellInfo[x][y][targetZ].congestion[path_index].pathTraversalsTimes100
                                    * mapInfo->viaCongestionMultiplier
                                    * user_inputs->detour_distance[target_DR_num][routing_subset_shapeType][target_DR_num][congestion_subset_shapeType]
                                    * (parent_via_cost_multiplier + target_via_cost_multiplier)/2);
          }
          else  {
            // 'pathNum' and 'congestion_pathNum' are not diff-pair partners, so add the
            // normal amount of congestion-related G-cost:
            added_congestion = (int)(cellInfo[x][y][targetZ].congestion[path_index].pathTraversalsTimes100
                                    * mapInfo->viaCongestionMultiplier
                                    * user_inputs->detour_distance[target_DR_num][routing_subset_shapeType][target_DR_num][congestion_subset_shapeType]
                                    * (parent_via_cost_multiplier + target_via_cost_multiplier)/2);
          }  // End of if/else block for routed path and congestion path being diff-pair partners


          // Add the congestion to 'congestion_penalty', which is the variable that will be
          // returned from this function:
          congestion_penalty += added_congestion;

          #ifdef DEBUG_calc_via_congestion
          if (DEBUG_ON)  {
            printf("DEBUG: In calc_via_congestion: Path index %d that traverses target cell (%d,%d,%d) is path number %d (subset %d, shapeType %d)\n",
                   path_index, x, y, targetZ, congestion_pathNum, congestion_DR_subset, congestion_shapeType);
            printf("DEBUG:    cellInfo[%d][%d][%d].congestion[%d].pathTraversalsTimes100 = %d\n", x, y, targetZ, path_index,
                   cellInfo[x][y][targetZ].congestion[path_index].pathTraversalsTimes100);
            printf("DEBUG:    congestionMultiplier = %.7f\n", mapInfo->congestionMultiplier);
            printf("DEBUG:    detour_distance[%d][%d][%d][%d] = %6.3f\n", target_DR_num, routing_subset_shapeType, target_DR_num,
                   congestion_subset_shapeType, user_inputs->detour_distance[target_DR_num][routing_subset_shapeType][target_DR_num][congestion_subset_shapeType]);
            printf("DEBUG:      parent_via_cost_multiplier = %d\n", parent_via_cost_multiplier);
            printf("DEBUG:      target_via_cost_multiplier = %d\n\n", target_via_cost_multiplier);
          }
          #endif

        }  // End of if-block for pathNum and congestion_pathNum not being related diff-pair nets.
      }  // End of if-block for congestion having the same shape-type and design-rule subset as the target

    }  // End of if-block for foreign congestion

  }  // End of for-loop for index 'path_index'

  #ifdef DEBUG_calc_via_congestion
  if (DEBUG_ON)  {
    printf("\nDEBUG: Exiting function calc_via_congestion with returned congestion of %'d.\n\n", congestion_penalty);
  }
  #endif

  // Return the aggregate congestion_penalty:
  return(congestion_penalty);

}  // End of function 'calc_via_congestion'


//-----------------------------------------------------------------------------
// Name: calc_corner_congestion
// Desc: Calculate the congestion penalty *between* (parentX, parentY, z) and
//       (x,y,z), taking into account the corner/diagonal cells that must be
//       traversed. Function accounts for whether the congestion is due
//       to foreign paths of the same design-rule subset and shape-type.
//       X/Y coordinates are relative to the parent cell. All cells are
//       assumed to be on the same layer 'z'. Congestion is recognized
//       consistent with the following table:
//
//==============================================================================================
// congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
// ---------------  |  ----------------------  ----------------------  ----------------------
//      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
//                  |
//   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
//                  |                                                    if not related
//                  |
//      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
//==============================================================================================
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calc_corner_congestion' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_calc_corner_congestion 1
#undef DEBUG_calc_corner_congestion

int calc_corner_congestion(const int path, const unsigned short shapeType,
                           CellInfo_t ***const cellInfo, const InputValues_t *user_inputs,
                           const MapInfo_t *mapInfo, const int parentX, const int parentY, const int z,
                           const int x, const int y, const int criteria_X_delta, const int criteria_Y_delta,
                           const int corn1_X_delta,    const int corn1_Y_delta,
                           const int corn2_X_delta,    const int corn2_Y_delta,
                           const int excludeCongestion, const int excludePathNum, const int recognizeSelfCongestion)  {

  #ifdef DEBUG_calc_corner_congestion
  // DEBUG code follows:
  //
  // For very large maps, define here the 3-dimensional window in (x,y,z) for which you want
  // detailed information sent to the log file. Without these constraints, the log file can
  // grow to >10 gigabytes:
  int x_window_min = 282;
  int x_window_max = 284;
  int y_window_min =  53;
  int y_window_max =  55;
  int z_window_min =   2;
  int z_window_max =   2;

  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (   (path == 3)  && (mapInfo->current_iteration >= 60) && (mapInfo->current_iteration <= 60)
      && (parentX >= x_window_min) && (parentY >= y_window_min) && (z >= z_window_min)
      && (parentX <= x_window_max) && (parentY <= y_window_max) && (z <= z_window_max))  {
    printf("\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in calc_corner_congestion() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;

    printf("DEBUG: (thread %2d) Entered 'calc_corner_congestion' with input parameters:\n", omp_get_thread_num());
    printf("DEBUG: (thread %2d)       path=%d, parent (x,y,z) = (%d,%d,%d), (x,y,z) = (%d,%d,%d)\n",
           omp_get_thread_num(), path, parentX, parentY, z, x, y, z);
    printf("DEBUG: (thread %2d)       criteria_X_delta=%d, criteria_Y_delta=%d\n", omp_get_thread_num(), criteria_X_delta, criteria_Y_delta);
    printf("DEBUG: (thread %2d)       corn1_X_delta=%d, corn1_Y_delta=%d, corn2_X_delta=%d, corn2_Y_delta=%d\n",
           omp_get_thread_num(), corn1_X_delta, corn1_Y_delta, corn2_X_delta, corn2_Y_delta);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif


  // Calculate locations of 2 cells: corner cell #1 and corner cell #2:
  const int x1 = parentX + corn1_X_delta;
  const int y1 = parentY + corn1_Y_delta;
  const int x2 = parentX + corn2_X_delta;
  const int y2 = parentY + corn2_Y_delta;

  int penalty  = 0;  // Composite congestion penalty of intervening cells
  int penalty1 = 0, penalty2 = 0; // Congestion penalty of each corner cell
  int path_index=0; // Temporary index for path number

  // Check whether location of child cell meets input criteria:
  if (((int)(x - parentX) != criteria_X_delta)
       || ((int)(y - parentY) != criteria_Y_delta)) {

    penalty = 0; // Return zero if location of child cell doesn't meet input criteria
  }
  else {
    // Location of child cell meets input criteria.

    // Determine whether any paths traverse the corner cells:
    const int path_count1 = cellInfo[x1][y1][z].numTraversingPaths; // Number of paths traversing corner cell #1
    const int path_count2 = cellInfo[x2][y2][z].numTraversingPaths; // Number of paths traversing corner cell #2

    #ifdef DEBUG_calc_corner_congestion
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) path_count1=%d, path_count2=%d in 'calc_corner_congestion'\n", omp_get_thread_num(),
              path_count1, path_count2);
    }
    #endif

    if ((path_count1 == 0) && (path_count2 == 0))  {
      penalty = 0; // Return penalty of zero because both corner cells
    }              // lack any traversing paths
    else {
      // At least 1 corner cell contains traversing paths. Now determine penalty based on
      // whether the cell contains foreign congestion (different path number) of
      // same design-rule subset and shape-type:

      // Calculate design-rule numbers of the two cells, which could possibly be in
      // different design-rule zones:
      int DR_num_1 = cellInfo[x1][y1][z].designRuleSet;
      int DR_num_2 = cellInfo[x2][y2][z].designRuleSet;

      // Calculate the design-rule subset numbers at the two cells, which might be
      // different from each other if the two cells are in different design-rule zones.
      int DR_subset_1 = user_inputs->designRuleSubsetMap[path][DR_num_1];
      int DR_subset_2 = user_inputs->designRuleSubsetMap[path][DR_num_2];

      // Calculate the subset/shapeType index for the routed path. This index is simply
      // the subset x 3   +   shapeType:
      int routing_subset_shapeType_1 = DR_subset_1 * NUM_SHAPE_TYPES   +   shapeType;
      int routing_subset_shapeType_2 = DR_subset_2 * NUM_SHAPE_TYPES   +   shapeType;

      #ifdef DEBUG_calc_corner_congestion
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) DR_subset_1 = %d, DR_subset_2 = %d in calc_corner_congestion for corner cells #1 and #2\n",
               omp_get_thread_num(), DR_subset_1, DR_subset_2);
        printf("DEBUG: (thread %2d) routing_subset_shapeType_1 = %d, routing_subset_shapeType_2 = %d in calc_corner_congestion for corner cells #1 and #2\n",
                 omp_get_thread_num(), routing_subset_shapeType_1, routing_subset_shapeType_2);
      }
      #endif

      // Calculate the cost-zone multipliers for each of the two corner-cells:
      int cell_1_cost_zone_multiplier = user_inputs->traceCostMultiplier[cellInfo[x1][y1][z].traceCostMultiplierIndex];
      int cell_2_cost_zone_multiplier = user_inputs->traceCostMultiplier[cellInfo[x2][y2][z].traceCostMultiplierIndex];

      // Iterate over the paths that traverse cell #1:
      #ifdef DEBUG_calc_corner_congestion
      if (DEBUG_ON)  {
        printf("\nDEBUG: (thread %2d) Iterating over traversing paths at corner-cell #1 at (%d,%d,%d):\n", omp_get_thread_num(), x1, y1, z);
      }
      #endif
      for (path_index = 0; path_index < path_count1; path_index++)  {

        // Check whether the congestion should be considered. Congestion should be considered if:
        //  (a) recognizeSelfCongestion is TRUE or the congestion is not from 'path', AND
        //  (b) excludeCongestion is FALSE, OR the congestion is not from 'excludePathNum':
        int congestion_pathNum = cellInfo[x1][y1][z].congestion[path_index].pathNum;

        #ifdef DEBUG_calc_corner_congestion
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)   path_index %d has congestion_pathNum = %d\n", omp_get_thread_num(), path_index, congestion_pathNum);
        }
        #endif

        if (   (recognizeSelfCongestion || (congestion_pathNum != path))           // Item (a) above
            && ((! excludeCongestion) || (congestion_pathNum != excludePathNum)))  // Item (b) above
        {

          // Check whether the congestion has the same shape-type and design-rule subset as the target:
          int congestion_DR_subset = cellInfo[x1][y1][z].congestion[path_index].DR_subset;
          int congestion_shapeType = cellInfo[x1][y1][z].congestion[path_index].shapeType;

          #ifdef DEBUG_calc_corner_congestion
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)   path_index %d has congestion_DR_subset = %d, congestion_shapeType = %d\n",
                   omp_get_thread_num(), path_index, congestion_DR_subset, congestion_shapeType);
          }
          #endif

          if ((DR_subset_1 == congestion_DR_subset) && (shapeType == congestion_shapeType))  {

            // Recognize congestion consistent with the following table:
            //==============================================================================================
            // congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
            // ---------------  |  ----------------------  ----------------------  ----------------------
            //      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
            //                  |
            //   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
            //                  |                                                    if not related
            //                  |
            //      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
            //==============================================================================================
            // Based on the above table, we recognize congestion only if:
            //   (1) 'congestion_pathNum' is not a pseudo-path and 'path' is not the parent pseudo-path of 'congestion_pathNum'
            // or
            //   (2) 'path' is a pseudo-path and 'congestion_pathNum' is a pseudo-path
            //
            if (   (( ! user_inputs->isPseudoNet[congestion_pathNum])                            // Item #1 in list above
                        && (user_inputs->diffPairToPseudoNetMap[congestion_pathNum] != path))    // Item #1 in list above (continued)
                || ( user_inputs->isPseudoNet[path]                                              // Item #2 in list above
                        && user_inputs->isPseudoNet[congestion_pathNum]) )  {                    // Item #2 in list above (continued)

              // We got here, so it's appropriate to recognize congestion. We now calculate the
              // subset/shapeType index for the congestion. This index is simply
              // the subset x 3   +   shapeType:
              int congestion_subset_shapeType = congestion_DR_subset * NUM_SHAPE_TYPES   +   congestion_shapeType;

              // Amount of congestion added by the current path_index:
              int added_congestion = 0;

              #ifdef DEBUG_calc_corner_congestion
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) *** Adding corner congestion from (%d,%d,%d)! pathTraversalsTimes100 = %d\n",
                       omp_get_thread_num(), x1, y1, z, cellInfo[x1][y1][z].congestion[path_index].pathTraversalsTimes100);
              }
              #endif

              // Next, check whether the routed net and the congestion net are diff-pair partners.
              // If so, then the congestion-related G-cost should be higher because such partners
              // have less freedom to move around:
              if (user_inputs->isDiffPair[path] && (user_inputs->diffPairPartner[path] == congestion_pathNum))  {

                // 'path' is a diff-pair path, and the 'congestion_pathNum' is its diff-pair
                // partner, so add more congestion from the foreign path (by a factor of DIFF_PAIR_PARTNER_TRACE_CONGESTION_FACTOR):
                added_congestion = DIFF_PAIR_PARTNER_TRACE_CONGESTION_FACTOR
                                   * (int)(cellInfo[x1][y1][z].congestion[path_index].pathTraversalsTimes100
// The following line was commented out and replaced with the subsequent line as an experiment on 6/12/2024:
                                   * mapInfo->traceCongestionMultiplier
//                                 * mapInfo->viaCongestionMultiplier
                                   * cell_1_cost_zone_multiplier
                                   * user_inputs->detour_distance[DR_num_1][routing_subset_shapeType_1][DR_num_1][congestion_subset_shapeType]);
              }
              else  {
                // 'pathNum' and 'congestion_pathNum' are not diff-pair partners, so add the
                // normal amount of congestion-related G-cost:
                added_congestion = (int)(cellInfo[x1][y1][z].congestion[path_index].pathTraversalsTimes100
// The following line was commented out and replaced with the subsequent line as an experiment on 6/12/2024:
                                   * mapInfo->traceCongestionMultiplier
//                                 * mapInfo->viaCongestionMultiplier
                                   * cell_1_cost_zone_multiplier
                                   * user_inputs->detour_distance[DR_num_1][routing_subset_shapeType_1][DR_num_1][congestion_subset_shapeType]);

              }  // End of if/else block for routed path and congestion path being diff-pair partners


              // Add the congestion to 'penalty1', which is the variable that reflects the congestion
              // penalty from this corner cell:
              penalty1 += added_congestion;

              #ifdef DEBUG_calc_corner_congestion
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) penalty1 = %'d after checking for foreign congestion in function 'calc_corner_congestion'\n",
                       omp_get_thread_num(), penalty1);
              }
              #endif

            }  // End of if-block for pathNum and congestion_pathNum not being related diff-pair nets.
          }  // End of if-block for congestion having the same shape-type and design-rule subset as the target
        }  // End of if-block for foreign congestion
      }  // End of for-loop for index 'path_index'


      //
      // Do the same for corner cell #2: Check all paths that traverse corner-cell #2:
      //
      #ifdef DEBUG_calc_corner_congestion
      if (DEBUG_ON)  {
        printf("\nDEBUG: (thread %2d) Iterating over traversing paths at corner-cell #2 at (%d,%d,%d):\n", omp_get_thread_num(), x2, y2, z);
      }
      #endif
      for (path_index = 0; path_index < path_count2; path_index++)  {

        // Check whether the congestion should be considered. Congestion should be considered if:
        //  (a) recognizeSelfCongestion is TRUE or the congestion is not from 'path', AND
        //  (b) excludeCongestion is FALSE, OR the congestion is not from 'excludePathNum':
        int congestion_pathNum = cellInfo[x2][y2][z].congestion[path_index].pathNum;

        #ifdef DEBUG_calc_corner_congestion
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)   path_index %d has congestion_pathNum = %d\n", omp_get_thread_num(), path_index, congestion_pathNum);
        }
        #endif

        if (   (recognizeSelfCongestion || (congestion_pathNum != path))           // Item (a) above
            && ((! excludeCongestion) || (congestion_pathNum != excludePathNum)))  // Item (b) above
        {

          // Check whether the congestion has the same shape-type and design-rule subset as the target:
          int congestion_DR_subset = cellInfo[x2][y2][z].congestion[path_index].DR_subset;
          int congestion_shapeType = cellInfo[x2][y2][z].congestion[path_index].shapeType;

          #ifdef DEBUG_calc_corner_congestion
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)   path_index %d has congestion_DR_subset = %d, congestion_shapeType = %d\n",
                   omp_get_thread_num(), path_index, congestion_DR_subset, congestion_shapeType);
          }
          #endif

          if ((DR_subset_2 == congestion_DR_subset) && (shapeType == congestion_shapeType))  {

            // Recognize congestion consistent with the following table:
            //==============================================================================================
            // congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
            // ---------------  |  ----------------------  ----------------------  ----------------------
            //      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
            //                  |
            //   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
            //                  |                                                    if not related
            //                  |
            //      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
            //==============================================================================================
            // Based on the above table, we recognize congestion only if:
            //   (1) 'congestion_pathNum' is not a pseudo-path and 'path' is not the parent pseudo-path of 'congestion_pathNum'
            // or
            //   (2) 'path' is a pseudo-path and 'congestion_pathNum' is a pseudo-path
            //
            if (   (( ! user_inputs->isPseudoNet[congestion_pathNum])                            // Item #1 in list above
                        && (user_inputs->diffPairToPseudoNetMap[congestion_pathNum] != path))    // Item #1 in list above (continued)
                || ( user_inputs->isPseudoNet[path]                                              // Item #2 in list above
                        && user_inputs->isPseudoNet[congestion_pathNum]) )  {                    // Item #2 in list above (continued)

              // We got here, so it's appropriate to recognize congestion. We now calculate the
              // subset/shapeType index for the congestion. This index is simply
              // the subset x 3   +   shapeType:
              int congestion_subset_shapeType = congestion_DR_subset * NUM_SHAPE_TYPES   +   congestion_shapeType;

              // Amount of congestion added by the current path_index:
              int added_congestion = 0;

              #ifdef DEBUG_calc_corner_congestion
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) *** Adding corner congestion from (%d,%d,%d)! pathTraversalsTimes100 = %d\n",
                       omp_get_thread_num(), x2, y2, z, cellInfo[x2][y2][z].congestion[path_index].pathTraversalsTimes100);
              }
              #endif

              // Next, check whether the routed net and the congestion net are diff-pair partners.
              // If so, then the congestion-related G-cost should be higher because such partners
              // have less freedom to move around:
              if (user_inputs->isDiffPair[path] && (user_inputs->diffPairPartner[path] == congestion_pathNum))  {

                // 'pathNum' is a diff-pair path, and the 'congestion_pathNum' is its diff-pair
                // partner, so add more congestion from the foreign path (by a factor of DIFF_PAIR_PARTNER_TRACE_CONGESTION_FACTOR):
                added_congestion = DIFF_PAIR_PARTNER_TRACE_CONGESTION_FACTOR
                                   * (int)(cellInfo[x2][y2][z].congestion[path_index].pathTraversalsTimes100
// The following line was commented out and replaced with the subsequent line as an experiment on 6/12/2024:
                                   * mapInfo->traceCongestionMultiplier
//                                 * mapInfo->viaCongestionMultiplier
                                   * cell_2_cost_zone_multiplier
                                   * user_inputs->detour_distance[DR_num_2][routing_subset_shapeType_2][DR_num_2][congestion_subset_shapeType]);
              }
              else  {
                // 'pathNum' and 'congestion_pathNum' are not diff-pair partners, so add the
                // normal amount of congestion-related G-cost:
                added_congestion = (int)(cellInfo[x2][y2][z].congestion[path_index].pathTraversalsTimes100
// The following line was commented out and replaced with the subsequent line as an experiment on 6/12/2024:
                                   * mapInfo->traceCongestionMultiplier
//                                 * mapInfo->viaCongestionMultiplier
                                   * cell_2_cost_zone_multiplier
                                   * user_inputs->detour_distance[DR_num_2][routing_subset_shapeType_2][DR_num_2][congestion_subset_shapeType]);

              }  // End of if/else block for routed path and congestion path being diff-pair partners


              // Add the congestion to 'penalty2', which is the variable that sums up the congestion
              // from this corner-cell:
              penalty2 += added_congestion;

              #ifdef DEBUG_calc_corner_congestion
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) penalty2 = %'d after checking for foreign congestion in function 'calc_corner_congestion'\n",
                       omp_get_thread_num(), penalty2);
              }
              #endif

            }  // End of if-block for pathNum and congestion_pathNum not being related diff-pair nets.
          }  // End of if-block for congestion having the same shape-type and design-rule subset as the target
        }  // End of if-block for foreign congestion
      }  // End of for-loop for index 'path_index'

      #ifdef DEBUG_calc_corner_congestion
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) penalty1=%d, penalty2=%d\n", omp_get_thread_num(), penalty1, penalty2);
      }
      #endif

      // Calculate composite penalty as the maximum penalty from both corner cells:
      penalty = max(penalty1, penalty2);

    }  // End of if/else-block for checking path-counts
  }  // End of if/else-block for checking cell location

  #ifdef DEBUG_calc_corner_congestion
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Returning %d from function 'calc_corner_congestion'\n", omp_get_thread_num(), penalty);
  }
  #endif

  return(penalty);

} // End of function 'calc_corner_congestion'


//-----------------------------------------------------------------------------
// Name: calc_congestion_penalty
// Desc: Calculate the penalty associated with congestion between destination
//       (target) cell (x,y,z) and parent cell (parentX, parentY, parentZ). Return
//       zero if cell (x,y,z) is in a pin-swappable zone. Congestion is recognized
//       consistent with the following table:
//
//==============================================================================================
// congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
// ---------------  |  ----------------------  ----------------------  ----------------------
//      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
//                  |
//   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
//                  |                                                    if not related
//                  |
//      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
//==============================================================================================
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calc_congestion_penalty' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_calc_congestion_penalty 1
#undef DEBUG_calc_congestion_penalty

long calc_congestion_penalty(const int x, const int y, const int z,
                             const int parentX, const int parentY, const int parentZ,
                             const int pathNum, const unsigned short shapeType,
                             CellInfo_t ***const cellInfo, const InputValues_t *user_inputs,
                             const MapInfo_t *mapInfo, const int excludeCongestion,
                             const int excludePathNum, const int recognizeSelfCongestion)  {

  #ifdef DEBUG_calc_congestion_penalty
  // DEBUG code follows:
  //
  // For very large maps, define here the 3-dimensional window in (x,y,z) for which you want
  // detailed information sent to the log file. Without these constraints, the log file can
  // grow to >10 gigabytes:
  int x_window_min =  72;
  int x_window_max =  74;
  int y_window_min = 104;
  int y_window_max = 106;
  int z_window_min =   0;
  int z_window_max =   2;

  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (   (pathNum == 13)  && (mapInfo->current_iteration >= 285) && (mapInfo->current_iteration <= 288)
      && (parentX >= x_window_min) && (parentY >= y_window_min) && (parentZ >= z_window_min)
      && (parentX <= x_window_max) && (parentY <= y_window_max) && (parentZ <= z_window_max))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in calc_congestion_penalty() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;

    printf("DEBUG: (thread %2d) Entered 'calc_congestion_penalty' within iteration %d and pathNum %d with parameters:\n",
           omp_get_thread_num(), mapInfo->current_iteration, pathNum);
    printf("DEBUG: (thread %2d)  x=%d, y=%d, z=%d, parentX=%d, parentY=%d, parentZ=%d, pathNum=%d,shapeType=%d\n",
           omp_get_thread_num(), x, y, z, parentX, parentY, parentZ, pathNum, shapeType);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // As an error-check, confirm that the distance between (parentX, parentY, parentZ) and (x,y,z)
  // is a valid distance:
  {
    int error = TRUE;  // Error flag. Value will be cleared if legal jump is detected.
    int deltaX = abs(x - parentX);
    int deltaY = abs(y - parentY);
    int deltaZ = abs(z - parentZ);
    if (deltaZ)  {
      if ((! deltaX) && (! deltaY))  {
        // We got here, so there's a (legal) vertical via. Clear the error flag:
        error = FALSE;
      }
    }
    else  {
      // We got here, so the parent and child coordinates are on the same routing layer:
      if (   (deltaX + deltaY == 1)            // (delX, delY) = (1,0) or (0,1)
          || ((deltaX == 1) && (deltaY == 1))  // (delX, delY) = (1,1)
          || ((deltaX == 1) && (deltaY == 2))  // (delX, delY) = (1,2)
          || ((deltaX == 2) && (deltaY == 1)))  // (delX, delY) = (2,1)
      {
        // We got here, so the jump is a legal distance. Clear the error flag:
        error = FALSE;
      }
    }

    if (error)  {
      printf("\nERROR: An illegal jump was detected in function calc_congestion_penalty from coordinate\n");
      printf(  "       (%d,%d,%d) to coordinate (%d,%d,%d) for path number %d during iteration %d\n",
             parentX, parentY, parentZ, x, y, z, pathNum, mapInfo->current_iteration );
      printf(  "       Please inform the software developer of this fatal error message.\n\n");
      exit(1);
    }
  }  // End of block for error-checking

  // If cell (x,y,z) is in a pin-swappable zone, then return zero congestion penalty:
  if (cellInfo[x][y][z].swap_zone)
    return(0);

  // Extract the design-rule subset number (0 to 15), which will be used for calculating
  // the congestion penalty:
  const unsigned short target_DR_num    = cellInfo[x][y][z].designRuleSet; // Get the design rule # for destination cell at (x,y,z)
  const unsigned short parent_DR_num    = cellInfo[parentX][parentY][parentZ].designRuleSet; // Get the design rule # for parent cell at (parentX,parentY,parentZ)
  const unsigned short target_DR_subset = user_inputs->designRuleSubsetMap[pathNum][target_DR_num]; // Get the DR subset number of destination cell
  const unsigned short parent_DR_subset = user_inputs->designRuleSubsetMap[pathNum][parent_DR_num]; // Get the DR subset number of parent cell

  // Determine the cost-zone multiplier for the target cell:
  int target_cost_zone_multiplier = user_inputs->traceCostMultiplier[cellInfo[x][y][z].traceCostMultiplierIndex];

  int penalty = 0;  // Congestion penalty between parent and destination
  int path_index;  // Temporary variable for path index

  // If (x,y,z) is directly above or below the parent cell (i.e., in z-direction),
  // then calculate the via-related congestion calculated using function
  // 'calc_via_congestion':
  if ((abs(z - parentZ) == 1) && (x == parentX) && (y == parentY)) {

    #ifdef DEBUG_calc_congestion_penalty
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In calc_congestion_penalty, parent and target are on different layers.\n", omp_get_thread_num());
    }
    #endif

    penalty = calc_via_congestion(pathNum, target_DR_num, target_DR_subset, parent_DR_num, parent_DR_subset, cellInfo,
                                  user_inputs, mapInfo, x, y, parentZ, z, excludeCongestion, excludePathNum, recognizeSelfCongestion);

    #ifdef DEBUG_calc_congestion_penalty
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In calc_congestion_penalty, calc_via_congestion returned a value of %'lu.\n", omp_get_thread_num(), congestion_penalty);
    }
    #endif

  }
  else  {
    // We got here, so parent and target cells are on same routing layer.

    #ifdef DEBUG_calc_congestion_penalty
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In calc_congestion_penalty, parent and target are on the same layer.\n", omp_get_thread_num());
    }
    #endif

    // We first calculate the subset/shapeType index for the routed path, whose shape-type
    // must be a TRACE. This index is simply the subset x 3   +   shapeType:
    int routing_subset_shapeType    = target_DR_subset * NUM_SHAPE_TYPES   +   shapeType;

    // Calculate the congestion penalty associated with the destination cell
    // at location (x,y,z), excluding any cells between the parent and destination.

    // Determine how many paths traverse the destination cell:
    const unsigned int dest_path_count = cellInfo[x][y][z].numTraversingPaths; // Path count associated with destination cell (x,y,z)

    // Check all paths that traverse destination cell:
    for (path_index = 0; path_index < dest_path_count; path_index++)  {

      // Get the path number of the congestion for the current path index:
      int congestion_pathNum   = cellInfo[x][y][z].congestion[path_index].pathNum;

      #ifdef DEBUG_calc_congestion_penalty
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Checking congestion path-index %d, which is path number %d...\n", omp_get_thread_num(), path_index, congestion_pathNum);
      }
      #endif

      // Check whether the congestion should be considered. Congestion should be considered if:
      //  (a) recognizeSelfCongestion is TRUE or the congestion is not from 'path', AND
      //  (b) excludeCongestion is FALSE, OR the congestion is not from 'excludePathNum':
      if (    (recognizeSelfCongestion || (congestion_pathNum != pathNum))       // Item (a) above
          && ((! excludeCongestion) || (congestion_pathNum != excludePathNum)))  // Item (b) above
      {

        // Check whether the congestion has the same shape-type and design-rule subset as the target:
        int congestion_DR_subset = cellInfo[x][y][z].congestion[path_index].DR_subset;
        int congestion_shapeType = cellInfo[x][y][z].congestion[path_index].shapeType;

        #ifdef DEBUG_calc_congestion_penalty
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)   shapeType = %d, congestion_shapeType = %d\n", omp_get_thread_num(), shapeType, congestion_shapeType);
          printf("DEBUG: (thread %2d)   target_DR_subset = %d, congestion_DR_subset = %d\n", omp_get_thread_num(), target_DR_subset, congestion_DR_subset);
        }
        #endif

        if ((shapeType == congestion_shapeType) && (target_DR_subset == congestion_DR_subset)) {

          // Recognize congestion consistent with the following table
          //==============================================================================================
          // congestion_path  |  Path = Normal Net       Path = Diff-pair Net    Path = Pseudo-Net
          // ---------------  |  ----------------------  ----------------------  ----------------------
          //      Normal Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
          //                  |
          //   Diff-pair Net  |  Normal congestion cost  Normal congestion cost  Normal congestion cost
          //                  |                                                    if not related
          //                  |
          //      Pseudo-Net  |  Zero congestion cost    Zero congestion cost    Normal congestion cost
          //==============================================================================================
          // Based on the above table, we recognize congestion only if:
          //   (1) 'congestion_pathNum' is not a pseudo-path and 'pathNum' is not the parent pseudo-path of 'congestion_pathNum'
          // or
          //   (2) 'pathNum' is a pseudo-path and 'congestion_pathNum' is a pseudo-path
          //
          if (   (( ! user_inputs->isPseudoNet[congestion_pathNum])                            // Item #1 in list above
                      && (user_inputs->diffPairToPseudoNetMap[congestion_pathNum] != pathNum)) // Item #1 in list above (continued)
              || ( user_inputs->isPseudoNet[pathNum]                                           // Item #2 in list above
                      && user_inputs->isPseudoNet[congestion_pathNum]) )  {                    // Item #2 in list above (continued)

            // We got here, so it's appropriate to recognize congestion. We now calculate
            // the subset/shapeType index for the congestion. This index is simply
            // the subset x 3   +   shapeType:
            int congestion_subset_shapeType = congestion_DR_subset * NUM_SHAPE_TYPES   +   congestion_shapeType;

            // Amount of congestion added by the current path_index:
            int added_congestion = 0;

            // Next, check whether the routed net and the congestion net are diff-pair partners.
            // If so, then the congestion-related G-cost should be higher because such partners
            // have less freedom to move around:
            if (user_inputs->isDiffPair[pathNum] && (user_inputs->diffPairPartner[pathNum] == congestion_pathNum))  {

              // 'pathNum' is a diff-pair path, and the 'congestion_pathNum' is its diff-pair
              // partner, so add more congestion from the foreign path (by a factor of DIFF_PAIR_PARTNER_TRACE_CONGESTION_FACTOR):
              added_congestion += DIFF_PAIR_PARTNER_TRACE_CONGESTION_FACTOR
                                          * (int)(cellInfo[x][y][z].congestion[path_index].pathTraversalsTimes100
// The following line was commented out and replaced with the subsequent line as an experiment on 6/12/2024:
                                          * mapInfo->traceCongestionMultiplier
//                                        * mapInfo->viaCongestionMultiplier
                                          * user_inputs->detour_distance[target_DR_num][routing_subset_shapeType][target_DR_num][congestion_subset_shapeType]
                                          * target_cost_zone_multiplier);
            }
            else  {
              // 'pathNum' and 'congestion_pathNum' are not diff-pair partners, so add the
              // normal amount of congestion-related G-cost:
              added_congestion += (int)(cellInfo[x][y][z].congestion[path_index].pathTraversalsTimes100
// The following line was commented out and replaced with the subsequent line as an experiment on 6/12/2024:
                                          * mapInfo->traceCongestionMultiplier
//                                        * mapInfo->viaCongestionMultiplier
                                          * user_inputs->detour_distance[target_DR_num][routing_subset_shapeType][target_DR_num][congestion_subset_shapeType]
                                          * target_cost_zone_multiplier);

            }  // End of if/else block for routed path and congestion path being diff-pair partners


            // Add the congestion to 'penalty', which is the variable that will hold the congestion calculated from this function:
            penalty += added_congestion;

            #ifdef DEBUG_calc_congestion_penalty
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) In calc_congestion_penalty, cumulative congestion_penalty is %'lu.\n", omp_get_thread_num(), congestion_penalty);
              printf("DEBUG: (thread %2d)   cellInfo[%d][%d][%d].congestion[%d].pathTraversalsTimes100 = %d\n", omp_get_thread_num(), x, y, z,
                     path_index, cellInfo[x][y][z].congestion[path_index].pathTraversalsTimes100);
              printf("DEBUG: (thread %2d)   mapInfo->congestionMultiplier = %6.3f\n", omp_get_thread_num(), mapInfo->congestionMultiplier);
              printf("DEBUG: (thread %2d)   detour_distance[%d][%d][%d][%d] = %6.3f\n", omp_get_thread_num(), target_DR_num,
                     routing_subset_shapeType, target_DR_num, congestion_subset_shapeType,
                     user_inputs->detour_distance[target_DR_num][routing_subset_shapeType][target_DR_num][congestion_subset_shapeType]);
              printf("DEBUG: (thread %2d)   target_cost_zone_multiplier = %d\n", omp_get_thread_num(), target_cost_zone_multiplier);
            }
            #endif

          }  // End of if-block for pathNum and congestion_pathNum not being related diff-pair nets.
        }  // End of if-block for congestion having the same shape-type and design-rule subset as the target
      }  // End of if-block for foreign congestion
    }  // End of for-loop for index 'path_index'

    #ifdef DEBUG_calc_congestion_penalty
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In calc_congestion_penalty, adding %'ld congestion due to destination cell (%d,%d,%d) from parent (%d,%d,%d)\n",
             omp_get_thread_num(), congestion_penalty, x, y, z, parentX, parentY, parentZ);
    }
    #endif


    // Calculate congestion of the path *between* (x,y,z) and
    // (parentX, parentY, parentZ). In the diagram below, we need to check
    // 12 cells that are diagonal from the middle (parent) cell:
    // cell #1, 2, 3, 4, 6, 7, 10, 11, 13, 14, 15, and 16.
    //
    //  --------------------  XX = parent cell
    //   |  | 1|  | 2|  |     nn = child cells
    //  --------------------
    //   | 3| 4| 5| 6| 7|
    //  --------------------
    //   |  | 8|XX| 9|  |
    //  --------------------
    //   |10|11|12|13|14|
    //  --------------------
    //   |  |15|  |16|  |
    //  --------------------

    // Cell #1 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                 -1, 2,   -1, 1,   0, 1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #2 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                  1, 2,    0, 1,   1, 1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #3 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                 -2, 1,   -1, 1,  -1, 0,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #4 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                 -1, 1,   -1, 0,   0, 1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #6 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                  1, 1,    0, 1,   1, 0,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #7 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                  2, 1,    1, 1,   1, 0,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #10 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                 -2,-1,   -1, 0,  -1,-1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #11 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                 -1,-1,   -1, 0,   0,-1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #13 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                  1,-1,    1, 0,   0,-1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #14 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                  2,-1,    1, 0,   1,-1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #15 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                 -1,-2,   -1,-1,   0,-1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);

    // Cell #16 in diagram above:
    penalty += calc_corner_congestion(pathNum, shapeType, cellInfo, user_inputs, mapInfo, parentX, parentY, parentZ, x, y,
                                                  1,-2,    0,-1,   1,-1,      excludeCongestion, excludePathNum, recognizeSelfCongestion);


    #ifdef DEBUG_calc_congestion_penalty
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In calc_congestion_penalty, congestion is %'lu due to destination cell (%d,%d,%d) and corner cells from parent (%d,%d,%d)\n",
             omp_get_thread_num(), congestion_penalty, x, y, z, parentX, parentY, parentZ);
    }
    #endif

  }  // End of else-block for parent and target being on same layers


  // Multiply the congestion penalty by 2^NON_PIN_SWAP_EXPONENT (2^30) and return to calling routine:
  long congestion_penalty = penalty;
  congestion_penalty = congestion_penalty << NON_PIN_SWAP_EXPONENT;

  #ifdef DEBUG_calc_congestion_penalty
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Returning %'ld from function 'calc_congestion_penalty' between parent (%d,%d,%d) and child (%d,%d,%d)\n\n",
           omp_get_thread_num(), congestion_penalty, parentX, parentY, parentZ, x, y, z);
    }
    #endif

  // Return the calculated congestion penalty to the calling program:

  return(congestion_penalty);

}  // End of function 'calc_congestion_penalty'


//-----------------------------------------------------------------------------
// Name: allocateMapInfo
// Desc: Allocate memory for variable of type MapInfo_t.
//-----------------------------------------------------------------------------
void allocateMapInfo(MapInfo_t *mapInfo, int num_nonPseudo_nets, int num_pseudo_nets,
                     int num_routing_layers)  {

  //
  // Allocate memory for data structures required for each user-defined net, each pseudo-net, and the
  // Acorn-defined 'global repellent' net:
  //
  int max_routed_nets = num_nonPseudo_nets + num_pseudo_nets + 1;

  mapInfo->start_cells                        = malloc(max_routed_nets * sizeof(Coordinate_t));
  mapInfo->end_cells                          = malloc(max_routed_nets * sizeof(Coordinate_t));
  mapInfo->swapZone                           = malloc(max_routed_nets * sizeof(short unsigned));
  mapInfo->diff_pair_terms_swapped            = malloc(max_routed_nets * sizeof(char * ));
  mapInfo->start_end_terms_swapped            = malloc(max_routed_nets * sizeof(char * ));
  mapInfo->addPseudoTraceCongestionNearVias   = malloc(max_routed_nets * sizeof(char * ));
  mapInfo->diffPairStartTermPitchMicrons      = malloc(max_routed_nets * sizeof(float * ));
  mapInfo->diffPairEndTermPitchMicrons        = malloc(max_routed_nets * sizeof(float * ));

  // Allocate memory for 2-dimensional arrays whose first dimension ranges from
  // zero to (num_nets + num_pseudo_nets + 1), which is 'max_routed_nets'.
  for (int pathNum = 0; pathNum < max_routed_nets; pathNum++)  {
    // Allocate memory for 2nd dimension of array 'addPseudoTraceCongestionNearVias':
    mapInfo->addPseudoTraceCongestionNearVias[pathNum] = malloc(num_routing_layers * sizeof(char));
    if (mapInfo->addPseudoTraceCongestionNearVias[pathNum] == 0)  {
      printf("Error: Failed to allocate memory for 'addPseudoTraceCongestionNearVias[%d]' array.\n", pathNum);
      exit (1);
    }  // End of if-block

    // Initialize each element of addPseudoTraceCongestionNearVias[net_number][layer] to FALSE:
    for (int layer = 0; layer < num_routing_layers; layer++)  {
      mapInfo->addPseudoTraceCongestionNearVias[pathNum][layer] = FALSE;  // Don't initially add TRACE pseudo-congestion near pseudo-vias.
      // printf("DEBUG: Initialized mapInfo->addPseudoTraceCongestionNearVias[%d][%d] to %d\n", i, layer,
      //        mapInfo->addPseudoTraceCongestionNearVias[i][layer]);
    }  // End of for-loop for index 'layer' (0 to num_routing_layers)

    // Initialize 'diff_pair_terms_swapped' to FALSE for all nets:
    mapInfo->diff_pair_terms_swapped[pathNum] = FALSE;

    // Initialize 'start_end_terms_swapped' to FALSE for all nets:
    mapInfo->start_end_terms_swapped[pathNum] = FALSE;

  }  // End of for-loop for index 'pathNum' (0 to max_routed_nets)

  // Initialize maximum interaction radius values to zero for each routing layer:
  for (int layer = 0; layer < maxRoutingLayers; layer++)  {
    mapInfo->maxInteractionRadiusCellsOnLayer[layer] = 0.0;
    mapInfo->maxInteractionRadiusSquaredOnLayer[layer] = 0.0;
  }

}  // End of function 'allocateMapInfo'


//-----------------------------------------------------------------------------
// Name: initializePathfinder
// Desc: Allocates small amount of memory for the arrays used to store paths.
//       Because we don't know the eventual lengths of each path, only space for
//       one (x,y,z) location is allocated in each of the two arrays:
//       (1) pathCoords and (2) contigPathCoords.
//-----------------------------------------------------------------------------
void initializePathfinder(int numPaths, int *pathLengths, Coordinate_t *pathCoords[],
                          int *contiguousPathLengths, Coordinate_t *contigPathCoords[])  {

  // Allocate memory for the second dimension of the two 2-dimensional arrays:
  // printf("\nDEBUG: Entered function 'InitializePathfinder'...\n");
  for (int i = 0; i < numPaths; i++) {

    // Allocate space for one (x,y,z) location in pathCoords array:
    pathCoords[i] = malloc(sizeof(Coordinate_t));
    if (pathCoords[i] == 0)  {
      printf("\n\nError: Failed to initially allocate memory for array pathCoords[%d]\n\n", i);
      exit(1);
    }
    // printf("       Allocated memory for 0th element of pathCoords[%d] at address %p.\n", i, pathCoords[i]);
    // printf("       Address of pathCoords[%d][0] is %p.\n", i, &pathCoords[i][0]);

    // Allocate space for one (x,y,z) location in contigPathCoords array:
    contigPathCoords[i] = malloc(sizeof(Coordinate_t));
    if (contigPathCoords[i] == 0)  {
      printf("\n\nError: Failed to initially allocate memory for array contigPathCoords[%d]\n\n", i);
      exit(1);
    }

    // Initialize to zero the number of elements for arrays pathLengths and contiguousPathLengths:
    pathLengths[i] = 0;
    contiguousPathLengths[i] = 0;

  }  // End of for-loop for variable 'i'

}  // End of function 'initializePathfinder'


//-----------------------------------------------------------------------------
// Name: allocateCellInfo
// Desc: Allocates memory for the 3D cellInfo array.
//-----------------------------------------------------------------------------
CellInfo_t *** allocateCellInfo(MapInfo_t *mapInfo)  {

  // printf("DEBUG: About to allocate memory for 3D 'cellInfo' structure from (0,0,0) to (%d,%d,%d)\n",
  //        mapInfo->mapWidth, mapInfo->mapHeight, mapInfo->numLayers);

  // Dynamically allocate memory from large heap space for 3D 'cellInfo' array:
  CellInfo_t ***cellInfo = malloc(sizeof(CellInfo_t **) * mapInfo->mapWidth);
  if (cellInfo == 0) {
    printf("\n\nERROR: Failed to allocate memory for the X-direction of a 'cellInfo' matrix\n");
    printf(    "       with dimensions %d cells wide by %d cells high by %d routing layers. \n",
            mapInfo->mapWidth, mapInfo->mapHeight, mapInfo->numLayers);
    printf(    "       Please inform the software developer of this fatal error.\n\n");
    exit (1);
  }
  // For each row in 'cellInfo' matrix, malloc space for its buckets
  // and add it to the array of arrays:
  for (int i = 0; i < mapInfo->mapWidth; i++)  {
    cellInfo[i] = malloc(mapInfo->mapHeight * sizeof(CellInfo_t *));
    if (cellInfo[i] == 0) {
      printf("\n\nERROR: Failed to allocate memory for the Y-direction of a 'cellInfo' matrix\n");
      printf(    "       with dimensions %d cells wide by %d cells high by %d routing layers. \n",
              mapInfo->mapWidth, mapInfo->mapHeight, mapInfo->numLayers);
      printf(    "       Please inform the software developer of this fatal error.\n\n");
      exit (1);
    }

    // For each cell in 2D 'cellInfo' matrix, malloc space for a 3rd dimension:
    for (int j = 0; j < mapInfo->mapHeight; j++ )  {
      cellInfo[i][j] = malloc((mapInfo->numLayers + 1) * sizeof(CellInfo_t));
      if (cellInfo[i][j] == 0) {
        printf("\n\nERROR: Failed to allocate memory for the Z-direction of a 'cellInfo' matrix\n");
        printf(    "       with dimensions %d cells wide by %d cells high by %d routing layers. \n",
                mapInfo->mapWidth, mapInfo->mapHeight, mapInfo->numLayers);
        printf(    "       Please inform the software developer of this fatal error.\n\n");
        exit (1);
      }

      // For each element in the 3rd dimension, initialize the pointers for
      // elements 'congestion' and 'pathCenters' to NULL, and initialize
      // elements 'numTraversingPaths' and 'numTraversingPathCenters' to zero.
      for (int k = 0; k <= mapInfo->numLayers; k++)  {
        cellInfo[i][j][k].congestion  = NULL;
        cellInfo[i][j][k].pathCenters = NULL;
        cellInfo[i][j][k].numTraversingPaths = 0;
        cellInfo[i][j][k].numTraversingPathCenters = 0;
      }

    }  // End of 'j' for-loop
  }  // End of 'i' for-loop

  // Return the pointer to the calling function:
  return cellInfo;

}  // End of function 'allocateCellInfo'


//-----------------------------------------------------------------------------
// Name: initializeCellInfo
// Desc: Initialize the 3D cellInfo array. The format of each element is
//       provided in the header file.
//-----------------------------------------------------------------------------
void initializeCellInfo(CellInfo_t ***cellInfo, MapInfo_t *mapInfo)  {

  // int cellInfoSize = 0;  // Number of cells in 3D cellInfo array

  // printf("DEBUG: About to initialize 3D 'cellInfo' structure from (0,0,0) to (%d,%d,%d)\n",
  //        mapInfo->mapWidth, mapInfo->mapHeight, mapInfo->numLayers);

  // Initialize 3-dimensional 'cellInfo' matrix to reflect no unwalkable cells
  // and zero paths traversing the cell:
  for (int i = 0; i < mapInfo->mapWidth; i++)  {
    for (int j = 0; j < mapInfo->mapHeight; j++)  {
      for (int k = 0; k <= mapInfo->numLayers; k++)  {

        cellInfo[i][j][k].forbiddenTraceBarrier       = FALSE;
        cellInfo[i][j][k].forbiddenUpViaBarrier       = FALSE;
        cellInfo[i][j][k].forbiddenDownViaBarrier     = FALSE;
        cellInfo[i][j][k].forbiddenProximityBarrier   = 0;
        cellInfo[i][j][k].forbiddenProximityPinSwap   = 0;
        cellInfo[i][j][k].designRuleSet               = 0;
        cellInfo[i][j][k].traceCostMultiplierIndex    = 0;
        cellInfo[i][j][k].viaUpCostMultiplierIndex    = 0;
        cellInfo[i][j][k].viaDownCostMultiplierIndex  = 0;
        cellInfo[i][j][k].routing_layer_metal_fill    = FALSE;
        cellInfo[i][j][k].pseudo_routing_layer_metal_fill  = FALSE;
        cellInfo[i][j][k].DRC_flag                    = FALSE;
        cellInfo[i][j][k].via_above_metal_fill        = FALSE;
        cellInfo[i][j][k].via_below_metal_fill        = FALSE;
        cellInfo[i][j][k].pseudo_via_above_metal_fill = FALSE;
        cellInfo[i][j][k].pseudo_via_below_metal_fill = FALSE;
        cellInfo[i][j][k].via_above_DRC_flag          = FALSE;
        cellInfo[i][j][k].center_line_flag            = FALSE;
        cellInfo[i][j][k].center_viaUp_flag           = FALSE;
        cellInfo[i][j][k].center_viaDown_flag         = FALSE;
        cellInfo[i][j][k].near_a_net                  = FALSE;
        cellInfo[i][j][k].swap_zone                   = 0;
        cellInfo[i][j][k].explored                    = FALSE;
        cellInfo[i][j][k].explored_PP                 = FALSE;
        cellInfo[i][j][k].flag                        = FALSE;

        // If the current (i,j,k) coordinate contains any congestion, then
        // free the 'congestion' array and reset the number of traversing
        // paths to zero:
        if (cellInfo[i][j][k].numTraversingPaths)  {
          free(cellInfo[i][j][k].congestion);
          cellInfo[i][j][k].congestion         = NULL;
          cellInfo[i][j][k].numTraversingPaths = 0;
        }

        // If the current (i,j,k) coordinate contains any traversing path-centers,
        // then free the 'pathCenters' array and reset the number of traversing
        // path-centers to zero:
        if (cellInfo[i][j][k].numTraversingPathCenters)  {
          free(cellInfo[i][j][k].pathCenters);
          cellInfo[i][j][k].pathCenters              = NULL;
          cellInfo[i][j][k].numTraversingPathCenters = 0;
        }

        // cellInfoSize++;
      }  // End of for-loop for index 'k'
    }  // End of for-loop for index 'j'
  }  // End of for-loop for index 'i'
  // printf("DEBUG: %d cells initialized in 'cellInfo' array.\n", cellInfoSize);

}  // End of function 'initializeCellInfo'


//-----------------------------------------------------------------------------
// Name: reInitializeCellInfo
// Desc: At each (x,y,z) cell in the map, clear the following variables/arrays so they can
//       be re-populated later on:
//          (1) center_line_flag Boolean variable
//          (2) center_viaUp_flag Boolean variable
//          (3) center_viaDown_flag Boolean variable
//          (4) near_a_net Boolean variable
//          (5) routing_layer_metal_fill Boolean variable
//          (6) pseudo_routing_layer_metal_fill Boolean variable
//          (7) DRC_flag Boolean variable
//          (8) via_above_metal_fill Boolean variable
//          (9) via_below_metal_fill Boolean variable
//         (10) pseudo_via_above_metal_fill Boolean variable
//         (11) via_above_DRC_flag Boolean variable
//         (12) pathCenters array
//         (13) numTraversingPathCenters variable
//-----------------------------------------------------------------------------
void reInitializeCellInfo(const MapInfo_t *mapInfo, CellInfo_t ***cellInfo)  {

  for (int z = 0; z < mapInfo->numLayers; z++)  {
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      for (int x = 0; x < mapInfo->mapWidth; x++)  {
        cellInfo[x][y][z].center_line_flag                = FALSE; // Clear flag for trace centerlines
        cellInfo[x][y][z].center_viaUp_flag               = FALSE; // Clear flag for via centerlines
        cellInfo[x][y][z].center_viaDown_flag             = FALSE; // Clear flag for via centerlines
        cellInfo[x][y][z].near_a_net                      = FALSE; // Clear flag for cells near a path
        cellInfo[x][y][z].routing_layer_metal_fill        = FALSE;
        cellInfo[x][y][z].pseudo_routing_layer_metal_fill = FALSE;
        cellInfo[x][y][z].DRC_flag                        = FALSE;
        cellInfo[x][y][z].via_above_metal_fill            = FALSE;
        cellInfo[x][y][z].via_below_metal_fill            = FALSE;
        cellInfo[x][y][z].pseudo_via_above_metal_fill     = FALSE;
        cellInfo[x][y][z].pseudo_via_below_metal_fill     = FALSE;
        cellInfo[x][y][z].via_above_DRC_flag              = FALSE;

        if(cellInfo[x][y][z].numTraversingPathCenters)  {
          free(cellInfo[x][y][z].pathCenters);
          cellInfo[x][y][z].pathCenters = NULL; // Set pointer to NULL as a precaution
          cellInfo[x][y][z].numTraversingPathCenters = 0;
        }  // End of if-block
      }  // End of for-loop for index 'x'
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'z'

}  // End of function 'reInitializeCellInfo'


//-----------------------------------------------------------------------------
// Name: initializeRoutability
// Desc: Initialize elements in variable 'routability'.
//-----------------------------------------------------------------------------
void initializeRoutability(RoutingMetrics_t *routability, const MapInfo_t *mapInfo,
                           unsigned char initialize_ALL_elements)  {

  int max_routed_nets = mapInfo->numPaths + mapInfo->numPseudoPaths;

  //
  // Initialize components in routability structure to zero for current iteration:
  //
  routability->total_num_DRC_cells                = 0;
  routability->num_pseudo_DRC_cells               = 0;
  routability->num_nonPseudo_DRC_cells            = 0;
  routability->total_cost                         = 0;
  routability->total_pseudo_cost                  = 0;
  routability->total_nonPseudo_cost               = 0;
  routability->total_lateral_length_mm            = 0.0;
  routability->total_lateral_pseudo_length_mm     = 0.0;
  routability->total_lateral_nonPseudo_length_mm  = 0.0;
  routability->total_vias                         = 0;
  routability->total_pseudo_vias                  = 0;
  routability->total_nonPseudo_vias               = 0;
  routability->num_DRCfree_paths                  = 0;
  routability->num_paths_with_DRCs                = 0;
  routability->total_elapsed_time                 = 0;
  routability->total_explored_cells               = 0;
  routability->best_iteration = 1;

  for (int i = 0; i < max_routed_nets; i++)  {
    routability->path_cost[i]                  = 0;
    routability->num_adjacent_steps[i]         = 0;
    routability->num_diagonal_steps[i]         = 0;
    routability->num_knights_steps[i]          = 0;
    routability->lateral_path_lengths_mm[i]    = 0.0;
    routability->path_DRC_cells[i]             = 0;
    routability->randomize_congestion[i]       = FALSE;
    routability->one_path_traversal[i]         = 0;
    routability->num_vias[i]                   = 0;
    routability->recent_path_DRC_fraction[i]   = 0.0;
    routability->recent_path_DRC_iterations[i] = 0;

    // Variable fractionRecentIterationsWithoutPathDRCs[] is only initialized at the first iteration.
    // This allows this variable's values to be used to calculate the amount of congestion to add at
    // the beginning of calcRoutabilityMetrics():
    if (mapInfo->current_iteration == 1)
      routability->fractionRecentIterationsWithoutPathDRCs[i] = 1.0;

    for (int j = 0; j < max_routed_nets; j++)  {
      routability->crossing_matrix[i][j]       = 0;
    }  // End of 'j' for-loop
  }  // End of for-loop for index 'i' (0 to max_routed_nets)

  // Initialize path_DRC_cells_by_layer 2-dimensional matrix:
  for (int i = 0; i < mapInfo->numPaths; i++)  {
    for (int j = 0; j < mapInfo->numLayers; j++)  {
      routability->path_DRC_cells_by_layer[i][j] = 0;
    }  // End of 'j' for-loop
  }  // End of 'i' for-loop

  // Initialize layer_DRC_cells for each layer:
  for (int i = 0; i < mapInfo->numLayers; i++)  {
    routability->layer_DRC_cells[i] = 0;
  }  // End of 'i' for-loop

  //
  // Some elements should only be initialized prior to the first iteration because they contain
  // information derived from previous iterations. Initialize these elements only if input
  // parameter 'initialize_ALL_elements' us set to TRUE:
  //
  if (initialize_ALL_elements)  {

    // Initialize to 1.0 the fractionRecentIterationsWithoutMapDRCs variable, which is used on the 1st iteration,
    // before calcRoutabilityMetrics is even called:
    routability->fractionRecentIterationsWithoutMapDRCs = 1.0;

    // Initialize to zero the most recent iteration for which a change was made to
    // the routing algorithm:
    routability->latestAlgorithmChange = 0;

    // Initialize to zero the number of times the autorouter swapped the start/end
    // terminals of nets with DRCs in order to improve the routing:
    routability->num_startEnd_terminal_swaps = 0;

    // Initialize to zero the number of times the autorouter changed and reduced
    // the trace and via congestion sensitivities in order to improve the routing:
    routability->num_viaCongSensitivity_changes   = 0;
    routability->num_traceCongSensitivity_changes = 0;
    routability->num_viaCongSensitivity_reductions   = 0;
    routability->num_traceCongSensitivity_reductions = 0;
    routability->num_viaCongSensitivity_stableRoutingMetrics   = 0;
    routability->num_traceCongSensitivity_stableRoutingMetrics = 0;

    // Initialize to zero the iteration at which the auto-router achieved the required
    // number of DRC-free iterations:
    routability->DRC_free_threshold_achieved = 0;

    // Initialize elements of the 'traceCongSensitivityMetrics' and
    // 'viaCongSensitivityMetrics' structures, which keep track of routing
    // metrics for each level of congestion sensitivity:
    for (int i = 0; i < NUM_CONG_SENSITIVITES; i++)  {
      // Initialize the congestion sensitivity for each index. This
      // expression results in values of 100%, 141%, 200%, 283%, 400%, 566%, 800%, etc:

// The following two lines were commented out and replaced with the subsequent two lines as an experiment on 6/27/2024:
      routability->traceCongSensitivityMetrics[i].dynamicParameter = (unsigned int)(100.0 * powf(2.0, i/2.0));
      routability->viaCongSensitivityMetrics[i].dynamicParameter   = (unsigned int)(100.0 * powf(2.0, i/2.0));

//    routability->traceCongSensitivityMetrics[i].dynamicParameter = (unsigned int)(100.0 * powf(3.0, i/2.0));
//    routability->viaCongSensitivityMetrics[i].dynamicParameter   = (unsigned int)(100.0 * powf(3.0, i/2.0));

//    routability->traceCongSensitivityMetrics[i].dynamicParameter = (unsigned int)(100.0 * powf(2.0, i));
//    routability->viaCongSensitivityMetrics[i].dynamicParameter   = (unsigned int)(100.0 * powf(2.0, i));

//    routability->traceCongSensitivityMetrics[i].dynamicParameter = (unsigned int)(100.0 * powf(3.0, i));
//    routability->viaCongSensitivityMetrics[i].dynamicParameter   = (unsigned int)(100.0 * powf(3.0, i));

//    routability->traceCongSensitivityMetrics[i].dynamicParameter = (unsigned int)(100.0 * powf(4.0, i));
//    routability->viaCongSensitivityMetrics[i].dynamicParameter   = (unsigned int)(100.0 * powf(4.0, i));

      // printf("DEBUG: viaCongSensitivityMetrics[%d].dynamicParameter = %d%%\n", i, routability->viaCongSensitivityMetrics[i].dynamicParameter);

      // Initialize 'iterationOfMeasuredMetrics' to iteration #0, indicating that no
      // routing metrics have yet been measured for any congestion sensitivity index:
      routability->traceCongSensitivityMetrics[i].iterationOfMeasuredMetrics = 0;
      routability->viaCongSensitivityMetrics[i].iterationOfMeasuredMetrics   = 0;

      // Initialize the other elements of the 'traceCongSensitivityMetrics' and
      // 'traceCongSensitivityMetrics' structures to 0.0:
      routability->traceCongSensitivityMetrics[i].avgNonPseudoNetsWithDRCs      = 0.0;
      routability->viaCongSensitivityMetrics[i].avgNonPseudoNetsWithDRCs        = 0.0;
      routability->traceCongSensitivityMetrics[i].stdErrNonPseudoNetsWithDRCs   = 0.0;
      routability->viaCongSensitivityMetrics[i].stdErrNonPseudoNetsWithDRCs     = 0.0;

      routability->traceCongSensitivityMetrics[i].avgNonPseudoRoutingCost       = 0.0;
      routability->viaCongSensitivityMetrics[i].avgNonPseudoRoutingCost         = 0.0;
      routability->traceCongSensitivityMetrics[i].stdErrNonPseudoRoutingCost    = 0.0;
      routability->viaCongSensitivityMetrics[i].stdErrNonPseudoRoutingCost      = 0.0;

      routability->traceCongSensitivityMetrics[i].fractionIterationsWithoutDRCs = 0.0;
      routability->viaCongSensitivityMetrics[i].fractionIterationsWithoutDRCs   = 0.0;
    }  // End of for-loop for index 'i' (0 to NUM_CONG_SENSITIVITES)


    for (int i = 0; i <= mapInfo->max_iterations; i++)  {
      routability->nonPseudoPathLengths[i]          = 0.0;
      routability->nonPseudo_num_DRC_cells[i]       = 0;
      routability->nonPseudo_num_via2via_DRC_cells[i]     = 0;
      routability->nonPseudo_num_trace2trace_DRC_cells[i] = 0;
      routability->nonPseudo_num_trace2via_DRC_cells[i]   = 0;
      routability->nonPseudoViaCounts[i]            = 0;
      routability->nonPseudoPathCosts[i]            = 0;
      routability->numNonPseudoDRCnets[i]           = 0;
      routability->nonPseudoPathCosts_stdDev_trailing_10_iterations[i] = 0.0;
      routability->nonPseudoPathCosts_slope_trailing_10_iterations[i] = 0.0;
      routability->inMetricsPlateau[i]              = FALSE;
      routability->swapStartAndEndTerms[i]          = FALSE;
      routability->changeViaCongSensitivity[i]      = FALSE;
      routability->enablePseudoTraceCongestion[i]   = FALSE;
      routability->cumulative_DRCfree_iterations[i] = 0;
      routability->iteration_explored_cells[i]      = 0;
      routability->iteration_elapsed_time[i]        = 0;
    }

    // Initialize to zero the elements of array 'recent_path_DRC_cells[][]':
    for (int i = 0; i < max_routed_nets; i++)  {
      for (int j = 0; j < numIterationsToReEquilibrate; j++)  {
        routability->recent_path_DRC_cells[i][j] = 0;
      }  // End of for-loop for index 'j' (0 to numIterationsToReEquilibrate)

      // Initialize the following two elements only once. Do not repeatedly re-initialize these two
      // elements; they are written by function findPath()!
      routability->path_elapsed_time[i]          = 0;
      routability->path_explored_cells[i]        = 0;

    }  // End of for-loop for index 'i' (0 to max_routed_nets)

    // Initialize recent_DRC_flags_by_pseudoPath_layer 2-dimensional matrix:
    for (int i = 0; i < mapInfo->numPseudoPaths; i++)  {
      for (int j = 0; j < mapInfo->numLayers; j++)  {
        routability->recent_DRC_flags_by_pseudoPath_layer[i][j] = 0;
      }  // End of 'j' for-loop
    }  // End of 'i' for-loop

  }  // End of if-block for initialize_ALL_elements == TRUE

}  // End of function 'initializeRoutability'


//-----------------------------------------------------------------------------
// Name: allocatePathFindingArrays
// Desc: Allocates memory for the large arrays used by the path-finding
//       function, findPath().
//-----------------------------------------------------------------------------
void allocatePathFindingArrays(PathFinding_t *pathFinding, MapInfo_t *mapInfo)  {

  // printf("DEBUG: Entered allocatePathFindingArrays in thread %d with pathFinding address = %p\n", omp_get_thread_num(), pathFinding);

  int i, j;  // Indices for coordinates in 3D map

  //
  // Allocate memory for 3D matrices that span the length, width, and height/thickness
  // of the 3-dimensional map:
  //

  // 1st dimension of 'whichList' array:
  pathFinding->whichList    = malloc(sizeof(char **) * (mapInfo->mapWidth+1) );
  if (pathFinding->whichList == 0) {
    printf("Error: Failed to allocate memory for 'whichList' matrix.\n");
    exit(1);
  }  // End of if-block

  // 1st dimension of 'parentCoords' array:
  pathFinding->parentCoords = malloc(sizeof(Coordinate_t **) * (mapInfo->mapWidth+1) );
  if (pathFinding->parentCoords == 0) {
    printf("Error: Failed to allocate memory for 'parentCoords' matrix.\n");
    exit(1);
  }  // End of if-block

  // 1st dimension of 'Gcost' array:
  pathFinding->Gcost = malloc(sizeof(unsigned long **) * (mapInfo->mapWidth+1) );
  if (pathFinding->Gcost == 0) {
    printf("Error: Failed to allocate memory for 'Gcost' matrix.\n");
    exit(1);
  }  // End of if-block

  // 1st dimension of 'sortNumber' array:
  pathFinding->sortNumber = malloc(sizeof(int **) * (mapInfo->mapWidth+1) );
  if (pathFinding->sortNumber == 0) {
    printf("Error: Failed to allocate memory for 'sortNumber' matrix.\n");
    exit(1);
  }  // End of if-block


  // Iterate over the 1st dimension to allocate memory for 2nd dimension:
  for (i = 0; i < (mapInfo->mapWidth+1) ; i++)  {

    // 2nd dimension of 'whichList' array:
    pathFinding->whichList[i] = malloc(sizeof(char *) * (mapInfo->mapHeight+1));
    if (pathFinding->whichList[i] == 0) {
      printf("Error: Failed to allocate memory for 'whichList' matrix.\n");
      exit(1);
    }  // End of if-block

    // 2nd dimension of 'parentCoords' array:
    pathFinding->parentCoords[i] = malloc(sizeof(Coordinate_t *) * (mapInfo->mapHeight+1));
    if (pathFinding->parentCoords[i] == 0) {
      printf("Error: Failed to allocate memory for 'parentCoords' matrix.\n");
      exit(1);
    }  // End of if-block

    // 2nd dimension of 'Gcost' array:
    pathFinding->Gcost[i] = malloc(sizeof(unsigned long *) * (mapInfo->mapHeight+1));
    if (pathFinding->Gcost[i] == 0) {
      printf("Error: Failed to allocate memory for 'Gcost' matrix.\n");
      exit(1);
    }  // End of if-block

    // 2nd dimension of 'sortNumber' array:
    pathFinding->sortNumber[i] = malloc(sizeof(int *) * (mapInfo->mapHeight+1));
    if (pathFinding->sortNumber[i] == 0) {
      printf("Error: Failed to allocate memory for 'sortNumber' matrix.\n");
      exit(1);
    }  // End of if-block

    // Iterate over the 2nd dimension to allocate memory for 3rd dimension:
    for (j = 0; j < (mapInfo->mapHeight+1); j++)  {

      // 3rd dimension of 'whichList' array:
      pathFinding->whichList[i][j] = malloc(sizeof(char) * (mapInfo->numLayers+1) );
      if (pathFinding->whichList[i][j] == 0)  {
        printf("Error: Failed to allocate memory for 'whichList' matrix.\n");
        exit(1);
      }  // End of if-block

      // 3rd dimension of 'parentCoords' array:
      pathFinding->parentCoords[i][j] = malloc(sizeof(Coordinate_t) * (mapInfo->numLayers+1) );
      if (pathFinding->parentCoords[i][j] == 0)  {
        printf("Error: Failed to allocate memory for 'parentCoords' matrix.\n");
        exit(1);
      }  // End of if-block

      // 3rd dimension of 'Gcost' array:
      pathFinding->Gcost[i][j] = malloc(sizeof(unsigned long) * (mapInfo->numLayers+1) );
      if (pathFinding->Gcost[i][j] == 0)  {
        printf("Error: Failed to allocate memory for 'Gcost' matrix.\n");
        exit(1);
      }  // End of if-block

      // 3rd dimension of 'sortNumber' array:
      pathFinding->sortNumber[i][j] = malloc(sizeof(int) * (mapInfo->numLayers+1) );
      if (pathFinding->sortNumber[i][j] == 0)  {
        printf("Error: Failed to allocate memory for 'sortNumber' matrix.\n");
        exit(1);
      }  // End of if-block

    }  // End of for-loop for index 'j'
  }  // End of for-loop for index 'i'

  //
  // Now that we're done allocating memory for 3D arrays, allocate memory
  // for 1-dimensional arrays:
  //

  // Allocate memory for 'openListCoords' array:
  pathFinding->openListCoords = malloc(sizeof(Coordinate_t) * (mapInfo->mapWidth * mapInfo->mapHeight * mapInfo->numLayers + 2) );
  if (pathFinding->openListCoords == 0) {
    printf("\n\nError: Failed to allocate memory for 'openListCoords' array.\n\n");
    exit(1);
  }

  // Allocate memory for 'Fcost' array:
  pathFinding->Fcost = malloc(sizeof(unsigned long) * (mapInfo->mapWidth * mapInfo->mapHeight * mapInfo->numLayers + 2) );
  if (pathFinding->Fcost == 0) {
    printf("\\nError: Failed to allocate memory for 'Fcost' array.\n\n");
    exit(1);
  }

  // Allocate memory for 'Hcost' array:
  pathFinding->Hcost = malloc(sizeof(unsigned long) * (mapInfo->mapWidth * mapInfo->mapHeight * mapInfo->numLayers + 2) );
  if (pathFinding->Hcost == 0) {
    printf("\n\nError: Failed to allocate memory for 'Hcost' array.\n\n");
    exit(1);
  }

  // Allocate memory for 'openList' array:
  pathFinding->openList = malloc(sizeof(int) * (mapInfo->mapWidth * mapInfo->mapHeight * mapInfo->numLayers + 2) );
  if (pathFinding->openList == 0) {
    printf("\n\nError: Failed to allocate memory for 'openList' array.\n\n");
    exit(1);
  }

}  // End of function 'allocatePathFindingArrays'


//-----------------------------------------------------------------------------
// Name: endPathfinder
// Desc: Free memory in the arrays that are used for storing path coordinates.
//       This function frees the memory that was allocated in function
//       'initializePathfinder'.
//-----------------------------------------------------------------------------
void endPathfinder(int numPaths, Coordinate_t *pathCoords[],
                   Coordinate_t *contigPathCoords[])  {

  for (int i = 0; i < numPaths; i++)  {
    free(pathCoords[i]);
    pathCoords[i] = NULL;  // Set pointer to NULL as a precaution

    free(contigPathCoords[i]);
    contigPathCoords[i] = NULL;  // Set pointer to NULL as a precaution
  }
}  // End of function 'endPathfinder'


//-----------------------------------------------------------------------------
// Name: freeMemory_cellInfo
// Desc: Free memory that was allocated dynamically in function
//       'allocateCellInfo'.
//-----------------------------------------------------------------------------
void freeMemory_cellInfo(MapInfo_t *mapInfo, CellInfo_t ***cellInfo)  {

  // Free memory in 3-dimensional cellInfo array:
  for (int x = 0; x < mapInfo->mapWidth; x++)  {

    // For each cell in 2D 'cellInfo' matrix, free space for the 3rd dimension:
    for (int y = 0; y < mapInfo->mapHeight; y++ )  {

      // For each cell in 3D 'CellInfo' matrix, free any structure elements that
      // were dynamically allocated:
      for (int z = 0; z <= mapInfo->numLayers; z++)  {
        // if (cellInfo[x][y][z].congestion != NULL)  {
        //   printf("DEBUG: Memory freed for cellInfo[%d][%d][%d].congestion at address %p\n", x, y, z, cellInfo[x][y][z].congestion);
        // }
        free(cellInfo[x][y][z].congestion);    cellInfo[x][y][z].congestion = NULL;
        free(cellInfo[x][y][z].pathCenters);   cellInfo[x][y][z].pathCenters = NULL;
      }  // End of 'z' for-loop

      free(cellInfo[x][y]);
      cellInfo[x][y] = NULL;
    }  // End of 'y' for-loop

    free(cellInfo[x]);
    cellInfo[x] = NULL;
  }  // End of 'x' for-loop

  free(cellInfo);
  cellInfo = NULL;

}  // End of function 'freeMemory_cellInfo'


//-----------------------------------------------------------------------------
// Name: freePathFindingArrays
// Desc: Free memory that was allocated dynamically in function
//       'allocatePathFindingArrays'.
//-----------------------------------------------------------------------------
void freePathFindingArrays(PathFinding_t *pathFinding, MapInfo_t *mapInfo)  {

  int i, j;  // Indices for coordinates in 3D map

  //
  // Free memory for 3-dimensional arrays:
  //
  for (i = 0; i < (mapInfo->mapWidth + 1); i++)  {
    for (j = 0; j < (mapInfo->mapHeight + 1); j++ )  {
      free(pathFinding->whichList[i][j]);         pathFinding->whichList[i][j]    = NULL;
      free(pathFinding->parentCoords[i][j]);      pathFinding->parentCoords[i][j] = NULL;
      free(pathFinding->Gcost[i][j]);             pathFinding->Gcost[i][j]        = NULL;
      free(pathFinding->sortNumber[i][j]);        pathFinding->sortNumber[i][j]   = NULL;
    }  // End of 'j' for-loop
    free(pathFinding->whichList[i]);              pathFinding->whichList[i]       = NULL;
    free(pathFinding->parentCoords[i]);           pathFinding->parentCoords[i]    = NULL;
    free(pathFinding->Gcost[i]);                  pathFinding->Gcost[i]           = NULL;
    free(pathFinding->sortNumber[i]);             pathFinding->sortNumber[i]      = NULL;
  }  // End of 'i' for-loop
  free(pathFinding->whichList);                   pathFinding->whichList          = NULL;
  free(pathFinding->parentCoords);                pathFinding->parentCoords       = NULL;
  free(pathFinding->Gcost);                       pathFinding->Gcost              = NULL;
  free(pathFinding->sortNumber);                  pathFinding->sortNumber         = NULL;

  //
  // Now that we're done freeing memory for 3D arrays, free memory for
  // one-dimensional arrays:
  //
  free(pathFinding->openListCoords);              pathFinding->openListCoords     = NULL;
  free(pathFinding->Fcost);                       pathFinding->Fcost              = NULL;
  free(pathFinding->Hcost);                       pathFinding->Hcost              = NULL;
  free(pathFinding->openList);                    pathFinding->openList           = NULL;

}  // End of function 'freePathFindingArrays'


//-----------------------------------------------------------------------------
// Name: freeMemory_mapInfo
// Desc: Free the memory that was allocated in function 'allocateMapInfo'.
//-----------------------------------------------------------------------------
void freeMemory_mapInfo(MapInfo_t *mapInfo)  {

  //
  // Allocate memory for data structures required for each user-defined net, each pseudo-net, and the
  // Acorn-defined 'global repellent' net:
  //
  int max_routed_nets = mapInfo->numPaths + mapInfo->numPseudoPaths + 1;

  for (int i = 0; i < max_routed_nets; i++)  {
    free(mapInfo->addPseudoTraceCongestionNearVias[i]);  mapInfo->addPseudoTraceCongestionNearVias[i] = NULL;
  }

  free(mapInfo->start_cells);                       mapInfo->start_cells = NULL;
  free(mapInfo->end_cells);                         mapInfo->end_cells = NULL;
  free(mapInfo->diff_pair_terms_swapped);           mapInfo->diff_pair_terms_swapped = NULL;
  free(mapInfo->start_end_terms_swapped);           mapInfo->start_end_terms_swapped = NULL;
  free(mapInfo->swapZone);                          mapInfo->swapZone = NULL;
  free(mapInfo->diffPairStartTermPitchMicrons);     mapInfo->diffPairStartTermPitchMicrons = NULL;
  free(mapInfo->diffPairEndTermPitchMicrons);       mapInfo->diffPairEndTermPitchMicrons = NULL;
  free(mapInfo->addPseudoTraceCongestionNearVias);  mapInfo->addPseudoTraceCongestionNearVias = NULL;

}  // End of function 'freeMemory_mapInfo'

