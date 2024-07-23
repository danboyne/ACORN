#include "global_defs.h"
#include "createDiffPairs.h"


//-----------------------------------------------------------------------------
// Name: getMaxCongRadiusToSegment
// Desc: Retrieve the maximum cong_radius value between all the segments in a
//       via-stack and all possible shape-types of a diff-pair segment. Allow
//       for the possibility that the 'via-stack' contains no vias, i.e., it's
//       simply a trace-segment on a single layer. In this case, the startSegment
//       and endSegment values are identical. Also allow for the case that
//       the via-stack contains the start-terminal, in which case the
//       startSegment element is '-1', and the endSegment element is also '-1'.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_postProcess' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_getMaxCongRadiusToSegment 1
#undef DEBUG_getMaxCongRadiusToSegment

static int getMaxCongRadiusToSegment(ViaStack_t viaStack, int diffPairPath, int diffPairSegment,
                                     Coordinate_t *pathCoords[], int pathLength[],
                                     const InputValues_t *user_inputs, CellInfo_t ***cellInfo,
                                     const MapInfo_t *mapInfo)  {

  // Ensure that the 'viaStack' structure indeed contains a valid via structure by
  // checking that its 'error' element is FALSE. If it's true, then issue a fatal
  // error message:
  if (viaStack.error)  {
    printf("\nERROR: Function 'getMaxCongRadiusToSegment' received an unexpected input value in which the\n");
    printf(  "       via-stack 'error' flag was set to TRUE, indicating that the via-stack is not valid.\n");
    printf(  "       Inform the software developer of this fatal error message.\n");
    printf(  "                    viaStack.error = %d\n", viaStack.error);
    printf(  "                  viaStack.pathNum = %d\n", viaStack.pathNum);
    printf(  "             viaStack.startSegment = %d\n", viaStack.startSegment);
    printf(  "               viaStack.startCoord = (%d,%d,%d)\n", viaStack.startCoord.X, viaStack.startCoord.Y, viaStack.startCoord.Z);
    printf(  "               viaStack.endSegment = %d\n", viaStack.endSegment);
    printf(  "                 viaStack.endCoord = (%d,%d,%d)\n", viaStack.endCoord.X, viaStack.endCoord.Y, viaStack.endCoord.Z);
    printf(  "             viaStack.endShapeType = %d\n", viaStack.endShapeType);
    printf(  "               viaStack.isVertical = %d\n", viaStack.isVertical);
    exit(1);
  }


  // Define the maximum cong_radius value that will be returned from this function:
  int max_cong_radius = 0.0;

  // Get (x,y,z) coordinates of the diff-pair path's segment:
  int diffPair_X = pathCoords[diffPairPath][diffPairSegment].X;
  int diffPair_Y = pathCoords[diffPairPath][diffPairSegment].Y;
  int diffPair_Z = pathCoords[diffPairPath][diffPairSegment].Z;

  // Get design-rule set and subset of the diff-pair segment:
  int diff_pair_DR = cellInfo[diffPair_X][diffPair_Y][diffPair_Z].designRuleSet;
  int diff_pair_DR_subset = user_inputs->designRuleSubsetMap[diffPairPath][diff_pair_DR];

  // Calculate the number of segments in the via-stack:
  int num_via_segments = viaStack.endSegment - viaStack.startSegment + 1;

  // Determine whether the ending segment in the via-stack is a VIA_UP, a VIA_DOWN, or a TRACE:
  int endViaSegment_shapeType = viaStack.endShapeType;
  if (num_via_segments == 1)  {
    // Via-stack is not really a via. It's either the start- or end terminal:
    endViaSegment_shapeType = TRACE;
  }

  //
  // Iterate of all possible shape-types of the diff-pair segment:
  //
  for (int diffPair_shapeType = 0; diffPair_shapeType < NUM_SHAPE_TYPES; diffPair_shapeType++)  {

    // Calculate the subset/shapeType index for the diff-pair segment:
    int diffPair_subset_shapeType = diff_pair_DR_subset * NUM_SHAPE_TYPES   +   diffPair_shapeType;

    // Iterate over all segments (layers) of the via-stack:
    for (int viaSegment = viaStack.startSegment; viaSegment <= viaStack.endSegment; viaSegment++)  {

      // Get (x,y,z) coordinates of the via's segment:
      int via_X, via_Y, via_Z;
      if (viaSegment >= 0)  {
        via_X = pathCoords[viaStack.pathNum][viaSegment].X;
        via_Y = pathCoords[viaStack.pathNum][viaSegment].Y;
        via_Z = pathCoords[viaStack.pathNum][viaSegment].Z;
      }
      else  {
        // If the viaSegment is -1, then it represents the start-terminal. Fetch the
        // coordinates of the start-terminal:
        via_X = mapInfo->start_cells[viaStack.pathNum].X;
        via_Y = mapInfo->start_cells[viaStack.pathNum].Y;
        via_Z = mapInfo->start_cells[viaStack.pathNum].Z;
      }

      // Get the design-rule set at the location of the via segment:
      int via_DR = cellInfo[via_X][via_Y][via_Z].designRuleSet;

      // Get the design-rule subset of a diff-pair path that hypothetically is located in the
      // same design-rule set as the via-stack.
      int via_DR_subset = user_inputs->designRuleSubsetMap[diffPairPath][via_DR];
      int via_subset_shapeType; // Shape-type of segment in via-stack

      #ifdef DEBUG_getMaxCongRadiusToSegment
      printf("DEBUG: In getMaxCongRadiusToSegment, analyzing pseudo-via segment #%d at (%d,%d,%d) with DR number %d and subset %d,\n",
             viaSegment, via_X, via_Y, via_Z, via_DR, via_DR_subset);
      printf("DEBUG: with diff-pair #%d, segment %d at (%d,%d,%d) with DR number %d, subset %d, and shape-type %d.\n",
             diffPairPath, diffPairSegment, diffPair_X, diffPair_Y, diffPair_Z, diff_pair_DR, diff_pair_DR_subset, diffPair_shapeType);
      #endif

      // Consider 4 cases for each segment in the via-stack: (1) segment is a trace on a single layer
      // (i.e., it's a terminal, and not really a via), or (2) segment is only a VIA_UP segment,
      // or (3) segment is only a VIA_DOWN segment, or (4) segment is both a VIA_UP and VIA_DOWN segment,
      // (i.e., it's in the middle of a via-stack):
      //
      // Case #1: Segment is a trace on a single layer at the start- or end-terminal:
      if (num_via_segments == 1)  {

        // We got here, so this 'via stack' is really just a trace (either at the start- or end-terminal). So
        // we calculate the subset/shapeType index using the TRACE shape-type:
        via_subset_shapeType = via_DR_subset * NUM_SHAPE_TYPES   +   TRACE;

        #ifdef DEBUG_getMaxCongRadiusToSegment
        printf("DEBUG:   In getMaxCongRadiusToSegment, pseudo-via segment is a TRACE\n");
        printf("DEBUG:     cong_radius[%d][%d][%d][%d] = %d\n", via_DR, via_subset_shapeType, diff_pair_DR, diffPair_subset_shapeType,
               user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType]);
        #endif

        // Compare the cong_radius to the current 'max_cong_radius' value. If it's greater ,then replace 'max_cong_radius'
        // with the cong_radius value:
        if (user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType] > max_cong_radius)  {
          max_cong_radius = user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType];
        }
      }  // End of if-block for segment being a TRACE

      // Case #2: Segment is only a VIA_UP segment
      else if (   ((viaSegment == viaStack.endSegment)   && (endViaSegment_shapeType == VIA_UP))
               || ((viaSegment == viaStack.startSegment) && (endViaSegment_shapeType == VIA_DOWN))) {
        // We got here, so the current segment is either the start-segment while the end-segment is a
        // via-down (making the current segment a via-up), or the current segment is the end-segment and
        // it's a via-up shape-type.  So we calculate the the subset/shapeType index using both the
        // via shape-type and a TRACE shape-type (because there must be a TRACE at the top and
        // bottom of a via-stack):

        // Consider the TRACE shape-type first:
        via_subset_shapeType = via_DR_subset * NUM_SHAPE_TYPES   +   TRACE;

        #ifdef DEBUG_getMaxCongRadiusToSegment
        printf("DEBUG:   In getMaxCongRadiusToSegment, pseudo-via segment is a TRACE or VIA_UP\n");
        printf("DEBUG:     cong_radius[%d][%d][%d][%d] = %d\n", via_DR, via_subset_shapeType, diff_pair_DR, diffPair_subset_shapeType,
               user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType]);
        #endif

        // Compare the cong_radius to the current 'max_cong_radius' value. If it's greater ,then replace 'max_cong_radius'
        // with the cong_radius value:
        if (user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType] > max_cong_radius)  {
          max_cong_radius = user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType];
        }

        // Secondly, consider the VIA_UP shape-type:
        via_subset_shapeType = via_DR_subset * NUM_SHAPE_TYPES   +   VIA_UP;

        #ifdef DEBUG_getMaxCongRadiusToSegment
        printf("DEBUG:     cong_radius[%d][%d][%d][%d] = %d\n", via_DR, via_subset_shapeType, diff_pair_DR, diffPair_subset_shapeType,
               user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType]);
        #endif

        // Compare the cong_radius to the current 'max_cong_radius' value. If it's greater ,then replace 'max_cong_radius'
        // with the cong_radius value:
        if (user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType] > max_cong_radius)  {
          max_cong_radius = user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType];
        }
      }  // End of if-block for segment being a VIA_UP only

      // Case #3: Segment is only a VIA_DOWN segment
      else if (( num_via_segments == 2)
                  && (   ((viaSegment == viaStack.endSegment)   && (endViaSegment_shapeType == VIA_DOWN))
                      || ((viaSegment == viaStack.startSegment) && (endViaSegment_shapeType == VIA_UP)))) {

        // We got here, so the current segment is either the start-segment while the end-segment is a
        // via-up (making the current segment a via-down), or the current segment is the end-segment and
        // it's a via-down shape-type.  So we calculate the the subset/shapeType index using both the
        // via shape-type and a TRACE shape-type (because there must be a TRACE at the top and
        // bottom of a via-stack):

        // Consider the TRACE shape-type first:
        via_subset_shapeType = via_DR_subset * NUM_SHAPE_TYPES   +   TRACE;

        #ifdef DEBUG_getMaxCongRadiusToSegment
        printf("DEBUG:   In getMaxCongRadiusToSegment, pseudo-via segment is a TRACE OR VIA_DOWN\n");
        printf("DEBUG:     cong_radius[%d][%d][%d][%d] = %d\n", via_DR, via_subset_shapeType, diff_pair_DR, diffPair_subset_shapeType,
               user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType]);
        #endif

        // Compare the cong_radius to the current 'max_cong_radius' value. If it's greater ,then replace 'max_cong_radius'
        // with the cong_radius value:
        if (user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType] > max_cong_radius)  {
          max_cong_radius = user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType];
        }

        // Secondly, consider the VIA_DOWN shape-type:
        via_subset_shapeType = via_DR_subset * NUM_SHAPE_TYPES   +   VIA_DOWN;

        #ifdef DEBUG_getMaxCongRadiusToSegment
        printf("DEBUG:     cong_radius[%d][%d][%d][%d] = %d\n", via_DR, via_subset_shapeType, diff_pair_DR, diffPair_subset_shapeType,
               user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType]);
        #endif

        // Compare the cong_radius to the current 'max_cong_radius' value. If it's greater ,then replace 'max_cong_radius'
        // with the cong_radius value:
        if (user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType] > max_cong_radius)  {
          max_cong_radius = user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType];
        }
      }  // End of if-block for segment being a VIA_DOWN only

      // Case #4: Segment is both a VIA_UP and VIA_DOWN segment
      else  {
        // We got here, so this via-stack segment is in the middle of the via-stack. That is, it's both
        // a VIA_UP and a VIA_DOWN via.

        // First, consider the VIA_UP case:
        via_subset_shapeType = via_DR_subset * NUM_SHAPE_TYPES   +   VIA_UP;

        #ifdef DEBUG_getMaxCongRadiusToSegment
        printf("DEBUG:   In getMaxCongRadiusToSegment, pseudo-via segment is a VIA_UP and VIA_DOWN\n");
        printf("DEBUG:     cong_radius[%d][%d][%d][%d] = %d\n", via_DR, via_subset_shapeType, diff_pair_DR, diffPair_subset_shapeType,
               user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType]);
        #endif

        // Compare the cong_radius to the current 'max_cong_radius' value. If it's greater ,then replace 'max_cong_radius'
        // with the cong_radius value:
        if (user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType] > max_cong_radius)  {
          max_cong_radius = user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType];
        }

        // Next, consider the VIA_DOWN case:
        via_subset_shapeType = via_DR_subset * NUM_SHAPE_TYPES   +   VIA_DOWN;

        #ifdef DEBUG_getMaxCongRadiusToSegment
        printf("DEBUG:     cong_radius[%d][%d][%d][%d] = %d\n", via_DR, via_subset_shapeType, diff_pair_DR, diffPair_subset_shapeType,
               user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType]);
        #endif

        // Compare the cong_radius to the current 'max_cong_radius' value. If it's greater ,then replace 'max_cong_radius'
        // with the cong_radius value:
        if (user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType] > max_cong_radius)  {
          max_cong_radius = user_inputs->cong_radius[via_DR][via_subset_shapeType][diff_pair_DR][diffPair_subset_shapeType];
        }
      }  // End of else-block for via segment being both a VIA_UP and VIA_DOWN shape-type
    }  // End of for-loop for index 'viaSegment'
  }  // End of for-loop for index 'diffPair_shapeType'

  #ifdef DEBUG_getMaxCongRadiusToSegment
  printf("DEBUG:   In getMaxCongRadiusToSegment, max_cong_radius = %d\n", max_cong_radius);
  #endif

  return(max_cong_radius);

}  // End of function 'getMaxCongRadiusToSegment'


//-----------------------------------------------------------------------------
// Name: calc_2D_unit_vector_ints
// Desc: Calculate the X- and Y-components of a unit vector along the line from
//       point 'startPoint' to point 'endPoint'. Both points have Z-coordinates,
//       which are not taken into account. All input coordinates must be
//       integers. The function will exit gracefully if the two points have
//       identical (x,y) coordinates.
//-----------------------------------------------------------------------------
static Vector2dFloat_t calc_2D_unit_vector_ints(Coordinate_t startPoint, Coordinate_t endPoint)  {

  // Define variable to be returned from this function:
  Vector2dFloat_t unit_vector;

  // Calculate the 2-dimensional distance between the two points:
  int delta_x = endPoint.X - startPoint.X;
  int delta_y = endPoint.Y - startPoint.Y;

  // Confirm that the two input coordinates do not have the same (x,y) values, which would
  // prohibit the calculation of a vector between the two coordinates:
  if ((delta_x == 0) && (delta_y == 0))  {
    printf("\nERROR: An unexpected condition was detected in function 'calc_2D_unit_vector_ints', which was asked to\n");
    printf(  "       calculate a unit-vector between two points with identical (x,y) coordinates. The two points were\n");
    printf(  "               (%d, %d, %d) and (%d, %d, %d)\n", startPoint.X, startPoint.Y, startPoint.Z,
                                                               endPoint.X,   endPoint.Y,   endPoint.Z);
    printf(  "       Inform the software developer of this fatal error message.\n\n");
    exit(1);
  }  // End of if-block for detecting coincident (x,y) coordinates

  // Calculate the 2-dimensional distance between the two points:
  float magnitude = sqrt(delta_x * delta_x   +   delta_y * delta_y);

  // Calculate the x- and y-components of the unit vector:
  unit_vector.X = delta_x / magnitude;
  unit_vector.Y = delta_y / magnitude;

  // Return the unit vector to the calling program:
  return(unit_vector);

}  // End of function 'calc_2D_unit_vector_ints'


//-----------------------------------------------------------------------------
// Name: calcAbsCosine
// Desc: Calculate absolute value of the cosine of the angle between unit vector
//       'unitVector' (with floating-point components) and a vector between the
//       points 'point_1' and 'point_2', which must have integer coordinates.
//
//       For the special case that the two points have identical (x,y) coordinates
//       (which prohibits the calculation of an angle), this function returns
//       zero.
//-----------------------------------------------------------------------------
static float calcAbsCosine(Vector2dFloat_t unitVector, Coordinate_t point_1, Coordinate_t point_2)  {

  // Calculate the X- and Y-components of the vector between 'point_1' and 'point_2':
  int delta_x = point_2.X - point_1.X;
  int delta_y = point_2.Y - point_1.Y;

  // Confirm that the two input coordinates do not have the same (x,y) values, which would
  // prohibit the calculation of a vector between the two coordinates:
  if ((delta_x == 0) && (delta_y == 0))  {
    printf("\nWARNING: (thread %2d) Function 'calcAbsCosine' was asked to calculate a vector between two points with identical (x,y) coordinates:\n", omp_get_thread_num());
    printf(  "         (thread %2d)         (%d, %d, %d) and (%d, %d, %d)\n", omp_get_thread_num(), point_1.X, point_1.Y, point_1.Z,
                                                                                                   point_2.X, point_2.Y, point_2.Z);
    printf(  "         (thread %2d) The function will return zero because an angle cannot be calculated in this situation.\n", omp_get_thread_num());
    return(0);
  }  // End of if-block for detecting coincident (x,y) coordinates

  // Calculate the magnitude of the vector between point_1 and point_2:
  float magnitude = sqrt(delta_x * delta_x   +   delta_y * delta_y);

  // Define variable to be returned from this function. We take the floating-point absolute
  // value (fabs) of the dot-product of the two vectors divided by the magnitudes of the
  // two vectors. (The magnitude of the unit-vector is unity, so it doesn't show up in the
  // equation below.)
  float abs_cosine = fabs((unitVector.X * delta_x  +  unitVector.Y * delta_y) / magnitude);

  // Return the absolute value of the cosine to the calling program:
  return(abs_cosine);

}  // End of function 'calcAbsCosine'


//-----------------------------------------------------------------------------
// Name: markDiffPairSegmentsNearPseudoVia
// Desc: Mark for deletion all segments of the two diff-pair paths associated with
//       pseudo-path 'pseudoPathNum' that are near the corresponding via in the
//       pseudo-path. Diff-pair segments are deleted if they are within the
//       following distance from the pseudo-via:
//
//            cong_radius[i][m][j][n] + Rdpv |COS(theta)|,
//
//       where Rdpv is half the distance between the diff-pair vias, and theta is
//       the angle between the line through the diff-pair vias and a line from the
//       pseudo-via to the path-segment that could be deleted. As a reminder,
//       the cong_radius values are equal to:
//             cong_radius[m][n] = radius[n] + spacing[m][n] + radius[m]
//
//       The indices for cong_radius are:
//        i = design-rule set at pseudo-via cell (Note: we assume that the design-rule set
//            at the pseudo-via location is the same set as at the diff-pair via location)
//        m = design-rule subset and shape-type of a diff-pair path in design-rule set i
//        j = design-rule set at the diff-pair segment that we’re considering deleting
//        n = design-rule subset and shape-type of the diff-pair path that we’re
//            considering deleting in design-rule zone j.
//
//       This function modifies the 2-dimensional array 'deleteSegment' by setting bit
//       #0 (LSB) of the element associated with the segment to be deleted.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_markDiffPairSegmentsNearPseudoVia' and re-compile if you want
// verbose debugging print-statements enabled:
//
// #define DEBUG_markDiffPairSegmentsNearPseudoVia 1
#undef DEBUG_markDiffPairSegmentsNearPseudoVia

static void markDiffPairSegmentsNearPseudoVia(const int pseudoPathNum, const int path_1_number,
                                              const int path_2_number, Coordinate_t *pathCoords[],
                                              int pathLength[], const InputValues_t *user_inputs,
                                              CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                                              unsigned char ** deleteSegment)  {

  // Get the start-locations of the pseudo-path:
  int pseudoStartX = mapInfo->start_cells[pseudoPathNum].X;
  int pseudoStartY = mapInfo->start_cells[pseudoPathNum].Y;
  int pseudoStartZ = mapInfo->start_cells[pseudoPathNum].Z;

  //
  // Define a 2-element array for the two diff-pair path numbers, which allows us to
  // parameterize and streamline algorithms below:
  //
  int diffPairPathNum[2];
  diffPairPathNum[0] = path_1_number;
  diffPairPathNum[1] = path_2_number;

  #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (   (mapInfo->current_iteration >= 264) && (mapInfo->current_iteration <= 280)
      && (pseudoPathNum == 6))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in markDiffPairSegmentsNearPseudoVia() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE

  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Entered function 'markDiffPairSegmentsNearPseudoVia' with pseudo-path %d (length %d), diff-pair path %d (length %d) and path %d (length %d)\n",
            omp_get_thread_num(), pseudoPathNum, pathLength[pseudoPathNum], path_1_number, pathLength[path_1_number], path_2_number, pathLength[path_2_number]);
  }
  #endif

  // Temporary variable to hold coordinates of previous segment of pseudo-path
  Coordinate_t prevPseudoSegmentCoords;
  Coordinate_t viaCoordsInSwapZone;
  viaCoordsInSwapZone.X = viaCoordsInSwapZone.Y = viaCoordsInSwapZone.Z = 0;
  Coordinate_t pseudoCoordsBeforeVia;

  // Iterate through all segments of the pseudo-path to locate each via. Do not
  // include the start- and end-segments (terminals) because a different algorithm
  // will handle vias at these locations.
  prevPseudoSegmentCoords.X = pseudoStartX;
  prevPseudoSegmentCoords.Y = pseudoStartY;
  prevPseudoSegmentCoords.Z = pseudoStartZ;
  pseudoCoordsBeforeVia     = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);
  int pseudoSegmentBeforeVia    = -1;  // Keeps track of segment before most recent via in pseudo-path
  int via_starts_in_swap_zone   = FALSE;  // Boolean variable that tracks whether pseudo-path via-stack starts in a swap-zone
  for (int pseudoPathSegment = 0; pseudoPathSegment < pathLength[pseudoPathNum]-1; pseudoPathSegment++)  {

    #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Pseudo-path #%d, segment %d: (%d,%d,%d) in swap-zone #%d. The previous segment was at (%d,%d,%d)\n", omp_get_thread_num(),
              pseudoPathNum, pseudoPathSegment, pathCoords[pseudoPathNum][pseudoPathSegment].X, pathCoords[pseudoPathNum][pseudoPathSegment].Y,
              pathCoords[pseudoPathNum][pseudoPathSegment].Z,
              cellInfo[pathCoords[pseudoPathNum][pseudoPathSegment].X][pathCoords[pseudoPathNum][pseudoPathSegment].Y][pathCoords[pseudoPathNum][pseudoPathSegment].Z].swap_zone,
              prevPseudoSegmentCoords.X, prevPseudoSegmentCoords.Y, prevPseudoSegmentCoords.Z);
      printf("DEBUG: (thread %2d)    pathLength[path_1_number=%d] = %d\n", omp_get_thread_num(), path_1_number, pathLength[path_1_number]);
    }
    #endif

    // If current segment is on the same routing layer (Z-value) as the previous segment, then save
    // this segment number as the last segment before the via. (This will be updated/overwritten each
    // time through the loop, until a via is actually discovered.)
    if (pseudoCoordsBeforeVia.Z == pathCoords[pseudoPathNum][pseudoPathSegment].Z)  {
      #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Updating pseudoSegmentBeforeVia from %d to %d\n", omp_get_thread_num(), pseudoSegmentBeforeVia, pseudoPathSegment);
      }
      #endif
      pseudoSegmentBeforeVia = pseudoPathSegment;
      pseudoCoordsBeforeVia = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);
    }  // End of if-block for recording 'pseudoSegmentBeforeVia'

    //
    // Check the rare case in which the starting terminal is in a swap-zone and and is
    // also the first segment of a via-stack:
    //
    if ((pseudoPathSegment == 0) && (pseudoStartZ != pathCoords[pseudoPathNum][pseudoPathSegment].Z))  {

      #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)   Beginning of a via-stack was detected at the pseudo-net's start-terminal (%d,%d,%d)\n",
                omp_get_thread_num(), pseudoStartX, pseudoStartY, pseudoStartZ);
      }
      #endif

      // We found the beginning of a via-stack at a terminal. Now check whether the terminal is located
      // in a swap-zone. For such via-stacks, no via segments exist from the swap-zone:
      if (cellInfo[pseudoStartX][pseudoStartY][pseudoStartZ].swap_zone)  {
        via_starts_in_swap_zone = TRUE;
        viaCoordsInSwapZone.X = pseudoStartX;
        viaCoordsInSwapZone.Y = pseudoStartY;
        viaCoordsInSwapZone.Z = pseudoStartZ;

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      Via-stack starts at a terminal in swap-zone #%d at (%d,%d,%d).\n",
                 omp_get_thread_num(), cellInfo[pseudoStartX][pseudoStartY][pseudoStartZ].swap_zone, pseudoStartX, pseudoStartY, pseudoStartZ);
        }
        #endif
      }
      else  {
        via_starts_in_swap_zone = FALSE;

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      Via-stack does NOT start in a swap-zone.\n", omp_get_thread_num());
        }
        #endif
      }
    }  // End of if-block for identifying the start-terminal as the beginning of a via-stack in the pseudo-path

    //
    // Check whether the current pseudo-path segment is the first segment of a via-stack:
    //
    else if (   (pathCoords[pseudoPathNum][pseudoPathSegment].Z == prevPseudoSegmentCoords.Z)
             && (pseudoPathSegment < pathLength[pseudoPathNum] - 1)
             && (pathCoords[pseudoPathNum][pseudoPathSegment].Z != pathCoords[pseudoPathNum][pseudoPathSegment+1].Z))  {

      // We found the beginning of a via-stack. Now check whether the first segment is located in a swap-zone.
      // For such via-stacks, no shoulder-path vias will be created later on:
      if (cellInfo[pathCoords[pseudoPathNum][pseudoPathSegment].X][pathCoords[pseudoPathNum][pseudoPathSegment].Y][pathCoords[pseudoPathNum][pseudoPathSegment].Z].swap_zone)  {
        via_starts_in_swap_zone = TRUE;
        viaCoordsInSwapZone = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      Via-stack starts in swap-zone #%d at (%d,%d,%d).\n", omp_get_thread_num(),
                  cellInfo[pseudoStartX][pseudoStartY][pseudoStartZ].swap_zone, pathCoords[pseudoPathNum][pseudoPathSegment].X,
                  pathCoords[pseudoPathNum][pseudoPathSegment].Y, pathCoords[pseudoPathNum][pseudoPathSegment].Z);
        }
        #endif
      }
      else  {
        via_starts_in_swap_zone = FALSE;

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      Via-stack does NOT start in a swap-zone.\n", omp_get_thread_num());
        }
        #endif
      }
    }  // End of if-block for locating the beginning of a via-stack in the pseudo-path


    // If current segment of pseudo-net satisfies the following criteria, then a via should exist in the shoulder paths:
    //   (a) pseudo-path segment is NOT in a swap-zone, AND
    //   (b) pseudo-path segment is on a different layer (Z-coordinate) than the previous segment, AND
    //   (c) the current segment is the last segment in the pseudo-path, or the next segment is on the same layer, AND
    //   (d) the X/Y coordinate is not the start- or end-coordinate of the pseudo-net (which would mean that the
    //       via is directly above/below one of the terminals), OR the via-stack began in a swap-zone:
    if ((! cellInfo[pathCoords[pseudoPathNum][pseudoPathSegment].X][pathCoords[pseudoPathNum][pseudoPathSegment].Y][pathCoords[pseudoPathNum][pseudoPathSegment].Z].swap_zone)
        && (pathCoords[pseudoPathNum][pseudoPathSegment].Z != prevPseudoSegmentCoords.Z)
        && ((pseudoPathSegment == pathLength[pseudoPathNum]-1)
               || (pathCoords[pseudoPathNum][pseudoPathSegment].Z == pathCoords[pseudoPathNum][pseudoPathSegment+1].Z))
        && ((via_starts_in_swap_zone)
              || (  ( !   ( (pathCoords[pseudoPathNum][pseudoPathSegment].X == mapInfo->start_cells[pseudoPathNum].X)
                       && (pathCoords[pseudoPathNum][pseudoPathSegment].Y == mapInfo->start_cells[pseudoPathNum].Y)))
                  && ( !  ( (pathCoords[pseudoPathNum][pseudoPathSegment].X == mapInfo->end_cells[pseudoPathNum].X)
                       && (pathCoords[pseudoPathNum][pseudoPathSegment].Y == mapInfo->end_cells[pseudoPathNum].Y))))))  {

      #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) We got to end of a pseudo-via-stack at pseudoPathSegment %d located at (%d,%d,%d).\n",
                omp_get_thread_num(), pseudoPathSegment, pathCoords[pseudoPathNum][pseudoPathSegment].X,
                                                         pathCoords[pseudoPathNum][pseudoPathSegment].Y,
                                                         pathCoords[pseudoPathNum][pseudoPathSegment].Z);

        if (! via_starts_in_swap_zone)  {
          printf("DEBUG: (thread %2d)   The pseudo-path segment at the beginning of this via-stack is segment %d at (%d,%d,%d)\n", omp_get_thread_num(),
                 pseudoSegmentBeforeVia, pseudoCoordsBeforeVia.X, pseudoCoordsBeforeVia.Y, pseudoCoordsBeforeVia.Z);
        }
        else  {
          printf("DEBUG: (thread %2d)   The pseudo-path segment at the beginning of this via-stack is segment %d at (%d,%d,%d), in a swap-zone\n", omp_get_thread_num(),
                 pseudoSegmentBeforeVia, viaCoordsInSwapZone.X, viaCoordsInSwapZone.Y, viaCoordsInSwapZone.Z);
        }
      }
      #endif

      // Save the X/Y coordinates of the pseudo-via, which we'll use later in this if-block:
      int pseudoViaX = pathCoords[pseudoPathNum][pseudoPathSegment].X;
      int pseudoViaY = pathCoords[pseudoPathNum][pseudoPathSegment].Y;

      // Save the attributes of the current via-stack, which we'll use later in this function:
      ViaStack_t pseudoViaStack;
      pseudoViaStack.startSegment = pseudoSegmentBeforeVia;
      if (pseudoSegmentBeforeVia == -1)  {
        pseudoViaStack.startCoord   = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);
      }
      else  {
        pseudoViaStack.startCoord   = copyCoordinates(pathCoords[pseudoPathNum][pseudoSegmentBeforeVia]);
      }
      pseudoViaStack.endSegment   = pseudoPathSegment;
      pseudoViaStack.endCoord     = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);
      pseudoViaStack.pathNum      = pseudoPathNum;
      if (pathCoords[pseudoPathNum][pseudoPathSegment].Z > prevPseudoSegmentCoords.Z)  {
        pseudoViaStack.endShapeType = VIA_DOWN;
      }
      else  {
        pseudoViaStack.endShapeType = VIA_UP;
      }
      pseudoViaStack.isVertical = TRUE;
      pseudoViaStack.error = FALSE;

      // We found a layer-transition in the pseudo-net. Now find the corresponding layer-
      // transitions in the two shoulder-paths. These transitions will be captured in variables
      // layerTransition[0] and layerTransition[1]:
      ViaStack_t layerTransition[2];

      #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
      if (DEBUG_ON)  {
        if (pseudoSegmentBeforeVia == -1)  {
          printf("DEBUG: (thread %2d) Pseudo-path #%d transitioned layers from segment %d (%d,%d,%d) to %d (%d,%d,%d).\n", omp_get_thread_num(),
                 pseudoPathNum, pseudoSegmentBeforeVia, mapInfo->start_cells[pseudoPathNum].X,
                 mapInfo->start_cells[pseudoPathNum].Y, mapInfo->start_cells[pseudoPathNum].Z,
                 pseudoPathSegment, pseudoViaX, pseudoViaY, pathCoords[pseudoPathNum][pseudoPathSegment].Z);
        }
        else  {
          printf("DEBUG: (thread %2d) Pseudo-path #%d transitioned layers from segment %d (%d,%d,%d) to %d (%d,%d,%d).\n", omp_get_thread_num(),
                 pseudoPathNum, pseudoSegmentBeforeVia, pseudoCoordsBeforeVia.X, pseudoCoordsBeforeVia.Y, pseudoCoordsBeforeVia.Z,
                 pseudoPathSegment, pseudoViaX, pseudoViaY, pathCoords[pseudoPathNum][pseudoPathSegment].Z);
        }
      }
      #endif

      //
      // Iterate over both diff-pair paths to find the diff-pair vias associated with the
      // pseudo-via:
      //
      for (int i = 0; i < 2; i++)  {
        // Find the layer-transtion in each shoulder-path:
        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Calling findNearbyLayerTransition_wrapper from markDiffPairSegmentsNearPseudoVia with pathNum = %d, startLayer = %d and endLayer = %d\n",
                  omp_get_thread_num(), diffPairPathNum[i], pseudoCoordsBeforeVia.Z, pathCoords[pseudoPathNum][pseudoPathSegment].Z);
        }
        #endif

        // Locate the nearest layer-transition in the diff-pair shoulder-path:
        layerTransition[i] = findNearbyLayerTransition_wrapper(diffPairPathNum[i], pathLength, pathCoords, pseudoCoordsBeforeVia.Z,
                                                               pathCoords[pseudoPathNum][pseudoPathSegment].Z, pathCoords[pseudoPathNum][pseudoPathSegment].X,
                                                               pathCoords[pseudoPathNum][pseudoPathSegment].Y, mapInfo, user_inputs);

        // Confirm that a layer-transition was found in the shoulder-path:
        if (layerTransition[i].endShapeType == TRACE)  {
          printf("\nERROR: (thread %2d) Function 'findNearbyLayerTransition_wrapper' failed to find a layer-transition in diff-pair path %d (%s) corresponding\n",
                 omp_get_thread_num(), diffPairPathNum[i], user_inputs->net_name[diffPairPathNum[i]]);
          printf(  "ERROR: (thread %2d) to the pseudo-via at coordinates (%d,%d) from layer %d (%s) to layer %d (%s). Please inform the\n", omp_get_thread_num(),
                 pathCoords[pseudoPathNum][pseudoPathSegment].X, pathCoords[pseudoPathNum][pseudoPathSegment].Y, pseudoCoordsBeforeVia.Z,
                 user_inputs->routingLayerNames[pseudoCoordsBeforeVia.Z],
                 pathCoords[pseudoPathNum][pseudoPathSegment].Z, user_inputs->routingLayerNames[pathCoords[pseudoPathNum][pseudoPathSegment].Z]);
          printf(  "ERROR: (thread %2d) software developer of this fatal error message.\n\n", omp_get_thread_num());
          exit(1);
        }

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          int x, y, z;
          if (layerTransition[i].startSegment == -1)  {
            x = mapInfo->start_cells[diffPairPathNum[i]].X;
            y = mapInfo->start_cells[diffPairPathNum[i]].Y;
            z = mapInfo->start_cells[diffPairPathNum[i]].Z;
          }
          else  {
            x = pathCoords[diffPairPathNum[i]][layerTransition[i].startSegment].X;
            y = pathCoords[diffPairPathNum[i]][layerTransition[i].startSegment].Y;
            z = pathCoords[diffPairPathNum[i]][layerTransition[i].startSegment].Z;
          }
          printf("       (thread %2d) Shoulder path %d transitioned from segment %d (%d,%d,%d) to %d (%d,%d,%d).\n", omp_get_thread_num(),
                 diffPairPathNum[i], layerTransition[i].startSegment, x, y, z,
                 layerTransition[i].endSegment, pathCoords[diffPairPathNum[i]][layerTransition[i].endSegment].X,
                 pathCoords[diffPairPathNum[i]][layerTransition[i].endSegment].Y, pathCoords[diffPairPathNum[i]][layerTransition[i].endSegment].Z);
        }
        #endif

      }  // End of for-loop for index 'i' (from 0 to 1)

      //
      // Based on design rules, calculate the ideal distance between the centers of the two
      // diff-pair vias. This distance is the maximum of the following:
      //            (a) (Dvu + Svu) for the via-up ('vu') layer,
      //            (b) (Dvd + Svd) for the via-down ('vd') layer,
      //            (c) (Wline + Pitch) for any layer involved in the via.
      //
      // Iterate over each segment of the pseudo-path that makes up the current via-stack,
      // keeping track of the maximum distance between centers of diff-pair vias:
      float diffPairViaPitch_cells = 0.0;  // Value of pitch in cell-units
      #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Analyzing design rules from pseudo-path via segments %d to %d (inclusive)...\n", omp_get_thread_num(), pseudoSegmentBeforeVia, pseudoPathSegment);
      }
      #endif
      for (int i = pseudoSegmentBeforeVia; i <= pseudoPathSegment; i++)  {

        // If i equals -1, then the via-stack started at the start-terminal. Skip
        // this segment of the via-stack:
        if (i < 0)  {
          continue;
        }

        // Get the design-rule number for this coordinate/layer:
        int DR_num = cellInfo[pathCoords[pseudoPathNum][i].X][pathCoords[pseudoPathNum][i].Y][pathCoords[pseudoPathNum][i].Z].designRuleSet;

        // Get the design-rule subset number of the pseudo net:
        int DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num];

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)  For pseudo-path segment %d at (%d,%d,%d), design-rule number is %d\n", omp_get_thread_num(), i, pathCoords[pseudoPathNum][i].X,
                  pathCoords[pseudoPathNum][i].Y, pathCoords[pseudoPathNum][i].Z, DR_num);
          printf("DEBUG: (thread %2d)  Design-rule subset for pseudo-net %d is %d\n", omp_get_thread_num(), pseudoPathNum, DR_subset);
        }
        #endif


        // Get the via-up distance if the previous or subsequent pseudo-path segment
        // has a layer number that's greater than the current layer number, or the via-stack
        // starts in a swap-zone:
        if (   ((i < pathLength[pseudoPathNum] - 1) && (pathCoords[pseudoPathNum][i+1].Z > pathCoords[pseudoPathNum][i].Z))
            || ((i > 0)                             && (pathCoords[pseudoPathNum][i-1].Z > pathCoords[pseudoPathNum][i].Z))
            || ( via_starts_in_swap_zone            && (viaCoordsInSwapZone.Z            > pathCoords[pseudoPathNum][i].Z)) )  {

          diffPairViaPitch_cells = max(diffPairViaPitch_cells, user_inputs->designRules[DR_num][DR_subset].diffPairPitchCells[VIA_UP]);

          #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) viaUpLimited_distance = %5.2f \n",
                   omp_get_thread_num(), user_inputs->designRules[DR_num][DR_subset].minDiffPairViaUpPitchMicrons);
          }
          #endif

        }  // End of if-block for upward-going via

        // Get the via-down distance if the previous or subsequent pseudo-path segment
        // has a layer number that's less than the current layer number, or the via starts in
        // a swap-zone:
        if (   ((i < pathLength[pseudoPathNum] - 1) && (pathCoords[pseudoPathNum][i+1].Z < pathCoords[pseudoPathNum][i].Z))
            || ((i > 0)                             && (pathCoords[pseudoPathNum][i-1].Z < pathCoords[pseudoPathNum][i].Z))
            || ( via_starts_in_swap_zone            && (viaCoordsInSwapZone.Z            < pathCoords[pseudoPathNum][i].Z)) )  {

          diffPairViaPitch_cells = max(diffPairViaPitch_cells, user_inputs->designRules[DR_num][DR_subset].diffPairPitchCells[VIA_DOWN]);

          #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) viaDownLimited_distance = %5.2f\n",
                   omp_get_thread_num(), user_inputs->designRules[DR_num][DR_subset].minDiffPairViaDownPitchMicrons);
          }
          #endif

        }  // End of if-block for downward-going via
      }  // End of for-loop for index 'i' (each segment of pseudo-via stack)

      #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Found ideal distance between via centers of diff-pair paths %d and %d: %5.5f microns.\n",
               omp_get_thread_num(), path_1_number, path_2_number, diffPairViaPitch_um);
      }
      #endif

      // Use the ideal distance between diff-pair via-centers to calculate the radius of these diff-pair
      // vias (Rdpv), relative to the pseudo-via:
      float radius_diffPairvias = diffPairViaPitch_cells / 2.0;


      // Calculate a unit vector that points along the line between the two diff-pair vias:
      Vector2dFloat_t diffPairViasUnitVector;     // Variable to hold X- and Y-coordinates of unit-vector.
      char diffPairViasUnitVector_exists = FALSE; // Boolean flag to indicate whether the unit-vector was successfully calculated.

      // First, determine whether both diff-pair 'vias' were indeed found and both are true vertical stacks:
      if (   (! layerTransition[0].error)  && (! layerTransition[1].error)
          && layerTransition[0].isVertical && layerTransition[1].isVertical)  {

        // We got here, so the two diff-pair vias are indeed vertically stacked vias. So we next confirm that the
        // two diff-pair vias are not at the same (x,y) coordinates:
        if (   (pathCoords[path_1_number][layerTransition[0].endSegment].X != pathCoords[path_2_number][layerTransition[1].endSegment].X)
            || (pathCoords[path_1_number][layerTransition[0].endSegment].Y != pathCoords[path_2_number][layerTransition[1].endSegment].Y))  {

          // We got here, so the two diff-pair vias are not located at the same (x,y) coordinates. So we
          // can calculate a unit vector between them:
          diffPairViasUnitVector = calc_2D_unit_vector_ints(pathCoords[path_1_number][layerTransition[0].endSegment],
                                                            pathCoords[path_2_number][layerTransition[1].endSegment]);
          diffPairViasUnitVector_exists = TRUE;

          #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Unit vector from diff-pair via (%d,%d,%d) to diff-pair via (%d,%d,%d) is (%6.3f, %6.3f)\n", omp_get_thread_num(),
                   pathCoords[path_1_number][layerTransition[0].endSegment].X,
                   pathCoords[path_1_number][layerTransition[0].endSegment].Y,
                   pathCoords[path_1_number][layerTransition[0].endSegment].Z,
                   pathCoords[path_2_number][layerTransition[1].endSegment].X,
                   pathCoords[path_2_number][layerTransition[1].endSegment].Y,
                   pathCoords[path_2_number][layerTransition[1].endSegment].Z,
                   diffPairViasUnitVector.X, diffPairViasUnitVector.Y);
          }
          #endif
        }  // End of if-block for confirming that diff-pair vias are not at same (x,y) location
      }  // End of if-block for vias being vertically aligned
      else  {
        // We got here, so the two diff-pair vias are not vertically stacked vias. So we create a unit vector based
        // on the directions of the pseudo-path before and after the pseudo-via.
        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Two diff-pair vias at (%d,%d,%d) and (%d,%d,%d) are not vertically aligned.\n", omp_get_thread_num(),
                  pathCoords[path_1_number][layerTransition[0].endSegment].X,
                  pathCoords[path_1_number][layerTransition[0].endSegment].Y,
                  pathCoords[path_1_number][layerTransition[0].endSegment].Z,
                  pathCoords[path_2_number][layerTransition[1].endSegment].X,
                  pathCoords[path_2_number][layerTransition[1].endSegment].Y,
                  pathCoords[path_2_number][layerTransition[1].endSegment].Z);
          printf("DEBUG: (thread %2d) So a unit-vector will be created based on the pseudo-path trace before and after the pseudo-via.\n", omp_get_thread_num());
        }
        #endif

        // Calculate the unit vector based on the pseudo-path segments immediately before and
        // after the pseudo-via:
        diffPairViasUnitVector = calcUnitVectorToDiffPairVia_wrapper(pseudoPathNum, pseudoSegmentBeforeVia, pseudoPathSegment,
                                                                     pathLength, pathCoords, mapInfo, cellInfo, radius_diffPairvias);
        diffPairViasUnitVector_exists = TRUE;

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Unit vector based on coordinates of pseudo-path before/after pseudo via is (%6.3f, %6.3f)\n",
                 omp_get_thread_num(), diffPairViasUnitVector.X, diffPairViasUnitVector.Y);
        }
        #endif

      }  // End of else-block for diff-pair vias *not* being vertically aligned



      //
      // Iterate over both diff-pair paths to find the diff-pair segments to mark for deletion.
      // For each of the two diff-pair paths, we mark segments for deletion in two steps:
      //   Step 1: Segments towards the beginning of the path,
      //   Step 2: Segments towards the end of the path.
      //
      for (int i = 0; i < 2; i++)  {

        //
        // Step #1: Mark segments for deletion, starting from the beginning of the
        // shoulder-path and progressing forward towards the via-stack:
        //
        int mark_segment_for_deletion = FALSE;
        float deletion_radius = 0.0;

        // Define the last segment to mark prior to the layer-transition. For vertically-aligned
        // via-segments, this segment is (startSegment - 1). If the via-segments are *not*
        // vertically aligned, then we also include segment 'startSegment'. That is, it's OK to
        // delete 'via' segments if they're not vertically stacked.

        // The following code prevents vertically-aligned via-segments from being deleted:
        int stopSegment;
        if (layerTransition[i].isVertical)  {
          stopSegment = layerTransition[i].startSegment - 1;
        }
        else  {
          stopSegment = layerTransition[i].startSegment;
        }

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          if (stopSegment >= 0)  {
            printf("DEBUG: (thread %2d) Checking segments #%d up to %d (inclusive) of path %d for marking for deletion in markDiffPairSegmentsNearPseudoVia.\n",
                   omp_get_thread_num(), 0, stopSegment, diffPairPathNum[i]);
          }
          else  {
            printf("DEBUG: (thread %2d) stopSegment is -1 (the start-terminal), so no segments will be checked for deleting in markDiffPairSegmentsNearPseudoVia.\n",
                   omp_get_thread_num());
          }
        }
        #endif

        for (int path_segment = 0; path_segment <= stopSegment; path_segment++)  {

          // Mark the current segment for deletion if:
          //   a. The flag 'mark_segment_for_deletion' is already TRUE, AND
          //   b. The current segment is on the same layer as the top or bottom of the pseudo-via.
          if (   (mark_segment_for_deletion)                                                                              // Item (a) above
              && (   (pathCoords[diffPairPathNum[i]][path_segment].Z == pseudoCoordsBeforeVia.Z)                          // Item (b) above
                  || (pathCoords[diffPairPathNum[i]][path_segment].Z == pathCoords[pseudoPathNum][pseudoPathSegment].Z))) // Item (b) above
          {

            // Set bit #0 of the deleteSegment element by OR'ing it with '1':
            deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 1;

            #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Marking segment %d (%d,%d,%d) for deletion from path %d because a segment closer to the start-terminal was closer than %5.2f cells to the pseudo-via at (%d,%d)\n",
                      omp_get_thread_num(), path_segment, pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                      pathCoords[diffPairPathNum[i]][path_segment].Z, diffPairPathNum[i], deletion_radius, pseudoViaX, pseudoViaY);
            }
            #endif

            // Move on to next segment in diff-pair path
            continue;
          }  // End of if-block for (mark_segment_for_deletion == TRUE)

          // Calculate the maximum congestion radius (cong_radius) between the current segment and any
          // layer in the pseudo-via:
          #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
          if (DEBUG_ON)  {
            printf("DEBUG: Calling 'getMaxCongRadiusToSegment' from markDiffPairSegmentsNearPseudoVia...\n");
          }
          #endif
          int max_cong_radius = getMaxCongRadiusToSegment(pseudoViaStack, diffPairPathNum[i], path_segment, pathCoords, pathLength, user_inputs, cellInfo, mapInfo);

          // Calculate the absolute value of the cosine of the angle between the line through the diff-pair
          // vias ('diffPairViasUnitVector') and a line from the pseudo-via to the path-segment:
          float abs_cosine_theta = 0.0;
          if (diffPairViasUnitVector_exists)  {
            abs_cosine_theta = calcAbsCosine(diffPairViasUnitVector, pathCoords[pseudoPathNum][pseudoPathSegment],
                                                                     pathCoords[diffPairPathNum[i]][path_segment]);
          }

          // Calculate a 'deletion radius' about the pseudo-via, within which all diff-pair segments should
          // be marked for deletion:
          deletion_radius = max_cong_radius +  abs_cosine_theta * radius_diffPairvias;

          // Calculate the distance between the current diff-pair path segment and the pseudo-via:
          float segment_to_pseudoVia_distance = calc_2D_Pythagorean_distance_ints(pathCoords[diffPairPathNum[i]][path_segment].X,
                                                                                  pathCoords[diffPairPathNum[i]][path_segment].Y,
                                                                                  pseudoViaX, pseudoViaY);

          // Mark this segment for deletion if:
          //  a. the distance to the current diff-pair segment is less than the deletion_radius, AND
          //  b. the current segment is on the same layer as the top or bottom of the pseudo-via.
          //
          // Also set the flag 'mark_segment_for_deletion' so that all segments will be marked for
          // deletion between the current segment and the the diff-pair via:
          if (   (segment_to_pseudoVia_distance < deletion_radius)                                                         // Item (a) above
              && (   (pathCoords[diffPairPathNum[i]][path_segment].Z == pseudoCoordsBeforeVia.Z)                           // Item (b) above
                  || (pathCoords[diffPairPathNum[i]][path_segment].Z == pathCoords[pseudoPathNum][pseudoPathSegment].Z)))  // Item (b) above
          {
            // Set bit #0 of the deleteSegment element by OR'ing it with '1':
            deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 1;

            #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Marking segment %d for deletion from path %d because its position (%d,%d,%d) is %5.2f cells from pseudo-via at (%d,%d).\n",
                     omp_get_thread_num(), path_segment, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                     pathCoords[diffPairPathNum[i]][path_segment].Z, segment_to_pseudoVia_distance,
                     pseudoViaX, pseudoViaY);
              printf("DEBUG: (thread %2d)     Threshold is %5.2f = max_cong_radius (%d) + Rdpv (%5.2f) x cosine(theta) (%5.2f).\n", omp_get_thread_num(),
                     deletion_radius, max_cong_radius, radius_diffPairvias, abs_cosine_theta);
            }
            #endif

            // Set 'mark_segment_for_deletion' flag to TRUE so that all subsequent segments
            // will be marked for deletion:
            mark_segment_for_deletion = TRUE;
          }  // End of if-block for distance being less than 'deletion_radius'

        }  // End of for-loop for marking segments starting from the start-terminal


        //
        // Step #2: Mark segments for deletion, starting from the end of the
        // shoulder-path and progressing backwards towards the via-stack. We
        // exclude the end-terminal in order to avoid deleting this segment.
        //
        mark_segment_for_deletion = FALSE;
        deletion_radius = 0.0;

        // Define the last segment to mark prior to the layer-transition. For vertically-aligned
        // via-segments, this segment is (endSegment + 1). If the via-segments are *not*
        // vertically aligned, then we also include segment 'endSegment'. That is, it's OK to
        // delete 'via' segments if they're not vertically stacked.

        // The following code prevents vertically-aligned via-segments from being deleted:
        if (layerTransition[i].isVertical)  {
          stopSegment = layerTransition[i].endSegment + 1;
        }
        else  {
          stopSegment = layerTransition[i].endSegment;
        }

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Checking segments #%d down to %d (inclusive) of path %d for marking for deletion in markDiffPairSegmentsNearPseudoVia.\n",
                 omp_get_thread_num(), pathLength[diffPairPathNum[i]] - 2, stopSegment, diffPairPathNum[i]);
        }
        #endif

        for (int path_segment = pathLength[diffPairPathNum[i]] - 2; path_segment >= stopSegment; path_segment--)  {

          // If 'mark_segment_for_deletion' is already TRUE, then simply mark this segment for
          // deletion and move on to the next segment:

          // Mark the current segment for deletion if:
          //   a. The flag 'mark_segment_for_deletion' is already TRUE, AND
          //   b. The current segment is on the same layer as the top or bottom of the pseudo-via.
          if (   (mark_segment_for_deletion)                                                                                // Item (a) above
              && (   (pathCoords[diffPairPathNum[i]][path_segment].Z == pseudoCoordsBeforeVia.Z)                            // Item (b) above
                  || (pathCoords[diffPairPathNum[i]][path_segment].Z == pathCoords[pseudoPathNum][pseudoPathSegment].Z)))   // Item (b) above
          {
            // Set bit #0 of the deleteSegment element by OR'ing it with '1':
            deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 1;

            #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Marking segment %d (%d,%d,%d) for deletion from path %d because a segment closer to the end-terminal was closer than %5.2f cells to the pseudo-via at (%d,%d)\n",
                      omp_get_thread_num(), path_segment, pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                      pathCoords[diffPairPathNum[i]][path_segment].Z, diffPairPathNum[i], deletion_radius, pseudoViaX, pseudoViaY);
            }
            #endif

            // Move on to next segment in diff-pair path
            continue;
          }  // End of if-block for (mark_segment_for_deletion == TRUE)

          // Calculate the maximum congestion radius (cong_radius) between the current segment and any
          // layer in the pseudo-via:
          #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
          if (DEBUG_ON)  {
            printf("DEBUG: Calling 'getMaxCongRadiusToSegment' from markDiffPairSegmentsNearPseudoVia...\n");
          }
          #endif
          int max_cong_radius = getMaxCongRadiusToSegment(pseudoViaStack, diffPairPathNum[i], path_segment, pathCoords, pathLength, user_inputs, cellInfo, mapInfo);

          // Calculate the absolute value of the cosine of the angle between the line through the diff-pair
          // vias ('diffPairViasUnitVector') and a line from the pseudo-via to the path-segment:
          float abs_cosine_theta = calcAbsCosine(diffPairViasUnitVector, pathCoords[pseudoPathNum][pseudoPathSegment],
                                                                         pathCoords[diffPairPathNum[i]][path_segment]);

          #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)    In markDiffPairSegmentsNearPseudoVia, calcAbsCosine returned %5.3f with unit vector of (%5.3f,%5.3f), pseudo-via at (%d,%d), and segment at (%d,%d,%d)\n",
                   omp_get_thread_num(), abs_cosine_theta, diffPairViasUnitVector.X, diffPairViasUnitVector.Y, pathCoords[pseudoPathNum][pseudoPathSegment].X,
                   pathCoords[pseudoPathNum][pseudoPathSegment].Y, pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                   pathCoords[diffPairPathNum[i]][path_segment].Z);
          }
          #endif

          // Calculate a 'deletion radius' about the pseudo-via, within which all diff-pair segments should
          // be marked for deletion:
          deletion_radius = max_cong_radius +  abs_cosine_theta * radius_diffPairvias;

          // Calculate the distance between the current diff-pair path segment and the pseudo-via:
          float segment_to_pseudoVia_distance = calc_2D_Pythagorean_distance_ints(pathCoords[diffPairPathNum[i]][path_segment].X,
                                                                                  pathCoords[diffPairPathNum[i]][path_segment].Y,
                                                                                  pseudoViaX, pseudoViaY);

          // Mark this segment for deletion if:
          //  a. the distance between the current diff-pair segment is less than the deletion_radius, AND
          //  b. the current segment is on the same layer as the top or bottom of the pseudo-via.
          //
          // Also set the flag 'mark_segment_for_deletion' so that all segments will be marked for
          // deletion between the current segment and the the diff-pair via:
          if (   (segment_to_pseudoVia_distance < deletion_radius)                                                         // Item (a) above
              && (   (pathCoords[diffPairPathNum[i]][path_segment].Z == pseudoCoordsBeforeVia.Z)                           // Item (b) above
                  || (pathCoords[diffPairPathNum[i]][path_segment].Z == pathCoords[pseudoPathNum][pseudoPathSegment].Z)))  // Item (b) above
          {

            // Set bit #0 of the deleteSegment element by OR'ing it with '1':
            deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 1;

            #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Marking segment %d for deletion from path %d because its position (%d,%d,%d) is %5.2f cells from pseudo-via at (%d,%d).\n",
                     omp_get_thread_num(), path_segment, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                     pathCoords[diffPairPathNum[i]][path_segment].Z, segment_to_pseudoVia_distance,
                     pseudoViaX, pseudoViaY);
              printf("DEBUG: (thread %2d)     Threshold is %5.2f = max_cong_radius (%d) + Rdpv (%5.2f) x cosine(theta) (%5.2f).\n", omp_get_thread_num(),
                     deletion_radius, max_cong_radius, radius_diffPairvias, abs_cosine_theta);
            }
            #endif

            // Set 'mark_segment_for_deletion' flag to TRUE so that all subsequent segments
            // will be marked for deletion:
            mark_segment_for_deletion = TRUE;
          }  // End of if-block for distance being less than 'deletion_radius'

        }  // End of for-loop for marking segments starting from the end-terminal

      }  // End of for-loop for index 'i' (from 0 to 1)

    }  // End of if-block for reaching end of a via-stack

    else  {
      // The current pseudo-path segment is not at the end of a via-stack. If the current
      // segment is on same layer as previous segment, then record this segment number
      // in 'pseudoSegmentBeforeVia', which we'll later use when we indeed reach the
      // end of a via-stack:
      if (pathCoords[pseudoPathNum][pseudoPathSegment].Z == prevPseudoSegmentCoords.Z)  {
        pseudoSegmentBeforeVia = pseudoPathSegment;
        pseudoCoordsBeforeVia = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);

        #ifdef DEBUG_markDiffPairSegmentsNearPseudoVia
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Setting pseudoSegmentBeforeVia to %d.\n", omp_get_thread_num(), pseudoSegmentBeforeVia);
        }
        #endif
      }
    }  // End of else-block (segment is not at end of via-stack)

    // In preparation for the next run through this loop, copy the current segment's
    // coordinates into the 'prevPseudoSegmentCoords' variable:
    prevPseudoSegmentCoords = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);

  }  // End of for-loop for index 'pseudoPathSegment'

}  // End of function 'markDiffPairSegmentsNearPseudoVia'


//-----------------------------------------------------------------------------
// Name: getLastCoordinatesBeforeExitingSwapZone
// Desc: Return the coordinates of the last segment before path 'pathNum'
//       exits a pin-swap zone. If the start-terminal of the path is not
//       in a swap-zone, then this function returns the start-terminals.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_getLastCoordinatesBeforeExitingSwapZone' and re-compile if you want
// verbose debugging print-statements enabled:
//
// #define DEBUG_getLastCoordinatesBeforeExitingSwapZone
#undef DEBUG_getLastCoordinatesBeforeExitingSwapZone

static Coordinate_t getLastCoordinatesBeforeExitingSwapZone(const int pathNum, MapInfo_t *mapInfo,
                                                            Coordinate_t *pathCoords[],
                                                            int pathLength[], CellInfo_t ***cellInfo)  {

  // Initialize the value to be returned as the start-terminal:
  Coordinate_t lastCoordsInSwapZone = copyCoordinates(mapInfo->start_cells[pathNum]);

  #ifdef DEBUG_getLastCoordinatesBeforeExitingSwapZone
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (mapInfo->current_iteration >= 1)  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in getLastCoordinatesBeforeExitingSwapZone() because specific requirements were met.\n\n", omp_get_thread_num());
    printf("DEBUG: (thread %2d) Entered function 'getLastCoordinatesBeforeExitingSwapZone' with path %d (length %d) with start-coords (%d,%d,%d) in swap-zone %d\n",
           omp_get_thread_num(), pathNum, pathLength[pathNum], lastCoordsInSwapZone.X, lastCoordsInSwapZone.Y, lastCoordsInSwapZone.Z,
           cellInfo[lastCoordsInSwapZone.X][lastCoordsInSwapZone.Y][lastCoordsInSwapZone.Z].swap_zone);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Confirm that the start-terminal is indeed in a pin-swap zone. If not, then return
  // the start-terminal's coordinates.
  if (! cellInfo[lastCoordsInSwapZone.X][lastCoordsInSwapZone.Y][lastCoordsInSwapZone.Z].swap_zone)  {
    return(lastCoordsInSwapZone);  // Return the coordinates of the starting terminal
  }  // End of if-block for fatal error condition

  #ifdef DEBUG_getLastCoordinatesBeforeExitingSwapZone
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Path #%d's start-terminal (%d,%d,%d) is in swap-zone %d, so we'll re-define the pseudo-start-terminal's location.\n",
           omp_get_thread_num(), pathNum, lastCoordsInSwapZone.X, lastCoordsInSwapZone.Y, lastCoordsInSwapZone.Z,
           cellInfo[lastCoordsInSwapZone.X][lastCoordsInSwapZone.Y][lastCoordsInSwapZone.Z].swap_zone);
  }
  #endif

  // Iterate over the length of the path, starting from its beginning:
  for (int segment = 0; segment < pathLength[pathNum]; segment++)  {

    #ifdef DEBUG_getLastCoordinatesBeforeExitingSwapZone
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d)   Checking path segment #%d at (%d,%d,%d) in swap-zone = %d.\n", omp_get_thread_num(), segment,
             pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z,
             cellInfo[pathCoords[pathNum][segment].X][pathCoords[pathNum][segment].Y][pathCoords[pathNum][segment].Z].swap_zone);
    }
    #endif

    // Check whether the path's current segment is in a swap-zone:
    if (cellInfo[pathCoords[pathNum][segment].X][pathCoords[pathNum][segment].Y][pathCoords[pathNum][segment].Z].swap_zone)  {
      // Moving forward along the path, we found a path segment that's in
      // a swap-zone. So re-define the start-terminal as this segment
      // before the path exits the swap-zone:
      lastCoordsInSwapZone = copyCoordinates(pathCoords[pathNum][segment]);

    }  // End of if-block for segment being in a swap-zone
    else  {
      // We got here, so the current segment is NOT in a swap-zone. We therefore break
      // out of the for-loop. The current value of 'lastCoordsInSwapZone' contains the
      // coordinates of the last segment that was in a swap-zone.

      #ifdef DEBUG_getLastCoordinatesBeforeExitingSwapZone
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) For pseudo-path %d, the start-terminal was re-defined in getLastCoordinatesBeforeExitingSwapZone to (%d,%d,%d)\n",
               omp_get_thread_num(), pathNum, lastCoordsInSwapZone.X, lastCoordsInSwapZone.Y, lastCoordsInSwapZone.Z);
      }
      #endif

      break;  // Break out of the for-loop with index 'segment'

    }  // End of else-block for segment being in a swap-zone
  }  // End of for-loop for index 'segment'

  return(lastCoordsInSwapZone);

}  // End of function 'getLastCoordinatesBeforeExitingSwapZone'


//-----------------------------------------------------------------------------
// Name: getFirstNonSwapZoneCoordinates
// Desc: Return the coordinates of the first segment after path 'pathNum' exits
//       a pin-swap zone. If the start-terminal of the path is not in a
//       pin-swap zone, then the start-terminal's coordinates are returned.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_getFirstNonSwapZoneCoordinates' and re-compile if you want
// verbose debugging print-statements enabled:
//
// #define DEBUG_getFirstNonSwapZoneCoordinates
#undef DEBUG_getFirstNonSwapZoneCoordinates

static Coordinate_t getFirstNonSwapZoneCoordinates(const int pathNum, MapInfo_t *mapInfo,
                                                   Coordinate_t *pathCoords[],
                                                   int pathLength[], CellInfo_t ***cellInfo)  {

  // Initialize the value to be returned as the start-terminal:
  Coordinate_t firstNonSwapZoneCoords = copyCoordinates(mapInfo->start_cells[pathNum]);

  #ifdef DEBUG_getFirstNonSwapZoneCoordinates
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (mapInfo->current_iteration >= 1)  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in getFirstNonSwapZoneCoordinates() because specific requirements were met.\n\n", omp_get_thread_num());
    printf("DEBUG: (thread %2d) Entered function 'getFirstNonSwapZoneCoordinates' with path %d (length %d) with start-coords (%d,%d,%d) in swap-zone %d\n",
           omp_get_thread_num(), pathNum, pathLength[pathNum], firstNonSwapZoneCoords.X, firstNonSwapZoneCoords.Y, firstNonSwapZoneCoords.Z,
           cellInfo[firstNonSwapZoneCoords.X][firstNonSwapZoneCoords.Y][firstNonSwapZoneCoords.Z].swap_zone);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // If the start-coordinates are not in a swap-zone, then immediately return these coordinates
  // to the calling program:
  if (! cellInfo[firstNonSwapZoneCoords.X][firstNonSwapZoneCoords.Y][firstNonSwapZoneCoords.Z].swap_zone)  {

    #ifdef DEBUG_getFirstNonSwapZoneCoordinates
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) For pseudo-path %d, the start-terminal at (%d,%d,%d) was not in a swap-zone in getFirstNonSwapZoneCoordinates.\n",
             omp_get_thread_num(), pathNum, firstNonSwapZoneCoords.X, firstNonSwapZoneCoords.Y, firstNonSwapZoneCoords.Z);
    }
    #endif

    return(firstNonSwapZoneCoords);
  }  // End of if-block for start-terminal NOT being in a swap-zone


  // Iterate over the length of the path, starting from its beginning:
  for (int segment = 0; segment < pathLength[pathNum]; segment++)  {

    #ifdef DEBUG_getFirstNonSwapZoneCoordinates
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d)   Checking path segment #%d at (%d,%d,%d) in swap-zone = %d.\n", omp_get_thread_num(), segment,
             pathCoords[pathNum][segment].X, pathCoords[pathNum][segment].Y, pathCoords[pathNum][segment].Z,
             cellInfo[pathCoords[pathNum][segment].X][pathCoords[pathNum][segment].Y][pathCoords[pathNum][segment].Z].swap_zone);
    }
    #endif

    // Check whether the path's current segment is in a swap-zone:
    if (! cellInfo[pathCoords[pathNum][segment].X][pathCoords[pathNum][segment].Y][pathCoords[pathNum][segment].Z].swap_zone)  {
      // Moving forward along the path, we found a path segment that's NOT in
      // a swap-zone. So capture these coordinates as the first coordinates
      // that are not in a swap-zone:
      firstNonSwapZoneCoords = copyCoordinates(pathCoords[pathNum][segment]);

      #ifdef DEBUG_getFirstNonSwapZoneCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) For pseudo-path %d, the start-terminal was re-defined in getFirstNonSwapZoneCoordinates to (%d,%d,%d)\n",
               omp_get_thread_num(), pathNum, firstNonSwapZoneCoords.X, firstNonSwapZoneCoords.Y, firstNonSwapZoneCoords.Z);
      }
      #endif

      // ... and then break out of the for-loop for index 'segment':
      break;

    }  // End of if-block for segment NOT being in a swap-zone
  }  // End of for-loop for index 'segment'

  // Return the coordinates to the calling program:
  return(firstNonSwapZoneCoords);

}  // End of function 'getFirstNonSwapZoneCoordinates'


//-----------------------------------------------------------------------------
// Name: markDiffPairSegmentsNearTerminals
// Desc: Mark for deletion all segments of the two diff-pair paths associated
//       with pseudo-path 'pseudoPathNum' that are near the terminals of the
//       corresponding pseudo-path 'pseudoPathNum'. Diff-pair segments are
//       deleted if they are within the following distance from the pseudo-via:
//
//            cong_radius[i][m][j][n]   +   2 x Rdp  x |COS(theta)|,
//
//       where Rdp is the maximum of:
//           a. half the distance between the diff-pair terminals, and
//           b. half the diff-pair (trace) pitch at the terminal.
//       Theta is the angle between the line through the diff-pair terminals
//       and a line from the pseudo-terminal to the path-segment that could be
//       deleted. As a reminder, the cong_radius values are equal to:
//             cong_radius[m][n] = radius[n] + spacing[m][n] + radius[m]
//
//       The indices for cong_radius are:
//        i = design-rule set at pseudo-via cell (Note: we assume that the design-rule set
//            at the pseudo-via location is the same set as at the diff-pair via location)
//        m = design-rule subset and shape-type of a diff-pair path in design-rule set i
//        j = design-rule set at the diff-pair segment that we’re considering deleting
//        n = design-rule subset and shape-type of the diff-pair path that we’re
//            considering deleting in design-rule zone j.
//
//       If *any* segment in a vertically aligned via-stack is marked for deletion by
//       this function, then it marks *all* segments in that via-stack for deletion.
//
//       This function modifies the 2-dimensional array 'deleteSegment' by setting bit
//       #2 of the element associated with the segment to be deleted due to proximity
//       to a pseudo-via, and setting bit #3 for vertically aligned via-segments
//       to be deleted because they're directly above/below a segment to be
//       deleted due to proximity to a pseudo-via.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_markDiffPairSegmentsNearTerminals' and re-compile if you want
// verbose debugging print-statements enabled:
//
// #define DEBUG_markDiffPairSegmentsNearTerminals
#undef DEBUG_markDiffPairSegmentsNearTerminals

static void markDiffPairSegmentsNearTerminals(const int pseudoPathNum, const int path_1_number,
                                              const int path_2_number, Coordinate_t *pathCoords[],
                                              int pathLength[], InputValues_t *user_inputs,
                                              CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                                              unsigned char ** deleteSegment)  {

  // Get the numbers of the two diff-pair paths associated with this pseudo-net:
  int diffPairPathNum[2];
  diffPairPathNum[0] = path_1_number;
  diffPairPathNum[1] = path_2_number;

  #ifdef DEBUG_markDiffPairSegmentsNearTerminals
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (   (mapInfo->current_iteration >= 248) && (mapInfo->current_iteration <= 250)
      && (pseudoPathNum == 35))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in markDiffPairSegmentsNearTerminals() because specific requirements were met.\n\n", omp_get_thread_num());
    printf("DEBUG: (thread %2d) Entered function 'markDiffPairSegmentsNearTerminals' with pseudo-path %d (length %d), diff-pair path %d (length %d) and path %d (length %d)\n",
           omp_get_thread_num(), pseudoPathNum, pathLength[pseudoPathNum], diffPairPathNum[0], pathLength[diffPairPathNum[0]], diffPairPathNum[1], pathLength[diffPairPathNum[1]]);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Get the coordinates of the start- and end-terminals of the associated pseudo-net:
  Coordinate_t pseudoStartTerm = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);  // Get starting coordinate of pseudo-net
  Coordinate_t pseudoEndTerm   = copyCoordinates(mapInfo->end_cells[pseudoPathNum]);    // Get ending coordinate of pseudo-net


  // Check whether the pseudo-net's start-terminal is located in a swap-zone. If so, then
  // re-define the location of the pseudo-start-terminal as the last segment before the
  // path exited the swap-zone
  if (cellInfo[pseudoStartTerm.X][pseudoStartTerm.Y][pseudoStartTerm.Z].swap_zone)  {

    // Call function that gets the last coordinate before path exits a swap-zone:
    pseudoStartTerm = getLastCoordinatesBeforeExitingSwapZone(pseudoPathNum, mapInfo, pathCoords, pathLength, cellInfo);

    #ifdef DEBUG_markDiffPairSegmentsNearTerminals
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Pseudo-path #%d's start-terminal (%d,%d,%d) was in swap-zone %d, so we re-defined the pseudo-start-terminal's location to (%d,%d,%d),\n",
             omp_get_thread_num(), pseudoPathNum, mapInfo->start_cells[pseudoPathNum].X, mapInfo->start_cells[pseudoPathNum].Y, mapInfo->start_cells[pseudoPathNum].Z,
             cellInfo[mapInfo->start_cells[pseudoPathNum].X][mapInfo->start_cells[pseudoPathNum].Y][mapInfo->start_cells[pseudoPathNum].Z].swap_zone,
             pseudoStartTerm.X, pseudoStartTerm.Y, pseudoStartTerm.Z);
      printf("DEBUG: (thread %2d) which is the last segment before the path exits the swap-zone.\n", omp_get_thread_num());
    }
    #endif
  }  // End of if-block for start-pseudo-terminal being in a swap-zone

  //
  // Calculate a unit-vector between the two start-terminals of the diff-pairs. Note that these
  // terminals might be in a swap-zone, as defined in function 'update_swapZone_startTerms()'
  // at the end of the previous iteration.
  //
  Vector2dFloat_t start_term_unit_vector;     // Variable to hold X- and Y-coordinates of unit-vector.
  char start_term_unit_vector_exists = FALSE; // Boolean flag to indicate whether the unit-vector was successfully calculated
  if (   (mapInfo->start_cells[path_1_number].X != mapInfo->start_cells[path_2_number].X)
      || (mapInfo->start_cells[path_1_number].Y != mapInfo->start_cells[path_2_number].Y))  {

    start_term_unit_vector = calc_2D_unit_vector_ints(mapInfo->start_cells[path_1_number], mapInfo->start_cells[path_2_number]);
    start_term_unit_vector_exists = TRUE;
  }

  //
  // Calculate a unit-vector between the two end-terminals of the diff-pairs:
  //
  Vector2dFloat_t end_term_unit_vector;  // Variable to hold X- and Y-coordinates of unit-vector.
  char end_term_unit_vector_exists = FALSE; // Boolean flag to indicate whether the unit-vector was successfully calculated
  if (   (mapInfo->end_cells[path_1_number].X != mapInfo->end_cells[path_2_number].X)
      || (mapInfo->end_cells[path_1_number].Y != mapInfo->end_cells[path_2_number].Y))  {

    end_term_unit_vector = calc_2D_unit_vector_ints(mapInfo->end_cells[path_1_number], mapInfo->end_cells[path_2_number]);
    end_term_unit_vector_exists = TRUE;
  }

  //
  // Calculate the radius to be used around the start-terminal:
  //
  float start_radius;
  {  // Start of block to calculate 'start_radius'

    // Get the design-rule number at the start-terminal of the pseudo-path, or the first segment of this
    // pseudo-path that is not in a pin-swap zone:
    Coordinate_t nonSwapZone_pseudoStartTerm = getFirstNonSwapZoneCoordinates(pseudoPathNum, mapInfo, pathCoords, pathLength, cellInfo);

    // Get the design-rule number at the location of the pseudo start-terminal:
    int DR_num = cellInfo[nonSwapZone_pseudoStartTerm.X][nonSwapZone_pseudoStartTerm.Y][nonSwapZone_pseudoStartTerm.Z].designRuleSet;

    // Get the design-rule subset associated with the pseudo-path:
    int DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num];

    // Get the diff-pair pitch from the appropriate design-rule set:
    float diff_pair_halfPitch = 0.5 * user_inputs->designRules[DR_num][DR_subset].traceDiffPairPitchMicrons / user_inputs->cell_size_um;

    // Calculate the radius (in cell-units) associated with the start-terminals of the two diff-pair nets. It's possible
    // that the two start-terminals are in a pin-swap zone, in which case the two start-terminals might be separated
    // by hundreds of cells, or even zero cells. So for start-terminals in swap-zones, we simply use half the diff-pair
    // pitch for the 'start_term_radius' value:
    if (mapInfo->swapZone[pseudoPathNum])  {
      // We got here, so the start-terminals are located in a swap-zone. So we use half the diff-pair pitch
      // as a dependable and reproducible radius for the start-terminals. Add 1 cell to accommodate
      // rounding errors:
      start_radius = diff_pair_halfPitch + 1.0;

      #ifdef DEBUG_markDiffPairSegmentsNearTerminals
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)   The radius at the start-terminal is %.2f, which is the diff-pair half-pitch (%.2f) plus 1 cell because start-terms are in swap-zone\n",
               omp_get_thread_num(), start_radius, diff_pair_halfPitch);
      }
      #endif
    }  // End of if-block for start-terminals being located in a swap-zone
    else  {
      // We got here, so the start-terminals are NOT located in a swap-zone. So we use half the maximum of
      // the diff-pair pitch and the inter-terminal distance as the radius for the start-terminals:

      // Calculate the inter-terminal distance:
      float start_term_radius = 0.5 * calc_2D_Pythagorean_distance_ints(mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y,
                                                                        mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y);

      // For the start-terminals, assign the diff-pair radius as the maximum of half the diff-pair pitch and half the
      // distance between start-terminals. Add 1 cell to accommodate rounding errors:
      start_radius = max(start_term_radius, diff_pair_halfPitch) + 1.0;

      #ifdef DEBUG_markDiffPairSegmentsNearTerminals
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)   The radius at the start-terminal is %.2f, which is the maximum of the diff-pair half-pitch (%.2f) and the terminal half-pitch (%.2f), plus 1 cell\n",
               omp_get_thread_num(), start_radius, diff_pair_halfPitch, start_term_radius);
      }
      #endif
    }  // End of else-block for start-terminals NOT in a swap-zone

  }  // End of block to calculate 'start_radius'

  //
  // Calculate the radius to be used around the end-terminal:
  //
  float end_radius;
  {  // Start of block to calculate 'end_radius'

    // Calculate the radius (in cell-units) associated with the end-terminals of the two diff-pair nets:
    float end_term_radius = 0.5 * calc_2D_Pythagorean_distance_ints(mapInfo->end_cells[path_1_number].X, mapInfo->end_cells[path_1_number].Y,
                                                                    mapInfo->end_cells[path_2_number].X, mapInfo->end_cells[path_2_number].Y);

    // Get the design-rule number at the location of the pseudo end-terminal:
    int DR_num = cellInfo[pseudoEndTerm.X][pseudoEndTerm.Y][pseudoEndTerm.Z].designRuleSet;

    // Get the design-rule subset associated with the pseudo-path:
    int DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num];

    // Get the diff-pair pitch from the appropriate design-rule set:
    float end_diffPair_radius_cells = 0.5 * user_inputs->designRules[DR_num][DR_subset].traceDiffPairPitchMicrons / user_inputs->cell_size_um;

    // For the end-terminals, assign the diff-pair radius as the maximum of half the diff-pair pitch and half the
    // distance between end-terminals. Add 1 cell to accommodate rounding errors:
    end_radius = max(end_term_radius, end_diffPair_radius_cells) + 1.0;

    #ifdef DEBUG_markDiffPairSegmentsNearTerminals
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d)   The radius at the end-terminal is %.2f, which is the maximum of the diff-pair half-pitch (%.2f) and the terminal half-pitch (%.2f), plus 1 cell\n",
             omp_get_thread_num(), end_radius, end_diffPair_radius_cells, end_term_radius);
    }
    #endif

  }  // End of block to calculate 'end_radius'

  #ifdef DEBUG_markDiffPairSegmentsNearTerminals
  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) When analyzing terminals for pseudo-path %d in markDiffPairSegmentsNearTerminals:\n", omp_get_thread_num(), pseudoPathNum);
    printf("DEBUG: (thread %2d)       Diff-pair start-terminals: (%d,%d,%d) and (%d,%d,%d)\n", omp_get_thread_num(),
            mapInfo->start_cells[path_1_number].X, mapInfo->start_cells[path_1_number].Y, mapInfo->start_cells[path_1_number].Z,
            mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y, mapInfo->start_cells[path_2_number].Z);
    printf("DEBUG: (thread %2d)       Diff-pair start-term unit-vector is (%5.3f, %5.3f). Radius is %6.3f cells.\n",
            omp_get_thread_num(), start_term_unit_vector.X, start_term_unit_vector.Y, start_radius);
    printf("DEBUG: (thread %2d)       Diff-pair end-terminals: (%d,%d,%d) and (%d,%d,%d)\n", omp_get_thread_num(),
            mapInfo->end_cells[path_1_number].X, mapInfo->end_cells[path_1_number].Y, mapInfo->end_cells[path_1_number].Z,
            mapInfo->end_cells[path_2_number].X, mapInfo->end_cells[path_2_number].Y, mapInfo->end_cells[path_2_number].Z);
    printf("DEBUG: (thread %2d)       Diff-pair end-term unit-vector is (%5.3f, %5.3f). Radius is %6.3f cells\n\n",
            omp_get_thread_num(), end_term_unit_vector.X, end_term_unit_vector.Y, end_radius);
  }
  #endif


  // Define the attributes of the start- and end-terminals using a ViaStack_t structure. If the
  // terminal is not part of a via, then the startSegment and endSegment elements will be identical,
  // and the endShapeType element will be 'TRACE' (i.e., zero).
  ViaStack_t startTermVia, endTermVia;

  // First, define attributes of start-terminal 'via':
  startTermVia.error = FALSE;
  startTermVia.pathNum = pseudoPathNum;
  startTermVia.startSegment = -1;    // '-1' refers to the start-segment.
  startTermVia.startCoord = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);
  startTermVia.endSegment = -1;      // Initialized to -1, but can be changed in loop below.
  startTermVia.endCoord   = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);
  startTermVia.endShapeType = TRACE; // Initialized to TRACE, but can be changed in loop below.
  // Iterate over the beginning of the path to find the 'endSegment' of the start-terminal
  // 'via':
  int prev_layer = pseudoStartTerm.Z;
  for (int pseudoPathSegment = 0; pseudoPathSegment < pathLength[pseudoPathNum]; pseudoPathSegment++)  {
    // Check if current segment is on same layer as previous segment.
    if (pathCoords[pseudoPathNum][pseudoPathSegment].Z == prev_layer)  {
      // We got here, so the current segment is on the same layer as the previous segment.
      // Break out of the for-loop. The 'endSegment' element is now correct.
      break;
    }
    else  {
      // We got here, so current segment is on a different layer than the previous segment. Update
      // the 'endShapeType' and 'endSegment' elements of the via-stack. Then update the
      // 'prev_layer' variable and move on to the next segment in the pseudo-path.
      startTermVia.endSegment = pseudoPathSegment;
      startTermVia.endCoord   = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);
      if (pathCoords[pseudoPathNum][pseudoPathSegment].Z > prev_layer)  {
        startTermVia.endShapeType = VIA_DOWN;
      }
      else  {
        startTermVia.endShapeType = VIA_UP;
      }
      prev_layer = pathCoords[pseudoPathNum][pseudoPathSegment].Z;
    }  // End of if/else-block
  }  // End of for-loop for index 'pseudoPathSegment'


  // Second, define attributes of end-terminal 'via':
  endTermVia.error = FALSE;
  endTermVia.pathNum = pseudoPathNum;
  endTermVia.endSegment = pathLength[pseudoPathNum] - 1;    // Last segment in pseudo-path.
  endTermVia.endCoord   = copyCoordinates(pathCoords[pseudoPathNum][pathLength[pseudoPathNum] - 1]);
  endTermVia.startSegment = pathLength[pseudoPathNum] - 1;  // Last segment in pseudo-path, but can be changed later.
  endTermVia.startCoord   = copyCoordinates(pathCoords[pseudoPathNum][pathLength[pseudoPathNum] - 1]);

  // Determine the shape-type of the last segment in the pseudo-path at element "pathLength[pseudoPathNum] - 1":
  if (pathCoords[pseudoPathNum][pathLength[pseudoPathNum] - 1].Z == pathCoords[pseudoPathNum][pathLength[pseudoPathNum] - 2].Z)  {
    endTermVia.endShapeType = TRACE;  // Last 2 segments are on same layer, making end-terminal a TRACE
  }
  else if (pathCoords[pseudoPathNum][pathLength[pseudoPathNum] - 1].Z > pathCoords[pseudoPathNum][pathLength[pseudoPathNum] - 2].Z)  {
    endTermVia.endShapeType = VIA_DOWN;  // Last segment is on higher layer than 2nd-to-last segment, making end-terminal a VIA_DOWN
  }
  else  {
    endTermVia.endShapeType = VIA_UP;  // Last segment is on lower layer than 2nd-to-last segment, making end-terminal a VIA_UP
  }

  // Iterate over the pseudo-path starting from the path's end to find the 'startSegment' of the
  // end-terminal's 'via':
  prev_layer = pseudoEndTerm.Z;
  for (int pseudoPathSegment = pathLength[pseudoPathNum] - 1; pseudoPathSegment >= 0; pseudoPathSegment--)  {
    // Check if current segment is on same layer as previous segment.
    if (pathCoords[pseudoPathNum][pseudoPathSegment].Z == prev_layer)  {
      // We got here, so the current segment is on the same layer as the previous segment.
      // Break out of the for-loop. The 'startSegment' element is now correct.
      break;
    }
    else  {
      // We got here, so current segment is on a different layer than the previous segment. Update
      // the 'endShapeType' and 'endSegment' elements of the via-stack. Then update the
      // 'prev_layer' variable and move on to the next segment in the pseudo-path.
      endTermVia.startSegment = pseudoPathSegment;
      endTermVia.startCoord   = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);
      prev_layer = pathCoords[pseudoPathNum][pseudoPathSegment].Z;
    }  // End of if/else-block
  }  // End of for-loop for index 'pseudoPathSegment'


  //
  // Iterate over both diff-pair paths 'i' associated with pseudo-path 'pseudoPathNum':
  //
  for (int i = 0; i < 2; i++)  {

    //
    // Step #1: Analyze segments relative to the start-terminal. Mark segments for deletion, starting
    // from the end of the shoulder-path and progressing backwards towards the start-terminal:
    //
    int mark_segment_for_deletion = FALSE;
    float deletion_radius = 0.0;
    for (int path_segment = pathLength[diffPairPathNum[i]] - 1; path_segment >= 0; path_segment--)  {

      // If 'mark_segment_for_deletion' is already TRUE and the current segment is on the same
      // routing layer as the start-terminal or, if there's a via at the start-terminal, the
      // same layer where the via ends, then simply mark this segment for deletion and
      // move on to the next segment:
      if (   (mark_segment_for_deletion)
          && (   (pathCoords[diffPairPathNum[i]][path_segment].Z == startTermVia.startCoord.Z)
              || (pathCoords[diffPairPathNum[i]][path_segment].Z == startTermVia.endCoord.Z)))  {

        // Set bit #2 of the deleteSegment element by OR'ing it with '4':
        deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 4;

        #ifdef DEBUG_markDiffPairSegmentsNearTerminals
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Marking segment %d (%d,%d,%d) for deletion from path %d because a segment farther from the start-terminal was closer than %5.2f cells to the pseudo-path's start-terminal at (%d,%d)\n",
                  omp_get_thread_num(), path_segment, pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                  pathCoords[diffPairPathNum[i]][path_segment].Z, diffPairPathNum[i], deletion_radius, pseudoStartTerm.X, pseudoStartTerm.Y);
        }
        #endif

        // Move on to next segment in diff-pair path
        continue;
      }  // End of if-block for (mark_segment_for_deletion == TRUE)


      // Calculate the maximum congestion radius (cong_radius) between the current segment and any
      // layer in the pseudo-path's start-terminal:
      #ifdef DEBUG_markDiffPairSegmentsNearTerminals
      if (DEBUG_ON)  {
        printf("DEBUG: Calling 'getMaxCongRadiusToSegment' from markDiffPairSegmentsNearTerminals...\n");
      }
      #endif
      int max_cong_radius = getMaxCongRadiusToSegment(startTermVia, diffPairPathNum[i], path_segment, pathCoords, pathLength, user_inputs, cellInfo, mapInfo);


      // Calculate the absolute value of the cosine of the angle between the line through the diff-pair
      // start-terminals ('start_term_unit_vector') and a line from the pseudo-terminal to the path-segment.
      // If the diff-pair terminals occupy the same (x,y) locations, then set the cosine equal to unity:
      float abs_cosine_theta = 1.0;
      if (start_term_unit_vector_exists)  {
        abs_cosine_theta = calcAbsCosine(start_term_unit_vector, pseudoStartTerm, pathCoords[diffPairPathNum[i]][path_segment]);

        // Calculate a 'deletion radius' about the pseudo-terminal, within which all diff-pair segments
        // should be marked for deletion:
        deletion_radius = max_cong_radius   +   2 * start_radius * abs_cosine_theta;
      }
      else  {
        deletion_radius = 2.0 * max_cong_radius;
      }



      // Calculate the distance between the current diff-pair path segment and the pseudo-path's start-terminal:
      float segment_to_pseudoTerm_distance = calc_2D_Pythagorean_distance_ints(pathCoords[diffPairPathNum[i]][path_segment].X,
                                                                               pathCoords[diffPairPathNum[i]][path_segment].Y,
                                                                               pseudoStartTerm.X, pseudoStartTerm.Y);

      // If the distance between the pseudo-terminal and the current diff-pair segment is less
      // than the deletion_radius, then mark this segment for deletion. Also set the flag
      // 'mark_segment_for_deletion' so that all segments will be marked for deletion between
      // the current segment and the diff-pair's terminal
      if (segment_to_pseudoTerm_distance < deletion_radius)  {

        // Set bit #2 of the deleteSegment element by OR'ing it with '4':
        deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 4;

        #ifdef DEBUG_markDiffPairSegmentsNearTerminals
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Marking segment %d for deletion from path %d because its position (%d,%d,%d) is %5.2f cells from pseudo-start-terminal at (%d,%d).\n",
                 omp_get_thread_num(), path_segment, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                 pathCoords[diffPairPathNum[i]][path_segment].Z, segment_to_pseudoTerm_distance,
                 pseudoStartTerm.X, pseudoStartTerm.Y);
          printf("DEBUG: (thread %2d)     Threshold is %5.2f = max_cong_radius (%d) + 2 x Rdpt (%5.2f) x cosine(theta) (%5.2f).\n", omp_get_thread_num(),
                 deletion_radius, max_cong_radius, start_radius, abs_cosine_theta);
        }
        #endif

        // Set 'mark_segment_for_deletion' flag to TRUE so that all subsequent segments
        // will be marked for deletion:
        mark_segment_for_deletion = TRUE;
      }  // End of if-block for distance being less than 'deletion_radius'
      #ifdef DEBUG_markDiffPairSegmentsNearTerminals
      else  {
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) NOT marking segment %d for deletion from path %d because its position (%d,%d,%d) is %5.2f cells from pseudo-start-terminal at (%d,%d).\n",
                 omp_get_thread_num(), path_segment, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][path_segment].X,
                 pathCoords[diffPairPathNum[i]][path_segment].Y, pathCoords[diffPairPathNum[i]][path_segment].Z,
                 segment_to_pseudoTerm_distance, pseudoStartTerm.X, pseudoStartTerm.Y);
        }
      }
      #endif

    }  // End of for-loop for marking segments near start-terminal, starting from the end-terminal


    //
    // Step #2: Analyze the end-terminal. Mark segments for deletion, starting from the
    // start of the shoulder-path and progressing forwards towards the end-terminal:
    //
    mark_segment_for_deletion = FALSE;
    deletion_radius = 0.0;
    for (int path_segment = 0; path_segment < pathLength[diffPairPathNum[i]]; path_segment++)  {

      // If 'mark_segment_for_deletion' is already TRUE and the current segment is on the same
      // routing layer as the end-terminal or, if there's a via at the end-terminal, the
      // same layer where the via ends, then simply mark this segment for deletion and
      // move on to the next segment:
      if (   (mark_segment_for_deletion)
          && (   (pathCoords[diffPairPathNum[i]][path_segment].Z == endTermVia.startCoord.Z)
              || (pathCoords[diffPairPathNum[i]][path_segment].Z == endTermVia.endCoord.Z)))  {

        // Set bit #2 of the deleteSegment element by OR'ing it with '4':
        deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 4;

        #ifdef DEBUG_markDiffPairSegmentsNearTerminals
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Marking segment %d (%d,%d,%d) for deletion from path %d because a segment farther from the end-terminal was closer than %5.2f cells to the pseudo-path's end-terminal at (%d,%d)\n",
                  omp_get_thread_num(), path_segment, pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                  pathCoords[diffPairPathNum[i]][path_segment].Z, diffPairPathNum[i], deletion_radius, pseudoEndTerm.X, pseudoEndTerm.Y);
        }
        #endif

        // Move on to next segment in diff-pair path
        continue;
      }  // End of if-block for (mark_segment_for_deletion == TRUE)


      // Calculate the maximum congestion radius (cong_radius) between the current segment and any
      // layer in the pseudo-path's end-terminal:
      #ifdef DEBUG_markDiffPairSegmentsNearTerminals
      if (DEBUG_ON)  {
        printf("DEBUG: Calling 'getMaxCongRadiusToSegment' from markDiffPairSegmentsNearTerminals...\n");
      }
      #endif
      int max_cong_radius = getMaxCongRadiusToSegment(endTermVia, diffPairPathNum[i], path_segment, pathCoords, pathLength, user_inputs, cellInfo, mapInfo);


      // Calculate the absolute value of the cosine of the angle between the line through the diff-pair
      // end-terminals ('end_term_unit_vector') and a line from the pseudo-terminal to the path-segment:
      float abs_cosine_theta = 0.0;
      if (end_term_unit_vector_exists)  {
        abs_cosine_theta = calcAbsCosine(end_term_unit_vector, pseudoEndTerm, pathCoords[diffPairPathNum[i]][path_segment]);
      }

      // Calculate a 'deletion radius' about the pseudo-terminal, within which all diff-pair segments
      // should be marked for deletion:
      deletion_radius = max_cong_radius   +   2 * end_radius * abs_cosine_theta;

      // Calculate the distance between the current diff-pair path segment and the pseudo-path's end-terminal:
      float segment_to_pseudoTerm_distance = calc_2D_Pythagorean_distance_ints(pathCoords[diffPairPathNum[i]][path_segment].X,
                                                                               pathCoords[diffPairPathNum[i]][path_segment].Y,
                                                                               pseudoEndTerm.X, pseudoEndTerm.Y);

      // If the distance between the pseudo-terminal and the current diff-pair segment is less
      // than the deletion_radius, then mark this segment for deletion. Also set the flag
      // 'mark_segment_for_deletion' so that all segments will be marked for deletion between
      // the current segment and the diff-pair's terminal
      if (segment_to_pseudoTerm_distance < deletion_radius)  {

        // Set bit #2 of the deleteSegment element by OR'ing it with '4':
        deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 4;

        #ifdef DEBUG_markDiffPairSegmentsNearTerminals
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Marking segment %d for deletion from path %d because its position (%d,%d,%d) is %5.2f cells from pseudo-end-terminal at (%d,%d).\n",
                 omp_get_thread_num(), path_segment, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][path_segment].X,
                 pathCoords[diffPairPathNum[i]][path_segment].Y, pathCoords[diffPairPathNum[i]][path_segment].Z,
                 segment_to_pseudoTerm_distance, pseudoEndTerm.X, pseudoEndTerm.Y);
          printf("DEBUG: (thread %2d)     Threshold is %5.2f = max_cong_radius (%d) + 2 x Rdpt (%5.2f) x cosine(theta) (%5.2f).\n", omp_get_thread_num(),
                 deletion_radius, max_cong_radius, end_radius, abs_cosine_theta);
        }
        #endif

        // Set 'mark_segment_for_deletion' flag to TRUE so that all subsequent segments
        // will be marked for deletion:
        mark_segment_for_deletion = TRUE;
      }  // End of if-block for distance being less than 'deletion_radius'
      #ifdef DEBUG_markDiffPairSegmentsNearTerminals
      else  {
        if (DEBUG_ON)  {
         printf("DEBUG: (thread %2d) NOT marking segment %d for deletion from path %d because its position (%d,%d,%d) is %5.2f cells from pseudo-end-terminal at (%d,%d).\n",
                omp_get_thread_num(), path_segment, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][path_segment].X,
                pathCoords[diffPairPathNum[i]][path_segment].Y, pathCoords[diffPairPathNum[i]][path_segment].Z,
                segment_to_pseudoTerm_distance, pseudoEndTerm.X, pseudoEndTerm.Y);
        }
      }
      #endif

    }  // End of for-loop for marking segments near end-terminal, starting from the start-terminal


    // Now that segments have been marked for deletion, we do one more pass over the shoulder path's
    // segments to enforce the following rule: If *any* segment in a vertically aligned via-stack is
    // marked for deletion by this function, then *all* segments in that via-stack must be marked
    // for deletion.
    //
    // First, iterate over all segments, starting at the *BEGINNING* of the shoulder-path and
    // moving forwards:
    //
    Coordinate_t prevSegmentCoord = copyCoordinates(mapInfo->start_cells[diffPairPathNum[i]]);
    unsigned char prev_deleteSegment = 0;

    #ifdef DEBUG_markDiffPairSegmentsNearTerminals
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Analyzing vertically aligned vias in the forward direction along path %d in function markDiffPairSegmentsNearTerminals.\n",
             omp_get_thread_num(), diffPairPathNum[i]);
    }
    #endif

    for (int path_segment = 0; path_segment < pathLength[diffPairPathNum[i]]; path_segment++)  {
      // Check if the current segment is vertically aligned with the previous segment:
      if (   (pathCoords[diffPairPathNum[i]][path_segment].X == prevSegmentCoord.X)
          && (pathCoords[diffPairPathNum[i]][path_segment].Y == prevSegmentCoord.Y))  {

        // We got here, so the current segment is vertically aligned with the previous segment.
        // If the previous segment was marked for deletion by this function, then also mark
        // the current segment for deletion:
        if ((prev_deleteSegment & 4) || (prev_deleteSegment & 8))  {  // Check if bit #2 or #3 is set
          // We got here, so the current segment is vertically aligned with the previous segment,
          // and the previous segment was marked for deletion by this function. So we mark the
          // the current segment for deletion, too, by setting bit #3 of the deleteSegment
          // element, by OR'ing it with '8':
          deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 8;

          #ifdef DEBUG_markDiffPairSegmentsNearTerminals
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Marking segment %d (%d,%d,%d) for deletion from path %d because another segment in the same vertically aligned via was marked.\n",
                   omp_get_thread_num(), path_segment, pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                   pathCoords[diffPairPathNum[i]][path_segment].Z, diffPairPathNum[i]);
          }
          #endif

        }  // End of if-block for previous segment being marked for deletion
      }  // End of if-block for current segment being vertically aligned with previous segment

      // In anticipation of the next time through this loop, populate the variables
      // 'prevSegmentCoord' and 'prev_deleteSegment' using the current segment's
      // values:
      prevSegmentCoord = copyCoordinates(pathCoords[diffPairPathNum[i]][path_segment]);
      prev_deleteSegment = deleteSegment[i][path_segment];

    }  // End of for-loop for index 'path_segment', moving forward through the path

    //
    // Second, iterate over all segments, starting at the *END* of the shoulder-path and
    // moving backwards:
    //
    prevSegmentCoord = copyCoordinates(mapInfo->end_cells[diffPairPathNum[i]]);
    prev_deleteSegment = 0;

    #ifdef DEBUG_markDiffPairSegmentsNearTerminals
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Analyzing vertically aligned vias in the reverse direction along path %d in function markDiffPairSegmentsNearTerminals.\n",
             omp_get_thread_num(), diffPairPathNum[i]);
    }
    #endif

    for (int path_segment = pathLength[diffPairPathNum[i]] - 2; path_segment >= 0; path_segment--)  {
      // Check if the current segment is vertically aligned with the 'previous' segment. Note that
      // the 'previous' segment is the one with the next higher segment index, since we're moving
      // backwards along the path:
      if (   (pathCoords[diffPairPathNum[i]][path_segment].X == prevSegmentCoord.X)
          && (pathCoords[diffPairPathNum[i]][path_segment].Y == prevSegmentCoord.Y))  {

        // We got here, so the current segment is vertically aligned with the 'previous' segment.
        // If the 'previous' segment was marked for deletion by this function, then also mark
        // the current segment for deletion:
        if ((prev_deleteSegment & 4) || (prev_deleteSegment & 8))  {  // Check if bit #2 or #3 is set
          // We got here, so the current segment is vertically aligned with the 'previous' segment,
          // and the 'previous' segment was marked for deletion by this function. So we mark the
          // the current segment for deletion, too, by setting bit #3 of the deleteSegment
          // element, by OR'ing it with '8':
          deleteSegment[i][path_segment] = deleteSegment[i][path_segment] | 8;

          #ifdef DEBUG_markDiffPairSegmentsNearTerminals
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Marking segment %d (%d,%d,%d) for deletion from path %d because another segment in the same vertically aligned via was marked.\n",
                   omp_get_thread_num(), path_segment, pathCoords[diffPairPathNum[i]][path_segment].X, pathCoords[diffPairPathNum[i]][path_segment].Y,
                   pathCoords[diffPairPathNum[i]][path_segment].Z, diffPairPathNum[i]);
          }
          #endif

        }  // End of if-block for 'previous' segment being marked for deletion
      }  // End of if-block for current segment being vertically aligned with 'previous' segment

      // In anticipation of the next time through this loop, populate the variables
      // 'prevSegmentCoord' and 'prev_deleteSegment' using the current segment's
      // values:
      prevSegmentCoord = copyCoordinates(pathCoords[diffPairPathNum[i]][path_segment]);
      prev_deleteSegment = deleteSegment[i][path_segment];

    }  // End of for-loop for index 'path_segment', moving backwards through the path

  }  // End of for-loop for index 'i' (ranging from 0 to 1)

}  // End of function 'markDiffPairSegmentsNearTerminals'


//-----------------------------------------------------------------------------
// Name: markDiffPairTracesNearDRboundary
// Desc: Mark for deletion the segments from the diff-pair shoulder points that
//       could cause DRC violations if the shoulder point crosses a design-rule
//       boundary for which the line/space rules are different on either side.
//       This function modifies the 2-dimensional array 'deleteSegment' by
//       setting bit #1 of the element associated with the segment to be deleted.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_markDiffPairTracesNearDRboundary' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_markDiffPairTracesNearDRboundary
#undef DEBUG_markDiffPairTracesNearDRboundary

static void markDiffPairTracesNearDRboundary(const int pseudoPathNum, const int path_1_number, const int path_2_number,
                                             Coordinate_t *pathCoords[], int pathLengths[], InputValues_t *user_inputs,
                                             CellInfo_t ***cellInfo, MapInfo_t *mapInfo, unsigned char ** deleteSegment)  {

  // The algorithm for deleting points in this function roughly follows these steps:
  //
  // For each diff-pair net i:
  //    For each segment j in diff-pair net i:
  //      If segment j is in different DR-zone than segment j-1:
  //        Calculate distance R as maximum of the 'cong_radius' parameter (i.e., RTRACE,1 + STRACE,1-2 + RTRACE,2) between the two DR-zones #1 and #2.
  //        Define point 'C' as the segment in path i midway between the two segments that are adjacent to the design-rule boundary.
  //        For each segment k of partner-net P of diff-pair net i:
  //          Delete the segment if:
  //            The segment is on the same layer as point C, and
  //            The segment is within a distance R of point C.

  // Get the numbers of the two diff-pair paths associated with this pseudo-net:
  int diffPairPathNum[2];
  diffPairPathNum[0] = path_1_number;
  diffPairPathNum[1] = path_2_number;

  #ifdef DEBUG_markDiffPairTracesNearDRboundary
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (   (mapInfo->current_iteration >= 248) && (mapInfo->current_iteration <= 250)
      && (pseudoPathNum == 35))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in markDiffPairTracesNearDRboundary() because specific requirements were met.\n\n", omp_get_thread_num());
    printf("DEBUG: (thread %2d) Entered function 'markDiffPairTracesNearDRboundary' with pseudo-path %d (length %d), diff-pair path %d (length %d) and path %d (length %d)\n",
           omp_get_thread_num(), pseudoPathNum, pathLengths[pseudoPathNum], diffPairPathNum[0], pathLengths[diffPairPathNum[0]], diffPairPathNum[1], pathLengths[diffPairPathNum[1]]);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif


  // Temporary variable to hold coordinates of previous segment and design-rule
  // numbers of diff-pair paths
  Coordinate_t prevSegmentCoords;
  int prev_DR_num;
  int prev_DR_subset;

  // Iterate over both diff-pair paths:
  for (int i = 0; i < 2; i++)  {

    // Capture the first coordinate of the diff-pair path, and its associated design-rule
    // number and subset:
    prevSegmentCoords = copyCoordinates(mapInfo->start_cells[diffPairPathNum[i]]);
    prev_DR_num       = cellInfo[mapInfo->start_cells[diffPairPathNum[i]].X][mapInfo->start_cells[diffPairPathNum[i]].Y][mapInfo->start_cells[diffPairPathNum[i]].Z].designRuleSet;
    prev_DR_subset    = user_inputs->designRuleSubsetMap[diffPairPathNum[i]][prev_DR_num];

    // Iterate over each segment of the (non-contiguous) diff-pair path:
    #ifdef DEBUG_markDiffPairTracesNearDRboundary
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) About to iterate over segments of path %d\n", omp_get_thread_num(), diffPairPathNum[i]);
    }
    #endif
    for (int segment = 0; segment < pathLengths[diffPairPathNum[i]]; segment++)  {

      // Get the design-rule number for this segment:
      int x = pathCoords[diffPairPathNum[i]][segment].X;
      int y = pathCoords[diffPairPathNum[i]][segment].Y;
      int z = pathCoords[diffPairPathNum[i]][segment].Z;
      int DR_num = cellInfo[x][y][z].designRuleSet;

      // Get the design-rule subset number of the diff-pair net:
      int DR_subset = user_inputs->designRuleSubsetMap[diffPairPathNum[i]][DR_num];

      // printf("DEBUG: (thread %2d) Segment %d of path %d at (%d,%d,%d) has design-rule zone %d and design-rule subset %d.\n",
      //        omp_get_thread_num(), segment, diffPairPathNum[i], x, y, z, DR_num, DR_subset);


      // If current segment is in a different design-rule zone from the previous segment
      // (but on the same layer), then start performing calculations and operations to mark
      // segments of the partner diff-pair net for deletion:
      //
      if ((DR_num != prev_DR_num) && (z == prevSegmentCoords.Z))  {

        #ifdef DEBUG_markDiffPairTracesNearDRboundary
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) At segment %d of path %d at (%d,%d,%d), we found a new design-rule zone: %d. The previous zone was %d at (%d,%d,%d).\n",
                 omp_get_thread_num(), segment, diffPairPathNum[i], x, y, z, DR_num, prev_DR_num, prevSegmentCoords.X, prevSegmentCoords.Y, prevSegmentCoords.Z);
        }
        #endif

        // Calculate distance R ('radius') as maximum of the 'cong_radius' parameter
        // [i.e., R(TRACE,1) + S(TRACE,1-2) + R*TRACE,2)] between the two
        // DR-zones #1 and #2. Recall that cong_radius has 4 indices:
        //    cong_radius[i][m][j][[n], with  i = source design-rule number,
        //                                    m = source_subset_shapeType_index,
        //                                    j = target design_rule number, and
        //                                    n = target_subset_shapeType_index.
        // Indices m and n are defined as:
        //   subset_shapeType_index = DR_subset * NUM_SHAPE_TYPES   +   shape_type.
        // Recall that 'shape_type' = 'TRACE' in the case of traces (not vias).
        int prev_subset_shapeType_index    = prev_DR_subset * NUM_SHAPE_TYPES  +  TRACE;
        int current_subset_shapeType_index = DR_subset      * NUM_SHAPE_TYPES  +  TRACE;
        float radius_before_boundary = user_inputs->cong_radius[prev_DR_num][ prev_subset_shapeType_index  ][prev_DR_num][ prev_subset_shapeType_index  ];
        float radius_after_boundary  = user_inputs->cong_radius[  DR_num   ][current_subset_shapeType_index][  DR_num   ][current_subset_shapeType_index];

        // Define the 'centerPoint' coordinate as the midpoint between the two segments that
        // are adjacent to the design-rule boundary:
        Coordinate_t centerPoint;
        centerPoint.X = (prevSegmentCoords.X + pathCoords[diffPairPathNum[i]][segment].X) / 2;
        centerPoint.Y = (prevSegmentCoords.Y + pathCoords[diffPairPathNum[i]][segment].Y) / 2;
        centerPoint.Z = pathCoords[diffPairPathNum[i]][segment].Z;

        // Define the 'radius' as the larger congestion radius values from the two
        // design-rule zones. Add 1 cell to account for rounding errors.
        float radius = max (radius_before_boundary, radius_after_boundary) + 1.0;

        #ifdef DEBUG_markDiffPairTracesNearDRboundary
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)    The new 'radius' value is %6.3f is the maximum of the pre- and post-boundary radii (%6.3f and %6.3f)\n",
                 omp_get_thread_num(), radius, radius_before_boundary, radius_after_boundary);
          printf("DEBUG: (thread %2d)    The coordinates before and after the boundary are (%d,%d,%d) and (%d,%d,%d)\n",
                 omp_get_thread_num(),  prevSegmentCoords.X, prevSegmentCoords.Y, prevSegmentCoords.Z,
                 pathCoords[diffPairPathNum[i]][segment].X, pathCoords[diffPairPathNum[i]][segment].Y, pathCoords[diffPairPathNum[i]][segment].Z);
          printf("DEBUG: (thread %2d)    The centerPoint from which the radius is measured is (%d,%d,%d)\n", omp_get_thread_num(),
                 centerPoint.X, centerPoint.Y, centerPoint.Z);
        }
        #endif

        // Get the path number of the partner net to the current path:
        int partnerPathNum, partner_index;
        if (i == 0)  {
          // If i=0, then the partner path number is in element 'diffPairPathNum[1]'
          partnerPathNum = diffPairPathNum[1];
          partner_index = 1;
        }
        else  {
          // If i=1, then the partner path number is in element 'diffPairPathNum[0]'
          partnerPathNum = diffPairPathNum[0];
          partner_index = 0;
        }

        // Iterate through each segment of this path's partner net and mark for
        // deletion the segments that are close to the boundary:
        for (int partnerSegment = 0; partnerSegment < pathLengths[partnerPathNum]; partnerSegment++)  {

          // Get the coordinates of the segment in the partner path:
          int partnerX = pathCoords[partnerPathNum][partnerSegment].X;
          int partnerY = pathCoords[partnerPathNum][partnerSegment].Y;

          // Mark the current partner-segment for deletion if it satisfies all of
          // the following criteria: (1) The segment is in the DR zone with the smaller
          // radius D-value, (2) the segment is on the same layer as the centerPoint,
          // and the segment is within a distance radius of point 'centerPoint':
          float distance_to_centerPoint = calc_2D_Pythagorean_distance_ints(partnerX, partnerY, centerPoint.X, centerPoint.Y);

          if (distance_to_centerPoint <= radius)  {

            // Flag segment of partner net for deletion by setting bit #1 of the deleteSegment element. This
            // is done by OR'ing the element with '2':
            deleteSegment[partner_index][partnerSegment] = deleteSegment[partner_index][partnerSegment] | 2;

            #ifdef DEBUG_markDiffPairTracesNearDRboundary
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d)          In partner net %d, segment %d at (%d,%d,%d) is marked for deletion. It's %6.3f cells from center-point (%d,%d,%d).\n",
                     omp_get_thread_num(), partnerPathNum, partnerSegment, partnerX, partnerY, pathCoords[partnerPathNum][partnerSegment].Z,
                     distance_to_centerPoint, centerPoint.X, centerPoint.Y, centerPoint.Z);
            }
            #endif

          }  // End of if-block for flagging segments for deletion
        }  // End of for-loop for index 'partnerSegment'
      }  // End of if-block for finding a new design-rule zone

      // In preparation for the next run through this loop, copy the current segment's
      // coordinates into the 'prevSegmentCoords' variable, and copy the current design-rule
      // set and subset numbers to the 'prev_DR_num' and 'prev_DR_subset' variables:
      prevSegmentCoords = copyCoordinates(pathCoords[diffPairPathNum[i]][segment]);
      prev_DR_num       = DR_num;
      prev_DR_subset    = DR_subset;

    }  // End of for-loop for index 'segment'
  }  // End of for-loop for index 'i' (from 0 to 1)

}  // End of function 'markDiffPairTracesNearDRboundary'


//-----------------------------------------------------------------------------
// Name: markPartialDiffPairVias
// Desc: Mark for deletion the segments from the diff-pair shoulder points that
//       are part of a vertically aligned via in which other segments have been
//       marked for deletion. This function modifies the 2-dimensional array
//       'deleteSegment' by setting bit #4 of the element associated with the
//       segment to be deleted.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_markPartialDiffPairVias' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_markPartialDiffPairVias
#undef DEBUG_markPartialDiffPairVias

static void markPartialDiffPairVias(const int path_1_number, const int path_2_number,
                                    Coordinate_t *pathCoords[], int pathLengths[],
                                    MapInfo_t *mapInfo, unsigned char ** deleteSegment)  {

  // The algorithm for deleting points in this function roughly follows these steps:
  //
  // For each diff-pair net i:
  //    For each segment j in diff-pair net i:
  //      Mark segment j for deletion if it satisfies all of the following criteria:
  //        (a) It is not already marked for deletion, and
  //        (a) It is vertically aligned with the previous or next segment, and
  //        (b) The previous or next segment is marked for deletion, and
  //        (c) Its 'flag' bit is not set, which would indicate that it should never
  //            be deleted.

  // Get the numbers of the two diff-pair paths associated with this pseudo-net:
  int diffPairPathNum[2];
  diffPairPathNum[0] = path_1_number;
  diffPairPathNum[1] = path_2_number;

  // Temporary variable to hold coordinates of previous segment:
  Coordinate_t prevSegmentCoords;

  // Iterate over both diff-pair paths:
  for (int i = 0; i < 2; i++)  {

    // Capture the first coordinate of the diff-pair path, and its associated design-rule
    // number and subset:
    prevSegmentCoords = copyCoordinates(mapInfo->start_cells[diffPairPathNum[i]]);

    // Iterate over each segment of the (non-contiguous) diff-pair path:
    #ifdef DEBUG_markDiffPairTracesNearDRboundary
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) About to iterate over segments of path %d\n", omp_get_thread_num(), diffPairPathNum[i]);
    }
    #endif
    for (int segment = 0; segment < pathLengths[diffPairPathNum[i]]; segment++)  {

      // If segment is already marked for deletion, or if the segment's 'flag' bit is
      // set, then move on to the next segment:
      if ((deleteSegment[i][segment]) || (pathCoords[diffPairPathNum[i]][segment].flag))  {
        continue;
      }

      // If the previous segment is vertically aligned with the current segment, and if the
      // previous segment is marked for deletion, then mark the current segment for deletion:
      if (   (segment > 0) && (pathCoords[diffPairPathNum[i]][segment].X == prevSegmentCoords.X)
          && (pathCoords[diffPairPathNum[i]][segment].Y == prevSegmentCoords.Y)
          && (deleteSegment[i][segment - 1]))  {

        // Set bit #4 of the deleteSegment element by OR'ing it with '16':
        deleteSegment[i][segment] = deleteSegment[i][segment] | 16;

        // If any subsequent segments are vertically aligned with the current segment, then also
        // mark them for deletion until we get to the end of the via, or the end of the diff-pair
        // path, or reach a flagged segment.
        for (int next_segment = segment + 1; next_segment < pathLengths[diffPairPathNum[i]] - 1; next_segment++)  {
          if (   (pathCoords[diffPairPathNum[i]][next_segment].X != pathCoords[diffPairPathNum[i]][segment].X)
              || (pathCoords[diffPairPathNum[i]][next_segment].Y != pathCoords[diffPairPathNum[i]][segment].Y)
              || (pathCoords[diffPairPathNum[i]][next_segment].flag))  {
        	  break;
          }

          // We got here, so a subsequent segment is vertically aligned. Mark it for deletion if it's
          // not already so marked:
          if (! deleteSegment[i][next_segment])  {
            // Set bit #4 of the deleteSegment element by OR'ing it with '16':
            deleteSegment[i][next_segment] = deleteSegment[i][next_segment] | 16;
          }
        }  // End of for-loop for index 'next_segment'
      }  // End of if-block for checking previous segment


      // If the subsequent segment is vertically aligned with the current segment, and if the
      // subsequent segment is marked for deletion, then mark the current segment for deletion:
      if (   (segment < pathLengths[diffPairPathNum[i]] - 1)
          && (pathCoords[diffPairPathNum[i]][segment].X == pathCoords[diffPairPathNum[i]][segment + 1].X)
          && (pathCoords[diffPairPathNum[i]][segment].Y == pathCoords[diffPairPathNum[i]][segment + 1].Y)
          && (deleteSegment[i][segment + 1]))  {

        // Set bit #4 of the deleteSegment element by OR'ing it with '16':
        deleteSegment[i][segment] = deleteSegment[i][segment] | 16;

        // If any previous segments are vertically aligned with the current segment, then also
        // mark them for deletion until we get to the beginning of the via, or the start of the diff-pair
        // path, or reach a flagged segment.
        for (int prev_segment = segment - 1; prev_segment >= 0; prev_segment--)  {
          if (   (pathCoords[diffPairPathNum[i]][prev_segment].X != pathCoords[diffPairPathNum[i]][segment].X)
              || (pathCoords[diffPairPathNum[i]][prev_segment].Y != pathCoords[diffPairPathNum[i]][segment].Y)
              || (pathCoords[diffPairPathNum[i]][prev_segment].flag))  {
        	  break;
          }

          // We got here, so a previous segment is vertically aligned. Mark it for deletion if it's
          // not already so marked:
          if (! deleteSegment[i][prev_segment])  {
            // Set bit #4 of the deleteSegment element by OR'ing it with '16':
            deleteSegment[i][prev_segment] = deleteSegment[i][prev_segment] | 16;
          }
        }  // End of for-loop for index 'prev_segment'
      }  // End of if-block for checking subsequent segment


      // In preparation for the next run through this loop, copy the current segment's
      // coordinates into the 'prevSegmentCoords' variable:
      prevSegmentCoords = copyCoordinates(pathCoords[diffPairPathNum[i]][segment]);

    }  // End of for-loop for index 'j' (0 to pathLength)

  }  // End of for-loop for index 'i' (0 to 1)

}  // End of function 'markPartialDiffPairVias'



//-----------------------------------------------------------------------------
// Name: deleteSelectedDiffPairSegments
// Desc: Delete selected diff-pair segments associated with pseudo-path
//       'pseudoPathNum'. The selected segments are (a) near pseudo-vias,
//       (b) near design-rule boundaries, and (c) near terminals.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_deleteSelectedDiffPairSegments' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_deleteSelectedDiffPairSegments 1
#undef DEBUG_deleteSelectedDiffPairSegments

void deleteSelectedDiffPairSegments(const int pseudoPathNum, Coordinate_t *pathCoords[],
                                    int pathLengths[], InputValues_t *user_inputs,
                                    CellInfo_t ***cellInfo, MapInfo_t *mapInfo)  {

  // Get the numbers of the two diff-pair paths associated with this pseudo-net:
  int diffPairPathNum[2];
  diffPairPathNum[0] = user_inputs->pseudoNetToDiffPair_1[pseudoPathNum];
  diffPairPathNum[1] = user_inputs->pseudoNetToDiffPair_2[pseudoPathNum];

  #ifdef DEBUG_deleteSelectedDiffPairSegments
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if (   (mapInfo->current_iteration >= 84) && (mapInfo->current_iteration <= 84)
      && ((pseudoPathNum == 2) || (pseudoPathNum == 2)))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in deleteSelectedDiffPairSegments() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE

  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Entered function deleteSelectedDiffPairSegments with pseudoPathNum = %d (diff-pair paths %d and %d)\n",
           omp_get_thread_num(), pseudoPathNum, diffPairPathNum[0], diffPairPathNum[1]);
  }
  #endif

  //
  // In preparation for deleting segments from the diff-pair paths, create arrays
  // that flag which segments to delete, and also why they're being deleted.
  //
  // Bit #0 of this 8-bit element is set if the segment is flagged because it's
  //        near a pseudo-via.
  // Bit #1 is set if the segment is flagged because it's near a design-rule boundary.
  // Bit #2 is set if the segment is flagged because it's near a terminal.
  // Bit #3 is set if the segment is flagged because it's vertically aligned with a
  //        segment that is near a terminal.
  // Bit #4 is set if the segment is flagged because it's vertically aligned with a
  //        segment that was flagged for deletion due to the above rules.
  //
  // deleteSegment[0 or 1][segment_number] is Boolean flag for deleting 'segment_number' of either
  // the first or second diff-pair path associated with pseudo-path 'pseudoPathNum'
  unsigned char *deleteSegment[2];
  for (int i = 0; i < 2; i++)  {
    deleteSegment[i] = malloc(pathLengths[diffPairPathNum[i]] * sizeof(unsigned char *));
    #ifdef DEBUG_deleteSelectedDiffPairSegments
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) malloc'd %d elements for deleteElement[%d]\n", omp_get_thread_num(), pathLengths[diffPairPathNum[i]], i);
    }
    #endif
    for (int segment = 0; segment < pathLengths[diffPairPathNum[i]]; segment++)  {
      deleteSegment[i][segment] = FALSE;  // Initialize 'deleteSegment' array of flags.
    }  // End of for-loop for index 'segment'
  }  // End of for-loop for index 'i' (either 0 or 1)

  // Mark for deletion non-via segments from shoulder-paths beside pseudo-net 'pathNum' that
  // are close to the newly created pseudo-vias:
  #ifdef DEBUG_deleteSelectedDiffPairSegments
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) About to enter markDiffPairSegmentsNearPseudoVia from deleteSelectedDiffPairSegments.\n", omp_get_thread_num());
  }
  #endif

  markDiffPairSegmentsNearPseudoVia(pseudoPathNum, diffPairPathNum[0], diffPairPathNum[1], pathCoords, pathLengths, user_inputs, cellInfo, mapInfo, deleteSegment);

  // Delete non-via segments from shoulder-paths associated with pseudo-net 'pathNum' that
  // are close to the boundaries of different design-rule zones:
  #ifdef DEBUG_deleteSelectedDiffPairSegments
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Returned from markDiffPairSegmentsNearPseudoVia in deleteSelectedDiffPairSegments. About to enter markDiffPairTracesNearDRboundary.\n",
           omp_get_thread_num());
  }
  #endif

  markDiffPairTracesNearDRboundary(pseudoPathNum, diffPairPathNum[0], diffPairPathNum[1], pathCoords, pathLengths, user_inputs, cellInfo, mapInfo, deleteSegment);

  // Delete via and trace segments from shoulder-paths associated with pseudo-net 'pathNum' that
  // are close to the terminals of the diff-pair nets:
  #ifdef DEBUG_deleteSelectedDiffPairSegments
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Returned from markDiffPairTracesNearDRboundary in deleteSelectedDiffPairSegments. About to enter markDiffPairSegmentsNearTerminals.\n",
           omp_get_thread_num());
  }
  #endif

  markDiffPairSegmentsNearTerminals(pseudoPathNum, diffPairPathNum[0], diffPairPathNum[1], pathCoords, pathLengths, user_inputs, cellInfo, mapInfo, deleteSegment);

  #ifdef DEBUG_deleteSelectedDiffPairSegments
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Returned from markDiffPairSegmentsNearTerminals in deleteSelectedDiffPairSegments. About to enter markPartialDiffPairVias.\n",
           omp_get_thread_num());
  }
  #endif

  markPartialDiffPairVias(diffPairPathNum[0], diffPairPathNum[1], pathCoords, pathLengths, mapInfo, deleteSegment);

  #ifdef DEBUG_deleteSelectedDiffPairSegments
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Returned from markPartialDiffPairVias in deleteSelectedDiffPairSegments. About to delete the marked segments...\n",
           omp_get_thread_num());
  }
  #endif


  //
  // Now that we've flagged segments to be deleted, go and delete the flagged elements
  // from the diff-pair path arrays. If the 'flag' element of the path segment is TRUE,
  // however, we do *not* delete the segment. Such segments were created in function
  // createDiffPairVias() by searching exhaustively for cells that allowed up/down
  // route-directions.
  //
  // Iterate over both diff-pair paths:
  for (int i = 0; i < 2; i++)  {
    int new_path_length = 0; // Length of new (shortened) path
    #ifdef DEBUG_deleteSelectedDiffPairSegments
    if (DEBUG_ON)  {
      printf("\nDEBUG: (thread %2d) Results for path %d from function deleteSelectedDiffPairSegments:\n", omp_get_thread_num(), diffPairPathNum[i]);
    }
    #endif
    for (int segment = 0; segment < pathLengths[diffPairPathNum[i]]; segment++)  {
      if ((! deleteSegment[i][segment]) || pathCoords[diffPairPathNum[i]][segment].flag)  {
        #ifdef DEBUG_deleteSelectedDiffPairSegments
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Retaining segment %d (%d,%d,%d) from diff-pair path %d in 'deleteSelectedDiffPairSegments'. deleteSegment = %d. flag = %d\n",
                 omp_get_thread_num(), segment,  pathCoords[diffPairPathNum[i]][segment].X, pathCoords[diffPairPathNum[i]][segment].Y,
                 pathCoords[diffPairPathNum[i]][segment].Z, diffPairPathNum[i], deleteSegment[i][segment], pathCoords[diffPairPathNum[i]][segment].flag);
        }
        #endif
        pathCoords[diffPairPathNum[i]][new_path_length] = copyCoordinates(pathCoords[diffPairPathNum[i]][segment]);
        new_path_length++;
      }  // End of if-block
      else  {
        #ifdef DEBUG_deleteSelectedDiffPairSegments
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) DELETING segment %d (%d,%d,%d) from diff-pair path %d in 'deleteSelectedDiffPairSegments'. deleteSegment = %d. flag = %d\n",
                 omp_get_thread_num(), segment, pathCoords[diffPairPathNum[i]][segment].X, pathCoords[diffPairPathNum[i]][segment].Y,
                 pathCoords[diffPairPathNum[i]][segment].Z, diffPairPathNum[i], deleteSegment[i][segment], pathCoords[diffPairPathNum[i]][segment].flag);
        }
        #endif
      }  // End of else-block
    }  // End of for-loop for index 'segment'

    #ifdef DEBUG_deleteSelectedDiffPairSegments
    if (DEBUG_ON)  {
      printf("\nDEBUG: (thread %2d) After iterating over %d segments of path %d, new_path_length = %d\n\n", omp_get_thread_num(),
             pathLengths[diffPairPathNum[i]], diffPairPathNum[i], new_path_length);
    }
    #endif

    // Check whether the ends of the shoulder-paths coincide with the end-terminals
    // of the diff-pairs. If not (which is likely), then append the end-terminal
    // coordinates to the ends of the shoulder-paths.
    if (   (new_path_length == 0)
        || (pathCoords[diffPairPathNum[i]][new_path_length - 1].X != mapInfo->end_cells[diffPairPathNum[i]].X)
        || (pathCoords[diffPairPathNum[i]][new_path_length - 1].Y != mapInfo->end_cells[diffPairPathNum[i]].Y)
        || (pathCoords[diffPairPathNum[i]][new_path_length - 1].Z != mapInfo->end_cells[diffPairPathNum[i]].Z))  {

      // We got here, so we need to append the end-terminal to the shoulder-path:
      new_path_length++;

      // If new_path_length exceeds the initial length of the diff-pair path, then re-allocate memory
      // to this path for one extra segment:
      if (new_path_length > pathLengths[diffPairPathNum[i]])  {
        pathCoords[diffPairPathNum[i]] = realloc(pathCoords[diffPairPathNum[i]], new_path_length * sizeof(Coordinate_t));
      }

      // Add the end-terminal of the diff-pair to the end of the shoulder-path:
      pathCoords[diffPairPathNum[i]][new_path_length - 1] = copyCoordinates(mapInfo->end_cells[diffPairPathNum[i]]);

      #ifdef DEBUG_deleteSelectedDiffPairSegments
      if (DEBUG_ON)  {
        printf("\nDEBUG: (thread %2d) After deleting selected segments, the last segment (#%d) of path #%d at (%d,%d,%d)\n",
               omp_get_thread_num(), new_path_length - 2, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][new_path_length - 2].X,
               pathCoords[diffPairPathNum[i]][new_path_length - 2].Y, pathCoords[diffPairPathNum[i]][new_path_length - 2].Z);
        printf(  "DEBUG: (thread %2d) DOES NOT MATCH the path's end-terminal coordinates (%d,%d,%d). So a segment\n",
               omp_get_thread_num(), mapInfo->end_cells[diffPairPathNum[i]].X, mapInfo->end_cells[diffPairPathNum[i]].Y,
               mapInfo->end_cells[diffPairPathNum[i]].Z);
        printf(  "DEBUG: (thread %2d) at these coordinates was appended to this path at segment #%d.\n\n", omp_get_thread_num(),
               new_path_length - 1);
      }
      #endif

    }  // End of if-block for appending the end-terminal to the end of the shoulder-path

    #ifdef DEBUG_deleteSelectedDiffPairSegments
    else  {
      if (DEBUG_ON)  {
        printf("\nDEBUG: (thread %2d) After deleting selected segments, the last segment (#%d) of path #%d at (%d,%d,%d)\n",
               omp_get_thread_num(), new_path_length - 1, diffPairPathNum[i], pathCoords[diffPairPathNum[i]][new_path_length - 1].X,
               pathCoords[diffPairPathNum[i]][new_path_length - 1].Y, pathCoords[diffPairPathNum[i]][new_path_length - 1].Z);
        printf(  "DEBUG: (thread %2d) matches the path's end-terminal coordinates (%d,%d,%d). So no additional segment was appended.\n\n",
               omp_get_thread_num(), mapInfo->end_cells[diffPairPathNum[i]].X, mapInfo->end_cells[diffPairPathNum[i]].Y,
               mapInfo->end_cells[diffPairPathNum[i]].Z);
      }
    }
    #endif


    // Update pathLength value for diff-pair path, and re-size the array for the path:
    // printf("DEBUG: Diff-pair path %d is being changed from %d to %d segments.\n", diffPairPathNum[i], pathLengths[diffPairPathNum[i]], new_path_length);
    pathLengths[diffPairPathNum[i]] = new_path_length;
    pathCoords[diffPairPathNum[i]] = realloc(pathCoords[diffPairPathNum[i]], pathLengths[diffPairPathNum[i]] * sizeof(Coordinate_t));
  }  // End of for-loop for index 'i' (from 0 to 1)

  // Free the temporary arrays that flag segments for deletion:
  for (int i = 0; i < 2; i++)  {
    free(deleteSegment[i]);
    deleteSegment[i] = NULL;
  }

  #ifdef DEBUG_deleteSelectedDiffPairSegments
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Exiting function deleteSelectedDiffPairSegments with pseudoPathNum = %d (diff-pair paths %d and %d)\n",
           omp_get_thread_num(), pseudoPathNum, diffPairPathNum[0], diffPairPathNum[1]);
  }
  #endif

}  // End of function deleteSelectedDiffPairSegments
