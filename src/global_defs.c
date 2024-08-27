#include "global_defs.h"
#include "aStarLibrary.h"

//
// Define structure for recording the path number, design-rule number,
// and shape-type of paths that interact with a given cell:
//
typedef struct Path_DR_Shape_Info_t  {
  unsigned pathNum     : 10  ; // Path number of traversing path (0 - 1023)
  unsigned DR_num      :  4  ; // Design-rule set number (0 to 15)
  unsigned shapeType   :  2  ; // 0=trace, 1=viaUp, 2=viaDown
} Path_DR_Shape_Info_t;


//-----------------------------------------------------------------------------
// Name: copyCoordinates
// Desc: Copy the (x,y,z) coordinates from one variable to another, in addition
//       to the Boolean 'flag' element. Both variables must be of type
//       'Coordinate_t'.
//-----------------------------------------------------------------------------
Coordinate_t copyCoordinates(const Coordinate_t sourceCoordinate)  {

  Coordinate_t destinationCoordinate;

  // Copy the x/y/z elements from the source to the destination, in addition
  // to the Boolean 'flag' element:
  destinationCoordinate.X    = sourceCoordinate.X;
  destinationCoordinate.Y    = sourceCoordinate.Y;
  destinationCoordinate.Z    = sourceCoordinate.Z;
  destinationCoordinate.flag = sourceCoordinate.flag;

  // Return the destination coordinate to the calling routine:
  return(destinationCoordinate);

}  // End of function 'copyCoordinates'


//-----------------------------------------------------------------------------
// Name: XYZpointIsOutsideOfMap
// Desc: Check if the point 'point' is within the map. If not, return TRUE.
//       Return FALSE otherwise.
//-----------------------------------------------------------------------------
int XYZpointIsOutsideOfMap(int x, int y, int z, const MapInfo_t *mapInfo)  {

  // Check if point's coordinates are outside of the map's range:
  if (  (x < 0) || (x >= mapInfo->mapWidth)
     || (y < 0) || (y >= mapInfo->mapHeight)
     || (z < 0) || (z >= mapInfo->numLayers))  {

    return(TRUE);  // Point is outside of map, so return TRUE
  }
  else {
    return(FALSE);  // Point is not outside of map, so return FALSE
  }

}  // End of function 'XYZpointIsOutsideOfMap'


//-----------------------------------------------------------------------------
// Name: delay
// Desc: Delay execution for 'microSecs' microseconds. This function is used
//       only for debugging, and was copied from the following web page:
//       https://c-for-dummies.com/blog/?p=69.
//-----------------------------------------------------------------------------
void delay(int microSecs)  {
  long pause;
  clock_t now, then;
  pause = microSecs * (CLOCKS_PER_SEC/1000000);
  now = then = clock();
  while( (now-then) < pause )
    now = clock();
}  // End of function 'delay'

//-----------------------------------------------------------------------------
// Name: getMemory
// Desc: Measures the current (and peak) resident and virtual memory usage of
//       the linux C process, in kB. This function is used only for debugging,
//       and was copied from the following web page:
// https://stackoverflow.com/questions/1558402/memory-usage-of-current-process-in-c
//-----------------------------------------------------------------------------
void getMemory(
  int* currRealMem, int* peakRealMem,
  int* currVirtMem, int* peakVirtMem) {

  // stores each word in status file
  char buffer[1024] = "";

  // linux file contains this-process info
  FILE* file = fopen("/proc/self/status", "r");
  if (file == NULL)  {
    printf("\nWARNING: File /proc/self/status was not found by function 'getMemory', so memory footprint\n");
    printf(  "WARNING: will not be reported.\n\n");
  }
  else  {
    // Read the entire file
    while (fscanf(file, " %1023s", buffer) == 1) {

      if (strcmp(buffer, "VmRSS:") == 0) {
        if(fscanf(file, " %d", currRealMem) != 1)  {
          printf("WARNING: Function 'getMemory' could not report the current real memory.\n");
        }
      }
      if (strcmp(buffer, "VmHWM:") == 0) {
        if (fscanf(file, " %d", peakRealMem) != 1)  {
          printf("WARNING: Function 'getMemory' could not report the peak real memory.\n");
        }
      }
      if (strcmp(buffer, "VmSize:") == 0) {
        if (fscanf(file, " %d", currVirtMem) != 1)  {
          printf("WARNING: Function 'getMemory' could not report the current virtual memory.\n");
        }
      }
      if (strcmp(buffer, "VmPeak:") == 0) {
        if (fscanf(file, " %d", peakVirtMem) != 1)  {
          printf("WARNING: Function 'getMemory' could not report the peak virtual memory.\n");
        }
      }
    }  // End of while-loop
    fclose(file);
  }  // End of else-block (file != NULL)
}  // End of function 'getMemory'


//-----------------------------------------------------------------------------
// Name: printRoutabilityMetrics
// Desc: Print routability metrics to file defined by pointer 'fp' (e.g.,
//       'stdout' or a previously opened file). If there are more nets than
//       maxNets, then the crossing matrix will not be printed out.
//-----------------------------------------------------------------------------
void printRoutabilityMetrics(FILE *fp, const RoutingMetrics_t *routability, const InputValues_t *user_inputs,
                             const MapInfo_t *mapInfo, int numPaths, int maxNets)  {

  int termSwap_count = 0;  // Count of how many paths' start/end-terminals were swapped for
                           // the current iteration.
  int randomize_increase_count = 0; // Count of how many randomly selected paths will have their congestion-
                                    // related G-cost increased in the next iteration.
  int randomize_decrease_count = 0; // Count of how many randomly selected paths will have their congestion-
                                    // related G-cost decreased in the next iteration.

  fprintf(fp, "\nRoutability metrics:\n");

  // If the design contains pseudo-nets, then print out the routing metrics separately for
  // pseudo-nets and user-defined nets:
  if (user_inputs->num_pseudo_nets > 0)  {
    fprintf(fp, "  Number of cells with non-pseudo-DRC violations: %'d\n", routability->num_nonPseudo_DRC_cells);
    fprintf(fp, "  Number of cells with pseudo-DRC violations: %'d\n", routability->num_pseudo_DRC_cells);
    fprintf(fp, "  Number of cells with any type of DRC violation: %'d\n\n", routability->total_num_DRC_cells);
  }  // End of if-block for num_pseudo_nets > 0
  else  {
    fprintf(fp, "  Number of cells with DRC violations: %'d\n\n", routability->total_num_DRC_cells);
  }

  //
  // Print out the DRC matrix if it's smaller than maxNets X maxNets:
  //
  if ((numPaths <= maxNets) && (numPaths > 1))  {
    fprintf(fp, "  DRC matrix (%d nets by %d nets):\n", numPaths, numPaths);
    fprintf(fp, "     Net | ");
    for (int col_header = 0; col_header < numPaths; col_header++)  {
      if (col_header == mapInfo->numPaths)  {
        fprintf(fp, "| ");  // Print '|' to separate pseudo-nets from non-pseudo-nets
      }
      fprintf(fp, "    %2d  ", col_header);  // Print net-number as column-header
    }
    fprintf(fp, "\n");
    fprintf(fp, "    -----| ");
    for (int col_header = 0; col_header < numPaths; col_header++)  {
      if (col_header == mapInfo->numPaths)  {
        fprintf(fp, "| ");  // Print '|' to separate pseudo-nets from non-pseudo-nets
      }
      fprintf(fp, "------- ");
    }
    fprintf(fp, "\n");
    for (int row = 0; row < numPaths; row++)  {
      if (row == mapInfo->numPaths)  {
        fprintf(fp, "  Pseudo:|");

        for (int col = 0; col < numPaths; col++)  {
          if (col == mapInfo->numPaths)  {
            fprintf(fp, " |");  // Print '|' to separate pseudo-nets from non-pseudo-nets
          }
          fprintf(fp, " - - - -");  // Print horizontal line to separate pseudo-nets from non-pseudo-nets
        }
        fprintf(fp, "\n");
      }

      fprintf(fp, "      %2d |", row);
      for (int col = 0; col < numPaths; col++)  {
        if (col == mapInfo->numPaths)  {
          fprintf(fp, " |");  // Print '|' to separate pseudo-nets from non-pseudo-nets
        }
        fprintf(fp, "%7d ", routability->crossing_matrix[row][col]);
      }
      fprintf(fp, "\n");
    }
  }
  else if (numPaths == 1)  {
    fprintf(fp, "DRC matrix was not printed because it consists of only 1 net.\n");
  }
  else  {
    fprintf(fp, "DRC matrix was not printed because it's larger than %dx%d nets.\n",
            maxNets, maxNets);
  }

  fprintf(fp, "\n  Lateral length and number of cells with DRC violations for each net:\n");
  for (int i = 0; i < numPaths; i++)  {

    // Depending on whether the net is a pseudo- or non-pseudo-net, preface the net's
    // number with 'Pseudo-net' or 'Net':
    if (user_inputs->isPseudoNet[i])  {
      fprintf(fp, "     Pseudo-net ");

      // Print out the net-number, length, via-count, etc:
      fprintf(fp, "%3d: %9.4f mm, %5d vias, %'10d DRC cells, name '%s'", i,
              routability->lateral_path_lengths_mm[i], routability->num_vias[i],
              routability->path_DRC_cells[i], user_inputs->net_name[i]);


    }  // End of if-block for (isPseudoNet)
    else  {
      fprintf(fp, "            Net ");

      // Print out the net-number, length, via-count, etc:
      fprintf(fp, "%3d: %9.4f mm, %5d vias, %'10d DRC cells, name '%s'", i,
              routability->lateral_path_lengths_mm[i], routability->num_vias[i],
              routability->path_DRC_cells[i], user_inputs->net_name[i]);
    }  // End of else-block (for ! isPseudoNet)

    // Print an asterisk that denotes if the start- and end-terminals have been swapped:
    if (mapInfo->start_end_terms_swapped[i])  {
      fprintf(fp, " *");
      termSwap_count++;
    }  // End of if-block for (start_end_terms_swapped == TRUE)

    // Print a plus-sign (+) that denotes if this path was randomly selected to have
    // its congestion-related G-cost increased in the next iteration:
    if (routability->randomize_congestion[i] == INCREASE)  {
      fprintf(fp, " +");
      randomize_increase_count++;
    }  // End of if-block for randomize_congestion == INCREASE

    // Print a minus-sign (-) that denotes if this path was randomly selected to have
    // its congestion-related G-cost reduced in the next iteration:
    if (routability->randomize_congestion[i] == DECREASE)  {
      fprintf(fp, " -");
      randomize_decrease_count++;
    }  // End of if-block for randomize_congestion == DECREASE

    if (user_inputs->isDiffPair[i])  {
      fprintf(fp, " (partner: '%s'", user_inputs->net_name[user_inputs->diffPairPartner[i]]);

      // If path has terminals in a swap-zone, then report the number of the swap-zone:
      if (mapInfo->swapZone[i])  {
        fprintf(fp, ", in swap-zone #%d", mapInfo->swapZone[i]);
      }

      // If path has P/N-swappable terminals, then report this. Also report whether the
      // terminals have been swapped:
      else if (user_inputs->isPNswappable[i])  {
        if (mapInfo->diff_pair_terms_swapped[i])  {
          fprintf(fp, ", P/N terminals swapped");
        }
        else  {
          fprintf(fp, ", swappable P/N terminals");
        }
      }  // End of else/if/else-block for 'isPNswappable'

      fprintf(fp, ")");  // Closing parenthesis for diff-pair information
    }  // End of if-block for 'isDiffPair'

    fprintf(fp, "\n");  // Closing carriage return for this net's info

  }  // End of for-loop for index 'i'

  fprintf(fp,   "                     -------------  ----------   -----------------------------------\n");

  // If the design contains pseudo-nets, then print out the routing metrics separately for
  // pseudo-nets and user-defined nets:
  if (user_inputs->num_pseudo_nets > 0)  {
    fprintf(fp,   "  User-defined nets: %9.4f mm, %'5d vias, %'10d cells with DRCs (%'d / 2)\n",
            routability->total_lateral_nonPseudo_length_mm, routability->total_nonPseudo_vias,
            routability->num_nonPseudo_DRC_cells, routability->num_nonPseudo_DRC_cells * 2  );
    fprintf(fp,   "        Pseudo-nets: %9.4f mm, %'5d vias, %'10d cells with DRCs (%'d / 2)\n",
            routability->total_lateral_pseudo_length_mm, routability->total_pseudo_vias,
            routability->num_pseudo_DRC_cells, routability->num_pseudo_DRC_cells * 2  );
  }  // End of if-block for num_pseudo_nets > 0

  fprintf(fp,   "           All nets: %9.4f mm, %'5d vias, %'10d cells with DRCs (%'d / 2)\n\n",
          routability->total_lateral_length_mm, routability->total_vias,
          routability->total_num_DRC_cells, routability->total_num_DRC_cells * 2);

  // If any net's terminals were swapped for this iteration, then include a footnote that
  // tells reader that nets with asterisks had their start/end-terminals swapped:
  if (termSwap_count > 0)  {
    fprintf(fp,   "  * denotes the %d net(s) for which the start- and end-terminals were swapped from the original terminals.\n", termSwap_count);
  }

  // If any nets were randomly chosen to have their congestion-related G-cost modified in the
  // next iteration, then include a footnote that tells reader that nets with plus-signs will be
  // treated differently in the next iteration:
  if (randomize_increase_count > 0)  {
    fprintf(fp,   "  + denotes the %d net(s) randomly chosen to have their congestion-related G-cost increased in the next iteration.\n", randomize_increase_count);
  }
  if (randomize_decrease_count > 0)  {
    fprintf(fp,   "  - denotes the %d net(s) randomly chosen to have their congestion-related G-cost decreased in the next iteration.\n", randomize_decrease_count);
  }

}  // End of function 'printRoutabilityMetrics'


//-----------------------------------------------------------------------------
// Name: add_HTML_message
// Desc: Add an HTML-encoded text string 'text_string'  to the array
//       routability->HTML_message_strings[], and add the integer 'iteration'
//       to array routability->HTML_message_iter_nums[]. Add category number
//       'category_num' to the array routability->HTML_message_categories. Also,
//       increment the number of HTML messages, routability->num_HTML_messages.
//-----------------------------------------------------------------------------
void add_HTML_message(char* HTML_message, short int iteration, unsigned char category_num,
                      RoutingMetrics_t *routability)  {

  // printf("\nDEBUG: Entered function add_HTML_message in iteration %d with message '%s'\n", iteration, HTML_message);

  // Extend the memory allocated to 'HTML_message_strings' array to accommodate one more message:
  routability->HTML_message_strings = realloc(routability->HTML_message_strings, (routability->num_HTML_messages + 1)
                                                                              * sizeof(routability->HTML_message_strings));
  if (routability->HTML_message_strings == 0)  {
    printf("\nERROR: Unable to re-allocate memory for %d elements of 'routability->HTML_message_strings' in function 'add_HTML_message'.\n",
           routability->num_HTML_messages + 1);
    exit(1);
  }

  // Extend the memory allocated to 'HTML_message_iter_nums' array to accommodate one more message:
  routability->HTML_message_iter_nums = realloc(routability->HTML_message_iter_nums, (routability->num_HTML_messages + 1)
                                                                              * sizeof(short int));
  if (routability->HTML_message_iter_nums == 0)  {
    printf("\nERROR: Unable to re-allocate memory for %d elements of 'routability->HTML_message_iter_nums' in function 'add_HTML_message'.\n",
           routability->num_HTML_messages + 1);
    exit(1);
  }

  // Extend the memory allocated to 'HTML_message_categories' array to accommodate one more message:
  routability->HTML_message_categories = realloc(routability->HTML_message_categories, (routability->num_HTML_messages + 1)
                                                                               * sizeof(routability->HTML_message_categories));
  if (routability->HTML_message_categories == 0)  {
    printf("\nERROR: Unable to re-allocate memory for %d elements of 'routability->HTML_message_categories' in function 'add_HTML_message'.\n",
           routability->num_HTML_messages + 1);
    exit(1);
  }


  // Get the length of the message string so we can allocate the appropriate amount of memory for its length. Note that
  // 'message_length' does not include the the extra byte needed for the terminating NULL character.
  int message_length = strlen(HTML_message);
  // printf("\nDEBUG: In function add_HTML_message, message_length = %d (excluding NULL character).\n\n", message_length);
  routability->HTML_message_strings[routability->num_HTML_messages] = malloc((message_length + 1) * sizeof(char));
  if (routability->HTML_message_strings[routability->num_HTML_messages] == 0)  {
    printf("\nERROR: Unable to re-allocate memory for %d bytes of 'routability->HTML_message_strings[%d]' in function 'add_HTML_message'.\n",
           message_length + 1, routability->num_HTML_messages);
    exit(1);
  }

  // Copy the text string into the 'routability' structure:
  strncpy(routability->HTML_message_strings[routability->num_HTML_messages], HTML_message, message_length + 1);
  // printf("\nDEBUG: In function add_HTML_message, routability->HTML_message_strings[%d] = '%s'\n\n",
  //         routability->num_HTML_messages, routability->HTML_message_strings[routability->num_HTML_messages]);

  // Capture the iteration number that this message is associated with:
  routability->HTML_message_iter_nums[routability->num_HTML_messages] = iteration;
  // printf("\nDEBUG: In function add_HTML_message, routability->HTML_message_iter_nums[%d] = '%d'\n\n",
  //        routability->num_HTML_messages, routability->HTML_message_iter_nums[routability->num_HTML_messages]);

  // Capture the category number that applies to this message:
  routability->HTML_message_categories[routability->num_HTML_messages] = category_num;


  // Increment the number of HTML messages:
  routability->num_HTML_messages++;

  // printf("\nDEBUG: In function add_HTML_message, num_HTML_messages was incremented to %d\n\n", routability->num_HTML_messages);


}  // End of function 'add_HTML_message'


//-----------------------------------------------------------------------------
// Name: get_unwalkable_barrier_proximity_by_path
// Desc: Reads the 'forbiddenProximityBarrier' element of the 3D cellInfo
//       matrix at location (x,y,z). This function returns whether this cell is
//       unwalkable due to proximity to a nearby, user-defined obstacle/barrier.
//       Whether the cell is unwalkable depends on the design-rule subset
//       and the shape type ('shape_type'). The design-rule subset is
//       calculated in this function based on the path number ('path_num').
//
//       This function assumes that (x,y,z) is a valid coordinate within the map.
//-----------------------------------------------------------------------------
int get_unwalkable_barrier_proximity_by_path(CellInfo_t ***const  cellInfo, const InputValues_t *user_inputs,
                                             const int x, const int y, const int z, const int path_num,
                                             const int shape_type)  {

  // printf("DEBUG: Entered 'get_unwalkable_barrier_proximity_by_path' at (%d,%d,%d). path_num = %d\n",
  //       x, y, z, path_num);

  // Initialize the result to zero:
  int result = 0;

  // Get the design-rule number for this (x,y,z) location:
  int DR_num = cellInfo[x][y][z].designRuleSet;

  // printf("DEBUG: In function 'get_unwalkable_barrier_proximity_by_path', DR_num was calculated to be %d.\n", DR_num);

  // Get the design-rule subset number for this path number and design-rule set:
  int DR_subset = user_inputs->designRuleSubsetMap[path_num][DR_num];

  // printf("DEBUG: Entered 'get_unwalkable_barrier_proximity_by_path' with path #%d at (%d,%d,%d). DR = %d, DR_subset = %d\n",
  //         path_num, x, y, z, DR_num, DR_subset);

  // Calculate bit-offset from base address, based on the design-rule subset
  // and the shape type. The largest possible value of offset is 47, because
  // the maximum DR_subset is 15, and the maximum shape_type is 2.
  int offset = DR_subset * NUM_SHAPE_TYPES   +   shape_type;

  // Initialize 'mask' as 64-bit integer with binary '1' in the right-most position, i.e.:
  // binary '0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001'.
  // Then shift the '1' to the left by 'offset' bits:
  long unsigned int mask = 1 << offset;

  // Calculate the result by logically AND'ing the mask with with 'forbiddenProximityBarrier'.
  // The result will be non-zero only if the corresponding bit is set in
  // 'forbiddenProximityBarrier'.
  result = mask  &  cellInfo[x][y][z].forbiddenProximityBarrier;

  // printf("DEBUG: Returning value %d from function 'get_unwalkable_barrier_proximity_by_path'.\n", result);

  // Return the result to the calling routine:
  return(result);

}  // End of function 'get_unwalkable_barrier_proximity_by_path'


//-----------------------------------------------------------------------------
// Name: pointIsOutsideOfMap
// Desc: Check if the point 'point' is within the map. If not, return TRUE.
//       Return FALSE otherwise.
//-----------------------------------------------------------------------------
int pointIsOutsideOfMap(const Coordinate_t point, const MapInfo_t *mapInfo)  {

  // Check if point's coordinates are outside of the map's range:
  if (  (point.X < 0) || (point.X >= mapInfo->mapWidth)
     || (point.Y < 0) || (point.Y >= mapInfo->mapHeight)
     || (point.Z < 0) || (point.Z >= mapInfo->numLayers))  {

    return(TRUE);  // Point is outside of map, so return TRUE
  }
  else {
    return(FALSE);  // Point is not outside of map, so return FALSE
  }

}  // End of function 'pointIsOutsideOfMap'


//-----------------------------------------------------------------------------
// Name: XY_coords_are_outside_of_map
// Desc: Check of the (x,y) coordinate is within the map. If not, return TRUE.
//       Return FALSE otherwise.
//-----------------------------------------------------------------------------
int XY_coords_are_outside_of_map(const int x, const int y, const MapInfo_t *mapInfo)  {

  // Check if X/Y coordinates are outside of the map's range:
  if (  (x < 0) || (x >= mapInfo->mapWidth)
     || (y < 0) || (y >= mapInfo->mapHeight))  {

    return(TRUE);  // Point is outside of map, so return TRUE
  }
  else {
    return(FALSE);  // Point is not outside of map, so return FALSE
  }

}  // End of function 'XY_coords_are_outside_of_map'


//-----------------------------------------------------------------------------
// Name: assignCongestionByPathIndex
// Desc: Assign the pathTraversalsTimes100 value to the 'cellInfo' cell for path
//       index 'pathIndex'. If congestion_value exceeds 2^24-1, or 16,777,215
//       (aka 'maxCongestion), then re-define congestion value to this value,
//       which is the largest value that can fit in the 24-bit field.
//-----------------------------------------------------------------------------
void assignCongestionByPathIndex(CellInfo_t *cellInfo, int pathIndex, unsigned congestion_value)  {

  // Issue warning if congestion exceeds maxCongestion:
  if (congestion_value > maxCongestion) {
    printf("WARNING: Congestion (%'d) exceeded maximum allowed value (%'d) at an x/y/z location. Value will be replaced with %'d.\n",
            congestion_value, maxCongestion, maxCongestion);

    // 'congestion_value' exceeds allowed value, so assign the value 'maxCongestion':
    cellInfo->congestion[pathIndex].pathTraversalsTimes100 = maxCongestion;
  }
  else  {
    // Assign the new congestion value:
    cellInfo->congestion[pathIndex].pathTraversalsTimes100 = congestion_value;
  }  // End of if/else-block

}  // End of function 'assignCongestionByPathIndex'


//-----------------------------------------------------------------------------
// Name: get_unwalkable_pinSwap_proximity_by_path
// Desc: Reads the 'forbiddenProximityPinSwap' element of the 3D cellInfo
//       matrix at location (x,y,z). This function returns whether this cell is
//       unwalkable due to proximity to a nearby, user-defined pin-swap zone.
//       Whether the cell is unwalkable depends on the design-rule subset
//       and the shape type ('shape_type'). The design-rule subset is
//       calculated in this function based on the path number ('path_num').
//
//       This function assumes that (x,y,z) is a valid coordinate within the map.
//-----------------------------------------------------------------------------
int get_unwalkable_pinSwap_proximity_by_path(CellInfo_t ***const  cellInfo, const InputValues_t *user_inputs,
                                             const int x, const int y, const int z, const int path_num,
                                             const int shape_type)  {

  // printf("DEBUG: Entered 'get_unwalkable_pinSwap_proximity_by_path' at (%d,%d,%d). path_num = %d\n",
  //       x, y, z, path_num);

  // Initialize the result to zero:
  int result = 0;

  // Get the design-rule number for this (x,y,z) location:
  int DR_num = cellInfo[x][y][z].designRuleSet;

  // printf("DEBUG: In function 'get_unwalkable_pinSwap_proximity_by_path', DR_num was calculated to be %d.\n", DR_num);

  // Get the design-rule subset number for this path number and design-rule set:
  int DR_subset = user_inputs->designRuleSubsetMap[path_num][DR_num];

  // printf("DEBUG: Entered 'get_unwalkable_pinSwap_proximity_by_path' with path #%d at (%d,%d,%d). DR = %d, DR_subset = %d\n",
  //         path_num, x, y, z, DR_num, DR_subset);

  // Calculate bit-offset from base address, based on the design-rule subset
  // and the shape type. The largest possible value of offset is 47, because
  // the maximum DR_subset is 15, and the maximum shape_type is 2.
  int offset = DR_subset * NUM_SHAPE_TYPES   +   shape_type;

  // Initialize 'mask' as 64-bit integer with binary '1' in the right-most position, i.e.:
  // binary '0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001'.
  // Then shift the '1' to the left by 'offset' bits:
  long unsigned int mask = 1 << offset;

  // Calculate the result by logically AND'ing the mask with with 'forbiddenProximityPinSwap'.
  // The result will be non-zero only if the corresponding bit is set in
  // 'forbiddenProximityBarrier'.
  result = mask  &  cellInfo[x][y][z].forbiddenProximityPinSwap;

  // printf("DEBUG: Returning value %d from function 'get_unwalkable_pinSwap_proximity_by_path'.\n", result);

  // Return the result to the calling routine:
  return(result);

}  // End of function 'get_unwalkable_pinSwap_proximity_by_path'


//-----------------------------------------------------------------------------
// Name: calc_2D_Pythagorean_distance_ints
// Desc: Calculate the the distance between (x1, y1) and (x2, y2) using the
//       Pythagorean formula. This function does not account for the separation
//       in the z-dimension, and is used for calculating the distance between
//       integer-based coordinates.
//-----------------------------------------------------------------------------
float calc_2D_Pythagorean_distance_ints(const int x1, const int y1, const int x2, const int y2)  {

  return sqrt( (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) );

}  // End of function 'calc_2D_Pythagorean_distance_ints'


//-----------------------------------------------------------------------------
// Name: findCloserVia
// Desc: Determine whether the current via that ends at 'current_end_via' is
//       closer to (x,y) than the via at segment 'end_via', which is located
//       a distance 'closest_distance' from (x,y). If so, then update
//       the values of 'start_via', 'end_via', and 'closest_distance' with the
//       values from 'current_start_via' and 'current_end_via' (respectively)
//       and the new 'closest_distance' value.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_findCloserVia' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_findCloserVia 1
#undef DEBUG_findCloserVia

void findCloserVia(const int current_start_via, const int current_end_via, int const pathNum, const int num_vias,
                   int *start_via, int *end_via, float *closest_distance, const int x, const int y,
                   Coordinate_t *pathCoords[], const MapInfo_t *mapInfo)  {

  #ifdef DEBUG_findCloserVia
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  int thread_num = omp_get_thread_num();
  if (mapInfo->current_iteration == 101)  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in findCloserVia() because specific requirements were met.\n\n", thread_num);
    DEBUG_ON = TRUE;

    printf("\nDEBUG: (thread %2d) Entered function findCloserVia with following inputs:\n", thread_num);
    printf(  "DEBUG: (thread %2d)            pathNum = %d\n", thread_num, pathNum);
    printf(  "DEBUG: (thread %2d)  current_start_via = %d\n", thread_num, current_start_via);
    printf(  "DEBUG: (thread %2d)    current_end_via = %d\n", thread_num, current_end_via);
    printf(  "DEBUG: (thread %2d)          start_via = %d\n", thread_num, *start_via);
    printf(  "DEBUG: (thread %2d)            end_via = %d\n", thread_num, *end_via);
    printf(  "DEBUG: (thread %2d)           num_vias = %d\n", thread_num, num_vias);
    printf(  "DEBUG: (thread %2d)           (x,y) = (%d,%d)\n", thread_num, x, y);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Because some layer-transitions do not contain vertical stacks of segments, we first calculate
  // the mid-point between the top and bottom of the via-stack:
  int X_mid = 0, Y_mid = 0;
  if (current_start_via == -1)  {
    // Start-via is the starting-terminal if 'start_via' is -1:
    X_mid = (mapInfo->start_cells[pathNum].X  +  pathCoords[pathNum][current_end_via].X) / 2;
    Y_mid = (mapInfo->start_cells[pathNum].Y  +  pathCoords[pathNum][current_end_via].Y) / 2;
  }
  else {
    // If start_via > -1, then the via does not start at the start-terminal:
    X_mid = (pathCoords[pathNum][current_start_via].X  +  pathCoords[pathNum][current_end_via].X) / 2;
    Y_mid = (pathCoords[pathNum][current_start_via].Y  +  pathCoords[pathNum][current_end_via].Y) / 2;
  }

  // Calculate distance between (x,y) and current via:
  float distance = calc_2D_Pythagorean_distance_ints(x, y, X_mid, Y_mid);

  // Depending on whether this is the first via or a subsequent via, determine whether it's
  // the closest via (thus far) to point (x,y):
  if (num_vias == 1)  {
    // If this is the first via that matches the startLayer and endLayer criteria, then
    // define this via's distance to (x,y) as the 'closest_distance', and also save the
    // start- and end-segments of the via:
    *closest_distance = distance;
    *start_via = current_start_via;
    *end_via   = current_end_via;

    #ifdef DEBUG_findCloserVia
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In findCloserVia, via at end of path was via #%d. current_start_via = %d, current_end_via = %d, start_via = %d, end_via = %d\n",
              thread_num, num_vias, current_start_via, current_end_via, *start_via, *end_via);
    }
    #endif

  }  // End of if-block for num_vias == 1
  else  {
    // We got here, so other vias have previously been found that match the startLayer
    // and endLayer criteria. Check if the current via is closer to (x,y). If so,
    // then save its start/end-segments and redefine 'closest_distance':
    if (distance < *closest_distance)  {
      *closest_distance = distance;
      *start_via = current_start_via;
      *end_via   = current_end_via;

      #ifdef DEBUG_findCloserVia
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) In findCloserVia, via at end of path was via #%d. current_start_via = %d, current_end_via = %d, start_via = %d, end_via = %d\n",
               thread_num, num_vias, current_start_via, current_end_via, *start_via, *end_via);
      }
      #endif

    }  // End of if-block for distance < closest_distance
  }  // End of else-block for num_vias > 1

}  // End of function 'findCloserVia'


//-----------------------------------------------------------------------------
// Name: findNearbyLayerTransition
// Desc: Locate layer-transitions (or 'vias') in path pathNum that start on
//       routing layer 'startLayer' and end on layer 'endLayer'. If more than
//       one layer-transition satisfies these requirements, then return the
//       one that is closest to coordinates (x,y). The function returns the
//       start- and end-segments of the via in the non-contiguous path array.
//       If the via begins at the start-terminal, then '-1' is returned for
//       the via's starting segment.
//
//       If pathNum is not a diff-pair path, then this function searches for
//       vertically stacked via-segments. (Pseudo-paths are an example.) If
//       pathNum is a diff-pair path, then the 'via' is any layer-transition,
//       vertically stacked or otherwise.
//
//       If 'enforceStartLayerOnly' is TRUE, then the function:
//         a) does not enforce the criterion for the end-layer, but
//         b) populates the 'end_via' output with the segment number
//            on layer 'endLayer' closest to the via-stack.
//       If 'enforceEndLayerOnly' is TRUE, then the function:
//         a) does not enforce the criterion for the start-layer, but
//         b) populates the 'start_via' output with the segment number
//            on layer 'startLayer' closest to the via-stack.
//
//       The parameters 'enforceStartLayerOnly' and 'enforceEndLayerOnly'
//       must not both be TRUE.
//
//       If no via satisfies the start-/end-layer constraints, then the
//       function returns -1 for the 'startLayer' and 'endLayer' values,
//       and sets the 'error' flag to TRUE.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_findNearbyLayerTransition' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_findNearbyLayerTransition 1
#undef DEBUG_findNearbyLayerTransition

ViaStack_t findNearbyLayerTransition(const int pathNum, int pathLengths[], Coordinate_t *pathCoords[],
                                     const int startLayer, const int endLayer, const int x, const int y,
                                     const int enforceStartLayerOnly, const int enforceEndLayerOnly,
                                     const MapInfo_t *mapInfo, const InputValues_t *user_inputs)  {

  // Determine whether pathNum is a diff-pair path. 'isDiffPair' is a Boolean variable that's TRUE
  // only if pathNum is a diff-pair path:
  unsigned char isDiffPair = user_inputs->isDiffPair[pathNum];

  #ifdef DEBUG_findNearbyLayerTransition
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  int thread_num = omp_get_thread_num();
  if ((mapInfo->current_iteration == 3) && (pathNum == 10))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in findNearbyLayerTransition() because specific requirements were met.\n\n", thread_num);
    DEBUG_ON = TRUE;

    printf("\nDEBUG: (thread %2d) Entered function findNearbyLayerTransition with following inputs:\n", thread_num);
    printf(  "DEBUG: (thread %2d)         pathNum = %d\n", thread_num, pathNum);
    printf(  "DEBUG: (thread %2d)      isDiffPair = %d\n", thread_num, isDiffPair);
    printf(  "DEBUG: (thread %2d)      startLayer = %d    (enforceStartLayerOnly = %d)\n", thread_num, startLayer, enforceStartLayerOnly);
    printf(  "DEBUG: (thread %2d)        endLayer = %d    (enforceEndLayerOnly = %d)\n", thread_num, endLayer, enforceEndLayerOnly);
    printf(  "DEBUG: (thread %2d)           (x,y) = (%d,%d)\n", thread_num, x, y);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Confirm that this function was not called with both enforceStartLayerOnly
  // and enforceEndLayerOnly set to TRUE:
  if (enforceStartLayerOnly && enforceEndLayerOnly)  {
    printf("\nERROR: Function findNearbyLayerTransition was called with a set of illegal conditions:\n");
    printf(  "       Input variables 'enforceStartLayerOnly' and 'enforceEndLayerOnly' were both set\n");
    printf(  "       to true. This is an unexpected situation. Please inform the software developer\n");
    printf(  "       of this fatal error message.\n\n");
    exit(1);
  }

  // Define variable that contains the start- and end-segments of the via,
  // as well as the path number.
  ViaStack_t via_stack;

  // As a safety precaution, initialize the coordinates of the via-stack's start- and end-segments
  // to the maximum values allowed in a Coordinate_t structure (2^13-1, 2^13-1, 2^5-1):
  via_stack.startCoord.X = 8191;
  via_stack.startCoord.Y = 8191;
  via_stack.startCoord.Z = 31;
  via_stack.endCoord.X   = 8191;
  via_stack.endCoord.Y   = 8191;
  via_stack.endCoord.Z   = 31;

  // As a safety precaution, initialize the start- and end-segments of the
  // via-stack to non-sensical (negative) values:
  via_stack.startSegment = -99;
  via_stack.endSegment   = -99;

  // Define variables to hold the start- and end-segments of the closest qualifying via:
  int start_via = -99;
  int end_via   = -99;

  // Define temporary variables to hold the start- and end-segments of the most recent via:
  int current_start_via = -99;
  int current_end_via   = -99;

  // Define temporary variables to hold the X- and Y-coordinates of the start-segment
  // of the most recent via:
  int current_start_via_X = -99;
  int current_start_via_Y = -99;

  // Copy the input path-number to the output in order to satisfy the 'ViaStack_t'
  // requirement to include the path-number:
  via_stack.pathNum = pathNum;

  // Define variable to hold the previous segment. Initialize this
  // variable as the start-terminal:
  Coordinate_t prev_segment = copyCoordinates(mapInfo->start_cells[pathNum]);

  // Define a counter for the number of vias that satisfy the criteria for
  // startLayer and endLayer:
  int num_vias = 0;

  // Define Boolean flags that are set when we find a via-stack that started on
  // layer 'startLayer' and 'endLayer':
  unsigned char correct_startLayer_found = FALSE;
  unsigned char correct_endLayer_found   = FALSE;

  // Define the distance between point (x,y) and the closest qualifying via:
  float closest_distance = 0.0;

  // Check whether the transition from the start-terminal to the first segment is a possible
  // via that satisfies the layer-criteria:
  if (   (prev_segment.Z != pathCoords[pathNum][0].Z)
      && (isDiffPair || ((prev_segment.X == pathCoords[pathNum][0].X) && (prev_segment.Y == pathCoords[pathNum][0].Y)))
      && ((prev_segment.Z == startLayer) || (enforceEndLayerOnly)))  {
    // The start-terminal is on a different layer and the start-terminal is on layer 'startLayer',
    // so set 'correct_startLayer_found' to TRUE:
    current_start_via = -1;
    correct_startLayer_found = TRUE;
    current_start_via_X = prev_segment.X;
    current_start_via_Y = prev_segment.Y;

    #ifdef DEBUG_findNearbyLayerTransition
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Before iterating over path segments, setting current_start_via to -1 because terminal is on start-layer layer.\n", thread_num);
    }
    #endif

  }  // End of if-block for terminal-layer != layer of first segment


  //
  // Iterate over the length of the path:
  //
  for (int i = 0; i < pathLengths[pathNum]; i++)  {

    #ifdef DEBUG_findNearbyLayerTransition
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In findNearbyLayerTransition, checking for via at segment %d of path %d at (%d,%d,%d). Previous segment was at (%d,%d,%d)...\n",
             thread_num, i, pathNum, pathCoords[pathNum][i].X, pathCoords[pathNum][i].Y, pathCoords[pathNum][i].Z,
             prev_segment.X, prev_segment.Y, prev_segment.Z);
    }
    #endif

    // Define Boolean values that will be used to determine if the status of our search for via-stacks:
    unsigned char same_Z_as_previous;   // TRUE if current segment is on same routing layer as the previous segment.
    unsigned char same_Z_as_next;       // TRUE if current segment is on same routing layer as the next segment.
    unsigned char same_XY_as_previous;  // TRUE if current segment has same X/Y coordinates as previous segment.
    unsigned char same_XY_as_next;      // TRUE if current segment has same X/Y coordinates as next segment.
    unsigned char Z_matches_startLayer; // TRUE if the current segment's routing layer matches the startLayer input parameter.
    unsigned char Z_matches_endLayer;   // TRUE if the current segment's routing layer matches the endLayer input parameter.

    // Determine whether current segment in on same routing layer as previous segment:
    if (pathCoords[pathNum][i].Z == prev_segment.Z)  {
      same_Z_as_previous = TRUE;
    }
    else  {
      same_Z_as_previous = FALSE;
    }

    // Determine whether current segment has same X/Y coordinates as previous segment:
    if ((pathCoords[pathNum][i].X == prev_segment.X) && (pathCoords[pathNum][i].Y == prev_segment.Y))  {
      same_XY_as_previous = TRUE;
    }
    else  {
      same_XY_as_previous = FALSE;
    }

    // Determine whether current segment in on same routing layer as next segment:
    if ((i < pathLengths[pathNum] - 1) && (pathCoords[pathNum][i].Z != pathCoords[pathNum][i + 1].Z))  {
      same_Z_as_next = FALSE;
    }
    else  {
      // We got here, so segment is either the end-terminal, or else the next segment is
      // on the same layer as the current segment.
      same_Z_as_next = TRUE;
    }

    // Determine whether current segment has the same X/Y coordinates as next segment:
    if (   (i < pathLengths[pathNum] - 1)
        && (pathCoords[pathNum][i].X == pathCoords[pathNum][i + 1].X)
        && (pathCoords[pathNum][i].Y == pathCoords[pathNum][i + 1].Y))  {
      same_XY_as_next = TRUE;
    }
    else  {
      // We got here, so segment is either the end-terminal, or else the next segment is
      // has different X/Y coordinates from the current segment.
      same_XY_as_next = FALSE;
    }

    // Determine whether the current segment's routing layer matches the startLayer parameter:
    if (pathCoords[pathNum][i].Z == startLayer)  {
      Z_matches_startLayer = TRUE;
    }
    else  {
      Z_matches_startLayer = FALSE;
    }

    // Determine whether the current segment's routing layer matches the endLayer parameter:
    if (pathCoords[pathNum][i].Z == endLayer)  {
      Z_matches_endLayer = TRUE;
    }
    else  {
      Z_matches_endLayer = FALSE;
    }

    #ifdef DEBUG_findNearbyLayerTransition
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d)   correct_startLayer_found = %d, Z_matches_startLayer = %d, Z_matches_endLayer = %d\n",
              thread_num, correct_startLayer_found, Z_matches_startLayer, Z_matches_endLayer);
      printf("DEBUG: (thread %2d)   same_Z_as_previous = %d, same_Z_as_next = %d, same_XY_as_previous = %d, same_XY_as_next = %d\n",
              thread_num, same_Z_as_previous, same_Z_as_next, same_XY_as_previous, same_XY_as_next);
    }
    #endif


    //
    // Set the 'current_start_via' to the current segment if the following criteria are satisfied:
    //  (a) [(Z_matches_startLayer is TRUE) OR (enforceEndLayerOnly is TRUE)], and
    //  (b) ((isDiffPair is TRUE) OR (same_Z_as_previous is TRUE)) AND
    //  (c) (same_Z_as_next is FALSE)
    //  (d) AND ((isDiffPair is TRUE) OR ((same_XY_as_previous is FALSE) AND (same_XY_as_next is TRUE))
    if (   (Z_matches_startLayer || enforceEndLayerOnly)                  // Item (a) above
        && (isDiffPair || same_Z_as_previous)                             // Item (b) above
        && (! same_Z_as_next)                                             // Item (c) above
        && (isDiffPair || ((! same_XY_as_previous) && same_XY_as_next)))  // Item (d) above
    {
      // We got here, so we're at the beginning of a via-stack that satisfies the input
      // criteria. Set the 'current_start_via' to the current segment, and then
      // 'correct_startLayer_found' to TRUE:
      current_start_via = i;
      correct_startLayer_found = TRUE;
      current_start_via_X = pathCoords[pathNum][i].X;
      current_start_via_Y = pathCoords[pathNum][i].Y;


      #ifdef DEBUG_findNearbyLayerTransition
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)     Found start of via-stack with correct start-layer (%d)\n", thread_num, startLayer);
      }
      #endif
    }  // End of if-block for detecting start of via-stack


    //
    // Set the 'current_end_via' to the current segment if the following criteria are satisfied:
    //  (a) Correct_startLayer_found is TRUE, and
    //  (b) Current Z-value does not match the Z-value of the start of the via-stack, and
    //  (c) [(Z_matches_endLayer is TRUE) OR (enforceStartLayerOnly is TRUE)], and
    //  (d) (isDiffPair is TRUE) OR (same_Z_as_next is TRUE), and
    //  (e) same_Z_as_previous is FALSE
    //  (f) (isDiffPair is TRUE) OR ((same_XY_as_previous is TRUE) AND (same_XY_as_next is FALSE))
    //  (g) (x,y) coordinates match start-via's (x,y), or isDiffPair is TRUE
    if (   correct_startLayer_found                                                                                   // Item (a) above
        && (   ((current_start_via >= 0)  && (pathCoords[pathNum][i].Z != pathCoords[pathNum][current_start_via].Z))  // Item (b) above
            || ((current_start_via == -1) && (pathCoords[pathNum][i].Z != mapInfo->start_cells[pathNum].Z)))          // Item (b) above, continued
        && (Z_matches_endLayer || enforceStartLayerOnly)                                                              // Item (c) above
        && (isDiffPair || same_Z_as_next)                                        // Item (d) above
        && (! same_Z_as_previous)                                                // Item (e) above
        && (isDiffPair || ((same_XY_as_previous) && (! same_XY_as_next)))        // Item (f) above
        && (    isDiffPair                                               // Item (g) above
            || (   (pathCoords[pathNum][i].X == current_start_via_X)     // Item (g) above, continued
                && (pathCoords[pathNum][i].Y == current_start_via_Y))))  // Item (g) above, continued
    {
      // We got here, so we're at the end of a via-stack that satisfies the input
      // criteria. Set the 'current_end_via' to the current segment, and then set
      // 'correct_endLayer_found' to TRUE:
      current_end_via = i;
      correct_endLayer_found = TRUE;

      #ifdef DEBUG_findNearbyLayerTransition
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)     Found end of via-stack with correct end-layer (%d)\n", thread_num, endLayer);
      }
      #endif
    }  // End of if-block for detecting end of via-stack

    // If a via-stack was found with the correct start- and end-layers, then
    // increment the number of vias found:
    if (correct_startLayer_found && correct_endLayer_found)  {
      // Increment the counter for the number of vias that meet the criteria:
      num_vias++;

      // Clear the flags for 'correct_startLayer_found' and 'correct_endLayer_found':
      correct_startLayer_found = FALSE;
      correct_endLayer_found   = FALSE;

      // Determine whether the current via (or layer-transition) is closer to (x,y) than
      // any other via/transition that has been found.
      findCloserVia(current_start_via, current_end_via, pathNum, num_vias, &start_via, &end_via, &closest_distance, x, y, pathCoords, mapInfo);

      // Clear the temporary variables for storing the current via's start/end segments and coordinates:
      current_start_via   = -99;
      current_end_via     = -99;
      current_start_via_X = -99;
      current_start_via_Y = -99;

    }  // End of if-block for correct_startLayer_found && correct_endLayer_found

    // Copy the current segment's coordinates into 'prev_segment' for the
    // next time through this loop:
    prev_segment = copyCoordinates(pathCoords[pathNum][i]);

  }  // End of for-loop for index 'i' (0 to pathLength)


  //
  // Populate elements of the structure that will be returned to the calling function:
  //
  if (num_vias > 0)  {
    via_stack.startSegment = start_via;
    via_stack.endSegment   = end_via;
    via_stack.endCoord     = copyCoordinates(pathCoords[pathNum][end_via]);
    via_stack.error        = FALSE;

    // The 'startCoord' element and the 'endShapeType' element are calculated below, and
    // depend on whether the start_via segment is/isn't at the path's start-terminal:
    via_stack.endShapeType = TRACE;  // Initialize to TRACE, but change to via-type below.
    if (start_via >= 0)  {
      // We got here, so the via does not begin at the path's start-terminal:
      via_stack.startCoord = copyCoordinates(pathCoords[pathNum][start_via]);

      // Determine shape-type of end-via segment:
      if (pathCoords[pathNum][end_via].Z > pathCoords[pathNum][start_via].Z)  {
        via_stack.endShapeType = VIA_DOWN;
      }
      else  {
        via_stack.endShapeType = VIA_UP;
      }
    }
    else  {
      // We got here, so the via starts at the start-terminal:
      via_stack.startCoord = copyCoordinates(mapInfo->start_cells[pathNum]);

      // Determine shape-type of end-via segment:
      if (pathCoords[pathNum][end_via].Z > mapInfo->start_cells[pathNum].Z)  {
        via_stack.endShapeType = VIA_DOWN;
      }
      else  {
        via_stack.endShapeType = VIA_UP;
      }
    }  // End of if/else-block for determining value of endShapeType
  }  // End of if-block for num_vias > 0
  else  {
    // We got here, so **no** vias were found that satisfy the criteria.
    via_stack.error = TRUE;
    via_stack.endShapeType = TRACE;
    via_stack.startSegment = -1;
    via_stack.endSegment   = -1;
    via_stack.startCoord.X = 0;
    via_stack.startCoord.Y = 0;
    via_stack.startCoord.Z = 0;
    via_stack.endCoord.X   = 0;
    via_stack.endCoord.Y   = 0;
    via_stack.endCoord.Z   = 0;
  }  // End of else-block for num_vias == 0

  // As a safety-check, confirm that via_stack elements don't contain junk data unless
  // the 'error' flag has been set to TRUE:
  if (   (via_stack.error == FALSE)
      &&  (   (via_stack.startCoord.X == 8191)
           || (via_stack.startCoord.Y == 8191)
           || (via_stack.startCoord.Z == 31  )
           || (via_stack.endCoord.X   == 8191)
           || (via_stack.endCoord.Y   == 8191)
           || (via_stack.endCoord.Z   == 31  )
           || (via_stack.startSegment == -99 )
           || (via_stack.endSegment   == -99 )))  {
      // We got here, so one or more of the via_stack elements contains junk data, even
      // though the 'error' flag is FALSE. Issue a fatal error and exit the program:
      printf("\nERROR: An unexpected condition was detected in function 'findNearbyLayerTransition'. Please inform the\n");
      printf(  "       software developer of this fatal error message:\n");
      printf(  "           pathNum = %d     startLayer = %d      endLayer = %d     (x,y) = (%d,%d)\n",
               pathNum, startLayer, endLayer, x, y);
      printf(  "           enforceStartLayerOnly = %d     enforceEndLayerOnly = %d\n", enforceStartLayerOnly, enforceEndLayerOnly);
      printf(  "           startSegment = %d       startCoord: (%d,%d,%d)  <<== Potential error\n", via_stack.startSegment,
               via_stack.startCoord.X, via_stack.startCoord.Y, via_stack.startCoord.Z);
      printf(  "           endSegment   = %d         endCoord: (%d,%d,%d)  <<== Potential error\n", via_stack.endSegment,
               via_stack.endCoord.X, via_stack.endCoord.Y, via_stack.endCoord.Z);
      printf("\n");
    exit(1);
  }


  #ifdef DEBUG_findNearbyLayerTransition
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) %d via-stacks satisfied the startLayer (%d) and endLayer (%d) requirements at the end of findNearbyLayerTransition.\n",
           thread_num, num_vias, startLayer, endLayer);

    // Print out DEBUG message reporting whether a via was found or not:
    if (via_stack.error)  {
      printf("\n\nDEBUG: (thread %2d) Function 'findNearbyLayerTransition' failed to find a via or layer-transition between\n",
             omp_get_thread_num());
      printf(    "DEBUG: (thread %2d) layers '%s' and '%s' for net '%s'.\n\n", omp_get_thread_num(), user_inputs->routingLayerNames[startLayer],
             user_inputs->routingLayerNames[endLayer], user_inputs->net_name[pathNum]);
    }  // End of if-block for detecting no vias

    else  {
      printf("DEBUG: (thread %2d) At end of function 'findNearbyLayerTransition', we found via or layer-transitions that starts at\n", thread_num);
      if (start_via >= 0)  {
        printf("       (thread %2d) segment %d (%d,%d,%d) and ends at segment %d (%d,%d,%d), and is located %6.3f cells from (%d,%d)\n=============\n",
               thread_num, start_via, pathCoords[pathNum][start_via].X, pathCoords[pathNum][start_via].Y, pathCoords[pathNum][start_via].Z,
               end_via, pathCoords[pathNum][end_via].X, pathCoords[pathNum][end_via].Y, pathCoords[pathNum][end_via].Z,
               closest_distance, x, y);
      }
      else  {
        // We got here, so start_via is equal to -1. Print out the coordinates of the path's start-terminal:
        printf("       (thread %2d) segment %d (%d,%d,%d) and ends at segment %d (%d,%d,%d), and is located %6.3f cells from (%d,%d)\n=============\n",
               thread_num, start_via, mapInfo->start_cells[pathNum].X, mapInfo->start_cells[pathNum].Y, mapInfo->start_cells[pathNum].Z,
               end_via, pathCoords[pathNum][end_via].X, pathCoords[pathNum][end_via].Y, pathCoords[pathNum][end_via].Z,
               closest_distance, x, y);
      }
    }  // End of if/else-block
  }  // End of if-block for DEBUG_ON
  #endif


  // Return information to calling routine:
  return(via_stack);

}  // End of function 'findNearbyLayerTransition'


//-----------------------------------------------------------------------------
// Name: findNearbyLayerTransition_wrapper
// Desc: Call function findNearbyLayerTransition() up to 3 times to locate
//       a via or layer-transition near coordinate (x,y), and which starts
//       on routing layer 'startLayer' and ends on routing layer 'endLayer'.
//       The first call to findNearbyLayerTransition() requires that both
//       the start- and end-layers match. If no such via is located, the
//       second call requires that only the start-layer match. If no such
//       vias are found, a final call is made that requires that only the
//       end-layer matches. If none of these attempts result in a via,
//       the function issues a non-fatal warning message.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_findNearbyLayerTransition_wrapper' and re-compile if you want
// verbose debugging print-statements enabled:
//
// #define DEBUG_findNearbyLayerTransition_wrapper 1
#undef DEBUG_findNearbyLayerTransition_wrapper

ViaStack_t findNearbyLayerTransition_wrapper(const int pathNum, int pathLengths[], Coordinate_t *pathCoords[],
                                             const int startLayer, const int endLayer, const int x, const int y,
                                             const MapInfo_t *mapInfo, const InputValues_t *user_inputs)  {

  #ifdef DEBUG_findNearbyLayerTransition_wrapper
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  int thread_num = omp_get_thread_num();
  if ((mapInfo->current_iteration >= 590) && (mapInfo->current_iteration <= 615) && ((pathNum == 24) || (pathNum == 25)))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in findNearbyLayerTransition_wrapper() because specific requirements were met.\n\n", thread_num);
    DEBUG_ON = TRUE;
    printf("\nDEBUG: (thread %2d) Entered function findNearbyLayerTransition_wrapper with following inputs:\n", thread_num);
    printf(  "DEBUG: (thread %2d)         pathNum = %d\n", thread_num, pathNum);
    printf(  "DEBUG: (thread %2d)      startLayer = %d\n", thread_num, startLayer);
    printf(  "DEBUG: (thread %2d)        endLayer = %d\n", thread_num, endLayer);
    printf(  "DEBUG: (thread %2d)           (x,y) = (%d,%d)\n", thread_num, x, y);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Define variable that contains the start- and end-segments of the via, the path number,
  // the start- and end-coordinates of the stack, and the shape-type of the last segment
  // in the via-stack:
  ViaStack_t via_stack;

  // Copy the input path-number to the output in order to satisfy the 'ViaStack_t'
  // requirement to include the path-number:
  via_stack.pathNum = pathNum;

  // Initialize the 'endShapeType' element of the via-stack to 'TRACE'. This will be
  // updated to 'VIA_UP' or 'VIA_DOWN' if a via-stack is successfully found:
  via_stack.endShapeType = TRACE;

  // Attempt #1 of 3: Call findNearbyLayerTransition(), requiring the via to match both
  //                  the start- and end-layer.
  #ifdef DEBUG_findNearbyLayerTransition_wrapper
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Function findNearbyLayerTransition_wrapper is about to call findNearbyLayerTransition for first time with following inputs:\n",
           omp_get_thread_num());
    printf("DEBUG: (thread %2d)            enforceStartLayerOnly = FALSE        enforceEndLayerOnly = FALSE\n", omp_get_thread_num());
  }
  #endif
  via_stack = findNearbyLayerTransition(pathNum, pathLengths, pathCoords, startLayer, endLayer, x, y, FALSE, FALSE, mapInfo, user_inputs);

  // Check whether a via was successfully found from attempt #1
  if (via_stack.error == TRUE)  {

    // We got here, so the first call to findNearbyLayerTransition() failed to find a via. So we proceed with...
    //
    // Attempt #2 of 3: Call findNearbyLayerTransition(), requiring the via to match only the start-layer:
    //
    #ifdef DEBUG_findNearbyLayerTransition_wrapper
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Function findNearbyLayerTransition_wrapper is about to call findNearbyLayerTransition for second time with following inputs:\n",
             omp_get_thread_num());
      printf("DEBUG: (thread %2d)            enforceStartLayerOnly = TRUE         enforceEndLayerOnly = FALSE\n", omp_get_thread_num());
    }
    #endif
    via_stack = findNearbyLayerTransition(pathNum, pathLengths, pathCoords, startLayer, endLayer, x, y, TRUE, FALSE, mapInfo, user_inputs);

    // Check whether a via was successfully found from attempt #2
    if (via_stack.error == TRUE)  {

      // We got here, so the second call to findNearbyLayerTransition() failed to find a via. So we proceed with...
      //
      // Attempt #3 of 3: Call findNearbyLayerTransition(), requiring the via to match only the end-layer:
      //
      #ifdef DEBUG_findNearbyLayerTransition_wrapper
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Function findNearbyLayerTransition_wrapper is about to call findNearbyLayerTransition for third time with following inputs:\n",
               omp_get_thread_num());
        printf("DEBUG: (thread %2d)            enforceStartLayerOnly = FALSE        enforceEndLayerOnly = TRUE\n", omp_get_thread_num());
      }
      #endif
      via_stack = findNearbyLayerTransition(pathNum, pathLengths, pathCoords, startLayer, endLayer, x, y, FALSE, TRUE, mapInfo, user_inputs);

      // Check whether a via was successfully found from attempt #3
      if (via_stack.error == TRUE)  {

        // We got here, so the third and final call to findNearbyLayerTransition() failed to find a via. So we issue
        // a warning:
        printf("\n\nWARNING: (thread %2d) Function 'findNearbyLayerTransition_wrapper' failed to find a via or layer-transition between\n",
               omp_get_thread_num());
        printf(    "WARNING: (thread %2d) layers '%s' and '%s' for net '%s' near coordinate (%d,%d) cells.\n", omp_get_thread_num(),
               user_inputs->routingLayerNames[startLayer], user_inputs->routingLayerNames[endLayer], user_inputs->net_name[pathNum], x, y);
      }  // End of if-block for unsuccessful attempt #3
    }  // End of if-block for unsuccessful attempt #2
  }  // End of if-block for unsuccessful attempt #1


  // Determine whether all segments in the via-stack are vertically aligned with each other:
  via_stack.isVertical = TRUE;
  if (! via_stack.error)  {
    // Iterate over all but the first segment of the via-stack to confirm these segments' (x,y)
    // coordinates match those of the first segment:
    for (int segment = max(via_stack.startSegment, 0); segment <= via_stack.endSegment; segment++)  {

      // printf("DEBUG: Value of via_stack.startCoord.X is %d\n", via_stack.startCoord.X);
      // printf("DEBUG: Value of via_stack.startCoord.Y is %d\n", via_stack.startCoord.Y);
      // printf("DEBUG: Value of pathCoords[%d][%d].X is %d\n", pathNum, segment, pathCoords[pathNum][segment].X);
      // printf("DEBUG: Value of pathCoords[%d][%d].Y is %d\n", pathNum, segment, pathCoords[pathNum][segment].Y);

      // Compare the x- and y-coordinates of the current segment with the initial segment.
      // If they differ, then set 'isVertical' to false and break out of the loop:
      if (   (via_stack.startCoord.X != pathCoords[pathNum][segment].X)
          || (via_stack.startCoord.Y != pathCoords[pathNum][segment].Y))  {
        via_stack.isVertical = FALSE;
        break;
      }  // End of if-block for finding different x- or y-coordinates
    }  // End of for-loop for index 'segment'
  }
  else  {
    // We got here, so via_stack.error is TRUE. So we define 'isVertical' as FALSE because
    // we didn't successfully find *any* via-stack:
    via_stack.isVertical = FALSE;
  }


  #ifdef DEBUG_findNearbyLayerTransition_wrapper
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Function findNearbyLayerTransition_wrapper found the following via:\n", omp_get_thread_num());
    printf("DEBUG: (thread %2d)            pathNum = %d\n", omp_get_thread_num(), via_stack.pathNum);
    printf("DEBUG: (thread %2d)              error = %d\n", omp_get_thread_num(), via_stack.error);
    printf("DEBUG: (thread %2d)       startSegment = %d\n", omp_get_thread_num(), via_stack.startSegment);
    printf("DEBUG: (thread %2d)         startCoord = (%d,%d,%d)\n", omp_get_thread_num(), via_stack.startCoord.X, via_stack.startCoord.Y, via_stack.startCoord.Z);
    printf("DEBUG: (thread %2d)         endSegment = %d\n", omp_get_thread_num(), via_stack.endSegment);
    printf("DEBUG: (thread %2d)           endCoord = (%d,%d,%d)\n", omp_get_thread_num(), via_stack.endCoord.X, via_stack.endCoord.Y, via_stack.endCoord.Z);
    printf("DEBUG: (thread %2d)       endShapeType = %d\n", omp_get_thread_num(), via_stack.endShapeType);
    printf("DEBUG: (thread %2d)         isVertical = %d\n", omp_get_thread_num(), via_stack.isVertical);
  }
  #endif

  // Return information to calling routine:
  return(via_stack);

}  // End of function 'findNearbyLayerTransition_wrapper'


//-----------------------------------------------------------------------------
// Name: add_path_center_info
// Desc: Add information about a path #pathNum that traverses cell 'cellInfo'
//       in the 'pathCenters' array. Also increment the 'numTraversingPathCenters'
//       variable.
//-----------------------------------------------------------------------------
void add_path_center_info(CellInfo_t *cellInfo, int pathNum, int shape_type)  {

  // printf("\nDEBUG: Entered function 'add_path_center_info' for pathNum %d and shapeType %d, numTraveringPathCenters=%d...\n",
  //         pathNum, shape_type, cellInfo->numTraversingPathCenters);

  // Increment number of traversing path-centers:
  cellInfo->numTraversingPathCenters++;

  if (cellInfo->numTraversingPathCenters > maxTraversingShapes)  {
    printf("ERROR: The value of numTraversingPathCenters exceeded the maximum\n");
    printf("       allowed value (%d). This is not expected, and reflects an\n", maxTraversingShapes);
    printf("       error in the software algorithm. Program is exiting.\n\n");
    exit(1);
  }

  // printf("        DEBUG: numTraversingPathCenters at (%d, %d, %d) incremented to %d.\n",
  //         x, y, z, cellInfo[x][y][z].numTraversingPathCenters);

  // Allocate memory for the additional traversing path-center in the
  // 'pathCenters' array:
  // printf("        DEBUG: Before realloc, address of cellInfo[%d][%d][%d].pathCenters is %p\n",
  //          x, y, z, cellInfo[x][y][z].pathCenters);
  cellInfo->pathCenters = realloc(cellInfo->pathCenters,
                   cellInfo->numTraversingPathCenters * sizeof(PathAndShapeInfo_t));
  if (cellInfo->pathCenters == 0)  {
    printf("ERROR: While adding path-center for path %d to 'pathCenters' member, program was\n", pathNum);
    printf("       unable to allocate memory for element %d of array 'cellInfo.pathCenters'.\n" ,
           cellInfo->numTraversingPathCenters);
    printf("       Program will terminate.\n");
    exit(1);
  }

  // printf("        DEBUG: Successfully allocated memory for cellInfo[%d][%d][%d].pathCenters.\n",
  //         x, y, z);

  // Add path number to the new array element:
  cellInfo->pathCenters[cellInfo->numTraversingPathCenters - 1].pathNum = pathNum;
  // printf("        DEBUG: cellInfo[%d][%d][%d].pathCenters[%d].pathNum = %d\n", x, y, z,
  //            cellInfo[x][y][z].numTraversingPathCenters - 1, pathNum);

  // Add shape-type to the new array element:
  cellInfo->pathCenters[cellInfo->numTraversingPathCenters - 1].shapeType = shape_type;
  // printf("        DEBUG: cellInfo[%d][%d][%d].pathCenters[%d].shapeType = %d\n", x, y, z,
  //            cellInfo[x][y][z].numTraversingPathCenters - 1, shape_type);

}  // End of function 'add_path_center_info'


//-----------------------------------------------------------------------------
// Name: getIndexOfTraversingPath
// Desc: Determine whether path 'pathNum' with shape type 'shapeType' and design-
//       rule subset 'DR_subset' traverses cell 'cellInfo'. If so, return the
//       index number. If path does not explicitly traverse cell, then return -1.
//-----------------------------------------------------------------------------
int getIndexOfTraversingPath(CellInfo_t *cellInfo, const int pathNum, const unsigned short DR_subset,
                             const unsigned short shapeType)  {

  // Get number of paths that traverse this cell:
  unsigned numTraversingPaths = cellInfo->numTraversingPaths;

  // If no paths traverse this cell, then return -1
  if (numTraversingPaths == 0)  {
    return(-1);
  }

  // Iterate through the paths that traverse this cell and compare
  // path number to 'pathNum', compare design-rule subset to 'DR_subset,'
  // and compare shape type to 'shapeType':
  for (int pathIndex = 0; pathIndex < numTraversingPaths; pathIndex++)  {

    // If pathNum is found among the traversing paths, then return the index:
    if ((     pathNum == cellInfo->congestion[pathIndex].pathNum)
        && (DR_subset == cellInfo->congestion[pathIndex].DR_subset)
        && (shapeType == cellInfo->congestion[pathIndex].shapeType))  {

      return(pathIndex);

    }  // End of if-block
  }  // End of for-loop

  // If we got here, then pathNum with shapeType was not found among the
  // traversing paths. So return -1
  return(-1);

}  // End of function 'getIndexOfTraversingPath'


//-----------------------------------------------------------------------------
// Name: swapStartAndEndTerminals
// Desc: Swap the starting and ending terminals of path number 'pathNum',
//       including the coordinates in cell units (but not in micron units).
//       If the net is a diff-pair net or a pseudo-path, then swap the
//       start- and end-pitch of the terminals (in microns, not in cells).
//
//       Finally, toggle the Boolean flag in the 'start_end_terms_swapped'
//       element for the given path number.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_swapStartAndEndTerminals' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_swapStartAndEndTerminals 1
#undef DEBUG_swapStartAndEndTerminals

void swapStartAndEndTerminals(int pathNum, MapInfo_t *mapInfo)  {

  int i = pathNum;  // Copy path number to variable with shorter name
  Coordinate_t temp_coordinate;  // Temporary cell/layer coordinates
  float temp_pitch_microns; // Temporary variable for swapping floating-point pitch values

  #ifdef DEBUG_swapStartAndEndTerminals
  printf("\nDEBUG: Before swapping start and end points for path #%d:\n", i);
  printf("       Starting cell coordinates: (%d, %d, %d)\n", mapInfo->start_cells[i].X, mapInfo->start_cells[i].Y, mapInfo->start_cells[i].Z);
  printf("       Ending cell coordinates: (%d, %d, %d)\n", mapInfo->end_cells[i].X, mapInfo->end_cells[i].Y, mapInfo->end_cells[i].Z);
  printf("       Start-terminal pitch: %6.3f microns\n", mapInfo->diffPairStartTermPitchMicrons[i]);
  printf("       End-terminal pitch: %6.3f microns\n", mapInfo->diffPairEndTermPitchMicrons[i]);
  printf("       start_end_terms_swapped flag = %d\n\n", mapInfo->start_end_terms_swapped[i]);
  #endif

  // Save starting cell-coordinates into temporary variables, and then
  // swap the starting and ending cell-coordinates:
  temp_coordinate = copyCoordinates(mapInfo->start_cells[i]);
  mapInfo->start_cells[i] = copyCoordinates(mapInfo->end_cells[i]);
  mapInfo->end_cells[i]   = copyCoordinates(temp_coordinate);

  temp_pitch_microns = mapInfo->diffPairStartTermPitchMicrons[i];
  mapInfo->diffPairStartTermPitchMicrons[i] = mapInfo->diffPairEndTermPitchMicrons[i];
  mapInfo->diffPairEndTermPitchMicrons[i] = temp_pitch_microns;

  // Toggle the 'start_end_terms_swapped' Boolean flag for this path:
  if (mapInfo->start_end_terms_swapped[i])
    mapInfo->start_end_terms_swapped[i] = FALSE;
  else
    mapInfo->start_end_terms_swapped[i] = TRUE;

  #ifdef DEBUG_swapStartAndEndTerminals
  printf("DEBUG: After swapping start and end points for path #%d:\n", i);
  printf("       Starting cell coordinates: (%d, %d, %d)\n", mapInfo->start_cells[i].X, mapInfo->start_cells[i].Y, mapInfo->start_cells[i].Z);
  printf("       Ending cell coordinates: (%d, %d, %d)\n", mapInfo->end_cells[i].X, mapInfo->end_cells[i].Y, mapInfo->end_cells[i].Z);
  printf("       Start-terminal pitch: %6.3f microns\n", mapInfo->diffPairStartTermPitchMicrons[i]);
  printf("       End-terminal pitch: %6.3f microns\n", mapInfo->diffPairEndTermPitchMicrons[i]);
  printf("       start_end_terms_swapped flag = %d\n\n", mapInfo->start_end_terms_swapped[i]);
  #endif

}  // End of function 'swapStartAndEndTerminals'


//-----------------------------------------------------------------------------
// Name: createOneContiguousPath
// Desc: Generate a contiguous path from path given by 'pathCoords[]' that
//       contains no skipped cells. The resulting path is stored in the
//       contigPathCoords[] array.
//
//       In design-rule zones for which the path's line width is at least two
//       cells, do not create intermediate path-segments. Otherwise, create
//       intermediate path-segments to ensure that the path has no gaps.
//-----------------------------------------------------------------------------
void createOneContiguousPath(int pathNum, Coordinate_t start_cells, MapInfo_t *mapInfo, int pathLength,
                             Coordinate_t pathCoords[], Coordinate_t *contigPathCoords[],
                             int *contiguousPathLength, InputValues_t *user_inputs, CellInfo_t ***cellInfo)  {

  // Define the minimum linewidth, in cell-units, for which this function will *NOT* insert
  // intermediate cells:
  const float min_linewidth_to_insert_cells = 2.0;

  // printf("\nDEBUG: At beginning of 'createOneContiguousPath' for path %d, pathLength=%d, *contiguousPathLength=%d, contigPathCoords=%p, *contigPathCoords=%p\n",
  //        pathNum, pathLength, *contiguousPathLength, contigPathCoords, *contigPathCoords);
  // printf("DEBUG:   start_cells = (%d,%d,%d), pathLength = %d, contiguousPathLength = %d\n", start_cells.X, start_cells.Y, start_cells.Z,
  //        pathLength, *contiguousPathLength);

  int x1, y1, z1, x2, y2, z2, x3, y3, z3, prevX, prevY, prevZ; // Temporary variables for x/y/z locations
  int length = 0;  // 'length' is length of new array

  // Allocate memory for new 'contigPathCoords' array. Initially, allocate
  // 4 times the memory of the corresponding 'pathCoords' array. We'll re-
  // allocate the space when we know the precise length. If the path-length
  // of the non-contiguous path is zero, then allocate 1 element to the
  // contiguous path's array to hold the start-terminal:
  *contigPathCoords = realloc(*contigPathCoords,
                              sizeof(Coordinate_t) * 4 * max(1, pathLength));

  if (*contigPathCoords == 0)  {
    printf("\n\nERROR: Failed to reallocate memory for array contigPathCoords in createOneContiguousPath.\n\n");
    exit(1);
  }

  // Add starting (x,y,z) location to new array:
  (*contigPathCoords)[length].X    = prevX = start_cells.X;
  (*contigPathCoords)[length].Y    = prevY = start_cells.Y;
  (*contigPathCoords)[length].Z    = prevZ = start_cells.Z;
  (*contigPathCoords)[length].flag = start_cells.flag;
  length++;

  // printf("DEBUG: In createOneContiguousPath, added start-coordinate (%d,%d,%d) to contigPathCoords array, element %d\n",
  //       (*contigPathCoords)[length].X, (*contigPathCoords)[length].Y, (*contigPathCoords)[length].Z, length);

  // Define the path-width, which will determine whether intermediate cells
  // are added to the path. This decision is captured in Boolean variable
  // 'add_intermediate_cells':
  int DR_set, DR_subset;
  float pathWidth_cells;
  unsigned char add_intermediate_cells = TRUE;
  if (XYZpointIsOutsideOfMap(prevX, prevY, prevZ, mapInfo))  {
    add_intermediate_cells = FALSE;
  }
  else  {
    DR_set = cellInfo[prevX][prevY][prevZ].designRuleSet;
    DR_subset = user_inputs->designRuleSubsetMap[pathNum][DR_set];
    pathWidth_cells = user_inputs->designRules[DR_set][DR_subset].copy_lineWidthMicrons / user_inputs->cell_size_um;
    if (pathWidth_cells >= min_linewidth_to_insert_cells)  {
      add_intermediate_cells = FALSE;
      // printf("\nDEBUG: In createOneContiguousPath, no intermediate cells will be created because pathWidth_cells (%.3f) is sufficiently large.\n\n",
      //        pathWidth_cells);
    }
    else  {
      // printf("\nDEBUG: In createOneContiguousPath, intermediate cells will be created because pathWidth_cells (%.3f) is sufficiently small.\n\n",
      //        pathWidth_cells);
    }
  }

  //
  // Iterate through each point in original 'pathCoords' path:
  //
  for (int i = 0; i < pathLength; i++ )  {
    // Get the (x,y,z) location for segment #i
    x1 = pathCoords[i].X;
    y1 = pathCoords[i].Y;
    z1 = pathCoords[i].Z;

    //
    // If intermediate cells should be added to the path, then do so:
    //
    if (add_intermediate_cells)  {

      //
      // Determine relationship between current (x,y,z) and previous.
      // Based on this relationship, add intermediate points to
      // the contigPathCoords array:
      //
      if (abs(z1 - prevZ) == 1)  {
        // Current (x,y,z) is above or below (in z-direction) from
        // previous (x,y,z) location. No need to add intermediate points.
      }  // End of if-block for vertical transition in z-direction

      else if (abs(x1 - prevX) + abs(y1 - prevY) == 2)  {
        // Current (x,y,z) is diagonal from previous (x,y,z). Add one
        // intermediate point north/south of original point at (x1,y1,z1):
        x2 = x1;
        y2 = prevY;
        z2 = prevZ;
        (*contigPathCoords)[length].X = x2;
        (*contigPathCoords)[length].Y = y2;
        (*contigPathCoords)[length].Z = z2;
        (*contigPathCoords)[length].flag = FALSE;
        length++;
      }  // End of if-block for diagonal cell

      else if ((abs(x1-prevX) == 2) && (abs(y1-prevY) == 1))  {
        // Current (x,y,z) is a knight's move from previous (x,y,z),
        // with deltaX = 2 and deltaY = 1. Add 2 intermediate points:
        //
        //      ----------   s = start = previous (x,y)
        //   y1 |  |i2| e|   e = end   = (x1, y1)
        //      ----------  i1 = 1st intermediate point = ((pX+x1)/2,py)
        //   pY |s |i1|  |  i2 = 2nd intermediate point = (x from i1, y1)
        //      ----------
        //       pX    x1

        // Add first intermediate point east/west of previous point:
        x2 = (x1 + prevX) / 2;
        y2 = prevY;
        z2 = prevZ;
        (*contigPathCoords)[length].X = x2;
        (*contigPathCoords)[length].Y = y2;
        (*contigPathCoords)[length].Z = z2;
        (*contigPathCoords)[length].flag = FALSE;
        length++;

        // Add second intermediate point diagonal from previous point:
        x3 = x2;
        y3 = y1;
        z3 = prevZ;
        (*contigPathCoords)[length].X = x3;
        (*contigPathCoords)[length].Y = y3;
        (*contigPathCoords)[length].Z = z3;
        (*contigPathCoords)[length].flag = FALSE;
        length++;
      }  // End of else-block for knight's move left/right

      else if ((abs(x1-prevX) == 1) && (abs(y1-prevY) == 2))  {
        // Current (x,y,z) is a knight's move from previous (x,y,z),
        // with deltaX = 1 and deltaY = 2. Add 2 intermediate points:
        //
        //      -------
        //   y1 |  | e|    s = start = previous (x,y)
        //      -------    e = end   = (x1, y1)
        //      |i1|i2|   i1 = 1st intermediate point = (pX,(y1+pY)/2)
        //      -------   i2 = 2nd intermediate point = (x1, y from i1)
        //   pY |s |  |
        //      -------
        //       pX x1

        // Add first intermediate point north/south of previous point:
        x2 = prevX;
        y2 = (y1 + prevY) / 2;
        z2 = prevZ;
        (*contigPathCoords)[length].X = x2;
        (*contigPathCoords)[length].Y = y2;
        (*contigPathCoords)[length].Z = z2;
        (*contigPathCoords)[length].flag = FALSE;
        length++;

        // Add second intermediate point diagonal from previous point:
        x3 = x1;
        y3 = y2;
        z3 = prevZ;
        (*contigPathCoords)[length].X = x3;
        (*contigPathCoords)[length].Y = y3;
        (*contigPathCoords)[length].Z = z3;
        (*contigPathCoords)[length].flag = FALSE;
        length++;
      }  // End of else-block for knight's move up/down
    }  // End of if-block for add_intermediate_cells == TRUE

    //
    // Now that intermediate points have been added to the new array,
    // add the point from the original pathCoords array:
    //
    (*contigPathCoords)[length].X = prevX = x1;
    (*contigPathCoords)[length].Y = prevY = y1;
    (*contigPathCoords)[length].Z = prevZ = z1;
    (*contigPathCoords)[length].flag = FALSE;
    length++;

    // printf("DEBUG:          Contiguous path segment %d is (%d,%d,%d).\n",
    //         length, x1, y1, z1);

    // Get the new path-width, which will determine whether intermediate cells
    // are added to the path. This decision is captured in Boolean variable
    // 'add_intermediate_cells':
    DR_set = cellInfo[prevX][prevY][prevZ].designRuleSet;
    DR_subset = user_inputs->designRuleSubsetMap[pathNum][DR_set];
    pathWidth_cells = user_inputs->designRules[DR_set][DR_subset].width_um[TRACE] / user_inputs->cell_size_um;
    if (pathWidth_cells >= min_linewidth_to_insert_cells)  {
      add_intermediate_cells = FALSE;
    }
    else  {
      add_intermediate_cells = TRUE;
    }

  }  // End of for-loop for variable 'i' (=0 to pathLength)

  // We're done with current path, so record the length of its
  // contiguous path:
  *contiguousPathLength = length;
  // printf("DEBUG: *contiguousPathLength incremented to %d.\n", *contiguousPathLength);

  // Re-allocate memory for the 'contigPathCoords' array
  *contigPathCoords = realloc(*contigPathCoords, *contiguousPathLength * sizeof(Coordinate_t));
  if (*contigPathCoords == 0)  {
    printf("\n\nERROR: Failed to re-allocate memory for 'contigPathCoords' array in createOneContiguousPath.\n\n");
    exit(1);
  }

}  // End of function 'createOneContiguousPath'


//-----------------------------------------------------------------------------
// Name: createContiguousPaths
// Desc: For each path in the pathCoords array, generate a contiguous path that
//       contains no skipped cells. The resulting paths are stored in the
//       contigPathCoords array.
//-----------------------------------------------------------------------------
void createContiguousPaths(int numPaths, int pathLengths[], MapInfo_t *mapInfo,
                          Coordinate_t **pathCoords, Coordinate_t **contigPathCoords,
                          int contiguousPathLengths[], InputValues_t *user_inputs,
                          CellInfo_t ***cellInfo)  {

  // printf("\nDEBUG: Entered 'createContiguousPaths'...\n\n");
  // printf("DEBUG: mapInfo->mapWidth = %d, mapInfo->mapHeight = %d, mapInfo->numLayers = %d\n",
  //        mapInfo->mapWidth, mapInfo->mapHeight, mapInfo->numLayers);

  // Because each path can 'skip' cells when making 45- or 26.6-degree turns,
  // generate an array for each path that explicitly includes the skipped cells:
  //
  //   Original path:           Contiguous path:
  //  --------------------     --------------------
  //   |  |3 |3 |1 |  |  |      |  |3 |3 |1 |  |  |
  //  --------------------     --------------------
  //   |2 |  |1 |3 |3 |3 |      |2 |  |13|31|3 |3 |
  //  --------------------     --------------------
  //   |  |2 |1 |  |  |  |      |2 |2 |1 |1 |  |  |
  //  --------------------     --------------------
  //   |  |1 |  |2 |2 |2 |      |  |12|12|2 |2 |2 |
  //  --------------------     --------------------
  //   |1 |  |  |  |  |  |      |1 |1 |  |  |  |  |
  //  --------------------     --------------------

  //
  // Iterate through each path:
  //
  #pragma omp parallel for schedule(dynamic, 1)
  for (int path = 0; path < numPaths; path++)  {
    // printf("\nDEBUG: At beginning of 'createContiguousPaths', pathLength[%d] = %d, contiguousPathLength[%d] = %d\n",
    //        path, pathLengths[path], path, contiguousPathLengths[path]);
    // printf(  "DEBUG:   pathCoords[%d]=%p\n", path, pathCoords[path]);
    // printf(  "DEBUG:   &(contigPathCoords[%d])=%p, &(contiguousPathLengths[%d])=%p\n", path, &(contigPathCoords[path]),
    //        path, &(contiguousPathLengths[path]));

    // For path number 'pathNum' create a contiguous path based on the non-contiguous path:
    createOneContiguousPath(path, mapInfo->start_cells[path], mapInfo, pathLengths[path], pathCoords[path],
                            &(contigPathCoords[path]), &(contiguousPathLengths[path]), user_inputs, cellInfo );


    // printf("\nDEBUG:           At end of 'createContiguousPaths', contiguousPathLength[%d] = %d\n",
    //         path, contiguousPathLengths[path]);
    // printf(  "DEBUG:           x/y/z of final cell is (%d,%d,%d)\n\n",
    //        contigPathCoords[path][contiguousPathLengths[path] - 1].X,
    //        contigPathCoords[path][contiguousPathLengths[path] - 1].Y,
    //        contigPathCoords[path][contiguousPathLengths[path] - 1].Z);

  }  // End of for-loop for variable 'path' (=0 to numPaths)

}  // End of function 'createContiguousPaths'




//-----------------------------------------------------------------------------
// Name: addCongestionAroundPoint_withSubsetAndShapeType
// Desc: Add a given amount of congestion in the 'cellInfo' 3D matrix with a given
//       path-number, design-rule subset, and shape-type within a given radius
//       about a given (x,y) location on a given routing layer. The amount of
//       congestion is given by 'max_congestion_amount', and represents the amount
//       of congestion deposited at/near the centerPoint coordinate. The amount
//       of congestion decreases linearly with the distance from this centerPoint
//       down to half the 'max_congestion_amount' at the distance of 'radius'.
//       The path-number, design-rule subset, and shape-type are given
//       (respectively) by 'pathNum', 'DR_subset', and 'shapeType'. The
//       radius is given by 'radius'. The square of this value must also be
//       provided as 'radius_squared'.
//-----------------------------------------------------------------------------
void addCongestionAroundPoint_withSubsetAndShapeType(const int pathNum, const int DR_set, const int DR_subset, const char shapeType,
                                                     const Coordinate_t centerPoint, const int radius, const int radius_squared,
                                                     const int max_congestion_amount, const InputValues_t *user_inputs,
                                                     const MapInfo_t *mapInfo, CellInfo_t ***cellInfo)  {

  // Make a local copy of the z-coordinate:
  int z = centerPoint.Z;

  // Variable amount of congestion to be deposited at each (x,y) location:
  int congestion_amount;

  // Raster over a square around the centerPoint with radius 'radius + 1':
  for (int y = centerPoint.Y - radius; y <=  centerPoint.Y + radius; y++)  {
    int delta_y_squared = (y - centerPoint.Y) * (y - centerPoint.Y);
    for (int x = centerPoint.X - radius; x <=  centerPoint.X + radius; x++)  {

      // Confirm that (x,y) is within the map:
      if ((x < 0) || (x >= mapInfo->mapWidth) || (y < 0) || (y >= mapInfo->mapHeight))  {
        continue; // Skip to next (x,y) coordinate
      }  // End of if-block for (x,y) being outside the map

      // Confirm that (x,y) is not in a user-defined barrier:
      if (cellInfo[x][y][z].forbiddenTraceBarrier)  {
        continue; // Skip to next (x,y) coordinate
      }  // End of if-block for (x,y) being within a barrier

      // Confirm that (x,y) is not in a pin-swap zone
      if (cellInfo[x][y][z].swap_zone)  {
        continue; // Skip to next (x,y) coordinate
      }  // End of if-block for (x,y) being within a pin-swap zone

      // Calculate the square of the distance ('point_radius_squared') between (x,y) and
      // the centerPoint coordinate:
      int point_radius_squared = (x - centerPoint.X) * (x - centerPoint.X)   +   delta_y_squared;

      // Check whether the (x,y) point is within 'radius' of the centerPoint.
      if (point_radius_squared <= radius_squared)  {

        // We got here, so (x,y) is within a distance 'radius' of the centerPoint. We
        // therefore add congestion to this (x,y) cell.

        // Get the design-rule number at (x,y). If it's different than the design-rule number at the
        // center point (rare case), then calculate the design-rule subset number that corresponds to subset
        // number 'point_DR_subset' from the center-point so we can deposit congestion of the correct
        // design-rule subset:
        int point_DR_set = cellInfo[x][y][z].designRuleSet;  // Get DR set number at (x,y) point

        // Initially, assign the DR subset for deposited congestion as though the (x,y) point had
        // the same design-rule number as the center-point:
        int deposited_DR_subset = DR_subset;

        // Compare the point's DR number to the center-point's DR number. They're usually the same,
        // so we use the __builtin_expect compiler directive to tell compiler to expect a FALSE result
        // most of the time:
        if (__builtin_expect(point_DR_set != DR_set, 0))  {

          // We got here, so the (x,y) point is in a different design-rule zone from the center-point.
          // Get the design-rule subset number that corresponds to the 'point_DR_subset' from the
          // design-rule zone at the center-point:
          deposited_DR_subset = user_inputs->foreign_DR_subset[DR_set][DR_subset][point_DR_set];

        }  // End of if-block for (point_DR_set != center_DR_set)

        // Calculate the amount of congestion to be deposited based on the radius for the current
        // (x,y) location:
        if (radius_squared > 0)  {
          congestion_amount = max_congestion_amount / 2
                              +  (max_congestion_amount / 2) * (float)(1.0 - sqrt((float)point_radius_squared / (float)radius_squared));
        }
        else  {
          congestion_amount = max_congestion_amount;
        }

        //
        // Add congestion to the point:
        //
        if (congestion_amount)  {
          addCongestion(&(cellInfo[x][y][z]), pathNum, deposited_DR_subset, shapeType, congestion_amount);
        }  // End of if-block for (congestion_amount > 0)

      }  // End of if-block for (x,y) being within the 'radius' of the centerPoint coordinate
    }  // End of for-loop for index 'x'
  }  // End of for-loop for index 'y'

}  // End of function 'addCongestionAroundPoint_withSubsetAndShapeType'


//-----------------------------------------------------------------------------
// Name: addCongestionAroundTerminal
// Desc: Add congestion (in the 'cellInfo' 3D matrix) at each cell around the
//       point 'centerPoint' that has shape-type 'centerShapeType.
//-----------------------------------------------------------------------------
void addCongestionAroundTerminal(const int pathNum, const Coordinate_t centerPoint, const char centerShapeType,
                                 const InputValues_t *user_inputs, const MapInfo_t *mapInfo, CellInfo_t ***cellInfo)  {

  // Factor by which we multiply ONE_TRAVERSAL when depositing additional congestion
  // around non-pseudo-terminals. A value of 1.0 means that we're essentially
  // doubling the congestion around each non-pseudo-terminal. This factor is used
  // in function addCongestionAroundTerminal() for both user-defined terminals and
  // for terminals of diff-pair connections.
  const float TERMINAL_CONGESTION_FACTOR = 1.0;

  // Get the design-rule set for the location of the centerPoint:
  int center_DR_set = cellInfo[centerPoint.X][centerPoint.Y][centerPoint.Z].designRuleSet;

  // Get the design-rule subset for this particular design-rule set and
  // path number:
  int center_DR_subset = user_inputs->designRuleSubsetMap[pathNum][center_DR_set];

  // Calculate the amount of congestion to be deposited.
  int congestion_amount = (int)(ONE_TRAVERSAL * TERMINAL_CONGESTION_FACTOR);

  // Calculate the subset/shapeType index for the centerPoint, which is simply:
  //      subset * 3     +     shapeType:
  int centerSubsetShapeTypeIndex = center_DR_subset * NUM_SHAPE_TYPES  +  centerShapeType;

  // Iterate over all design-rule subsets 'point_DR_subset' in design-rule set 'center_DR_set':
  for (int point_DR_subset = 0; point_DR_subset < user_inputs->numDesignRuleSubsets[center_DR_set]; point_DR_subset++)  {

    // If the subset is not used by any nets in the map, then continue to the next subset:
    if (! user_inputs->DR_subsetUsed[center_DR_set][point_DR_subset])  {
      continue;
    }

    for (int point_shapeType = 0; point_shapeType < NUM_SHAPE_TYPES; point_shapeType++)  {

      // Calculate the subset/shapeType index for the combination of design-rule
      // subset 'point_DR_subset' and shape-type 'point_shapeType' at point (x,y)
      int pointSubsetShapeTypeIndex = point_DR_subset * NUM_SHAPE_TYPES  +  point_shapeType;

      // Calculate the radius from the centerPoint where congestion should be deposited. Note that we add 1 in order to
      // slightly increase the radius in which congestion is deposited:
      int congestion_radius = 1 + user_inputs->cong_radius[center_DR_set][centerSubsetShapeTypeIndex][center_DR_set][pointSubsetShapeTypeIndex];

      // Calculate the square of this radius. Note that we add 1 in order to slightly increase the radius
      // in which congestion is deposited:
      int congestion_radius_squared = 1 + user_inputs->cong_radius_squared[center_DR_set][centerSubsetShapeTypeIndex][center_DR_set][pointSubsetShapeTypeIndex];

      // Add congestion around point 'centerPoint' with path number 'pathNum', design-rule subset 'point_DR_subset', and
      // shape-type 'point_shapeType':
      addCongestionAroundPoint_withSubsetAndShapeType(pathNum, center_DR_set, point_DR_subset, point_shapeType, centerPoint, congestion_radius,
                                                            congestion_radius_squared, congestion_amount, user_inputs, mapInfo, cellInfo);

    }  // End of for-loop for index 'point_shapeType'
  }  // End of for-loop for index 'point_DR_subset'

}  // End of function 'addCongestionAroundTerminal'


//-----------------------------------------------------------------------------
// Name: findPath
// Desc: Finds a path using A* algorithm. The information in structure
//       'routingRestrictions' is used to limit the lateral search within a
//       given distance of a given (x,y) coordinate on a given layer. This
//       function returns the G-cost of the path, which will be zero if no
//       path was found. The 'record_explored_cells' parameter tells function
//       which cells to flag as having been explored:
//
//       record_explored_cells   Action
//       ---------------------   ---------------------------------------------
//                 0             Record no explored cells
//                 1             Record explored cells in '.explored' variable
//                 2             Record explored cells in '.explored_PP' variable
//                 3             Record explored cells in '.explored' and
//                               in '.explored_PP' variables
//
//       If Boolean parameter 'record_elapsed_time' is TRUE, then the elapsed
//       (wall-clock) time for finding the path will be saved in variable:
//       routability->path_elapsed_time[pathNum].
//
//       If Boolean input parameter 'useDijkstra' is TRUE, then this function
//       uses a heuristic value of zero. Otherwise, it uses a heuristic that
//       depends on the distance to the target and the DRC histories of the
//       routed path and of other paths in the map.
//
//       If Boolean parameter 'disableRandomCosts' is TRUE, then findPath()
//       disables any randomized changes to the congestion-related G-cost.
//
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_findPath' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_findPath 1
#undef DEBUG_findPath

unsigned long findPath(const MapInfo_t *mapInfo, CellInfo_t ***const cellInfo,
                       int pathNum, const Coordinate_t startCoord, const Coordinate_t endCoord,
                       Coordinate_t *pathCoords[], int *pathLength, InputValues_t *user_inputs,
                       RoutingMetrics_t *routability, PathFinding_t *pathFinding, const int record_explored_cells,
                       const int record_elapsed_time, const int useDijkstra, const RoutingRestriction_t *routingRestrictions,
                       const int disableRandomCosts, const int recognizeSelfCongestion)  {

  // Return-value to inform the calling routine that the start-
  // and end-coordinates are identical:
  const int zeroLength = 0;

  // Value to assign to 'pathLength' before/until a path is found:
  const int notStarted = 0;

  // Constants for the 'pathFinding->whichList[][][]' array:
  const char onOpenList = 0;            // Denotes that cell is on the Open List
  const char onClosedList = 10;         // Denotes that cell is on the Closed List

  // Constants for the variable 'path':
  const int nonexistent = 0;
  const int found = 1;


  #ifdef DEBUG_findPath
  // DEBUG code follows:
  //
  // For very large maps, define here the 3-dimensional window in (x,y,z) for which you want
  // detailed information sent to the log file. Without these constraints, the log file can
  // grow to >10 gigabytes:
  int x_window_min =    0;
  int x_window_max =  200;
  int y_window_min =    0;
  int y_window_max =  300;
  int z_window_min =    0;
  int z_window_max =    5;
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  int DEBUG_CRITERIA_MET = FALSE;
  if ((mapInfo->current_iteration >= 1) && (mapInfo->current_iteration <= 288) && ((pathNum == 0) || (pathNum == 0)))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in findPath() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;  // TRUE for now, but will be set to FALSE for certain x/y/z coordinates
    DEBUG_CRITERIA_MET = TRUE;  // TRUE throughout this function, regardless of x/y/z coordinates
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE

  // DEBUG code follows:
  if (DEBUG_ON)  {
    time_t tim = time(NULL);
    struct tm *now = localtime(&tim);
    printf("\nDEBUG: Entered 'findPath' at %02d-%02d-%d, %02d:%02d in thread %d for path %d from (%d,%d,%d) to (%d,%d,%d)...\n",
           now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, omp_get_thread_num(),
           pathNum, startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
    if (routingRestrictions->restrictionFlag)  {
      printf("DEBUG: (thread %2d)\n", omp_get_thread_num());
      printf("DEBUG: (thread %2d) Routing restrictions are in effect:\n", omp_get_thread_num());
      printf("DEBUG: (thread %2d)     centerX: %d\n", omp_get_thread_num(), routingRestrictions->centerX);
      printf("DEBUG: (thread %2d)     centerY: %d\n", omp_get_thread_num(), routingRestrictions->centerY);
      for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
        if (routingRestrictions->allowedLayers[layer])
          printf("DEBUG: (thread %2d) layer %d: routing ALLOWED with radius %6.3f microns (%6.3f cells)\n",
                 omp_get_thread_num(), layer, routingRestrictions->allowedRadiiMicrons[layer], routingRestrictions->allowedRadiiCells[layer]);
        else
        printf("DEBUG: (thread %2d) layer %d: routing NOT ALLOWED\n", omp_get_thread_num(), layer);
      }
    }
    else  {
      printf("DEBUG: (thread %2d) No routing restrictions are in effect.\n", omp_get_thread_num());
    }
    printf("DEBUG: (thread %2d)\n", omp_get_thread_num());
  }
  // End of DEBUG code.
  #endif


  // Keep track of how much elapsed time this call to function 'findPath' uses:
  time_t start, end;
  start = time(NULL); // Get number of seconds since the Epoch (Jan 1, 1970)

  // Create arrays that define the 18 allowed transitions from one cell to another cell:
  int num_transitions = 18;
  int allowedTransitions[] = {E,  N,  W,  S, NE, SE, NW, SW, NxNE, ExNE, ExSE, SxSE, SxSW, WxSW, WxNW, NxNW,   Up, Down};
  int allowedDeltaX[]      = {1,  0, -1,  0,  1,  1, -1, -1,    1,    2,    2,    1,   -1,   -2,   -2,   -1,    0,    0};
  int allowedDeltaY[]      = {0,  1,  0, -1,  1, -1,  1, -1,    2,    1,   -1,   -2,   -2,   -1,    1,    2,    0,    0};
  int allowedDeltaZ[]      = {0,  0,  0,  0,  0,  0,  0,  0,    0,    0,    0,    0,    0,    0,    0,    0,    1,   -1};

  // After the first iteration, shuffle the sequence of the above arrays so that the child-cells
  // are explored in a pseudo-random order each time this function is called. The seed for the
  // pseudo-random number generator is the cost of this path (in cell units) from the most recent
  // iteration. The for-loops below use the Fisher–Yates shuffle algorithm.
  if (mapInfo->current_iteration > 1)  {
    unsigned int seed = abs((unsigned int) routability->path_cost[pathNum]);
    // printf("DEBUG: (thread %2d) Seed for pseudo-random numbers is %'d for path %d ( = cost from previous iteration)\n",
    //         omp_get_thread_num(), seed, pathNum);

    for (int i = num_transitions - 1; i > 0; i--)  {
      // Generate a random index value from 0 to i
      int j = rand_r(&seed) % (i + 1);

      // Exchange elements for 'allowedTransitions' array:
      int temp = allowedTransitions[j];
      allowedTransitions[j] = allowedTransitions[i];
      allowedTransitions[i] = temp;

      // Exchange elements for 'allowedDeltaX' array:
      temp = allowedDeltaX[j];
      allowedDeltaX[j] = allowedDeltaX[i];
      allowedDeltaX[i] = temp;

      // Exchange elements for 'allowedDeltaY' array:
      temp = allowedDeltaY[j];
      allowedDeltaY[j] = allowedDeltaY[i];
      allowedDeltaY[i] = temp;

      // Exchange elements for 'allowedDeltaZ' array:
      temp = allowedDeltaZ[j];
      allowedDeltaZ[j] = allowedDeltaZ[i];
      allowedDeltaZ[i] = temp;
    }  // End of for-loop for index 'i' to shuffle arrays
  }  // End of if-block for current_iteration > 1


  #ifdef DEBUG_findPath
  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) In iteration #%d, the sequence of child-cell exploration for path %d follows:\n",
           omp_get_thread_num(), mapInfo->current_iteration, pathNum);
    for (int i = 0; i < num_transitions; i++)  {
      printf("DEBUG: (thread %2d)     (delta-X, delta-Y, delta-Z) = (%d,%d,%d)\n", omp_get_thread_num(), allowedDeltaX[i], allowedDeltaY[i], allowedDeltaZ[i]);
    }  // End of for-loop for index 'i'
    printf("DEBUG: (thread %2d) ----------------------\n", omp_get_thread_num());
  }
  #endif

  // Fetch the Boolean flag (randomize_congestion) that determines whether the current
  // path was randomly selected to have its congestion-related G-cost increased or reduced:
  int random_reduction_flag;
  if (disableRandomCosts)  {
    random_reduction_flag = FALSE;
  }
  else  {
    random_reduction_flag = routability->randomize_congestion[pathNum];
  }

  // If this path was randomly selected to have it's congestion-related G-cost modified, then
  // calculate the scaling factor, which ranges from 0.02 to 1.00 (for reductions), or from
  // 1.00 to 4.0 (for increases), based on the history of DRC violations in the map and
  // history of DRC violations for this specific path. Also disable any routing restrictions
  // for this pseudo-randomly selected path:
  float congestion_scale_factor = 1.0;
  if (random_reduction_flag)  {

    // We got here, so the random_reduction_flag equals either 'DECREASE' or 'INCREASE'

    // Reduce the 'congestion_scale_factor' if the flag is set to 'DECREASE':
    if (random_reduction_flag == DECREASE)  {

      congestion_scale_factor = 1.0 - 0.98  *  (1.0   -   0.2 * routability->fractionRecentIterationsWithoutMapDRCs)
                                            *  (1.0   -         routability->fractionRecentIterationsWithoutPathDRCs[pathNum]);

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("\nDEBUG (thread %2d): Randomly decreasing congestion_scale_factor in findPath: congestion_scale_factor = %.4f for path %d in iter #%d, with recent map DRC-free fraction=%1.3f and path DRC-free fraction=%1.3f\n\n",
               omp_get_thread_num(), congestion_scale_factor, pathNum, mapInfo->current_iteration,
               routability->fractionRecentIterationsWithoutMapDRCs, routability->fractionRecentIterationsWithoutPathDRCs[pathNum]);
      }
      #endif

    }  // End of if-block for (random_reduction_flag == DECREASE)

    // Increase the 'congestion_scale_factor' because the flag is set to 'INCREASE':
    else  {

      congestion_scale_factor = 1.0 +  4.0  *  (1.0   -   0.2 * routability->fractionRecentIterationsWithoutMapDRCs)
                                            *  (1.0   -         routability->fractionRecentIterationsWithoutPathDRCs[pathNum]);

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("\nDEBUG (thread %2d): Randomly increasing congestion_scale_factor in findPath:, congestion_scale_factor = %.4f for path %d in iter #%d, with recent map DRC-free fraction=%1.3f and path DRC-free fraction=%1.3f\n\n",
               omp_get_thread_num(), congestion_scale_factor, pathNum, mapInfo->current_iteration,
               routability->fractionRecentIterationsWithoutMapDRCs, routability->fractionRecentIterationsWithoutPathDRCs[pathNum]);
      }
      #endif
    }  // End of else-block for (random_reduction_flag == INCREASE)
  }  // End of if-block for random_reduction_flag != zero

  // Define variable that will hold the congestion-related G-cost:
  unsigned long congestion_penalty;

  #ifdef DEBUG_findPath
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) In findPath for path %d for iteration %d: fractionRecentIterationsWithoutPathDRCs = %6.3f.\n",
           omp_get_thread_num(), pathNum, mapInfo->current_iteration, routability->fractionRecentIterationsWithoutPathDRCs[pathNum]);
    printf("DEBUG: (thread %2d)   fractionRecentIterationsWithoutMapDRCs = %6.3f.\n",
           omp_get_thread_num(), routability->fractionRecentIterationsWithoutMapDRCs);
  }
  #endif

  // Capture the swap-zone number of this path if its start-terminal is in a swap-zone. Otherwise,
  // assign the swap-zone number to zero. (Even though a net's user-defined terminal might lie in
  // a swap-zone, this function is also used to connect intermediate points along such paths. For
  // these intermediate path-finding exercises, we don't want findPath to explore a swap-zone):
  unsigned short pathSwapZone;
  if (cellInfo[startCoord.X][startCoord.Y][startCoord.Z].swap_zone)  {
    pathSwapZone = mapInfo->swapZone[pathNum];
  }
  else  {
    // The user-defined terminal might lie in a pin-swap zone, but not the start-coordinate
    // for this path-finding run. So we assign 'pathSwapZone' to zero, thereby preventing findPath()
    // from exploring cells within any pin-swap zones:
    pathSwapZone = 0;
  }

  const int walkable = 0;     // denotes that corner cell is walkable
  const int unwalkable = -1;  // denotes that corner cell is unwalkable
  const int walkable_swap_interface = 0;  // denotes that interface to/from pin-swap zone is walkable
  const int unwalkable_swap_interface = -1;  // denotes that interface to/from pin-swap zone is not walkable
  int parentXval=0, parentYval=0, parentZval=0,
  m=0, u=0, v=0, temp=0, corner=0, swap_interface=0, numberOfOpenListItems=0,
  cellPosition, newOpenListItemID=0;
  unsigned long tempGcost = 0;
  int tempx, tempy, tempz, pathX, pathY, pathZ;

  // Returned result from this function:
  unsigned long path = 0;

  // Variable to record the final path's total G-cost:
  unsigned long total_Gcost = 0;

  // Initialize the 'whichList' and 'sortNumber' arrays:
  initializePathFindingArrays(pathFinding, mapInfo);

  #ifdef DEBUG_findPath
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) In iteration #%d, searching for path %d from (%d, %d, %d) to (%d, %d, %d)...\n",
           omp_get_thread_num(), mapInfo->current_iteration, pathNum, startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);
  }
  #endif

  // Step 1: Quick Path Checks: Under some circumstances no path needs to be generated ...

  //  If starting and ending locations are in the same location...
  if ((startCoord.X == endCoord.X) && (startCoord.Y == endCoord.Y) && (startCoord.Z == endCoord.Z))  {
    printf("\nINFO: (thread %2d) Failed to find a path because start- and end-coordinates are identical.\n\n",
           omp_get_thread_num());
    printf(  "      (thread %2d)     Start: (%d,%d,%d)   End: (%d,%d,%d)\n", omp_get_thread_num(),
            startCoord.X, startCoord.Y, startCoord.Z, endCoord.X, endCoord.Y, endCoord.Z);

    // Place the ending coordinate in the pathCoords array:
    *pathLength = 1;
    *pathCoords = realloc(*pathCoords, *pathLength * sizeof(Coordinate_t));
    if (*pathCoords == 0)  {
      printf("\n\nERROR: Failed to reallocate memory for array pathCoords in function 'findPath'.\n\n");
      exit(1);
    }

    (*pathCoords)[0].X    = endCoord.X;
    (*pathCoords)[0].Y    = endCoord.Y;
    (*pathCoords)[0].Z    = endCoord.Z;
    (*pathCoords)[0].flag = FALSE;

    // Calculate the elapsed (wall-clock) time to find this path:
    if (record_elapsed_time)  {
      end = time(NULL);
      routability->path_elapsed_time[pathNum] = (int) (end - start);
    }

    // Return the value '0' to inform the calling routine that the start-
    // and end-coordinates are identical:
    return(zeroLength);
  }

  //  Return a nonexistent path if start location or target location is outside of the map:
  if (    pointIsOutsideOfMap(startCoord, mapInfo)
       || pointIsOutsideOfMap(endCoord, mapInfo)  )  {
    printf("\n\n");
    printf("INFO: Exiting 'findPath' function because the start- or end-location is outside of the map:\n");
    printf("       start: (%d,%d,%d) cells, (%6.3f, %6.3f) microns\n", startCoord.X, startCoord.Y, startCoord.Z,
           startCoord.X * user_inputs->cell_size_um, startCoord.Y * user_inputs->cell_size_um);
    printf("         end: (%d,%d,%d) cells, (%6.3f, %6.3f) microns\n", endCoord.X, endCoord.Y, endCoord.Z,
           endCoord.X * user_inputs->cell_size_um, endCoord.Y * user_inputs->cell_size_um);
    goto noPath;
  }  // End of if-block for net terminals beyond map boundaries

  //
  // Return a nonexistent path if start location or target location is unwalkable or otherwise illegal. Such
  // configurations include:
  //    =>  start- or end-terminal is in a user-defined barrier, or
  //    =>  start- or end-terminal is too close to a user-defined barrier, or
  //    =>  end-terminal is too close to a swap-zone, and the start-terminal is not close to
  //        a swap zone and not inside a swap-zone.
  //
  if (   cellInfo[startCoord.X][startCoord.Y][startCoord.Z].forbiddenTraceBarrier
      || cellInfo[endCoord.X][endCoord.Y][endCoord.Z].forbiddenTraceBarrier
      || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, startCoord.X, startCoord.Y, startCoord.Z, pathNum, TRACE)
      || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, endCoord.X, endCoord.Y, endCoord.Z, pathNum, TRACE)
      || (    get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, endCoord.X, endCoord.Y, endCoord.Z, pathNum, TRACE)
           && (! get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, startCoord.X, startCoord.Y, startCoord.Z, pathNum, TRACE)
           && (! cellInfo[startCoord.X][startCoord.Y][startCoord.Z].swap_zone))))  {
    printf("\n\n");
    printf("INFO: Exiting 'findPath' function because the start- or end-location is unwalkable or otherwise illegal:\n");
    printf("       start: (%d,%d,%d) cells [(%6.3f, %6.3f) microns] with\n", startCoord.X, startCoord.Y, startCoord.Z,
            startCoord.X * user_inputs->cell_size_um, startCoord.Y * user_inputs->cell_size_um);

    printf("             user-defined un-walkability %d, barrier-proximity un-walkability %d, pin-swap-proximity un-walkability %d, and pin-swap zone %d.\n",
            cellInfo[startCoord.X][startCoord.Y][startCoord.Z].forbiddenTraceBarrier,
            get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, startCoord.X, startCoord.Y, startCoord.Z, pathNum, TRACE),
            get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, startCoord.X, startCoord.Y, startCoord.Z, pathNum, TRACE),
            cellInfo[startCoord.X][startCoord.Y][startCoord.Z].swap_zone);

    printf("         end: (%d,%d,%d) cells [(%6.3f, %6.3f) microns] with\n", endCoord.X, endCoord.Y, endCoord.Z,
            endCoord.X * user_inputs->cell_size_um, endCoord.Y * user_inputs->cell_size_um);

    printf("             user-defined un-walkability %d, barrier-proximity un-walkability %d, pin-swap-proximity un-walkability %d, and pin-swap zone %d.\n",
            cellInfo[endCoord.X][endCoord.Y][endCoord.Z].forbiddenTraceBarrier,
            get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, endCoord.X, endCoord.Y, endCoord.Z, pathNum, TRACE),
            get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, endCoord.X, endCoord.Y, endCoord.Z, pathNum, TRACE),
            cellInfo[endCoord.X][endCoord.Y][endCoord.Z].swap_zone);

    goto noPath;
  }  // End of if-block for unwalkable net terminals

  // Return a nonexistent path if design rules at the start-location or target-location
  // prohibit routing for this particular net:
  //
  {
    // Get the allowed directions for routing from the start-cell. This information
    // is based on the design-rule number (location-specific) and design-rule subset
    // (net-specific):
    int DR_num    = cellInfo[startCoord.X][startCoord.Y][startCoord.Z].designRuleSet;
    int DR_subset = user_inputs->designRuleSubsetMap[pathNum][DR_num];
    int allowedRoutingDirections = user_inputs->designRules[DR_num][DR_subset].routeDirections;
    if (allowedRoutingDirections == NONE)  {
      printf("INFO: Exiting 'findPath' function because the design rules at the start-location, (%6.3f, %6.3f,%6.3f) microns,\n",
             startCoord.X * user_inputs->cell_size_um, startCoord.Y * user_inputs->cell_size_um, startCoord.Z * user_inputs->cell_size_um);
      printf("      allow routing directions of 'NONE'. Please fix this issue by modifying the input text file\n\n");
      goto noPath;
    }
    // Get the allowed directions for routing from the end-cell. This information
    // is based on the design-rule number (location-specific) and design-rule subset
    // (net-specific):
    DR_num    = cellInfo[endCoord.X][endCoord.Y][endCoord.Z].designRuleSet;
    DR_subset = user_inputs->designRuleSubsetMap[pathNum][DR_num];
    allowedRoutingDirections = user_inputs->designRules[DR_num][DR_subset].routeDirections;
    if (allowedRoutingDirections == NONE)  {
      printf("INFO: Exiting 'findPath' function because the design rules at the end-location, (%6.3f, %6.3f) microns\n",
             endCoord.X * user_inputs->cell_size_um, endCoord.Y * user_inputs->cell_size_um);
      printf("      on layer %s allow routing directions of 'NONE' for net %s. Please fix this issue by modifying the input text file.\n\n",
              user_inputs->routingLayerNames[endCoord.Z], user_inputs->net_name[pathNum]);
      goto noPath;
    }
  }  // End of block for checking start- and end-coordinates for 'NONE' routing direction

  *pathLength = notStarted; // i.e, = 0
  pathFinding->Gcost[startCoord.X][startCoord.Y][startCoord.Z] = 0; // Reset starting square's G value to 0

  #ifdef DEBUG_findPath
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Finished Step (1). About to start Step (2) for path %d.\n", omp_get_thread_num(), pathNum);
  }
  #endif

  // Step 2: Add the starting location to the open list of squares to be checked.
  numberOfOpenListItems = 1;
  pathFinding->openList[1] = 1;  // Assign it as the top (and currently only) item in the open list,
                                 // which is maintained as a binary heap (explained below)
  pathFinding->openListCoords[1].X = startCoord.X;
  pathFinding->openListCoords[1].Y = startCoord.Y;
  pathFinding->openListCoords[1].Z = startCoord.Z;
  // Capture the current sort-number for the first item, indexed by the (x,y,z) coordinate
  pathFinding->sortNumber[startCoord.X][startCoord.Y][startCoord.Z] = 1;

  #ifdef DEBUG_findPath
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) About to start Step 3 for path %d. numberOfOpenListItems = %d\n",
           omp_get_thread_num(), pathNum, numberOfOpenListItems);
  }
  #endif

  // Step 3: Do the following until a path is found or deemed nonexistent.
  do  {

    // Step 4: If the open list is not empty, take the first cell off of the list.
    //    This is the lowest F cost cell on the open list.
    if (numberOfOpenListItems != 0)  {

      // Step 5: Pop the first item off the open list.
      parentXval = pathFinding->openListCoords[pathFinding->openList[1]].X;
      parentYval = pathFinding->openListCoords[pathFinding->openList[1]].Y;
      parentZval = pathFinding->openListCoords[pathFinding->openList[1]].Z; // Record cell coordinates of the item

      pathFinding->whichList[parentXval][parentYval][parentZval] = onClosedList; // Add the item to the closed list
      pathFinding->sortNumber[parentXval][parentYval][parentZval] = 0;  // Since this cell is no longer on open list, change its sort number to zero.

      #ifdef DEBUG_findPath
      if (   DEBUG_CRITERIA_MET
          && (parentXval >= x_window_min) && (parentXval <= x_window_max)
          && (parentYval >= y_window_min) && (parentYval <= y_window_max)
          && (parentZval >= z_window_min) && (parentZval <= z_window_max))  {
        DEBUG_ON = TRUE;
      }
      else  {
        DEBUG_ON = FALSE;
      }
      #endif


      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Analyzing new parent cell at (%d,%d,%d) and also moving it to Closed List for path %d because it's at top of Open List (lowest F-cost).\n",
               omp_get_thread_num(), parentXval, parentYval, parentZval, pathNum);
      }
      #endif

      //  Open List = Binary Heap: Delete this item from the open list, which
      //  is maintained as a binary heap. For more information on binary heaps, see:
      //  http://www.policyalmanac.org/games/binaryHeaps.htm
      numberOfOpenListItems = numberOfOpenListItems - 1; //reduce number of open list items by 1

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) After removing (%d,%d,%d) from Open List, numberOfOpenListItems = %d\n", omp_get_thread_num(), parentXval, parentYval,
               parentZval, numberOfOpenListItems);
      }
      #endif

      //  Delete the top item in binary heap and reorder the heap, with the
      //  lowest F-cost item rising to the top.
      pathFinding->openList[1] = pathFinding->openList[numberOfOpenListItems+1]; // Move the last item in the heap up to slot #1
      pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[1]].X][pathFinding->openListCoords[pathFinding->openList[1]].Y][pathFinding->openListCoords[pathFinding->openList[1]].Z] = 1;

      v = 1;

      //  Repeat the following until the new item in slot #1 sinks to its proper spot in the heap.
      do  {
        u = v;
        if (2*u+1 <= numberOfOpenListItems) {  // if both children exist
          // Check if the F-cost of the parent is greater than each child.
          // Select the lower of the two children.
          if (pathFinding->Fcost[pathFinding->openList[u]] > pathFinding->Fcost[pathFinding->openList[2*u]])
            v = 2*u;
          if (pathFinding->Fcost[pathFinding->openList[v]] > pathFinding->Fcost[pathFinding->openList[2*u+1]])
            v = 2*u+1;
          }
        else {
          if (2*u <= numberOfOpenListItems)  {  // If only child #1 exists
            // Check if the F cost of the parent is greater than child #1
            if (pathFinding->Fcost[pathFinding->openList[u]] > pathFinding->Fcost[pathFinding->openList[2*u]])
              v = 2*u;
          }
        }  // End of else-clause

        if (u != v)  { // If parent's F is > one of its children, swap them

          // Swap the sort-numbers in the 'sortNumber[x][y][z]' array:
          temp = pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[u]].X][pathFinding->openListCoords[pathFinding->openList[u]].Y][pathFinding->openListCoords[pathFinding->openList[u]].Z];
          pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[u]].X][pathFinding->openListCoords[pathFinding->openList[u]].Y][pathFinding->openListCoords[pathFinding->openList[u]].Z]
                      = pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[v]].X][pathFinding->openListCoords[pathFinding->openList[v]].Y][pathFinding->openListCoords[pathFinding->openList[v]].Z];
          pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[v]].X][pathFinding->openListCoords[pathFinding->openList[v]].Y][pathFinding->openListCoords[pathFinding->openList[v]].Z] = temp;


          // Swap the ID numbers for sort numbers 'u' and 'v':
          temp = pathFinding->openList[u];
          pathFinding->openList[u] = pathFinding->openList[v];
          pathFinding->openList[v] = temp;
        }
        else
          break; // otherwise, exit loop

      }
      while (1);  // End of do-while block

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)     openList[1] = %'d after moving (%d,%d,%d) from  Open List to Closed List, and removing it from heap\n", omp_get_thread_num(),
               pathFinding->openList[1], parentXval, parentYval, parentZval);
        printf("DEBUG: (thread %2d)     Fcost[%'d] = %'lu at (%d,%d,%d) near top of findPath\n", omp_get_thread_num(), pathFinding->openList[1],
               pathFinding->Fcost[pathFinding->openList[1]],
               pathFinding->openListCoords[pathFinding->openList[1]].X,
               pathFinding->openListCoords[pathFinding->openList[1]].Y,
               pathFinding->openListCoords[pathFinding->openList[1]].Z);

        printf("DEBUG: (thread %2d) About to start Step 6 for path %d.\n", omp_get_thread_num(), pathNum);
        printf("DEBUG: (thread %2d)       (parentXval, parentYval, parentZval) = (%d,%d,%d)\n",
               omp_get_thread_num(), parentXval, parentYval, parentZval);
      }
      #endif

      // Get the allowed directions for routing from the parent cell. This information
      // is based on the design-rule number (location-specific) and design-rule subset
      // (net-specific):
      int parent_DR_num    = cellInfo[parentXval][parentYval][parentZval].designRuleSet;
      int parent_DR_subset = user_inputs->designRuleSubsetMap[pathNum][parent_DR_num];
      int parent_allowedRoutingDirections = user_inputs->designRules[parent_DR_num][parent_DR_subset].routeDirections;

      // Step 6: Check the 18 nearby 'children' cells. Add these nearby cells
      //    to the open list for later consideration if appropriate (see
      //    various if-statements below).
      //
      //  ---------------------------     XX = parent cell
      //   |    |NxNW|    |NxNE|    |     ?? = cells that will be checked (child cells)
      //   |    | ?? |    | ?? |    |
      //  ---------------------------
      //   |WxNW| NW | N  | NE |ExNE|
      //   | ?? | ?? | ?? | ?? | ?? |
      //  ---------------------------
      //   |    | W  | XX | E  |    |
      //   |    | ?? | XX | ?? |    |
      //  ---------------------------
      //   |WxSW| SW | S  | SE |ExSE|
      //   | ?? | ?? | ?? | ?? | ?? |
      //  ---------------------------
      //   |    |SxSW|    |SxSE|    |
      //   |    | ?? |    | ?? |    |
      //  ---------------------------
      //
      // Note the following rules related to pin-swappable regions. These rules guarantee that
      // paths enter/exit pin-swappable regions only in north/south/east/west directions:
      //   (a) If parent cell ('XX' above) is in a pin-swap zone, then algorithm only
      //       moves north, south, east, and west.
      //   (b) If child cell ('??' above) is in a pin-swap zone, it will only be checked
      //       if the parent cell ('XX') is north, south, east, or west of child cell.
      int deltaX, deltaY, deltaZ, target_deltaX, target_deltaY;  // Represents direction of (a,b,c) relative to (x,y,z) and target
      int direction_allowed;

      //
      // Iterate over all allowed transitions from the parent cell:
      //
      for (int i = 0; i < num_transitions; i++)  {

        // For each allowed transition, calculate the (a,b,c) coordinates of the child cell:
        int a = parentXval + allowedDeltaX[i];
        int b = parentYval + allowedDeltaY[i];
        int c = parentZval + allowedDeltaZ[i];

        // Skip this child cell if it's outside the map:
        if ((a < 0) || (b < 0) || (c < 0) || (a >= mapInfo->mapWidth) || (b >= mapInfo->mapHeight) || (c >= mapInfo->numLayers))  {
          continue;
        }

        // For each allowed transition, also calculate the absolute value of the delta-X, -Y, -Z:
        deltaX = abs(allowedDeltaX[i]);
        deltaY = abs(allowedDeltaY[i]);
        deltaZ = abs(allowedDeltaZ[i]);

        // For each allowed transition, calculate the distance to the end-terminal:
        target_deltaX = abs(a - endCoord.X);
        target_deltaY = abs(b - endCoord.Y);

        #ifdef DEBUG_findPath
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) ================ Checking child cell at (%d,%d,%d)... =======================\n", omp_get_thread_num(), a, b, c);
        }
        #endif

        // Get the allowed directions for routing *to* the child cell. This information
        // is based on the design-rule number (location-specific) and design-rule subset
        // (net-specific):
        int child_DR_num    = cellInfo[a][b][c].designRuleSet;
        int child_DR_subset = user_inputs->designRuleSubsetMap[pathNum][child_DR_num];
        int child_allowedRoutingDirections = user_inputs->designRules[child_DR_num][child_DR_subset].routeDirections;

        // Calculate the minimum allowed routing directions based on the allowed directions
        // in the parent cell and the child cell:
        int minimum_allowedRoutingDirections = calcMinimumAllowedDirection(parent_allowedRoutingDirections, child_allowedRoutingDirections);

        #ifdef DEBUG_findPath
        if (DEBUG_ON)  {
          char *route_direction_text; // String to hold routing direction
          route_direction_text = malloc(50 * sizeof(char)); // Allocate memory for routing description

          printf("DEBUG: (thread %2d) For path %d, parent cell (%d,%d,%d) and child cell (%d,%d,%d):\n", omp_get_thread_num(), pathNum,
                 parentXval, parentYval, parentZval, a, b, c);
          directionToText(parent_allowedRoutingDirections, route_direction_text);
          printf("DEBUG: (thread %2d)          Allowed parent direction = '%s' (%X)\n", omp_get_thread_num(), route_direction_text,
                 parent_allowedRoutingDirections);

          directionToText(child_allowedRoutingDirections, route_direction_text);
          printf("DEBUG: (thread %2d)           Allowed child direction = '%s' (%X)\n", omp_get_thread_num(), route_direction_text,
                 child_allowedRoutingDirections);

          directionToText(minimum_allowedRoutingDirections, route_direction_text);
          printf("DEBUG: (thread %2d)         Minimum allowed direction = '%s' (%X) \n", omp_get_thread_num(), route_direction_text,
                 minimum_allowedRoutingDirections);

          free(route_direction_text);  // Free memory allocated earlier in this block
          route_direction_text = NULL;
        }
        #endif

        // Confirm that cell at (a,b,c) is not a prohibited direction for routing from the parent cell,
        // based on the design-rules at the parent cell:
        direction_allowed = allowedDirection(deltaX, deltaY, deltaZ, minimum_allowedRoutingDirections);
        #ifdef DEBUG_findPath
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) For cell at (%d,%d,%d), direction_allowed is provisionally set to %d\n", omp_get_thread_num(),
                 a, b, c, direction_allowed);
        }
        #endif

        // Now check the rare case in which only X_ROUTING is allowed, and (a,b,c) is located adjacent to the target cell.
        // Without this check, the path-finder might never find its target. Use the '__builtin_expect' compiler directive to
        // tell the compiler to expect the result to be FALSE more often than TRUE:
        if (__builtin_expect((parent_allowedRoutingDirections == X_ROUTING) && (target_deltaX + target_deltaY == 1), FALSE))  {
          direction_allowed = allowedDirection(deltaX, deltaY, deltaZ, ANY);
          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)  Because X_ROUTING is used and (%d,%d,%d) is adjacent to target, direction_allowed is updated to %d\n",
                   omp_get_thread_num(), a, b, c, direction_allowed);
          }
          #endif
        }

        // Now check the rare case in which only X_ROUTING is allowed, and either (a,b,c) or the parent cell
        // is in a pin-swap zone. Use the '__builtin_expect' compiler directive to tell the compiler to expect the
        // result to be FALSE more often than TRUE:
        if (__builtin_expect((parent_allowedRoutingDirections == X_ROUTING)
            && (cellInfo[parentXval][parentYval][parentZval].swap_zone || cellInfo[a][b][c].swap_zone)
            && (deltaX + deltaY + deltaZ == 1), FALSE))  {
          direction_allowed = TRUE;
          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)  Because X_ROUTING is used and child cell (%d,%d,%d) and/or parent cell (%d,%d,%d) is in a swap-zone, direction_allowed is updated to %d\n",
                   omp_get_thread_num(), a, b, c, parentXval, parentYval, parentZval, direction_allowed);
          }
          #endif
        }

        if (! direction_allowed)  {
          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) *** Skipping cell at (%d,%d,%d) because it represents a prohibited transition from (%d,%d,%d)\n",
                   omp_get_thread_num(), a, b, c, parentXval, parentYval, parentZval);
          }
          #endif
          continue;
        }

        // Variable 'shapeType' describes the shape (TRACE, VIA_UP, or VIA_DOWN) that
        // connects location (a,b,c) to (parentXval, parentYval, parentZval):
        unsigned short shapeType = 999; // Initialized to a nonsensical value.

        // Don't check the cell located on layer above parent cell if any of the following are true:
        //   a. The via is blocked toward that cell, either from the parent cell or the child cell, or
        //   b. The child cell is in close proximity to a barrier or swap-zone, or
        //   c. The parent cell is in close proximity to a barrier or swap-zone.
        if ((allowedTransitions[i] == Up)
              && (   cellInfo[parentXval][parentYval][parentZval].forbiddenUpViaBarrier
                  || cellInfo[a][b][c].forbiddenDownViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_DOWN)
                  || get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_DOWN)
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, VIA_UP)
                  || get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, VIA_UP))) {

          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)  Skipping child cell (%d,%d,%d) because via is blocked up to that layer:\n", omp_get_thread_num(), a, b, c);
            printf("DEBUG: (thread %2d)                      forbiddenUpViaBarrier = %d at parent cell (%d,%d,%d)\n",
                   omp_get_thread_num(), cellInfo[parentXval][parentYval][parentZval].forbiddenUpViaBarrier, parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)    VIA_UP get_unwalkable_barrier_proximity_by_path = %d at parent cell (%d,%d,%d)\n",
                   omp_get_thread_num(), get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval,
                   pathNum, VIA_UP), parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)    VIA_UP get_unwalkable_pinSwap_proximity_by_path = %d at parent cell (%d,%d,%d)\n",
                   omp_get_thread_num(), get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval,
                   pathNum, VIA_UP), parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)                    forbiddenDownViaBarrier = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    cellInfo[a][b][c].forbiddenDownViaBarrier, a, b, c);
            printf("DEBUG: (thread %2d)  VIA_DOWN get_unwalkable_barrier_proximity_by_path = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_DOWN), a, b, c);
            printf("DEBUG: (thread %2d)  VIA_DOWN get_unwalkable_pinSwap_proximity_by_path = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_DOWN), a, b, c);
            printf("DEBUG: (thread %2d)                                  swap zone = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                    cellInfo[parentXval][parentYval][parentZval].swap_zone, parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)                                  swap zone = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    cellInfo[a][b][c].swap_zone, a, b, c);
          }
          #endif

          continue;
        }  // End of else/if-block for an 'Up' transition

        // Don't check the cell located on layer below parent cell if any of the following are true:
        //   a. The via is blocked toward that cell, either from the parent cell or the child cell, or
        //   b. The child cell is in close proximity to a barrier or swap-zone, or
        //   c. The parent cell is in close proximity to a barrier or swap-zone.
        else if ((allowedTransitions[i] == Down)
              && (   cellInfo[parentXval][parentYval][parentZval].forbiddenDownViaBarrier
                  || cellInfo[a][b][c].forbiddenUpViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_UP)
                  || get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_UP)
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, VIA_DOWN)
                  || get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, VIA_DOWN)))  {

          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)  Skipping child cell (%d,%d,%d) because via is blocked down to that layer:\n",
                   omp_get_thread_num(), a, b, c);
            printf("DEBUG: (thread %2d)                    forbiddenDownViaBarrier = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                    cellInfo[parentXval][parentYval][parentZval].forbiddenDownViaBarrier, parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)  VIA_DOWN get_unwalkable_barrier_proximity_by_path = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                    get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, VIA_DOWN),
                    parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)  VIA_DOWN get_unwalkable_pinSwap_proximity_by_path = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                    get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, VIA_DOWN),
                    parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)                      forbiddenUpViaBarrier = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    cellInfo[a][b][c].forbiddenUpViaBarrier, a, b, c);
            printf("DEBUG: (thread %2d)    VIA_UP get_unwalkable_barrier_proximity_by_path = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_UP), a, b, c);
            printf("DEBUG: (thread %2d)    VIA_UP get_unwalkable_pinSwap_proximity_by_path = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, VIA_UP), a, b, c);
            printf("DEBUG: (thread %2d)                                  swap zone = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                    cellInfo[parentXval][parentYval][parentZval].swap_zone, parentXval, parentYval, parentZval);
            printf("DEBUG: (thread %2d)                                  swap zone = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                    cellInfo[a][b][c].swap_zone, a, b, c);
          }
          #endif

          continue;
        }  // End of else/if-block for a 'Down' transition

        else  {
          // We got here, so the transition is neither 'Up' nor 'Down'. In other words, it's a
          // lateral transition. Don't check the child cell if any of the following are true:
          //   a. The child-cell is in a user-defined barrier, or
          //   b. The child cell is in close proximity to a barrier or swap-zone, and the parent
          //      cell is not in a proximity zone that is not part of a swap-zone. (In other words,
          //      we're getting too close to a barrier or swap zone.)
          if (   cellInfo[a][b][c].forbiddenTraceBarrier
              || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, TRACE)
              || (get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, TRACE)
                  && (! get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, TRACE))
                  && (! cellInfo[parentXval][parentYval][parentZval].swap_zone))) {

            #ifdef DEBUG_findPath
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d)  Skipping child cell (%d,%d,%d) because cell is blocked at that location:\n",
                     omp_get_thread_num(), a, b, c);
              printf("DEBUG: (thread %2d)                      forbiddenTraceBarrier = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                      cellInfo[a][b][c].forbiddenTraceBarrier, a, b, c);
              printf("DEBUG: (thread %2d)     TRACE get_unwalkable_barrier_proximity_by_path = %d at child cell (%d,%d,%d)\n",
                     omp_get_thread_num(), get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, TRACE), a, b, c);
              printf("DEBUG: (thread %2d)     TRACE get_unwalkable_barrier_proximity_by_path = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                     get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, TRACE),
                     parentXval, parentYval, parentZval);
              printf("DEBUG: (thread %2d)     TRACE get_unwalkable_pinSwap_proximity_by_path = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                      get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, a, b, c, pathNum, TRACE), a, b, c);
              printf("DEBUG: (thread %2d)     TRACE get_unwalkable_pinSwap_proximity_by_path = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                      get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, parentXval, parentYval, parentZval, pathNum, TRACE),
                      parentXval, parentYval, parentZval);
              printf("DEBUG: (thread %2d)                                  swap zone = %d at parent cell (%d,%d,%d)\n", omp_get_thread_num(),
                      cellInfo[parentXval][parentYval][parentZval].swap_zone, parentXval, parentYval, parentZval);
              printf("DEBUG: (thread %2d)                                  swap zone = %d at child cell (%d,%d,%d)\n", omp_get_thread_num(),
                      cellInfo[a][b][c].swap_zone, a, b, c);
            }
            #endif

            continue;
          }
        }  // End of else-block for a lateral transition

        // Don't check the cell if it's part of a pin-swappable zone, and the swap-zone
        // number of the zone does not match the current path's pin-swap zone:
        if ((cellInfo[a][b][c].swap_zone) && (cellInfo[a][b][c].swap_zone != pathSwapZone))  {
          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Skipping cell (%d, %d, %d) because it's in a pin-swappable zone (%d)\n",
                   omp_get_thread_num(), a, b, c, cellInfo[a][b][c].swap_zone);
            printf("DEBUG: (thread %2d)       that does not match the swap-zone number (%d) of path %d.\n",
                   omp_get_thread_num(), pathSwapZone, pathNum);
          }
          #endif

          continue;
        }  // End of if-bock for (swap_zone == TRUE && swap_zone != pathSwapZone)

        // If 'restrictionFlag' is TRUE and the cell is not in a swap-zone, then don't
        // check this cell if it's on a routing layer that is prohibited (based on
        // 'allowedLayers' value), or the cell is beyond a distance 'allowedRadiiCells'
        // from coordinate (centerX, centerY):
        if (routingRestrictions->restrictionFlag
             && (! cellInfo[a][b][c].swap_zone)
             && ((! routingRestrictions->allowedLayers[c])
                  || (   (deltaZ == 0)    // Current cell is on same layer as previous cell (it's not a via)
                      // Routing radius is not zero (which would imply an infinite radius):
                      && (routingRestrictions->allowedRadiiCells[c] > 0.1)
                      // Cell is farther from center-point than allowed radius:
                      && (calc_2D_Pythagorean_distance_ints(a, b, routingRestrictions->centerX, routingRestrictions->centerY) > routingRestrictions->allowedRadiiCells[c])) ))  {
          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Cell (%d,%d,%d) is subject to routing restrictions. Mid-point located at (%d,%d).\n",
                   omp_get_thread_num(), a, b, c, routingRestrictions->centerX, routingRestrictions->centerY);
            printf("DEBUG: (thread %2d)   Distance between cell and mid-point is %6.3f cells.\n", omp_get_thread_num(),
                    calc_2D_Pythagorean_distance_ints(a, b, routingRestrictions->centerX, routingRestrictions->centerY) * user_inputs->cell_size_um);
          }
          #endif

          continue;
        }
        else  {
          if (routingRestrictions->restrictionFlag)  {
            #ifdef DEBUG_findPath
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Cell (%d,%d,%d) is not subject to routing restrictions.\n", omp_get_thread_num(), a, b, c);
            }
            #endif
          }
        }

        //  If not already on the closed list (items on the closed list have
        //  already been considered and can now be ignored).
        if (pathFinding->whichList[a][b][c] != onClosedList)  {

          // Don't cut across corners: For each of the 16 neighbors within the 5x5
          // grid (on same routing layer), check that the path from the parent cell to the
          // child cell is walkable. If you have to cut across a corner that's unwalkable, then
          // set the variable 'corner' to a value of 'unwalkable'.
          corner = walkable;
          switch (allowedTransitions[i])  {
            case WxSW :
              // Check cell located at (-2, -1) relative to parent:
              if (  cellInfo[parentXval-1][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval-1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval-1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval, parentZval, pathNum, TRACE) )  {

                corner = unwalkable;
              }
              break;

            case WxNW :
              // Check cell located at (-2, +1) relative to parent:
              if (  cellInfo[parentXval-1][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval+1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval-1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval, parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case SW :
              // Check cell at location (-1,-1) relative to parent cell:
              if (  cellInfo[parentXval-1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval  , parentZval, pathNum, TRACE)
                 || cellInfo[parentXval  ][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval-1, parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case NW :
              // Check cell at location (-1,+1) relative to parent cell:
              if (  cellInfo[parentXval  ][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval+1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval-1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval  , parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case SxSW :
              // Check cell at location (-1, -2) relative to parent:
              if (  cellInfo[parentXval-1][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval-1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval  ][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval-1, parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case NxNW :
              // Check cell at location (-1, +2) relative to parent:
              if (  cellInfo[parentXval-1][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval-1, parentYval+1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval  ][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval+1, parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case SE :
              // Check 1 cell to lower right:
              if (  cellInfo[parentXval  ][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval-1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval+1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval  , parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case NE :
              // Check 1 cell to upper right:
              if (  cellInfo[parentXval+1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval  , parentZval, pathNum, TRACE)
                 || cellInfo[parentXval  ][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval+1, parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case SxSE :
              // Check cell at location (+1, -2) relative to parent:
              if (  cellInfo[parentXval+1][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval-1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval  ][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval-1, parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case NxNE :
              // Check cell at location (+1, +2) relative to parent:
              if (  cellInfo[parentXval+1][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval+1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval  ][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval  , parentYval+1, parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case ExSE :
              // Check cell located at (+2, -1) relative to parent:
              if (  cellInfo[parentXval+1][parentYval-1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval-1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval+1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval  , parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

            case ExNE :
              // Check cell located at (+2, +1) relative to parent:
              if (  cellInfo[parentXval+1][parentYval+1][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval+1, parentZval, pathNum, TRACE)
                 || cellInfo[parentXval+1][parentYval  ][parentZval].forbiddenTraceBarrier
                 || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, parentXval+1, parentYval  , parentZval, pathNum, TRACE))  {

                corner = unwalkable;
              }
              break;

          }  // End of switch-block


          // Now that we know whether the child cell is walkable from the parent cell, we also
          // must confirm that the parent and child cells satisfy the rules related to
          // pin-swap zones, namely:
          //   (a) If parent cell is in a pin-swap zone, then algorithm only checks child cells
          //       located north, south, east, and west of parent cell.
          //   (b) If child cell is in a pin-swap zone, it will only be checked if the parent
          //       cell is north, south, east, or west of child cell.
          swap_interface = walkable_swap_interface;  // Initialize 'swap_interface' to a walkable value
          if ((cellInfo[parentXval][parentYval][parentZval].swap_zone || cellInfo[a][b][c].swap_zone)
              && (deltaX + deltaY + deltaZ != 1))  {
            // The parent and child cells are oriented in a diagonal manner or via a knight's
            // move, and at least one of the cells is in a pin-swappable zone, so we flag
            // this child cell to be skipped.
            swap_interface = unwalkable_swap_interface;

            #ifdef DEBUG_findPath
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Child cell at (%d,%d,%d) will not be analyzed because its relationship to\n",
                     omp_get_thread_num(), a, b, c);
              printf("DEBUG: (thread %2d)       parent cell at (%d,%d,%d) is not north/south/east/west, and at least\n",
                     omp_get_thread_num(), parentXval, parentYval, parentZval);
              printf("DEBUG: (thread %2d)       one of the cells is in a pin-swappable region.\n", omp_get_thread_num());
            }
            #endif
          }  // End of if-block for pin-swappable parent cell and diagonal child cell


          if ((corner == walkable) && (swap_interface == walkable_swap_interface)) {
            #ifdef DEBUG_findPath
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Child cell at (%d,%d,%d) is walkable from (%d,%d,%d)\n", omp_get_thread_num(),
                     a, b, c, parentXval, parentYval, parentZval);
            }
            #endif

            // Calculate the shape-type of the *child* cell (TRACE, VIA_UP, or VIA_DOWN)
            // based on the geometric relationship between parentZval and c. Note that the
            // *child* cell is a via-down if we need to go up to reach it, and the child cell
            // is a via-up if we need to go down to reach it.
            if (allowedDeltaZ[i] == 0)
              shapeType = TRACE;
            else if (allowedDeltaZ[i] < 0)
              shapeType = VIA_UP;
            else
              shapeType = VIA_DOWN;

            //  If not already on the open list, add it to the open list.
            if (pathFinding->whichList[a][b][c] != onOpenList)  {

              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) Adding (%d,%d,%d) to open list since it was not previously on this list...\n",
                       omp_get_thread_num(), a, b, c);
                printf("DEBUG: (thread %2d)   parentXval=%d parentYval=%d parentZval=%d\n", omp_get_thread_num(),
                       parentXval, parentYval, parentZval);
              }
              #endif

              // Create a new open-list item in the binary heap.
              newOpenListItemID = newOpenListItemID + 1; // each new item has a unique ID #
              m = numberOfOpenListItems+1;
              pathFinding->openList[m] = newOpenListItemID; // place the new open list item (actually,
                                                 // its ID#) at the bottom of the heap
              pathFinding->openListCoords[newOpenListItemID].X = a;
              pathFinding->openListCoords[newOpenListItemID].Y = b;
              pathFinding->openListCoords[newOpenListItemID].Z = c; // record the x, y, and z coordinates of the new item

              // Add this open-list item's coordinates and current sort-number to the 'sortNumber' array:
              pathFinding->sortNumber[a][b][c] = m;

              // Calculate the congestion-related G-cost between the parent cell and the cell at (a,b,c):
              congestion_penalty = calc_congestion_penalty(a, b, c, parentXval, parentYval, parentZval, pathNum, shapeType,
                                                           cellInfo, user_inputs, mapInfo, FALSE, 0, recognizeSelfCongestion);


              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) findPath received a congestion_penalty of %'lu from first 'calc_congestion_penalty' between parent (%d,%d,%d) and child (%d,%d,%d)\n",
                       omp_get_thread_num(), congestion_penalty, parentXval, parentYval, parentZval, a, b, c);
              }
              #endif

              // Based on whether this net has been randomly selected for modifying, change the value of the congestion-related G-cost:
              if (random_reduction_flag)  {
                congestion_penalty = (long)(congestion_penalty * congestion_scale_factor);

                #ifdef DEBUG_findPath
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d) findPath changed the congestion_penalty to %'lu because random_reduction_flag = %d and congestion_scale_factor = %.2f\n",
                         omp_get_thread_num(), congestion_penalty, random_reduction_flag, congestion_scale_factor);
                }
                #endif
              }  // End of if-block for random_reduction_flag

              // Calculate the G-cost for child cell:
              unsigned long distance_G_cost = calc_distance_G_cost(a, b, c, parentXval, parentYval, parentZval, user_inputs, cellInfo, mapInfo, pathNum);
              pathFinding->Gcost[a][b][c]
                               = pathFinding->Gcost[parentXval][parentYval][parentZval]
                                 + distance_G_cost
                                 + congestion_penalty;

              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                 printf("DEBUG: (thread %2d) Calculated Gcost for child cell (%d,%d,%d) is %'lu, consisting of:\n", omp_get_thread_num(),
                        a, b, c, pathFinding->Gcost[a][b][c]);
                 printf("DEBUG: (thread %2d)        Parent's G-cost: %'lu\n", omp_get_thread_num(), pathFinding->Gcost[parentXval][parentYval][parentZval]);
                 printf("DEBUG: (thread %2d)        Distance G-cost: %'lu\n", omp_get_thread_num(), distance_G_cost);
                 printf("DEBUG: (thread %2d)      Congestion G-cost: %'lu\n", omp_get_thread_num(), congestion_penalty);
              }
              #endif

              // Check that G-cost has not exceeded the maximum allowed value of an unsigned long integer (2^64-1) by ensuring that
              // this value is larger than each of the three components that comprise it:
              if (   (pathFinding->Gcost[a][b][c] < pathFinding->Gcost[parentXval][parentYval][parentZval])
                  || (pathFinding->Gcost[a][b][c] < distance_G_cost)
                  || (pathFinding->Gcost[a][b][c] < congestion_penalty))  {

                printf("ERROR: An unexpected problem occurred. The variable 'Gcost' exceeded the maximum allowed value\n");
                printf("       for a 64-bit unsigned integer (%'lu) at cell (%d,%d,%d) for net number %d.\n", 0xFFFFFFFFFFFFFFFF-1, a, b, c, pathNum);
                printf("       This can be caused by exceptionally large designs, very long nets/traces, or large values for\n");
                printf("       parameter 'trace_cost_multiplier' in the input file. The value of Gcost is %'lu.\n\n", pathFinding->Gcost[a][b][c]);
                printf("       Please inform the software developer of this fatal error message. The program is exiting.\n\n");
                printf("       Diagnostic information: G-cost is the sum of these three variables\n");
                printf("               Parent G-cost = %'lu\n", pathFinding->Gcost[parentXval][parentYval][parentZval]);
                printf("             Distance G-cost = %'lu\n", distance_G_cost);
                printf("           Congestion G-cost = %'lu\n\n", congestion_penalty);
                exit(1);
              }


              //
              // If 'useDijkstra' is FALSE, then calculate the H-cost of child cell, based on distance to the target
              // cell:
              //
              if (! useDijkstra)  {
                #ifdef DEBUG_findPath
                if (DEBUG_ON)  {
                  printf("DEBUG: About to calculate H-cost for A* algorithm for (%d,%d,%d)\n", a, b, c);
                  printf("DEBUG:     endCoord = (%d,%d,%d)\n",
                         endCoord.X, endCoord.Y, endCoord.Z);
                }
                #endif

                pathFinding->Hcost[pathFinding->openList[m]] = calc_heuristic(a, b, c, endCoord.X, endCoord.Y, endCoord.Z,
                                                                              minimum_allowedRoutingDirections, user_inputs, cellInfo);

              }
              else  {
                // If 'useDijkstra' is TRUE, then set the H-cost to zero:
                pathFinding->Hcost[pathFinding->openList[m]] = 0.0;
              }

              //
              // Now that G and H costs are calculated, determine the F cost and parent:
              //
              pathFinding->Fcost[pathFinding->openList[m]] = pathFinding->Gcost[a][b][c] + pathFinding->Hcost[pathFinding->openList[m]];

              // Check that F-cost has not exceeded the maximum allowed value of a long, unsigned integer (2^64-1) by
              // confirming that its value is larger than each of the two components that comprise it:
              if (   (pathFinding->Fcost[pathFinding->openList[m]] < pathFinding->Gcost[a][b][c])
                  || (pathFinding->Fcost[pathFinding->openList[m]] < pathFinding->Hcost[pathFinding->openList[m]]))  {
                printf("\nERROR: An unexpected problem occurred. The variable 'Fcost' exceeded the maximum allowed value\n");
                printf(  "       for a 64-bit integer (%'lu) at cell (%d,%d,%d) for net number %d.\n", 0xFFFFFFFFFFFFFFFF-1, a, b, c, pathNum);
                printf(  "       This can be caused by exceptionally large designs, very long nets/traces, or large values for\n");
                printf(  "       parameter 'trace_cost_multiplier' in the input file. Diagnostic information follows:\n");
                printf(  "            Gcost = %'lu\n", pathFinding->Gcost[a][b][c]);
                printf(  "            Hcost = %'lu\n", pathFinding->Hcost[pathFinding->openList[m]]);
                printf(  "            Fcost = %'lu\n", pathFinding->Fcost[pathFinding->openList[m]]);
                printf(  "       Please inform the software developer of this fatal error message. The program is exiting.\n\n");
                exit(1);
              }

              // Define the parent X/Y/Z locations for the child cell at (a,b,c):
              pathFinding->parentCoords[a][b][c].X = parentXval;
              pathFinding->parentCoords[a][b][c].Y = parentYval;
              pathFinding->parentCoords[a][b][c].Z = parentZval;

              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) From parent cell (%d,%d,%d), child cell (%d,%d,%d)'s   G = %'lu   H = %'lu   F = %'lu   for path %d\n",
                       omp_get_thread_num(), parentXval, parentYval, parentZval, a, b, c, pathFinding->Gcost[a][b][c],
                       pathFinding->Hcost[pathFinding->openList[m]], pathFinding->Fcost[pathFinding->openList[m]], pathNum) ;
              }
              #endif

              // Move the new open-list item to the proper place in the binary heap.
              // Starting at the bottom, successively compare to parent items,
              // swapping as needed until the item finds its place in the heap
              // or bubbles all the way to the top (if it has the lowest F cost).
              while (m != 1)  {  // While item hasn't bubbled to the top (m==1)
                // Check if child's F cost is < parent's F cost. If so, swap them.
                if (pathFinding->Fcost[pathFinding->openList[m]] < pathFinding->Fcost[pathFinding->openList[m/2]])  {

                  // Swap 'sortNumber' elements:
                  temp = pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m/2]].X][pathFinding->openListCoords[pathFinding->openList[m/2]].Y][pathFinding->openListCoords[pathFinding->openList[m/2]].Z];
                  pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m/2]].X][pathFinding->openListCoords[pathFinding->openList[m/2]].Y][pathFinding->openListCoords[pathFinding->openList[m/2]].Z]
                    = pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m]].X][pathFinding->openListCoords[pathFinding->openList[m]].Y][pathFinding->openListCoords[pathFinding->openList[m]].Z];
                  pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m]].X][pathFinding->openListCoords[pathFinding->openList[m]].Y][pathFinding->openListCoords[pathFinding->openList[m]].Z] = temp;

                  // Swap 'openList' elements:
                  temp = pathFinding->openList[m/2];
                  pathFinding->openList[m/2] = pathFinding->openList[m];
                  pathFinding->openList[m] = temp;

                  // Cut 'm' in half and continue until m == 1:
                  m = m/2;

                }
                else
                  break;
              }  // End of while-loop for (m != 1)

              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d)     openList[1] = %'d after adding child cell (%d,%d,%d) to binary heap in middle of findPath (with parent (%d,%d,%d))\n",
                       omp_get_thread_num(), pathFinding->openList[1], a, b, c, parentXval, parentYval, parentZval);
              }
              #endif

              // Add one to the number of items in the heap
              numberOfOpenListItems = numberOfOpenListItems + 1;
              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d)     numberOfOpenListItems increased to %d\n", omp_get_thread_num(), numberOfOpenListItems);
              }
              #endif

              // Change whichList to show that the new item is on the open list.
              pathFinding->whichList[a][b][c] = onOpenList;

              // If the 'record_explored_cells' flag is 1 or 3, then set the 'explored' flag in 'cellInfo' data
              // structure to show that this cell was explored during the A* path-finding algorithm:
              if (record_explored_cells & 1)  {
                // Use the 'omp atomic' pragma to avoid writing to a memory location while another thread is writing to same location.
                // But this pragma prevents compilation on AWS ARM-based instance, as of 2 Dec 2021. And because we only write 'TRUE'
                // to this bit (never 'FALSE'), the lack of an 'atomic write' should not corrupt the memory.
//              #pragma omp atomic write
                cellInfo[a][b][c].explored = TRUE;

              }
              // If the 'record_explored_cells' flag is 2 or 3, then set the 'explored_PP' flag in 'cellInfo' data
              // structure to show that this cell was explored during post-processing:
              if (record_explored_cells & 2)  {
                // Use the 'omp atomic' pragma to avoid writing to a memory location while another thread is writing to same location.
                // But this pragma prevents compilation on AWS ARM-based instance, as of 2 Dec 2021. And because we only write 'TRUE'
                // to this bit (never 'FALSE'), the lack of an 'atomic write' should not corrupt the memory.
//              #pragma omp atomic write
                cellInfo[a][b][c].explored_PP = TRUE;

              }
            }  // End of if-block for (whichList[a][b][c] != onOpenList)

            // Step 7: If adjacent cell is already on the open list, check to see if this
            //   path to that cell from the starting location is a better one.
            //   If so, change the parent of the cell and its G and F costs.

            else  {     // If whichList(a,b,c) = onOpenList

              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) Child cell (%d,%d,%d) is already on open list.\n", omp_get_thread_num(), a, b, c);
              }
              #endif


              // Calculate the congestion-related G-cost between the parent cell and the cell at (a,b,c):
              congestion_penalty = calc_congestion_penalty(a, b, c, parentXval, parentYval, parentZval, pathNum, shapeType,
                                                           cellInfo, user_inputs, mapInfo, FALSE, 0, recognizeSelfCongestion);



              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) findPath received a congestion_penalty of %'lu from second 'calc_congestion_penalty' between parent (%d,%d,%d) and child (%d,%d,%d)\n",
                       omp_get_thread_num(), congestion_penalty, parentXval, parentYval, parentZval, a, b, c);
              }
              #endif


              // Based on whether this net has been randomly selected for modifying, change the value of the congestion-related G-cost:
              if (random_reduction_flag)  {
                congestion_penalty = (long)(congestion_penalty * congestion_scale_factor);
              }

              //
              // Calculate the total G-cost of child cell at (a,b,c) from the parent cell:
              //

              // Calculate G-cost for child cell:
              unsigned long distance_G_cost = calc_distance_G_cost(a, b, c, parentXval, parentYval, parentZval, user_inputs, cellInfo, mapInfo, pathNum);

              tempGcost = pathFinding->Gcost[parentXval][parentYval][parentZval]
                           + distance_G_cost
                           + congestion_penalty;

              #ifdef DEBUG_findPath
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d) Calculated tempGcost for child cell (%d,%d,%d) is %'lu, consisting of:\n", omp_get_thread_num(),
                       a, b, c, tempGcost);
                printf("DEBUG: (thread %2d)         Parent's G-cost: %'lu\n", omp_get_thread_num(), pathFinding->Gcost[parentXval][parentYval][parentZval]);
                printf("DEBUG: (thread %2d)         Distance G-cost: %'lu\n", omp_get_thread_num(), distance_G_cost);
                printf("DEBUG: (thread %2d)       Congestion G-cost: %'lu\n", omp_get_thread_num(), congestion_penalty);
              }
              #endif

              // Check that tempGcost has not exceeded the maximum allowed value of an unsigned long integer (2^64-1) by ensuring that
              // this value is larger than each of the three components that comprise it:
              if (   (tempGcost < pathFinding->Gcost[parentXval][parentYval][parentZval])
                  || (tempGcost < distance_G_cost)
                  || (tempGcost < congestion_penalty))  {

                printf("ERROR: An unexpected problem occurred. The variable 'tempGcost' exceeded the maximum allowed value\n");
                printf("       for a 64-bit unsigned integer (%'lu) at cell (%d,%d,%d) for net number %d.\n", 0xFFFFFFFFFFFFFFFF-1, a, b, c, pathNum);
                printf("       This can be caused by exceptionally large designs, very long nets/traces, or large values for\n");
                printf("       parameter 'trace_cost_multiplier' in the input file. The value of tempGcost is %'lu.\n\n", tempGcost);
                printf("       Please inform the software developer of this fatal error message. The program is exiting.\n\n");
                printf("       Diagnostic information: tempGcost is the sum of these three variables\n");
                printf("               Parent G-cost = %'lu\n", pathFinding->Gcost[parentXval][parentYval][parentZval]);
                printf("             Distance G-cost = %'lu\n", distance_G_cost);
                printf("           Congestion G-cost = %'lu\n\n", congestion_penalty);
                exit(1);
              }


              // If this path is shorter (G cost is lower), then change
              // the parent cell, G cost and F cost.
              if (tempGcost < pathFinding->Gcost[a][b][c])  {  // If G cost is less,

                #ifdef DEBUG_findPath
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d) A lower G-cost was found for cell (%d,%d,%d): %'ld (was %'ld), with new parent cell (%d,%d,%d).\n",
                         omp_get_thread_num(), a, b, c, tempGcost, pathFinding->Gcost[a][b][c], parentXval, parentYval, parentZval);
                  printf("DEBUG: (thread %2d)  Program will next look through the %d cells to find its location in the sorted binary heap.\n",
                         omp_get_thread_num(), numberOfOpenListItems);
                }
                #endif

                pathFinding->parentCoords[a][b][c].X = parentXval; // Change the cell's parent
                pathFinding->parentCoords[a][b][c].Y = parentYval;
                pathFinding->parentCoords[a][b][c].Z = parentZval;

                pathFinding->Gcost[a][b][c] = tempGcost;    // Change the G cost


                // Because changing the G cost also changes the F cost, and because this
                // cell is on the open list, we need to change the cell's recorded F-cost
                // and its position on the open list to make sure that we maintain a
                // properly ordered open list.
                m = pathFinding->sortNumber[a][b][c];

                pathFinding->Fcost[pathFinding->openList[m]] = pathFinding->Gcost[a][b][c] + pathFinding->Hcost[pathFinding->openList[m]]; // Change the F cost

                // Check that F-cost has not exceeded the maximum allowed value of a long, unsigned integer (2^64-1) by
                // confirming that its value is larger than each of the two components that comprise it:
                if (   (pathFinding->Fcost[pathFinding->openList[m]] < pathFinding->Gcost[a][b][c])
                    || (pathFinding->Fcost[pathFinding->openList[m]] < pathFinding->Hcost[pathFinding->openList[m]]))  {
                  printf("\nERROR: An unexpected problem occurred. The variable 'Fcost' exceeded the maximum allowed value\n");
                  printf(  "       for a 64-bit integer (%'lu) at cell (%d,%d,%d) for net number %d.\n", 0xFFFFFFFFFFFFFFFF-1, a, b, c, pathNum);
                  printf(  "       This can be caused by exceptionally large designs, very long nets/traces, or large values for\n");
                  printf(  "       parameter 'trace_cost_multiplier' in the input file. Diagnostic information follows:\n");
                  printf(  "            Gcost = %'lu\n", pathFinding->Gcost[a][b][c]);
                  printf(  "            Hcost = %'lu\n", pathFinding->Hcost[pathFinding->openList[m]]);
                  printf(  "            Fcost = %'lu\n", pathFinding->Fcost[pathFinding->openList[m]]);
                  printf(  "       Please inform the software developer of this fatal error message.  The program is exiting.\n\n");
                  exit(1);
                }

                // See if changing the F score bubbles the item up from
                // its current location in the heap
                while (m != 1) {  // While item hasn't bubbled to the top (m==1)
                  // Check if child is < parent. If so, swap them.
                  if (pathFinding->Fcost[pathFinding->openList[m]] < pathFinding->Fcost[pathFinding->openList[m/2]])  {

                    // Swap 'sortNumber' elements:
                    temp = pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m/2]].X][pathFinding->openListCoords[pathFinding->openList[m/2]].Y][pathFinding->openListCoords[pathFinding->openList[m/2]].Z];
                    pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m/2]].X][pathFinding->openListCoords[pathFinding->openList[m/2]].Y][pathFinding->openListCoords[pathFinding->openList[m/2]].Z]
                      = pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m]].X][pathFinding->openListCoords[pathFinding->openList[m]].Y][pathFinding->openListCoords[pathFinding->openList[m]].Z];
                    pathFinding->sortNumber[pathFinding->openListCoords[pathFinding->openList[m]].X][pathFinding->openListCoords[pathFinding->openList[m]].Y][pathFinding->openListCoords[pathFinding->openList[m]].Z] = temp;

                    // Swap 'openList' elements:
                    temp = pathFinding->openList[m/2];
                    pathFinding->openList[m/2] = pathFinding->openList[m];
                    pathFinding->openList[m] = temp;

                    // Cut 'm' in half and continue until m == 1:
                    m = m/2;
                  }  // End of if-block for (Fcost[openList[m]...
                  else
                    break;
                }   // End of while-loop for (m != 1)

                #ifdef DEBUG_findPath
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)     openList[1] = %'d at bottom of findPath, after calculating new G- and F-values for cell (%d,%d,%d)\n",
                         omp_get_thread_num(), pathFinding->openList[1], a, b, c);
                  printf("DEBUG: (thread %2d)     Fcost[%'d] = %'lu at (%d,%d,%d) at bottom of findPath\n", omp_get_thread_num(), pathFinding->openList[1],
                         pathFinding->Fcost[pathFinding->openList[1]], pathFinding->openListCoords[pathFinding->openList[1]].X,
                         pathFinding->openListCoords[pathFinding->openList[1]].Y, pathFinding->openListCoords[pathFinding->openList[1]].Z);
                }
                #endif

              }  // End of if-clause for (tempGcost < Gcost[a][b][c]...)
              else  {
                #ifdef DEBUG_findPath
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d) A lower G-cost was NOT found for cell (%d,%d,%d)\n", omp_get_thread_num(), a, b, c);
                }
                #endif
              }  // End of if/else-block for (tempGcost < Gcost[a][b][c]...)

            }  // End of else-block for (whichList[a][b][c] = onOpenList)
          } // End of if-block for (corner == walkable)
          else  {
            #ifdef DEBUG_findPath
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) corner cell at (%d,%d,%d) is NOT walkable\n", omp_get_thread_num(), a, b, c);
            }
            #endif
          }

        }  // End of if-block for (whichList[a][b][c] != onClosedList), i.e., if not already on the closed list
        else  {
          #ifdef DEBUG_findPath
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Not analyzing cell (%d,%d,%d) because it's already on Closed List.\n", omp_get_thread_num(), a, b, c);
          }
          #endif
        }  // End of if/else block for (whichList[a][b][c] != onClosedList)

      }  // End of for-loop for index 'i' (0 to num_transitions)

    }  // End of if-clause for (numberOfOpenListItems != 0)

    // Step 8: If open list is empty then there is no path.
    else  {
      path = nonexistent;

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) In Step 8, the open list is empty. Variable 'path' is assigned to %lu\n", omp_get_thread_num(), path);
      }
      #endif

      break;
    }


    #ifdef DEBUG_findPath
    if (DEBUG_ON)  {
      //
      // In DEBUG mode, print out the coordinates with the 10 lowest F-costs:
      //
      printf("\nDEBUG: (thread %2d) 10 lowest F-cost coordinates are:\n", omp_get_thread_num());
      for (int i_openList = 1; i_openList <= min(10, numberOfOpenListItems); i_openList++)  {
        printf("DEBUG: (thread %2d)   (%2d) (%d,%d,%d) with F-cost %'lu\n", omp_get_thread_num(), i_openList,
               pathFinding->openListCoords[pathFinding->openList[i_openList]].X, pathFinding->openListCoords[pathFinding->openList[i_openList]].Y,
               pathFinding->openListCoords[pathFinding->openList[i_openList]].Z, pathFinding->Fcost[pathFinding->openList[i_openList]]);
      }
    }
    #endif



    //
    // If cell with lowest F-cost is the target, then lowest-cost path has been found
    //
    if (   (pathFinding->openListCoords[pathFinding->openList[1]].X == endCoord.X)
        && (pathFinding->openListCoords[pathFinding->openList[1]].Y == endCoord.Y)
        && (pathFinding->openListCoords[pathFinding->openList[1]].Z == endCoord.Z))  {

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) We found the target cell (%d,%d,%d) on the Open List, with parent cell (%d,%d,%d).\n",
               omp_get_thread_num(), endCoord.X, endCoord.Y, endCoord.Z, pathFinding->parentCoords[endCoord.X][endCoord.Y][endCoord.Z].X,
               pathFinding->parentCoords[endCoord.X][endCoord.Y][endCoord.Z].Y, pathFinding->parentCoords[endCoord.X][endCoord.Y][endCoord.Z].Z);
        printf("DEBUG: (thread %2d) Lowest F-value is %'lu for cell (%d,%d,%d)\n", omp_get_thread_num(),
               pathFinding->Fcost[pathFinding->openList[1]], pathFinding->openListCoords[pathFinding->openList[1]].X,
               pathFinding->openListCoords[pathFinding->openList[1]].Y, pathFinding->openListCoords[pathFinding->openList[1]].Z);
      }
      #endif

      // We got here, so we found a path. Capture the G-cost, which
      // will be returned from this function:
      path = found;
      total_Gcost = pathFinding->Gcost[endCoord.X][endCoord.Y][endCoord.Z];

      break;  // Break out of the while-loop

    }  // End of if-block for target cell being found on Open List

  }  // End of do/while-block for finding a path or determining it's nonexistent
  while (1);  // Do until path is found or deemed nonexistent

  // printf("DEBUG: Finished do/while loop. Variable 'path' is %lu. About to start Step 9.\n", path);

  // Variable 'newOpenListItemID' contains the total number of cells that were explored during
  // this path-finding algorithm, including cells on the open list and the closed list. Capture
  // this value in the 'routability' structure to report to the user later on:
  routability->path_explored_cells[pathNum] = newOpenListItemID;

  #ifdef DEBUG_findPath
  if (DEBUG_CRITERIA_MET)  {
    DEBUG_ON = TRUE;
  }

  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) %'d cells were explored to find path # %d.\n", omp_get_thread_num(), newOpenListItemID, pathNum);
  }
  #endif


  // Step 9: Save the path if it exists.
  if (path != nonexistent)  {

    // a. Working backwards from the target to the starting location by checking
    //    each cell's parent, figure out the length of the path.
    pathX = endCoord.X;
    pathY = endCoord.Y;
    pathZ = endCoord.Z;
    do  {
      //Look up the parent of the current cell.
      tempx = pathFinding->parentCoords[pathX][pathY][pathZ].X;
      tempy = pathFinding->parentCoords[pathX][pathY][pathZ].Y;
      tempz = pathFinding->parentCoords[pathX][pathY][pathZ].Z;
      pathZ = tempz;
      pathY = tempy;
      pathX = tempx;

      //Figure out the path length
      *pathLength = *pathLength + 1;
    }  // End of do/while-loop
    while ((pathX != startCoord.X) || (pathY != startCoord.Y) || (pathZ != startCoord.Z));

    #ifdef DEBUG_findPath
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Completed 1st do/while loop in 'if (path == found)' block.\n", omp_get_thread_num());
      printf("DEBUG: (thread %2d)  pathNum = %d\n", omp_get_thread_num(), pathNum);
      // printf("DEBUG: (thread %2d)  pathCoords = %p\n", omp_get_thread_num(), pathCoords);
      printf("DEBUG: (thread %2d)  *pathLength = %d\n", omp_get_thread_num(), *pathLength);
      // printf("DEBUG: (thread %2d)  sizeof(*pathCoords) = %d\n", omp_get_thread_num(), (int)sizeof(*pathCoords));
    }
    #endif

    // b. Resize the data bank to the right size in bytes
    *pathCoords = realloc(*pathCoords, *pathLength * sizeof(Coordinate_t));
    if (*pathCoords == 0)  {
      printf("\nERROR: Failed to re-allocate memory for 'pathCoords' array.\n\n");
      exit(1);
    }

    // c. Now copy the path information over to the path coordinates array.
    //    Since we are working backwards from the target to the start location,
    //    we copy the information to the data bank in reverse order. The result
    //    is a properly ordered set of path data, from the first step to the last.
    pathX = endCoord.X;
    pathY = endCoord.Y;
    pathZ = endCoord.Z;
    cellPosition = *pathLength;  //start at the end
    #ifdef DEBUG_findPath
    if (DEBUG_ON)  {
      printf("\nDEBUG: (thread %2d) For path #%d ('%s'), target cell is (%d,%d,%d)\n", omp_get_thread_num(), pathNum, user_inputs->net_name[pathNum], pathX, pathY, pathZ);
      printf("DEBUG: (thread %2d) Entering do/while loop to determine path...\n", omp_get_thread_num());
    }
    #endif
    do  {
      cellPosition = cellPosition - 1;  // Work backwards by 1 cell position in the path
      (*pathCoords)[cellPosition].X    = pathX;
      (*pathCoords)[cellPosition].Y    = pathY;
      (*pathCoords)[cellPosition].Z    = pathZ;
      (*pathCoords)[cellPosition].flag = FALSE;

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) At end of function 'findPath' for path %d, segment %d = (%d,%d,%d)\n",
               omp_get_thread_num(), pathNum, cellPosition, (*pathCoords)[cellPosition].X, (*pathCoords)[cellPosition].Y, (*pathCoords)[cellPosition].Z);
      }
      #endif

      // d. Look up the parent of the current cell.
      tempx = pathFinding->parentCoords[pathX][pathY][pathZ].X;
      tempy = pathFinding->parentCoords[pathX][pathY][pathZ].Y;
      tempz = pathFinding->parentCoords[pathX][pathY][pathZ].Z;
      pathZ = tempz;
      pathY = tempy;
      pathX = tempx;

      #ifdef DEBUG_findPath
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)    Segment %d of path %d: (%d,%d,%d)\n", omp_get_thread_num(), cellPosition, pathNum, pathX, pathY, pathZ);
      }
      #endif

      // e. If we have reached the starting cell, exit the loop.
    }  // End of do/while-block
    while ((pathX != startCoord.X) || (pathY != startCoord.Y) || (pathZ != startCoord.Z));

    #ifdef DEBUG_findPath
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Finished Step 9.\n", omp_get_thread_num());
    }
    #endif

  }  // End of if-block for (path == found)

  // Calculate elapsed (wall-clock) time to find this path:
  if (record_elapsed_time)  {
    end = time(NULL);
    routability->path_elapsed_time[pathNum] = (int) (end - start);
    // printf("DEBUG: In iteration %d, path %d took %d seconds to find.\n", mapInfo->current_iteration, pathNum, routability->path_elapsed_time[pathNum]);
  }

  #ifdef DEBUG_findPath
  if (DEBUG_ON)  {
    // Return the G-cost of the full path:
    printf("DEBUG: (thread %2d) About to return from 'findPath()'. Gcost = %'lu. Number of segments = %d.\n\n",
           omp_get_thread_num(), total_Gcost, *pathLength);
  }
  #endif

  return(total_Gcost);

noPath:
  // 13. There is no path to the selected target

  #ifdef DEBUG_findPath
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Exiting function 'findPath' through the 'noPath' label.\n", omp_get_thread_num());
  }
  #endif

  // Calculate elapsed (wall-clock) time to find this path:
  if (record_elapsed_time)  {
    end = time(NULL);
    routability->path_elapsed_time[pathNum] = (int) (end - start);
  }

  printf("\nINFO: No path was found to route net #%d ('%s') after exploring %'d cells in %'d seconds.\n\n",
         pathNum, user_inputs->net_name[pathNum], newOpenListItemID, routability->path_elapsed_time[pathNum]);

  return(nonexistent);
}  // End of function 'findPath'


//-----------------------------------------------------------------------------
// Name: calcRoutabilityMetrics
// Desc: Perform design-rule-check (DRC), with results stored in 'DRC_details' and
//       'routability.' Function also calculates the path lengths and via counts
//       for each path, storing these in 'routability.' Function also updates the
//       'cellInfo' matrix with the locations of traces and vias, for use in generating/
//       displaying maps of the layouts. Function also updates the congestion
//       (in 'cellInfo') at each cell if the 'addCongestion' flag is set. This
//       congestion affects subsequent path-finding.
//
//       Path lengths are calculated based on the sparse (non-contiguous) paths.
//       Design-rule violations are calculated based on the contiguous paths.
//
//       If the Boolean flag 'exitIfInvalidJump' is TRUE, then this function will cause
//       program to exit if it detects an illegal jump between two adjacent segments.
//       Set this to FALSE for sub-maps, in which the start-terminals may be outside
//       of the sub-map's boundaries, and the path might exit and re-enter the sub-map.
//
//       If the Boolean flag 'beQuiet' is TRUE, then this function outputs nothing
//       to STDOUT. This can be useful when calculating routability metrics for
//       many small sub-maps. If 'parallelProcessing' is TRUE, then processing is
//       performed in multiple threads.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_routability' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_routability 1
#undef DEBUG_routability

void calcRoutabilityMetrics(const MapInfo_t *mapInfo, const int pathLength[],
                            Coordinate_t *pathCoords[], int contiguousPathLength[],
                            Coordinate_t *contigPathCoords[], RoutingMetrics_t *routability,
                            const InputValues_t *user_inputs, CellInfo_t ***cellInfo,
                            int addCongestionFlag, int addCongOnlyForDiffPair,
                            int exitIfInvalidJump, int beQuiet, int parallelProcessing)  {

  // Get the upper bound on the number of threads that could be used to form a new team if
  // 'omp parallel' construct were encountered without a num_threads clause:
  int num_threads = omp_get_max_threads();

  // Define the multiplier for calculating the additional congestion cost for a cell when
  // the cell contains design-rule violation(s). Zero implies no DRC-related congestion, and 1.0
  // implies the same amount of congestion as a trace crossing the cell.
  const float DRC_congestion_multiplier = 0.1;

  // Variable needed for debugging
  int DEBUG_ON = FALSE;

  #ifdef DEBUG_routability
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_X = 142;  // Set the (x,y,z) coordinates of point that you want to debug.
  int DEBUG_Y =  68;  // Set to (-1,-1,-1) if you don't want (x,y,z) debug output.
  int DEBUG_Z =   0;
  int DEBUG_Xprime = 143;  // Set the (Xprime, Yprime) coordinates of point that you want to debug (on same layer as DEBUG_Z).
  int DEBUG_Yprime =  99;  // Set to (-1,-1) if you don't want (x',y',z') debug output.
  if ((mapInfo->current_iteration >= 10) && (mapInfo->current_iteration <= 10))  {
    DEBUG_ON = TRUE;
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in calcRoutabilityMetrics() because specific requirements were met.\n", omp_get_thread_num());
    printf("DEBUG: (thread %2d) calcRoutabilityMetrics was entered with num_threads = %d and beQuiet = %d\n",
            omp_get_thread_num(), num_threads, beQuiet);
    printf("DEBUG: (thread %2d) Debug point (x,y,z) = (%d,%d,%d)      Debug point prime = (Xprime,Yprime) = (%d,%d)\n\n", omp_get_thread_num(),
           DEBUG_X, DEBUG_Y, DEBUG_Z, DEBUG_Xprime, DEBUG_Yprime);

  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  if (DEBUG_ON || ! beQuiet)  {
    printf("\nINFO: (thread %2d) Entered calcRoutabilityMetrics...\n\n", omp_get_thread_num());
  }

  // printf("\nDEBUG: (thread %2d) Entered calcRoutabilityMetrics with omp_get_num_threads = %d and omp_get_nested = %d\n",
  //        omp_get_thread_num(), omp_get_num_threads(), omp_get_nested());
  // printf(  "DEBUG: (thread %2d) and omp_get_max_threads = %d.\n\n", omp_get_thread_num(), omp_get_max_threads());

  // Calculate the small amount of 'congestion' to deposit in each cell that contains
  // a DRC violation. This causes such cells to have a slightly higher cost to traverse.
  // So if the autorouter later finds two paths that otherwise could have equal costs, it will
  // choose the one with no (or fewer) DRC cells:
  int DRC_congestion = (int)(ONE_TRAVERSAL * DRC_congestion_multiplier);

  int total_non_pseudo_DRC_count = 0; // Number of design-rule violations found in the entire map by all
                                      // threads, excluding pseudo-DRCs.





  //
  // Before branching into multiple threads, allocate memory for thread-specific variables:
  //
  // non_pseudo_DRC_count_per_thread[i] = Number of non-pseudo design-rule violations found by CPU thread 'i'.
  int *non_pseudo_DRC_count_per_thread;
  non_pseudo_DRC_count_per_thread = malloc(num_threads * sizeof(int));


////  // non_pseudo_DRC_count_per_thread[i] = Number of non-pseudo design-rule violations found by CPU thread 'i'.
////  int non_pseudo_DRC_count_per_thread[num_threads];

  // num_printed_DRCs_per_thread[i] = Number of DRC violations printed to STDOUT by thread 'i'
  short *num_printed_DRCs_per_thread;
  num_printed_DRCs_per_thread = malloc(num_threads * sizeof(short));

////  // num_printed_DRCs_per_thread[i] = Number of DRC violations printed to STDOUT by thread 'i'
////  short num_printed_DRCs_per_thread[num_threads];


  // Define 'DRC_details_per_thread' array that contains details of DRC violations
  // for the first N violations from thread 'i', with N = maxRecordedDRCs:
  DRC_details_t **DRC_details_per_thread;
  DRC_details_per_thread = malloc(num_threads * sizeof(DRC_details_t *));
  for (int i = 0; i < num_threads; i++)  {
    DRC_details_per_thread[i] = malloc(maxRecordedDRCs * sizeof(DRC_details_t));
  }

////  // Define 'DRC_details_per_thread' array that contains details of DRC violations
////  // for the first N violations from thread 'i', with N = maxRecordedDRCs:
////  DRC_details_t DRC_details_per_thread[num_threads][maxRecordedDRCs];


  // non_pseudo_via2via_DRC_count_per_thread[i] = Number of via-to-via spacing violations found by CPU thread 'i'.
  int *non_pseudo_via2via_DRC_count_per_thread;
  non_pseudo_via2via_DRC_count_per_thread = malloc(num_threads * sizeof(int));

////  // non_pseudo_via2via_DRC_count_per_thread[i] = Number of via-to-via spacing violations found by CPU thread 'i'.
////  int non_pseudo_via2via_DRC_count_per_thread[num_threads];

  // non_pseudo_trace2trace_DRC_count_per_thread[i] = Number of trace-to-trace spacing violations found by CPU thread 'i'.
  int *non_pseudo_trace2trace_DRC_count_per_thread;
  non_pseudo_trace2trace_DRC_count_per_thread = malloc(num_threads * sizeof(int));

////  // non_pseudo_trace2trace_DRC_count_per_thread[i] = Number of trace-to-trace spacing violations found by CPU thread 'i'.
////  int non_pseudo_trace2trace_DRC_count_per_thread[num_threads];

  // non_pseudo_trace2via_DRC_count_per_thread[i] = Number of trace-to-viae spacing violations found by CPU thread 'i'.
  int *non_pseudo_trace2via_DRC_count_per_thread;
  non_pseudo_trace2via_DRC_count_per_thread = malloc(num_threads * sizeof(int));

////  // non_pseudo_trace2via_DRC_count_per_thread[i] = Number of trace-to-viae spacing violations found by CPU thread 'i'.
////  int non_pseudo_trace2via_DRC_count_per_thread[num_threads];











  // Define the maximum number of DRC violations that will be printed out by
  // each thread:
  int maxPrintedDRCs_per_thread = (int) (maxPrintedDRCs / num_threads);

  // Calculate the total number of nets to analyze, including user-defined nets and
  // (if applicable) pseudo nets for differential pairs:
  int total_nets = mapInfo->numPaths + mapInfo->numPseudoPaths;

  // Calculate the maximum possible number of DRC interactions between all nets
  // and all shape-types, which will be used to dimension arrays:
  const int max_DRC_interactions = (total_nets * total_nets  -  total_nets) * NUM_SHAPE_TYPES * NUM_SHAPE_TYPES / 2;

  // Calculate the number of 8-bit bytes needed for max_DRC_interactions bits. Add 1 byte for safety.
  const int DRC_interaction_byte_length = max_DRC_interactions/8 + 1;
  #ifdef DEBUG_routability
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) DRC_interaction_byte_length=%d, because max_DRC_interactions=%d.\n", omp_get_thread_num(),
            DRC_interaction_byte_length, max_DRC_interactions);
    printf("DEBUG: (thread %2d) Entered calcRoutabilityMetrics. routability->total_num_DRC_cells is %d before re-initializing variables.\n",
           omp_get_thread_num(), (*routability).total_num_DRC_cells);
  }
  #endif

  //
  // Clear variables/arrays in the routability structure so they can be re-populated later on:
  //
  initializeRoutability(routability, mapInfo, FALSE);

  #ifdef DEBUG_routability
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) In calcRoutabilityMetrics, routability->total_num_DRC_cells is %d after re-initializing variables\n",
           omp_get_thread_num(), (*routability).total_num_DRC_cells);
  }
  #endif


  // For each path, calculate the fraction of DRC cells that it contains relative
  // to the entire map over the most recent 'numIterationsToReEquilibrate' iterations:
  {
    int sum_map_DRC_cells = 0;  // Number of DRC cells in entire map over 'numIterationsToReEquilibrate' iterations
    for (int path = 0; path < total_nets; path++)  {
      #ifdef DEBUG_routability
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) For path %d near beginning of calcRoutabilityMetrics:\n", omp_get_thread_num(), path);
      }
      #endif
      int sum_path_DRC_cells = 0;  // Number of DRC cells in current path over 'numIterationsToReEquilibrate' iterations
      for (int recent_iteration = 0; recent_iteration < numIterationsToReEquilibrate; recent_iteration++)  {
        sum_path_DRC_cells += routability->recent_path_DRC_cells[path][recent_iteration];
        sum_map_DRC_cells  += routability->recent_path_DRC_cells[path][recent_iteration];

        #ifdef DEBUG_routability
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      recent_path_DRC_cells[path=%d][recent_iteration=%d]= %d\n", omp_get_thread_num(),
                 path, recent_iteration, routability->recent_path_DRC_cells[path][recent_iteration]);
        }
        #endif
      }  // End of for-loop for index 'recent_iteration'
      routability->recent_path_DRC_fraction[path] = (float)sum_path_DRC_cells;
      #ifdef DEBUG_routability
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)    Preliminary recent_path_DRC_fraction[path=%d] = %.3f DRC cells. sum_map_DRC_cells = %'d\n",
               omp_get_thread_num(), path, routability->recent_path_DRC_fraction[path], sum_map_DRC_cells);
      }
      #endif
    }  // End of for-loop for index 'path' to calculate intermediate recent_path_DRC_fraction[] values

    #ifdef DEBUG_routability
    if (DEBUG_ON)  {
      printf("\nDEBUG: (thread %2d) Over latest %d iterations, sum_map_DRC_cells = %'d\n\n", omp_get_thread_num(),
             numIterationsToReEquilibrate, sum_map_DRC_cells);
    }
    #endif

    // Now divide the number of DRC cells in each path by the number of DRC cells in the entire map to
    // calculate a fraction. Also, calculate the amount of congestion to be added to each traversed cell:
    for (int path = 0; path < total_nets; path++)  {
      if (sum_map_DRC_cells > 0)  {
        routability->recent_path_DRC_fraction[path] = routability->recent_path_DRC_fraction[path] / sum_map_DRC_cells;
      }
      else  {
        routability->recent_path_DRC_fraction[path] = 0.0;
      }

      // Calculate the amount of congestion to deposit in each traversed cell for path 'path'. The
      // value includes a baseline value (ONE_TRAVERSAL) plus an amount that depends on the fraction
      // of DRC cells that this path contains.
      //
      // WARNING: The following line can result in oscillatory routing behavior, since the amount of congestion
      //          in the map will increase with DRCs, but then decrease again when the DRCs are resolved.

//// The following line was commented out 6/18/2024 and replaced with the subsequent line:
      routability->one_path_traversal[path]
              = (int)(ONE_TRAVERSAL * (1.0 + 2.0 * routability->recent_path_DRC_fraction[path]) );


//// The following assignment statement was commented out 6/22/2024 and replaced with the subsequent statement:
////  routability->one_path_traversal[path]
////          = (int)(ONE_TRAVERSAL * (1.0 + 10.0 * (1.0 - routability->fractionRecentIterationsWithoutPathDRCs[path])) );



////  routability->one_path_traversal[path]
////                  = (int)(ONE_TRAVERSAL * (1.0 + mapInfo->iterationDependentRatio * 10.0
////                                            * (1.0 - routability->fractionRecentIterationsWithoutPathDRCs[path])) );


////  if (mapInfo->current_iteration < 5)  {
////    routability->one_path_traversal[path] = ONE_TRAVERSAL;
////  }
////  else  {
////    routability->one_path_traversal[path]
////                    = (int)(ONE_TRAVERSAL * (1.0 + mapInfo->iterationDependentRatio * 2.0
////                                              * (1.0 - routability->fractionRecentIterationsWithoutPathDRCs[path])) );
////  }




      #ifdef DEBUG_routability
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Path %d contains %.1f percent of the DRC cells in the map over the last %d iterations.\n",
               omp_get_thread_num(), path, 100 * routability->recent_path_DRC_fraction[path], numIterationsToReEquilibrate);
        printf("DEBUG: (thread %2d) %.1f percent of path %d's recent iterations contained DRCs over the last %d iterations.\n",
               omp_get_thread_num(), 100.0 * (1.0 - routability->fractionRecentIterationsWithoutPathDRCs[path]), path,
               numIterationsToReEquilibrate);
        printf("DEBUG: (thread %2d)     Value of 'one_path_traversal[%d] = %d\n", omp_get_thread_num(), path,
               routability->one_path_traversal[path]);
      }
      #endif

    }  // End of for-loop for index 'path' to calculate final recent_path_DRC_fraction[] values
  }  // End of block to calculate recent_path_DRC_fraction[] values

  //
  // Calculate path-specific metrics like path length, via count, etc:
  //
  calcPathMetrics(total_nets, user_inputs, mapInfo, pathLength, pathCoords, cellInfo, routability, exitIfInvalidJump);

  //
  // In anticipation of design-rule checking, use the contiguous path array to mark the x/y/z locations
  // of every path and via. The path-number and shape-type are stored in each cell that the path traverses.
  //
  markPathCenterlinesInMap(total_nets, contiguousPathLength, contigPathCoords, cellInfo, mapInfo, routability, user_inputs);

  //
  // In anticipation of design-rule checking, flag cells that are near the centers of paths,
  // so we can avoid other cells when checking design rules. This function also adds extra
  // congestion at the path-center cells, thereby repelling foreign nets from crossing
  // the path-center cells.
  //
  markCellsNearCenterlinesInMap(total_nets, mapInfo, contiguousPathLength, contigPathCoords, user_inputs, cellInfo);


  // Before forking into multiple threads, initialize the DRC count that
  // each thread will report back:
  for (int i = 0; i < num_threads; i++)  {
    non_pseudo_DRC_count_per_thread[i] = 0;
    num_printed_DRCs_per_thread[i]     = 0;  // Number of DRC violations printed to STDOUT per thread
    non_pseudo_via2via_DRC_count_per_thread[i]     = 0;
    non_pseudo_trace2trace_DRC_count_per_thread[i] = 0;
    non_pseudo_trace2via_DRC_count_per_thread[i]   = 0;
  }  // End of for-loop for index 'i'


  //
  // Visit each (x,y,z) location in the map to determine whether it violates
  // any design rules, and whether to add congestion to the cell.
  //
  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);
  if (DEBUG_ON || ! beQuiet)  {
    printf("INFO: (thread %2d) About to check for DRC violations by rastering over all (x,y,z) locations, starting at %02d-%02d-%d, %02d:%02d:%02d.\n",
           omp_get_thread_num(), now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  }

  for (int z = 0; z < mapInfo->numLayers; z++)  {

    tim = time(NULL); now = localtime(&tim);
    if (DEBUG_ON || ! beQuiet)  {
      printf("\nINFO: (thread %2d) Started checking layer #%d of %d ('%s') for design-rule violations after iteration %d at %02d-%02d-%d, %02d:%02d:%02d.\n",
             omp_get_thread_num(), z, mapInfo->numLayers - 1, user_inputs->routingLayerNames[z], mapInfo->current_iteration,
             now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
    }

    // For current routing layer, get the maximum radius of interaction, so that we can limit
    // our calculations to cells close to nets.
    const int interaction_radius = mapInfo->maxInteractionRadiusCellsOnLayer[z];
    const int radius_squared     = mapInfo->maxInteractionRadiusSquaredOnLayer[z] ;
    #ifdef DEBUG_routability
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) interaction_radius = maxInteractionRadiusCellsOnLayer[%d] = %d\n", omp_get_thread_num(),
             z, interaction_radius);
      printf("DEBUG: (thread %2d)     radius_squared = maxInteractionRadiusSquaredOnLayer[%d] = %d\n",
             omp_get_thread_num(), z, radius_squared);
    }
    #endif


    //
    // Collapse the nested loops for indices y and x to apply parallel threading:
    //
    #pragma omp parallel for if (parallelProcessing) collapse(2) schedule(dynamic, 1)
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      for (int x = 0; x < mapInfo->mapWidth; x++)  {

        #ifdef DEBUG_routability
        // DEBUG code follows:
        //
        // Check if the input parameters satisfy specific requirements. If so, then set the
        // 'FOUND_XYZ' flag to TRUE so that this function will print out additional debug information:
        int FOUND_XYZ;
        if (DEBUG_ON && (DEBUG_X >= 0) && (x == DEBUG_X) && (DEBUG_Y >= 0) && (y == DEBUG_Y) && (DEBUG_Z >= 0) && (z == DEBUG_Z))  {
          printf("\n\nDEBUG: (thread %2d) For point (%d,%d,%d), setting FOUND_XYZ to TRUE in calcRoutabilityMetrics() because specific requirements were met.\n\n",
                 omp_get_thread_num(), x, y, z);
          FOUND_XYZ = TRUE;
        }  // End of if-block for determining whether to set DEBUG_ON to TRUE
        else  {
          FOUND_XYZ = FALSE;
        }
        #endif

        // If the current (x,y,z) location is not near a net, then
        // there's no reason to check for design-rule violations.
        // In such cases, continue on to the next (x,y,z) location:
        if (cellInfo[x][y][z].near_a_net)  {

          // We got here, so the cell at (x,y,z) is within an 'interaction distance' of a path's
          // centerline, based on the design rules used on the layer 'z'.

          // Get the thread number used for this (x,y,z) location:
          const short current_thread = omp_get_thread_num();

          // printf("  DEBUG: (thread %2d) Analyzing location (%d, %d, %d) for DRCs in parallel-processing part of code, with omp_get_num_threads = %d and omp_get_nested = %d...\n",
          //        omp_get_thread_num(), x, y, z, omp_get_num_threads(), omp_get_nested());


          // For the current (x,y,z) coordinate, get the number of the design-rule set and
          // the number of subsets within this design-rule set:
          const short DR_num_at_source = cellInfo[x][y][z].designRuleSet;
          const short num_source_DR_subsets = user_inputs->numDesignRuleSubsets[DR_num_at_source];

          // Calculate the product of the design-rule subsets (num_DR_subsets) and the
          // number of shape-types (3, or NUM_SHAPE_TYPES).  This product is used for
          // dimensioning arrays that we use for design-rule checking and congestion.
          const short num_subset_shapeType_indices = num_source_DR_subsets * NUM_SHAPE_TYPES;

          // Calculate the product of the number of nets and the number of shape-types
          // (3, or NUM_SHAPE_TYPES).  This product is used for dimensioning arrays
          // that we use for design-rule checking and congestion.
          const short num_path_shapeType_indices = total_nets * NUM_SHAPE_TYPES;


          // Create the 2-dimensional interaction matrix 'interaction_count[m,n]'
          // that will count the number of interacting nets at a
          // given (x,y,z) location. Interacting nets are those where the
          // perimeter of one shape is separated from the center of an adjacent
          // shape by a distance of < DRC_radius, i.e., spacing + half-width.
          // The indices m and n both vary from 0 to (num_DR_subsets * NUM_SHAPE_TYPES - 1),
          // and represent all possible combinations of design-rule subsets and
          // shape-types. The value of interaction_count[m,n] represents the
          // number of UNIQUE nets that interact between designRuleSubset/shape-type 'm'
          // and designRuleSubset/shape-type 'n' at a given (x,y,z) location.
          short interaction_count[num_subset_shapeType_indices][num_subset_shapeType_indices];

          // Create the 3D matrix 'interacting_nets[m,n,p]' that will track the
          // path numbers and shape-types that interact with a given (x,y,z) position.
          // The indices m and n represent all possible combinations of design-rule
          // sub-types and shape-types. Index 'p' varies from 0 to up to a maximum
          // of maxNets * NUM_SHAPE_TYPES, and lists the unique path numbers and
          // shape-types that interact between designRuleSubset/shape-type m and
          // designRuleSubset/shape-type n at a given location.
          Path_DR_Shape_Info_t interacting_nets[num_subset_shapeType_indices][num_subset_shapeType_indices][num_path_shapeType_indices];

          // Create the 2-dimensional matrix 'congestion_count[m,n]' that
          // will count the number of nets whose congestion must be added to
          // location (x,y,z). These nets are those whose center-line (or center)
          // is separated from the center of an adjacent shape by a distance
          // of <= cong_radius, i.e., half-width of 1st shape + spacing + half-
          // width of 2nd shape. The indices m and n both vary from 0 to
          // (num_DR_subsets * NUM_SHAPE_TYPES - 1), and represent all possible
          // combinations of design-rule sub-types and shape-types. The value of
          // congestion_count[m,n] represents the number of UNIQUE nets whose congestion must be
          // added to (x,y,z) with designRuleSubset/shape-type 'm' and
          // designRuleSubset/shape-type 'n'.
          short congestion_count[num_subset_shapeType_indices][num_subset_shapeType_indices];

          // Create the 3D matrix 'congestion_nets[m,n,p]' that will track the
          // path numbers whose congestion must be added to a given (x,y,z) position.
          // The indices m and n represent all possible combinations of design-rule
          // sub-types and shape-types. Index 'p' varies from 0 to up to a maximum
          // of total_nets, and lists the unique path numbers.
          short congestion_nets[num_subset_shapeType_indices][num_subset_shapeType_indices][total_nets];

          // Define 1-dimensional array that lists the unique path-numbers, design-rule numbers,
          // and shape-types that overlap a given (x,y,z) location.
          Path_DR_Shape_Info_t overlapping_nets[num_path_shapeType_indices];


          // Create and initialize a 1-dimensional array of 1-bit Boolean flags disguised
          // as type 'char' to save memory. Each element (bit) in the array corresponds
          // to a unique combination of these 4 variables: path_1, shapeType_1,
          // path_2, and shapeType_2. This function sets a bit equals 1 if it finds a
          // DRC violation between path_1/shapeType_1 and path_2/shapeType_2.
          char current_cell_DRCs[DRC_interaction_byte_length];
          // printf("DEBUG: Dimensioned 'current_cell_DRCs[]' array to a length of %d.\n", DRC_interaction_byte_length);
          for (int i = 0; i < DRC_interaction_byte_length; i++)  {
            current_cell_DRCs[i] = 0;  // Initialize each element to zero.
            // printf("DEBUG: Initialized current_cell_DRCs[%d] to zero.\n", i);
          }


          // Clear the variable num_overlapping_nets, which is the number of
          // UNIQUE path numbers and shape-types whose shapes overlap location (x,y,z).
          int num_overlapping_nets = 0;


          // Clear the 2D matrices 'interaction_count[m][n]' and
          // 'congestion_count[m][n]' for this (x,y,z) position. The indices m and n
          // both vary from 0 to (num_DR_subsets * NUM_SHAPE_TYPES - 1),
          // and represent all possible combinations of design-rule sub-types and
          // shape-types.
          for (int m = 0; m < num_subset_shapeType_indices; m++)  {
            for (int n = 0; n < num_subset_shapeType_indices; n++)  {
              interaction_count[m][n] = 0;
              congestion_count[m][n]  = 0;
            }  // End of for-loop for index 'n'
          }  // End of for-loop for index 'm'

          //
          // Explore the cells around current location (x,y,z) by rastering over a box
          // with sides +/- interaction_radius.
          //
          #ifdef DEBUG_routability
          if (FOUND_XYZ)  {
            printf("  DEBUG: (thread %2d) About to raster around (%d, %d, %d) by +/-%d...\n", omp_get_thread_num(), x, y, z, interaction_radius);
          }
          #endif
          for (int x_prime = x - interaction_radius; x_prime <= x + interaction_radius; x_prime++)  {
            const int delta_x_squared = (x - x_prime)  *  (x - x_prime);
            for (int y_prime = y - interaction_radius; y_prime <= y + interaction_radius; y_prime++)  {
              #ifdef DEBUG_routability
              int FOUND_XprimeYprimeZprime;
              if (DEBUG_ON && FOUND_XYZ && (DEBUG_Xprime >= 0) && (x_prime == DEBUG_Xprime) && (DEBUG_Yprime >= 0) && (y_prime == DEBUG_Yprime))  {
                printf("    DEBUG: (thread %2d) Setting FOUND_XprimeYprimeZprime to TRUE because (x',y',z) = (%d,%d,%d), with (x,y,z) = (%d,%d,%d).\n",
                       omp_get_thread_num(), x_prime, y_prime, z, x, y, z);
                FOUND_XprimeYprimeZprime = TRUE;
              }
              else  {
                FOUND_XprimeYprimeZprime = FALSE;
              }
              #endif

              // Calculate square of distance between (x,y) and (x', y'):
              const int distance_squared = delta_x_squared    +    (y - y_prime) * (y - y_prime);

              // If (x_prime, y_prime) is outside of the interaction_radius, then move on to
              // next (x_prime, y_prime) location.
              if (distance_squared > radius_squared)  {

                // printf("      DEBUG: Skipping (x',y',z) = (%d, %d, %d) because it's beyond %d cells from (%d,%d,%d).\n",
                //         x_prime, y_prime, z, interaction_radius, z, y, z);
                continue;
              }

              // If (x_prime, y_prime) is outside of the map, then move on to
              // next (x_prime, y_prime) location. Use the '__builtin_expect' compiler
              // directive to tell the compiler to expect the result to be FALSE more often than TRUE:
              if (__builtin_expect( ((x_prime < 0) || (x_prime >= mapInfo->mapWidth) ||
                  (y_prime < 0) || (y_prime >= mapInfo->mapHeight)), 0 ))  {
                // printf("      DEBUG: Skipping (x',y',z) = (%d, %d, %d) because it's outside of the map.\n", x_prime, y_prime, z);
                continue;
              }


              // If cell (x',y',z) is in pin-swappable zone, then move on to the next
              // (x,y,z) location. Use the '__builtin_expect' compiler directive to tell
              // the compiler to expect the 'swap_zone' to be zero more often than TRUE:
              if (__builtin_expect(cellInfo[x_prime][y_prime][z].swap_zone, 0))  {
                // printf("      DEBUG: Skipping (x',y',z) = (%d, %d, %d) because it's in a pin-swap zone.\n", x_prime, y_prime, z);
                continue;
              }


              // Calculate routability metrics for (x, y)'s interaction with (x_prime, y_prime).
              // First, get number of paths whose centers traverse (x_prime, y_prime, z):
              const int numTraversingPaths = cellInfo[x_prime][y_prime][z].numTraversingPathCenters;

              #ifdef DEBUG_routability
              if ((FOUND_XprimeYprimeZprime) && (numTraversingPaths > 0))  {
                printf("      DEBUG: (thread %2d) At (%d, %d, %d), found %d traversing paths.\n", omp_get_thread_num(), x_prime, y_prime, z, numTraversingPaths);
              }
              #endif

              // Get the design-rule number at the target location (x',y',z):
              const short DR_num_at_target = cellInfo[x_prime][y_prime][z].designRuleSet;

              // For each path & shape-type found at (x',y',z), check whether it
              // interacts with parent location (x,y,z). If so, add it to the appropriate
              // array or matrix:
              for (int i = 0; i < numTraversingPaths; i++)  {

                // Get path number and shape-type of path whose center is at (x',y',z):
                const short path_number     = cellInfo[x_prime][y_prime][z].pathCenters[i].pathNum;
                const short path_shape_type = cellInfo[x_prime][y_prime][z].pathCenters[i].shapeType;

                // Get the design-rule subset number for this net at the source cell (x,y,z) and target cell (x',y',z):
                const short DR_subset_at_source = user_inputs->designRuleSubsetMap[path_number][DR_num_at_source];
                const short DR_subset_at_target = user_inputs->designRuleSubsetMap[path_number][DR_num_at_target];

                // Calculate the index used for matrices 'congestion_count' and 'congestion_nets'.
                // The index is based on the values of DR_subset and shape_type, and is
                // simply 3 * DR_subset  +  shape_type. Note that the DR_subset is mapped back into
                // the design-rule set at (x,y,z), and not at the target cell at (x',y',z):
                const short subset_shapeType_index_at_target = DR_subset_at_target * NUM_SHAPE_TYPES   +   path_shape_type;
                const short subset_shapeType_index_at_source = DR_subset_at_source * NUM_SHAPE_TYPES   +   path_shape_type;

                #ifdef DEBUG_routability
                if (FOUND_XprimeYprimeZprime)  {
                  printf("      DEBUG: (thread %2d) Traversing path %d at (x',y',z) = (%d,%d,%d) has pathNum %d and shapeType %d. (x,y,z) = (%d,%d,%d).\n",
                         omp_get_thread_num(), i, x_prime, y_prime, z, path_number, path_shape_type, x, y, z);
                }
                #endif

                //
                // For the shape-type of the path at (x',y',z), check whether it's
                // within one half-width (or radius) of (x,y,z). If so, then add
                // the path number and shape type to the 'overlapping_nets' array.
                //
                if (distance_squared <= user_inputs->designRules[DR_num_at_target][DR_subset_at_target].radius_squared[path_shape_type])  {

                  // printf("        DEBUG: Traversing path %d at (%d,%d,%d) with shape-type %d is within 1 (target) radius of (%d, %d, %d). Target DR set is #%d; Target DR subset is #%d.\n",
                  //         target_path_number, x_prime, y_prime, z, target_shape_type, x, y, z, target_DR_num, target_DR_subset);

                  // If target_path_number & target_shape_type are not already in array overlapping_nets, then:
                  //   (a) add target_path_number and target_shape_type to array overlapping_nets,
                  //   (b) increment the variable 'num_overlapping_nets'
                  int already_in_array = FALSE;
                  for (int j = 0; j < num_overlapping_nets; j++)  {
                    if ((path_number == overlapping_nets[j].pathNum)
                        && (path_shape_type == overlapping_nets[j].shapeType))  {
                      already_in_array = TRUE;
                      break;  // We found path_number and shape_type already in array, so exit the
                              // array after setting 'already_in_array' flag.
                    }  // End of if-block for path_number and shape_type matching an array element
                  }  // End of for-loop for index 'j'

                  // If we didn't find path_number and shape_type in the array, add it now:
                  if (! already_in_array)  {
                    overlapping_nets[num_overlapping_nets].pathNum   = path_number;
                    overlapping_nets[num_overlapping_nets].shapeType = path_shape_type;
                    // printf("DEBUG: overlapping_nets[%d].shape_type = %d at (%d,%d,%d), traversing path index %d\n",
                    //         num_overlapping_nets, target_shape_type, x_prime, y_prime, z, i);
                    num_overlapping_nets++;
                    // printf("DEBUG: Added path #%d (shape-type %d) to overlapping_nets array at (%d, %d, %d).\n",
                    //         target_path_number, target_shape_type, x, y, z);
                  }  // End of if-block for (! already_in_array)
                }  // End of if-block for distance^2 < radius^2

                //
                // Iterate over each shape-type and design-rule subset that's valid for the design-
                // rule set at (x,y,z) to process the 2D matrix 'congestion_count', the 3D matrix
                // 'congestion_nets', the 2D matrix 'interaction_count' and the 3D matrix
                // 'interacting_nets'. The indices m and n range from zero to num_subset_shapeType_indices
                // (= num_source_DR_subsets * NUM_SHAPE_TYPES) for the cell at (x,y,z).
                //
                for (int m_DR_subset = 0; m_DR_subset < num_source_DR_subsets; m_DR_subset++)  {

                  // If this subset is not used by any nets, then continue on to the next subset:
                  if (! user_inputs->DR_subsetUsed[DR_num_at_source][m_DR_subset])  {
                    continue;
                  }

                  for (int m_shape_type = 0; m_shape_type < NUM_SHAPE_TYPES; m_shape_type++)  {

                    // Calculate the index used for matrices 'congestion_count' and 'congestion_nets'.
                    // The index is based on the values of m_DR_subset and m_shape_type, and is
                    // simply 3 * m_DR_subset  +  m_shape_type:
                    const short m_subset_shapeType_index = m_DR_subset * NUM_SHAPE_TYPES   +   m_shape_type;

                    #ifdef DEBUG_routability
                    if (FOUND_XprimeYprimeZprime)  {
                      printf("          DEBUG: (thread %2d) m_DR_subset = %d, m_shape_type = %d, m_subset_shapeType_index = %d\n",
                             omp_get_thread_num(), m_DR_subset, m_shape_type, m_subset_shapeType_index);
                    }
                    #endif

                    //
                    // To the 'congestion_count' and 'congestion_nets' matrices, add the
                    // current 'path_number' if the distance from (x,y,z) to (x',y',z) is
                    // within cong_radius[DR_num_A][m][DR_num_B][n], for m = subset_shapeType_index
                    // and n = subset_shapeType_index_at_source. Because we do not add congestion if
                    // path_number is that of a pseudo-net, we also do not add pseudo-nets
                    // to the 'congestion_count' and 'congestion_nets' matrices.
                    //

                    if ((   (! user_inputs->isPseudoNet[path_number])
                         || (mapInfo->addPseudoTraceCongestionNearVias[path_number][z]))
                          && ((distance_squared <=  user_inputs->cong_radius_squared[DR_num_at_source][m_subset_shapeType_index][DR_num_at_target][subset_shapeType_index_at_target])
                                || (distance_squared == 0))  )  {



                      // We got here, so current net should be part of the array:
                      //   congestion_nets[m_subset_shapeType_index][target_subset_shapeType_index]:
                      int already_in_array = FALSE;
                      for (int p = 0; p < congestion_count[m_subset_shapeType_index][subset_shapeType_index_at_source]; p++)  {
                        if (path_number == congestion_nets[m_subset_shapeType_index][subset_shapeType_index_at_source][p])  {
                          already_in_array = TRUE;
                          break;  // Break out of loop because we found target_path_number in array
                        }  // End of if-block for target_path_number = array element
                      }  // End of for-loop for index 'p'

                      // Add target_path_number to array if it wasn't found there already:
                      if (! already_in_array)  {

                        #ifdef DEBUG_routability
                        if (FOUND_XprimeYprimeZprime)  {
                          printf("            DEBUG: (thread %2d) Adding path #%d to congestion_nets[m=%d][n=%d][count=%d] at (x,y,z) = (%d,%d,%d)\n",
                                omp_get_thread_num(), path_number, m_subset_shapeType_index, subset_shapeType_index_at_target,
                                congestion_count[m_subset_shapeType_index][subset_shapeType_index_at_target], x, y, z);
                          printf("            DEBUG: (thread %2d) because the distance^2 between this point and (%d,%d,%d) (%d) is less than the\n",
                                 omp_get_thread_num(), x_prime, y_prime, z, distance_squared);
                          printf("            DEBUG: (thread %2d) cong_radius_squared[DR=%d][m=%d][DR=%d][n=%d], which is %.3f\n\n",
                                 omp_get_thread_num(), DR_num_at_source, m_subset_shapeType_index, DR_num_at_target,
                                 subset_shapeType_index_at_target, user_inputs->cong_radius_squared[DR_num_at_source][m_subset_shapeType_index][DR_num_at_target][subset_shapeType_index_at_target]);
                        }
                        #endif

                        congestion_nets[m_subset_shapeType_index][subset_shapeType_index_at_source][ congestion_count[m_subset_shapeType_index][subset_shapeType_index_at_source] ] = path_number;
                        congestion_count[m_subset_shapeType_index][subset_shapeType_index_at_source]++;  // Increment number of nets in array
                      }
                    }  // End of if-block for distance^2 < cong_radius^2
                    else  {
                      #ifdef DEBUG_routability
                      if (FOUND_XprimeYprimeZprime)  {
                        printf("            DEBUG: (thread %2d) NOT adding path #%d to congestion_nets[m=%d][n=%d][count=%d] at (x,y,z) = (%d,%d,%d)\n",
                               omp_get_thread_num(), path_number, m_subset_shapeType_index, subset_shapeType_index_at_target,
                               congestion_count[m_subset_shapeType_index][subset_shapeType_index_at_target], x, y, z);
                        printf("            DEBUG: (thread %2d) because the distance^2 between this point and (%d,%d,%d) (%d) is greater than the\n",
                               omp_get_thread_num(), x_prime, y_prime, z, distance_squared);
                        printf("            DEBUG: (thread %2d) cong_radius_squared[DR=%d][m=%d][DR=%d][n=%d], which is %.3f\n\n",
                               omp_get_thread_num(), DR_num_at_source, m_subset_shapeType_index, DR_num_at_target,
                               subset_shapeType_index_at_target, user_inputs->cong_radius_squared[DR_num_at_source][m_subset_shapeType_index][DR_num_at_target][subset_shapeType_index_at_target]);

                      }
                      #endif
                    }  // End of else-block: distance^2 not less than cong_radius^2


                    //
                    // Check for DRC violations. For each of the array-elements in the interaction_count
                    // (m x n) matrix and interacting_nets (m x n) matrix, check whether location
                    // (x,y,z) is within distance DRC_radius[m][n]. If so, then add the path
                    // number and shape type to the array associated with element (m,n), if the
                    // net is not already in the array.
                    //
                    for (int n_DR_subset = 0; n_DR_subset < num_source_DR_subsets; n_DR_subset++)  {

                      // If this subset is not used by any nets in the map, then continue on to next subset:
                      if (! user_inputs->DR_subsetUsed[DR_num_at_source][n_DR_subset])  {
                        continue;
                      }

                      for (int n_shape_type = 0; n_shape_type < NUM_SHAPE_TYPES; n_shape_type++)  {

                        // Calculate the index used for matrices 'DRC_count' and 'DRC_nets'.
                        // The index is based on the values of n_DR_subset and n_shape_type, and is
                        // simply 3 * n_DR_subset  +  n_shape_type:
                        const short n_subset_shapeType_index = n_DR_subset * NUM_SHAPE_TYPES   +   n_shape_type;

                        #ifdef DEBUG_routability
                        if (FOUND_XprimeYprimeZprime)  {
                          printf("              DEBUG: (thread %2d) n_DR_subset = %d, n_shape_type = %d, n_subset_shapeType_index = %d\n",
                                 omp_get_thread_num(), n_DR_subset, n_shape_type, n_subset_shapeType_index);
                        }
                        #endif

                        //
                        // 'interaction_count' and 'interacting_nets' matrices:
                        //
                        // Check if distance_squared is zero or is less than DRC_radius_squared:
                        //
                        if ((distance_squared < user_inputs->DRC_radius_squared[DR_num_at_source][m_subset_shapeType_index][DR_num_at_target][subset_shapeType_index_at_target])
                             || (distance_squared == 0))  {

                          //
                          // We got here, so current net should be inserted into array interacting_nets[m][n], if
                          // it's not already in the array:
                          //
                          // Check if current net, design-rule set, and shape-type are already in the array:
                          int already_in_array = FALSE;
                          for (int p = 0; p < interaction_count[m_subset_shapeType_index][n_subset_shapeType_index]; p++)  {
                            if (     (     path_number == interacting_nets[m_subset_shapeType_index][n_subset_shapeType_index][p].pathNum)
                                  && (DR_num_at_target == interacting_nets[m_subset_shapeType_index][n_subset_shapeType_index][p].DR_num)
                                  && ( path_shape_type == interacting_nets[m_subset_shapeType_index][n_subset_shapeType_index][p].shapeType))  {
                              already_in_array = TRUE;
                              break;  // Break out of loop because we found path_number in array
                            }  // End of if-block for path_number = array element
                          }  // End of for-loop for index 'p'

                          // Add path_number and shape_type to array if it wasn't found there already:
                          if (! already_in_array)  {

                            #ifdef DEBUG_routability
                            if (FOUND_XprimeYprimeZprime)  {
                              printf("DEBUG:          (thread %2d) Adding path #%d in design-rule set #%d (shape #%d) to interacting_nets[m=%d][n=%d][count=%d] (%d,%d,%d)\n",
                                     omp_get_thread_num(), path_number, DR_num_at_target, path_shape_type, m_subset_shapeType_index, n_subset_shapeType_index,
                                     interaction_count[m_subset_shapeType_index][n_subset_shapeType_index], x, y, z);
                              printf("DEBUG:          (thread %2d) because distance_squared (%d) between (%d,%d,%d) and (%d,%d,%d) is less than\n",
                                     omp_get_thread_num(), distance_squared, x, y, z, x_prime, y_prime, z);
                              printf("DEBUG:          (thread %2d)  DRC_radius_squared[%d][%d][%d][%d] (%.3f).\n", omp_get_thread_num(), DR_num_at_source,
                                     m_subset_shapeType_index, DR_num_at_target, subset_shapeType_index_at_target,
                                     user_inputs->DRC_radius_squared[DR_num_at_source][m_subset_shapeType_index][DR_num_at_target][subset_shapeType_index_at_target]);
                            }
                            #endif
                            interacting_nets[m_subset_shapeType_index][n_subset_shapeType_index][ interaction_count[m_subset_shapeType_index][n_subset_shapeType_index] ].pathNum   = path_number;
                            interacting_nets[m_subset_shapeType_index][n_subset_shapeType_index][ interaction_count[m_subset_shapeType_index][n_subset_shapeType_index] ].DR_num    = DR_num_at_target;
                            interacting_nets[m_subset_shapeType_index][n_subset_shapeType_index][ interaction_count[m_subset_shapeType_index][n_subset_shapeType_index] ].shapeType = path_shape_type;
                            interaction_count[m_subset_shapeType_index][n_subset_shapeType_index]++;  // Increment number of nets in array
                          }  // End of if-block for (! already_in_array)
                        }  // End of if-block for (distance^2 < DRC_radius^2) or (distance^2 == 0)
                        #ifdef DEBUG_routability
                        else  {
                          if (FOUND_XprimeYprimeZprime)  {
                            printf("DEBUG:          (thread %2d) NOT adding path #%d in design-rule set #%d (shape #%d) to interacting_nets[m=%d][n=%d][count=%d] (%d,%d,%d)\n",
                                   omp_get_thread_num(), path_number, DR_num_at_target, path_shape_type, m_subset_shapeType_index, n_subset_shapeType_index,
                                   interaction_count[m_subset_shapeType_index][n_subset_shapeType_index], x, y, z);
                            printf("DEBUG:          (thread %2d) because distance_squared (%d) between (%d,%d,%d) and (%d,%d,%d) is greater than\n",
                                   omp_get_thread_num(), distance_squared, x, y, z, x_prime, y_prime, z);
                            printf("DEBUG:          (thread %2d)  DRC_radius_squared[%d][%d][%d][%d] (%.3f).\n", DR_num_at_source, m_subset_shapeType_index,
                                   omp_get_thread_num(), DR_num_at_target, subset_shapeType_index_at_target,
                                   user_inputs->DRC_radius_squared[DR_num_at_source][m_subset_shapeType_index][DR_num_at_target][subset_shapeType_index_at_target]);
                          }
                        }  // End of if/else-block (for debug-purposes only)
                        #endif
                      }  // End of for-loop for index 'n_shape_type'
                    }  // End of for-loop for index 'n_DR_subset'

                  }  // End of for-loop for index 'm_shape_type'
                }  // End of for-loop for index 'm_DR_subset'
              }  // End of for-loop for index i (0 to numTraversingPaths)
            }  // End of for-loop for index y_prime
          }  // End of for-loop for index x_prime

          #ifdef DEBUG_routability
          if (FOUND_XYZ)  {
            printf("  DEBUG: (thread %2d) Done rastering around (%d, %d, %d) by +/-%d...\n", omp_get_thread_num(), x, y, z, interaction_radius);
            printf("  DEBUG: (thread %2d) num_overlapping_nets = %d at (%d, %d, %d).\n", omp_get_thread_num(), num_overlapping_nets, x, y, z);
          }
          #endif

          // Set flags in the cellInfo matrix that determine how the PNG map
          // will appear for this (x,y,z) location. Later on, some of these values
          // may be changed if we determine there are DRC violations.
          if (! cellInfo[x][y][z].swap_zone)  {  // Disregard cell if it's in a pin-swappable zone
            for (int i = 0; i < num_overlapping_nets; i++)  {
              #ifdef DEBUG_routability
              if (FOUND_XYZ)  {
                printf("    DEBUG: (thread %2d) overlapping_nets[%d].pathNum   = %d.\n", omp_get_thread_num(), i, overlapping_nets[i].pathNum);
                printf("    DEBUG: (thread %2d) overlapping_nets[%d].shapeType = %d.\n", omp_get_thread_num(), i, overlapping_nets[i].shapeType);
              }
              #endif
              if (   (overlapping_nets[i].shapeType == TRACE)
                  || (overlapping_nets[i].shapeType == VIA_UP)
                  || (overlapping_nets[i].shapeType == VIA_DOWN))  {

                // Set the appropriate cellInfo bit for routing layers, depending on whether
                // the net is a pseudo-net or a non-pseudo-net:
                if (user_inputs->isPseudoNet[overlapping_nets[i].pathNum])  {
                  cellInfo[x][y][z].pseudo_routing_layer_metal_fill = TRUE;
                }
                else  {
                  cellInfo[x][y][z].routing_layer_metal_fill = TRUE;
                }
              }  // End of if-block for shapeType == TRACE

              // Set the appropriate cellInfo bit for up-vias, depending on
              // whether the net is a pseudo-net or a non-pseudo-net:
              if (overlapping_nets[i].shapeType == VIA_UP)  {
                if (user_inputs->isPseudoNet[overlapping_nets[i].pathNum])  {
                  cellInfo[x][y][z].pseudo_via_above_metal_fill = TRUE;
                }
                else  {
                  cellInfo[x][y][z].via_above_metal_fill = TRUE;
                }
              }  // End of if-block for shapeType == VIA_UP

              // Set the appropriate cellInfo bit for down-vias, depending on
              // whether the net is a pseudo-net or a non-pseudo-net:
              if (overlapping_nets[i].shapeType == VIA_DOWN)  {
                if (user_inputs->isPseudoNet[overlapping_nets[i].pathNum])  {
                  cellInfo[x][y][z].pseudo_via_below_metal_fill = TRUE;
                }
                else  {
                    cellInfo[x][y][z].via_below_metal_fill = TRUE;
                }
              }  // End of if-block for shapeType == VIA_UP

            }  // End of for-loop for index 'i' (0 to num_overlapping_nets)
          }  // End of if-block for (! swap_zone)

          //
          // Debug code follows.
          //
          #ifdef DEBUG_routability
          if (FOUND_XYZ)  {
            printf("DEBUG: (thread %2d) --------------------------------------------------------------------------------------------\n", omp_get_thread_num());
            printf("DEBUG: (thread %2d) congestion_count[m][n] at (%d,%d,%d):\n", omp_get_thread_num(), x, y, z);
            printf("DEBUG: (thread %2d)         |", omp_get_thread_num());
            for (int m = 0; m < num_subset_shapeType_indices; m++)
              printf(" m = %2d |", m);
            printf("\n");
            printf("DEBUG: (thread %2d)         |", omp_get_thread_num());
            for (int m = 0; m < num_subset_shapeType_indices; m++)
              printf("--------|");
            printf("\n");
            for (int n = 0; n < num_subset_shapeType_indices; n++)  {
              printf("DEBUG: (thread %2d) n = %3d |", omp_get_thread_num(), n);
              for (int m = 0; m < num_subset_shapeType_indices; m++)  {
                printf("  %3d   |",  congestion_count[m][n]);
              }  // End of for-loop for index 'm'
              printf("\n");
            }  // End of for-loop for index 'n'
            printf("DEBUG: (thread %2d)        -", omp_get_thread_num());
            for (int m = 0; m < num_subset_shapeType_indices; m++)
              printf("---------");
            printf("\n");
            printf("DEBUG: (thread %2d) --------------------------------------------------------------------------------------------\n", omp_get_thread_num());

            printf("\n");

            printf("DEBUG: (thread %2d) --------------------------------------------------------------------------------------------\n", omp_get_thread_num());
            printf("DEBUG: (thread %2d) interaction_count[m][n] at (%d,%d,%d):\n", omp_get_thread_num(), x, y, z);
            printf("DEBUG: (thread %2d)         |", omp_get_thread_num());
            for (int m = 0; m < num_subset_shapeType_indices; m++)
              printf(" m = %2d |", m);
            printf("\n");
            printf("DEBUG: (thread %2d)         |", omp_get_thread_num());
            for (int m = 0; m < num_subset_shapeType_indices; m++)
              printf("--------|");
            printf("\n");
            for (int n = 0; n < num_subset_shapeType_indices; n++)  {
              printf("DEBUG: (thread %2d) n = %3d |", omp_get_thread_num(), n);
              for (int m = 0; m < num_subset_shapeType_indices; m++)  {
                printf("  %3d   |",  interaction_count[m][n]);
              }  // End of for-loop for index 'm'
              printf("\n");
            }  // End of for-loop for index 'n'
            printf("DEBUG: (thread %2d)        -", omp_get_thread_num());
            for (int m = 0; m < num_subset_shapeType_indices; m++)
              printf("---------");
            printf("\n");
            printf("DEBUG: (thread %2d) --------------------------------------------------------------------------------------------\n", omp_get_thread_num());
          }
          #endif
          //
          // End of debug code.
          //

          //
          // If (x,y,z) is not in a pin-swappable zone, then check for DRC violations:
          //
          if (! cellInfo[x][y][z].swap_zone)  {
            //
            // Now that we've determined which nets and shape-types are in the
            // vicinity of (x,y,z), we can determine whether location (x,y,z)
            // represents a DRC violation. A violation exists if both of these
            // criteria are met:
            //
            //  (1) overlapping_nets array contains pathNum i with design-rule subset/shape-type
            //      index j, and
            //  (2) interacting_nets[j,k] contains a net with the following attributes:
            //         (a) design-rule subset/shape-type equal to k, and
            //         (b) path number is NOT equal to i, and
            //         (c) path number is not a diff-pair net while the offending path is a pseudo-net, and
            //         (d) path number is not a pseudo-net while the offending path is a diff-pair net.
            //
            #ifdef DEBUG_routability
            if (FOUND_XYZ)  {
              printf("    DEBUG: (thread %2d) Cell (%d, %d, %d) has %d overlapping nets/shapeTypes.\n", omp_get_thread_num(), x, y, z, num_overlapping_nets);
            }
            #endif
            for (int i = 0; i < num_overlapping_nets; i++)  {
              const short path_number = overlapping_nets[i].pathNum;
              const short shape_type  = overlapping_nets[i].shapeType;
              #ifdef DEBUG_routability
              if (FOUND_XYZ)  {
                printf("      DEBUG: (thread %2d) Overlapping net/shapeType #%d has path number %d and shape-type %d.\n",
                       omp_get_thread_num(), i, path_number, shape_type);
              }
              #endif

              // Get the design-rule subset number for this path number:
              const short DR_subset = user_inputs->designRuleSubsetMap[path_number][DR_num_at_source];

              // Calculate the index used for matrices 'interaction_count' and 'interaction_nets'.
              // The index is based on the values of DR_subset and shape_type, and is
              // simply 3 * DR_subset  +  shape_type:
              const short subset_shapeType_index = DR_subset * NUM_SHAPE_TYPES   +   shape_type;


              // Visit each of the combinations of design-rule subset and target shape-types:
              #ifdef DEBUG_routability
              if (FOUND_XYZ)  {
                printf("      DEBUG: (thread %2d) About to visit all combinations of DR subsets and shape-types...\n", omp_get_thread_num());
              }
              #endif
              for (int target_subset_shapetype = 0; target_subset_shapetype < num_subset_shapeType_indices; target_subset_shapetype++)  {
                #ifdef DEBUG_routability
                if (FOUND_XYZ)  {
                  printf("        DEBUG: (thread %2d) Visiting target subset/shapeType index %d...\n", omp_get_thread_num(), target_subset_shapetype);
                  printf("        DEBUG: (thread %2d) Found %d interacting paths between subset/shape_type %d and target subset/shape_type %d.\n",
                         omp_get_thread_num(), interaction_count[subset_shapeType_index][target_subset_shapetype], subset_shapeType_index,
                         target_subset_shapetype);
                }
                #endif


                // For interactions at (x,y,z) between 'subset_shapeType_index' and 'target_subset_shapetype',
                // visit each of the interacting paths:
                for (int path_index = 0; path_index < interaction_count[subset_shapeType_index][target_subset_shapetype]; path_index++)  {
                  // Get the attributes of the interacting path. Note that we map the design-rule subset number back into the design-rule
                  // set of the source cell (x,y,z).
                  const short interacting_path_num            = interacting_nets[subset_shapeType_index][target_subset_shapetype][path_index].pathNum;
                  const short interacting_DR_num              = interacting_nets[subset_shapeType_index][target_subset_shapetype][path_index].DR_num;
                  const short interacting_shape_type          = interacting_nets[subset_shapeType_index][target_subset_shapetype][path_index].shapeType;
                  const short interacting_DR_subset_at_source = user_inputs->designRuleSubsetMap[interacting_path_num][DR_num_at_source];
                  const short interacting_DR_subset_at_target = user_inputs->designRuleSubsetMap[interacting_path_num][interacting_DR_num];

                  // Calculate the matrix index associated with the interacting design-rule subset and shape-type.
                  // This index is simply 3 * DR_subset  +  shape_type:
                  const short interacting_subset_shapeType = interacting_DR_subset_at_source * NUM_SHAPE_TYPES   +   interacting_shape_type;

                  // Calculate the subset/shapeType index at the target location, which is used for getting the correct DRC_radius:
                  const short interacting_subset_shapeType_at_target = interacting_DR_subset_at_target * NUM_SHAPE_TYPES + interacting_shape_type;

                  #ifdef DEBUG_routability
                  if (FOUND_XYZ)  {
                    printf("          DEBUG: (thread %2d) Interacting path-index #%d has path number %d, design-rule subset %d at source, DR subset %d at target, shape type %d,\n",
                           omp_get_thread_num(), path_index, interacting_path_num, interacting_DR_subset_at_source, interacting_DR_subset_at_target, interacting_shape_type);
                    printf("          DEBUG: (thread %2d)  interacting_subset_shapeType = %d, interacting_subset_shapeType_at_target = %d.\n", omp_get_thread_num(),
                           interacting_subset_shapeType, interacting_subset_shapeType_at_target);
                  }
                  #endif

                  //
                  // DRCs are categorized into normal DRCs and 'pseudo-DRCs', per the following table:
                  //                         --------------------------------------------------
                  //                        |    Normal Net    | Diff-Pair Net |   Pseudo-Net  |
                  //     -------------------|------------------|---------------|---------------|
                  //     |       Normal Net |       DRC        |      DRC      |  Pseudo-DRC   |
                  //     |------------------|------------------|---------------|---------------|
                  //     |    Diff-Pair Net |       DRC        |      DRC      |   Not a DRC   |
                  //     |------------------|------------------|---------------|---------------|
                  //     |       Pseudo-Net |    Pseudo-DRC    |   Not a DRC   |  Pseudo-DRC   |
                  //      ----------------------------------------------------------------------

                  // Determine whether the two nets consist of a pseudo-net and diff-pair net:
                  int diffPair_pseuoNet_combination = (   (user_inputs->isDiffPair[path_number]          && user_inputs->isPseudoNet[interacting_path_num])
                                                       || (user_inputs->isDiffPair[interacting_path_num] && user_inputs->isPseudoNet[path_number]));

                  // Check if the cell satisfies the criteria for a design-rule violation:
                  //         (a) path number is NOT equal to the i (interacting_path_num), and
                  //         (b) design-rule subset/shape-type equal to k, and
                  //         (c) path number is not a diff-pair net while the offending path is a pseudo-net, and
                  //         (d) path number is not a pseudo-net while the offending path is a diff-pair net.
                  if (   (interacting_path_num != path_number)                      // Item (a) above
                      && (interacting_subset_shapeType == target_subset_shapetype)  // Item (b) above
                      && (! diffPair_pseuoNet_combination ))                        // Items (c) and (d) above
                  {

                    #ifdef DEBUG_routability
                    if (FOUND_XYZ)  {
                      printf("            DEBUG: (thread %2d) DRC violation at (%d,%d,%d) in DR set #%d because interacting_path_num (%d) in DR set %d is different than path_number (%d),\n",
                             omp_get_thread_num(), x, y, z, DR_num_at_source, interacting_path_num, interacting_DR_num, path_number);
                      printf("            DEBUG: (thread %2d)       and interacting_subset_shapeType (%d) equals target_subset_shapetype (%d),\n",
                             omp_get_thread_num(), interacting_subset_shapeType, target_subset_shapetype);
                    }
                    #endif

                    //
                    // We found an interacting shape with a different path number (different electrical net!),
                    // so location (x,y,z) contains a DRC violation -- either a pseudo-DRC or non-pseudo-DRC.
                    //

                    // If this DRC violation represents a new combination of path number and shape type
                    // for the current (x,y,z) location, then count it as a new violation:
                    if (! check_for_DRC(current_cell_DRCs, total_nets, path_number, shape_type, interacting_path_num, interacting_shape_type) )  {

                      // Record the fact that we detected a DRC violation between the specified paths and shape-types:
                      record_DRC_by_paths(total_nets, current_cell_DRCs, path_number, shape_type, interacting_path_num, interacting_shape_type);

                      // Determine whether the current DRC is a 'pseudo-DRC'. A pseudo-DRC is one in which
                      // one or both of the interacting nets is a pseudo-net.
                      int isPseudoDRC = (user_inputs->isPseudoNet[path_number] || user_inputs->isPseudoNet[interacting_path_num]);

                      // Increment the DRC counts for the current thread:
                      if (! isPseudoDRC)  {
                        non_pseudo_DRC_count_per_thread[current_thread]++;
                      }

                      #ifdef DEBUG_routability
                      if (FOUND_XYZ)  {
                        printf("DEBUG: (thread %2d) Incremented non_pseudo_DRC_count_per_thread[%d] to %d. num_printed_DRCs_per_thread[%d] = %d.\n",
                               omp_get_thread_num(), current_thread, non_pseudo_DRC_count_per_thread[current_thread], current_thread,
                               num_printed_DRCs_per_thread[current_thread]);
                      }
                      #endif

                      // Write non-pseudo DRC details to STDOUT if we have not printed more than maxPrintedDRCs_per_thread:
                      if (num_printed_DRCs_per_thread[current_thread] < maxPrintedDRCs_per_thread)  {

                        // Print out details of non-pseudo DRC violation:
                        if (! isPseudoDRC)  {

                          if (DEBUG_ON || ! beQuiet)  {
                            printf("INFO: (thread %2d) Non-pseudo-DRC violation #%'d: Location (%d,%d,%d) within path number %d (shape type %d) is\n",
                                   current_thread, non_pseudo_DRC_count_per_thread[current_thread], x, y, z, path_number, shape_type);
                            printf("      (thread %2d) within %.2f cells (%.2f microns) of the center of path number %d (with shape-type %d).\n", current_thread,
                                   user_inputs->DRC_radius[DR_num_at_source][subset_shapeType_index][interacting_DR_num][interacting_subset_shapeType_at_target],
                                   user_inputs->DRC_radius[DR_num_at_source][subset_shapeType_index][interacting_DR_num][interacting_subset_shapeType_at_target] * user_inputs->cell_size_um,
                                   interacting_path_num, interacting_shape_type);
                          }

                          num_printed_DRCs_per_thread[current_thread]++;

                          #ifdef DEBUG_routability
                          if (FOUND_XYZ)  {
                            printf("DEBUG: (thread %2d)                           source_DR_num = %d\n", omp_get_thread_num(), DR_num_at_source);
                            printf("DEBUG: (thread %2d)                             path_number = %d\n", omp_get_thread_num(), path_number);
                            printf("DEBUG: (thread %2d)                               DR_subset = %d\n", omp_get_thread_num(), DR_subset);
                            printf("DEBUG: (thread %2d)                              shape_type = %d\n", omp_get_thread_num(), shape_type);
                            printf("DEBUG: (thread %2d)                  subset_shapeType_index = %d\n", omp_get_thread_num(), subset_shapeType_index);
                            printf("DEBUG: (thread %2d)                      interacting_DR_num = %d\n", omp_get_thread_num(), interacting_DR_num);
                            printf("DEBUG: (thread %2d)                    interacting_path_num = %d\n", omp_get_thread_num(), interacting_path_num);
                            printf("DEBUG: (thread %2d)         interacting_DR_subset_at_source = %d\n", omp_get_thread_num(), interacting_DR_subset_at_source);
                            printf("DEBUG: (thread %2d)         interacting_DR_subset_at_target = %d\n", omp_get_thread_num(), interacting_DR_subset_at_target);
                            printf("DEBUG: (thread %2d)                  interacting_shape_type = %d\n", omp_get_thread_num(), interacting_shape_type);
                            printf("DEBUG: (thread %2d)            interacting_subset_shapeType = %d\n", omp_get_thread_num(), interacting_subset_shapeType);
                            printf("DEBUG: (thread %2d)  interacting_subset_shapeType_at_target = %d\n", omp_get_thread_num(), interacting_subset_shapeType_at_target);
                            printf("DEBUG: (thread %2d)              DRC_radius[%d][%d][%d][%d] = %.3f cells\n", omp_get_thread_num(), DR_num_at_source,
                                    subset_shapeType_index, interacting_DR_num, interacting_subset_shapeType_at_target,
                                    user_inputs->DRC_radius[DR_num_at_source][subset_shapeType_index][interacting_DR_num][interacting_subset_shapeType_at_target]);
                            printf("DEBUG: (thread %2d)         num_printed_DRCs_per_thread[%d] = %d after incrementing.\n", omp_get_thread_num(), current_thread,
                                    num_printed_DRCs_per_thread[current_thread]);
                          }
                          #endif

                        }  // End of if-block for (! isPseudoDRC)
                      }  // End of if-block for (num_printed_DRCs_per_thread < maxPrintedDRCs_per_thread)


                      //
                      // If the DRC is not a pseudo-DRC, then update the cellInfo matrix with DRC information for the current violation:
                      //
                      // Note: Comment out the following if-statement if you want to view the pseudo-DRCs in the PNG maps.
                      if (! isPseudoDRC)  {
                        cellInfo[x][y][z].DRC_flag          = TRUE;
                        if (shape_type == VIA_UP) {
                          cellInfo[x][y][z].via_above_DRC_flag   = TRUE;
                        }
                      }  // End of if-block for (! isPseudoDRC)

                      //
                      // Add a small amount of congestion to the cell with a non-pseudo-DRC. This causes such cells to
                      // have a slightly higher cost to traverse. So if the autorouter later finds two paths
                      // that otherwise could have equal costs, it will choose the one with no (or fewer) DRC cells:
                      //
                      #ifdef DEBUG_routability
                      if (FOUND_XYZ)  {
                        printf("DEBUG: (thread %2d) Adding %d congestion at (%d,%d,%d) for path/DR_subset/shape-type %d/%d/%d and %d/%d/%d due to DRC violations.\n",
                               omp_get_thread_num(), DRC_congestion, x, y, z, path_number, DR_subset, shape_type, interacting_path_num, interacting_DR_subset_at_source,
                               interacting_shape_type);
                      }
                      #endif
                      if (! isPseudoDRC)  {
                        addCongestion(&(cellInfo[x][y][z]), path_number,          DR_subset,                       shape_type,             DRC_congestion);
                        addCongestion(&(cellInfo[x][y][z]), interacting_path_num, interacting_DR_subset_at_source, interacting_shape_type, DRC_congestion);
                      }

                      #ifdef DEBUG_routability
                      if (FOUND_XYZ)  {
                        printf("DEBUG: (thread %2d) Done adding congestion due to DRC violations.\n", omp_get_thread_num());
                      }
                      #endif


                      //
                      // Capture the DRC details in appropriate arrays. For the 'crossing_matrix' array
                      // that's shared among all parallel threads, use the 'atomic update' OMP pragma
                      // to ensure that only 1 thread updates the variable at a time.
                      //
                      // printf("DEBUG: About to increment crossing_matrix values for element [%d,%d]...\n",
                      //         path_number, interacting_path_num);

                      // printf("DEBUG: About to increment crossing_matrix[%d][%d] from %d...\n", path_number,
                      //         interacting_path_num, routability->crossing_matrix[path_number][interacting_path_num]);
                      #pragma omp atomic update
                      routability->crossing_matrix[path_number][interacting_path_num]++;
                      // printf("done.\n");

                      // printf("DEBUG: About to increment crossing_matrix[%d][%d] from %d...\n", interacting_path_num,
                      //         path_number, routability->crossing_matrix[interacting_path_num][path_number]);
                      #pragma omp atomic update
                      routability->crossing_matrix[interacting_path_num][path_number]++;

                      // printf("DEBUG:    Done incrementing crossing_matrix elements.\n");


                      // If DRC is not a pseudo-DRC, then categorize the DRC by (1) via-to-via spacing violation, or
                      // (2) trace-to-trace spacing violation, or (3) trace-to-via spacing violation. These
                      // categories can later be used to modify the sensitivity during path-finding of different
                      // categories of congestion.
                      if ((shape_type != TRACE) && (interacting_shape_type != TRACE))  {
                        // We got here, so the two shape-types are vias. Increment the DRC count
                        // for via-to-via violations:
                        non_pseudo_via2via_DRC_count_per_thread[current_thread]++;
                      }
                      else if ((shape_type == TRACE) && (interacting_shape_type == TRACE))  {
                        // We got here, so the two shape-types are traces. Increment the DRC count
                        // for trace-to-trace violations:
                        non_pseudo_trace2trace_DRC_count_per_thread[current_thread]++;
                      }
                      else  {
                        // We got here, so one of the two shape-types is a trace, and the other is
                        // a via. Increment the DRC count for trace-to-via violations:
                        non_pseudo_trace2trace_DRC_count_per_thread[current_thread]++;
                      }


                      // If DRC is not a pseudo-DRC, then add its details to the 'DRC_details_per_thread' array:
                      // printf("DEBUG: About to check if non_pseudo_DRC_count_per_thread[%d] (%d) is <= maxRecordedDRCs (%d)...\n",
                      //         current_thread, non_pseudo_DRC_count_per_thread[current_thread], maxRecordedDRCs);
                      if (! isPseudoDRC)  {
                        if (non_pseudo_DRC_count_per_thread[current_thread] <= maxRecordedDRCs)  {
                          // printf("DEBUG: About to update DRC_details_per_thread[%d][%d]... ", current_thread,
                          //         non_pseudo_DRC_count_per_thread[current_thread]-1);

                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].x                  = x;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].y                  = y;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].z                  = z;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].pathNum            = path_number;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].shapeType          = shape_type;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].offendingPathNum   = interacting_path_num;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].offendingShapeType = interacting_shape_type;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].minimumAllowedDistance =
                                       user_inputs->DRC_radius[DR_num_at_source][DR_subset*NUM_SHAPE_TYPES + shape_type][interacting_DR_num][interacting_DR_subset_at_source*NUM_SHAPE_TYPES + interacting_shape_type]
                                         * user_inputs->cell_size_um;
                          DRC_details_per_thread[current_thread][non_pseudo_DRC_count_per_thread[current_thread] - 1].minimumAllowedSpacing =
                                    user_inputs->designRules[DR_num_at_source][DR_subset].space_um[shape_type][interacting_shape_type];

                          // printf("DEBUG:    done.\n");
                        }  // End of if-block for numDRCs <= maxRecordedDRCs
                        // printf("DEBUG: Done checking if non_pseudo_DRC_count_per_thread[%d] (%d) is <= maxRecordedDRCs (%d)...\n",
                        //         current_thread, non_pseudo_DRC_count_per_thread[current_thread], maxRecordedDRCs);
                      }  // End of if-block for (! isPseudoDRC)
                    }  // End of if-block for (! already_in_array)
                    // printf("DEBUG: (thread %2d) Exited if-block for (! already_in_array).\n", current_thread);

                  }  // End of if-block for detecting DRC violation
                  // printf("DEBUG: (thread %2d) Exited if-block for detecting DRC violation.\n", current_thread);

                }  // End of for-loop for index 'path_index'
                // printf("DEBUG: (thread %2d) Exited for-loop for index 'path_index'.\n", current_thread);

              }  // End of for-loop for index 'target_shape' (0 to 2)
              // printf("DEBUG: (thread %2d) Exited for-loop for index 'target_shape'.\n", current_thread);

            }  // End of for-loop for index 'i' (0 to num_overlapping_nets - 1)
            // printf("DEBUG: (thread %2d) Exited for-loop for index 'i' (0 to num_overlapping_nets -1).\n", current_thread);
          }  // End of if-block for (! swap_zone)

          //
          // If 'addCongestionFlag' flag is set, then add congestion to this cell if it's
          // not in a pin-swappable region:
          //
          if ((addCongestionFlag) && (! cellInfo[x][y][z].swap_zone))  {
            //
            // Using the information in the m-by-m "congestion_nets" matrix, add congestion
            // to cell at location (x,y,z). (m = NUM_SHAPE_TYPES x num_source_DR_subsets.) Congestion
            // of design-rule subset/shape-type 'j' should be added to (x,y,z) if the
            // path-center of any net with design-rule subset/shape-type 'i' is within
            // 'cong_radius[i][j]' of (x,y,z).
            //
            #ifdef DEBUG_routability
            if (FOUND_XYZ)  {
              printf("DEBUG: (thread %2d) About to add congestion to (%d, %d, %d) based on 'congestion_nets' matrix...\n",
                     omp_get_thread_num(), x, y, z);
            }
            #endif
            for (int congestion_DR_subset = 0; congestion_DR_subset < num_source_DR_subsets; congestion_DR_subset++)  {

              // If current subset is not used by any nets, then continue on to the next subset:
              if (! user_inputs->DR_subsetUsed[DR_num_at_source][congestion_DR_subset])  {
                continue;
              }

              for (int congestion_shape_type = 0; congestion_shape_type < NUM_SHAPE_TYPES; congestion_shape_type++)  {

                // Calculate the index used for matrices 'congestion_count' and 'congestion_nets'.
                // The index is based on the values of DR_subset and shape_type, and is
                // simply 3 * DR_subset  +  shape_type:
                const short congestion_subset_shapeType = congestion_DR_subset * NUM_SHAPE_TYPES   +   congestion_shape_type;

                for (int target_DR_subset = 0; target_DR_subset < num_source_DR_subsets; target_DR_subset++)  {

                  // If current subset is not used by any nets, then continue on to the next subset:
                  if (! user_inputs->DR_subsetUsed[DR_num_at_source][target_DR_subset])  {
                    continue;
                  }

                  for (int target_shape_type = 0; target_shape_type < NUM_SHAPE_TYPES; target_shape_type++)  {

                    // Calculate the index used for matrices 'congestion_count' and 'congestion_nets'.
                    // The index is based on the values of DR_subset and shape_type, and is
                    // simply 3 * DR_subset  +  shape_type:
                    const short target_subset_shapeType = target_DR_subset * NUM_SHAPE_TYPES   +   target_shape_type;

                    #ifdef DEBUG_routability
                    if (FOUND_XYZ)  {
                      printf(" DEBUG: (thread %2d) congestion_DR_subset = %d, congestion_shape_type = %d, target_DR_subset = %d, target_shape_type = %d.\n",
                             omp_get_thread_num(), congestion_DR_subset, congestion_shape_type, target_DR_subset, target_shape_type);

                      printf(" DEBUG: (thread %2d) congestion_subset_shapeType = %d, target_subset_shapeType = %d.\n", omp_get_thread_num(),
                             congestion_subset_shapeType, target_subset_shapeType);

                      printf(" DEBUG: (thread %2d) congestion_count[%d][%d] = %d at (%d,%d,%d).\n", omp_get_thread_num(), congestion_subset_shapeType,
                             target_subset_shapeType, congestion_count[congestion_subset_shapeType][target_subset_shapeType], x, y, z);
                    }
                    #endif
                    for (int path_index = 0; path_index < congestion_count[congestion_subset_shapeType][target_subset_shapeType]; path_index++)  {

                      // Get the path number that interacts with this (x,y,z) location:
                      const short interacting_path_num = congestion_nets[congestion_subset_shapeType][target_subset_shapeType][path_index];

                      #ifdef DEBUG_routability
                      if (FOUND_XYZ)  {
                        printf("DEBUG: (thread %2d) At beginning of for-loop for index 'path_index'.\n", omp_get_thread_num());
                        printf("   DEBUG: (thread %2d) Preparing to enter 'addCongestion': path_index=%d, congestion_shape_type=%d, target_shape_type=%d, interacting_path_num=%d.\n",
                               omp_get_thread_num(), path_index, congestion_shape_type, target_shape_type, interacting_path_num);
                        printf("  DEBUG: (thread %2d) At (%d,%d,%d), interacting_path_num = %d, isPseudoNet = %d, target_shape_type = %d, addPseudoTraceCongestionNearVias = %d\n",
                               omp_get_thread_num(), x, y, z, interacting_path_num, user_inputs->isPseudoNet[interacting_path_num], target_shape_type,
                               mapInfo->addPseudoTraceCongestionNearVias[interacting_path_num][z]);
                      }
                      #endif

                      // Add congestion if:
                      //  (1a) 'interacting_path_num' is not a pseudo-net, and
                      //  (1b) 'addCongOnlyForDiffPair' is -1 (ADD_CONGESTION_FOR_ALL_NETS),
                      // OR:
                      //  (2a) 'addCongOnlyForDiffPair' does not equal -1 (ADD_CONGESTION_FOR_ALL_NETS), and
                      //  (2b) 'interacting_path_num' is a diff-pair path associated with pseudo-path number 'addCongOnlyForDiffPair'
                      if (   (   (! user_inputs->isPseudoNet[interacting_path_num])                                       // Criteria (1a) above
                              && (addCongOnlyForDiffPair == ADD_CONGESTION_FOR_ALL_NETS))                                 // Criteria (1b) above
                          || (   (addCongOnlyForDiffPair != ADD_CONGESTION_FOR_ALL_NETS)                                  // Criteria (2a) above
                              && (user_inputs->diffPairToPseudoNetMap[interacting_path_num] == addCongOnlyForDiffPair)))  // Criteria (2b) above
                      {
                        //
                        // Add congestion to cell (x,y,z):
                        //
                        addCongestion(&(cellInfo[x][y][z]), interacting_path_num, congestion_DR_subset, congestion_shape_type,
                                                routability->one_path_traversal[interacting_path_num]);

                        #ifdef DEBUG_routability
                        if (FOUND_XYZ)  {
                          printf("  DEBUG: (thread %2d)  ** %d congestion added to (%d, %d, %d) with path %d, subset %d, and shape_type %d\n",
                                 omp_get_thread_num(), routability->one_path_traversal[interacting_path_num], x, y, z, interacting_path_num,
                                 congestion_DR_subset, congestion_shape_type);
                        }
                        #endif
                      }  // End of if-block for non-pseudo-net

                    }  // End of for-loop for index 'path_index'
                  }  // End of for-loop for index 'target_shape_type'
                }  // End of for-loop for index 'target_DR_subset'
              }  // End of for-loop for index 'congestion_shape_type'
            }  // End of for-loop for index 'congestion_DR_subset'

            #ifdef DEBUG_routability
            if (FOUND_XYZ)  {
              printf("DEBUG: (thread %2d) Done adding congestion to (%d,%d,%d) based on 'congestion_nets' matrix.\n-------------\n",
                     omp_get_thread_num(), x, y, z);
            }
            #endif
          }  // End of if-block for (addCongestionFlag == TRUE)
        }  // End of if-block for (near_a_net == TRUE)
      }  // End of for-loop for x-coordinate
    }  // End of for-loop for y-coordinate
    //
    // NOTE: The above line marks the end of parallel processing
    //

    if (DEBUG_ON || ! beQuiet)  {
      tim = time(NULL); now = localtime(&tim);
      printf("\nINFO: (thread %2d) Done checking layer #%d of %d ('%s') for design-rule violations after iteration %d at %02d-%02d-%d, %02d:%02d:%02d.\n",
             omp_get_thread_num(), z, mapInfo->numLayers - 1, user_inputs->routingLayerNames[z], mapInfo->current_iteration,
             now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
    }

    //
    // Before moving on to the next routing layer, use the 2-dimensional crossing matrix to determine how
    // many DRC-cells were added for each path on the current routing layer 'z'. Iterate over user-defined
    // nets only (excluding pseudo-nets).
    //
    for (int path_A = 0; path_A < mapInfo->numPaths; path_A++)  {
      int num_DRCs = 0;  // Sum of (non-pseudo) DRC-cells for 'path_A'
      for (int path_B = 0; path_B < mapInfo->numPaths; path_B++)  {
        // Sum up all the DRC-cells for path_A across all other (non-pseudo) paths:
        num_DRCs += routability->crossing_matrix[path_A][path_B];
      }  // End of for-loop for index 'path_B'

      // Depending on which routing layer (z-value) we just analyzed, determine
      // how to calculate the number of DRCs by path and by layer:
      if (z > 0)  {
        // If we've currently analyzed *beyond* layer #0, then the total number of DRCs for
        // each path on layer 'z' is equal to the total number of DRCs on the path
        // minus the number of DRCs from all previous layers:
        for (int prev_layer = 0; prev_layer < z; prev_layer++)  {
          num_DRCs -= routability->path_DRC_cells_by_layer[path_A][prev_layer];
        }  // End of for-loop for index 'prev_layer'
      }  // End of else-block for z != 0

      // If we've currently analyzed only layer #0, then the total number of DRCs for
      // each path on layer #0 is equal to the total number of DRCs on the path.
      // Assign the number of DRC-cells for path 'path_A' and layer 'z':
      routability->path_DRC_cells_by_layer[path_A][z] = num_DRCs;
    } // End of for-loop for index 'path_A'

  }  // End of for-loop for z-coordinate

  if (DEBUG_ON || ! beQuiet)  {
    tim = time(NULL); now = localtime(&tim);
    printf("INFO: (thread %2d) Done checking for DRC violations by rastering over all (x,y,z) locations at %02d-%02d-%d, %02d:%02d:%02d.\n",
           omp_get_thread_num(), now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  }


  //
  // Print out DRCs by path and layer:
  //
  if (DEBUG_ON || ! beQuiet)  {
    printf("\nINFO: (thread %2d) DRCs by net and layer after iteration %d:\n", omp_get_thread_num(), mapInfo->current_iteration);
    printf("INFO: (thread %2d)           ", omp_get_thread_num());
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
      printf("Layer No. %2d   ", layer);    // Print number of routing layer
    }  // End of for-loop for index 'layer'

    printf("\nINFO: (thread %2d) Net No.   ", omp_get_thread_num());
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
      printf("(%s)", user_inputs->routingLayerNames[layer]);  // Print name of routing layer
      printf("%*s", (int)(13 - strlen(user_inputs->routingLayerNames[layer])), "");  // Print some buffer spaces
    }  // End of for-loop for index 'layer'
    printf(" (Net name)\n");
    printf("INFO: (thread %2d) -------   ", omp_get_thread_num());
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
      printf("-------------  ");
    }  // End of for-loop for index 'layer'
    printf("--------------------------\n");

    for (int path = 0; path < mapInfo->numPaths; path++)  {
      printf("INFO: (thread %2d) %6d   ", omp_get_thread_num(), path);
      for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
        printf("%'13d  ", routability->path_DRC_cells_by_layer[path][layer]);

        // Sum up the DRC's on layer 'layer' across all paths:
        routability->layer_DRC_cells[layer] += routability->path_DRC_cells_by_layer[path][layer];
      }
      printf("  (%s)\n", user_inputs->net_name[path]);
    }  // End of for-loop for index 'path'

    // Print the sums for each routing layer, while also dividing the DRC-count for each
    // layer by two, because we double-count DRCs when we accumulate them by paths:
    printf("INFO: (thread %2d) -------   ", omp_get_thread_num());
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
      printf("-------------  ");
      routability->layer_DRC_cells[layer] /= 2;
    }  // End of for-loop for index 'layer'

    printf("\nINFO: (thread %2d)  Total:  ", omp_get_thread_num());
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {
      printf("%'13d  ", routability->layer_DRC_cells[layer]);
    }  // End of for-loop for index 'layer'
    printf("(without double-counting)\n\n");

  }  // End of if-block


  // After returning from multiple threads, sum up the variables from each thread:
  //   (1) total_non_pseudo_DRC_count,  non_pseudo_via2via_DRC_count_per_thread,
  //       non_pseudo_trace2trace_DRC_count_per_thread, and non_pseudo_trace2via_DRC_count_per_thread.
  //   (2) routability->crossing_matrix[i][j]
  //
  for (int i = 0; i < num_threads; i++)  {
    #ifdef DEBUG_routability
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Consolidating DRC details from thread #%d...\n", omp_get_thread_num(), i);
      printf("  DEBUG: (thread %2d) %d DRCs for thread %d...\n", omp_get_thread_num(), non_pseudo_DRC_count_per_thread[i], i);
    }
    #endif

    // Sum up the DRC counts by shape-category across all threads and archive the sums in the 'routability'
    // data structure for the current iteration:
    routability->nonPseudo_num_via2via_DRC_cells[mapInfo->current_iteration]     += non_pseudo_via2via_DRC_count_per_thread[i];
    routability->nonPseudo_num_trace2trace_DRC_cells[mapInfo->current_iteration] += non_pseudo_trace2trace_DRC_count_per_thread[i];
    routability->nonPseudo_num_trace2via_DRC_cells[mapInfo->current_iteration]   += non_pseudo_trace2via_DRC_count_per_thread[i];


    for (int j = 0; j < non_pseudo_DRC_count_per_thread[i]; j++)  {

      // If the total DRC count is less than maxRecordedDRCs (typically ~10), then
      // add the details of the DRC to array 'DRC_details':
      if (total_non_pseudo_DRC_count < maxRecordedDRCs)  {
        // printf("    DEBUG: Adding non-pseudo DRC #%d from thread #%d to 'DRC_details[%d]...\n",
        //         j, i, total_non_pseudo_DRC_count);
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].x                      = DRC_details_per_thread[i][j].x;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].y                      = DRC_details_per_thread[i][j].y;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].z                      = DRC_details_per_thread[i][j].z;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].pathNum                = DRC_details_per_thread[i][j].pathNum;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].shapeType              = DRC_details_per_thread[i][j].shapeType;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].offendingPathNum       = DRC_details_per_thread[i][j].offendingPathNum;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].offendingShapeType     = DRC_details_per_thread[i][j].offendingShapeType;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].minimumAllowedDistance = DRC_details_per_thread[i][j].minimumAllowedDistance;
        routability->DRC_details[mapInfo->current_iteration][total_non_pseudo_DRC_count].minimumAllowedSpacing  = DRC_details_per_thread[i][j].minimumAllowedSpacing;
      }  // End of if-block for (total_DRC_count < maxRecordedDRCs)

      // Increment the non-pseudo DRC count from all threads:
      total_non_pseudo_DRC_count++;
    }  // End of for-block for index 'j' (0 to non_pseudo_DRC_count_per_thread)

    #ifdef DEBUG_routability
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Added %d DRC violations from thread %d to non-pseudo DRC count (%d).\n",
             omp_get_thread_num(), non_pseudo_DRC_count_per_thread[i], i, total_non_pseudo_DRC_count);
    }
    #endif

  }  // End of for-loop for index 'i'


  //
  // Using the fully-populated crossing matrix, calculate the number of cells with DRCs
  // for the entire system. Also count the number of DRC-clean paths:
  //
  routability->num_DRCfree_paths   = 0;
  routability->num_paths_with_DRCs = 0;
  for (int path_1 = 0; path_1 < mapInfo->numPaths; path_1++)  {

    // Move each element of array recent_path_DRC_cells[path_1][j] to
    // recent_path_DRC_cells[path_1][j+1]. This causes Acorn to 'forget' one iteration
    // of DRC data while enabling Acorn to add the most recent DRC data to this array:
    for (int i = numIterationsToReEquilibrate - 2; i >= 0; i--)  {
      routability->recent_path_DRC_cells[path_1][i+1] = routability->recent_path_DRC_cells[path_1][i];
      #ifdef DEBUG_routability
      if (DEBUG_ON)  {
        printf("DEBUG: Moved %d value from recent_path_DRC_cells[path = %d][recent_iteration = %d] to recent_path_DRC_cells[path = %d][recent_iteration = %d]\n",
                routability->recent_path_DRC_cells[path_1][i], path_1, i, path_1, i+1);
      }
      #endif
    }  // End of for-loop for index 'i'


    // Iterate over the second dimension of the crossing matrix:
    for (int path_2 = 0; path_2 < mapInfo->numPaths; path_2++)  {

      // Neither of the offending nets is a pseudo-net, so the DRC is a non-pseudo-DRC (normal DRC):
      routability->num_nonPseudo_DRC_cells  += routability->crossing_matrix[path_1][path_2];
      routability->path_DRC_cells[path_1]   += routability->crossing_matrix[path_1][path_2];

      // Total DRCs (pseudo and non-pseudo):
      routability->total_num_DRC_cells  += routability->crossing_matrix[path_1][path_2];

    }  // End of for-loop for index 'path_2'

    // Count the number of user-defined paths that have (and don't have) DRC violations:
    if (routability->path_DRC_cells[path_1] > 0)  {
      routability->num_paths_with_DRCs++;
    }
    else {
      routability->num_DRCfree_paths++;
    }

    // Store the number of DRC cells for the current path into recent_path_DRC_cells[path_1][0]
    // so we can keep track of the number of DRC cells over the last several iterations:
    routability->recent_path_DRC_cells[path_1][0] = routability->path_DRC_cells[path_1];

    // Re-calculate the number of recent iterations that this path has had *any* design-rule
    // violations. Using this value, also calculate the fractionRecentIterationsWithoutPathDRCs
    // for this path.
    {
      int interationsToAverage = min(numIterationsToReEquilibrate, mapInfo->current_iteration);
      int sum_path_DRC_iterations = 0;  // Number of recent iterations that had DRCs for current path
      //
      // Iterate over the recent iterations:
      for (int recent_iteration = 0; recent_iteration < interationsToAverage; recent_iteration++)  {

        #ifdef DEBUG_routability
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) recent_path_DRC_cells[path = %d][recent_iteration = %d] = %d\n",
                 omp_get_thread_num(), path_1, recent_iteration, routability->recent_path_DRC_cells[path_1][recent_iteration]);
        }
        #endif
        if (routability->recent_path_DRC_cells[path_1][recent_iteration] > 0)  {
          sum_path_DRC_iterations++;
        }
      }  // End of for-loop for index 'recent_iteration'
      routability->recent_path_DRC_iterations[path_1] = sum_path_DRC_iterations;
      routability->fractionRecentIterationsWithoutPathDRCs[path_1]
                   = 1.0 - (float)(routability->recent_path_DRC_iterations[path_1]) / interationsToAverage;

      #ifdef DEBUG_routability
      if (DEBUG_ON)  {
        printf("DEBUG: Path %d has had %d iterations with DRCs over the last %d iterations.\n",
                path_1, routability->recent_path_DRC_iterations[path_1], numIterationsToReEquilibrate);
        printf("DEBUG:  fractionRecentIterationsWithoutPathDRCs[%d] = %6.3f\n", path_1, routability->fractionRecentIterationsWithoutPathDRCs[path_1]);
      }
      #endif
    }

  }  // End of for-loop for index 'path_1'

  //
  // Calculate the DRC metrics for pseudo-paths:
  //
  for (int pseudo_path = mapInfo->numPaths; pseudo_path < total_nets; pseudo_path++)  {

    // Move each element of array recent_path_DRC_cells[pseudo_path][j] to
    // recent_path_DRC_cells[pseudo_path][j+1]. This causes Acorn to 'forget' one iteration
    // of DRC data while enabling Acorn to add the most recent DRC data to this array:
    for (int i = numIterationsToReEquilibrate - 2; i >= 0; i--)  {
      routability->recent_path_DRC_cells[pseudo_path][i+1] = routability->recent_path_DRC_cells[pseudo_path][i];
      #ifdef DEBUG_routability
      if (DEBUG_ON)  {
        printf("DEBUG: Moved %d value from recent_path_DRC_cells[path = %d][recent_iteration = %d] to recent_path_DRC_cells[path = %d][recent_iteration = %d]\n",
                routability->recent_path_DRC_cells[pseudo_path][i], pseudo_path, i, pseudo_path, i+1);
      }
      #endif
    }  // End of for-loop for index 'i'


    int child_path_1 = user_inputs->pseudoNetToDiffPair_1[pseudo_path];
    int child_path_2 = user_inputs->pseudoNetToDiffPair_2[pseudo_path];

    routability->path_DRC_cells[pseudo_path] =   routability->path_DRC_cells[child_path_1]
                                               + routability->path_DRC_cells[child_path_2];

    routability->num_pseudo_DRC_cells += routability->path_DRC_cells[pseudo_path];
    routability->total_num_DRC_cells  += routability->path_DRC_cells[pseudo_path];

    // Store the number of DRC cells for the current path into recent_path_DRC_cells[path_1][0]
    // so we can keep track of the number of DRC cells over the last several iterations:
    routability->recent_path_DRC_cells[pseudo_path][0] = routability->path_DRC_cells[pseudo_path];

    // Re-calculate the number of recent iterations that this path has had *any* design-rule
    // violations. Using this value, also calculate the fractionRecentIterationsWithoutPathDRCs
    // for this path.
    {
      int interationsToAverage = min(numIterationsToReEquilibrate, mapInfo->current_iteration);
      int sum_path_DRC_iterations = 0;  // Number of recent iterations that had DRCs for current path
      //
      // Iterate over the recent iterations:
      for (int recent_iteration = 0; recent_iteration < interationsToAverage; recent_iteration++)  {

        #ifdef DEBUG_routability
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) recent_path_DRC_cells[path = %d][recent_iteration = %d] = %d\n",
                 omp_get_thread_num(), pseudo_path, recent_iteration, routability->recent_path_DRC_cells[pseudo_path][recent_iteration]);
        }
        #endif
        if (routability->recent_path_DRC_cells[pseudo_path][recent_iteration] > 0)  {
          sum_path_DRC_iterations++;
        }
      }  // End of for-loop for index 'recent_iteration'
      routability->recent_path_DRC_iterations[pseudo_path] = sum_path_DRC_iterations;
      routability->fractionRecentIterationsWithoutPathDRCs[pseudo_path]
                   = 1.0 - (float)(routability->recent_path_DRC_iterations[pseudo_path]) / interationsToAverage;

      #ifdef DEBUG_routability
      if (DEBUG_ON)  {
        printf("DEBUG: Path %d has had %d iterations with DRCs over the last %d iterations.\n",
               pseudo_path, routability->recent_path_DRC_iterations[pseudo_path], numIterationsToReEquilibrate);
        printf("DEBUG:  fractionRecentIterationsWithoutPathDRCs[%d] = %6.3f\n", pseudo_path,
               routability->fractionRecentIterationsWithoutPathDRCs[pseudo_path]);
      }
      #endif
    }

  }  // End of for-loop for index 'pseudo_path'

  // Archive the number of nets that have DRCs (excluding pseudo-nets) for the current iteration:
  routability->numNonPseudoDRCnets[mapInfo->current_iteration] = routability->num_paths_with_DRCs;

  // Divide the DRC counts by 2, since we double-count each DRC violation in loop above
  // (If net A crosses path B, then net B also crosses path A. But this counts
  // as a single crossing.)
  routability->total_num_DRC_cells     /= 2;
  routability->num_pseudo_DRC_cells    /= 2;
  routability->num_nonPseudo_DRC_cells /= 2;

  if (routability->num_nonPseudo_DRC_cells != total_non_pseudo_DRC_count)  {
    printf("\n\nERROR: In function 'calcRoutabilityMetrics', the variable 'total_non_pseudo_DRC_count' (%d) is not\n", total_non_pseudo_DRC_count);
    printf(    "       equal to the variable 'routability->num_nonPseudo_DRC_cells' (%d).\n",
             routability->num_nonPseudo_DRC_cells);
    printf(    "       This represents an unexpected error in the software, and the program is terminating.\n\n");
    exit(1);
  }  // End of if-block for (routability->total_num_DRC_cells != DRC_count)

  // printf("DEBUG: In calcRoutabilityMetrics, finished calculating DRCs.\n");


  // Calculate the total number of explored cells for the current iteration, summing over all paths:
  routability->iteration_explored_cells[mapInfo->current_iteration] = 0; // Initialize (although this was previously done)
  for (int path = 0; path < total_nets; path++)  {
    routability->iteration_explored_cells[mapInfo->current_iteration] += routability->path_explored_cells[path];
  }  // End of for-loop for index 'path'

  // Update the total number of explored cells across all iterations:
  routability->total_explored_cells = 0;
  for (int iteration = 1; iteration <= mapInfo->current_iteration; iteration++)  {
    routability->total_explored_cells += routability->iteration_explored_cells[iteration];
  }

  // Archive the non-pseudo path cost, path length, via count, and number of non-pseudo DRCs for each iteration:
  // printf("DEBUG: About to write element #%d of num_DRC_cells, totalPathLengths, and viaCounts.\n", mapInfo->current_iteration);
  routability->nonPseudoPathCosts[mapInfo->current_iteration]      = routability->total_nonPseudo_cost;
  routability->nonPseudo_num_DRC_cells[mapInfo->current_iteration] = routability->num_nonPseudo_DRC_cells;
  routability->nonPseudoPathLengths[mapInfo->current_iteration]    = routability->total_lateral_nonPseudo_length_mm;
  routability->nonPseudoViaCounts[mapInfo->current_iteration]      = routability->total_nonPseudo_vias;


  // printf("\nDEBUG: In iteration %d:\n", mapInfo->current_iteration);
  // printf(  "DEBUG:            nonPseudo_num_via2via_DRC_cells = %'d\n", routability->nonPseudo_num_via2via_DRC_cells[mapInfo->current_iteration]);
  // printf(  "DEBUG:        nonPseudo_num_trace2trace_DRC_cells = %'d\n", routability->nonPseudo_num_trace2trace_DRC_cells[mapInfo->current_iteration]);
  // printf(  "DEBUG:          nonPseudo_num_trace2via_DRC_cells = %'d\n\n", routability->nonPseudo_num_trace2via_DRC_cells[mapInfo->current_iteration]);
  // printf(  "DEBUG:                    nonPseudo_num_DRC_cells = %'d\n\n", routability->nonPseudo_num_DRC_cells[mapInfo->current_iteration]);

  //
  // Archive the cumulative number of DRC-free iterations:
  //
  if (mapInfo->current_iteration == 1)  {
    if (routability->num_nonPseudo_DRC_cells == 0)  {
      routability->cumulative_DRCfree_iterations[mapInfo->current_iteration] = 1;
    }
    else  {
      routability->cumulative_DRCfree_iterations[mapInfo->current_iteration] = 0;
    }
  }  // End of if-block for current_iteration == 1
  else  {
    if (routability->num_nonPseudo_DRC_cells == 0)  {
      routability->cumulative_DRCfree_iterations[mapInfo->current_iteration] = 1 + routability->cumulative_DRCfree_iterations[mapInfo->current_iteration - 1];
    }
    else  {
      routability->cumulative_DRCfree_iterations[mapInfo->current_iteration] = routability->cumulative_DRCfree_iterations[mapInfo->current_iteration - 1];
    }
  }  // End of if/else-block for calculating cumulative_DRCfree_iterations

  #ifdef DEBUG_routability
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) After iteration %d, cumulative_DRCfree_iterations = %d.\n", omp_get_thread_num(),
           mapInfo->current_iteration, routability->cumulative_DRCfree_iterations[mapInfo->current_iteration]);
  }
  #endif


  // Determine whether the routing metrics reached a plateau. Such a
  // plateau is defined if the following criteria are satisfied:
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
  determineIfMetricsPlateaued(mapInfo, routability);


  //
  // If the number of user-defined nets with DRCs is 2 or more, and the iteration number has
  // exceeded the value of 20*log(numPaths),

//// The following was added as an experiment on 7/7/2024:
  // and the iteration number is at least 30 iterations (1.5 x numIterationsToReEquilibrate) beyond
  // the (non-zero) iteration number of the last algorithm change,

  // then randomly assign which of the DRC-containing
  // paths will be handled differently in the next iteration. This pseudo-randomization helps
  // to eliminate oscillatory behavior between nets, and also helps to avoid getting stuck in
  // a routing configuration that's a local minimum in overall cost.
  //
  unsigned int seed = mapInfo->current_iteration;  // Create a seed in current thread for random numbers
//if ((routability->num_paths_with_DRCs > 1) && (mapInfo->current_iteration > 20*log10(mapInfo->numPaths)))  {
  if (   (routability->num_paths_with_DRCs > 1) && (mapInfo->current_iteration > 20*log10(mapInfo->numPaths))
      && (   (routability->latestAlgorithmChange == 0)
          || (mapInfo->current_iteration < routability->latestAlgorithmChange + (int)(1.5 * numIterationsToReEquilibrate))) )  {

////#ifdef DEBUG_routability
////if (DEBUG_ON)  {
      printf("\nDEBUG: Two or more paths have DRCs after iteration %d, so we'll randomly select paths to modify their congestion-related G-cost in next iteration.\n",
              mapInfo->current_iteration);
      printf(  "DEBUG:                                          num_paths_with_DRCs = %d\n", routability->num_paths_with_DRCs);
      printf(  "DEBUG:                                          20 * log(num_paths) = %d\n", (int)(20*log10(mapInfo->numPaths)));
      printf(  "DEBUG:                                        latestAlgorithmChange = iteration %d\n", routability->latestAlgorithmChange);
      printf(  "DEBUG:   latestAlgorithmChange + 1.5 x numIterationsToReEquilibrate = %d\n", routability->latestAlgorithmChange + (int)(1.5 * numIterationsToReEquilibrate));
////}
////#endif

    // Iterate over all paths, including pseudo-paths:
    for (int path = 0; path < total_nets; path++)  {

      // Check if the current path had DRCs in at least 4 of the last 10 most recent iterations that
      // would make it eligible to have its congestion-related G-cost randomly adjusted during the
      // next iteration.

//// The following if-statement was commented out 6/20/2024 as an experiment to randomize *all* nets' congestion sensitivity:
      if (calc_fraction_of_recent_iterations_with_DRCs(routability->recent_path_DRC_cells[path], 10) >= 0.4)  {


        // Check whether the current path had DRCs during most recent iteration, subject to following constraints:
        //  (a) The path is a non-pseudo-path and has DRC related to non-pseudo paths, or
        //  (b) The path is a pseudo-path and at least one of its child diff-pair paths has
        //      DRCs related to non-pseudo paths.

//// The following if-statement was commented out 6/20/2024 as an experiment to randomize *all* nets' congestion sensitivity:
        if (   ( ( ! user_inputs->isPseudoNet[path]) && (routability->path_DRC_cells[path] > 0) )            // Item (a) above
            || ( (    user_inputs->isPseudoNet[path])                                                        // Item (b) above
                   && (   (routability->path_DRC_cells[ user_inputs->pseudoNetToDiffPair_1[path] ] > 0)      // Item (b) above, continued
                       || (routability->path_DRC_cells[ user_inputs->pseudoNetToDiffPair_2[path] ] > 0) )))  // Item (b) above, continued
        {

          #ifdef DEBUG_routability
          if (DEBUG_ON)  {
            printf("DEBUG:   Path %d or one of its two diff-pair child-paths had non-pseudo DRCs during iteration %d,\n",
                   path, mapInfo->current_iteration);
            printf("DEBUG:   so it's a candidate to be randomly selected from the %d paths with DRCs.\n",
                   routability->num_paths_with_DRCs);
          }
          #endif

          // We got here, so the current path had DRC violations during the most recent
          // iteration. It is therefore a candidate to have its congestion treated differently
          // during the next iteration. We therefore roll a pseudo-random 'dice' that has the
          // the same number of 'sides' as there are paths with DRCs. If the roll yields a
          // a 'zero' (arbitrarily chosen value), then we assign this path to have its
          // congestion handled differently in the next iteration. On average, only N paths
          // will be chosen during each iteration, where N equals the maximum of (a) the
          // number of nets with DRCs, and (b) 20% of the total number of nets:

//// The following line was commented out 6/20/2024 as an experiment to randomize *all* nets' congestion sensitivity,
//// and replaced with the subsequent line:
          int dice_roll = rand_r(&seed) % routability->num_paths_with_DRCs;
////      int dice_roll = rand_r(&seed) % total_nets;


          #ifdef DEBUG_routability
          if (DEBUG_ON)  {
            printf("DEBUG:     Dice roll resulted in value of %d\n", dice_roll);
          }
          #endif


//// The following if-statement was commented out 7/4/2024 as an experiment and replaced with the subsequent line
//// to subject 2 DRC-nets (not 1) to randomized sensitivity:
////      if (dice_roll == 0)  {

//// The following if-statement was commented out 7/7/2024 and replaced with the subsequent compound if-statement:
////      if (dice_roll <= 1)  {
          if (   ((  user_inputs->isPseudoNet[path]) && (dice_roll == 0))
              || ((! user_inputs->isPseudoNet[path]) && (dice_roll <= 1)))  {

          // Following line causes, on average, the minimum of the DRC-nets or 20% of the total nets
          // (with a minimum of 1 net) to be selected to have their congestion sensitivity modified
          // in the subsequent iteration:
//        if (dice_roll <= max(routability->num_paths_with_DRCs, 0.20 * total_nets))  {
//        if (dice_roll <= min(routability->num_paths_with_DRCs, 0.20 * total_nets))  {
//        if (dice_roll < min(routability->num_paths_with_DRCs, 0.20 * total_nets))  {


            // Now that we know the current path was selected to have its congestion-related
            // G-cost handled differently, we now randomly choose whether this G-cost should
            // be DECREASEd or INCREASEd:
            if (rand_r(&seed) % 2)  {  // This randomly results in 0 or 1
              routability->randomize_congestion[path] = DECREASE;

              #ifdef DEBUG_routability
              if (DEBUG_ON)  {
                printf("DEBUG:       Path %d WAS selected as random path to have congestion-related G-cost DECREASED.\n", path);
              }
              #endif
            }
            else  {
              routability->randomize_congestion[path] = INCREASE;

              #ifdef DEBUG_routability
              if (DEBUG_ON)  {
                printf("DEBUG:       Path %d WAS selected as random path to have congestion-related G-cost INCREASED.\n", path);
              }
              #endif

            }
          }  // End of if-block for dice_roll == zero
          else  {

            #ifdef DEBUG_routability
            if (DEBUG_ON)  {
              printf("DEBUG:       Path %d was NOT selected as random path\n", path);
            }
            #endif

            routability->randomize_congestion[path] = FALSE;
          }  // End of else-block for dice_roll being non-zero

//// The following line was commented out 6/20/2024 as an experiment to randomize *all* nets' congestion sensitivity:
        }  // End of if-block for path_DRC_cells > 0


//// The following line was commented out 6/20/2024 as an experiment to randomize *all* nets' congestion sensitivity:
      }  // End of if-block for >40% of recent iterations having DRCs


    }  // End of for-loop for index 'path'
  }  // End of if-block for (num_paths_with_DRCs > 1)

  //
  // Free memory allocated in this function from the heap:
  //
  // Free 1-dimensional arrays:
  free(non_pseudo_DRC_count_per_thread);              non_pseudo_DRC_count_per_thread = NULL;
  free(num_printed_DRCs_per_thread);                  num_printed_DRCs_per_thread = NULL;
  free(non_pseudo_via2via_DRC_count_per_thread);      non_pseudo_via2via_DRC_count_per_thread = NULL;
  free(non_pseudo_trace2trace_DRC_count_per_thread);  non_pseudo_trace2trace_DRC_count_per_thread = NULL;
  free(non_pseudo_trace2via_DRC_count_per_thread);    non_pseudo_trace2via_DRC_count_per_thread = NULL;

  // Free 2-dimensional array:
  for (int i = 0; i < num_threads; i++)  {
    free(DRC_details_per_thread[i]);  DRC_details_per_thread[i] = NULL;
  }
  free(DRC_details_per_thread);  DRC_details_per_thread = NULL;


}  // End of function 'calcRoutabilityMetrics'




