#include "global_defs.h"


//-----------------------------------------------------------------------------
// Name: compile_regex
// Desc: Compile regular expression given by regex_string, and place result
//       into compiled_regex.
//-----------------------------------------------------------------------------
void compile_regex(char *regex_string, regex_t *compiled_regex)  {

  // Compile the regular expression. Exit if it fails to compile.
  if (regcomp(compiled_regex, regex_string, REG_EXTENDED|REG_ICASE) != 0)  {
    fprintf(stderr, "Failed to compile regex '%s'\n", regex_string);
    exit(1);
  }

}  // End of function 'compile_regex'


//-----------------------------------------------------------------------------
// Name: calc_2D_Pythagorean_distance_floats
// Desc: Calculate the distance between (x1, y1) and (x2, y2) using the
//       Pythagorean formula. This function does not account for the separation
//       in the z-dimension, and is used for calculating the distance between
//       floating-point-based coordinates.
//-----------------------------------------------------------------------------
float calc_2D_Pythagorean_distance_floats(const float x1, const float y1, const float x2, const float y2)  {

  return sqrt( (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) );

}  // End of function 'calc_2D_Pythagorean_distance_floats'


//-----------------------------------------------------------------------------
// Name: copyDesignRuleSubset
// Desc: Copy user-supplied design-rule parameters from a source design-rule
//       subset to a destination design-rule subset. This function does not
//       copy derived/calculated parameters that are not supplied in the
//       user's input file.
//-----------------------------------------------------------------------------
void copyDesignRuleSubset(InputValues_t *user_inputs, int source_set, int source_subset,
                          int destination_set, int destination_subset)  {

  // printf("DEBUG: Copying design-rule set %d, subset %d, to to set %d, subset %d.\n",
  //    source_set, source_subset, destination_set, destination_subset);

  // Allowed routing directions:
  user_inputs->designRules[destination_set][destination_subset].routeDirections
        = user_inputs->designRules[source_set][source_subset].routeDirections;

  // Line width in microns:
  user_inputs->designRules[destination_set][destination_subset].lineWidthMicrons
        = user_inputs->designRules[source_set][source_subset].lineWidthMicrons;

  // Copy of line width in microns:
  user_inputs->designRules[destination_set][destination_subset].copy_lineWidthMicrons
        = user_inputs->designRules[source_set][source_subset].copy_lineWidthMicrons;

  // Diameter of upward-going via, in microns
  user_inputs->designRules[destination_set][destination_subset].viaUpDiameterMicrons
        = user_inputs->designRules[source_set][source_subset].viaUpDiameterMicrons;

  // Copy of diameter of upward-going via, in microns
  user_inputs->designRules[destination_set][destination_subset].copy_viaUpDiameterMicrons
        = user_inputs->designRules[source_set][source_subset].copy_viaUpDiameterMicrons;

  // Diameter of downward-going via, in microns
  user_inputs->designRules[destination_set][destination_subset].viaDownDiameterMicrons
        = user_inputs->designRules[source_set][source_subset].viaDownDiameterMicrons;

  // Copy of diameter of downward-going via, in microns
  user_inputs->designRules[destination_set][destination_subset].copy_viaDownDiameterMicrons
        = user_inputs->designRules[source_set][source_subset].copy_viaDownDiameterMicrons;

  // Trace-to-trace spacing in microns
  user_inputs->designRules[destination_set][destination_subset].lineSpacingMicrons
        = user_inputs->designRules[source_set][source_subset].lineSpacingMicrons;

  // Spacing between upward-going via and adjacent traces
  user_inputs->designRules[destination_set][destination_subset].viaUpToTraceSpacingMicrons
        = user_inputs->designRules[source_set][source_subset].viaUpToTraceSpacingMicrons;

  // Spacing between downward-going via and adjacent traces
  user_inputs->designRules[destination_set][destination_subset].viaDownToTraceSpacingMicrons
        = user_inputs->designRules[source_set][source_subset].viaDownToTraceSpacingMicrons;

  // Spacing between adjacent, upward-going vias
  user_inputs->designRules[destination_set][destination_subset].viaUpToViaUpSpacingMicrons
        = user_inputs->designRules[source_set][source_subset].viaUpToViaUpSpacingMicrons;

  // Spacing between adjacent, downward-going vias
  user_inputs->designRules[destination_set][destination_subset].viaDownToViaDownSpacingMicrons
        = user_inputs->designRules[source_set][source_subset].viaDownToViaDownSpacingMicrons;

  // Spacing between adjacent up- and down-ward going vias
  user_inputs->designRules[destination_set][destination_subset].viaUpToViaDownSpacingMicrons
        = user_inputs->designRules[source_set][source_subset].viaUpToViaDownSpacingMicrons;

  // Boolean flag to specify if subset is dedicated to differential pairs:
  user_inputs->designRules[destination_set][destination_subset].isDiffPairSubset
        = user_inputs->designRules[source_set][source_subset].isDiffPairSubset;

  // Boolean flag to specify if subset is dedicated to a diff-pair's pseudo-net:
  user_inputs->designRules[destination_set][destination_subset].isPseudoNetSubset
        = user_inputs->designRules[source_set][source_subset].isPseudoNetSubset;

  // Trace-to-trace pitch for differential pairs, in microns:
  user_inputs->designRules[destination_set][destination_subset].traceDiffPairPitchMicrons
        = user_inputs->designRules[source_set][source_subset].traceDiffPairPitchMicrons;

  // Iterate over all shape-types to copy parameters that use shape-type as
  // an index to an array:
  for (int shape_1 = 0; shape_1 < NUM_SHAPE_TYPES; shape_1++)  {

    // Shape width, in microns:
    user_inputs->designRules[destination_set][destination_subset].width_um[shape_1]
          = user_inputs->designRules[source_set][source_subset].width_um[shape_1];

    for (int shape_2 = 0; shape_2 < NUM_SHAPE_TYPES; shape_2++)  {

      // Shape-to-shape spacing, in microns:
      user_inputs->designRules[destination_set][destination_subset].space_um[shape_1][shape_2]
            = user_inputs->designRules[source_set][source_subset].space_um[shape_1][shape_2];

    }  // End of for-loop for index 'shape_2'
  }  // End of for-loop for index 'shape_1'

}  // End of function 'copyDesignRuleSubset'


//-----------------------------------------------------------------------------
// Name: verifyDiffPairPitch
// Desc: Verify that the diff-pair pitch for a net is equal to the diff-pair
//       pitch for that net's partner net in each design-rule set and subset.
//-----------------------------------------------------------------------------
void verifyDiffPairPitch(InputValues_t *user_inputs)  {

  char diffPairPitchError = FALSE;
  for (int pathNum = 0; pathNum < user_inputs->num_nets; pathNum++)  {
    if (user_inputs->isDiffPair[pathNum])  {
      // Get net number for the other net in this diff pair:
      int partnerNet = user_inputs->diffPairPartner[pathNum];

      // Iterate over the design-rule sets:
      for (int DR_set = 0; DR_set < user_inputs->numDesignRuleSets; DR_set++ )  {

        // Check that values in 'diffPairPitchCells' match between diff-pair partners:
        // printf("DEBUG: About to compare diffPairPitchCells[%d][%d] to diffPairPitchCells[%d][%d]...(%d vs %d)\n", pathNum, DR_set, partnerNet, DR_set,
        //        user_inputs->diffPairPitchCells[pathNum][DR_set], user_inputs->diffPairPitchCells[partnerNet][DR_set]);
        if (user_inputs->diffPairPitchCells[pathNum][DR_set] != user_inputs->diffPairPitchCells[partnerNet][DR_set])  {
          printf("\nERROR: Net number #%d ('%s') has a target diff-pair pitch of %.3f cells in design-rule set '%s',\n",
                 pathNum, user_inputs->net_name[pathNum], user_inputs->diffPairPitchCells[pathNum][DR_set], user_inputs->designRuleSetName[DR_set]);
          printf("       but the partner net #%d ('%s') has a different diff-pair pitch of %.3f cells in this design-rule set.\n\n",
                 partnerNet, user_inputs->net_name[partnerNet], user_inputs->diffPairPitchCells[partnerNet][DR_set]);
          diffPairPitchError = TRUE;
        }  // End of if-block for unequal values of 'diffPairPitchCells' variables

        // Check that values in 'diffPairPitchMicrons' match between diff-pair partners:
        if (user_inputs->diffPairPitchMicrons[pathNum][DR_set] != user_inputs->diffPairPitchMicrons[pathNum][DR_set])  {
          printf("\nERROR: Net number #%d ('%s') has a target diff-pair pitch of %5.2f microns in design-rule set '%s',\n",
                 pathNum, user_inputs->net_name[pathNum], user_inputs->diffPairPitchMicrons[pathNum][DR_set], user_inputs->designRuleSetName[DR_set]);
          printf("       but the partner net #%d ('%s') has a different diff-pair pitch of %5.2f microns on this layer.\n\n",
                 partnerNet, user_inputs->net_name[partnerNet], user_inputs->diffPairPitchMicrons[partnerNet][DR_set]);
          diffPairPitchError = TRUE;
        }  // End of if-block for unequal values of 'diffPairPitchMicrons' variables
      }  // End of for-loop for index 'DR_set' (0 to numDesignRuleSets)
    }  // End of if-block for 'isDiffPair[pathNum]'
  }  // End of for-loop for index 'pathNum'

  // If we found discrepancy(ies) above between any pairs of diff-pair nets, then exit the program:
  if (diffPairPitchError)  {
    printf("       Program is exiting.\n\n");
    exit(1);
  }

}  // End of function 'verifyDiffPairPitch'


//-----------------------------------------------------------------------------
// Name: mapPseudoNets
// Desc: Map the user-defined diff-pair nets to pseudo nets, storing the results
//       in array:
//
//         user_inputs->diffPairToPseudoNetMap[net_number] = pseudo_net_number
//
//       Also, map the pseudo nets back to the user-defined diff-pair nets,
//       storing the results in the following two arrays:
//
//         user_inputs->pseudoNetToDiffPair_1[pseudo_net_number] = diff_pair_net_1
//         user_inputs->pseudoNetToDiffPair_2[pseudo_net_number] = diff_pair_net_2
//
//-----------------------------------------------------------------------------
void mapPseudoNets(InputValues_t *user_inputs)  {

  printf("INFO: Input netlist contains %d diff-pair nets. Each pair is mapped to one of %d pseudo nets for routing.\n",
         user_inputs->num_diff_pair_nets, user_inputs->num_pseudo_nets);

  // Initialize 'pseudo_net_number' to the first number after the number
  // of user-defined nets:
  int pseudo_net_number = user_inputs->num_nets;

  // Iterate over all the user-defined nets:
  for (int pathNum = 0; pathNum < user_inputs->num_nets; pathNum++)  {

    // Check whether net is part of a differential pair:
    if (user_inputs->isDiffPair[pathNum])  {

      // Check whether we've already handled this net. If we have, then the following
      // variable will be greater than (or equal to) 'num_nets'.
      if (user_inputs->diffPairToPseudoNetMap[pathNum] >= user_inputs->num_nets)  {
        // We've already handled this net when we handled its diff-pair partner.
        // So skip to the next net:
        continue;
      }

      // Get net number for the other net in this diff pair:
      int partnerNet = user_inputs->diffPairPartner[pathNum];

      // Map this net and its partner to its pseudo net:
      user_inputs->diffPairToPseudoNetMap[pathNum   ] = pseudo_net_number;
      user_inputs->diffPairToPseudoNetMap[partnerNet] = pseudo_net_number;

      // Map the pseudo net back to current net and its partner:
      user_inputs->pseudoNetToDiffPair_1[pseudo_net_number] = pathNum;
      user_inputs->pseudoNetToDiffPair_2[pseudo_net_number] = partnerNet;

      // Flag path 'pseudo_net_number' as a pseudo net by setting to TRUE the Boolean
      // flag 'isPseudoNet':
      user_inputs->isPseudoNet[pseudo_net_number] = TRUE;

      printf("DEBUG: Net #%d ('%s') is mapped to pseudo net #%d.\n",
             pathNum, user_inputs->net_name[pathNum], pseudo_net_number);
      printf("       Pseudo net #%d is mapped to:\n", pseudo_net_number);
      printf("         1.) Net #%d ('%s')\n", user_inputs->pseudoNetToDiffPair_1[pseudo_net_number],
             user_inputs->net_name[user_inputs->pseudoNetToDiffPair_1[pseudo_net_number]]);
      printf("         2.) Net #%d ('%s')\n", user_inputs->pseudoNetToDiffPair_2[pseudo_net_number],
             user_inputs->net_name[user_inputs->pseudoNetToDiffPair_2[pseudo_net_number]]);

      // Increment 'pseudo_net_number':
      pseudo_net_number++;

    }  // End of if-block for 'isDiffPair[pathNum]'
  }  // End of for-loop for index 'pathNum'

  // Confirm that we added the appropriate number of pseudo nets:
  if (pseudo_net_number != user_inputs->num_nets + user_inputs->num_pseudo_nets)  {
    printf("\nERROR: An error was detected at the end of function 'mapPseudoNets', in which\n");
    printf(  "       the largest pseudo net number added for routing (%d) does not equal the expected\n",
             pseudo_net_number - 1);
    printf(  "       value of %d. This reflects an error in the software. Please inform the software\n",
             user_inputs->num_nets + user_inputs->num_pseudo_nets - 1);
    printf(  "       developer. Program will exit.\n\n");
    exit(1);
  }  // End of if-block for (pseudo_nets != num_nets + num_pseudo_nets)

}  // End of function 'mapPseudoNets'


//-----------------------------------------------------------------------------
// Name: checkTerminalLocations
// Desc: Confirm that start- and end-locations are within the map. Also,
//       calculate the coordinates of pseudo nets' terminals, which are the
//       midpoints of the corresponding differential-pair nets.
//-----------------------------------------------------------------------------
void checkTerminalLocations(InputValues_t *user_inputs, const MapInfo_t *mapInfo)  {

  // Iterate through each path:
  for (int i = 0; i < mapInfo->numPaths; i++)  {


    // Confirm that the (x,y) coordinates of the start- and end-terminals are within the
    // perimeter of the map.
    if (pointIsOutsideOfMap(mapInfo->start_cells[i], mapInfo) || pointIsOutsideOfMap(mapInfo->end_cells[i], mapInfo))  {
      printf("\nERROR: Net #%d ('%s') has a starting or ending terminal that is outside of valid map perimeter. The boundaries\n",
               i, user_inputs->net_name[i]);
      printf("       are (0, 0) to (%5.1f, %5.1f), in microns. This path runs from (%5.1f, %5.1f) to (%5.1f, %5.1f).\n",
               mapInfo->mapWidth * user_inputs->cell_size_um, mapInfo->mapHeight * user_inputs->cell_size_um,
               mapInfo->start_cells[i].X * user_inputs->cell_size_um, mapInfo->start_cells[i].Y * user_inputs->cell_size_um,
               mapInfo->end_cells[i].X * user_inputs->cell_size_um, mapInfo->end_cells[i].Y * user_inputs->cell_size_um);
      printf("       Program is exiting.\n\n");
      exit(1);

    }  // End of if-block

  }  // End of for-loop for index 'i' (0 to numPaths)


  // Calculate for the coordinates of the pseudo nets's terminals, which are located at the
  // midpoints of the corresponding differential-pair nets.
  int max_routed_nets = user_inputs->num_nets + user_inputs->num_pseudo_nets;
  for (int pseudo_net_num = user_inputs->num_nets; pseudo_net_num < max_routed_nets; pseudo_net_num++)  {

    // printf("DEBUG: In function 'checkTerminalLocations', calculating terminal locations for pseudo net #%d.\n", pseudo_net_num);

    // Get the user-defined diff-pair nets associated with this pseudo net:
    int net_1 = user_inputs->pseudoNetToDiffPair_1[pseudo_net_num];
    int net_2 = user_inputs->pseudoNetToDiffPair_2[pseudo_net_num];

    // printf("DEBUG: Pseudo net #%d is associated with nets %d (%s) and %d (%s).\n",
    //        pseudo_net_num, net_1, user_inputs->net_name[net_1], net_2, user_inputs->net_name[net_2]);

    // Get the coordinates of the starting and ending terminals for both diff-pair nets:
    float startX_1 = user_inputs->start_X_um[net_1];    // In microns
    float startY_1 = user_inputs->start_Y_um[net_1];    // In microns
    int   startZ_1 = mapInfo->start_cells[net_1].Z;     // Expressed as routing-layer number
    float endX_1   = user_inputs->end_X_um[net_1];      // In microns
    float endY_1   = user_inputs->end_Y_um[net_1];      // In microns
    int   endZ_1   = mapInfo->end_cells[net_1].Z;       // Expressed as routing-layer number
    float startX_2 = user_inputs->start_X_um[net_2];    // In microns
    float startY_2 = user_inputs->start_Y_um[net_2];    // In microns
    float endX_2   = user_inputs->end_X_um[net_2];      // In microns
    float endY_2   = user_inputs->end_Y_um[net_2];      // In microns

    // printf("DEBUG: Diff-pair net %d runs from (%6.3f, %6.3f, %d) to (%6.3f, %6.3f, %d).\n",
    //        net_1, startX_1, startY_1, startZ_1, endX_1, endY_1, endZ_1);
    // printf("DEBUG: Diff-pair net %d runs from (%6.3f, %6.3f, %d) to (%6.3f, %6.3f, %d).\n",
    //        net_2, startX_2, startY_2, startZ_1, endX_2, endY_2, endZ_1);

    // Get the cell size (in microns) so we can convert micron units to cell units below:
    float microns_per_cell = user_inputs->cell_size_um;

    // Calculate coordinates of starting and ending terminals for both diff-pair nets.
    // Coordinates are in (integer) cell-units.
    int pseudo_start_X = (int)roundf((startX_1 + startX_2) / 2.0 / microns_per_cell);
    int pseudo_start_Y = (int)roundf((startY_1 + startY_2) / 2.0 / microns_per_cell);
    int pseudo_start_Z = startZ_1;
    int pseudo_end_X   = (int)roundf(( endX_1  +  endX_2 ) / 2.0 / microns_per_cell);
    int pseudo_end_Y   = (int)roundf(( endY_1  +  endY_2 ) / 2.0 / microns_per_cell);
    int pseudo_end_Z   = endZ_1;

    // Save the coordinates (in cell units) of the pseudo net's terminals in
    // the 'mapInfo' data structure:
    mapInfo->start_cells[pseudo_net_num].X = pseudo_start_X;
    mapInfo->start_cells[pseudo_net_num].Y = pseudo_start_Y;
    mapInfo->start_cells[pseudo_net_num].Z = pseudo_start_Z;
    mapInfo->start_cells[pseudo_net_num].flag = FALSE;
    mapInfo->end_cells[pseudo_net_num].X   = pseudo_end_X;
    mapInfo->end_cells[pseudo_net_num].Y   = pseudo_end_Y;
    mapInfo->end_cells[pseudo_net_num].Z   = pseudo_end_Z;
    mapInfo->end_cells[pseudo_net_num].flag = FALSE;

    // Save the layer-names of the pseudo-net's terminals:
    strcpy(user_inputs->start_layer[pseudo_net_num], user_inputs->routingLayerNames[pseudo_start_Z]);
    strcpy(user_inputs->end_layer[pseudo_net_num],   user_inputs->routingLayerNames[pseudo_end_Z]);

    // printf("DEBUG:   user_inputs->start_X_cells[pseudo_net_num=%d] = %d\n", pseudo_net_num, user_inputs->start_X_cells[pseudo_net_num]);
    // printf("DEBUG:   user_inputs->start_Y_cells[pseudo_net_num=%d] = %d\n", pseudo_net_num, user_inputs->start_Y_cells[pseudo_net_num]);
    // printf("DEBUG:   user_inputs->start_Z[pseudo_net_num=%d]       = %d\n", pseudo_net_num, user_inputs->start_Z[pseudo_net_num]);
    // printf("DEBUG:   user_inputs->end_X_cells[pseudo_net_num=%d]   = %d\n", pseudo_net_num, user_inputs->end_X_cells[pseudo_net_num]);
    // printf("DEBUG:   user_inputs->end_Y_cells[pseudo_net_num=%d]   = %d\n", pseudo_net_num, user_inputs->end_Y_cells[pseudo_net_num]);
    // printf("DEBUG:   user_inputs->end_Z[pseudo_net_num=%d]         = %d\n", pseudo_net_num, user_inputs->end_Z[pseudo_net_num]);

    // Save the micron coordinates of the pseudo net's terminals in the 'user_inputs' data structure:
    user_inputs->start_X_um[pseudo_net_num] = pseudo_start_X * microns_per_cell;
    user_inputs->start_Y_um[pseudo_net_num] = pseudo_start_Y * microns_per_cell;
    user_inputs->end_X_um[pseudo_net_num]   = pseudo_end_X   * microns_per_cell;
    user_inputs->end_Y_um[pseudo_net_num]   = pseudo_end_Y   * microns_per_cell;


    // Define names for the pseudo net (length limited to 'maxNetNameLength' characters):
    sprintf(user_inputs->net_name[pseudo_net_num], "_DIFF_PAIR_PSEUDO_NET_%04d", pseudo_net_num);
    printf("INFO: Pseudo net #%d is mapped to name '%s'\n", pseudo_net_num, user_inputs->net_name[pseudo_net_num]);

  }  // End of for-loop for index 'pseudo_net_num'

}  // End of function 'checkTerminalLocations'


//-----------------------------------------------------------------------------
// Name: mapDesignRuleSubsets
// Desc: Create 2-dimensional mapping structure 'user_inputs->designRuleSubsetMap'
//       that maps net numbers and design-rule sets to the correct design-rule
//       subset:
//
//        user_inputs->designRuleSubsetMap[net_num][DR_set_num] = DR_subset_num
//
//       Also populate the Boolean flags of the following 2-dimensional array to
//       reflect whether a design-rule subset is used by any nets:
//
//        user_inputs->DR_subsetUsed[DR_set_num][DR_subset_num] = TRUE or FALSE
//
//-----------------------------------------------------------------------------
void mapDesignRuleSubsets(InputValues_t *user_inputs)  {

  // Iterate through each path:
  for (int i = 0; i < user_inputs->num_nets; i++)  {
    //
    // Create 2-dimensional mapping structure 'user_inputs->designRuleSubsetMap' that maps
    // net numbers and design-rule sets to the correct design-rule subset:
    //
    //   user_inputs->designRuleSubsetMap[net_num][DR_set_num] = DR_subset_num
    //
    if (user_inputs->usesSpecialRule[i])  {

      // printf("DEBUG: For special net '%s' (#%d) with special design-rule '%s'\n",
      //         user_inputs->net_name[i], i, user_inputs->netSpecificRuleName[i] );
      // printf("       we're now looking for the matching exception name in each design-rule set.\n");

      // Iterate through each design-rule set to find the design-rule subset name
      // that matches the exception name associated with net #i:
      for (int DR_set = 0; DR_set < user_inputs->numDesignRuleSets; DR_set++)  {

        // Iterate through each design-rule subset:
        char subset_found = FALSE;
        for (int DR_subset = 0; DR_subset < user_inputs->numDesignRuleSubsets[DR_set]; DR_subset++)  {

          // printf("DEBUG: Checking DR set %d ('%s'), subset %d ('%s')\n", DR_set, user_inputs->designRuleSetName[DR_set],
          //                                                                DR_subset, user_inputs->designRules[DR_set][DR_subset].subsetName);

          // Compare the exception name specified for this net to the name of the design-rule subset.
          // If they match, then populate the 'designRuleSubsetMap' mapping structure:
          if (0 == strcasecmp(user_inputs->netSpecificRuleName[i], user_inputs->designRules[DR_set][DR_subset].subsetName))  {
            subset_found = TRUE;
            // printf("         We found the matching exception name in DR set %d, subset %d: '%s'.\n",
            //         DR_set, DR_subset, user_inputs->designRules[DR_set][DR_subset].subsetName);

            if (! user_inputs->designRules[DR_set][DR_subset].isPseudoNetSubset)
              user_inputs->designRuleSubsetMap[i][DR_set] = DR_subset;

            // Flag this design-rule subset as being used. (The user can define subsets that
            // are not used by any nets, and we want to know which ones are/aren't used
            // in order to reduce unnecessary calculations later on.) If we later find out
            // that design-rule set 'DR_set' is not used anywhere in the map, then we'll
            // later change the usage to FALSE for this combination of DR_set and DR_subset.
            user_inputs->DR_subsetUsed[DR_set][DR_subset] = TRUE;

            // If net is a diff-pair net, and if DR subset is 'isPseudoNetSubset', then
            // assign the pseudo-net associated with the diff-pair net to the DR subset:
            if (user_inputs->isDiffPair[i]  &&  user_inputs->designRules[DR_set][DR_subset].isPseudoNetSubset)  {
              // Get the net number of the pseudo net for this diff-pair net:
              int pseudo_net_number = user_inputs->diffPairToPseudoNetMap[i];

              // Assign the diff-pair's DR subset to the pseudo-net, too:
              user_inputs->designRuleSubsetMap[pseudo_net_number][DR_set] = DR_subset;
              // printf("DEBUG: Pseudo-net #%d ('%s') is mapped to design-rule subset #%d ('%s') within design-rule-set %d ('%s')\n",
              //        pseudo_net_number, user_inputs->net_name[pseudo_net_number], DR_subset,
              //        user_inputs->designRules[DR_set][DR_subset].subsetName, DR_set, user_inputs->designRuleSetName[DR_set]);
            }  // End of if-block for (isDiffPair[i] == TRUE)

            // printf("DEBUG: Net #%d ('%s') is mapped to design-rule subset #%d ('%s') within design-rule set #%d ('%s')\n",
            //         i, user_inputs->net_name[i], DR_subset, user_inputs->designRules[DR_set][DR_subset].subsetName,
            //         DR_set, user_inputs->designRuleSetName[DR_set]);
          }  // End of if-block for netSpecificRuleName == design-rule subset name
        }  // End of for-loop for index 'DR_subset' (0 to numDesignRuleSubsets)

        // If a matching design-rule subset was not found that matches the rule name
        // in the netlist, then issue a warning message to the user:
        if (subset_found == FALSE)  {
          printf("\nWARNING: The input file specified a special rule '%s' for net '%s', but no such\n",
                user_inputs->netSpecificRuleName[i], user_inputs->net_name[i]);
          printf(  "         rule was found within the design-rule set '%s'. The software will use\n",
                user_inputs->designRuleSetName[DR_set]);
          printf(  "         the default rules from design-rule set '%s', instead.\n\n",
                user_inputs->designRuleSetName[DR_set]);
        }  // End of if-block for (subset_found == FALSE)
      }  // End of for-loop for variable 'DR_num' (0 to numDesignRuleSets)
    }  // End of if-block for usesSpeciaRule == TRUE
  }  // End of for-loop for index 'i' (0 to numPaths)

}  // End of function 'mapDesignRuleSubsets'


//-----------------------------------------------------------------------------
// Name: calc_XYZ_cell_coordinates
// Desc: Convert the starting and ending (x,y) coordinates from microns to
//       cell units, and calculate the Z-coordinates based on the names of
//       the starting- and ending layer names.
//-----------------------------------------------------------------------------
void calc_XYZ_cell_coordinates(InputValues_t *user_inputs, const MapInfo_t *mapInfo)  {

  for (int i = 0; i < user_inputs->num_nets; i++)  {
    mapInfo->start_cells[i].X = (int)roundf(user_inputs->start_X_um[i] / user_inputs->cell_size_um);
    mapInfo->start_cells[i].Y = (int)roundf(user_inputs->start_Y_um[i] / user_inputs->cell_size_um);
    mapInfo->end_cells[i].X   = (int)roundf(user_inputs->end_X_um[i]   / user_inputs->cell_size_um);
    mapInfo->end_cells[i].Y   = (int)roundf(user_inputs->end_Y_um[i]   / user_inputs->cell_size_um);
    mapInfo->start_cells[i].flag = FALSE;
    mapInfo->end_cells[i].flag   = FALSE;

    // Temporarily set the Z-coordinate to zero for the terminals, so that we can call function
    // 'pointIsOutsideOfMap()' to check the (x,y) coordinates:
    mapInfo->start_cells[i].Z = 0;
    mapInfo->end_cells[i].Z = 0;

    // Confirm that the (x,y) coordinates of the start- and end-terminals are within the
    // perimeter of the map.
    if (pointIsOutsideOfMap(mapInfo->start_cells[i], mapInfo) || pointIsOutsideOfMap(mapInfo->end_cells[i], mapInfo))  {
      printf("\nERROR: Net #%d ('%s') has a starting or ending terminal that is outside of valid map perimeter. The boundaries\n",
             i, user_inputs->net_name[i]);
      printf("       are (0, 0) to (%5.1f, %5.1f), in microns. This path runs from (%5.1f, %5.1f) to (%5.1f, %5.1f).\n",
             mapInfo->mapWidth * user_inputs->cell_size_um, mapInfo->mapHeight * user_inputs->cell_size_um,
             mapInfo->start_cells[i].X * user_inputs->cell_size_um, mapInfo->start_cells[i].Y * user_inputs->cell_size_um,
             mapInfo->end_cells[i].X * user_inputs->cell_size_um, mapInfo->end_cells[i].Y * user_inputs->cell_size_um);
      printf("       Program is exiting.\n\n");
      exit(1);

    }  // End of if-block


    // Define Boolean flags that will be set to TRUE when the correct layer names
    // are found for the start- and end-terminals of net #i:
    int start_layer_found = FALSE, end_layer_found = FALSE;

    // Iterate through each layer name to determine which routing layer the net
    // starts and ends on:
    for (int j = 0; j < user_inputs->num_routing_layers; j++)  {
      // printf("\nDEBUG: Comparing the start- and end-layers for net %d against layer %d.\n", i, j);
      // printf("  DEBUG: Name of start-layer is '%s'. Name of end-layer is '%s'.\n\n",
      //       user_inputs->start_layer[i], user_inputs->end_layer[i]);

      //
      // Check the starting layer name:
      //
      // printf("DEBUG: *** Checking the start-layer ***\n");
      // printf("DEBUG: Comparing routingLayerNames[%d], user_inputs->start_layer[%d]\n", j, i);
      // printf("DEBUG: Comparing '%s' to '%s'...\n",
      //         user_inputs->routingLayerNames[j], user_inputs->start_layer[i]);

      if (strcmp(user_inputs->routingLayerNames[j], user_inputs->start_layer[i]) == 0)  {
        // printf("DEBUG: ...a match was found!\n");
        if (! start_layer_found) {
          mapInfo->start_cells[i].Z = j;
          start_layer_found = TRUE;
          // printf("DEBUG: start_cells[%d].Z has been assigned a value of %d.\n", i, j);
        }
        else  {
          printf("\nERROR: The list of layer names in the input file contains a duplicate name '%s'.\n",
                 user_inputs->start_layer[i]);
          printf("       Edit input file to make each name unique.\n\n");
          exit(1);
        }  // End of if/else-block
      }  // End of if-block for strcmp() == 0


      //
      // Check the ending layer name:
      //
      // printf("DEBUG: *** Checking the end-layer ***\n");
      // printf("DEBUG: Comparing routingLayerNames[%d], user_inputs->end_layer[%d]\n", j, i);
      // printf("DEBUG: Comparing '%s' to '%s'...\n",
      //         user_inputs->routingLayerNames[j], user_inputs->end_layer[i]);
      if (strcmp(user_inputs->routingLayerNames[j], user_inputs->end_layer[i]) == 0)  {
        // printf("DEBUG: ...a match was found!\n");
        if (! end_layer_found) {
          mapInfo->end_cells[i].Z = j;
          end_layer_found = TRUE;
          // printf("DEBUG: end_cells[%d].Z has been assigned a value of %d.\n", i, j);
        }
        else  {
          printf("\nERROR: The list of layer names in the input file contains a duplicate name '%s'.\n",
                  user_inputs->start_layer[i]);
          printf("       Edit input file to make each name unique.\n\n");
          exit(1);
        }  // End of if/else-block
      }  // End of if-block for strcmp() == 0

    }  // End of for-loop for variable 'j'

    // Check whether program found the start- and end-layers for each net:
    if (start_layer_found == FALSE  ||  end_layer_found == FALSE)  {
      printf("\nERROR: Failed to determine the start- or end-layer for net #%d ('%s).\n",
             i, user_inputs->net_name[i]);
      printf("       The start- and end-layer names for this net are '%s' and '%s', respectively.\n",
             user_inputs->start_layer[i], user_inputs->end_layer[i]);
      printf("       But one or both of these names is missing from the 'layer_names' statement in the input file.\n");
      printf("       Please correct the input file and re-start the program.\n\n");
      exit(1);
    }

    printf("DEBUG:   Net # %d: (%d,%d,%d) to (%d,%d,%d) in cell coordinates.\n", i,
            mapInfo->start_cells[i].X, mapInfo->start_cells[i].Y, mapInfo->start_cells[i].Z,
            mapInfo->end_cells[i].X,   mapInfo->end_cells[i].Y,   mapInfo->end_cells[i].Z);

  }  // End of for-loop for variable 'i' (0 to num_nets)

}  // End of function 'calc_XYZ_cell_coordinates'


//-----------------------------------------------------------------------------
// Name: getDiffPairPartnerAndPitch
// Desc:  For each net #i that is part of a differential pair, determine the
//        number j of the net's partner and save this in variable
//        'user_inputs->diffPairPartner[i] = j'.  Also, for each diff-pair net,
//        assign the pitch (in microns and cell units) for each design-rule set.
//-----------------------------------------------------------------------------
void getDiffPairPartnerAndPitch(InputValues_t *user_inputs)  {


  // Iterate through the nets, locate the diff-pair nets, and then locate the partner net for
  // each diff-pair net:
  for (int i = 0; i < user_inputs->num_nets; i++)  {
    if (user_inputs->isDiffPair[i] == TRUE)  {
      // printf("DEBUG: Net #%d (named '%s') is part of a differential pair with partner net name '%s'.\n", i,
      //         user_inputs->net_name[i], user_inputs->diffPairPartnerName[i]);

      // Based on the name of the diff-pair partner (e.g., 'TX_p'), determine the *number* of this partner net:
      int partner_net_number = -1;  // Initialize partner net number to nonsensical value
      for (int j = 0; j < user_inputs->num_nets; j++)  {
        if (strcmp(user_inputs->diffPairPartnerName[i], user_inputs->net_name[j]) == 0)  {
          partner_net_number = j;
          break;  // Break out of for-loop because we found a matching net name
        }  // End of if-block for diffPairPartnerName[i] == net_name[j]
      }  // End of for-loop for index 'j' (from 0 to num_nets)

      // If partner net name was not found in the list of nets, then issue a fatal error
      // message and terminate the program:
      if (partner_net_number == -1)  {
        printf("\nERROR: Net '%s' is defined to be part of a differential pair with partner net name '%s'.\n",
                user_inputs->net_name[i], user_inputs->diffPairPartnerName[i]);
        printf("       However, net name '%s' is not defined in the list of nets in the input file.\n",
                user_inputs->diffPairPartnerName[i]);
        printf("       Fix this discrepancy in the input file and restart the program.\n\n");
        exit(1);
      }  // End of if-block for partner_net_number == -1

      // Assign the partner net number to the 'diffPairPartner[i]' variable:
      user_inputs->diffPairPartner[i] = partner_net_number;

      // printf("DEBUG:    Net #%d (named '%s') is part of a differential pair with partner net name '%s', which is net #%d.\n", i,
      //         user_inputs->net_name[i], user_inputs->diffPairPartnerName[i], user_inputs->diffPairPartner[i]);

      // For each design-rule set, determine (a) which design-rule subset name matches the diff-pair rule
      // from the net list, and (b) what the pitch is for that diff-pair rule:
      for (int DR_set_number = 0; DR_set_number < user_inputs->numDesignRuleSets; DR_set_number++)  {
        int diff_pair_subset_number = -1; // Initialize subset number to a nonsensical value.
        for (int DR_subset_number = 0; DR_subset_number < user_inputs->numDesignRuleSubsets[DR_set_number]; DR_subset_number++)  {

          // Compare the diff-pair rule name from the netlist to the name of the design-rule subset name:
          if (strcmp(user_inputs->netSpecificRuleName[i], user_inputs->designRules[DR_set_number][DR_subset_number].subsetName) == 0)  {

            // We found a name-match, so capture the design-rule subset number:
            diff_pair_subset_number = DR_subset_number;

            // printf("DEBUG: In function 'getDiffPairPartnerAndPitch', net-specific rule '%s' for path %d matches subset name '%s' of set %d, subset %d.\n",
            //        user_inputs->netSpecificRuleName[i], i, user_inputs->designRules[DR_set_number][DR_subset_number].subsetName, DR_set_number, DR_subset_number);

            // Break out of inner for-loop because we found a matching DR subset name:
            break;

          }  // End of if-block for matching names of DR subset and diff-pair rule from netlist

        }  // End of for-loop for index 'DR_subset_number (from 0 to numDesignRuleSubsets)

        // Confirm that a realistic DR subset number was found among the design-rule subsets.
        // If not, then issue a fatal error message and terminate:
        if (diff_pair_subset_number < 0)  {
          printf("\nERROR: For net #%d ('%s'), which uses diff-pair rule '%s',\n",
                  i, user_inputs->net_name[i], user_inputs->netSpecificRuleName[i]);
          printf("       no diff-pair pitch was defined in design-rule set '%s'.\n",
                  user_inputs->designRuleSetName[DR_set_number]);
          printf("       Fix the input text file and restart the program. Program is\n");
          printf("       terminating.\n\n");
          exit(1);
        }  // End of if-block for nonsensical value of pitch_microns

        else  {
          //
          // printf("DEBUG: In function 'getDiffPairPartnerAndPitch', setting user_inputs->diffPairPitchMicrons[%d][%d] = user_inputs->designRules[%d][%d].traceDiffPairPitchMicrons (= %6.3f)\n",
          //        i, DR_set_number, DR_set_number, diff_pair_subset_number, user_inputs->designRules[DR_set_number][diff_pair_subset_number].traceDiffPairPitchMicrons);
          // printf("DEBUG: In function 'getDiffPairPartnerAndPitch', setting user_inputs->diffPairPitchCells[%d][%d] = user_inputs->designRules[%d][%d].diffPairPitchCells (= %d)\n",
          //        i, DR_set_number, DR_set_number, diff_pair_subset_number, user_inputs->designRules[DR_set_number][diff_pair_subset_number].diffPairPitchCells);

          user_inputs->diffPairPitchMicrons[i][DR_set_number] = user_inputs->designRules[DR_set_number][diff_pair_subset_number].traceDiffPairPitchMicrons;
          user_inputs->diffPairPitchCells[i][DR_set_number]   = user_inputs->designRules[DR_set_number][diff_pair_subset_number].diffPairPitchCells[TRACE];
          // printf("DEBUG: assigned user_inputs->diffPairPitchCells[%d][%d] to user_inputs->designRules[%d][%d].diffpairPitchCells[TRACE], which is %.3f.\n",
          //        i, DR_set_number, DR_set_number, diff_pair_subset_number, user_inputs->diffPairPitchCells[i][DR_set_number]);
        }  // End of else-block

      }  // End of for-loop for index 'DR_set_number' (from 0 to numDesignRuleSets)

    }  // End of if-block for isDiffPair[i] == TRUE
    else  {
      if (user_inputs->isDiffPair[i] == FALSE)  {
        // printf("DEBUG: Net #%d is NOT part of a differential pair.\n", i);
      }
      else  {
        printf("\nERROR: The variable 'isDiffPair[%d]' has a value of '%d', which is not expected.\n", i, user_inputs->isDiffPair[i]);
        printf("       This error indicates a problem in the software. Inform the software developer of\n");
        printf("       this error message. Program is terminating.\n\n");
        exit(1);
      }  // End of else-clause (isDiffPair != FALSE)

    }  // End of else-block (isDiffPair[i] != TRUE)
  }  // End of for-loop for index 'i' (0 to num_nets)
}  // End of function 'getDiffPairPartnerAndPitch'


//-----------------------------------------------------------------------------
// Name: calc_congestion_adder
// Desc: Calculate a floating-point 'adder' by which a congestion radius is
//       augmented to ensure that a foreign path-center passing through a
//       discrete cell just beyond a congestion radius of a path-center would
//       not cause a design-rule violation between the two path-centers. The
//       square of the DRC radius between the path-centers is DRC_radius_squared.
//       All units are in units of cells or cells^2.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calc_congestion_adder' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_calc_congestion_adder 1
#undef DEBUG_calc_congestion_adder
//
float calc_congestion_adder(float shape_radius, float baseline_cong_radius, float DRC_radius_squared)  {

  // Define adder in cell-units that is returned from this function:
  float adder = 0.0;

  #ifdef DEBUG_calc_congestion_adder
  printf("\nDEBUG: Entered function calc_congestion_adder with input values:\n");
  printf("DEBUG:            shape_radius = %.3f\n", shape_radius);
  printf("DEBUG:    baseline_cong_radius = %.3f\n", baseline_cong_radius);
  printf("DEBUG:      DRC_radius_squared = %.3f\n\n", DRC_radius_squared);
  #endif

  // Get set of points on the perimeter of the overlap quarter-circle:
  Coordinate_t *perimeter_cells;
  perimeter_cells = malloc(0 * sizeof(Coordinate_t));
  if (perimeter_cells == NULL)  {
    printf("\nERROR: Unable to allocate memory for array 'perimeter_cells' in function calc_congestion_adder. Please notify\n");
    printf(  "       the software developer of this fatal error message.\n\n");
    exit(1);
  }

  // Define variable to track the number of discrete cells found within a couple of cells within
  // the shape_radius from the path-center at (x,y) = (0,0)
  int num_perimeter_cells = 0;

  // Calculate the square of the shape's radius:
  float shape_radius_squared = shape_radius * shape_radius;

  // Define a radius that's 2 cells smaller than the shape's radius. Save the square of this smaller radius:
  float inner_shape_radius = max(shape_radius - 2.0, 0.0);
  float inner_shape_radius_squared = inner_shape_radius * inner_shape_radius;

  for (int x = 0; x <= shape_radius + 2; x++)  {
    int x_squared = x * x;
    for (int y = 0; y <= shape_radius + 2; y++)  {
      int y_squared = y * y;
      if (   (x_squared + y_squared <= shape_radius_squared)
          && (x_squared + y_squared >= inner_shape_radius_squared))  {

        // We got here, so the point at (x,y) represents a cell that's part of the shape
        // with path-center at the origin, and is within a couple cells of the perimeter
        // of this shape. Add this cell's coordinates to the 'perimeter_cells' array:
        perimeter_cells = realloc(perimeter_cells, (num_perimeter_cells + 1) * sizeof(Coordinate_t));

        perimeter_cells[ num_perimeter_cells ].X = x;
        perimeter_cells[ num_perimeter_cells ].Y = y;
        num_perimeter_cells++;
      }  // End of if-block
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'x'

  #ifdef DEBUG_calc_congestion_adder
  printf("DEBUG: Found %d cells near edge of shape within %.2f of path-center: ", num_perimeter_cells, shape_radius);
  for (int i = 0; i < num_perimeter_cells; i++)  {
    printf("(%d,%d) ", perimeter_cells[i].X, perimeter_cells[i].Y);
  }
  printf("\n\n");
  #endif

  // Define a Boolean flag that will be set to TRUE if an 'adder' value is found that prevents
  // design-rule violations when path-centers are spaced a congestion-radius from each other:
  unsigned char correct_adder_found = FALSE;

  // Loop until a value for 'adder' is found that satisfies the requirements:
  while (! correct_adder_found ) {

    // Define provional values for the congestion radius and its square, taking into
    // account the 'adder' addition:
    float cong_radius = baseline_cong_radius + adder;
    float cong_radius_squared = cong_radius * cong_radius;

    // Set Boolean flag to TRUE, but it will be negated if the adder is not
    // large enough.
    correct_adder_found = TRUE;

    // Define a radius that's 2 cells larger than the current congestion radius. Save the square
    // of this larger radius:
    float outer_cong_radius = cong_radius + 2.0;
    float outer_cong_radius_squared = outer_cong_radius * outer_cong_radius;

    // Iterate over the points just beyond the congestion radius:
    for (int x = 0; x <= outer_cong_radius; x++)  {
      int x_squared = x * x;
      for (int y = 0; y <= outer_cong_radius; y++)  {
        int y_squared = y * y;
        if (   (x_squared + y_squared > cong_radius_squared)
            && (x_squared + y_squared <= outer_cong_radius_squared))  {

          // We found a point just outside the congestion radius. Now check if it's within a
          // DRC_radius from any points within a shape-radius of the path-center at the origin:
          for (int i = 0; i < num_perimeter_cells; i++)  {
            if (  (x - perimeter_cells[ i ].X) * (x - perimeter_cells[ i ].X)
                + (y - perimeter_cells[ i ].Y) * (y - perimeter_cells[ i ].Y)
                < DRC_radius_squared )   {

              // We got here, so a point just outside the congestion radius is within a DRC radius
              // of a point within a shape-radius of the path-center. This means that the congestion
              // radius is not large enough, and needs to be augmented with a larger 'adder'.
              correct_adder_found = FALSE;

              adder = adder + 0.1;  // Increment the adder by 0.1 cells

              #ifdef DEBUG_calc_congestion_adder
              printf("\nDEBUG: In function calc_congestion_adder, the point (%d,%d), which is just outside the congestion radius of %.3f cells,\n",
                     x, y, cong_radius);
              printf("DEBUG: is within a DRC-radius (%.3f cells ) of point (%d,%d), which is within a half-width (%.3f cells) of a path-center\n",
                     sqrt(DRC_radius_squared), perimeter_cells[i].X, perimeter_cells[i].Y, shape_radius);
              printf("DEBUG: at (0,0). The congestion adder will therefore be increased from %.3f to %.3f\n", adder - 0.1, adder);
              #endif

              // Check if the 'adder' variable has become unrealistically large due to an error in the
              // input variables to this function. If the adder exceeds the sum of the shape_radius and
              // the baseline_cong_radius, then issue a fatal error message:
              if (adder > baseline_cong_radius + shape_radius)  {
                printf("\nERROR: An unexpected problem was detected in function calc_congestion_adder, in which the 'adder' value (%.3f cells) exceeded\n", adder);
                printf(  "       exceeded the sum of the baseline congestion distance (%.3f cells) plus the shape-radius (%.3f cells). Please\n",
                       baseline_cong_radius, shape_radius);
                printf(  "       notify the software developer of this fatal error message.\n\n");
                exit(1);
              }

              break;  // Break out of for-loop for index 'i'
            }  // End of if-block

          }  // End of for-loop for index 'i'

        }  // End of if-block

        if ( ! correct_adder_found )
          break;  // Break out of for-loop for index 'y'
      }  // End of for-loop for index 'y'

      if ( ! correct_adder_found )
        break;  // Break out of for-loop for index 'x'
    }  // End of for-loop for index 'x'

  } // End of while-loop

  #ifdef DEBUG_calc_congestion_adder
  printf("\nDEBUG: Found adder value %.3f at end of function calc_congestion_adder\n\n", adder);
  #endif

  // Free the memory allocated in this function:
  free(perimeter_cells);
  perimeter_cells = NULL;

  // Return the adder to the calling routine:
  return(adder);

}  // End of function 'calc_congestion_adder'


//-----------------------------------------------------------------------------
// Name: calc_diffPair_design_rules
// Desc: Calculate optimized values of the diff-pair half-pitches of two shapes,
//       A and B, accounting for rounding errors due to discretization of continuous,
//       user-defined values into approximated, discrete, grid-based values.
//
//       Inputs to the function are:
//         o  Reference to the nominal diff-pair half-pitch of shape A, in cell units
//            (* diffPair_halfPitch_A)
//         o  Reference to the nominal diff-pair half-pitch of shape B, in cell units
//            (* diffPair_halfPitch_B)
//         o  Nominal diff-pair shape half-width (radius) for shape A, in cell units
//            (diffPair_shapeRadius_A)
//         o  Nominal diff-pair shape half-width (radius) for shape B, in cell units
//            (diffPair_shapeRadius_B)
//         o  DRC_radius[A][B] between diff-pair partner-shapes A and B, which is equal
//            to radius[B] + spacing[A][B]. The variable name is DRC_radius_AB.
//         o  DRC_radius[B][A] between diff-pair partner-shapes A and B, which is equal
//            to radius[A] + spacing[A][B]. The variable name is DRC_radius_BA.
//
//       Outputs from the function are:
//         o  Reference to the adjusted diff-pair half-pitch for shape A, including any
//            necessary adder to avoid design-rule violations, in cell units
//            (* diffPair_halfPitch_A)
//         o  Reference to the adjusted diff-pair half-pitch for shape B, including any
//            necessary adder to avoid design-rule violations, in cell units
//            (* diffPair_halfPitch_B)
//         o  Reference to the half-width (radius) of the pseudo-structure using shape A,
//            i.e., the largest distance from the pseudo-path centerline that is occupied
//            by shape A (* pseudo_halfWidth_A)
//         o  Reference to the half-width (radius) of the pseudo-structure using shape B,
//            i.e., the largest distance from the pseudo-path centerline that is occupied
//            by shape B (* pseudo_halfWidth_B)
//
//       If the function attempts to increase the diff-pair half-pitch values
//       or the pseudo-half-width values by more than 'max_adder_value_cells',
//       the program issues a fatal error message and exits. This prevents the
//       function from executing an infinite loop.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calc_diffPair_design_rules' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_calc_diffPair_design_rules 1
#undef DEBUG_calc_diffPair_design_rules
//
void calc_diffPair_design_rules(float diffPair_shapeRadius_A, float diffPair_shapeRadius_B,
                                float DRC_radius_AB, float DRC_radius_BA,
                                float * diffPair_halfPitch_A, float * diffPair_halfPitch_B,
                                float * pseudo_halfWidth_A, float * pseudo_halfWidth_B,
                                float max_adder_value_cells)  {

  #ifdef DEBUG_calc_diffPair_design_rules
  printf("\nDEBUG: Entered function calc_diffPair_design_rules with following inputs:\n");
  printf(  "DEBUG:   diffPair_shapeRadius_A = %.3f cells\n", diffPair_shapeRadius_A);
  printf(  "DEBUG:   diffPair_shapeRadius_B = %.3f cells\n", diffPair_shapeRadius_B);
  printf(  "DEBUG:    *diffPair_halfPitch_A = %.3f cells\n", *diffPair_halfPitch_A);
  printf(  "DEBUG:    *diffPair_halfPitch_B = %.3f cells\n", *diffPair_halfPitch_B);
  printf(  "DEBUG:            DRC_radius_AB = %.3f cells\n", DRC_radius_AB);
  printf(  "DEBUG:            DRC_radius_BA = %.3f cells\n", DRC_radius_BA);
  printf(  "DEBUG:      *pseudo_halfWidth_A = %.3f cells\n",   *pseudo_halfWidth_A);
  printf(  "DEBUG:      *pseudo_halfWidth_B = %.3f cells\n\n", *pseudo_halfWidth_B);
  #endif

  //
  // If the radius value of either shape-A or shape-B is less than 1.0 cell, then increment
  // the radius to 1.0 cell so that this function is consistent with assumptions in
  // function createOneContiguousPath, which inserts intermediate cells if the
  // linewidth is less than 2.0 cells:
  //
  const float min_linewidth_to_insert_cells = 2.0;
  const float min_allowable_shape_radius = 0.5 * min_linewidth_to_insert_cells;
  if (diffPair_shapeRadius_A < min_allowable_shape_radius)  {

    // Capture the amount by which the shape's radius must be increased in this function:
    float increase_for_shape_A = min_allowable_shape_radius - diffPair_shapeRadius_A;

    // Increase DRC_radius_BA by the amount that we increase the nominal diff-pair
    // half-width (radius) for shape-A:
    DRC_radius_BA += increase_for_shape_A;

	// Increase shape-A's radius to the minimum allowable value:
    diffPair_shapeRadius_A = min_allowable_shape_radius;

    #ifdef DEBUG_calc_diffPair_design_rules
    printf(  "DEBUG:   diffPair_shapeRadius_A increased to %.3f cells, which is the minimum allowed.\n", min_allowable_shape_radius);
    printf(  "DEBUG:   DRC_radius_BA increased from %.3f to %.3f cells to account for shape-A's increase.\n\n", DRC_radius_BA - increase_for_shape_A,
           DRC_radius_BA);
    #endif
  }
  if (diffPair_shapeRadius_B < min_allowable_shape_radius)  {

    // Capture the amount by which the shape's radius must be increased in this function:
    float increase_for_shape_B = min_allowable_shape_radius - diffPair_shapeRadius_B;

    // Increase DRC_radius_AB by the amount that we increase the nominal diff-pair
    // half-width (radius) for shape-B:
    DRC_radius_AB += increase_for_shape_B;

    // Increase shape-B's radius to the minimum allowable value:
    diffPair_shapeRadius_B = min_allowable_shape_radius;

    #ifdef DEBUG_calc_diffPair_design_rules
    printf(  "DEBUG:   diffPair_shapeRadius_B increased to %.3f cells, which is the minimum allowed.\n", min_allowable_shape_radius);
    printf(  "DEBUG:   DRC_radius_AB increased from %.3f to %.3f cells to account for shape-B's increase.\n\n", DRC_radius_AB - increase_for_shape_B,
           DRC_radius_AB);
    #endif
  }

  // Make local copies of the shapes' nominal half-pitches and pseudo-structure's nominal half-widths:
  float nominal_halfPitch_A = *diffPair_halfPitch_A; // In cell units
  float nominal_halfPitch_B = *diffPair_halfPitch_B; // In cell units
  #ifdef DEBUG_calc_diffPair_design_rules
  float nominal_pseudo_halfWidth_A = *pseudo_halfWidth_A;   // In cell units, used for debugging only
  float nominal_pseudo_halfWidth_B = *pseudo_halfWidth_B;   // In cell units, used for debugging only
  #endif

  // Define adder in cell-units that is added to the nominal half-pitches, if necessary:
  float adder = 0.0;

  // Define a Boolean flag that will be set to TRUE if a half-pitch value is found that prevents
  // design-rule violations when diff-pair path-centers are spaced at distances of a
  // half-pitch from the centerline of the pseudo-path:
  unsigned char correct_halfPitches_found = FALSE;

  // Define six variables and four x/y coordinates that will result from the following while-loop:
  float min_centerline_radius_A;  // Minimum distance from pseudo-path's center-line to centerline for shape A
  float min_centerline_radius_B;  // Minimum distance from pseudo-path's center-line to centerline for shape B
  float min_shape_radius_A;       // Minimum distance from pseudo-path's center-line to shape A of diff-pair
  float min_shape_radius_B;       // Minimum distance from pseudo-path's center-line to shape B of diff-pair
  float max_shape_radius_A;       // Maximum distance from pseudo-path's center-line to shape A of diff-pair
  float max_shape_radius_B;       // Maximum distance from pseudo-path's center-line to shape B of diff-pair

  // X/Y coordinates of the center-line point that's closest to the pseudo-path's center for shapes A and B:
  unsigned short X_CL_A, Y_CL_A, X_CL_B, Y_CL_B;

  // X/Y coordinates of the point within shapes A and B that are closest to the pseudo-path's center:
  unsigned short X_shape_A, Y_shape_A, X_shape_B, Y_shape_B;

  // Loop until a value for the half-pitch is found that satisfies the requirements:
  while (! correct_halfPitches_found )  {

    // Set Boolean flag to TRUE, but it will be negated if the half-pitch is not
    // large enough.
    correct_halfPitches_found = TRUE;

    // Define variable half-pitch values for shape A and B. Each is the sum of the user-supplied nominal half-pitch
    // plus an adder that starts at 0.0, but increases as necessary to avoid design-rule violations:
    float current_halfPitch_A = nominal_halfPitch_A + adder;
    float current_halfPitch_B = nominal_halfPitch_B + adder;


    // Create an N x N array of bytes whose elements represent cells in the map on a single layer.
    // The dimension N is large enough to hold the half of the larger of the two pseudo-structure, i.e.,
    // the larger of the following two values:
    //   (* diffPair_halfPitch_A)  +  (diffPair_shapeRadius_A / 2)  +  6 cells, or
    //   (* diffPair_halfPitch_B)  +  (diffPair_shapeRadius_B / 2)  +  6 cells,
    // where the 6 cells are added to account for potential rounding errors. Four Boolean
    // elements in each byte will specify:
    //   a) Bit #0: whether the cell contains a centerline of a diff-pair shape A,
    //   b) Bit #1: whether the cell contains a centerline of a diff-pair shape B,
    //   c) Bit #2: whether the cell contains the diff-pair shape A, and
    //   d) Bit #3: whether the cell contains the diff-pair shape B.
    int max_size = 6 + max(current_halfPitch_A  +  diffPair_shapeRadius_A / 2,
                           current_halfPitch_B  +  diffPair_shapeRadius_B / 2);
    unsigned char ** cells;
    cells = malloc(max_size * sizeof(unsigned char *));
    if (cells == NULL)  {
      printf("\nERROR: Unable to allocate memory for 'cells' 2-dimensional array in function calc_diffPair_design_rules.\n");
      printf(  "       Please inform the software developer of this fatal error message.\n\n");
      exit(1);
    }  // End of if-block for fatal error message
    for (int i = 0; i < max_size; i++)  {
      cells[i] = malloc(max_size * sizeof(unsigned char));
      if (cells[i] == NULL)  {
        printf("\nERROR: Unable to allocate memory for second dimension of 'cells[%d]' array in function calc_diffPair_design_rules.\n", i);
        printf(  "       Please inform the software developer of this fatal error message.\n\n");
        exit(1);
      }  // End of if-block for fatal error message

      // Initialize all elements to zero:
      for (int j = 0; j < max_size; j++)  {
        cells[i][j] = 0;
      }  // End of for-loop for index 'j'
    }  // End of for-loop for index 'i'

    //
    // Using polar coordinates, find the cells in the two-dimensional matrix that would be
    // centerlines of the diff-pair shape A. First, define a delta-theta, in radians,
    // that specifies how fine the angular resolution will be. This is chosen so that the
    // arc-length is 0.1 cells ('small_cell_fraction') at a distance of the diffPair_half_pitch:
    //
    const float small_cell_fraction = 0.05;
    float delta_theta_A = small_cell_fraction / current_halfPitch_A;

    //
    // Next, vary theta from 0 to 90 degrees to locate coordinates that could be path-centers.
    // (Because of the 8-fold symmetry of the X/Y coordinate system, we could limit theta's
    // range to only 45 degrees, or PI/4.)
    //
    for (float theta_A = 0; theta_A <= M_PI/2.0; theta_A += delta_theta_A)  {
      int x = (int)round(current_halfPitch_A * cos(theta_A));
      int y = (int)round(current_halfPitch_A * sin(theta_A));
      #ifdef DEBUG_calc_diffPair_design_rules
      printf("DEBUG:   For theta = %.2f radians (%.1f degrees), (x,y) = (%d,%d) is a path-center for shape A.\n",
             theta_A, theta_A * 180.0 / M_PI, x, y);
      #endif

      // Check for illegal values of (x,y) that would cause a segmentation fault
      // when used as indices in the 'cells' 2-dimensional array:
      if ((x < 0) || (x >= max_size) || (y < 0) || (y >= max_size))  {
        printf("\nERROR: An unexpected (x,y) coordinate of (%d,%d) was calculated in function calc_diffPair_design_rules.\n",
               x, y);
        printf(  "       for shape 'A'. The allowed range for this coordinate is (0,0) to (%d,%d). Please inform the software\n",
               max_size, max_size);
        printf(  "       developer of this fatal error message. The value of theta_A was %.3f radians. The value of\n",
               theta_A);
        printf(  "       current_halfPitch_A was %.3f cells\n\n", current_halfPitch_A);
        exit(1);
      }

      // Set the bit #0 of the 'cells[][]' element to 1, indicating that this
      // (x,y) coordinate is a path-center coordinate for the diff-pair:
      cells[x][y] = cells[x][y] | 1;  // Use bitwise 'OR' to set bit #0
    }  // End of for-loop for index 'theta_A'

    //
    // Repeat the above loop for shape-type 'B', thereby flagging the path-centers of
    // this shape-type:
    //
    float delta_theta_B = small_cell_fraction / current_halfPitch_B;
    for (float theta_B = 0; theta_B <= M_PI/2.0; theta_B += delta_theta_B)  {
      int x = (int)round(current_halfPitch_B * cos(theta_B));
      int y = (int)round(current_halfPitch_B * sin(theta_B));
      #ifdef DEBUG_calc_diffPair_design_rules
      printf("DEBUG:   For theta = %.2f radians (%.1f degrees), (x,y) = (%d,%d) is a path-center for shape B.\n",
             theta_B, theta_B * 180.0 / M_PI, x, y);
      #endif

      // Check for illegal values of (x,y) that would cause a segmentation fault
      // when used as indices in the 'cells' 2-dimensional array:
      if ((x < 0) || (x >= max_size) || (y < 0) || (y >= max_size))  {
        printf("\nERROR: An unexpected (x,y) coordinate of (%d,%d) was calculated in function calc_diffPair_design_rules.\n",
               x, y);
        printf(  "       for shape 'B'. The allowed range for this coordinate is (0,0) to (%d,%d). Please inform the software\n",
               max_size, max_size);
        printf(  "       developer of this fatal error message. The value of thetaB was %.3f radians. The value of\n",
               theta_B);
        printf(  "       current_halfPitch_B was %.3f cells\n\n", current_halfPitch_B);
        exit(1);
      }

      // Set the bit #1 of the 'cells[][]' element to 1, indicating that this
      // (x,y) coordinate is a path-center coordinate for the diff-pair:
      cells[x][y] = cells[x][y] | 2;  // Use bitwise 'OR' to set bit #1
    }  // End of for-loop for index 'theta_B'

    //
    // Now that we know the (x,y) coordinates of each pseudo-path's centerline, flag each of
    // the cells within a diff-pair shape-radius of these cells to indicate that the flagged
    // cells are part of the diff-pair shape:
    //
    // Define squares of the diff-pair shape radius values
    float shapeRadius_squared_A = diffPair_shapeRadius_A * diffPair_shapeRadius_A;
    float shapeRadius_squared_B = diffPair_shapeRadius_B * diffPair_shapeRadius_B;
    for (int x = 0; x < max_size; x++)  {
      for (int y = 0; y < max_size; y++)  {

        //
        // First, check if the cell at (x,y) contains path-center of the diff-pair path 'A' by
        // logically AND'ing the value with 1 (thereby checking only bit #0):
        //
        if (cells[x][y] & 1)  {
          // We got here, so the cell at (x,y) contains a path-center of the diff-pair shape 'A'.
          // We therefore raster around this (x,y) location and flag all cells within a
          // distance of 'diffPair_shapeRadius_A':
          int raster_distance_A = (int)(diffPair_shapeRadius_A + 1.0);
          for (int x_prime = x - raster_distance_A; x_prime <= x + raster_distance_A; x_prime++)  {

            // Confirm that the x_prime value is not outside the allowed range:
            if ((x_prime < 0) || (x_prime >= max_size))  {
              continue;  // Out of range, so move on to next x_prime value
            }

            int delta_x_squared = (x_prime - x) * (x_prime - x);
            for (int y_prime = y - raster_distance_A; y_prime <= y + raster_distance_A; y_prime++)  {

              // Confirm that the y_prime value is not outside the allowed range:
              if ((y_prime < 0) || (y_prime >= max_size))  {
                continue;  // Out of range, so move on to next y_prime value
              }

              int distance_squared = delta_x_squared + (y_prime - y) * (y_prime - y);
              if (distance_squared <= shapeRadius_squared_A)  {
                // We got here, so coordinate (x_prime, y_prime) is within a distance of
                // diffPair_shapeRadius_A of coordinate (x,y). The former coordinate is
                // therefore within the diff-pair shape 'A'. So flag the 'cells[][]' element
                // at coordinate (x_prime, y_prime) by setting bit #2 of the 8-bit byte:
                cells[x_prime][y_prime] = cells[x_prime][y_prime] | 4;  // Bit-wise 'OR' with '4' to set bit #2

              }  // End of if-block for (distance_squared <= shapeRadius_squared_A)
            }  // End of for-loop for index 'y_prime'
          }  // End of for-loop for index 'x_prime'
        }  // End of if-block for cell containing a path-center of shape 'A'

        //
        // Second, check if the cell at (x,y) contains path-center of the diff-pair path 'B' by
        // logically AND'ing the value with 2 (thereby checking only bit #1):
        //
        if (cells[x][y] & 2)  {
          // We got here, so the cell at (x,y) contains a path-center of the diff-pair shape 'B'.
          // We therefore raster around this (x,y) location and flag all cells within a
          // distance of 'diffPair_shapeRadius_B':
          int raster_distance_B = (int)(diffPair_shapeRadius_B + 1.0);
          for (int x_prime = x - raster_distance_B; x_prime <= x + raster_distance_B; x_prime++)  {

            // Confirm that the x_prime value is not outside the allowed range:
            if ((x_prime < 0) || (x_prime >= max_size))  {
              continue;  // Out of range, so move on to next x_prime value
            }

            int delta_x_squared = (x_prime - x) * (x_prime - x);
            for (int y_prime = y - raster_distance_B; y_prime <= y + raster_distance_B; y_prime++)  {

              // Confirm that the y_prime value is not outside the allowed range:
              if ((y_prime < 0) || (y_prime >= max_size))  {
                continue;  // Out of range, so move on to next y_prime value
              }

              int distance_squared = delta_x_squared + (y_prime - y) * (y_prime - y);
              if (distance_squared <= shapeRadius_squared_B)  {
                // We got here, so coordinate (x_prime, y_prime) is within a distance of
                // diffPair_shapeRadius_B of coordinate (x,y). The former coordinate is
                // therefore within the diff-pair shape 'B'. So flag the 'cells[][]' element
                // at coordinate (x_prime, y_prime) by setting bit #3 of the 8-bit byte:
                cells[x_prime][y_prime] = cells[x_prime][y_prime] | 8;  // Bit-wise 'OR' with '8' to set bit #3

              }  // End of if-block for (distance_squared <= shapeRadius_squared_B)
            }  // End of for-loop for index 'y_prime'
          }  // End of for-loop for index 'x_prime'
        }  // End of if-block for cell containing a path-center of shape 'B'

      }  // End of for-loop for index 'y'
    }  // End of for-loop for index 'x'


    //
    // Now that the cells[][] array is populated with the locations of path-centers
    // and shape-regions, we next raster over this array to find six values:
    //   (a) the coordinate (X_CL_A, Y_CL_A) that represents the closest point to the
    //       origin that's flagged as a centerline of shape 'A'. This distance is the
    //       minimum centerline radius for shape A, or min_centerline_radius_A.
    //   (b) the coordinate (X_CL_B, Y_CL_B) that represents the closest point to the
    //       origin that's flagged as a centerline of shape 'B'. This distance is the
    //       minimum centerline radius for shape B, or min_centerline_radius_B.
    //   (c) the coordinate (X_shape_A, Y_shape_A) that represents the closest point
    //       to the origin that's flagged as within 'A'. This distance is the minimum
    //       shape radius for shape A, or min_shape_radius_A.
    //   (d) the coordinate (X_shape_B, Y_shape_B) that represents the closest point
    //       to the origin that's flagged as within 'B'. This distance is the minimum
    //       shape radius for shape B, or min_shape_radius_B.
    //   (e) the coordinate that represents the farthest point to the origin that's flagged
    //       as within 'A'. This distance is the maximum shape radius for shape A, or
    //       max_shape_radius_A.
    //   (f) the coordinate that represents the farthest point to the origin that's flagged
    //       as within 'B'. This distance is the maximum shape radius for shape B, or
    //       max_shape_radius_B.
    X_CL_A    = 0;   Y_CL_A    = 0;
    X_CL_B    = 0;   Y_CL_B    = 0;
    X_shape_A = 0;   Y_shape_A = 0;
    X_shape_B = 0;   Y_shape_B = 0;
    min_centerline_radius_A = (float)max_size;
    min_centerline_radius_B = (float)max_size;
    min_shape_radius_A      = (float)max_size;
    min_shape_radius_B      = (float)max_size;
    max_shape_radius_A      = 0.0;
    max_shape_radius_B      = 0.0;
    for (int x = 0; x < max_size; x++)  {
      int x_squared = x * x;
      for (int y = 0; y < max_size; y++)  {
        // Calculate distance to origin:
        float distance = sqrt(x_squared  +  y * y);

        // Check if (x,y) cell is flagged as a path-center for shape A:
        if (cells[x][y] & 1)  {
          // Cell at (x,y) is a path-center for shape A. Check if the distance to the origin is
          // less than the current value of min_centerline_radius_A:
          if (distance < min_centerline_radius_A)  {
            min_centerline_radius_A = distance;
            X_CL_A = x;
            Y_CL_A = y;
          }
        }  // End of if-block for (x,y) cell containing a path-centerline of shape A

        // Check if (x,y) cell is flagged as a path-center for shape B:
        if (cells[x][y] & 2)  {
          // Cell at (x,y) is a path-center for shape B. Check if the distance to the origin is
          // less than the current value of min_centerline_radius_B:
          if (distance < min_centerline_radius_B)  {
            min_centerline_radius_B = distance;
            X_CL_B = x;
            Y_CL_B = y;
          }
        }  // End of if-block for (x,y) cell containing a path-centerline of shape B

        // Check if (x,y) cell is flagged as part of the diff-pair shape A:
        if (cells[x][y] & 4)  {
          // Cell at (x,y) part of the diff-pair shape A. Check if the distance to the
          // origin is less than the current value of min_shape_radius_A:
          if (distance < min_shape_radius_A)  {
            min_shape_radius_A = distance;
            X_shape_A = x;
            Y_shape_A = y;
          }

          // Also check if the distance to the origin is greater than the
          // current value of max_shape_radius_A:
          if (distance > max_shape_radius_A)  {
            max_shape_radius_A = distance;
          }
        }  // End of if-block for (x,y) cell being part of diff-pair shape A

        // Check if (x,y) cell is flagged as part of the diff-pair shape B:
        if (cells[x][y] & 8)  {
          // Cell at (x,y) part of the diff-pair shape B. Check if the distance to the
          // origin is less than the current value of min_shape_radius_B:
          if (distance < min_shape_radius_B)  {
            min_shape_radius_B = distance;
            X_shape_B = x;
            Y_shape_B = y;
          }

          // Also check if the distance to the origin is greater than the
          // current value of max_shape_radius_B:
          if (distance > max_shape_radius_B)  {
            max_shape_radius_B = distance;
          }
        }  // End of if-block for (x,y) cell being part of diff-pair shape B

      }  // End of for-loop for index 'y'
    }  // End of for-loop for index 'x'

    //
    // Calculate the minimum distances between the centerline of shape-A and the nearest portion
    // of shape-B. Do the same for the distance between the centerline of shape-B and the nearest
    // distance of shape-A. Because of the 8-fold symmetry of the X/Y coordinate system, we check
    // all permutations to find the minimum distance between:
    //
    //  Centerline of shape-A to nearest point in shape-B:
    //    1)  (X_CL_A, Y_CL_A) and (-X_shape_B, -Y_shape_B), which is sqrt[ (X_CL_A + X_shape_B)^2 + (Y_CL_A + Y_shape_B)^2 ]
    //    2)  (X_CL_A, Y_CL_A) and (-Y_shape_B, -X_shape_B), which is sqrt[ (X_CL_A + Y_shape_B)^2 + (Y_CL_A + X_shape_B)^2 ]
    //    3)  (Y_CL_A, X_CL_A) and (-X_shape_B, -Y_shape_B), which is sqrt[ (Y_CL_A + X_shape_B)^2 + (X_CL_A + Y_shape_B)^2 ] <<== duplicate of (2)
    //    4)  (Y_CL_A, X_CL_A) and (-Y_shape_B, -X_shape_B), which is sqrt[ (Y_CL_A + Y_shape_B)^2 + (X_CL_A + X_shape_B)^2 ] <<== duplicate of (1)
    //
    //  Centerline of shape-B to nearest point in shape-A:
    //    5)  (X_CL_B, Y_CL_B) and (-X_shape_A, -Y_shape_A), which is sqrt[ (X_CL_B + X_shape_A)^2 + (Y_CL_B + Y_shape_A)^2 ]
    //    6)  (X_CL_B, Y_CL_B) and (-Y_shape_A, -X_shape_A), which is sqrt[ (X_CL_B + Y_shape_A)^2 + (Y_CL_B + X_shape_A)^2 ]
    //    7)  (Y_CL_B, X_CL_B) and (-X_shape_A, -Y_shape_A), which is sqrt[ (Y_CL_B + X_shape_A)^2 + (X_CL_B + Y_shape_A)^2 ] <<== duplicate of (6)
    //    8)  (Y_CL_B, X_CL_B) and (-Y_shape_A, -X_shape_A), which is sqrt[ (Y_CL_B + Y_shape_A)^2 + (X_CL_B + X_shape_A)^2 ] <<== duplicate of (5)
    //
    float min_distance_centerlineA_to_shapeB
	         = min( sqrt((X_CL_A + X_shape_B)*(X_CL_A + X_shape_B)  +  (Y_CL_A + Y_shape_B)*(Y_CL_A + Y_shape_B)),   // Item (1) in above list
                    sqrt((X_CL_A + Y_shape_B)*(X_CL_A + Y_shape_B)  +  (Y_CL_A + X_shape_B)*(Y_CL_A + X_shape_B)));  // Item (2) in above list

    float min_distance_centerlineB_to_shapeA
             = min( sqrt((X_CL_B + X_shape_A)*(X_CL_B + X_shape_A)  +  (Y_CL_B + Y_shape_A)*(Y_CL_B + Y_shape_A)),   // Item (5) in above list
                    sqrt((X_CL_B + Y_shape_A)*(X_CL_B + Y_shape_A)  +  (Y_CL_B + X_shape_A)*(Y_CL_B + X_shape_A)));  // Item (6) in above list

    #ifdef DEBUG_calc_diffPair_design_rules
    printf("\nDEBUG: With 'adder' value of %.2f, the following values were calculated:\n", adder);
    printf("DEBUG:              min_centerline_radius_A = %.3f cells at (%d,%d)\n", min_centerline_radius_A, X_CL_A, Y_CL_A);
    printf("DEBUG:              min_centerline_radius_B = %.3f cells at (%d,%d)\n", min_centerline_radius_B, X_CL_B, Y_CL_B);
    printf("DEBUG:                   min_shape_radius_A = %.3f cells at (%d,%d)\n", min_shape_radius_A, X_shape_A, Y_shape_A);
    printf("DEBUG:                   min_shape_radius_B = %.3f cells at (%d,%d)\n", min_shape_radius_B, X_shape_B, Y_shape_B);
    printf("DEBUG:                   max_shape_radius_A = %.3f cells\n", max_shape_radius_A);
    printf("DEBUG:                   max_shape_radius_B = %.3f cells\n", max_shape_radius_B);
    printf("DEBUG:   min_distance_centerlineA_to_shapeB = %.3f cells\n", min_distance_centerlineA_to_shapeB);
    printf("DEBUG:   min_distance_centerlineB_to_shapeA = %.3f cells\n\n", min_distance_centerlineB_to_shapeA);

    if (min_centerline_radius_B + min_shape_radius_A < DRC_radius_AB)  {
      printf("DEBUG: Failure: The minimum centerline radius for shape B (%.3f) plus the minimum shape-radius for shape A (%.3f)\n",
             min_centerline_radius_B, min_shape_radius_A);
      printf("DEBUG:          sum to %.3f cells, which is less than the corresponding DRC_radius_AB (%.3f cells).\n\n",
             min_centerline_radius_B + min_shape_radius_A, DRC_radius_AB);
    }
    else  {
        printf("DEBUG: Success: The minimum centerline radius for shape B (%.3f) plus the minimum shape-radius for shape A (%.3f)\n",
               min_centerline_radius_B, min_shape_radius_A);
        printf("DEBUG:          sum to %.3f cells, which is greater than or equal to the corresponding DRC_radius_AB (%.3f cells).\n\n",
               min_centerline_radius_B + min_shape_radius_A, DRC_radius_AB);
    }
    if ((min_centerline_radius_A + min_shape_radius_B < DRC_radius_BA))  {
      printf("DEBUG: Failure: The minimum centerline radius for shape A (%.3f) plus the minimum shape-radius for shape B (%.3f)\n",
             min_centerline_radius_A, min_shape_radius_B);
      printf("DEBUG:          sum to %.3f cells, which is less than the corresponding DRC_radius_BA (%.3f cells).\n\n",
             min_centerline_radius_A + min_shape_radius_B, DRC_radius_BA);
    }
    else  {
      printf("DEBUG: Success: The minimum centerline radius for shape A (%.3f) plus the minimum shape-radius for shape B (%.3f)\n",
             min_centerline_radius_A, min_shape_radius_B);
      printf("DEBUG:          sum to %.3f cells, which is greater than or equal to the corresponding DRC_radius_BA (%.3f cells).\n\n",
             min_centerline_radius_A + min_shape_radius_B, DRC_radius_BA);
    }
    if (min_distance_centerlineB_to_shapeA < DRC_radius_AB)  {
      printf("DEBUG: Failure: The minimum distance from shape B's centerline to a shape-A region (%.3f) is less than the corresponding DRC_radius_AB (%.3f).\n\n",
             min_distance_centerlineB_to_shapeA, DRC_radius_AB);
    }
    else  {
      printf("DEBUG: Success: The minimum distance from shape B's centerline to a shape-A region (%.3f) is greater than or equal to the corresponding DRC_radius_AB (%.3f).\n\n",
             min_distance_centerlineB_to_shapeA, DRC_radius_AB);
    }
    if (min_distance_centerlineA_to_shapeB < DRC_radius_BA)  {
      printf("DEBUG: Failure: The minimum distance from shape A's centerline to a shape-B region (%.3f) is less than the corresponding DRC_radius_BA (%.3f).\n\n",
             min_distance_centerlineA_to_shapeB, DRC_radius_BA);
    }
    else  {
      printf("DEBUG: Success: The minimum distance from shape A's centerline to a shape-B region (%.3f) is greater than or equal to the corresponding DRC_radius_BA (%.3f).\n\n",
             min_distance_centerlineA_to_shapeB, DRC_radius_BA);
    }
    #endif

    // Free the memory allocated in this while-loop:
    for (int i = 0; i < max_size; i++)  {
      free(cells[i]);
      cells[i] = NULL;
    }  // End of for-loop for index 'i'
    free(cells);
    cells = NULL;

    //
    // If any of the following is true, then it means that the 'adder' value was not
    // large enough to prevent design-rule violations:
    //
    //   (1)  min_centerline_radius_B + min_shape_radius_A < DRC_radius_AB, or
    //   (2)  min_centerline_radius_A + min_shape_radius_B < DRC_radius_BA, or
    //   (3)  distance from (X_CL_B, Y_CL_B) to (-X_shape_A, -Y_shape_A) is less than DRC_radius_AB, or
    //   (4)  distance from (X_CL_A, Y_CL_A) to (-X_shape_B, -Y_shape_B) is less than DRC_radius_BA, or
    //
    // If any of the above is true, then increment the 'adder' value by 0.1 cells,
    // which will increase both current_halfPitch_A and current_halfPitch_B by the
    // same amount. Also set the Boolean flag to FALSE, thereby forcing the enclosing
    // while-loop to iterate again:
    if (   (min_centerline_radius_B + min_shape_radius_A < DRC_radius_AB)
        || (min_centerline_radius_A + min_shape_radius_B < DRC_radius_BA)
        ||           (min_distance_centerlineB_to_shapeA < DRC_radius_AB)
        ||           (min_distance_centerlineA_to_shapeB < DRC_radius_BA))  {
      adder += 0.1;

      // Clear the Boolean flag so that we repeat the while-loop with the new, larger
      // value of the half-pitch:
      correct_halfPitches_found = FALSE;

      #ifdef DEBUG_calc_diffPair_design_rules
      printf("\nDEBUG: In function calc_diffPair_design_rules, the 'adder' value will be increased from %.3f to %.3f cells because:\n", adder - 0.1, adder);
      printf("DEBUG:   The minimum centerline radius for shape B (%.3f) plus the minimum shape-radius for shape A (%.3f)\n",
             min_centerline_radius_B, min_shape_radius_A);
      printf("DEBUG:     sum to %.3f cells, which is less than the corresponding DRC_radius_AB (%.3f cells), or\n", min_centerline_radius_B + min_shape_radius_A, DRC_radius_AB);
      printf("DEBUG:   The minimum centerline radius for shape A (%.3f) plus the minimum shape-radius for shape B (%.3f)\n",
             min_centerline_radius_A, min_shape_radius_B);
      printf("DEBUG:     sum to %.3f cells, which is less than the corresponding DRC_radius_BA (%.3f cells), or\n", min_centerline_radius_A + min_shape_radius_B, DRC_radius_BA);
      printf("DEBUG:   The minimum distance from shape B's centerline to a shape-A region (%.3f) is less than the corresponding DRC_radius_AB (%.3f), or\n",
             min_distance_centerlineB_to_shapeA, DRC_radius_AB);
      printf("DEBUG:   The minimum distance from shape A's centerline to a shape-B region (%.3f) is less than the corresponding DRC_radius_BA (%.3f).\n\n",
             min_distance_centerlineA_to_shapeB, DRC_radius_BA);
      #endif

    }  // End of if-block for sum < DRC_radius

    // Check if the 'adder' variable has become unrealistically large due to an error in the
    // input variables to this function. Issue a fatal error message if the adder exceeds
    // either of the DRC_radius_* value (assuming these are non-zero), or the adder
    // exceeds the value of 'max_adder_value_cells'. This prevents the enclosing while-loop
    // from going on forever.
    if (   ((DRC_radius_AB > 0.1) && (adder > DRC_radius_AB))
        || ((DRC_radius_BA > 0.1) && (adder > DRC_radius_BA))
        || (adder > max_adder_value_cells))  {
      printf("\nERROR: An unexpected problem was detected in function calc_diffPair_design_rules, in which the 'adder' value (%.3f cells) exceeded at\n", adder);
      printf(  "       least one of the DRC_radius values (%.3f and %.3f cells), or exceeded a value of %.2f cells. Please notify the software developer\n",
             DRC_radius_AB, DRC_radius_BA, max_adder_value_cells);
      printf(  "       of this fatal error message.\n\n");
      exit(1);
    }

  }  // End of while-loop


  // We exited the while-loop, so we've successfully calculated allowable values for the
  // following seven variables:
  //   (a) min_centerline_radius_A: Minimum distance from pseudo-path's center-line to
  //       diff-pair's centerline for shape A
  //   (b) min_centerline_radius_B: Minimum distance from pseudo-path's center-line to
  //       diff-pair's centerline for shape B
  //   (c) min_shape_radius_A: Minimum distance from pseudo-path's center-line to diff-pair
  //       for shape A
  //   (d) min_shape_radius_B: Minimum distance from pseudo-path's center-line to diff-pair
  //       for shape B
  //   (e) max_shape_radius_A: Maximum distance from pseudo-path's center-line to diff-pair
  //       for shape A
  //   (f) max_shape_radius_B: Maximum distance from pseudo-path's center-line to diff-pair
  //       for shape B
  //   (g) adder: additional distance in cell-units that had to be added to the centerline
  //       radius values to avoid design-rule violations between diff-pair partner-nets
  //       of shapes A and B

  // Finally, we update the four values that were passed by reference into this function:
  *diffPair_halfPitch_A = nominal_halfPitch_A + adder;
  *diffPair_halfPitch_B = nominal_halfPitch_B + adder;
  *pseudo_halfWidth_A = max_shape_radius_A;
  *pseudo_halfWidth_B = max_shape_radius_B;

  #ifdef DEBUG_calc_diffPair_design_rules
  printf("\nDEBUG: At end of function calc_diffPair_design_rules:\n");
  printf("DEBUG:      diffPair_halfPitch_A updated from %.3f to %.3f cells (increase of %.3f).\n",
         nominal_halfPitch_A, *diffPair_halfPitch_A, adder);
  printf("DEBUG:      diffPair_halfPitch_B updated from %.3f to %.3f cells (increase of %.3f).\n",
         nominal_halfPitch_B, *diffPair_halfPitch_B, adder);
  printf("DEBUG:      pseudo_halfWidth_A updated from %.3f to %.3f cells.\n", nominal_pseudo_halfWidth_A, *pseudo_halfWidth_A);
  printf("DEBUG:      pseudo_halfWidth_B updated from %.3f to %.3f cells.\n\n", nominal_pseudo_halfWidth_B, *pseudo_halfWidth_B);
  #endif

}  // End of function 'calc_diffPair_design_rules'


//-----------------------------------------------------------------------------
// Name: createUsefulDesignRuleInfo
// Desc: For each design-rule subset, convert the design-rule parameters
//       to 'cell' dimensions from microns. Also, compute useful parameters
//       for each design-rule set and subset that are derived from
//       user-supplied values.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_createUsefulDesignRuleInfo' and re-compile if you want
// verbose debugging print-statements enabled:
//
// #define DEBUG_createUsefulDesignRuleInfo 1
#undef DEBUG_createUsefulDesignRuleInfo

void createUsefulDesignRuleInfo(const MapInfo_t *mapInfo, InputValues_t *user_inputs)  {

  //
  // Iterate through each design-rule set and subset to calculate parameters that depend
  // only on each design-rule subset's inputs from the user, and not on the *interaction*
  // of different design-rule sets/subsets:
  //
  for (int i = 0; i < user_inputs->numDesignRuleSets; i++)  {

    for (int j = 0; j < user_inputs->numDesignRuleSubsets[i]; j++)  {

      #ifdef DEBUG_createUsefulDesignRuleInfo
      printf("\nDEBUG: In function 'createUsefulDesignRuleInfo', analyzing design-rule set %d and subset %d.\n", i, j);
      printf("DEBUG:     isDiffPairSubset = %d           isPseudoNetSubset = %d\n",
              user_inputs->designRules[i][j].isDiffPairSubset, user_inputs->designRules[i][j].isPseudoNetSubset);
      #endif

      //
      // Check if this design-rule subset is dedicated to diff-pair pseudo-net. If not, 
      // then treat the design rules as one would expect:
      //
      if (! user_inputs->designRules[i][j].isPseudoNetSubset)  {

        #ifdef DEBUG_createUsefulDesignRuleInfo
        printf("DEBUG:   Design-rule set %d, subset %d, is NOT a pseudo-net subset.\n", i, j);
        #endif

        //
        // Half-width (radius) of trace (in cell units) = radius[0]. Square of this radius
        // is radius_squared[0].
        //
        // Note that we do *NOT* use the 'round' function to round the floating point calculation to the nearest
        // integer value. Instead, we want the shape's radius value to be truncated (rounded downward) to avoid
        // design-rule violations if the shape's radius were rounded upwards.
        user_inputs->designRules[i][j].radius[TRACE]
           = 0.5 * user_inputs->designRules[i][j].lineWidthMicrons
                     / user_inputs->cell_size_um;

        user_inputs->designRules[i][j].radius_squared[TRACE]
           = 0.25 * user_inputs->designRules[i][j].lineWidthMicrons
                  * user_inputs->designRules[i][j].lineWidthMicrons
                        / user_inputs->cell_size_um / user_inputs->cell_size_um ;

        #ifdef DEBUG_createUsefulDesignRuleInfo
        printf("DEBUG:           radius[TRACE] = %.2f\n", user_inputs->designRules[i][j].radius[TRACE]);
        printf("DEBUG:   radius_squared[TRACE] = %.2f\n", user_inputs->designRules[i][j].radius_squared[TRACE]);
        #endif

        //
        // Radius of upward-going via (in cell units) = radius[1]. Square of this radius
        // is radius_squared[1].
        //
        user_inputs->designRules[i][j].radius[VIA_UP]
           = 0.5 * user_inputs->designRules[i][j].viaUpDiameterMicrons
                         / user_inputs->cell_size_um;

        user_inputs->designRules[i][j].radius_squared[VIA_UP]
           = 0.25 * user_inputs->designRules[i][j].viaUpDiameterMicrons
                  * user_inputs->designRules[i][j].viaUpDiameterMicrons
                        / user_inputs->cell_size_um / user_inputs->cell_size_um;
        #ifdef DEBUG_createUsefulDesignRuleInfo
        printf("DEBUG:           radius[VIA_UP] = %.2f\n", user_inputs->designRules[i][j].radius[VIA_UP]);
        printf("DEBUG:   radius_squared[VIA_UP] = %.2f\n", user_inputs->designRules[i][j].radius_squared[VIA_UP]);
        #endif


        //
        // Radius of downward-going via (in cell units) = radius[2]. Square of this radius
        // is radius_squared[2].
        //
        user_inputs->designRules[i][j].radius[VIA_DOWN]
           = 0.5 * user_inputs->designRules[i][j].viaDownDiameterMicrons
                         / user_inputs->cell_size_um;
        user_inputs->designRules[i][j].radius_squared[VIA_DOWN]
           = 0.25 * user_inputs->designRules[i][j].viaDownDiameterMicrons
                  * user_inputs->designRules[i][j].viaDownDiameterMicrons
                          / user_inputs->cell_size_um / user_inputs->cell_size_um;
        #ifdef DEBUG_createUsefulDesignRuleInfo
        printf("DEBUG:           radius[VIA_DOWN] = %.2f\n", user_inputs->designRules[i][j].radius[VIA_DOWN]);
        printf("DEBUG:   radius_squared[VIA_DOWN] = %.2f\n", user_inputs->designRules[i][j].radius_squared[VIA_DOWN]);
        #endif

        //
        // If this design-rule subset is also a diff-pair subset (but NOT a pseudo-net
        // subset), then calculate the diff-pair pitch for each shape-type (TRACE, VIA_UP,
        // and VIA_DOWN). We start with user-supplied values for (trace) diff-pair pitch,
        // via-up diameters/spacings, and via-down diameters/spacings. Due to rounding,
        // however, these values could result in intra-diff-pair design-rule violations.
        // The diff-pair pitch is therefore enlarged to avoid such intra-diff-pair spacing
        // violations -- usually by a fraction of a cell.
        //
        if (user_inputs->designRules[i][j].isDiffPairSubset)  {
          //
          // Calculate the diff-pair pitch (in cell-units) between two TRACE shapes:
          //
          float diffPair_halfWidth_TRACE = 0.5 * user_inputs->designRules[i][j].lineWidthMicrons / user_inputs->cell_size_um;
          float DRC_radius_TRACEtoTRACE  = (user_inputs->designRules[i][j].lineSpacingMicrons
                                             + user_inputs->designRules[i][j].lineWidthMicrons / 2.0)
                                                / user_inputs->cell_size_um;

          float diffPair_halfPitch_TRACE_A = 0.5 * user_inputs->designRules[i][j].traceDiffPairPitchMicrons / user_inputs->cell_size_um;
          float diffPair_halfPitch_TRACE_B = diffPair_halfPitch_TRACE_A;
          float pseudo_halfWidth_TRACE_A   = 0.0;
          float pseudo_halfWidth_TRACE_B   = 0.0;

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: About to call calc_diffPair_design_rules to calculate TRACE-to-TRACE interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:         diffPair_halfWidth_TRACE = %.3f cells\n", diffPair_halfWidth_TRACE);
          printf("DEBUG:          DRC_radius_TRACEtoTRACE = %.3f cells\n", DRC_radius_TRACEtoTRACE);
          printf("DEBUG:     diffPair_halfPitch_TRACE_A/B = %.3f cells\n", diffPair_halfPitch_TRACE_A);
          printf("DEBUG:       pseudo_halfWidth_TRACE_A/B = %.3f cells\n", pseudo_halfWidth_TRACE_A);
          #endif

          calc_diffPair_design_rules(diffPair_halfWidth_TRACE,    diffPair_halfWidth_TRACE,
									 DRC_radius_TRACEtoTRACE,     DRC_radius_TRACEtoTRACE,
                                     &diffPair_halfPitch_TRACE_A, &diffPair_halfPitch_TRACE_B,
                                     &pseudo_halfWidth_TRACE_A,   &pseudo_halfWidth_TRACE_B,
                                     mapInfo->mapDiagonal);

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: Returned from calc_diffPair_design_rules after calculating TRACE-to-TRACE interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:     UPDATED diffPair_halfPitch_TRACE_A = %.3f cells\n", diffPair_halfPitch_TRACE_A);
          printf("DEBUG:     UPDATED diffPair_halfPitch_TRACE_B = %.3f cells\n", diffPair_halfPitch_TRACE_B);
          printf("DEBUG:       UPDATED pseudo_halfWidth_TRACE_A = %.3f cells\n", pseudo_halfWidth_TRACE_A);
          printf("DEBUG:       UPDATED pseudo_halfWidth_TRACE_B = %.3f cells\n", pseudo_halfWidth_TRACE_B);
          #endif

          // Calculate the diff-pair pitch between TRACE shapes as twice the half-pitch calculated
          // from function calc_diffPair_design_rules():
          float diffPairPitchTraceTrace = 2.0 * max(diffPair_halfPitch_TRACE_A, diffPair_halfPitch_TRACE_B);

          // Save the TRACE diff-pair pitch in the 'designRules' structure.
          user_inputs->designRules[i][j].diffPairPitchCells[TRACE] = diffPairPitchTraceTrace;

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("DEBUG:   Calculated user_inputs->designRules[%d][%d].diffPairPitchCells[TRACE] = %.2f (simulated to avoid DRCs)\n",
                 i, j, user_inputs->designRules[i][j].diffPairPitchCells[TRACE]);
          #endif


          //
          // Calculate the diff-pair pitch (in cell-units) between two VIA_UP shapes. This pitch will be the
          // maximum of the following three quantities:
          //   (1) TRACE-to-TRACE pitch (calculated above),
          //   (2) VIA_UP-to-VIA_UP pitch (calculated immediately below), and
          //   (3) VIA_UP-to-TRACE pitch (calculated below)
          //
          // First, calculated the diff-pair pitch based only on adjacent VIA_UP shapes:
          float diffPair_halfWidth_VIA_UP  = 0.5 * user_inputs->designRules[i][j].viaUpDiameterMicrons / user_inputs->cell_size_um;
          float DRC_radius_VIA_UPtoVIA_UP  = (user_inputs->designRules[i][j].viaUpToViaUpSpacingMicrons
                                             + user_inputs->designRules[i][j].viaUpDiameterMicrons / 2.0)
                                                / user_inputs->cell_size_um;

          float diffPair_halfPitch_VIA_UP_A = 0.5 * (user_inputs->designRules[i][j].viaUpToViaUpSpacingMicrons
                                                     + user_inputs->designRules[i][j].viaUpDiameterMicrons)
                                                / user_inputs->cell_size_um;
          float diffPair_halfPitch_VIA_UP_B = diffPair_halfPitch_VIA_UP_A;
          float pseudo_halfWidth_VIA_UP_A   = 0.0;
          float pseudo_halfWidth_VIA_UP_B   = 0.0;

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: About to call calc_diffPair_design_rules to calculate ViaUp-to-ViaUp interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:          diffPair_halfWidth_VIA_UP = %.3f cells\n", diffPair_halfWidth_VIA_UP);
          printf("DEBUG:          DRC_radius_VIA_UPtoVIA_UP = %.3f cells\n", DRC_radius_VIA_UPtoVIA_UP);
          printf("DEBUG:      diffPair_halfPitch_VIA_UP_A/B = %.3f cells\n", diffPair_halfPitch_VIA_UP_A);
          printf("DEBUG:        pseudo_halfWidth_VIA_UP_A/B = %.3f cells\n", pseudo_halfWidth_VIA_UP_A);
          #endif

          calc_diffPair_design_rules(diffPair_halfWidth_VIA_UP,    diffPair_halfWidth_VIA_UP,
                                     DRC_radius_VIA_UPtoVIA_UP,    DRC_radius_VIA_UPtoVIA_UP,
                                     &diffPair_halfPitch_VIA_UP_A, &diffPair_halfPitch_VIA_UP_B,
                                     &pseudo_halfWidth_VIA_UP_A,   &pseudo_halfWidth_VIA_UP_B,
                                     mapInfo->mapDiagonal);

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: Returned from calc_diffPair_design_rules after calculating ViaUp-to-ViaUp interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:     UPDATED diffPair_halfPitch_VIA_UP_A = %.3f cells\n", diffPair_halfPitch_VIA_UP_A);
          printf("DEBUG:     UPDATED diffPair_halfPitch_VIA_UP_B = %.3f cells\n", diffPair_halfPitch_VIA_UP_B);
          printf("DEBUG:       UPDATED pseudo_halfWidth_VIA_UP_A = %.3f cells\n", pseudo_halfWidth_VIA_UP_A);
          printf("DEBUG:       UPDATED pseudo_halfWidth_VIA_UP_B = %.3f cells\n", pseudo_halfWidth_VIA_UP_B);
          #endif

          // Calculate the diff-pair pitch between VIA_UP shapes as twice the half-pitch calculated from
          // function calc_diffPair_design_rules():
          float diffPairViaUpViaUp = 2.0 * max(diffPair_halfPitch_VIA_UP_A, diffPair_halfPitch_VIA_UP_B);

          // Next, calculated the diff-pair pitch based on adjacent VIA_UP and TRACE shapes:
          float DRC_radius_VIA_UPtoTRACE = (user_inputs->designRules[i][j].viaUpToTraceSpacingMicrons
                                            + user_inputs->designRules[i][j].lineWidthMicrons / 2.0)
                                             / user_inputs->cell_size_um; // Includes radius of TRACE

          float DRC_radius_TRACEtoVIA_UP = (user_inputs->designRules[i][j].viaUpToTraceSpacingMicrons
                                            + user_inputs->designRules[i][j].viaUpDiameterMicrons / 2.0)
                                             / user_inputs->cell_size_um; // Includes radius of VIA_UP

          float diffPair_halfPitch_VIA_UP_beside_TRACE = 0.5 * (user_inputs->designRules[i][j].viaUpToViaUpSpacingMicrons
                                                                + user_inputs->designRules[i][j].viaUpDiameterMicrons)
                                                                 / user_inputs->cell_size_um;;
          float diffPair_halfPitch_TRACE_beside_VIA_UP = 0.5 * user_inputs->designRules[i][j].traceDiffPairPitchMicrons / user_inputs->cell_size_um;
          float pseudo_halfWidth_VIA_UP_beside_TRACE = 0.0;
          float pseudo_halfWidth_TRACE_beside_VIA_UP = 0.0;

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: About to call calc_diffPair_design_rules to calculate ViaUp-to-Trace interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:               diffPair_halfWidth_VIA_UP = %.3f cells\n", diffPair_halfWidth_VIA_UP);
          printf("DEBUG:                diffPair_halfWidth_TRACE = %.3f cells\n", diffPair_halfWidth_TRACE);
          printf("DEBUG:                DRC_radius_VIA_UPtoTRACE = %.3f cells\n", DRC_radius_VIA_UPtoTRACE);
          printf("DEBUG:                DRC_radius_TRACEtoVIA_UP = %.3f cells\n", DRC_radius_TRACEtoVIA_UP);
          printf("DEBUG:  diffPair_halfPitch_VIA_UP_beside_TRACE = %.3f cells\n", diffPair_halfPitch_VIA_UP_beside_TRACE);
          printf("DEBUG:  diffPair_halfPitch_TRACE_beside_VIA_UP = %.3f cells\n", diffPair_halfPitch_TRACE_beside_VIA_UP);
          printf("DEBUG:    pseudo_halfWidth_VIA_UP_beside_TRACE = %.3f cells\n", pseudo_halfWidth_VIA_UP_beside_TRACE);
          printf("DEBUG:    pseudo_halfWidth_TRACE_beside_VIA_UP = %.3f cells\n", pseudo_halfWidth_TRACE_beside_VIA_UP);
          #endif

          calc_diffPair_design_rules(diffPair_halfWidth_VIA_UP,               diffPair_halfWidth_TRACE,
                                     DRC_radius_VIA_UPtoTRACE,                DRC_radius_TRACEtoVIA_UP,
                                     &diffPair_halfPitch_VIA_UP_beside_TRACE, &diffPair_halfPitch_TRACE_beside_VIA_UP,
                                     &pseudo_halfWidth_VIA_UP_beside_TRACE,   &pseudo_halfWidth_TRACE_beside_VIA_UP,
                                     mapInfo->mapDiagonal);

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: Returned from calc_diffPair_design_rules after calculating ViaUp-to-Trace interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:     UPDATED diffPair_halfPitch_VIA_UP_beside_TRACE = %.3f cells\n", diffPair_halfPitch_VIA_UP_beside_TRACE);
          printf("DEBUG:      UPDATED diffPair_halfPitch_TRACE_beside_VIA_UP = %.3f cells\n", diffPair_halfPitch_TRACE_beside_VIA_UP);
          printf("DEBUG:       UPDATED pseudo_halfWidth_VIA_UP_beside_TRACE = %.3f cells\n", pseudo_halfWidth_VIA_UP_beside_TRACE);
          printf("DEBUG:        UPDATED pseudo_halfWidth_TRACE_beside_VIA_UP = %.3f cells\n", pseudo_halfWidth_TRACE_beside_VIA_UP);
          #endif

          // Calculate the diff-pair pitch between a VIA_UP shape and TRACE shape as twice the maximum
          // of the half-pitch distances calculated from function calc_diffPair_design_rules():
          float diffPairPitchViaUpTrace = 2.0 * max(diffPair_halfPitch_VIA_UP_beside_TRACE, diffPair_halfPitch_TRACE_beside_VIA_UP);

          //
          // Save the VIA_UP diff-pair pitch in the 'designRules' structure. This value is the maximum of the pitch
          // calculated by simulating the interactions between (a) TRACE-to-TRACE, (b) VIA_UP-to-VIA_UP, and
          // (c) VIA_UP-to-TRACE.
          //
          user_inputs->designRules[i][j].diffPairPitchCells[VIA_UP] = max(diffPairPitchTraceTrace, max(diffPairViaUpViaUp, diffPairPitchViaUpTrace));

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("DEBUG:   Calculated user_inputs->designRules[%d][%d].diffPairPitchCells[VIA_UP] = %.2f (simulated to avoid DRCs)\n",
                 i, j, user_inputs->designRules[i][j].diffPairPitchCells[VIA_UP]);
          #endif


          //
          // Calculate the diff-pair pitch (in cell-units) between two VIA_DOWN shapes. This pitch will
          // be the maximum of the following three quantities:
          //   (1) TRACE-to-TRACE pitch (calculated above),
          //   (2) VIA_DOWN-to-VIA_DOWN pitch (calculated immediately below), and
          //   (3) VIA_DOWN-to-TRACE pitch (calculated below)
          //
          // First, calculated the diff-pair pitch based only on adjacent VIA_DOWN shapes:
          float diffPair_halfWidth_VIA_DOWN    = 0.5 * user_inputs->designRules[i][j].viaDownDiameterMicrons / user_inputs->cell_size_um;

          float DRC_radius_VIA_DOWNtoVIA_DOWN  = (user_inputs->designRules[i][j].viaDownToViaDownSpacingMicrons
                                                  + user_inputs->designRules[i][j].viaDownDiameterMicrons / 2.0)
                                                    / user_inputs->cell_size_um;

          float diffPair_halfPitch_VIA_DOWN_A  = 0.5 * (user_inputs->designRules[i][j].viaDownToViaDownSpacingMicrons
                                                        + user_inputs->designRules[i][j].viaDownDiameterMicrons)
                                                    / user_inputs->cell_size_um;
          float diffPair_halfPitch_VIA_DOWN_B = diffPair_halfPitch_VIA_DOWN_A;
          float pseudo_halfWidth_VIA_DOWN_A   = 0.0;
          float pseudo_halfWidth_VIA_DOWN_B   = 0.0;

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: About to call calc_diffPair_design_rules to calculate ViaDown-to-ViaDown interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:            diffPair_halfWidth_VIA_DOWN = %.3f cells\n", diffPair_halfWidth_VIA_DOWN);
          printf("DEBUG:          DRC_radius_VIA_DOWNtoVIA_DOWN = %.3f cells\n", DRC_radius_VIA_DOWNtoVIA_DOWN);
          printf("DEBUG:        diffPair_halfPitch_VIA_DOWN_A/B = %.3f cells\n", diffPair_halfPitch_VIA_DOWN_A);
          printf("DEBUG:          pseudo_halfWidth_VIA_DOWN_A/B = %.3f cells\n", pseudo_halfWidth_VIA_DOWN_A);
          #endif

          calc_diffPair_design_rules(diffPair_halfWidth_VIA_DOWN,    diffPair_halfWidth_VIA_DOWN,
                                     DRC_radius_VIA_DOWNtoVIA_DOWN,  DRC_radius_VIA_DOWNtoVIA_DOWN,
                                     &diffPair_halfPitch_VIA_DOWN_A, &diffPair_halfPitch_VIA_DOWN_B,
                                     &pseudo_halfWidth_VIA_DOWN_A,   &pseudo_halfWidth_VIA_DOWN_B,
                                     mapInfo->mapDiagonal);

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: Returned from calc_diffPair_design_rules after calculating ViaDown-to-ViaDown interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:     UPDATED diffPair_halfPitch_VIA_DOWN_A = %.3f cells\n", diffPair_halfPitch_VIA_DOWN_A);
          printf("DEBUG:     UPDATED diffPair_halfPitch_VIA_DOWN_B = %.3f cells\n", diffPair_halfPitch_VIA_DOWN_B);
          printf("DEBUG:       UPDATED pseudo_halfWidth_VIA_DOWN_A = %.3f cells\n", pseudo_halfWidth_VIA_DOWN_A);
          printf("DEBUG:       UPDATED pseudo_halfWidth_VIA_DOWN_B = %.3f cells\n", pseudo_halfWidth_VIA_DOWN_B);
          #endif

          // Calculate the diff-pair pitch between VIA_DOWN shapes as twice the half-pitch calculated from
          // function calc_diffPair_design_rules():
          float diffPairViaDownViaDown = 2.0 * max(diffPair_halfPitch_VIA_DOWN_A, diffPair_halfPitch_VIA_DOWN_B);

          // Next, calculated the diff-pair pitch based on adjacent VIA_DOWN and TRACE shapes:
          float DRC_radius_VIA_DOWNtoTRACE = (user_inputs->designRules[i][j].viaDownToTraceSpacingMicrons
                                              + user_inputs->designRules[i][j].lineWidthMicrons / 2.0)
                                               / user_inputs->cell_size_um; // Includes radius of TRACE

          float DRC_radius_TRACEtoVIA_DOWN = (user_inputs->designRules[i][j].viaDownToTraceSpacingMicrons
                                              + user_inputs->designRules[i][j].viaDownDiameterMicrons / 2.0)
                                               / user_inputs->cell_size_um; // Includes radius of VIA_DOWN

          float diffPair_halfPitch_VIA_DOWN_beside_TRACE = 0.5 * (user_inputs->designRules[i][j].viaDownToViaDownSpacingMicrons
                                                     + user_inputs->designRules[i][j].viaDownDiameterMicrons)
                                                      / user_inputs->cell_size_um;
          float diffPair_halfPitch_TRACE_beside_VIA_DOWN  = 0.5 * user_inputs->designRules[i][j].traceDiffPairPitchMicrons / user_inputs->cell_size_um;
          float pseudo_halfWidth_VIA_DOWN_beside_TRACE = 0.0;
          float pseudo_halfWidth_TRACE_beside_VIA_DOWN  = 0.0;

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: About to call calc_diffPair_design_rules to calculate ViaDown-to-Trace interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:               diffPair_halfWidth_VIA_DOWN = %.3f cells\n", diffPair_halfWidth_VIA_DOWN);
          printf("DEBUG:                  diffPair_halfWidth_TRACE = %.3f cells\n", diffPair_halfWidth_TRACE);
          printf("DEBUG:                DRC_radius_VIA_DOWNtoTRACE = %.3f cells\n", DRC_radius_VIA_DOWNtoTRACE);
          printf("DEBUG:                DRC_radius_TRACEtoVIA_DOWN = %.3f cells\n", DRC_radius_TRACEtoVIA_DOWN);
          printf("DEBUG:  diffPair_halfPitch_VIA_DOWN_beside_TRACE = %.3f cells\n", diffPair_halfPitch_VIA_DOWN_beside_TRACE);
          printf("DEBUG:  diffPair_halfPitch_TRACE_beside_VIA_DOWN = %.3f cells\n", diffPair_halfPitch_TRACE_beside_VIA_DOWN);
          printf("DEBUG:    pseudo_halfWidth_VIA_DOWN_beside_TRACE = %.3f cells\n", pseudo_halfWidth_VIA_DOWN_beside_TRACE);
          printf("DEBUG:    pseudo_halfWidth_TRACE_beside_VIA_DOWN = %.3f cells\n", pseudo_halfWidth_TRACE_beside_VIA_DOWN);
          #endif

          calc_diffPair_design_rules(diffPair_halfWidth_VIA_DOWN,               diffPair_halfWidth_TRACE,
                                     DRC_radius_VIA_DOWNtoTRACE,                DRC_radius_TRACEtoVIA_DOWN,
                                     &diffPair_halfPitch_VIA_DOWN_beside_TRACE, &diffPair_halfPitch_TRACE_beside_VIA_DOWN,
                                     &pseudo_halfWidth_VIA_DOWN_beside_TRACE,   &pseudo_halfWidth_TRACE_beside_VIA_DOWN,
                                     mapInfo->mapDiagonal);

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: Returned from calc_diffPair_design_rules after calculating ViaDown-to-Trace interactions for design-rule set #%d, subset #%d, with\n",
                  i, j);
          printf("DEBUG:  UPDATED diffPair_halfPitch_VIA_DOWN_beside_TRACE = %.3f cells\n", diffPair_halfPitch_VIA_DOWN_beside_TRACE);
          printf("DEBUG:  UPDATED diffPair_halfPitch_TRACE_beside_VIA_DOWN = %.3f cells\n", diffPair_halfPitch_TRACE_beside_VIA_DOWN);
          printf("DEBUG:    UPDATED pseudo_halfWidth_VIA_DOWN_beside_TRACE = %.3f cells\n", pseudo_halfWidth_VIA_DOWN_beside_TRACE);
          printf("DEBUG:    UPDATED pseudo_halfWidth_TRACE_beside_VIA_DOWN = %.3f cells\n", pseudo_halfWidth_TRACE_beside_VIA_DOWN);
          #endif

          // Calculate the diff-pair pitch between a VIA_DOWN shape and TRACE shape as twice the maximum
          // of the half-pitch distances calculated from function calc_diffPair_design_rules():
          float diffPairPitchViaDownTrace = 2.0 * max(diffPair_halfPitch_VIA_DOWN_beside_TRACE, diffPair_halfPitch_TRACE_beside_VIA_DOWN);

          //
          // Save the VIA_DOWN diff-pair pitch in the 'designRules' structure. This value is the maximum of the pitch
          // calculated by simulating the interactions between (a) TRACE-to-TRACE, (b) VIA_DOWN-to-VIA_DOWN, and
          // (c) VIA_DOWN-to-TRACE.
          //
          user_inputs->designRules[i][j].diffPairPitchCells[VIA_DOWN] = max(diffPairPitchTraceTrace, max(diffPairViaDownViaDown, diffPairPitchViaDownTrace));

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("DEBUG:   Calculated user_inputs->designRules[%d][%d].diffPairPitchCells[VIA_DOWN] = %.2f (simulated to avoid DRCs)\n",
                 i, j, user_inputs->designRules[i][j].diffPairPitchCells[VIA_DOWN]);
          #endif


          //
          // Now that the design-rules have been calculated/optimized for the diff-pair design-rule
          // subset, we use the results to calculate design-rule values for the associated
          // pseudo-net design-rule subset. The subset number of the latter subset is always one
          // greater than that of the diff-pair subset:
          //
          int j_pseudo_subset = j + 1;

          // Confirm that the design-rule subset associated with 'j_pseudo_subset' is indeed a pseudo-net subset:
          if (! user_inputs->designRules[i][j_pseudo_subset].isPseudoNetSubset)  {
            printf("\nERROR: An unexpected condition was encountered in function createUsefulDesignRuleInfo in which design-rule set #%d,\n", i);
            printf(  "       subset #%d, is *not* flagged as 'isPseudoNetSubset'. This design-rule set/subset should indeed be flagged as\n", j_pseudo_subset);
            printf(  "       a design-rule subset dedicated to pseudo-nets. Please inform the software developer of this fatal error message.\n\n");
            exit(1);
          }

          //
          // The diff-pair pitch values are the same between the diff-pair subset and the corresponding pseudo-net subset:
          //
          user_inputs->designRules[i][j_pseudo_subset].diffPairPitchCells[TRACE]    = user_inputs->designRules[i][j].diffPairPitchCells[TRACE];
          user_inputs->designRules[i][j_pseudo_subset].diffPairPitchCells[VIA_UP]   = user_inputs->designRules[i][j].diffPairPitchCells[VIA_UP];
          user_inputs->designRules[i][j_pseudo_subset].diffPairPitchCells[VIA_DOWN] = user_inputs->designRules[i][j].diffPairPitchCells[VIA_DOWN];


          //
          // The radius (half-width) of the pseudo-path's TRACE is based on the values of pseudo_halfWidth_TRACE_A/B,
          // as simulated above in function calc_diffPair_design_rules():
          //
          user_inputs->designRules[i][j_pseudo_subset].radius[TRACE] = max(pseudo_halfWidth_TRACE_A, pseudo_halfWidth_TRACE_B);

          // Calculate the radius_squared by squaring the radius:
          user_inputs->designRules[i][j_pseudo_subset].radius_squared[TRACE] = user_inputs->designRules[i][j_pseudo_subset].radius[TRACE]
                                                                               * user_inputs->designRules[i][j_pseudo_subset].radius[TRACE];

          // Also use the pseudo-trace's radius to calculate the 'linWidthMicrons' and 'width_um' variables:
          user_inputs->designRules[i][j_pseudo_subset].lineWidthMicrons
            = user_inputs->designRules[i][j_pseudo_subset].width_um[TRACE]
              = 2.0 * user_inputs->designRules[i][j_pseudo_subset].radius[TRACE] * user_inputs->cell_size_um;


          //
          // The radius of the pseudo-path's VIA_UP is the maximum of the following four values, all of
          // where were simulated above in function calc_diffPair_design_rules():
          //   (1) pseudo_halfWidth_VIA_UP_A/B,
          //   (2) pseudo_halfWidth_TRACE_A/B,
          //   (3) pseudo_halfWidth_VIA_UP_beside_TRACE,
          //   (4) pseudo_halfWidth_TRACE_beside_VIA_UP.
          //
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP] = max(pseudo_halfWidth_VIA_UP_A, pseudo_halfWidth_VIA_UP_B);       // Item (1) from above list
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP] = max(user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP], // Item (2) from above list
                                                                            user_inputs->designRules[i][j_pseudo_subset].radius[TRACE]); // Item (2), continued
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP] = max(user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP], // Item (3) from above list
                                                                            pseudo_halfWidth_VIA_UP_beside_TRACE);                       // Item (3), continued
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP] = max(user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP], // Item (4) from above list
                                                                            pseudo_halfWidth_TRACE_beside_VIA_UP);                       // Item (4), continued

          // Calculate the radius_squared by squaring the radius:
          user_inputs->designRules[i][j_pseudo_subset].radius_squared[VIA_UP] = user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP]
                                                                                * user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP];

          // Also use the pseudo-via-up's radius to calculate the 'viaUpDiameterMicrons' and 'width_um' variables:
          user_inputs->designRules[i][j_pseudo_subset].viaUpDiameterMicrons
            = user_inputs->designRules[i][j_pseudo_subset].width_um[VIA_UP]
              = 2.0 * user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP] * user_inputs->cell_size_um;


          //
          // The radius of the pseudo-path's VIA_DOWN is the maximum of the following four values, all of
          // where were simulated above in function calc_diffPair_design_rules():
          //   (1) pseudo_halfWidth_VIA_DOWN_A/B,
          //   (2) pseudo_halfWidth_TRACE_A/B,
          //   (3) pseudo_halfWidth_VIA_DOWN_beside_TRACE,
          //   (4) pseudo_halfWidth_TRACE_beside_VIA_DOWN.
          //
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN] = max(pseudo_halfWidth_VIA_DOWN_A, pseudo_halfWidth_VIA_DOWN_B);     // Item (1) from above list
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN] = max(user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN], // Item (2) from above list
                                                                              user_inputs->designRules[i][j_pseudo_subset].radius[TRACE]);   // Item (2), continued
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN] = max(user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN], // Item (3) from above list
                                                                              pseudo_halfWidth_VIA_DOWN_beside_TRACE);                       // Item (3), continued
          user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN] = max(user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN], // Item (4) from above list
                                                                              pseudo_halfWidth_TRACE_beside_VIA_DOWN);                       // Item (4), continued

          // Calculate the radius_squared by squaring the radius:
          user_inputs->designRules[i][j_pseudo_subset].radius_squared[VIA_DOWN] = user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN]
                                                                                  * user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN];

          // Also use the pseudo-via-down's radius to calculate the 'viaDownDiameterMicrons' and 'width_um' variables:
          user_inputs->designRules[i][j_pseudo_subset].viaDownDiameterMicrons
            = user_inputs->designRules[i][j_pseudo_subset].width_um[VIA_DOWN]
              = 2.0 * user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN] * user_inputs->cell_size_um;


          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("\nDEBUG: For design-rule set %d, subset %d, the corresponding pseudo-net subset is subset #%d, for which the following were calculated:\n", i, j, j_pseudo_subset);
          printf("DEBUG:   user_inputs->designRules[%d][%d].diffPairPitchCells[TRACE] = %.3f cells\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].diffPairPitchCells[TRACE]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].diffPairPitchCells[VIA_UP] = %.3f cells\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].diffPairPitchCells[VIA_UP]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].diffPairPitchCells[VIA_DOWN] = %.3f cells\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].diffPairPitchCells[VIA_DOWN]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].lineWidthMicrons = %.3f microns\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].lineWidthMicrons);
          printf("DEBUG:   user_inputs->designRules[%d][%d].width_um[TRACE] = %.3f microns\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].width_um[TRACE]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].radius[TRACE] = %.3f cells\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].radius[TRACE]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].radius_squared[TRACE] = %.3f cells^2\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].radius_squared[TRACE]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].viaUpDiameterMicrons = %.3f microns\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].viaUpDiameterMicrons);
          printf("DEBUG:   user_inputs->designRules[%d][%d].width_um[VIA_UP] = %.3f microns\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].width_um[VIA_UP]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].radius[VIA_UP] = %.3f cells\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].radius[VIA_UP]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].radius_squared[VIA_UP] = %.3f cells^2\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].radius_squared[VIA_UP]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].viaDownDiameterMicrons = %.3f microns\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].viaDownDiameterMicrons);
          printf("DEBUG:   user_inputs->designRules[%d][%d].width_um[VIA_DOWN] = %.3f microns\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].width_um[VIA_DOWN]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].radius[VIA_DOWN] = %.3f cells\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].radius[VIA_DOWN]);
          printf("DEBUG:   user_inputs->designRules[%d][%d].radius_squared[VIA_DOWN] = %.3f cells^2\n\n", i, j_pseudo_subset,
                 user_inputs->designRules[i][j_pseudo_subset].radius_squared[VIA_DOWN]);
          #endif

        }  // End of if-block for (isDiffPairSubset == TRUE)

      }  // End of if-block for (isPseudoNetSubset == FALSE)
      #ifdef DEBUG_createUsefulDesignRuleInfo
      else  {
        printf("DEBUG:   Design-rule set %d, subset %d, *IS* a pseudo-net subset, and is handled along with its corresponding diff-pair subset.\n", i, j);
      }   // End of else-block for (isPseudoNetSubset == TRUE)
      #endif

    }  // End of for-loop for j = 0 to number of design-rule subsets
  }  // End of for-loop for i = 0 to number of design rules


  //
  // Iterate through each design-rule set and subset to calculate the spacing values
  // in cell-units, regardless of whether the design-rule subset is used for diff-pair
  // pseudo-nets or not:
  //
  for (int i = 0; i < user_inputs->numDesignRuleSets; i++)  {
    for (int j = 0; j < user_inputs->numDesignRuleSubsets[i]; j++)  {

      //
      // Trace-to-trace spacing (in cells) = spacing[0][0]. If this value is zero, then round
      // up to 1 cell.
      //
      user_inputs->designRules[i][j].spacing[TRACE][TRACE]
         = user_inputs->designRules[i][j].lineSpacingMicrons
           / user_inputs->cell_size_um;
      if (user_inputs->designRules[i][j].spacing[TRACE][TRACE] < 1.0)  {
        printf("INFO: Trace-to-trace spacing was rounded up to 1 cell (from zero) for design-rule set %d, subset %d.\n", i, j);
        user_inputs->designRules[i][j].spacing[TRACE][TRACE] = 1.0;
      }
      #ifdef DEBUG_createUsefulDesignRuleInfo
      printf("DEBUG:   lineSpacingCells = spacing[TRACE][TRACE] = %.2f\n", user_inputs->designRules[i][j].spacing[TRACE][TRACE]);
      #endif

      //
      // UpVia-to-UpVia spacing (in cells) = spacing[1][1]. If this value is zero, then
      // round up to 1 cell.
      //
      user_inputs->designRules[i][j].spacing[VIA_UP][VIA_UP]
         = user_inputs->designRules[i][j].viaUpToViaUpSpacingMicrons
           / user_inputs->cell_size_um;
      if (user_inputs->designRules[i][j].spacing[VIA_UP][VIA_UP] < 1.0)  {
        printf("INFO: Spacing between adjacent VIA-UP shapes was rounded up to 1 cell (from zero) for design-rule set %d, subset %d.\n", i, j);
        user_inputs->designRules[i][j].spacing[VIA_UP][VIA_UP] = 1.0;
      }
      #ifdef DEBUG_createUsefulDesignRuleInfo
      printf("DEBUG:   viaUpToViaUpSpacingCells = spacing[VIA_UP][VIA_UP] = %.2f\n",
                   user_inputs->designRules[i][j].spacing[VIA_UP][VIA_UP]);
      #endif

      //
      // DownVia-to-DownVia spacing (in cells) = spacing[2][2]. If this value is zero,
      // then round up to 1 cell.
      //
      user_inputs->designRules[i][j].spacing[VIA_DOWN][VIA_DOWN]
       = user_inputs->designRules[i][j].viaDownToViaDownSpacingMicrons
         / user_inputs->cell_size_um;
      if (user_inputs->designRules[i][j].spacing[VIA_DOWN][VIA_DOWN] < 1.0)  {
        printf("INFO: Spacing between adjacent VIA-DOWN shapes was rounded up to 1 cell (from zero) for design-rule set %d, subset %d.\n", i, j);
        user_inputs->designRules[i][j].spacing[VIA_DOWN][VIA_DOWN] = 1.0;
      }
      #ifdef DEBUG_createUsefulDesignRuleInfo
      printf("DEBUG:   viaDownToViaDownSpacingCells = spacing[VIA_DOWN][VIA_DOWN] = %.2f\n",
                   user_inputs->designRules[i][j].spacing[VIA_DOWN][VIA_DOWN]);
      #endif

      //
      // Trace-to-UpVia spacing (in cells) = spacing[0][1] = spacing[1][0]. If this value
      // is zero, then round up to 1 cell.
      //
      user_inputs->designRules[i][j].spacing[TRACE][VIA_UP]
         = user_inputs->designRules[i][j].spacing[VIA_UP][TRACE]
           = user_inputs->designRules[i][j].viaUpToTraceSpacingMicrons
              / user_inputs->cell_size_um;
      if (user_inputs->designRules[i][j].spacing[TRACE][VIA_UP] < 1.0)  {
        printf("INFO: Spacing between TRACE and VIA-UP shapes was rounded up to 1 cell (from zero) for design-rule set %d, subset %d.\n", i, j);
        user_inputs->designRules[i][j].spacing[TRACE][VIA_UP]
           = user_inputs->designRules[i][j].spacing[VIA_UP][TRACE]
             = 1.0;
      }
      #ifdef DEBUG_createUsefulDesignRuleInfo
      printf("DEBUG:   viaUpToTraceSpacingCells = spacing[TRACE][VIA_UP] = spacing[VIA_UP][TRACE] = %.2f = %.2f\n",
              user_inputs->designRules[i][j].spacing[TRACE][VIA_UP], user_inputs->designRules[i][j].spacing[VIA_UP][TRACE]);
      #endif

      //
      // Trace-to-DownVia spacing (in cells) = spacing[0][2] = spacing[2][0]. If this value
      // is zero, then round up to 1 cell.
      //
      user_inputs->designRules[i][j].spacing[TRACE][VIA_DOWN]
         = user_inputs->designRules[i][j].spacing[VIA_DOWN][TRACE]
           = user_inputs->designRules[i][j].viaDownToTraceSpacingMicrons
              / user_inputs->cell_size_um;
      if (user_inputs->designRules[i][j].spacing[TRACE][VIA_DOWN] < 1.0)  {
        printf("INFO: Spacing between TRACE and VIA-DOWN shapes was rounded up to 1 cell (from zero) for design-rule set %d, subset %d.\n", i, j);
        user_inputs->designRules[i][j].spacing[TRACE][VIA_DOWN]
           = user_inputs->designRules[i][j].spacing[VIA_DOWN][TRACE]
             = 1.0;
      }
      #ifdef DEBUG_createUsefulDesignRuleInfo
      printf("DEBUG:   viaDownToTraceSpacingCells = spacing[TRACE][VIA_DOWN] = spacing[VIA_DOWN][TRACE] = %.2f = %.2f\n",
              user_inputs->designRules[i][j].spacing[TRACE][VIA_DOWN], user_inputs->designRules[i][j].spacing[VIA_DOWN][TRACE]);
      #endif

      //
      // UpVia-to-DownVia spacing (in cells) = spacing[1][2] = spacing[2][1]. If this value
      // is zero, then round up to 1 cell.
      //
      user_inputs->designRules[i][j].spacing[VIA_UP][VIA_DOWN]
         = user_inputs->designRules[i][j].spacing[VIA_DOWN][VIA_UP]
           = user_inputs->designRules[i][j].viaUpToViaDownSpacingMicrons
              / user_inputs->cell_size_um;
      if (user_inputs->designRules[i][j].spacing[VIA_UP][VIA_DOWN] < 1.0)  {
        printf("INFO: Spacing between VIA-UP and VIA-DOWN shapes was rounded up to 1 cell (from zero) for design-rule set %d, subset %d.\n", i, j);
        user_inputs->designRules[i][j].spacing[VIA_UP][VIA_DOWN]
           = user_inputs->designRules[i][j].spacing[VIA_DOWN][VIA_UP]
             = 1.0;
      }
      #ifdef DEBUG_createUsefulDesignRuleInfo
      printf("DEBUG:   viaUpToViaDownSpacingCells = spacing[VIA_UP][VIA_DOWN] = spacing[VIA_DOWN][VIA_UP] = %.2f = %.2f\n",
              user_inputs->designRules[i][j].spacing[VIA_UP][VIA_DOWN], user_inputs->designRules[i][j].spacing[VIA_DOWN][VIA_UP]);
      #endif

    }  // End of for-loop for j = 0 to number of design-rule subsets
  }  // End of for-loop for i = 0 to number of design rules

  //
  // For each design-rule set, calculate the maximum radius of interaction, i.e., the maximum
  // linewidth/diameter of traces and vias, added to the maximum shape-to-shape spacing.
  //   Rmax = 2*max(Ra,Rb,Rc) + max(Sab,Sac,Sbc,Saa,Sbb,Scc)
  //
  for (int i = 0; i < user_inputs->numDesignRuleSets; i++)  {

    // Initialize the 'maxInteractionRadiusCellsInDR' and 'maxInteractionRadiusSquaredInDR'
    // variables to zero for design-rule set #i:
    user_inputs->maxInteractionRadiusCellsInDR[i]   = 0;
    user_inputs->maxInteractionRadiusSquaredInDR[i] = 0;

    float max_width = 0.0;   // = maximum width of any trace or via in design-rule set #i
    float max_spacing = 0.0; // = maximum spacing between any trace or via in design-rule set #i

    for (int j = 0; j < user_inputs->numDesignRuleSubsets[i]; j++)  {
      for (int m = 0; m < NUM_SHAPE_TYPES; m++)  {
        #ifdef DEBUG_createUsefulDesignRuleInfo
        printf("DEBUG: width_um[%d] = %.3f um for design-rule set #%d, subset %d.\n", m,
                user_inputs->designRules[i][j].width_um[m], i, j);
        #endif
        if (user_inputs->designRules[i][j].width_um[m] > max_width)
          max_width = user_inputs->designRules[i][j].width_um[m];

        for (int n = m; n < NUM_SHAPE_TYPES; n++) {

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("DEBUG: spacing[%d][%d] = %.3f um for design-rule set #%d, subset %d.\n",
                  m, n, user_inputs->designRules[i][j].space_um[m][n], i, j);
          #endif

          if (user_inputs->designRules[i][j].space_um[m][n] > max_spacing)
            max_spacing = user_inputs->designRules[i][j].space_um[m][n];
        }  // End of for-loop for index 'n'
      }  // End of for-loop for index 'm'
    }  // End of for-loop for j = 0 to number of design-rule subsets

    //
    // Now that we know the maximum width and spacing for design-rule set #i, calculate
    // the maximum interaction radius for this design-rule set:
    //
    user_inputs->maxInteractionRadiusCellsInDR[i] = 0.5 + (max_width + max_spacing) / user_inputs->cell_size_um;

    // Also calculate the square of the 'maxInteractionRadiusCellsInDR', since this value
    // will be used frequently:
    user_inputs->maxInteractionRadiusSquaredInDR[i] = user_inputs->maxInteractionRadiusCellsInDR[i] * user_inputs->maxInteractionRadiusCellsInDR[i];

    #ifdef DEBUG_createUsefulDesignRuleInfo
    printf("DEBUG: maxInteractionRadiusCellsInDR = %.2f for design rule set #%d.\n",
            user_inputs->maxInteractionRadiusCellsInDR[i], i);
    printf("DEBUG: maxInteractionRadiusSquaredInDR = %.2f for design rule set #%d.\n",
            user_inputs->maxInteractionRadiusSquaredInDR[i], i);
    #endif
  }  // End of for-loop for index 'i' (0 to numDesignRuleSets)


  //
  // For each combination of design-rule sets given by indices i and j, calculate the
  // values in the following matrices with matrices m and n. For each matrix, 'm' and 'n'
  // range from 0 to num_subset_shapeType_indices, where
  // num_subset_shapeType_indices = NUM_SHAPE_TYPES * user_inputs->numDesignRuleSubsets[i]
  //  (1) 'DRC_radius' matrix. Each element, DRC_radius[i][m][j][n], of this
  //      matrix represents the radius[n] + spacing[m][n], in units of cells.
  //  (2) 'DRC_radius_squared[m][n]' matrix, whose elements are the squares
  //      of the 'DRC_radius' elements.
  //  (3) 'detour_distance' matrix. Each element, detour_distance[m][n], of this
  //      matrix contains the detour distance for calculating the amount of
  //      congestion for routing a net of subset/shapeType 'm' in the presence
  //      of congestion from subset/shapeType 'n'
  //
  // Iterate over all design-rule sets:
  for (int i = 0; i < user_inputs->numDesignRuleSets; i++)  {

    // Iterate over all design-rule subsets of design-rule set 'i':
    for (int m_DR_subset = 0; m_DR_subset < user_inputs->numDesignRuleSubsets[i]; m_DR_subset++)  {

      // Iterate over all shape-types:
      for (int m_shapeType = 0; m_shapeType < NUM_SHAPE_TYPES; m_shapeType++)  {
        // Calculate the first index ('m') used for the matrices. The index is based on the values
        // m_DR_subset and m_shape_type, and is simply 3 * m_DR_subset  +  m_shape_type:
        short m_subset_shapeType = m_DR_subset * NUM_SHAPE_TYPES   +   m_shapeType;

        // Iterate over all design-rule sets (again):
        for (int j = 0; j < user_inputs->numDesignRuleSets; j++)  {

          // Iterate over all design-rule subsets of design-rule set 'j':
          for (int n_DR_subset = 0; n_DR_subset < user_inputs->numDesignRuleSubsets[j]; n_DR_subset++)  {

            // Iterate over all shape-types:
            for (int n_shapeType = 0; n_shapeType < NUM_SHAPE_TYPES; n_shapeType++) {

              // Calculate the second index ('n') used for the matrices. The index is based on the values
              // n_DR_subset and n_shape_type, and is simply 3 * n_DR_subset  +  n_shape_type:
              short n_subset_shapeType = n_DR_subset * NUM_SHAPE_TYPES   +   n_shapeType;

              // Calculate the minimum and maximum of the minimum spacings between shape-types within
              // each of the two design-rule sets. These calculations are necessary because the user does
              // not specify the minimum spacing between shapes from different design-rule sets and subsets
              // (or even within the same design-rule set, i.e,. when i equals j):
              float max_spacing_um = max(user_inputs->designRules[i][m_DR_subset].space_um[m_shapeType][n_shapeType],
                                         user_inputs->designRules[j][n_DR_subset].space_um[m_shapeType][n_shapeType]);
              float min_spacing_um = min(user_inputs->designRules[i][m_DR_subset].space_um[m_shapeType][n_shapeType],
                                         user_inputs->designRules[j][n_DR_subset].space_um[m_shapeType][n_shapeType]);

              // (1) DRC_radius: Note that we use the minimum spacing between different design-rules
              //     and shape-types.
              user_inputs->DRC_radius[i][m_subset_shapeType][j][n_subset_shapeType]
                                       = (user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType] / 2.0
                                                 + min_spacing_um) / user_inputs->cell_size_um;

              // (2) DRC_radius_squared:
              user_inputs->DRC_radius_squared[i][m_subset_shapeType][j][n_subset_shapeType]
                         =   user_inputs->DRC_radius[i][m_subset_shapeType][j][n_subset_shapeType]
                           * user_inputs->DRC_radius[i][m_subset_shapeType][j][n_subset_shapeType];

              #ifdef DEBUG_createUsefulDesignRuleInfo
              printf("DEBUG: DRC_radius[%d][%d][%d][%d] = %.2f = (%.3f / 2.0 + %.3f)/%.3f\n", i, m_subset_shapeType, j, n_subset_shapeType,
                      user_inputs->DRC_radius[i][m_subset_shapeType][j][n_subset_shapeType], user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType],
                      min_spacing_um, user_inputs->cell_size_um);

              printf("DEBUG: DRC_radius_squared[%d][%d][%d][%d] = %.2f\n", i, m_subset_shapeType, j, n_subset_shapeType,
                      user_inputs->DRC_radius_squared[i][m_subset_shapeType][j][n_subset_shapeType]);

              #endif

              //
              // (5) detour_distance:
              //     The 'detour_distance' depends on the shape-types associated with the indices 'm' and 'n':
              //
              #ifdef DEBUG_createUsefulDesignRuleInfo
              printf("\nDEBUG: Calculating detour_distance[%d][%d][%d][%d]...\n", i, m_subset_shapeType, j, n_subset_shapeType);
              #endif
              if ((m_shapeType == TRACE) && (n_shapeType == TRACE))  {

                // Both shape-types are TRACEs. The detour distance is (Ln + Smn + Wm/2) / Wn, where
                // L, S, and W refer to line length, line width, and line spacing, respectively. Because
                // L (line length) is net-specific, we use the average length of all lines:

                #ifdef DEBUG_createUsefulDesignRuleInfo
                printf("DEBUG: routing and congestion are both TRACE types...\n");

                printf("  DEBUG: user_inputs->avg_rats_nest_length_um = %.3f\n", user_inputs->avg_rats_nest_length_um);
                printf("  DEBUG: max_spacing_um = %.3f\n", max_spacing_um);
                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n",
                                 i, m_DR_subset, m_shapeType, user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType]);
                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n",
                                 j, n_DR_subset, n_shapeType, user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]);
                printf("  DEBUG: user_inputs->cell_size_um = %.3f\n", user_inputs->cell_size_um);
                printf("  DEBUG: MAX of above two values is %.3f\n", max(user_inputs->cell_size_um, user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]));
                #endif

                user_inputs->detour_distance[i][m_subset_shapeType][j][n_subset_shapeType]
                    = (user_inputs->avg_rats_nest_length_um + max_spacing_um + user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType] / 2.0)
                       / max(user_inputs->cell_size_um, user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]);

              }  // End of if-block for m and n both referring to TRACE shape-types

              else if ((m_shapeType == TRACE) && (n_shapeType != TRACE))  {

                // The 'm' index is a TRACE, and the 'n' index is a VIA_UP or VIA_DOWN. In this case, the
                // detour distance is (Rn + Smn + Wm/2)/(2Rn), where R, S, and W, are the via radii,
                // via-to-trace spacing, and linewidth, respectively.

                #ifdef DEBUG_createUsefulDesignRuleInfo
                printf("DEBUG: routing is TRACE type; congestion is VIA type...\n");

                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n", j, n_DR_subset, n_shapeType,
                                 user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]);
                printf("  DEBUG: max_spacing_um = %.3f\n", max_spacing_um);
                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n",
                                 i, m_DR_subset, m_shapeType, user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType]);
                printf("  DEBUG: user_inputs->cell_size_um = %.3f\n", user_inputs->cell_size_um);
                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n",
                                 j, n_DR_subset, n_shapeType, user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]);
                printf("  DEBUG: MAX of above two values is %.3f\n", max(user_inputs->cell_size_um, user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]));
                #endif

                user_inputs->detour_distance[i][m_subset_shapeType][j][n_subset_shapeType]
                    = (   user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType] / 2.0  +  max_spacing_um
                       +  user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType] / 2.0)
                       / max(user_inputs->cell_size_um, user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]);

              }  // End of else/if-block for m = TRACE and n = VIA_UP or VIA_DOWN

              else if ((m_shapeType != TRACE) && (n_shapeType == TRACE))  {
                // The 'm' index is a VIA, and the 'n' index is a TRACE. In this case, the
                // detour distance is (Wn/2 + Smn + Rm), where R, S, and W, are the via radii,
                // via-to-trace spacing, and linewidth, respectively.

                #ifdef DEBUG_createUsefulDesignRuleInfo
                printf("DEBUG: routing is VIA type; congestion is TRACE type...\n");

                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n", j, n_DR_subset, n_shapeType,
                                 user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]);
                printf("  DEBUG: max_spacing_um = %.3f\n", max_spacing_um);
                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n",
                                 i, m_DR_subset, m_shapeType, user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType]);
                printf("  DEBUG: user_inputs->cell_size_um = %.3f\n", user_inputs->cell_size_um);
                #endif

                user_inputs->detour_distance[i][m_subset_shapeType][j][n_subset_shapeType]
                    = (   user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType] / 2.0  +  max_spacing_um
                       +  user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType] / 2.0)
                       / user_inputs->cell_size_um;

              }  // End of else/if-block for m = VIA_UP or VIA_DOWN, and n = TRACE

              else if ((m_shapeType != TRACE) && (n_shapeType != TRACE))  {
                // Both shape-types are VIA's. The detour distance is (Rn + Smn + Rm), where
                // R and S refer to via radii and via-to-via spacing, respectively:

                #ifdef DEBUG_createUsefulDesignRuleInfo
                printf("DEBUG: routing and congestion are both VIA types...\n");

                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n", j, n_DR_subset, n_shapeType,
                                 user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType]);
                printf("  DEBUG: max_spacing_um = %.3f\n", max_spacing_um);
                printf("  DEBUG: user_inputs->designRules[%d][%d].width_um[%d] = %.3f\n",
                                 i, m_DR_subset, m_shapeType, user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType]);
                printf("  DEBUG: user_inputs->cell_size_um = %.3f\n", user_inputs->cell_size_um);
                #endif

                user_inputs->detour_distance[i][m_subset_shapeType][j][n_subset_shapeType]
                    = (user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType] / 2.0
                          +  max_spacing_um
                            +  user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType] / 2.0)
                                  / user_inputs->cell_size_um;

              }
              else  {
                // We got here, which is not expected. This reflects an error in the above logic, so
                // issue a fatal error message and exit:
                printf("\n\nERROR: An unexpected situation occurred in subroutine 'createUsefulDesignRuleInfo' when\n");
                printf(    "       calculating elements of the matrix 'detour_distance'. Inform the software developer\n");
                printf(    "       of this error message. Diagnostic information follows:\n");
                printf(    "                       i (DR set) = %d\n", i);
                printf(    "                      m_DR_subset = %d\n", m_DR_subset);
                printf(    "                      m_shapeType = %d\n", m_shapeType);
                printf(    "               m_subset_shapeType = %d\n", m_subset_shapeType);
                printf(    "                       j (DR set) = %d\n", j);
                printf(    "                      n_DR_subset = %d\n", n_DR_subset);
                printf(    "                      n_shapeType = %d\n", n_shapeType);
                printf(    "               n_subset_shapeType = %d\n", n_subset_shapeType);
                printf("\n\n");
                exit(1);
              }

              // Ensure that the detour_distance is at least 0.5 cells. Values of zero are possible
              // for (unrealistic) cases in which the vias have zero diameter and spacing:
              if (user_inputs->detour_distance[i][m_subset_shapeType][j][n_subset_shapeType] < 0.1)  {
                user_inputs->detour_distance[i][m_subset_shapeType][j][n_subset_shapeType] = 0.5;
              }

              // As an experiment, modify the detour_distance to see its effects on escape routing
              // (done on Jan 23 - 27, 2022):
              // user_inputs->detour_distance[i][m_subset_shapeType_index][n_subset_shapeType_index] *= 2;
              // user_inputs->detour_distance[i][m_subset_shapeType_index][n_subset_shapeType_index] /= 2;
              // user_inputs->detour_distance[i][m_subset_shapeType_index][n_subset_shapeType_index] *= 8;
              // user_inputs->detour_distance[i][m_subset_shapeType_index][n_subset_shapeType_index] /= 4;

            }  // End of for-loop for index 'n_shapeType'
          }  // End of for-loop for index 'n_DR_subset'
        }  // End of for-loop for index j = 0 to number of design rules
      }  // End of for-loop for index 'm_shapeType'
    }  // End of for-loop for index 'm_DR_subset'
  }  // End of for-loop for i = 0 to number of design rules


  //
  // For each combination of design-rule sets given by indices i and j, calculate the
  // values in the following matrices with matrices m and n. For each matrix, 'm' and 'n'
  // range from 0 to num_subset_shapeType_indices, where
  // num_subset_shapeType_indices = NUM_SHAPE_TYPES * user_inputs->numDesignRuleSubsets[i]
  //  (1) 'cong_radius' matrix. Each element, cong_radius[m][n], of this
  //      matrix represents the radius[n] + spacing[m][n] + radius[m], in
  //      units of cells.
  //  (2) 'cong_radius_squared[m][n]' matrix, whose elements are the squares
  //      of the 'cong_radius' elements.
  //
  // Iterate over all design-rule sets:
  for (int i = 0; i < user_inputs->numDesignRuleSets; i++)  {

    // Iterate over all design-rule subsets of design-rule set 'i':
    for (int m_DR_subset = 0; m_DR_subset < user_inputs->numDesignRuleSubsets[i]; m_DR_subset++)  {

      // Iterate over all shape-types:
      for (int m_shapeType = 0; m_shapeType < NUM_SHAPE_TYPES; m_shapeType++)  {
        // Calculate the first index ('m') used for the matrices. The index is based on the values
        // m_DR_subset and m_shape_type, and is simply 3 * m_DR_subset  +  m_shape_type:
        short m_subset_shapeType = m_DR_subset * NUM_SHAPE_TYPES   +   m_shapeType;

        // Iterate over all design-rule sets (again):
        for (int j = 0; j < user_inputs->numDesignRuleSets; j++)  {

          // Iterate over all design-rule subsets of design-rule set 'j':
          for (int n_DR_subset = 0; n_DR_subset < user_inputs->numDesignRuleSubsets[j]; n_DR_subset++)  {

            // Iterate over all shape-types:
            for (int n_shapeType = 0; n_shapeType < NUM_SHAPE_TYPES; n_shapeType++) {

              // Calculate the second index ('n') used for the matrices. The index is based on the values
              // n_DR_subset and n_shape_type, and is simply 3 * n_DR_subset  +  n_shape_type:
              short n_subset_shapeType = n_DR_subset * NUM_SHAPE_TYPES   +   n_shapeType;

              // Calculate the minimum of the minimum spacings between shape-types within each
              // of the two design-rule sets. These calculations are necessary because the user does
              // not specify the minimum spacing between shapes from different design-rule sets and subsets
              // (or even within the same design-rule set, i.e,. when i equals j):
              float max_spacing_um = max(user_inputs->designRules[i][m_DR_subset].space_um[m_shapeType][n_shapeType],
                                         user_inputs->designRules[j][n_DR_subset].space_um[m_shapeType][n_shapeType]);

              // (1) cong_radius: Note that we use the maximum spacing between different design-rules
              //     and shape-types in order to repel foreign nets.
              float baseline_cong_radius_cells = (  user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType] / 2.0
                                                  + max_spacing_um
                                                  + user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType] / 2.0  )
                                                  / user_inputs->cell_size_um;


              // Calculate an 'adder' distance that we add to the baseline congestion radius to account for
              // rounding errors when we approximate user-defined (exact) dimensions with less precise,
              // grid-based, discrete dimensions:
              float cong_adder_cells_m = calc_congestion_adder(user_inputs->designRules[i][m_DR_subset].radius[m_shapeType],
                                                               baseline_cong_radius_cells,
                                                               user_inputs->DRC_radius_squared[i][m_subset_shapeType][j][n_subset_shapeType]);

              float cong_adder_cells_n = calc_congestion_adder(user_inputs->designRules[j][n_DR_subset].radius[n_shapeType],
                                                               baseline_cong_radius_cells,
                                                               user_inputs->DRC_radius_squared[j][n_subset_shapeType][i][m_subset_shapeType]);

              float cong_adder_cells = max(cong_adder_cells_m, cong_adder_cells_n);

              #ifdef DEBUG_createUsefulDesignRuleInfo
              if (cong_adder_cells == 0.0)  {
                printf("\nDEBUG: calc_congestion_adder returned %.3f cells for the congestion adder.\n\n", cong_adder_cells);
              }
              else  {
                printf("\nDEBUG: First call to calc_congestion_adder had following inputs:\n");
                printf("DEBUG:                        user_inputs->designRules[i=%d][m_DR_subset=%d].radius[m_shapeType=%d] = %.3f\n",
                       i, m_DR_subset, m_shapeType, user_inputs->designRules[i][m_DR_subset].radius[m_shapeType]);
                printf("DEBUG:                                                                baseline_cong_radius_cells = %.3f\n", baseline_cong_radius_cells);
                printf("DEBUG:     user_inputs->DRC_radius_squared[i=%d][m_subset_shapeType=%d][j=%d][n_subset_shapeType=%d] = %.3f\n",
                       i, m_subset_shapeType, j, n_subset_shapeType, user_inputs->DRC_radius_squared[i][m_subset_shapeType][j][n_subset_shapeType]);

                printf("\nDEBUG: Second call to calc_congestion_adder had following inputs:\n");
                printf("DEBUG:                        user_inputs->designRules[j=%d][n_DR_subset=%d].radius[n_shapeType=%d] = %.3f\n",
                       j, n_DR_subset, n_shapeType, user_inputs->designRules[j][n_DR_subset].radius[n_shapeType]);
                printf("DEBUG:                                                                baseline_cong_radius_cells = %.3f\n", baseline_cong_radius_cells);
                printf("DEBUG:     user_inputs->DRC_radius_squared[j=%d][n_subset_shapeType=%d][i=%d][m_subset_shapeType=%d] = %.3f\n",
                       j, n_subset_shapeType, i, m_subset_shapeType, user_inputs->DRC_radius_squared[j][n_subset_shapeType][i][m_subset_shapeType]);

                printf("\nDEBUG: In createUsefulDesignRuleInfo after both calls to calc_congestion_adder:\n");
                printf("DEBUG:       cong_adder_cells_m = %.3f\n", cong_adder_cells_m);
                printf("DEBUG:       cong_adder_cells_n = %.3f\n", cong_adder_cells_n);
                printf("DEBUG:         cong_adder_cells = %.3f\n\n", cong_adder_cells);
              }
              #endif

              user_inputs->cong_radius[i][m_subset_shapeType][j][n_subset_shapeType] = baseline_cong_radius_cells + cong_adder_cells;

              // (2) cong_radius_squared:
              user_inputs->cong_radius_squared[i][m_subset_shapeType][j][n_subset_shapeType]
                         =   max(0.9, user_inputs->cong_radius[i][m_subset_shapeType][j][n_subset_shapeType]
                                       * user_inputs->cong_radius[i][m_subset_shapeType][j][n_subset_shapeType]);

              #ifdef DEBUG_createUsefulDesignRuleInfo
              if (   user_inputs->designRules[i][m_DR_subset].isPseudoNetSubset
                                || user_inputs->designRules[j][n_DR_subset].isPseudoNetSubset)  {
                printf("DEBUG: cong_radius[%d][%d][%d][%d] = %.2f = %.2f + (%.3f / 2.0 + %.3f + %.3f / 2.0) / %.3f\n",
                        i, m_subset_shapeType, j, n_subset_shapeType, user_inputs->cong_radius[i][m_subset_shapeType][j][n_subset_shapeType],
                        cong_adder_cells, user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType], max_spacing_um,
                        user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType], user_inputs->cell_size_um);
              }
              else {
                printf("DEBUG: cong_radius[%d][%d][%d][%d] = %.2f = %.2f + (%.3f / 2.0 + %.3f + %.3f / 2.0) / %.3f\n",
                        i, m_subset_shapeType, j, n_subset_shapeType, user_inputs->cong_radius[i][m_subset_shapeType][j][n_subset_shapeType],
                        cong_adder_cells, user_inputs->designRules[i][m_DR_subset].width_um[m_shapeType], max_spacing_um,
                        user_inputs->designRules[j][n_DR_subset].width_um[n_shapeType], user_inputs->cell_size_um);
              }

              printf("DEBUG: cong_radius_squared[%d][%d][%d][%d] = %.2f\n", i, m_subset_shapeType, j, n_subset_shapeType,
                      user_inputs->cong_radius_squared[i][m_subset_shapeType][j][n_subset_shapeType]);
              #endif

            }  // End of for-loop for index 'n_shapeType'
          }  // End of for-loop for index 'n_DR_subset'
        }  // End of for-loop for index j = 0 to number of design rules
      }  // End of for-loop for index 'm_shapeType'
    }  // End of for-loop for index 'm_DR_subset'
  }  // End of for-loop for i = 0 to number of design rules


  //
  // Populate the 3-dimensional matrix 'foreign_DR_subset' that uses the names of the design-rule
  // subsets to map these subsets across different design-rule sets.
  //
  for (int DR_set_1 = 0; DR_set_1 < user_inputs->numDesignRuleSets; DR_set_1++)  {

    // Iterate over all design-rule subsets of design-rule set 'DR_set_1':
    for (int DR_subset_1 = 0; DR_subset_1 < user_inputs->numDesignRuleSubsets[DR_set_1]; DR_subset_1++)  {

      // Iterate (again) over all design-rule sets:
      for (int DR_set_2 = 0; DR_set_2 < user_inputs->numDesignRuleSets; DR_set_2++)  {

        // Define a flag to indicate whether a matching subset name was found between different design-rule sets:
        unsigned char found_matching_subset_name = FALSE;

        // Iterate over all design-rule subsets of design-rule set 'DR_set_2':
        for (int DR_subset_2 = 0; DR_subset_2 < user_inputs->numDesignRuleSubsets[DR_set_2]; DR_subset_2++)  {

          // Check if the subset names are equal, and if the 'isPseudoNetSubset' flags are equal:
          if (   (strcmp(user_inputs->designRules[DR_set_1][DR_subset_1].subsetName, user_inputs->designRules[DR_set_2][DR_subset_2].subsetName) == 0)
              && (user_inputs->designRules[DR_set_1][DR_subset_1].isPseudoNetSubset == user_inputs->designRules[DR_set_2][DR_subset_2].isPseudoNetSubset))  {

            // We got here, so the design-rule subset names are equal, and the 'isPseudoNetSubset' flags are equal. So these
            // subsets support the same nets in the netlist, and therefore correspond to each other:
            user_inputs->foreign_DR_subset[DR_set_1][DR_subset_1][DR_set_2] = DR_subset_2;

            // Set the flag to TRUE to indicate that a matching subset name was found
            found_matching_subset_name = TRUE;

            #ifdef DEBUG_createUsefulDesignRuleInfo
            printf("DEBUG: Design-rule sets/subsets correspond to each other: %d / %d (%s) <<===>> %d / %d (%s)\n",
                   DR_set_1, DR_subset_1, user_inputs->designRules[DR_set_1][DR_subset_1].subsetName, DR_set_2, DR_subset_2,
                   user_inputs->designRules[DR_set_2][DR_subset_2].subsetName);
            printf("DEBUG:   user_inputs->foreign_DR_subset[DR_set_1=%d][DR_subset_1=%d][DR_set_2=%d] = %d\n", DR_set_1,
                   DR_subset_1, DR_set_2, user_inputs->foreign_DR_subset[DR_set_1][DR_subset_1][DR_set_2]);
            #endif

            break; // Break out of for-loop for index DR_subset_2

          }  // End of if-block
        }  // End of for-lop for index 'DR_subset_2'

        // Check whether we found a subset name in design-rule set DR_set_2 that matches the name
        // of subset #DR_subset_1 in design-rule set DR_set_1:
        if (! found_matching_subset_name)  {

          #ifdef DEBUG_createUsefulDesignRuleInfo
          printf("DEBUG: No design-rule subset named '%s' (from design-rule set %d) exists in design-rule set %d\n",
                 user_inputs->designRules[DR_set_1][DR_subset_1].subsetName, DR_set_1, DR_set_2);
          #endif

          // We got here so no matching subset name was found between the two design-rule sets. So we
          // map the unmatched design-rule subset in design-rule set # DR_set_1 to the default subset
          // (subset #0) of design-rule set # DR_set_2, but only if they have the same values of
          // 'isPseudoNetSubset':
          if (   user_inputs->designRules[DR_set_1][DR_subset_1].isPseudoNetSubset
              == user_inputs->designRules[DR_set_2][     0     ].isPseudoNetSubset) {

            user_inputs->foreign_DR_subset[DR_set_1][DR_subset_1][DR_set_2] = 0;

            #ifdef DEBUG_createUsefulDesignRuleInfo
            printf("DEBUG: Mapping set %d / subset %d (%s) to set %d / subset %d (%s)\n",
                   DR_set_1, DR_subset_1, user_inputs->designRules[DR_set_1][DR_subset_1].subsetName,
                   DR_set_2, 0,           user_inputs->designRules[DR_set_2][0].subsetName);
            printf("DEBUG:   user_inputs->foreign_DR_subset[DR_set_1=%d][DR_subset_1=%d][DR_set_2=%d] = %d\n", DR_set_1,
                   DR_subset_1, DR_set_2, user_inputs->foreign_DR_subset[DR_set_1][DR_subset_1][DR_set_2]);
            #endif

          }  // End of if-block for equal 'isPseudoNetSubset' values
          else  {
            // We got here, so we cannot map the unmatched subset name between different
            // design-rule sets because the unmatched name is a diff-pair subset, but the
            // default subset is not.

            // We next check whether the unmatched subset name is used by any user-defined
            // nets:
            int subset_not_used_by_nets = TRUE;
            for (int pathNum = 0; pathNum < user_inputs->num_nets; pathNum++)  {
              if (0 == strcasecmp(user_inputs->netSpecificRuleName[pathNum], user_inputs->designRules[DR_set_1][DR_subset_1].subsetName))  {
                subset_not_used_by_nets = FALSE;
                break;  // Break out of for-loop
              }  // End of if-block for comparing unmatched subset name to exception name for path 'pathNum'
            }  // End of for-loop for index 'pathNum'

            // Issue a fatal error or warning message, depending on whether the unmatched design-rule subset
            // is used, or is not used, by any nets:
            if  (subset_not_used_by_nets)  {
              printf("\nWARNING: Design-rule set '%s' has an exception named '%s' that has \n",
                     user_inputs->designRuleSetName[DR_set_1], user_inputs->designRules[DR_set_1][DR_subset_1].subsetName);
              printf(  "         no identically named exception in design-rule set '%s'. This would cause problems when\n",
                     user_inputs->designRuleSetName[DR_set_2]);
              printf(  "         nets traverse the boundary between these two design-rule zones. However, no nets use this particular\n");
              printf(  "         exception. Consider removing this design-rule exception from the input file to improve (reduce)\n");
              printf(  "         run-times in the future.\n\n");
            }
            else  {
              printf("\nERROR: A fatal error was detected in the input file. Design-rule set '%s' has an exception\n",
                     user_inputs->designRuleSetName[DR_set_1]);
              printf(  "       named '%s' that is used for differential pairs. This exception has no identically named \n",
                     user_inputs->designRules[DR_set_1][DR_subset_1].subsetName);
              printf(  "       exception in design-rule set '%s', which causes problems when nets traverse the boundary\n",
                     user_inputs->designRuleSetName[DR_set_2]);
              printf(  "       between these two design-rule zones. Please modify the input file so that each differential-pair\n");
              printf(  "       exception has an identically named exception in every other design-rule set.\n\n");
              exit(1);
            }  // End of if/sle block for warning/error message
          }  // End of else-block for detecting an unmatched diff-pair design-rule subset
        }  // End of if-block for (found_matching_subset_name == FALSE)
      }  // End of for-loop for index 'DR_set_2'
    }  // End of for-loop for index 'DR_subset_1'
  }  // End of for-loop for index 'DR_set_1'

  #ifdef DEBUG_createUsefulDesignRuleInfo
  //
  // Debug code follows.
  //
  for (int i = 0; i < user_inputs->numDesignRuleSets; i++)  {
    int num_indices = NUM_SHAPE_TYPES * user_inputs->numDesignRuleSubsets[i];
    printf("DEBUG --------------------------------------------------------------------------------------------\n");
    printf("DEBUG: cong_radius[m][n] for design rule set %d (%s):\n", i, user_inputs->designRuleSetName[i]);
    printf("DEBUG:         |");
    for (int m = 0; m < num_indices; m++)
      printf(" m = %2d |", m);
    printf("\n");
    printf("DEBUG:         |");
    for (int m = 0; m < num_indices; m++)
      printf("--------|");
    printf("\n");
    for (int n = 0; n < num_indices; n++)  {
      printf("DEBUG: n = %3d |", n);
      for (int m = 0; m < num_indices; m++)  {
        printf(" %6.2f |",  user_inputs->cong_radius[i][m][i][n]);
      }  // End of for-loop for index 'm'
      printf("\n");
    }  // End of for-loop for index 'n'
    printf("DEBUG:         -");
    for (int m = 0; m < num_indices; m++)
      printf("---------");
    printf("\n");
    printf("DEBUG --------------------------------------------------------------------------------------------\n");
    printf("DEBUG:\n");
    printf("DEBUG --------------------------------------------------------------------------------------------\n");
    printf("DEBUG: DRC_radius[m][n] for design rule set %d (%s):\n", i, user_inputs->designRuleSetName[i]);
    printf("DEBUG:         |");
    for (int m = 0; m < num_indices; m++)
      printf(" m = %2d |", m);
    printf("\n");
    printf("DEBUG:         |");
    for (int m = 0; m < num_indices; m++)
      printf("--------|");
    printf("\n");
    for (int n = 0; n < num_indices; n++)  {
      printf("DEBUG: n = %3d |", n);
      for (int m = 0; m < num_indices; m++)  {
        printf(" %6.2f |",  user_inputs->DRC_radius[i][m][i][n]);
      }  // End of for-loop for index 'm'
      printf("\n");
    }  // End of for-loop for index 'n'
    printf("DEBUG:         -");
    for (int m = 0; m < num_indices; m++)
      printf("---------");
    printf("\n");
    printf("DEBUG --------------------------------------------------------------------------------------------\n");

    printf("DEBUG:\n");
    printf("DEBUG --------------------------------------------------------------------------------------------\n");
    printf("DEBUG: detour_distance[m][n] for design rule set %d (%s):\n", i, user_inputs->designRuleSetName[i]);
    printf("DEBUG:         |");
    for (int m = 0; m < num_indices; m++)
      printf(" m = %2d |", m);
    printf("\n");
    printf("DEBUG:         |");
    for (int m = 0; m < num_indices; m++)
      printf("--------|");
    printf("\n");
    for (int n = 0; n < num_indices; n++)  {
      printf("DEBUG: n = %3d |", n);
      for (int m = 0; m < num_indices; m++)  {
        printf("  %6.2f|",  user_inputs->detour_distance[i][m][i][n]);
      }  // End of for-loop for index 'm'
      printf("\n");
    }  // End of for-loop for index 'n'
    printf("DEBUG:         -");
    for (int m = 0; m < num_indices; m++)
      printf("---------");
    printf("\n");
    printf("DEBUG --------------------------------------------------------------------------------------------\n");

  }  // End of for-loop for i = 0 to number of design-rule sets
  //
  // End of debug code.
  //
  #endif

}  // End of function 'createUsefulDesignRuleInfo'


//-----------------------------------------------------------------------------
// Name: defineDefaultDesignRuleSet
// Desc: If the input file contains no user-defined design-rule sets, then
//       define a default design-rule set. In this set, all spaces, trace
//       widths, and via diameters are set to the equivalent of 1 cell.
//-----------------------------------------------------------------------------
void defineDefaultDesignRuleSet(InputValues_t *user_inputs)  {

  strcpy(user_inputs->designRuleSetName[0], "_DEFAULT_RULE_");
  strcpy(user_inputs->designRuleSetDescription[0], "Default design-rule set with minimum linewidths/spaces");
  strcpy(user_inputs->designRules[0][0].subsetName, "_NO EXCEPTION_");

  // Set all widths and spaces equal to 1 cell size:
  user_inputs->designRules[0][0].viaUpDiameterMicrons
      = user_inputs->designRules[0][0].width_um[VIA_UP]
      = user_inputs->designRules[0][0].lineWidthMicrons
      = user_inputs->designRules[0][0].width_um[TRACE]
      = user_inputs->designRules[0][0].viaDownDiameterMicrons
      = user_inputs->designRules[0][0].width_um[VIA_DOWN]
      = user_inputs->designRules[0][0].lineSpacingMicrons
      = user_inputs->designRules[0][0].space_um[TRACE][TRACE]
      = user_inputs->designRules[0][0].viaUpToTraceSpacingMicrons
      = user_inputs->designRules[0][0].space_um[VIA_UP][TRACE]
      = user_inputs->designRules[0][0].space_um[TRACE][VIA_UP]
      = user_inputs->designRules[0][0].viaDownToTraceSpacingMicrons
      = user_inputs->designRules[0][0].space_um[VIA_DOWN][TRACE]
      = user_inputs->designRules[0][0].space_um[TRACE][VIA_DOWN]
      = user_inputs->designRules[0][0].viaUpToViaUpSpacingMicrons
      = user_inputs->designRules[0][0].space_um[VIA_UP][VIA_UP]
      = user_inputs->designRules[0][0].viaDownToViaDownSpacingMicrons
      = user_inputs->designRules[0][0].space_um[VIA_DOWN][VIA_DOWN]
      = user_inputs->designRules[0][0].viaUpToViaDownSpacingMicrons
      = user_inputs->designRules[0][0].space_um[VIA_UP][VIA_DOWN]
      = user_inputs->designRules[0][0].space_um[VIA_DOWN][VIA_UP]
      = user_inputs->cell_size_um;

  user_inputs->numDesignRuleSets = 1;
  printf("\nINFO: Because the input file contained no user-defined design-rule sets,\n");
  printf(  "      a default set will be used. All spaces, line widths, and via diameters\n");
  printf(  "      are set to %.2f microns, which is the grid resolution specified in the\n",
            user_inputs->cell_size_um);
  printf(  "      input file.\n\n");

}  // End of function 'defineDefaultDesignRuleSet'


//-----------------------------------------------------------------------------
// Name: verify_net_designRule_consistency
// Desc: Verify that design-rule exceptions that contain the ‘diff_pair_pitch’
//       keyword are not used for nets that don’t contain a diff-pair partner net.
//-----------------------------------------------------------------------------
void verify_net_designRule_consistency(InputValues_t *user_inputs)  {

  // Iterate through all the nets:
  for (int path = 0; path < user_inputs->num_nets; path++)  {

    // Check if current net is a diff-pair net:
    if (! user_inputs->isDiffPair[path])  {

      // We got here, so current net is not a diff-pair net. Now
      // iterate through all design-rule sets:
      for (int DR_set = 0; DR_set < user_inputs->numDesignRuleSets; DR_set++)  {

        // Get the design-rule subset for this design-rule and this path:
        int DR_subset = user_inputs->designRuleSubsetMap[path][DR_set];

        // Check whether this design-rule subset is dedicated to diff-pair nets:
        if (user_inputs->designRules[DR_set][DR_subset].isDiffPairSubset)  {

          // printf("DEBUG: user_inputs->designRules[DR_set=%d][DR_subset=%d].isDiffPairSubset = %d.\n",
          //        DR_set, DR_subset, user_inputs->designRules[DR_set][DR_subset].isDiffPairSubset);
          // printf("DEBUG: user_inputs->designRules[DR_set=%d][DR_subset=%d].diffPairPitchCells[TRACE] = %d.\n",
          //        DR_set, DR_subset, user_inputs->designRules[DR_set][DR_subset].diffPairPitchCells[TRACE]);

          // We got here, so the current net ('path') is not defined as a diff-pair
          // net, yet it uses a design-rule subset with a 'diff_pair_pitch' definition.
          // This is not allowed, so issue an error message and exit the program:
          printf("\nERROR: Net '%s' is not defined as a diff-pair net in the input file, yet\n",
                  user_inputs->net_name[path]);
          printf("       the input file defines this net as using a design-rule exception, '%s', that\n",
                  user_inputs->designRules[DR_set][DR_subset].subsetName);
          printf("       defines a pitch for differential pairs. This is not allowed. Please modify\n");
          printf("       the input file so that non-diff-pair nets use design rules that don't contain\n");
          printf("       the 'diff_pair_pitch' keyword.\n\n");
          exit(1);
        }  // End of if-block for isDiffPairSubset == TRUE
      }  // End of for-loop for index 'DR_set'
    }  // End of if-block for (isDiffPair == FALSE)
  }  // End of for-loop for index 'path'

}  // End of function 'verify_net_designRule_consistency'

