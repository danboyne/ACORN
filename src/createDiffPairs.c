#include "global_defs.h"


//-----------------------------------------------------------------------------
// Name: calcParabolaCoefficients
// Desc: Calculate the coefficients of a parabola of the form:
//
//         y = ax^2 + bx + c,
//
//       given two or three data points. The number of data points is specified
//       by the input variable 'numPoints'. (If numPoints is 2, then the functions
//       returns the equation of a line, with 'a' equal to zero.) The function
//       sets the Boolean variable 'y_versus_x' to TRUE if the coefficients are
//       for a function plotted as y versus x. This variable is FALSE if the
//       function is plotted as x versus y. For a parabolic function, the three
//       coefficients are calculated using:
//
//                         y1                       y2                       y3
//         a  =   ------------------   +   ------------------   +   ------------------
//                (x1 - x2)(x1 - x3)       (x2 - x1)(x2 - x3)       (x3 - x1)(x3 - x2)
//
//                    y1(x2 + x3)              y2(x3 + x1)              y3(x1 + x2)
//         b  = - ------------------   -   ------------------   -   ------------------
//                (x1 - x2)(x1 - x3)       (x2 - x1)(x2 - x3)       (x3 - x1)(x3 - x2)
//
//                      y1x2x3                  y2x3x1                    y3x1x2
//         c  =   ------------------   +   ------------------   +   ------------------
//                (x1 - x2)(x1 - x3)       (x2 - x1)(x2 - x3)       (x3 - x1)(x3 - x2)
//
//                                                                   ---------      ---------
//       If the 3 data points are arranged in an 'L' shape (right),  | 1 | 2 |      | 1 | 3 |
//       then we must copy point_3 to point_2 and fit point_1        ---------  or  ---------
//       and point_2 to a straight line using the equations          |   | 3 |      |   | 2 |
//       listed below.                                               ---------      ---------
//
//       For a linear function, the coefficients are calculated by disregarding point_3:
//
//         a  =  0
//
//                 y2 - y1
//         b  =   ---------
//                 x2 - x1
//
//                x2y1 - x1y2
//         c  =  -------------  =  y1  -  bx1
//                  x2 - x1
//-----------------------------------------------------------------------------
static void calcParabolaCoefficients(int numPoints, Coordinate_t point_1, Coordinate_t point_2,
                                     Coordinate_t point_3, double *a, double *b, double *c,
                                     int *y_versus_x) {

  // Check that numPoints contains a valid value (2 or 3):
  if ((numPoints !=2 ) && (numPoints != 3))  {
    printf("\n\nERROR: An unexpected error occurred in function 'calcParabolaCoefficients' in which\n");
    printf("       the number of points is '%d', even though legal values are '2' or '3'.\n", numPoints);
    printf("       Please report this fatal error message to the software developer.\n\n");
    exit(1);
  }  // End of if-block for numPoints containing an illegal value

  // Initialize 'y_versus_x' to TRUE. It will be changed to FALSE below, if necessary.
  *y_versus_x = TRUE;

  // Check for the occurrence of an 'L' shape for the 3 points. If this situation is
  // detected, then we'll drop the middle point and return a linearly fitted function:
  if (numPoints == 3)  {
    if (   ((point_1.X == point_2.X) && (point_1.X != point_3.X) && (point_2.Y == point_3.Y) && (point_1.Y != point_3.Y))
        || ((point_1.X == point_3.X) && (point_1.X != point_2.X) && (point_2.Y == point_3.Y) && (point_1.Y != point_3.Y))
        || ((point_2.X == point_3.X) && (point_1.X != point_3.X) && (point_1.Y == point_2.Y) && (point_1.Y != point_3.Y))
        || ((point_1.X == point_3.X) && (point_1.X != point_2.X) && (point_1.Y == point_2.Y) && (point_1.Y != point_3.Y))
        || ((point_1.X == point_2.X) && (point_1.X != point_3.X) && (point_1.Y == point_3.Y) && (point_1.Y != point_2.Y))
        || ((point_2.X == point_3.X) && (point_1.X != point_3.X) && (point_1.Y == point_3.Y) && (point_1.Y != point_2.Y)) )  {

      // printf("DEBUG: An L-shaped tuple of points was detected, so dropping the middle of these points: (%d,%d,%d), (%d,%d,%d), and (%d,%d,%d).\n",
      //         point_1.X, point_1.Y, point_1.Z, point_2.X, point_2.Y, point_2.Z, point_3.X, point_3.Y, point_3.Z);

      // Redefine 'numPoints' from 3 to 2, and overwrite point_2 coordinates
      // from coordinates of point_3:
      numPoints = 2;
      point_2 = copyCoordinates(point_3);

    }  // End of if-block for detecting L-shaped coordinates
  }  // End of if-block for numPoints = 3

  // Check for the validity of the input coordinates:
  if (numPoints == 3)  {
    // Determine whether any of the three X-values are identical. If so, then set
    // y_versus_x to FALSE. But then also confirm that none of the Y-values are
    // identical:
    if ((point_1.X == point_2.X) || (point_1.X == point_3.X) || (point_2.X == point_3.X))  {
      *y_versus_x = FALSE;
      if ((point_1.Y == point_2.Y) || (point_1.Y == point_3.Y) || (point_2.Y == point_3.Y))  {
        printf("\n\nERROR: An unexpected error occurred in function 'calcParabolaCoefficients' in which\n");
        printf("       the three input data points contain equal X-values and equal Y-values. The three\n");
        printf("       data points are (%d,%d,%d), (%d,%d,%d), and (%d,%d,%d).\n", point_1.X, point_1.Y, point_1.Z,
                point_2.X, point_2.Y, point_2.Z, point_3.X, point_3.Y, point_3.Z);
        printf("       Please report this fatal error message to the software developer.\n\n");
        exit(1);
      }  // End of if-block for a pair of Y-values equal to each other.
    }  // End of if-block for a pair of X-values equal to each other.
  }
  else  {
    // Determine whether the two X-values are identical. If so, then set y_versus_x
    // to FALSE. But then also confirm that the Y-values are not identical.
    if (point_1.X == point_2.X)  {
      *y_versus_x = FALSE;
      if (point_1.Y == point_2.Y)  {
        printf("\n\nERROR: An unexpected error occurred in function 'calcParabolaCoefficients' in which\n");
        printf("       the two input data points contain equal X-values and equal Y-values. The two\n");
        printf("       data points are (%d,%d,%d) and (%d,%d,%d).\n", point_1.X, point_1.Y, point_1.Z,
                point_2.X, point_2.Y, point_2.Z);
        printf("       Please report this fatal error message to the software developer.\n\n");
        exit(1);
      }  // End of if-block for a pair of Y-values equal to each other.
    }  // End of if-block for a pair of X-values equal to each other.
  }  // End of if/else block

  // Local variables for calculating parabola coefficients:
  int x1, y1, x2, y2, x3, y3;

  // Assign 'x' and 'y' values, based on whether y_versus_x is TRUE:
  if (*y_versus_x)  {
    x1 = point_1.X;
    y1 = point_1.Y;

    x2 = point_2.X;
    y2 = point_2.Y;

    if (numPoints == 3)  {
      x3 = point_3.X;
      y3 = point_3.Y;
    }
  }
  else  {
    x1 = point_1.Y;
    y1 = point_1.X;

    x2 = point_2.Y;
    y2 = point_2.X;

    if (numPoints == 3)  {
      x3 = point_3.Y;
      y3 = point_3.X;
    }
  }  // End of if/else block for *y_versus_x

  //
  // Calculate the coefficients for the function:
  //
  if (numPoints == 2)  {
    // numPoints is 2, so calculate the coefficients of a line:

    // Coefficient 'a' is zero for a line:
    *a = 0.0;

    // Calculate coefficient 'b' for straight line:
    //                 y2 - y1
    //         b  =   ---------
    //                 x2 - x1
    *b = (double)(y2 - y1) / (double)(x2 - x1);

    // Calculate coefficient 'c' for straight line:
    //                x2y1 - x1y2
    //         c  =  -------------  =  y1  -  bx1
    //                  x2 - x1
    *c = (double)y1 - (*b)*(double)x1;
  }
  else  {
    // numPoints = 3, so calculate the denominators used for the coefficients 'a', 'b', and 'c':
    double denominator_1 = (double)( (x1-x2)*(x1-x3) );
    double denominator_2 = (double)( (x2-x1)*(x2-x3) );
    double denominator_3 = (double)( (x3-x1)*(x3-x2) );

    // Calculate coefficient 'a' for polynomial:
    //                       y1                       y2                       y3
    //       a  =   ------------------   +   ------------------   +   ------------------
    //              (x1 - x2)(x1 - x3)       (x2 - x1)(x2 - x3)       (x3 - x1)(x3 - x2)
    *a = (double)y1/denominator_1 + (double)y2/denominator_2 + (double)y3/denominator_3;

    // Calculate coefficient 'b' for polynomial:
    //                  y1(x2 + x3)              y2(x3 + x1)              y3(x1 + x2)
    //       b  = - ------------------   -   ------------------   -   ------------------
    //              (x1 - x2)(x1 - x3)       (x2 - x1)(x2 - x3)       (x3 - x1)(x3 - x2)
    *b = -(double)(y1*(x2+x3))/denominator_1 - (double)(y2*(x3+x1))/denominator_2 - (double)(y3*(x1+x2))/denominator_3;

    // Calculate coefficient 'c' for polynomial:
    //                    y1x2x3                  y2x3x1                    y3x1x2
    //       c  =   ------------------   +   ------------------   +   ------------------
    //              (x1 - x2)(x1 - x3)       (x2 - x1)(x2 - x3)       (x3 - x1)(x3 - x2)
    *c = (double)(y1*x2*x3)/denominator_1 + (double)(y2*x3*x1)/denominator_2 + (double)(y3*x1*x2)/denominator_3;
  }

}  // End of function 'calcParabolaCoefficients'


//-----------------------------------------------------------------------------
// Name: createDiffPairShoulderPoints
// Desc: Populate two arrays of path-coordinates: one for each diff-pair net on
//       either side of the pseudo-net contained in array 'pseudoPathCoords[]'.
//       This function places the coordinates of the two diff-pair nets in the
//       arrays pathCoords[p1] and pathCoords[p2], where p1 and p2 are the
//       diff-pair nets associated with the pseudo-path 'pseudoNetNumber' (in
//       the sequence that these nets fall in the user-defined netlist).
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_createDiffPairShoulderPoints' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_createDiffPairShoulderPoints 1
#undef DEBUG_createDiffPairShoulderPoints

void createDiffPairShoulderPoints(const int pseudoNetNumber, Coordinate_t *pathCoords[],
                                  int pathLength[], const InputValues_t *user_inputs,
                                  CellInfo_t ***cellInfo, const MapInfo_t *mapInfo)  {

  #ifdef DEBUG_createDiffPairShoulderPoints
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration >= 84) && (mapInfo->current_iteration <= 84) && (pseudoNetNumber == 2))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in createDiffPairShoulderPoints() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;
    printf("DEBUG: (thread %2d) Entered function 'createDiffPairShoulderPoints' in iteration %d with following input values:\n",
            omp_get_thread_num(), mapInfo->current_iteration);
    printf("       (thread %2d)     pseudoNetNumber = %d\n\n", omp_get_thread_num(), pseudoNetNumber);
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif

  // Get the path numbers of the two nets that this pseudo-net corresponds to:
  int path_1_number = user_inputs->pseudoNetToDiffPair_1[pseudoNetNumber];
  int path_2_number = user_inputs->pseudoNetToDiffPair_2[pseudoNetNumber];

  #ifdef DEBUG_createDiffPairShoulderPoints
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d)    path_1_number = %d\n", omp_get_thread_num(), path_1_number);
    printf("DEBUG: (thread %2d)    path_2_number = %d\n\n", omp_get_thread_num(), path_2_number);
  }
  #endif

  // Reallocate memory for the two diff-pair nets, so that we can add as many segments
  // as there are segments in the pseudo-net, plus one additional segment:
  pathCoords[path_1_number] = realloc(pathCoords[path_1_number], (pathLength[pseudoNetNumber] + 1) * sizeof(Coordinate_t));
  if (pathCoords[path_1_number] == 0)  {
    printf("\n\nERROR: Failed to reallocate memory for pathCoords[%d].\n\n", path_1_number);
    exit(1);
  }
  pathCoords[path_2_number] = realloc(pathCoords[path_2_number], (pathLength[pseudoNetNumber] + 1) * sizeof(Coordinate_t));
  if (pathCoords[path_2_number] == 0)  {
    printf("\n\nERROR: Failed to reallocate memory for pathCoords[%d].\n\n", path_2_number);
    exit(1);
  }

  // Copy the pseudo-path coordinates to a new array 'fullPseudoPathCoords[]' that
  // includes the starting coordinates of the pseudo-path:
  Coordinate_t *fullPseudoPathCoords;
  fullPseudoPathCoords = malloc((pathLength[pseudoNetNumber] + 1) * sizeof(Coordinate_t));
  fullPseudoPathCoords[0] = copyCoordinates(mapInfo->start_cells[pseudoNetNumber]);
  for (int i = 0; i < pathLength[pseudoNetNumber]; i++)  {
    fullPseudoPathCoords[i+1] = copyCoordinates(pathCoords[pseudoNetNumber][i]);
    // printf("DEBUG: For pseudo-path %d, segment %d is (%d,%d,%d)\n", pseudoNetNumber, i,
    //        pathCoords[pseudoNetNumber][i].X, pathCoords[pseudoNetNumber][i].Y, pathCoords[pseudoNetNumber][i].Z);
  }  // End of for-loop for index 'i'

  // Define variables to hold coefficients of parabola that we fit to 3 adjacent points
  // of the pseudo-net:
  double a = 0.0, b = 0.0, c = 0.0;

  // Define variables to hold the slope of the line that's tangent to the pseudo-net,
  // and perpendicular to the pseudo-net:
  double normal_slope = 0.0, tangent_slope = 0.0;

  // Define variables for the x- and y-unit-vectors that describe the normals
  // to the pseudo-net. (The initial values for both vectors are chosen
  // arbitrarily to point along the positive x-axis.)
  double X_unit_vector = 1.0, Y_unit_vector = 0.0, prev_X_unit_vector = 1.0, prev_Y_unit_vector = 0.0;

  // Define variables for the signs (+1 or -1) of the x- and y-unit-vectors:
  double X_unit_vector_sign = 1.0, Y_unit_vector_sign = 1.0;

  // Define variables used for calculating the angle (in radians) between the
  // unit-normal vectors of adjacent segments of the pseudo net:
  double normalAngle, normalDotProduct;

  // Define Boolean variable that specifies whether the fitted parabola is y versus x (TRUE)
  // or x versus y (FALSE):
  int y_versus_x = TRUE;

  // Define variables to hold the user-prescribed half-pitch (in units of cells) for
  // these two diff-pair nets. The pitch depends on the net number and the (x,y,z)
  // coordinate, which both affect the design-rule (DR) number and design-rule (DR)
  // subset number:
  int DR_set_number, DR_subset;
  float halfPitchCells;

  // Define a Boolean flag that tells the algorithm to NOT calculate shoulder points
  // in the special cases when:
  //   (a) a point is preceded and succeeded by points on different routing layers, or
  //   (b) if the terminal points are themselves via points, or
  //   (c) the pseudo-point is located in a swap-zone.
  int calculateShoulderPoints = TRUE;

  // Define a Boolean flag that determines whether the calculated shoulder points must
  // not be discarded, e.g., because they're associated with a terminal or a via:
  int doNotDiscard = TRUE;

  // Define a Boolean flag that determines whether the shoulder points must be
  // copied from the pseudo-path's points (as is done for some terminal points):
  int reUsePseudoCoordinates = FALSE;

  // Initialize shoulder_1_polarity and shoulder_2_polarity to +1 and -1,
  // respectively. These variables describe which side of the pseudo-net
  // each shoulder net is on, and can flip polarity during the algorithm.
  int shoulder_1_polarity =  1;
  int shoulder_2_polarity = -1;

  // Variables for calculating the angles between adjacent segments pseudo-path
  // and of shoulder paths:
  int      delX,      delY,      delX_1,     delY_1,     delX_2,     delY_2;
  double angle_1, angle_2;  // Angles between pseudo-path and shoulder-path segments
  double arccos_arg;  // Argument for the arccosine

  // Temporary coordinates for shoulder paths:
  int x_1, y_1, z_1, x_2, y_2, z_2, prev_x_1 = 0, prev_y_1 = 0, prev_x_2 = 0, prev_y_2 = 0;

  // Indices for shoulder paths:
  int i_1 = 0, i_2 = 0;

  //
  // Iterate over each segment of the pseudo-net to extract the segment's coordinates
  // and calculate the coordinates of the shoulder points:
  //
  for (int i = 0; i <= pathLength[pseudoNetNumber]; i++)  {

    #ifdef DEBUG_createDiffPairShoulderPoints
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Pseudo-path #%d, segment %d: (%d,%d,%d). Swap-zone = %d.\n", omp_get_thread_num(), pseudoNetNumber, i,
             fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z,
             cellInfo[fullPseudoPathCoords[i].X][fullPseudoPathCoords[i].Y][fullPseudoPathCoords[i].Z].swap_zone);
    }
    #endif

    // Set the flag that tells algorithm whether to calculate shoulder-point coordinates
    // for the current segment:
    calculateShoulderPoints = TRUE;

    // Set the flag that tells the algorithm whether to keep the calculated shoulder
    // points because, e.g., they're associated with terminals or vias.
    doNotDiscard = TRUE;

    // Clear the flag that determines whether the shoulder points must be
    // copied from the pseudo-path's points (as is done for some terminal points):
    reUsePseudoCoordinates = FALSE;


    // Check whether pseudo-path segment is in a swap-zone. If so, then we don't generate
    // shoulder-path points:
    if (cellInfo[fullPseudoPathCoords[i].X][fullPseudoPathCoords[i].Y][fullPseudoPathCoords[i].Z].swap_zone)  {
      // Do not calculate shoulder points if pseudo-path segment is in swap-zone.
      calculateShoulderPoints = FALSE;

      // No shoulder points will be calculated for this pseudo-net segment:
      doNotDiscard = FALSE;

      #ifdef DEBUG_createDiffPairShoulderPoints
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Pseudo-path #%d, segment %d at (%d,%d,%d) is in swap-zone #%d, so no shoulder-paths will be created.\n",
                omp_get_thread_num(), pseudoNetNumber, i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z,
                cellInfo[fullPseudoPathCoords[i].X][fullPseudoPathCoords[i].Y][fullPseudoPathCoords[i].Z].swap_zone);
      }
      #endif
    }
    else  {
      // If the pseudo-path segment is not in a swap-zone, then we categorize the current pseudo-path segment
      // into one of seven buckets, depending on nearby points:

      //
      // Category 1: The points before and after current point are on the same routing layer:
      if ((i >= 1) && (i <= pathLength[pseudoNetNumber] - 1)
          && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-1].Z)
          && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+1].Z))  {

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)  Found category #1 point for pseudo-path %d, segment %d at (%d,%d,%d).\n", omp_get_thread_num(),
                  pseudoNetNumber, i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
        }
        #endif

        // This point is not at a terminal or part of a via, so it may be discarded later.
        doNotDiscard = FALSE;

        // Calculate the equation of a parabola through the previous, current, and next
        // points in the path. The equation is given by:
        //     y = ax^2 + bx + c  if y_versus_x is TRUE, or
        //     x = ay^2 + by + c  if y_versus_x is FALSE.
        calcParabolaCoefficients(3, fullPseudoPathCoords[i-1], fullPseudoPathCoords[i], fullPseudoPathCoords[i+1],
                                 &a, &b, &c, &y_versus_x);
      }  // End of category 1's if-block
      //
      // Category 2a: The point is the last point in the pseudo-net and the two previous points are
      //              on the same routing layer.
      //    or:
      // Category 2b: The point is the last point on the current routing layer, before transitioning
      //              to a different routing layer, and the two preceding points are on the same routing
      //              layer as the current point.
      else if (   (   (i == pathLength[pseudoNetNumber]) && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-1].Z)
                   && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-2].Z))

               || (   (i >= 2) && (i <= pathLength[pseudoNetNumber] - 1) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i+1].Z)
                   && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-1].Z )
                   && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-2].Z )))  {

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)  Found category #2a/2b point for pseudo-path %d, segment %d at (%d,%d,%d).\n", omp_get_thread_num(),
                  pseudoNetNumber, i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
        }
        #endif

        // This point is a terminal or associated with a via, so do not discard the associated shoulder points:
        doNotDiscard = TRUE;

        // Calculate the equation of a parabola through the current and two previous points:
        calcParabolaCoefficients(3, fullPseudoPathCoords[i-2], fullPseudoPathCoords[i-1], fullPseudoPathCoords[i],
                                 &a, &b, &c, &y_versus_x);
      }  // End of category 2's if/else block
      //
      // Category 3a: The point is the first point in the pseudo-net and the next two points are
      //              on the same routing layer.
      //    or:
      // Category 3b: The point is the first point on the current routing layer, after transitioning
      //              from a different routing layer, and the next two points are on the same routing
      //              layer as the current point.
      else if (((i == 0) && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+1].Z)
                && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+2].Z))

              || ((i >= 1) && (i <= pathLength[pseudoNetNumber] - 2) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i-1].Z)
                && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+1].Z )
                && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+2].Z )))  {

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)  Found category #3a/3b point for pseudo-path %d, segment %d at (%d,%d,%d).\n", omp_get_thread_num(),
                  pseudoNetNumber, i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
        }
        #endif

        // This point is a terminal or associated with a via, so do not discard the associated shoulder points:
        doNotDiscard = TRUE;

        // Calculate the equation of a parabola through the current and next two points:
        calcParabolaCoefficients(3, fullPseudoPathCoords[i], fullPseudoPathCoords[i+1], fullPseudoPathCoords[i+2],
                                 &a, &b, &c, &y_versus_x);

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)    calcParabolaCoefficients returned with y_versus_x=%d, a=%.9f, b=%.9f, c=%.9f\n",
                 omp_get_thread_num(), y_versus_x, a, b, c);
        }
        #endif

      }  // End of category 3's if/else block
      //
      // Category 4a: The point is the first point in the pseudo-net, and the next point is on the
      //              same routing layer, and the point after that is on a different routing layer.
      //    or:
      // Category 4b: The point is the first point on the current routing layer, after transitioning
      //              from a different routing layer, and the next point is on the same routing
      //              layer as the current point, and the point after that is on a different layer.
      //    or:
      // Category 4c: The point is the second-to-last point in the pseudo-net, and is the first point
      //              on the current routing layer, after transitioning from a different routing layer,
      //              and the next point is on the same routing layer as the current point.
      else if (((i == 0) && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+1].Z)
              && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i+2].Z))

           || ((i >= 1) && (i <= pathLength[pseudoNetNumber] - 2) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i-1].Z)
              && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+1].Z )
              && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i+2].Z ))

           || ((i == pathLength[pseudoNetNumber] - 1) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i-1].Z)
              && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i+1].Z )))  {

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)  Found category #4a/4b/4c point for pseudo-path %d, segment %d at (%d,%d,%d).\n", omp_get_thread_num(),
                  pseudoNetNumber, i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
        }
        #endif

        // This point is a terminal or associated with a via, so do not discard the associated shoulder points:
        doNotDiscard = TRUE;

        // Calculate the equation of a straight line through the current point and next point:
        calcParabolaCoefficients(2, fullPseudoPathCoords[i], fullPseudoPathCoords[i+1], fullPseudoPathCoords[i+1],
                                 &a, &b, &c, &y_versus_x);
      }  // End of category 4's if/else block
      //
      // Category 5a: The point is the last point in the pseudo-net, and the previous point is on the
      //              same routing layer, and the point before that is on a different routing layer.
      //    or:
      // Category 5b: The point is the last point on the current routing layer, before transitioning
      //              to a different routing layer, and the previous point is on the same routing
      //              layer as the current point, and the point before that is on a different layer.
      //    or:
      // Category 5c: The point is the second point in the pseudo-net, and is the last point on the
      //              current routing layer, before transitioning to a different routing layer.
      else if (((i == pathLength[pseudoNetNumber]) && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-1].Z)
              && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i-2].Z))

           || ((i >= 2) && (i <= pathLength[pseudoNetNumber] - 1) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i+1].Z)
              && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-1].Z )
              && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i-2].Z ))

           || ((i == 1) && (fullPseudoPathCoords[i].Z == fullPseudoPathCoords[i-1].Z)
              && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i+1].Z )))  {

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)  Found category #5a/5b/5c point for pseudo-path %d, segment %d at (%d,%d,%d).\n", omp_get_thread_num(),
                  pseudoNetNumber, i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
        }
        #endif

        // This point is a terminal or associated with a via, so do not discard the associated shoulder points:
        doNotDiscard = TRUE;

        // Calculate the equation of a straight line through the current point and previous point:
        calcParabolaCoefficients(2, fullPseudoPathCoords[i-1], fullPseudoPathCoords[i], fullPseudoPathCoords[i],
                                 &a, &b, &c, &y_versus_x);
      }  // End of category 5's if/else block
      //
      // Category 6a: The point's preceding and subsequent points are both on different layers.
      //
      else if ((i >= 1) && (i <= pathLength[pseudoNetNumber] - 1) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i-1].Z)
               && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i+1].Z))  {

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)  Found category #6a point for pseudo-path %d, segment %d at (%d,%d,%d).\n", omp_get_thread_num(),
                  pseudoNetNumber, i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
        }
        #endif

        // Do not calculate shoulder points in this case.
        calculateShoulderPoints = FALSE;

        // No shoulder points will be calculated for this pseudo-net segment:
        doNotDiscard = FALSE;

      }  // End of category 6's if/else block
      //
      // Category 7a: The point is the first point in the pseudo-net, and the next point is
      //              on a different layer.
      //    or:
      // Category 7b: The point is the last point in the pseudo-net, and the preceding point is
      //              on a different layer.
      else if (((i == 0) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i+1].Z))

              || ((i == pathLength[pseudoNetNumber]) && (fullPseudoPathCoords[i].Z != fullPseudoPathCoords[i-1].Z)))  {

        // Do not calculate shoulder points in this case.
        calculateShoulderPoints = FALSE;

        // Instead, re-use the coordinates of the pseudo-terminals as the terminal-points
        // for both shoulder-paths:
        reUsePseudoCoordinates = TRUE;

        // Terminal points should not be discarded:
        doNotDiscard = TRUE;

      }  // End of category 7's if/else block
      //
      // We got here, which is not expected. Issue a fatal error message and exit the program:
      else  {
        printf("\n\nERROR: An unexpected error occurred in function 'createDiffPairShoulderPoints' in which\n");
        printf("       shoulder points could not be calculated on either side of point #%d at (%d,%d,%d).\n",
               i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
        printf("       This pseudo-net's indices range from 0 to %d, including the starting point.\n", pathLength[pseudoNetNumber]);
        printf("       Coordinates of the preceding and succeeding data points provided below, for reference.\n");
        if (i >= 2)
          printf("          2 points before: (%d,%d,%d) (#%d)\n", fullPseudoPathCoords[i-2].X, fullPseudoPathCoords[i-2].Y, fullPseudoPathCoords[i-2].Z, i-2);
        if (i >= 1)
          printf("           1 point before: (%d,%d,%d) (#%d)\n", fullPseudoPathCoords[i-1].X, fullPseudoPathCoords[i-1].Y, fullPseudoPathCoords[i-1].Z, i-1);
        printf(  "          0 points before: (%d,%d,%d) (#%d)\n", fullPseudoPathCoords[i  ].X, fullPseudoPathCoords[i  ].Y, fullPseudoPathCoords[i  ].Z, i);
        if (i <= pathLength[pseudoNetNumber] - 1)
          printf("            1 point after: (%d,%d,%d) (#%d)\n", fullPseudoPathCoords[i+1].X, fullPseudoPathCoords[i+1].Y, fullPseudoPathCoords[i+1].Z, i+1);
        if (i <= pathLength[pseudoNetNumber] - 2)
          printf("           2 points after: (%d,%d,%d) (#%d)\n", fullPseudoPathCoords[i+2].X, fullPseudoPathCoords[i+2].Y, fullPseudoPathCoords[i+2].Z, i+2);
        printf("\n       Please report this fatal error message to the software developer.\n\n");
        exit(1);
      }  // End of final else-block in multi-stage if/else structure
    }  // End of else-block for pseudo-path segment NOT being in a swap-zone

    //
    // Go and calculate the coordinates of the shoulder points for the current
    // segment of the pseudo-path:
    //
    if (calculateShoulderPoints || reUsePseudoCoordinates)  {

      if (calculateShoulderPoints)  {
        //
        // Calculate provisional shoulder points based on the 'a', 'b', and 'c'
        // quadratic coefficients calculated above:
        //

        // Calculate the slope of the line that's tangent to the pseudo-net, and the slope of
        // the line that's normal to the tangent:
        if (y_versus_x)  {
          // Slope of line is first derivative of parabola, which is dy/dx = 2ax + b:
          tangent_slope = 2 * a * fullPseudoPathCoords[i].X    +    b;

          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)  y_versus_x is TRUE, and tangent_slope = %.9f\n",
                   omp_get_thread_num(), tangent_slope);
          }
          #endif

        }  // End of if-block for y_versus_x == TRUE
        else  {
          // Slope of line is reciprocal of first derivative of parabola, which is 1/(dx/dy) = 1/(2ay + b):
          tangent_slope = 2 * a * fullPseudoPathCoords[i].Y    +    b;   // == dx/dy

          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)  y_versus_x is FALSE, and preliminary tangent_slope = %.9f\n",
                   omp_get_thread_num(), tangent_slope);
          }
          #endif

          // If dx/dy is non-zero, then take reciprocal to get dy/dx:
          if (fabs(tangent_slope) > 0.000001)  {
            tangent_slope = 1.0 / tangent_slope;
          }
          // If dx/dy is zero, then assign a very large slope (essentially vertical line):
          else  {
            tangent_slope = 100000.0;
          }  // End of if/else-block for |tangent_slope| > 0

          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)  y_versus_x is FALSE, and final tangent_slope = %.9f\n",
                   omp_get_thread_num(), tangent_slope);
          }
          #endif

        }  // End of else-block (y_versus_x == FALSE)


        // Calculate unit-vector that is normal to the tangent slope.
        // Check if slope is zero:
        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) fabs(tangent_slope) is %5.3f\n", omp_get_thread_num(), fabs(tangent_slope));
        }
        #endif
        if (fabs(tangent_slope) > 0.000001)  {
          // Slope is non-zero, so calculate the slope of the line perpendicular to
          // to the tangent. This perpendicular slope is the negative reciprocal of
          // the tangent slope:
          normal_slope = -1.0/tangent_slope;

          // Calculate the x- and y-components of the unit vector along the normal slope:
          X_unit_vector = 1.0/sqrt(normal_slope * normal_slope   +   1);
          Y_unit_vector = normal_slope * X_unit_vector;

          // Calculate the signs of the x- and y-components of the unit-vector:
          X_unit_vector_sign = X_unit_vector / fabs(X_unit_vector);
          Y_unit_vector_sign = Y_unit_vector / fabs(Y_unit_vector);
        }  // End of if-block for non-zero value of 'tangent_slope'

        else  {
          // Slope is zero, so unit-normal line is a vertical line:
          X_unit_vector = 0.0;
          Y_unit_vector = 1.0;

          // Calculate the signs of the x- and y-components of the unit-vector. The sign of
          // the X-unit-vector, is arbitrarily assigned to be positive 1.0.
          X_unit_vector_sign = 1.0;
          Y_unit_vector_sign = 1.0;
        }  // End of else-block, in which tangent_slope is essentially zero

        // Calculate the angle between the current unit vector and the previous
        // unit vector. The angle should be less than 90 degrees (pi/2 radians),
        // but will exceed 90 degrees if the polarity flips for the unit
        // normal vector (because of how the quadratic coefficients are
        // calculated).
        normalDotProduct = X_unit_vector * prev_X_unit_vector  +  Y_unit_vector * prev_Y_unit_vector;
        if (normalDotProduct > 1.0)   // Due to rounding, dot product's magnitude might exceed unity.
          normalDotProduct = 1.0;     // Fix this before calculating the arc-cosine in order to avoid
        if (normalDotProduct < -1.0)  // an illegal argument of the 'acosf()' function below.
          normalDotProduct = -1.0;
        normalAngle = acosf(normalDotProduct);

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) previous unit vector = (%.5f, %.5f), current unit vector = (%.5f, %.5f)\n",
                 omp_get_thread_num(), prev_X_unit_vector, prev_Y_unit_vector, X_unit_vector, Y_unit_vector);
          printf("DEBUG: (thread %2d) normalDotProduct = %.9f, normalAngle = %.9f, PI/2 = %.9f\n", omp_get_thread_num(),
                 normalDotProduct, normalAngle, M_PI/2);
        }
        #endif


        // If 'angle' is less than 90°, then the polarity/sense of the shoulder points
        // is the same as the previous segment. But if angle > 90°, then we need to
        // flip the sense of the 'shoulder_*_polarity' variables:
        if (normalAngle > M_PI/2) {
          int temp = shoulder_1_polarity;
          shoulder_1_polarity = shoulder_2_polarity;
          shoulder_2_polarity = temp;
          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Swapping polarity/sense of two shoulder-nets in function 'createDiffPairShoulderPoints' at pseudo-path coordinate (%d,%d,%d)\n",
                   omp_get_thread_num(), fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
          }
          #endif
        }  // End of if-block for normalAngle exceeding 90 degrees

        // Get the pitch for these two diff-pair nets. This pitch
        // depends on the net number and the (x,y,z) coordinate:
        DR_set_number = cellInfo[fullPseudoPathCoords[i].X][fullPseudoPathCoords[i].Y][fullPseudoPathCoords[i].Z].designRuleSet;
        DR_subset = user_inputs->designRuleSubsetMap[pseudoNetNumber][DR_set_number];
        halfPitchCells = 0.5 * user_inputs->designRules[DR_set_number][DR_subset].diffPairPitchCells[TRACE];


        // Using the unit vectors and polarities calculated above, now calculate the
        // coordinates of the two proposed shoulder-points on either side of the current
        // segment of the pseudo-net. These points are located a half-pitch from the pseudo-net:

        //
        // Start of block to calculate coordinates of the two proposed shoulder-points
        //
        {

          // Define a counter for the number of adjustments are necessary to keep shoulder-points
          // out of illegal cells. This should be rare, but the counter prevents an infinite
          // loop in the do/while-loop:
          int adjustment_counter = 0;
          const int max_adjustments = 10;

          // Define Boolean variables for recording whether the calculated shoulder-points
          // are located in legal positions within the map:
          int point_1_is_legal = TRUE;
          int point_2_is_legal = TRUE;

          // Define offsets (in units of cells) that both shoulder-points will be shifted if
          // one of the points is found to be located in an illegal position in the map:
          float point_1_asymmetry_cells = 0.0;
          float point_2_asymmetry_cells = 0.0;

          do {   // Start of do/while-loop for (!point_1_is_legal || !point_2_is_legal)

            // Initialize Boolean variables to TRUE for both shoulder-points. They will be negated
            // if points are found to be illegal:
            point_1_is_legal = TRUE;
            point_2_is_legal = TRUE;

            // Calculate provisional coordinates for shoulder-path #1:
            x_1 = fullPseudoPathCoords[i].X
                   + shoulder_1_polarity * (int)round(X_unit_vector * halfPitchCells  +  X_unit_vector_sign * point_1_asymmetry_cells);
            y_1 = fullPseudoPathCoords[i].Y
                   + shoulder_1_polarity * (int)round(Y_unit_vector * halfPitchCells  +  Y_unit_vector_sign * point_1_asymmetry_cells);
            z_1 = fullPseudoPathCoords[i].Z;

            // Calculate coordinates for shoulder-path #2:
            x_2 = fullPseudoPathCoords[i].X
                   + shoulder_2_polarity * (int)round(X_unit_vector * halfPitchCells  +  X_unit_vector_sign * point_2_asymmetry_cells);
            y_2 = fullPseudoPathCoords[i].Y
                   + shoulder_2_polarity * (int)round(Y_unit_vector * halfPitchCells  +  Y_unit_vector_sign * point_2_asymmetry_cells);
            z_2 = fullPseudoPathCoords[i].Z;


            #ifdef DEBUG_createDiffPairShoulderPoints
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) In createDiffPairShoulderPoints for pseudo-net segment %d at (%d,%d,%d):\n",
                     omp_get_thread_num(), i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
              printf("       (thread %2d)   DR set = %d, DR subset = %d, diff-pair half-pitch = %5.2f microns\n", omp_get_thread_num(), DR_set_number, DR_subset, halfPitchMicrons);
              printf("       (thread %2d)   shoulder-1 polarity = %d, shoulder-2 polarity = %d\n", omp_get_thread_num(), shoulder_1_polarity, shoulder_2_polarity);
              printf("       (thread %2d)   X unit vector = %6.3f microns, Y unit vector = %6.3f microns\n", omp_get_thread_num(), X_unit_vector, Y_unit_vector);
              printf("       (thread %2d)   roundUp = %5.2f\n", omp_get_thread_num(), roundUp);
              printf("       (thread %2d)   cell size = %5.2f microns\n", omp_get_thread_num(), micronsPerCell);
            }
            #endif

            // Check that the provisional coordinates for shoulder-path #1 are not
            // outside the map:
            if (  (x_1 < 0) || (x_1 >= mapInfo->mapWidth)
               || (y_1 < 0) || (y_1 >= mapInfo->mapHeight)
               || (z_1 < 0) || (z_1 >= mapInfo->numLayers))  {

              printf("\n\nWARNING: (thread %2d) An unexpected situation occurred in function 'createDiffPairShoulderPoints'. Provisional shoulder-point (%d,%d,%d)\n",
                      omp_get_thread_num(), x_1, y_1, z_1);
              printf(    "WARNING: (thread %2d) for path %d is outside the bounds of the map. The corresponding pseudo-path coordinate is (%d,%d,%d).\n",
                      omp_get_thread_num(), path_1_number, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
              printf(    "WARNING: (thread %2d) The program will shift both shoulder-points to correct this rare situation.\n\n", omp_get_thread_num());

              // Flag point-1 as being in an illegal location
              point_1_is_legal = FALSE;

            }  // End of if-block for checking if (x_1, y_1, z_1) is outside of map

            #ifdef DEBUG_createDiffPairShoulderPoints
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Checking whether provisional shoulder-point (%d,%d,%d) for path %d is outside of the map.\n",
                     omp_get_thread_num(), x_2, y_2, z_2, path_2_number);
            }
            #endif

            // Check that the provisional coordinates for shoulder-path #2 are not
            // outside the map:
            if (  (x_2 < 0) || (x_2 >= mapInfo->mapWidth)
               || (y_2 < 0) || (y_2 >= mapInfo->mapHeight)
               || (z_2 < 0) || (z_2 >= mapInfo->numLayers))  {

              printf("\n\nWARNING: (thread %2d) An unexpected situation occurred in function 'createDiffPairShoulderPoints'. Provisional shoulder-point (%d,%d,%d)\n",
                      omp_get_thread_num(), x_2, y_2, z_2);
              printf(    "WARNING: (thread %2d) for path %d is outside the bounds of the map. The corresponding pseudo-path coordinate is (%d,%d,%d).\n",
                      omp_get_thread_num(), path_1_number, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
              printf(    "WARNING: (thread %2d) The program will shift both shoulder-points to correct this rare situation.\n\n", omp_get_thread_num());

              // Flag point-2 as being in an illegal location
              point_2_is_legal = FALSE;

            }  // End of if-block for checking if (x_2, y_2, z_2) is outside of map

            #ifdef DEBUG_createDiffPairShoulderPoints
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Pseudo-net segment #%d (%d,%d,%d):\n", omp_get_thread_num(), i, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
              printf("       (thread %2d)    provisional shoulder #1 segment #%d is (%d,%d,%d)\n", omp_get_thread_num(), i_1, x_1, y_1, z_1);
              printf("       (thread %2d)                         swap zone = %d\n", omp_get_thread_num(), cellInfo[x_1][y_1][z_1].swap_zone);
              printf("       (thread %2d)           forbidden trace barrier = %d\n", omp_get_thread_num(), cellInfo[x_1][y_1][z_1].forbiddenTraceBarrier);
              printf("       (thread %2d)                 barrier proximity = %d\n", omp_get_thread_num(),
                     get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x_1, y_1, z_1, path_1_number, TRACE));
              printf("       (thread %2d)                pin-swap proximity = %d\n", omp_get_thread_num(),
                     get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x_1, y_1, z_1, path_1_number, TRACE));
              printf("       (thread %2d)   provisional shoulder #2 segment #%d is (%d,%d,%d)\n", omp_get_thread_num(), i_2, x_2, y_2, z_2);

              printf("       (thread %2d)                         swap zone = %d\n", omp_get_thread_num(), cellInfo[x_2][y_2][z_2].swap_zone);
              printf("       (thread %2d)           forbidden trace barrier = %d\n", omp_get_thread_num(), cellInfo[x_2][y_2][z_2].forbiddenTraceBarrier);
              printf("       (thread %2d)                 barrier proximity = %d\n", omp_get_thread_num(),
                     get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x_2, y_2, z_2, path_2_number, TRACE));
              printf("       (thread %2d)                pin-swap proximity = %d\n", omp_get_thread_num(),
                     get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x_2, y_2, z_2, path_2_number, TRACE));
              printf("       (thread %2d) ------------------\n", omp_get_thread_num());
            }
            #endif


            // Check that the provisional coordinates for shoulder-path #1 are not
            // in a user-defined barrier.
            #ifdef DEBUG_createDiffPairShoulderPoints
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Checking whether provisional shoulder-point (%d,%d,%d) for path %d is in a 'forbiddenTraceBarrier'.\n",
                      omp_get_thread_num(), x_1, y_1, z_1, path_1_number);
            }
            #endif
            if (point_1_is_legal && cellInfo[x_1][y_1][z_1].forbiddenTraceBarrier) {
              printf("\n\nWARNING: (thread %2d) Cell at (%d,%d,%d) of shoulder path %d (#1) is within a user-defined barrier.\n",
                     omp_get_thread_num(), x_1, y_1, z_1, path_1_number);
              printf(    "WARNING: (thread %2d) The corresponding pseudo-path coordinate is (%d,%d,%d). This behavior is not expected.\n",
                     omp_get_thread_num(), fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
              printf(    "WARNING: (thread %2d) The program will shift both shoulder-points to correct this rare situation.\n\n",
                     omp_get_thread_num());

              // Flag point-1 as being in an illegal location
              point_1_is_legal = FALSE;

            }  // End of if-block

            // Check that the provisional coordinates for shoulder-path #2 are not
            // in a user-defined barrier.
            #ifdef DEBUG_createDiffPairShoulderPoints
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Checking whether provisional shoulder-point (%d,%d,%d) for path %d is in a 'forbiddenTraceBarrier'.\n",
                      omp_get_thread_num(), x_2, y_2, z_2, path_2_number);
            }
            #endif
            if (point_2_is_legal && cellInfo[x_2][y_2][z_2].forbiddenTraceBarrier) {
              printf("\n\nWARNING: (thread %2d) Cell at (%d,%d,%d) of shoulder path %d (#2) is within a user-defined barrier.\n",
                     omp_get_thread_num(), x_2, y_2, z_2, path_2_number);
              printf(    "WARNING: (thread %2d) The corresponding pseudo-path coordinate is (%d,%d,%d). This behavior is not expected.\n",
                     omp_get_thread_num(), fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
              printf(    "WARNING: (thread %2d) The program will shift both shoulder-points to correct this rare situation.\n\n",
                     omp_get_thread_num());

              // Flag point-2 as being in an illegal location
              point_2_is_legal = FALSE;

            }  // End of if-block

            // Check whether one or both provisional shoulder-points were found to be located in legal
            // or illegal locations in the map:
            if (!point_1_is_legal && !point_2_is_legal)  {
              // If both shoulder-points are in illegal positions, there is nothing that this function
              // can do, so we issue a fatal error-message and exit the program:

              printf("\n\nERROR: (thread %2d) Cell at (%d,%d,%d) of shoulder path %d (#1) is in or near a user-defined barrier.\n",
                     omp_get_thread_num(), x_1, y_1, z_1, path_1_number);
              printf("\n\nERROR: (thread %2d) Cell at (%d,%d,%d) of shoulder path %d (#2) is ALSO in or near a user-defined barrier.\n",
                     omp_get_thread_num(), x_2, y_2, z_2, path_2_number);
              printf(    "ERROR: (thread %2d) The corresponding pseudo-path coordinate is (%d,%d,%d). This behavior is not expected.\n",
                     omp_get_thread_num(), fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
              printf(    "ERROR: (thread %2d) Please inform the software developer of this fatal error.\n\n",
                     omp_get_thread_num());
              exit(1);
            }
            else  if (! point_1_is_legal  &&  point_2_is_legal)  {
              // If point-1 is located in an illegal position, but point-2 is legal, then we will re-calculate
              // the locations after shifting the cells' positions so point-1 is closer to the pseudo-path
              // and point-2 is farther:
              point_1_asymmetry_cells -= 0.3;  // Subtract 0.3 cells from the distance that point-1 is located from pseudo-path
              point_2_asymmetry_cells += 0.3;  // Add 0.3 cells to the distance that point-2 is located from pseudo-path

              adjustment_counter++;  // Increment the counter, which prevents an infinite do/while-loop

              printf("DEBUG: (thread %2d) Only point-1 is illegal. point_1_assymmetry_cells decreased to %.2f. point_2_assymmetry_cells increased to %.2f.\n",
                     omp_get_thread_num(), point_1_asymmetry_cells, point_2_asymmetry_cells);
              printf("DEBUG: (thread %2d) Counter increased to %d. Unit vect: (%.2f, %.2f). shoulder_1_polarity = %d. shoulder_2_polarity = %d.\n",
                     omp_get_thread_num(), adjustment_counter, X_unit_vector, Y_unit_vector, shoulder_1_polarity, shoulder_2_polarity);
              printf("DEBUG: (thread %2d) X_unit_vector_sign = %2f. Y_unit_vector_sign = %2f.\n",
                     omp_get_thread_num(), X_unit_vector_sign, Y_unit_vector_sign);
            }
            else  if (point_1_is_legal  &&  ! point_2_is_legal)  {
              // If point-2 is located in an illegal position, but point-1 is legal, then we will re-calculate
              // the locations after shifting the cells' positions so point-2 is closer to the pseudo-path
              // and point-1 is farther:
              point_1_asymmetry_cells += 0.3;  // Add 0.3 cells to the distance that point-1 is located from pseudo-path
              point_2_asymmetry_cells -= 0.3;  // Subtract 0.3 cells from the distance that point-2 is located from pseudo-path

              adjustment_counter++;  // Increment the counter, which prevents an infinite do/while-loop

              printf("DEBUG: (thread %2d) Only point-2 is illegal. point_1_assymmetry_cells increased to %.2f. point_2_assymmetry_cells decreased to %.2f.\n",
                     omp_get_thread_num(), point_1_asymmetry_cells, point_2_asymmetry_cells);
              printf("DEBUG: (thread %2d) Counter increased to %d. Unit vect: (%.2f, %.2f). shoulder_1_polarity = %d. shoulder_2_polarity = %d.\n",
                     omp_get_thread_num(), adjustment_counter, X_unit_vector, Y_unit_vector, shoulder_1_polarity, shoulder_2_polarity);
              printf("DEBUG: (thread %2d) X_unit_vector_sign = %2f. Y_unit_vector_sign = %2f.\n",
                     omp_get_thread_num(), X_unit_vector_sign, Y_unit_vector_sign);
            }

            if (adjustment_counter > max_adjustments)  {
              printf("\n\nERROR: (thread %2d) Function createDiffPairShouldPoints failed to find legal locations for diff-pair shoulder-points\n",
                     omp_get_thread_num());
              printf(    "ERROR: (thread %2d) after %d attempts around (%d,%d,%d) of pseudo-path %d. Please inform the software developer\n",
                     omp_get_thread_num(), max_adjustments, fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z,
                     pseudoNetNumber);
              printf(    "ERROR: (thread %2d) of this fatal error message.\n\n", omp_get_thread_num());
              exit(1);
            }  // End of if-block for adjustment_counter exceeding 10

          } while (!point_1_is_legal || !point_2_is_legal); // End of do-while

        }
        //
        // The above line is the end of the block to calculate coordinates of the
        // two proposed shoulder-points
        //


      }  // End of if-block for (calculateShoulderPoints == TRUE)

      else if (reUsePseudoCoordinates)  {
        //
        // Copy the coordinates of the pseudo-path segment over to both
        // shoulder-path segments:
        //
        x_1 = x_2 = fullPseudoPathCoords[i].X;
        y_1 = y_2 = fullPseudoPathCoords[i].Y;
        z_1 = z_2 = fullPseudoPathCoords[i].Z;
      }  // End of if/else block for (reUsePseudoCoordinates == TRUE)

      //
      // Decide whether to use the calculated shoulder points. Some points will be discarded
      // because they back-track when the pseudo-net makes a sharp turn.
      //
      if (doNotDiscard)  {
        // For terminal points and via points, automatically place the calculated or copied
        // shoulder-points into the path_*_coords[] arrays and increment the counters
        // for the segments in each shoulder path if:
        //    => This is the first or the last segment of the pseudo-net, or
        //    => This segment is associated with a layer-transition (via)

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Automatically putting shoulder-points into shoulder arrays because 'doNotDiscard' is TRUE:\n", omp_get_thread_num());
          printf("       (thread %2d)   At i = %d, i_1 = %d (%d,%d,%d)      i_2 = %d (%d,%d,%d)\n", omp_get_thread_num(), i, i_1, x_1, y_1, z_1, i_2, x_2, y_2, z_2);
        }
        #endif


        // Add segment to shoulder-path #1, but only if the segment is not in a proximity-zone:
        if (!  (get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x_1, y_1, z_1, path_1_number, TRACE)
            ||  get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x_1, y_1, z_1, path_1_number, TRACE)))  {

          pathCoords[path_1_number][i_1].X    = x_1;
          pathCoords[path_1_number][i_1].Y    = y_1;
          pathCoords[path_1_number][i_1].Z    = z_1;
          pathCoords[path_1_number][i_1].flag = FALSE;

          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Added segment #%d point (%d,%d,%d) to shoulder path #1 (path %d)\n", omp_get_thread_num(), i_1,
                    pathCoords[path_1_number][i_1].X,  pathCoords[path_1_number][i_1].Y,  pathCoords[path_1_number][i_1].Z, path_1_number);
          }
          #endif

          i_1++;  // Increment counter for number of segments in shoulder path #1
        }

        // Add segment to shoulder-path #2, but only if the segment is not in a proximity-zone:
        if (!  (get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x_2, y_2, z_2, path_2_number, TRACE)
            ||  get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x_2, y_2, z_2, path_2_number, TRACE)))  {
          pathCoords[path_2_number][i_2].X    = x_2;
          pathCoords[path_2_number][i_2].Y    = y_2;
          pathCoords[path_2_number][i_2].Z    = z_2;
          pathCoords[path_2_number][i_2].flag = FALSE;

          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Added segment #%d point (%d,%d,%d) to shoulder path #2 (path %d)\n", omp_get_thread_num(), i_2,
                    pathCoords[path_2_number][i_2].X,  pathCoords[path_2_number][i_2].Y,  pathCoords[path_2_number][i_2].Z, path_2_number);
          }
          #endif

          i_2++;  // Increment counter for number of segments in shoulder path #2
        }

      }  // End of if-block for first/last segment of pseudo-net

      else  {

        // For all other pseudo-path segments, determine whether each provisional shoulder-path
        // segment should be used or skipped.
        int use_provisional_segment_1 = TRUE;
        int use_provisional_segment_2 = TRUE;

        // Create some preliminary variables that will help us determine whether to keep the
        // provisional coordinates for shoulder-paths #1 and #2:
        delX   = fullPseudoPathCoords[i].X - fullPseudoPathCoords[i-1].X;
        delY   = fullPseudoPathCoords[i].Y - fullPseudoPathCoords[i-1].Y;
        delX_1 = x_1 - prev_x_1;
        delY_1 = y_1 - prev_y_1;
        delX_2 = x_2 - prev_x_2;
        delY_2 = y_2 - prev_y_2;

        #ifdef DEBUG_createDiffPairShoulderPoints
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) pseudo-path segment #%d: x_1=%d, y_1=%d, x_2=%d, y_2=%d, prev_x_1=%d, prev_y_1=%d, prev_x_2=%d, prev_y_2=%d\n",
                 omp_get_thread_num(), i, x_1, y_1, x_2, y_2, prev_x_1, prev_y_1, prev_x_2, prev_y_2);
          printf("DEBUG: (thread %2d) Changes from previous shoulder-path points: delX_1 = %d, delY_1 = %d,      delX_2=%d, delY_2=%d\n",
                 omp_get_thread_num(), delX_1, delY_1, delX_2, delY_2);
        }
        #endif

        //
        // Check whether provisional shoulder-segment #1 is in a proximity zone:
        //
        if (get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x_1, y_1, z_1, path_1_number, TRACE))  {
          use_provisional_segment_1 = FALSE;
        }
        else if (get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x_1, y_1, z_1, path_1_number, TRACE))  {
          use_provisional_segment_1 = FALSE;
        }
        // Check whether provisional coordinate for shoulder #1 is different than previous coordinate:
        else if (abs(delX_1) + abs(delY_1) == 0)  {
          use_provisional_segment_1 = FALSE;
        }

        // Check whether provisional coordinate for shoulder #1 is in different design-rule zone
        // from the shoulder-path coordinate:
        else if (DR_set_number != cellInfo[x_1][y_1][z_1].designRuleSet)  {
          use_provisional_segment_1 = FALSE;
        }

        else  {
          // Calculate angle between the proposed segment for shoulder
          // point #1 and the current pseudo-net segment:
          arccos_arg = (double)(delX * delX_1  +  delY * delY_1)/(sqrt(delX*delX + delY*delY) * sqrt(delX_1*delX_1 + delY_1*delY_1));
          if (arccos_arg > 1.0)
            arccos_arg = 1.0;
          if (arccos_arg < -1.0)
            arccos_arg = -1.0;
          angle_1 = acosf(arccos_arg);
          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) angle_1 = %6.3f degrees.\n", omp_get_thread_num(), angle_1 * 180/M_PI);
          }
          #endif

          // Check if the angle is too large to keep the proposed shoulder point. The threshold is
          // 20 degrees (pi/9, in radians):
          if (angle_1 > M_PI/9)  {
            use_provisional_segment_1 = FALSE;

            // Because the shoulder-path and the pseudo-path are pointing in different directions (by more than
            // 20 degrees), we also delete the preceding shoulder-path point, as this point is likely too close
            // to the diff-pair partner-path.
            if (i_1 > 1)  {
              i_1--;
            }
          }
          else  {
            // We got here, so this provisional segment has not been eliminated. We next check
            // whether any cells between this segment and the corresponding pseudo-segment are
            // in a pin-swap zone or in close proximity to such zones. Either one of these
            // would disqualify the provisional segment from being created:

            // Calculate the X- and Y-components of the vector between the pseudo-path segment
            // and the provisional shoulder-path segment:
            int x_vector = x_1 - fullPseudoPathCoords[i].X;
            int y_vector = y_1 - fullPseudoPathCoords[i].Y;

            // Calculate the number of points to check between the pseudo-path segment and the
            // provisional shoulder-path segment:
            int num_steps = max(abs(x_vector), abs(y_vector));

            // Iterate over the points between the pseudo-path segment and the provisional
            // shoulder-path segment:
            for (int j = 0; j <= num_steps; j++)  {
              // Calculate intermediate (x,y) coordinates between the pseudo-path segment and
              // provisional shoulder-path segment:
              int x_intermediate = fullPseudoPathCoords[i].X + round(((float)j / (float)num_steps) * x_vector);
              int y_intermediate = fullPseudoPathCoords[i].Y + round(((float)j / (float)num_steps) * y_vector);

              // If the intermediate (x,y) point is in or near a pin-swap zone, then clear the
              // 'use_provisional_segment_1' flag so the provisional shoulder-path segment will not be created.
              if (   cellInfo[x_intermediate][y_intermediate][z_1].swap_zone
                  || get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x_intermediate, y_intermediate, z_1, path_1_number, TRACE))  {

                // We got here, so intermediate cell is in/near a pin-swap zone. Clear the 'use_provisional_segment' variable
                // so this provisional segment will not be created:
                use_provisional_segment_1 = FALSE;

                #ifdef DEBUG_createDiffPairShoulderPoints
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d) For shoulder-path #1 (path #%d), provisional shoulder-path segment (%d,%d,%d) is discarded because\n",
                          omp_get_thread_num(), path_1_number, x_1, y_1, z_1);
                  printf("       (thread %2d) swap-zone or swap-zone-proximity was found at (%d,%d,%d), which is between the provisional shoulder-\n",
                          omp_get_thread_num(), x_intermediate, y_intermediate, z_1);
                  printf("       (thread %2d) path segment and the associated pseudo-path segment at (%d,%d,%d)\n", omp_get_thread_num(),
                          fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
                }
                #endif

                break;  // Break out of for-loop for index 'j'
              }
            }  // End of for-loop for index 'j'
          }  // End of else-block for angle < 20 degrees
        }  // End of if/else-blocks for deciding whether to use provisional segment for shoulder-path #1

        //
        // If the provisional segment for shoulder-path #1 passed all the above tests, then
        // add it to the pathCoords array and increment the 'i_1' counter:
        if (use_provisional_segment_1)  {

          pathCoords[path_1_number][i_1].X    = x_1;
          pathCoords[path_1_number][i_1].Y    = y_1;
          pathCoords[path_1_number][i_1].Z    = z_1;
          pathCoords[path_1_number][i_1].flag = FALSE;

          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Added segment #%d point (%d,%d,%d) to shoulder path #1.\n", omp_get_thread_num(),
                    i_1,  pathCoords[path_1_number][i_1].X,  pathCoords[path_1_number][i_1].Y,  pathCoords[path_1_number][i_1].Z);
          }
          #endif

          i_1++;  // Increment counter for number of segments in shoulder path #1
        }  // End of if-block for use_provisional_segment_1 == TRUE

        //
        // Now repeat for shoulder-path #2: Check whether provisional shoulder-segment #2 is in a proximity zone:
        //
        if (get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x_2, y_2, z_2, path_2_number, TRACE))  {
          use_provisional_segment_2 = FALSE;
        }
        else if (get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x_2, y_2, z_2, path_2_number, TRACE))  {
          use_provisional_segment_2 = FALSE;
        }
        // Check whether provisional coordinate for shoulder #2 is different than previous coordinate:
        else if (abs(delX_2) + abs(delY_2) == 0)  {
          use_provisional_segment_2 = FALSE;
        }

        // Check whether provisional coordinate for shoulder #2 is in different design-rule zone
        // from the shoulder-path coordinate:
        else if (DR_set_number != cellInfo[x_2][y_2][z_2].designRuleSet)  {
          use_provisional_segment_2 = FALSE;
        }

        else  {
          // Calculate angle between the proposed segment for shoulder
          // point #2 and the current pseudo-net segment:
          arccos_arg = (double)(delX * delX_2  +  delY * delY_2)/(sqrt(delX*delX + delY*delY) * sqrt(delX_2*delX_2 + delY_2*delY_2));
          if (arccos_arg > 1.0)
            arccos_arg = 1.0;
          if (arccos_arg < -1.0)
            arccos_arg = -1.0;
          angle_2 = acosf(arccos_arg);
          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) angle_2 = %6.3f degrees.\n", omp_get_thread_num(), angle_2 * 180/M_PI);
          }
          #endif

          // Check if angle too large to keep the proposed shoulder point. The threshold is
          // 20 degrees (pi/9, in radians):
          if (angle_2 > M_PI/9)  {
            use_provisional_segment_2 = FALSE;

            // Because the shoulder-path and the pseudo-path are pointing in different directions (by more than
            // 20 degrees), we also delete the preceding shoulder-path point, as this point would likely be too close
            // to the diff-pair partner-path.
            if (i_2 > 1)  {
              i_2--;
            }

          }
          else  {
            // We got here, so this provisional segment has not been eliminated. We next check
            // whether any cells between this segment and the corresponding pseudo-segment are
            // in a pin-swap zone or in close proximity to such zones. Either one of these
            // would disqualify the provisional segment from being created:

            // Calculate the X- and Y-components of the vector between the pseudo-path segment
            // and the provisional shoulder-path segment:
            int x_vector = x_2 - fullPseudoPathCoords[i].X;
            int y_vector = y_2 - fullPseudoPathCoords[i].Y;

            // Calculate the number of points to check between the pseudo-path segment and the
            // provisional shoulder-path segment:
            int num_steps = max(abs(x_vector), abs(y_vector));

            // Iterate over the points between the pseudo-path segment and the provisional
            // shoulder-path segment:
            for (int j = 0; j <= num_steps; j++)  {
              // Calculate intermediate (x,y) coordinates between the pseudo-path segment and
              // provisional shoulder-path segment:
              int x_intermediate = fullPseudoPathCoords[i].X + round(((float)j / (float)num_steps) * x_vector);
              int y_intermediate = fullPseudoPathCoords[i].Y + round(((float)j / (float)num_steps) * y_vector);

              // If the intermediate (x,y) point is in or near a pin-swap zone, then clear the
              // 'use_provisional_segment_2' flag so the provisional shoulder-path segment will not be created.
              if (   cellInfo[x_intermediate][y_intermediate][z_2].swap_zone
                  || get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x_intermediate, y_intermediate, z_2, path_2_number, TRACE))  {

                // We got here, so intermediate cell is in/near a pin-swap zone. Clear the 'use_provisional_segment' variable
                // so this provisional segment will not be created:
                use_provisional_segment_2 = FALSE;

                #ifdef DEBUG_createDiffPairShoulderPoints
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d) For shoulder-path #2 (path #%d), provisional shoulder-path segment (%d,%d,%d) is discarded because\n",
                          omp_get_thread_num(), path_2_number, x_2, y_2, z_2);
                  printf("       (thread %2d) swap-zone or swap-zone-proximity was found at (%d,%d,%d), which is between the provisional shoulder-\n",
                          omp_get_thread_num(), x_intermediate, y_intermediate, z_2);
                  printf("       (thread %2d) path segment and the associated pseudo-path segment at (%d,%d,%d)\n", omp_get_thread_num(),
                          fullPseudoPathCoords[i].X, fullPseudoPathCoords[i].Y, fullPseudoPathCoords[i].Z);
                }
                #endif

                break;  // Break out of for-loop for index 'j'
              }
            }  // End of for-loop for index 'j'
          }  // End of else-block for angle < 20 degrees
        }  // End of if/else-blocks for deciding whether to use provisional segment for shoulder-path #2

        //
        // If the provisional segment for shoulder-path #2 passed all the above tests, then
        // add it to the pathCoords array and increment the 'i_2' counter:
        if (use_provisional_segment_2)  {

          pathCoords[path_2_number][i_2].X    = x_2;
          pathCoords[path_2_number][i_2].Y    = y_2;
          pathCoords[path_2_number][i_2].Z    = z_2;
          pathCoords[path_2_number][i_2].flag = FALSE;

          #ifdef DEBUG_createDiffPairShoulderPoints
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) Added segment #%d point (%d,%d,%d) to shoulder path #2.\n", omp_get_thread_num(),
                    i_2,  pathCoords[path_2_number][i_2].X,  pathCoords[path_2_number][i_2].Y,  pathCoords[path_2_number][i_2].Z);
          }
          #endif

          i_2++;  // Increment counter for number of segments in shoulder path #2
        }  // End of if-block for use_provisional_segment_2 == TRUE

      }  // End of else-block (i.e., i>0 and i<pathLength[pseudoNetNumber])


      // Re-initialize the 'previous' provisional coordinates; we'll re-use them to
      // calculate angles in the next segment:
      prev_x_1 = x_1;
      prev_y_1 = y_1;
      prev_x_2 = x_2;
      prev_y_2 = y_2;


      // Re-initialize the 'previous' unit vectors before moving on to the next segment:
      prev_X_unit_vector = X_unit_vector;
      prev_Y_unit_vector = Y_unit_vector;

    }  // End of if-block for 'calculateShoulderPoints' == TRUE

    #ifdef DEBUG_createDiffPairShoulderPoints
    if (DEBUG_ON)  {
      if (i_1 > 0)  {
        printf("DEBUG: (thread %2d) Pseudo-net segment #%d (%d,%d,%d):         shoulder #1 segment #%d is (%d,%d,%d).\n", omp_get_thread_num(),
                i      , fullPseudoPathCoords[i].X,             fullPseudoPathCoords[i].Y,             fullPseudoPathCoords[i].Z,
                i_1 - 1, pathCoords[path_1_number][i_1 - 1].X,  pathCoords[path_1_number][i_1 - 1].Y,  pathCoords[path_1_number][i_1 - 1].Z);
      }
      if (i_2 > 0)  {
        printf("DEBUG: (thread %2d) Pseudo-net segment #%d (%d,%d,%d):         shoulder #2 segment #%d is (%d,%d,%d).\n", omp_get_thread_num(),
                i      , fullPseudoPathCoords[i].X,             fullPseudoPathCoords[i].Y,             fullPseudoPathCoords[i].Z,
                i_2 - 1, pathCoords[path_2_number][i_2 - 1].X,  pathCoords[path_2_number][i_2 - 1].Y,  pathCoords[path_2_number][i_2 - 1].Z);
      }
    }
    #endif

  }  // End of for-loop for index 'i' (0 to pathLength[pseudoNetNumber])

  // Free the temporary array used to hold the pseudo-net's coordinates, including
  // the net's starting coordinate:
  free(fullPseudoPathCoords);
  fullPseudoPathCoords = NULL;

  // Place the lengths of the diff-pair paths into the 'pathLength' variable:
  pathLength[path_1_number] = i_1;
  pathLength[path_2_number] = i_2;

  // Reallocate memory for the two diff-pair nets so their respective sizes are i_1 and i_2:
  pathCoords[path_1_number] = realloc(pathCoords[path_1_number], pathLength[path_1_number] * sizeof(Coordinate_t));
  if (pathCoords[path_1_number] == 0)  {
    printf("\n\nERROR: Failed to reallocate memory for pathCoords[%d].\n\n", path_1_number);
    exit(1);
  }
  pathCoords[path_2_number] = realloc(pathCoords[path_2_number], pathLength[path_2_number] * sizeof(Coordinate_t));
  if (pathCoords[path_2_number] == 0)  {
    printf("\n\nERROR: Failed to reallocate memory for pathCoords[%d].\n\n", path_2_number);
    exit(1);
  }

  #ifdef DEBUG_createDiffPairShoulderPoints
  if (DEBUG_ON)  {
    printf("\n\nDEBUG: (thread %2d) At end of createDiffPairShoulderPoints, path %d (#1) has %d segments:\n", omp_get_thread_num(), path_1_number, pathLength[path_1_number]);
    for (int i = 0; i < pathLength[path_1_number]; i++)  {
      printf("DEBUG: (thread %2d) Path %d (#1), segment %d: (%d,%d,%d)\n", omp_get_thread_num(), path_1_number, i, pathCoords[path_1_number][i].X, pathCoords[path_1_number][i].Y, pathCoords[path_1_number][i].Z);
    }
    printf("DEBUG: (thread %2d)\n\n", omp_get_thread_num());
    printf("\n\nDEBUG: (thread %2d) At end of createDiffPairShoulderPoints, path %d (#2) has %d segments:\n", omp_get_thread_num(), path_2_number, pathLength[path_2_number]);
    for (int i = 0; i < pathLength[path_2_number]; i++)  {
      printf("DEBUG: (thread %2d) Path %d (#2), segment %d: (%d,%d,%d)\n", omp_get_thread_num(), path_2_number, i, pathCoords[path_2_number][i].X, pathCoords[path_2_number][i].Y, pathCoords[path_2_number][i].Z);
    }
    printf("DEBUG: (thread %2d)\n\n", omp_get_thread_num());

    printf("DEBUG: (thread %2d) Exiting function 'createDiffPairShoulderPoints' with following path lengths:\n", omp_get_thread_num());
    printf("       (thread %2d)        Pseudo net #%d: %d segments\n", omp_get_thread_num(), pseudoNetNumber, pathLength[pseudoNetNumber]);
    printf("       (thread %2d)     Diff-pair net #%d: %d segments (path '1')\n", omp_get_thread_num(), path_1_number, pathLength[path_1_number]);
    printf("       (thread %2d)     Diff-pair net #%d: %d segments (path '2')\n", omp_get_thread_num(), path_2_number, pathLength[path_2_number]);
  }
  #endif

}  // End of function 'createDiffPairShoulderPoints'



//-----------------------------------------------------------------------------
// Name: calcUnitVectorToDiffPairVia
// Desc: Calculate a unit vector that points from the center of a pseudo-via
//       to one of the two diff-pair vias. The direction of this vector is
//       calculated from two pseudo-path points before the pseudo-via, and
//       from two pseudo-path points after the pseudo-via. All 4 of these points
//       are passed as parameters into the function.
//
//       If the before-via points have identical (x,y) coordinates, then the
//       unit vector is calculated based only on the after-via points.
//       Likewise, if the aftere-via points have identical (x,y) coordinates,
//       then the unit vector is calculated based only on the before-via points.
//-----------------------------------------------------------------------------

static Vector2dFloat_t calcUnitVectorToDiffPairVia(const Coordinate_t coord_1_before_via,
                                                   const Coordinate_t coord_2_before_via,
                                                   const Coordinate_t coord_1_after_via,
                                                   const Coordinate_t coord_2_after_via)  {

  // Define variable that will be returned from this function:
  Vector2dFloat_t unit_vector;

  // Calculate vector from coord_1_before_via to coord_2_before_via:
  int x_before = coord_2_before_via.X - coord_1_before_via.X;
  int y_before = coord_2_before_via.Y - coord_1_before_via.Y;

  // Calculate vector from coord_1_after_via to coord_2_after_via:
  int x_after  = coord_2_after_via.X - coord_1_after_via.X;
  int y_after  = coord_2_after_via.Y - coord_1_after_via.Y;

  // Define 3 Boolean flags that will determine which coordinates to use:
  int use_before_and_after_points = TRUE;
  int use_beforePoints_only       = FALSE;
  int use_afterPoints_only        = FALSE;

  //
  // Determine which points will be used for calculating the unit vector:
  //
  if ((x_before == 0) && (y_before == 0))  {
    use_afterPoints_only = TRUE;
    use_before_and_after_points = FALSE;
  }
  if ((x_after == 0) && (y_after == 0))  {
    use_beforePoints_only = TRUE;
    use_before_and_after_points = FALSE;
  }

  // Check for error condition, in which a unit vector cannot be calculated:
  if (use_beforePoints_only && use_afterPoints_only)  {
    printf("\n\nERROR: Function 'calcUnitVectorToDiffPairVia' received illegal input parameters that do not\n");
    printf(    "       allow any vectors to be calculated. The two points before the via are:\n");
    printf(    "           (%d,%d,%d) and (%d,%d,%d)\n", coord_1_before_via.X, coord_1_before_via.Y, coord_1_before_via.Z,
                                                         coord_2_before_via.X, coord_2_before_via.Y, coord_2_before_via.Z);
    printf(    "       The two points after the via are:\n");
    printf(    "           (%d,%d,%d) and (%d,%d,%d)\n", coord_1_after_via.X, coord_1_after_via.Y, coord_1_after_via.Z,
                                                         coord_2_after_via.X, coord_2_after_via.Y, coord_2_after_via.Z);
    printf(    "       Inform the software developer of this fatal error message.\n\n");
    exit(1);
  }

  // printf("DEBUG:   Function 'calcUnitVectorToDiffPairVia' received input vectors of (%d,%d) before the via, and (%d,%d) after the via.\n",
  //        x_before, y_before, x_after, y_after);

  // Check that the before-via points are on a different routing layer than the after-via points:
  if (use_before_and_after_points)  {
    if (coord_1_before_via.Z == coord_1_after_via.Z)  {
      printf("\n\nERROR: Function 'calcUnitVectorToDiffPairVia' received illegal input parameters in which the before-via\n");
      printf(    "       and after-via points are on the same routing layer. The two points before the via are:\n");
      printf(    "           (%d,%d,%d) and (%d,%d,%d)\n", coord_1_before_via.X, coord_1_before_via.Y, coord_1_before_via.Z,
                                                           coord_2_before_via.X, coord_2_before_via.Y, coord_2_before_via.Z);
      printf(    "       The two points after the via are:\n");
      printf(    "           (%d,%d,%d) and (%d,%d,%d)\n", coord_1_after_via.X, coord_1_after_via.Y, coord_1_after_via.Z,
                                                           coord_2_after_via.X, coord_2_after_via.Y, coord_2_after_via.Z);
      printf(    "       Inform the software developer of this fatal error message.\n\n");
    }
  }

  // Define variable to hold the angle between the positive X-axis and the
  // diff-pair via:
  float angle_to_diffPair_via;

  //
  // If the input parameters allow us to use the before-via points *AND* the
  // after-via points to calculate the angle, then do so:
  //
  if (use_before_and_after_points)  {

    // Calculate the angle between the before-via vector and the after-via vector:
    float angle_between_vectors = atan2(x_before*y_after - x_after*y_before, x_before*x_after + y_before*y_after);

    // printf("DEBUG:   Angle between two vectors is %5.2f degrees.\n", angle_between_vectors * 180.0 / M_PI);

    // If the angle between the before-via vector and the after-via vector is less than
    // 90 degress (PI/2), then the after-via vector is back-tracking towards the before-
    // via vector. In this case, we rotate the diff-pair vias by 90 degrees:
    float angle_between_beforeVector_and_via;
    if (fabs(angle_between_vectors) <= M_PI/2.0)  {
      // Calculate the angle to the diff-pair via, relative to the before-via vector.
      // This angle is 90 degrees plus half the angle between the two vectors:
      angle_between_beforeVector_and_via = angle_between_vectors / 2.0  +   M_PI/2.0;
    }  // End of if-block for angle < 90 degrees
    else  {
      // Calculate the angle to the diff-pair via, relative to the before-via vector.
      // This angle is half the angle between the two vectors:
      angle_between_beforeVector_and_via = angle_between_vectors / 2.0 ;
    }  // End of else-block

    // printf("DEBUG:   Angle between before-vector and via is %5.2f degrees.\n",
    //         angle_between_beforeVector_and_via * 180.0 / M_PI);

    // Calculate the angle to the via, relative to the fixed X/Y coordinate
    // system. We simply add the angle that the before-via vector makes with
    // the X-axis:
    float angle_of_beforeVia_vector = atan2(y_before, x_before);

    angle_to_diffPair_via = angle_of_beforeVia_vector + angle_between_beforeVector_and_via;
  }  // End of if-block for use_before_and_after_points == TRUE

  else  {
    // We got here, so we can use only the before-via points *OR* the
    // after-via points
    int delta_x, delta_y;
    if (use_beforePoints_only)  {  // We'll use the before-via points
      delta_x = x_before;
      delta_y = y_before;
    }
    else  {  // We'll use the after-via points
      delta_x = x_after;
      delta_y = y_after;
    }

    // Calculate the angle along the pseudo-path line, relative to the
    // positive x-axis:
    float angle_along_pseudo_path = atan2(delta_y, delta_x);

    // Calculate the angle to the via, relative to the fixed X/Y coordinate
    // system. We simply add 90 degrees to the angle that the pseudo-path
    // makes with the x-axis:
    angle_to_diffPair_via = angle_along_pseudo_path + M_PI/2.0;

  }  // End of else-block for use_before_and_after_points == FALSE

  // printf("DEBUG:   Angle between x-axis and via is %5.2f degrees.\n", angle_to_diffPair_via * 180.0 / M_PI);

  // Calculate the X- and Y-components of the unit vector, based on the
  // angle to the diff-pair via:
  unit_vector.X = cos(angle_to_diffPair_via);
  unit_vector.Y = sin(angle_to_diffPair_via);

    // Return the unit vector to the calling program:
  return(unit_vector);

}  // End of function 'calcUnitVectorToDiffPairVia'


//-----------------------------------------------------------------------------
// Name: calcUnitVectorToDiffPairVia_wrapper
// Desc: Calculate the 4 coordinates that will be passed into function
//       calcUnitVectorToDiffPairVia, which creates a unit-vector that points
//       in the direction from the pseudo-via to one of the associated diff-
//       pair vias. The four coordinates consist of:
//         (1) The start-segment of the pseudo-via.
//         (2) A pseudo-path segment that is found by starting from the pseudo-
//             via, we follow the pseudo-path backward until a segment is found
//             that either (a) is as far from the pseudo-via as the diff-pair via
//             half-pitch, or (b) is a start-terminal of the pseudo-path, or (c)
//             is the segment immediately after the pseudo-path exited a pin-swap
//             zone.
//         (3) The end-segment of the pseudo-via.
//         (4) A pseudo-path segment that is found by starting from the pseudo-via,
//             we follow the pseudo-path forward until a segment is found that
//             either (a) is as far from the pseudo-via as the diff-pair via
//             half-pitch, or (b) is an end-terminal of the pseudo-path.
//-----------------------------------------------------------------------------
//
// Define 'calcUnitVectorToDiffPairVia_wrapper' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_calcUnitVectorToDiffPairViaWrapper 1
#undef DEBUG_calcUnitVectorToDiffPairViaWrapper

Vector2dFloat_t calcUnitVectorToDiffPairVia_wrapper(int pseudoPathNum, int viaStartSegment,
                                                    int viaEndSegment, int pathLengths[],
                                                    Coordinate_t *pathCoords[], const MapInfo_t *mapInfo,
                                                    CellInfo_t ***cellInfo, float pseudoVia_to_diffPairVia_distance) {

  // Define variable that will be returned from this function:
  Vector2dFloat_t unit_vector;

  // Create a 'dummy' coordinate at (0,0,0) that is occasionally used for passing into function
  // calcUnitVectorToDiffPairVia() when we're analyzing vias at start- or end-terminals, in which
  // case there are no segments (respectively) before or after the via.
  Coordinate_t dummy_point;
  dummy_point.X    = 0;
  dummy_point.Y    = 0;
  dummy_point.Z    = 0;
  dummy_point.flag = FALSE;

  // Define the four coordinates that will be passed to function calcUnitVectorToDiffPairVia():
  Coordinate_t segment_1, segment_2, segment_3, segment_4;

  #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration >= 264) && (mapInfo->current_iteration <= 280) && (pseudoPathNum == 6))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in calcUnitVectorToDiffPairVia_wrapper() because specific requirements were met.\n",
           omp_get_thread_num());
    printf("DEBUG: (thread %2d)     pseudoPathNum = %d, viaStartSegment = %d, viaEndSegment = %d, pseudoVia_to_diffPairVia_distance = %.4f.\n\n",
           omp_get_thread_num(), pseudoPathNum, viaStartSegment, viaEndSegment, pseudoVia_to_diffPairVia_distance);
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif


  //
  // Calculate the before-via coordinates: segment_1 and segment_2:
  //
  if (viaStartSegment == -1)  {
    // The via is located at the start-terminal of the pseudo-path, so use dummy-points
    // for the before-via coordinates:
    segment_1 = copyCoordinates(dummy_point);
    segment_2 = copyCoordinates(dummy_point);

    #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d)  viaStartSegment = -1, so setting segment_1 and segment_2 to (0,0,0)\n", omp_get_thread_num());
    }
    #endif
  }
  else if (viaStartSegment == 0)  {
    // The via is located near the start-terminal of the pseudo-path, so
    // use the start-terminal as the earliest pseudo-path segment:
    segment_1 = copyCoordinates(pathCoords[pseudoPathNum][viaStartSegment]);
    segment_2 = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);

    #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d)  viaStartSegment = 0, so setting segment_2 to start-terminal, and segment_1 to via location\n",
             omp_get_thread_num());
    }
    #endif
  }
  else  {
    // The pseudo-via is not located near the pseudo-path's start-terminal, so trace
    // backwards from the pseudo-via until a segment is found that either (a) is as
    // far from the pseudo-via as the diff-pair via half-pitch, or (b) is a start-terminal
    // of the pseudo-path, or (c) is the segment immediately after the pseudo-path exited
    // a pin-swap zone.

    #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) viaStartSegment is not near the start-terminal, so tracing backwards from pseudo-via...\n",
             omp_get_thread_num());
    }
    #endif

    segment_1 = copyCoordinates(pathCoords[pseudoPathNum][viaStartSegment]);
    segment_2 = copyCoordinates(pathCoords[pseudoPathNum][viaStartSegment]);
    for (int segment = viaStartSegment - 1; segment >= 0; segment--)  {

      // Get the coordinates of the current segment:
      Coordinate_t current_segment = copyCoordinates(pathCoords[pseudoPathNum][segment]);
      #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)   Tracing back to segment %d (%d,%d,%d)...\n", omp_get_thread_num(), segment,
               current_segment.X, current_segment.Y, current_segment.Z);
      }
      #endif

      // If the current segment is in a swap-zone, then use the previous segment for segment_1:
      if (cellInfo[current_segment.X][current_segment.Y][current_segment.Z].swap_zone)  {
        #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)     Segment %d is in a swap-zone, so using previous segment.\n",
                 omp_get_thread_num(), segment);
        }
        #endif

        break;  // Break out of for-loop, thereby using prev_segment
      }

      // If the current segment is farther from the pseudo-via than the half-pitch of
      // the diff-pair vias, then use the current segment for segment_1:
      float distance_to_via = calc_2D_Pythagorean_distance_ints(current_segment.X, current_segment.Y,
                                                                pathCoords[pseudoPathNum][viaStartSegment].X,
                                                                pathCoords[pseudoPathNum][viaStartSegment].Y);
      #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)     Segment %d is %.1f cells from pseudo-via.\n",
               omp_get_thread_num(), segment, distance_to_via);
      }
      #endif

      if (distance_to_via > pseudoVia_to_diffPairVia_distance)  {

        #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)     Segment %d is located %.1f from via, which is farther than %.1f, so using this segment.\n",
                 omp_get_thread_num(), segment, distance_to_via, pseudoVia_to_diffPairVia_distance);
        }
        #endif

        segment_2 = copyCoordinates(current_segment);
        break;
      }

      // If we've reached segment #0 and none of the above criteria have been
      // satisfied, then use the start-segment of the pseudo-path for segment_1:
      if (segment == 0)  {
        segment_2 = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);

        #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)     We reached segment 0, so using start-terminal.\n",
                 omp_get_thread_num());
        }
        #endif

        break;
      }

      // In anticipation of the next iteration through this for-loop, update the
      // 'segment_1' variable to be the current iteration's current_segment:
      segment_2 = copyCoordinates(current_segment);
    }  // End of for-loop for index segment, tracking backwards
  }  // End of else-block for calculating segment_1 and segment_2

  #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) Found segment_1 = (%d,%d,%d) and segment_2 = (%d,%d,%d)\n\n", omp_get_thread_num(),
           segment_1.X, segment_1.Y, segment_1.Z, segment_2.X, segment_2.Y, segment_2.Z);
  }
  #endif

  //
  // Calculate the end-via coordinates: segment_3 and segment_4:
  //
  if (viaEndSegment == pathLengths[pseudoPathNum] - 1)  {
    // The via is located at the end-terminal of the pseudo-path, so use dummy points
    // for the after-via points:

    #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d)  viaStartSegment = %d (at end-terminal), so setting segment_3 and segment_3 to (0,0,0)\n",
             pathLengths[pseudoPathNum] - 1, omp_get_thread_num());
    }
    #endif

    segment_3 = copyCoordinates(dummy_point);
    segment_4 = copyCoordinates(dummy_point);
  }
  else  {
    // The via is not located at the end-terminal of the pseudo-path, so trace forward from
    // the pseudo-via until a segment is found that either (a) is as far from the pseudo-via
    // as the diff-pair via half-pitch, or (b) is an end-terminal of the pseudo-path.
    segment_3 = copyCoordinates(pathCoords[pseudoPathNum][viaEndSegment]);
    segment_4 = copyCoordinates(pathCoords[pseudoPathNum][viaEndSegment]);
    for (int segment = viaEndSegment + 1; segment < pathLengths[pseudoPathNum]; segment++)  {

      // Get the coordinates of the current segment:
      Coordinate_t current_segment = copyCoordinates(pathCoords[pseudoPathNum][segment]);

      float distance_to_via = calc_2D_Pythagorean_distance_ints(current_segment.X, current_segment.Y,
                                                                pathCoords[pseudoPathNum][viaEndSegment].X,
                                                                pathCoords[pseudoPathNum][viaEndSegment].Y);

      // If the current segment is farther from the pseudo-via than the half-pitch of
      // the diff-pair vias, then use the current segment for segment_4:
      if (distance_to_via > pseudoVia_to_diffPairVia_distance)  {
        segment_4 = copyCoordinates(current_segment);
        break;
      }

      // If we've reached the last segment (end-terminal) of the pseudo-path and none
      // of the above criteria have been satisfied, then use the end-terminal of the
      // pseudo-path for segment_4:
      if (segment == pathLengths[pseudoPathNum] - 1)  {
        segment_4 = copyCoordinates(mapInfo->end_cells[pseudoPathNum]);
        break;
      }

      // In anticipation of the next iteration through this for-loop, update the
      // 'segment_4' variable to be the current iteration's current_segment:
      segment_4 = copyCoordinates(current_segment);
    }  // End of for-loop for index segment, tracking forward
  }  // End of else-block for calculating segment_3 and segment_4

  #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) Found segment_3 = (%d,%d,%d) and segment_4 = (%d,%d,%d)\n\n", omp_get_thread_num(),
           segment_3.X, segment_3.Y, segment_3.Z, segment_4.X, segment_4.Y, segment_4.Z);
  }
  #endif

  // Call the function 'calcUnitVectorToDiffPairVia' using the four coordinates
  // calculated above to find a unit vector:
  #ifdef DEBUG_calcUnitVectorToDiffPairViaWrapper
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) In calcUnitVectorToDiffPairVia_wrapper, calling calcUnitVectorToDiffPairVia with following 4 coordinates:\n", omp_get_thread_num());
    printf("       (thread %2d)    (%d,%d,%d), (%d,%d,%d), (%d,%d,%d), (%d,%d,%d)\n", omp_get_thread_num(),
           segment_1.X, segment_1.Y, segment_1.Z, segment_2.X, segment_2.Y, segment_2.Z, segment_3.X, segment_3.Y, segment_3.Z, segment_4.X, segment_4.Y, segment_4.Z);
  }
  #endif
  unit_vector = calcUnitVectorToDiffPairVia(segment_1, segment_2, segment_3, segment_4);


  // Return the unit vector to the calling program:
  return(unit_vector);

}  // End of function 'calcUnitVectorToDiffPairVia_wrapper'


//-----------------------------------------------------------------------------
// Name: calcDiffPairViaCoordinates
// Desc: Calculate the (x,y) coordinates of diff-pair vias associated with
//       the pseudo-via from segments 'pseudoViaStartSeg' to 'pseudoViaEndSeg'
//       in pseudo-path number 'pseudoPathNum'. The coordinates of the two
//       diff-pair vias are saved in variables via_A_X, via_A_Y, via_B_X,
//       and via_B_Y. If via 'A' or 'B' is located in an illegal zone, then
//       this function sets to TRUE the variables via_A_is_in_forbidden_zone
//       and/or via_B_is_in_forbidden_zone, respectively. If the normal via
//       locations violated routing-direction design-rules, then the via locations
//       are determined by exhaustively searching the map for areas that allow
//       the necessary routing directions. In these cases, the Boolean
//       variables 'via_A_routeDir_violation' and/or 'via_B_routeDir_violation'
//       are set to TRUE.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_calcDiffPairViaCoordinates' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_calcDiffPairViaCoordinates 1
#undef DEBUG_calcDiffPairViaCoordinates

static void calcDiffPairViaCoordinates(int pseudoPathNum, int pseudoViaStartSeg, int pseudoViaEndSegment,
                                       Coordinate_t pseudoCoordsBeforeVia, Coordinate_t pseudoCoordsAfterVia,
                                       int path_1_number, int path_2_number, int *via_A_X, int *via_A_Y, int *via_B_X, int *via_B_Y,
                                       unsigned char *via_A_is_in_forbidden_zone, unsigned char *via_B_is_in_forbidden_zone,
                                       unsigned char *via_A_routeDir_violation, unsigned char *via_B_routeDir_violation,
                                       Coordinate_t *pathCoords[], int pathLengths[], const InputValues_t *user_inputs,
                                       CellInfo_t ***cellInfo, const MapInfo_t *mapInfo)  {

  #ifdef DEBUG_calcDiffPairViaCoordinates
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration >= 1) && (mapInfo->current_iteration <= 1) && (pseudoPathNum > 0))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in calcDiffPairViaCoordinates() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE


  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) Entered function 'createDiffPairVias' with pseudoPathNum=%d and diff-pair paths %d and %d in iteration %d.\n",
           omp_get_thread_num(), pseudoPathNum, path_1_number, path_2_number, mapInfo->current_iteration);
  }  // End of if-block for DEBUG_ON
  #endif


  //
  // Based on design rules, calculate the distance from the center of the pseudo-net via
  // to the center of the diff-pair (shoulder-path) via. This distance is the maximum  of
  // the following:
  //            (a) 0.5 * (Dvu + Svu) for the via-up ('vu') layer,
  //            (b) 0.5 * (Dvu + Strace) for the via-up ('vu') layer,
  //            (c) 0.5 * (Dvd + Svd) for the via-down ('vd') layer,
  //            (d) 0.5 * (Dvd + Strace) for the via-down ('vd') layer,
  //            (e) 0.5 * (diff-pair pitch) for any layer involved in the via.
  //
  // Iterate over each segment of the pseudo-path that makes up the current via-stack,
  // keeping track of the maximum distance between centers of pseudo-vias and diff-pair vias:
  float pseudoVia_to_diffPairVia_distance_cells = 0.0;
  #ifdef DEBUG_calcDiffPairViaCoordinates
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Analyzing design rules from pseudo-path via segments %d (%d,%d,%d) to %d (%d,%d,%d), inclusive...\n",
           omp_get_thread_num(), pseudoViaStartSeg, pseudoCoordsBeforeVia.X, pseudoCoordsBeforeVia.Y, pseudoCoordsBeforeVia.Z,
           pseudoViaEndSegment, pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y, pseudoCoordsAfterVia.Z);
  }
  #endif
  for (int i = pseudoViaStartSeg; i <= pseudoViaEndSegment; i++)  {

    // If i equals -1, then the via-stack started at the start-terminal. Skip
    // this segment of the via-stack:
    if (i < 0)  {
      continue;
    }

    // For the current segment of the via-stack, initialize the distance to zero:
    float current_via_segment_distance_cells = 0.0;

    // Get the design-rule number for this coordinate/layer:
    int DR_num = cellInfo[pathCoords[pseudoPathNum][i].X][pathCoords[pseudoPathNum][i].Y][pathCoords[pseudoPathNum][i].Z].designRuleSet;

    // Get the design-rule subset number of the pseudo net, which is the same subset
    // as the associated diff-pair nets:
    int DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num];


    // Calculate the two distances that could limit the distance between centers of the pseudo-via
    // and the diff-pair vias:
    //   diffPairPitchCells[VIA_UP]
    //   diffPairPitchCells[VIA_DOWN]
    float viaUpLimited_distance_cells   = 0.0;
    float viaDownLimited_distance_cells = 0.0;

    // Calculate a via-up distance if the previous or subsequent pseudo-path segment
    // has a layer number that's greater than the current layer number:
    if (   ((i < pathLengths[pseudoPathNum] - 1) && (pathCoords[pseudoPathNum][i+1].Z > pathCoords[pseudoPathNum][i].Z))
        || ((i > 0)                             && (pathCoords[pseudoPathNum][i-1].Z > pathCoords[pseudoPathNum][i].Z)))  {

      int max_Z;  // Larger Z-coodinate in the via:
      if (i == 0) {
        // We're near start-terminal, so look at Z-value of terminal and subsequent segment:
        max_Z = max(mapInfo->start_cells[pseudoPathNum].Z, pathCoords[pseudoPathNum][i+1].Z);
      }  else if (i == pathLengths[pseudoPathNum] - 1)  {
        // We're at end-terminal, so look at Z-value of terminal and previous segment:
        max_Z = max(pathCoords[pseudoPathNum][i-1].Z, pathCoords[pseudoPathNum][i].Z);
      }
      else  {
        // We're in middle of pseudo-path, so look at previous and subsequent segments:
        max_Z = max(pathCoords[pseudoPathNum][i-1].Z, pathCoords[pseudoPathNum][i+1].Z);
      }  // End of if/else/else-block for calculating max_Z

      int DR_num_above    = cellInfo[pathCoords[pseudoPathNum][i].X][pathCoords[pseudoPathNum][i].Y][max_Z].designRuleSet;
      int DR_subset_above = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num_above];

      viaUpLimited_distance_cells = 0.5 * max(user_inputs->designRules[DR_num][DR_subset].diffPairPitchCells[VIA_UP],
                                              user_inputs->designRules[DR_num_above][DR_subset_above].diffPairPitchCells[VIA_DOWN]);
      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) viaUpLimited_distance_cells = %5.2f = 0.5 x [max(diffPairPitchCells[VIA_UP] for current layer (%5.2f), diffPairPitchCells[VIA_DOWN] for above layer (%5.2f)]\n",
               omp_get_thread_num(), viaUpLimited_distance_cells, user_inputs->designRules[DR_num][DR_subset].diffPairPitchCells[VIA_UP],
                                                            user_inputs->designRules[DR_num_above][DR_subset_above].diffPairPitchCells[VIA_DOWN]);
      }
      #endif

    }  // End of if-block for upward-going via

    // Calculate a via-down distance if the previous or subsequent pseudo-path segment
    // has a layer number that's less than the current layer number:
    if (   ((i < pathLengths[pseudoPathNum] - 1) && (pathCoords[pseudoPathNum][i+1].Z < pathCoords[pseudoPathNum][i].Z))
        || ((i > 0)                             && (pathCoords[pseudoPathNum][i-1].Z < pathCoords[pseudoPathNum][i].Z)))  {

      int min_Z;  // Smaller Z-coodinate in the via:
      if (i == 0) {
        // We're near start-terminal, so look at Z-value of terminal and subsequent segment:
        min_Z = min(mapInfo->start_cells[pseudoPathNum].Z, pathCoords[pseudoPathNum][i+1].Z);
      }  else if (i == pathLengths[pseudoPathNum] - 1)  {
        // We're at end-terminal, so look at Z-value of terminal and previous segment:
        min_Z = min(pathCoords[pseudoPathNum][i-1].Z, pathCoords[pseudoPathNum][i].Z);
      }
      else  {
        // We're in middle of pseudo-path, so look at previous and subsequent segments:
        min_Z = min(pathCoords[pseudoPathNum][i-1].Z, pathCoords[pseudoPathNum][i+1].Z);
      }  // End of if/else/else-block for calculating min_Z

      int DR_num_below    = cellInfo[pathCoords[pseudoPathNum][i].X][pathCoords[pseudoPathNum][i].Y][min_Z].designRuleSet;
      int DR_subset_below = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num_below];

      viaDownLimited_distance_cells = 0.5 * max(user_inputs->designRules[DR_num][DR_subset].diffPairPitchCells[VIA_DOWN],
                                                user_inputs->designRules[DR_num_below][DR_subset_below].diffPairPitchCells[VIA_UP]);

      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) viaDownLimited_distance = %5.2f = 0.5 x [max(diffPairPitchCells[VIA_DOWN] for current layer (%5.2f), diffPairPitchCells[VIA_UP] for below layer (%5.2f)]\n",
               omp_get_thread_num(), viaDownLimited_distance_cells, user_inputs->designRules[DR_num][DR_subset].diffPairPitchCells[VIA_DOWN],
                                                  user_inputs->designRules[DR_num_below][DR_subset_below].diffPairPitchCells[VIA_UP]);
      }
      #endif

    }  // End of if-block for downward-going via

    // Choose the larger of the two distances calculated above to be the
    // distance for the current segment of the via-stack:
    current_via_segment_distance_cells = max(viaUpLimited_distance_cells, viaDownLimited_distance_cells);

    // Compare the current segment's distance to those of other segments in
    // the current via-stack. Choose the larger before moving to next segment:
    pseudoVia_to_diffPairVia_distance_cells = max(pseudoVia_to_diffPairVia_distance_cells, current_via_segment_distance_cells);

    #ifdef DEBUG_calcDiffPairViaCoordinates
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Pseudo segment #%d: viaUpLimited_distance_cells=%5.2f, viaDownLimited_distance_cells=%5.2f, current_via_segment_distance_cells=%5.2f, pseudoVia_to_diffPairVia_distance=%5.2f\n",
             omp_get_thread_num(), i, viaUpLimited_distance_cells, viaDownLimited_distance_cells, current_via_segment_distance_cells, pseudoVia_to_diffPairVia_distance_cells);
    }
    #endif

  }  // End of for-loop for index 'i' (each segment of via-stack)

  #ifdef DEBUG_calcDiffPairViaCoordinates
  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) Found distance between via centers from pseudo-path %d to diff-pair paths %d and %d: %6.5f cells (%6.3f microns).\n",
           omp_get_thread_num(), pseudoPathNum, path_1_number, path_2_number, pseudoVia_to_diffPairVia_distance_cells,
           pseudoVia_to_diffPairVia_distance_cells * user_inputs->cell_size_um);
  }
  #endif


  // Calculate a unit vector from the pseudo-via's center to one of the diff-pair vias. This vector is
  // based on segments before the via and segments after the via:
  Vector2dFloat_t unit_vector_to_diffPair_via;
  unit_vector_to_diffPair_via = calcUnitVectorToDiffPairVia_wrapper(pseudoPathNum, pseudoViaStartSeg,
                                              pseudoViaEndSegment, pathLengths, pathCoords, mapInfo,
                                              cellInfo, pseudoVia_to_diffPairVia_distance_cells);

  #ifdef DEBUG_calcDiffPairViaCoordinates
  if (DEBUG_ON)  {
  printf("\nDEBUG: After calcUnitVectorToDiffPairVia_wrapper, unit_vector_to_diffPair_via is (%.6f, %.6f)\n\n",
         unit_vector_to_diffPair_via.X, unit_vector_to_diffPair_via.Y);
  }
  #endif

  // Calculate X/Y cell-coordinates of the two vias using the unit vector and
  // the via-to-via distance. We temporarily label the vias 'A' and 'B'.
  // X/Y coordinates equal the psuedo-path via coordinates ('origin_*') plus the
  // X/Y-component of the unit vector multiplied by the via-to-via distance.
  //

  // Initialize the following Boolean variables to FALSE. They will be set to TRUE if
  // the coordinates of via A and/or via B reside in an illegal/forbidden zone:
  *via_A_is_in_forbidden_zone = FALSE;
  *via_B_is_in_forbidden_zone = FALSE;

  // Calculate coordinates for via 'A':
  *via_A_X = pseudoCoordsAfterVia.X
             + (int)lround(unit_vector_to_diffPair_via.X * pseudoVia_to_diffPairVia_distance_cells);
  *via_A_Y = pseudoCoordsAfterVia.Y
             + (int)lround(unit_vector_to_diffPair_via.Y * pseudoVia_to_diffPairVia_distance_cells);

  // Calculate coordinates for via 'B' by changing the sign of the unit vector:
  *via_B_X = pseudoCoordsAfterVia.X
             + (int)lround( -unit_vector_to_diffPair_via.X * pseudoVia_to_diffPairVia_distance_cells);
  *via_B_Y = pseudoCoordsAfterVia.Y
             + (int)lround( -unit_vector_to_diffPair_via.Y * pseudoVia_to_diffPairVia_distance_cells);

  // Check whether via 'A' is in forbidden zone or outside of map:
  if ((*via_A_X < 0) || (*via_A_X >= mapInfo->mapWidth) || (*via_A_Y < 0) || (*via_A_Y >= mapInfo->mapHeight)) {
    *via_A_is_in_forbidden_zone = TRUE;
    #ifdef DEBUG_calcDiffPairViaCoordinates
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'A' were outside of map: (%d,%d),\n",
              omp_get_thread_num(), *via_A_X, *via_A_Y);
      printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
    }
    #endif
  }

  // Check whether via 'B' is in forbidden zone or outside of map:
  if ((*via_B_X < 0) || (*via_B_X >= mapInfo->mapWidth) || (*via_B_Y < 0) || (*via_B_Y >= mapInfo->mapHeight)) {
    *via_B_is_in_forbidden_zone = TRUE;
    #ifdef DEBUG_calcDiffPairViaCoordinates
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'B' were outside of map: (%d,%d),\n",
              omp_get_thread_num(), *via_B_X, *via_B_Y);
      printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
    }
    #endif
  }


  // At each level of the via stack, ensure the via location is not too close to a barrier.
  // If so, then set the Boolean flag 'via_*_is_in_forbidden_zone' so we don't create these vias.
  for (int i = pseudoViaStartSeg; i <= pseudoViaEndSegment; i++)  {

    // Get the Z-coordinate of the current segment and next segment in the pseudo-path:
    int via_Z;
    if (i >= 0)  {
      via_Z = pathCoords[pseudoPathNum][i].Z;
    }
    else {
      // Segment 'i' refers to the start-terminal:
      via_Z = mapInfo->start_cells[pseudoPathNum].Z;
    }

    // Check whether the proposed via 'A' coordinates are in/near a forbidden region for trace segments:
    if (   cellInfo[*via_A_X][*via_A_Y][via_Z].forbiddenTraceBarrier
        || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, *via_A_X, *via_A_Y, via_Z, path_1_number, TRACE))  {
      *via_A_is_in_forbidden_zone = TRUE;
      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'A' were in or near a user-defined forbidden zone: (%d,%d,%d),\n",
                omp_get_thread_num(), *via_A_X, *via_A_Y, via_Z);
        printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
      }
      #endif
    }

    // Check whether the proposed via 'B' coordinates are in/near a forbidden region for trace segments:
    if (   cellInfo[*via_B_X][*via_B_Y][via_Z].forbiddenTraceBarrier
        || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, *via_B_X, *via_B_Y, via_Z, path_1_number, TRACE))  {
      *via_B_is_in_forbidden_zone = TRUE;
      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'B' were in or near a user-defined forbidden zone: (%d,%d,%d),\n",
                omp_get_thread_num(), *via_B_X, *via_B_Y, via_Z);
        printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
      }
      #endif
    }

    // Define the Z-coordinate of the previous segment in the pseudo-path, acknowledging
    // that the previous segment might be a start-terminal:
    int prev_Z = 0;
    if (i <= 0)  {
      prev_Z = mapInfo->start_cells[pseudoPathNum].Z;
    }
    else  {
      prev_Z = pathCoords[pseudoPathNum][i-1].Z;
    }

    // Define the Z-coordinate of the next segment in the pseudo-path, acknowledging
    // that the next segment might be an end-terminal:
    int next_Z = 0;
    if (i == pathLengths[pseudoPathNum] - 1)  {
      next_Z = mapInfo->end_cells[pseudoPathNum].Z;
    }
    else  {
      next_Z = pathCoords[pseudoPathNum][i+1].Z;
    }

    #ifdef DEBUG_calcDiffPairViaCoordinates
    if (DEBUG_ON)  {
      if (i >= 0)  {
        printf("DEBUG: (thread %2d) Checking whether pseudo-path via at (%d,%d,%d) (segment %d) is up- or down-via\n",
                omp_get_thread_num(), pathCoords[pseudoPathNum][i].X, pathCoords[pseudoPathNum][i].Y, pathCoords[pseudoPathNum][i].Z, i);
      }
      else  {
        printf("DEBUG: (thread %2d) Checking whether pseudo-path via at (%d,%d,%d) (segment %d) is up- or down-via\n",
                omp_get_thread_num(), mapInfo->start_cells[pseudoPathNum].X, mapInfo->start_cells[pseudoPathNum].Y,
                mapInfo->start_cells[pseudoPathNum].Z, i);
      }
    }
    #endif

    // Calculate whether vias 'A' and 'B' are 'viaUp' or 'viaDown' vias:
    int via_shape_type = 0;
    if ((via_Z > prev_Z) || (via_Z > next_Z))  {
      //
      // An adjacent segment to segment 'i' has a lower Z-value, so the via goes
      // downward from the perspective of the current via 'i'. Check whether a via-down
      // segment is allowed at this location:
      //
      via_shape_type = VIA_DOWN;
      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)    Pseudo-path %d, segment %d, is a VIA_DOWN\n", omp_get_thread_num(), pseudoPathNum, i);
      }
      #endif

      // Check if via 'A' is in or near a user-defined barrier for via-down shape-types:
      if (   cellInfo[*via_A_X][*via_A_Y][via_Z].forbiddenDownViaBarrier
          || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, *via_A_X, *via_A_Y, via_Z, path_1_number, via_shape_type))  {
        *via_A_is_in_forbidden_zone = TRUE;
        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'A' were in or near a user-defined forbidden zone for via-down shape-types: (%d,%d,%d),\n",
                  omp_get_thread_num(), *via_A_X, *via_A_Y, via_Z);
          printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
        }
        #endif
      }

      // Check if via 'B' is in or near a user-defined barrier for via-down shape-types:
      if (   cellInfo[*via_B_X][*via_B_Y][via_Z].forbiddenDownViaBarrier
          || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, *via_B_X, *via_B_Y, via_Z, path_1_number, via_shape_type))  {
        *via_B_is_in_forbidden_zone = TRUE;
        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'B' were in or near a user-defined forbidden zone for via-down shape-types: (%d,%d,%d),\n",
                  omp_get_thread_num(), *via_B_X, *via_B_Y, via_Z);
          printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
        }
        #endif
      }
    }  // End of if-block for checking whether a VIA_DOWN via can be placed in the proposed locations
    else if ((via_Z < prev_Z) || (via_Z < next_Z))  {
      //
      // An adjacent segment to segment 'i' has a higher Z-value, so the via goes
      // upward from the perspective of the current via 'i'. Check whether a via-up
      // segment is allowed at this location:
      //
      via_shape_type = VIA_UP;
      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)    Pseudo-path %d, segment %d, is a VIA_UP\n", omp_get_thread_num(), pseudoPathNum, i);
      }
      #endif

      // Check if via 'A' is in or near a user-defined barrier for via-up shape-types:
      if (   cellInfo[*via_A_X][*via_A_Y][via_Z].forbiddenUpViaBarrier
          || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, *via_A_X, *via_A_Y, via_Z, path_1_number, via_shape_type))  {
        *via_A_is_in_forbidden_zone = TRUE;
        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'A' were in or near a user-defined forbidden zone for via-up shape-types: (%d,%d,%d),\n",
                  omp_get_thread_num(), *via_A_X, *via_A_Y, via_Z);
          printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
        }
        #endif
      }

      // Check if via 'B' is in or near a user-defined barrier for via-up shape-types:
      if (   cellInfo[*via_B_X][*via_B_Y][via_Z].forbiddenUpViaBarrier
          || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, *via_B_X, *via_B_Y, via_Z, path_1_number, via_shape_type))  {
        *via_B_is_in_forbidden_zone = TRUE;
        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Provisional coordinates of diff-pair via 'B' were in or near a user-defined forbidden zone for via-up shape-types: (%d,%d,%d),\n",
                  omp_get_thread_num(), *via_B_X, *via_B_Y, via_Z);
          printf("DEBUG: (thread %2d)   so this via will not be created.\n", omp_get_thread_num());
        }
        #endif
      }

    }  // End of if-block for checking whether a VIA_UP via can be placed in the proposed locations
    else  {
      printf("\n\nERROR: An unexpected error occurred in function 'createDiffPairVias'. The pseudo-via located at (%d,%d,%d)\n",
                  pathCoords[pseudoPathNum][i].X, pathCoords[pseudoPathNum][i].Y, pathCoords[pseudoPathNum][i].Z);
      printf(    "       (segment #%d) does not have adjacent segments at different levels. The two adjacent segments are:\n", i);
      if (i != 0)  {
        printf(    "            Segment %d at coordinates (%d,%d,%d)\n", i-1, pathCoords[pseudoPathNum][i-1].X,
                    pathCoords[pseudoPathNum][i-1].Y, pathCoords[pseudoPathNum][i-1].Z);
      }
      else  {
        printf(    "            Segment %d at coordinates (%d,%d,%d)\n", i-1, mapInfo->start_cells[pseudoPathNum].X,
                    mapInfo->start_cells[pseudoPathNum].Y, mapInfo->start_cells[pseudoPathNum].Z);
      }
      if (i < pathLengths[pseudoPathNum] - 1)  {
        printf(    "            Segment %d at coordinates (%d,%d,%d)\n", i+1, pathCoords[pseudoPathNum][i+1].X,
                    pathCoords[pseudoPathNum][i+1].Y, pathCoords[pseudoPathNum][i+1].Z);
      }
      else  {
        printf(    "            Segment %d at coordinates (%d,%d,%d)\n", i+1, mapInfo->end_cells[pseudoPathNum].X,
                    mapInfo->end_cells[pseudoPathNum].Y, mapInfo->end_cells[pseudoPathNum].Z);
      }
      printf(    "       Please inform the software developer of this fatal error message.\n\n");
      exit(1);
    }

  }  // End of for-loop for index 'i' (from pseudoSegmentBeforeVia to pseudoPathSegment)


  //
  // Confirm that the locations of the proposed vias do not violate routing directions specified
  // by the user. At each level of the via stack, ensure the via location does not violate
  // route-direction restrictions. If a violation is found, set via_*_routeDir_violations to TRUE.
  *via_A_routeDir_violation = FALSE;
  *via_B_routeDir_violation = FALSE;
  for (int i = pseudoViaStartSeg; i <= pseudoViaEndSegment; i++)  {

    // Get the Z-coordinate of the current segment of the pseudo-path:
    int via_Z;
    if (i >= 0)  {
      via_Z = pathCoords[pseudoPathNum][i].Z;
    }
    else {
      // Segment 'i' refers to the start-terminal:
      via_Z = mapInfo->start_cells[pseudoPathNum].Z;
    }

    // Get the design-rule numbers at the locations of via A and via B:
    int DR_num_A = cellInfo[*via_A_X][*via_A_Y][via_Z].designRuleSet;
    int DR_num_B = cellInfo[*via_B_X][*via_B_Y][via_Z].designRuleSet;

    // Get the design-rule subset numbers for via A and via B:
    int DR_subset_A = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num_A];
    int DR_subset_B = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num_B];

    // Check whether via A's location allows for up/down routing directions. We logically
    // 'AND' the 'routeDirections' element with binary 11 0000 0000 0000 0000. If the
    // result is non-zero, then up/down routing is allowed at this location:
    if (! (user_inputs->designRules[DR_num_A][DR_subset_A].routeDirections & 0x030000))  {
      // We got here, so up/down routing is *not* allowed at via A's location
      *via_A_routeDir_violation = TRUE;

      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) In createDiffPairVias, location of proposed via 'A' at location (%d,%d,%d)\n", omp_get_thread_num(), *via_A_X, *via_A_Y, via_Z);
        printf("       (thread %2d) in design-rule set %d and subset %d does NOT permit up/down routing.\n", omp_get_thread_num(), DR_num_A, DR_subset_A);
      }
      #endif
    }

    // Repeat for via B's location:
    if (! (user_inputs->designRules[DR_num_B][DR_subset_B].routeDirections & 0x030000))  {
      // We got here, so up/down routing is *not* allowed at via B's location
      *via_B_routeDir_violation = TRUE;

      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) In createDiffPairVias, location of proposed via 'B' at location (%d,%d,%d)\n", omp_get_thread_num(), *via_B_X, *via_B_Y, via_Z);
        printf("       (thread %2d) in design-rule set %d and subset %d does NOT permit up/down routing.\n", omp_get_thread_num(), DR_num_A, DR_subset_A);
      }
      #endif
    }

  }  // End of for-loop for index 'i' (from pseudoSegmentBeforeVia to pseudoPathSegment)

  //
  // If the locations of one or both proposed vias violate route-direction restrictions, then locate
  // alternate locations near the pseudo-via that don't violate the route-direction restrictions:
  //
  // Start with via 'A':
  if (*via_A_routeDir_violation)  {

    // The 'closest_distance_via_A' variables keep track of the closest distance between the proposed
    // via locations (which prohibit up/down routing) and cells that allow up/down routing. Initialize
    // the variable to a relatively large distance of twice the 'pseudoVia_to_diffPairVia_distance_microns'
    // distance:
    float closest_distance_via_A = 2.0 * pseudoVia_to_diffPairVia_distance_cells;

    // The coordinates (alternate_via_A_X, alternate_via_A_Y) hold the alternate coordinates of the
    // vias for 'A'. We initialize them with the (illegal) coordinates (via_A_X, via_A_Y):
    int alternate_via_A_X = *via_A_X;
    int alternate_via_A_Y = *via_A_Y;

    // Define the maximum distance that we'll search from the pseudo-via for cells that allow
    // up/down routing. This radius is 20% larger than 'pseudoVia_to_diffPairVia_distance_microns', or
    // one cell larger than 'pseudoVia_to_diffPairVia_distance_microns' -- whichever is larger.
    int max_radius_from_pseudoVia = (int)(max(ceil(1.2*pseudoVia_to_diffPairVia_distance_cells),
                                              1.0 + ceil(pseudoVia_to_diffPairVia_distance_cells)));

    // Calculate the squares of the maximum and minimum radii from the pseudo-via that we search for alternate
    // via locations. The maximum radius is ~1.2X the ideal pseudo-via-to-diff-pair-via distance. The minimum
    // radius is half of this ideal distance:
    int max_radius_squared = max_radius_from_pseudoVia * max_radius_from_pseudoVia;
    int min_radius_squared = (int)(pseudoVia_to_diffPairVia_distance_cells * pseudoVia_to_diffPairVia_distance_cells / 4.0);

    // Determine the direction of the via: VIA_UP or VIA_DOWN.
    int via_direction = VIA_UP;
    int via_antiDirection = VIA_DOWN;
    int delta_Z = 1;
    if (pseudoCoordsBeforeVia.Z > pseudoCoordsAfterVia.Z)  {
      via_direction = VIA_DOWN;
      via_antiDirection = VIA_UP;
      delta_Z = -1;
    }

    // Determine how many layers are involved in the via-stack:
    int via_stack_height = 1 + abs(pseudoCoordsBeforeVia.Z - pseudoCoordsAfterVia.Z);

    // Raster over a circle centered at the pseudo-via to locate the (x,y) location closest to
    // the proposed via locations, which also allow vertical route-directions.
    #ifdef DEBUG_calcDiffPairViaCoordinates
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) About to raster +/- %d cells around pseudo-via point (%d,%d)...\n", omp_get_thread_num(),
             max_radius_from_pseudoVia, pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y);
    }
    #endif
    for (int x = pseudoCoordsAfterVia.X - max_radius_from_pseudoVia; x <= pseudoCoordsAfterVia.X + max_radius_from_pseudoVia; x++)  {
      int deltaX_squared = (x - pseudoCoordsAfterVia.X) * (x - pseudoCoordsAfterVia.X);
      for (int y = pseudoCoordsAfterVia.Y - max_radius_from_pseudoVia; y <= pseudoCoordsAfterVia.Y + max_radius_from_pseudoVia; y++)  {

        // Skip this (x,y) point if it's farther than max_radius from pseudo-via, or closer than min_radius:
        int radius_squared = deltaX_squared + (y - pseudoCoordsAfterVia.Y) * (y - pseudoCoordsAfterVia.Y);
        if ((radius_squared > max_radius_squared) || (radius_squared < min_radius_squared))  {
          continue;  // Skip this cell and move on to next (x,y) location
        }

        // Calculate the 2-dimensional distance from the current (x,y,z) cell to the proposed via location:
        float distance_via_A = calc_2D_Pythagorean_distance_ints(x, y, *via_A_X, *via_A_Y);

        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Checking location (%d,%d), located %6.3f cells from ideal via A location...\n",
                 omp_get_thread_num(), x, y, distance_via_A);
        }
        #endif

        // Confirm that the cell is closer to the proposed via location than previous candidates. If not,
        // then move on to the next (x,y) coordinate:
        if (distance_via_A > closest_distance_via_A)  {
          continue;
        }  // End of if-block for (x,y) being too far from proposed via locations

        // Confirm that the cell is not outside of the map's perimeter. If it is, then move on to
        // the next (x,y) coordinate:
        if (XY_coords_are_outside_of_map(x, y, mapInfo))  {
          continue;
        }  // End of if-block for (x,y) being outside of map

        // In preparation for iterating over all routing layers for the pseudo-via stack, capture the
        // pin-swap zone number and the pinSwap proximity status at the beginning of the via-stack:
        int prev_pinSwap_zone = cellInfo[x][y][pseudoCoordsBeforeVia.Z].swap_zone;
        int prev_pinSwap_proximity = get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x, y, pseudoCoordsBeforeVia.Z, path_1_number, via_direction);

        //
        // Iterate over all layers of the pseudo-via stack. If each cell in the via-stack doesn't violate any
        // rules, then set 'valid_XY_via_location' to TRUE. Otherwise, change it to FALSE:
        //
        int valid_XY_via_location = TRUE;
        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)   About to check %d cells vertically from (%d,%d,%d) to (%d,%d,%d)...\n",
                 omp_get_thread_num(), via_stack_height, x, y, pseudoCoordsBeforeVia.Z, x, y, pseudoCoordsAfterVia.Z);
        }
        #endif
        for (int z = pseudoCoordsBeforeVia.Z; abs(z - pseudoCoordsBeforeVia.Z) < via_stack_height; z += delta_Z)  {

          #ifdef DEBUG_calcDiffPairViaCoordinates
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)     Checking via-stack segment at (%d,%d,%d)...\n", omp_get_thread_num(), x, y, z);
          }
          #endif

          // Confirm that route-direction restrictions do not prohibit up/down routing at this cell. If they do,
          // then break out of the innermost for-loop (for index 'z') and move on to the next (x,y) coordinate:
          int DR_num = cellInfo[x][y][z].designRuleSet;
          int DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num];
          if (! (user_inputs->designRules[DR_num][DR_subset].routeDirections & 0x030000))  {

            #ifdef DEBUG_calcDiffPairViaCoordinates
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d)       Cell's route-directions do not allow up/down routing.\n", omp_get_thread_num());
            }
            #endif

            valid_XY_via_location = FALSE;
            break;

          }  // End of if-block for !(routeDirection & 0x030000)

          // Confirm that the cell is not in or near a user-defined barrier for vias. If it is, then
          // break out of the innermost for-loop (for index 'z') and continue on to the next
          // (x,y) coordinate.
          //
          // The forbidden direction depends on which segment of the via we're at. There are 3 cases:
          //   (1) The first segment in the via-stack,
          //   (2) The last segment in the via-stack,
          //   (3) Intermediate segments in the via-stack.
          //
          // First, we handle the case where the segment is the first segment in the via-stack:
          if (z == pseudoCoordsBeforeVia.Z)  {
            if (via_direction == VIA_UP)  {
              // We got here, so via is an upward-going via. Confirm that there's no barrier to such vias:
              if (   cellInfo[x][y][z].forbiddenUpViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       1st cell in via-stack is forbiddenUpViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of if-block for (via_direction == VIA_UP)
            else  {
              // We got here, so via is a downward-going via. Confirm that there's no barrier to such vias. If such
              // vias exist, then break out of the innermost for-loop (for index 'z') and move on to the next
              // (x,y) coordinate:
              if (   cellInfo[x][y][z].forbiddenDownViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       1st cell in via-stack is forbiddenDownViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of else-block for (via_direction == VIA_DOWN)
          }  // End of if-block for (z == pseudoCoordsBeforeVia.Z)

          // Secondly, we handle the case where the segment is the last segment in the via-stack:
          else if (z == pseudoCoordsAfterVia.Z)  {
            if (via_direction == VIA_UP)  {
              // We got here, so via is an upward-going via. But since this is the last segment in the via-stack, we must
              // confirm that there's no barrier to DOWNWARD-going vias:
              if (   cellInfo[x][y][z].forbiddenDownViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_antiDirection)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       Last cell in via-stack is forbiddenDownViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of if-block for (via_direction == VIA_UP)
            else  {
              // We got here, so via is a downward-going via. But since this is the last segment in the via-stack, we must
              // confirm that there's no barrier to UPWARD-going vias.
              if (   cellInfo[x][y][z].forbiddenUpViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_antiDirection)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       Last cell in via-stack is forbiddenUpViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of else-block for (via_direction == VIA_DOWN)
          }  // End of if-block for (z == pseudoCoordsAfterVia.Z)

          // Finally, we handle the case where the segment is an intermediate segment in the via-stack. It's
          // neither the first nor the last segment in the via-stack:
          else  {
            // We got here, so we must confirm that both upward- and downward-going vias are allowed. If there
            // are barriers to such vias, then we break out of the innermost for-loop (for index 'z') and move
            // on to the next (x,y) coordinate:
            if (cellInfo[x][y][z].forbiddenUpViaBarrier || cellInfo[x][y][z].forbiddenDownViaBarrier
                || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)
                || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_antiDirection)) {

              #ifdef DEBUG_calcDiffPairViaCoordinates
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d)       Intermediate cell in via-stack is in/near an up- or down-forbidden zone.\n", omp_get_thread_num());
              }
              #endif

              valid_XY_via_location = FALSE;
              break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
            }
          }  // End of else-block for z being an intermediate segment in the via-stack

          // For the 2nd and subsequent cells in the current via-stack, confirm that the cell is not in or near a pin-swap
          // zone unless the previous cell was in or near a pin-swap zone. If this condition exists, then break out of the
          // innermost for-loop (for index 'z') and move on to the next (x,y) coordinate:
          if (z != pseudoCoordsBeforeVia.Z)  {
            if (get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)
                 && (! prev_pinSwap_zone) && (! prev_pinSwap_proximity))  {

              #ifdef DEBUG_calcDiffPairViaCoordinates
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d)       Cell is illegally in a pinSwap proximity zone.\n", omp_get_thread_num());
              }
              #endif

              valid_XY_via_location = FALSE;
              break;

            }  // End of if-block for finding cell near swap-zone but previous cell was not in/near a swap-zone
          }  // End of if-block for z != pseudoCoordsBeforeVia.Z

          // Re-populate the pin-swap zone and pin-swap-proximity status for the next time through this loop:
          prev_pinSwap_zone = cellInfo[x][y][z].swap_zone;
          prev_pinSwap_proximity = get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction);

        }  // End of for-loop for index 'z'

        // Check if 'valid_XY_via_location' is still TRUE. If so, then save the coordinates and re-define the
        // closest_distance_via_* values:
        if (valid_XY_via_location)  {

          #ifdef DEBUG_calcDiffPairViaCoordinates
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)         (%d,%d) is a valid candidate for a via.\n", omp_get_thread_num(), x, y);
          }
          #endif

          // We got here, so via 'A' needs a new location, and the current (x,y) location is a candidate.
          // Check if the (x,y) candidate is closer than previous candidates:
          if (distance_via_A < closest_distance_via_A)  {

            // We got here, so the current candidate for via 'A' is closer than previous candidates.
            // Save the (x,y) location of this candidate, as well as its distance from the proposed
            // via 'A' location:
            closest_distance_via_A = distance_via_A;

            // Save the new, alternate coordinates for via 'A':
            alternate_via_A_X = x;
            alternate_via_A_Y = y;
          }  // End of if-block for (distance_via_A < closest_distance_via_A)
        }  // End of if-block for valid_XY_via_location
      }  // End of for-loop for index 'y'
    }  // End of for-loop for index 'x'


    // We got here after rastering over the area around the pseudo-via to find valid locations
    // for alternate vias. Confirm that new via locations were indeed found.
    if ((*via_A_X == alternate_via_A_X) && (*via_A_Y == alternate_via_A_Y))  {
      printf("\nERROR: In function calcDiffPairViaCoordinates, the calculated location for via 'A' at (%.2f, %.2f) microns\n",
             *via_A_X * user_inputs->cell_size_um, *via_A_Y  * user_inputs->cell_size_um);
      printf(  "       had route-direction restrictions that prevented up/down vias between layers '%s'\n",
             user_inputs->layer_names[pseudoCoordsBeforeVia.Z]);
      printf(  "       and '%s'. No valid, alternate locations were found for paths '%s' and\n",
             user_inputs->layer_names[pseudoCoordsAfterVia.Z], user_inputs->net_name[path_1_number]);
      printf(  "       '%s', despite searching a zone from %d to %d cells from the pseudo-via at\n",
             user_inputs->net_name[path_2_number], (int)sqrt(min_radius_squared), max_radius_from_pseudoVia);
      printf(  "       (%.2f, %.2f) microns. Review the input text file to ensure that DR_zone, BLOCK, and\n",
             pseudoCoordsAfterVia.X * user_inputs->cell_size_um, pseudoCoordsAfterVia.Y * user_inputs->cell_size_um);
      printf(  "       UNBLOCK statements allow the necessary route-directions.\n\n");
      exit(1);
    }  // End of error condition for via 'A'
    else  {
      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) In function createDiffPairVias, the calculated location for via 'A' of (%d,%d) cells\n", omp_get_thread_num(), *via_A_X, *via_A_Y);
        printf("       (thread %2d) was replaced with alternate location (%d,%d).\n", omp_get_thread_num(), alternate_via_A_X, alternate_via_A_Y);
      }
      #endif
      *via_A_X = alternate_via_A_X;
      *via_A_Y = alternate_via_A_Y;
    }  // End of else-block for replacing via-A's location with alternate location
  }  // End of if-block for *via_A_routeDir_violation == TRUE

  //
  // Now repeat the above for via 'B'. In other words, if the proposed location for via 'B' violates
  // route-direction restrictions, then locate an alternate location near the pseudo-via that doesn't
  // violate the route-direction restrictions
  if (*via_B_routeDir_violation)  {

    // The 'closest_distance_via_B' variables keep track of the closest distance between the proposed
    // via locations (which prohibit up/down routing) and cells that allow up/down routing. Initialize
    // the variable to a relatively large distance of twice the 'pseudoVia_to_diffPairVia_distance_cells'
    // distance:
    float closest_distance_via_B = 2.0 * pseudoVia_to_diffPairVia_distance_cells;

    // The coordinates (alternate_via_B_X, alternate_via_B_Y) hold the alternate coordinates of the
    // vias for 'B'. We initialize them with the (illegal) coordinates (via_B_X, via_B_Y):
    int alternate_via_B_X = *via_B_X;
    int alternate_via_B_Y = *via_B_Y;

    // Define the maximum distance that we'll search from the pseudo-via for cells that allow
    // up/down routing. This radius is 20% larger than 'pseudoVia_to_diffPairVia_distance_cells', or
    // one cell larger than 'pseudoVia_to_diffPairVia_distance' -- whichever is larger.
    int max_radius_from_pseudoVia = (int)(max(round(1.2*pseudoVia_to_diffPairVia_distance_cells),
                                              1.0 + round(pseudoVia_to_diffPairVia_distance_cells)));

    // Calculate the squares of the maximum and minimum radii from the pseudo-via that we search for alternate
    // via locations. The maximum radius is ~1.2X the ideal pseudo-via-to-diff-pair-via distance. The minimum
    // radius is half of this ideal distance:
    int max_radius_squared = max_radius_from_pseudoVia * max_radius_from_pseudoVia;
    int min_radius_squared = (int)round(pseudoVia_to_diffPairVia_distance_cells * pseudoVia_to_diffPairVia_distance_cells / 4.0);

    // Determine the direction of the via: VIA_UP or VIA_DOWN.
    int via_direction = VIA_UP;
    int via_antiDirection = VIA_DOWN;
    int delta_Z = 1;
    if (pseudoCoordsBeforeVia.Z > pseudoCoordsAfterVia.Z)  {
      via_direction = VIA_DOWN;
      via_antiDirection = VIA_UP;
      delta_Z = -1;
    }

    // Determine how many layers are involved in the via-stack:
    int via_stack_height = 1 + abs(pseudoCoordsBeforeVia.Z - pseudoCoordsAfterVia.Z);

    // Raster over a circle centered at the pseudo-via to locate the (x,y) location closest to
    // the proposed via locations, which also allow vertical route-directions.
    #ifdef DEBUG_calcDiffPairViaCoordinates
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) About to raster +/- %d cells around pseudo-via point (%d,%d)...\n", omp_get_thread_num(),
             max_radius_from_pseudoVia, pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y);
    }
    #endif
    for (int x = pseudoCoordsAfterVia.X - max_radius_from_pseudoVia; x <= pseudoCoordsAfterVia.X + max_radius_from_pseudoVia; x++)  {
      int deltaX_squared = (x - pseudoCoordsAfterVia.X) * (x - pseudoCoordsAfterVia.X);
      for (int y = pseudoCoordsAfterVia.Y - max_radius_from_pseudoVia; y <= pseudoCoordsAfterVia.Y + max_radius_from_pseudoVia; y++)  {

        // Skip this (x,y) point if it's farther than max_radius from pseudo-via, or closer than min_radius:
        int radius_squared = deltaX_squared + (y - pseudoCoordsAfterVia.Y) * (y - pseudoCoordsAfterVia.Y);
        if ((radius_squared > max_radius_squared) || (radius_squared < min_radius_squared))  {
          continue;  // Skip this cell and move on to next (x,y) location
        }

        // Calculate the 2-dimensional distance from the current (x,y,z) cell to the proposed via location:
        float distance_via_B = calc_2D_Pythagorean_distance_ints(x, y, *via_B_X, *via_B_Y);

        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) Checking location (%d,%d), located %6.3f cells from ideal via B location...\n",
                 omp_get_thread_num(), x, y, distance_via_B);
        }
        #endif

        // Confirm that the cell is not too close to the via location for the partner via 'A'. If it's too
        // close, then move on to the next (x,y) coordinate. We re-use the 'max_radius_from_pseudoVia' variable
        // as the distance threshold, which is 20% longer than the pseudo-via-to-diff-pair-via-distance.
        if (calc_2D_Pythagorean_distance_ints(x, y, *via_A_X, *via_A_Y) < max_radius_from_pseudoVia)  {
          continue;  // Skip this cell and move on to next (x,y) location
        }

        // Confirm that the cell is closer to the proposed via location than previous candidates. If not,
        // then move on to the next (x,y) coordinate:
        if (distance_via_B > closest_distance_via_B)  {
          continue;
        }  // End of if-block for (x,y) being too far from proposed via locations

        // Confirm that the cell is not outside of the map's perimeter. If it is, then move on to
        // the next (x,y) coordinate:
        if (XY_coords_are_outside_of_map(x, y, mapInfo))  {
          continue;
        }  // End of if-block for (x,y) being outside of map

        // In preparation for iterating over all routing layers for the pseudo-via stack, capture the
        // pin-swap zone number and the pinSwap proximity status at the beginning of the via-stack:
        int prev_pinSwap_zone = cellInfo[x][y][pseudoCoordsBeforeVia.Z].swap_zone;
        int prev_pinSwap_proximity = get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x, y, pseudoCoordsBeforeVia.Z, path_1_number, via_direction);

        //
        // Iterate over all layers of the pseudo-via stack. If each cell in the via-stack doesn't violate any
        // rules, then set 'valid_XY_via_location' to TRUE. Otherwise, change it to FALSE:
        //
        int valid_XY_via_location = TRUE;
        #ifdef DEBUG_calcDiffPairViaCoordinates
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)   About to check %d cells vertically from (%d,%d,%d) to (%d,%d,%d)...\n",
                 omp_get_thread_num(), via_stack_height, x, y, pseudoCoordsBeforeVia.Z, x, y, pseudoCoordsAfterVia.Z);
        }
        #endif
        for (int z = pseudoCoordsBeforeVia.Z; abs(z - pseudoCoordsBeforeVia.Z) < via_stack_height; z += delta_Z)  {

          #ifdef DEBUG_calcDiffPairViaCoordinates
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)     Checking via-stack segment at (%d,%d,%d)...\n", omp_get_thread_num(), x, y, z);
          }
          #endif

          // Confirm that route-direction restrictions do not prohibit up/down routing at this cell. If they do,
          // then break out of the innermost for-loop (for index 'z') and move on to the next (x,y) coordinate:
          int DR_num = cellInfo[x][y][z].designRuleSet;
          int DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_num];
          if (! (user_inputs->designRules[DR_num][DR_subset].routeDirections & 0x030000))  {

            #ifdef DEBUG_calcDiffPairViaCoordinates
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d)       Cell's route-directions do not allow up/down routing.\n", omp_get_thread_num());
            }
            #endif

            valid_XY_via_location = FALSE;
            break;

          }  // End of if-block for !(routeDirection & 0x030000)

          // Confirm that the cell is not in or near a user-defined barrier for vias. If it is, then
          // break out of the innermost for-loop (for index 'z') and continue on to the next
          // (x,y) coordinate.
          //
          // The forbidden direction depends on which segment of the via we're at. There are 3 cases:
          //   (1) The first segment in the via-stack,
          //   (2) The last segment in the via-stack,
          //   (3) Intermediate segments in the via-stack.
          //
          // First, we handle the case where the segment is the first segment in the via-stack:
          if (z == pseudoCoordsBeforeVia.Z)  {
            if (via_direction == VIA_UP)  {
              // We got here, so via is an upward-going via. Confirm that there's no barrier to such vias:
              if (   cellInfo[x][y][z].forbiddenUpViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       1st cell in via-stack is forbiddenUpViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of if-block for (via_direction == VIA_UP)
            else  {
              // We got here, so via is a downward-going via. Confirm that there's no barrier to such vias. If such
              // vias exist, then break out of the innermost for-loop (for index 'z') and move on to the next
              // (x,y) coordinate:
              if (   cellInfo[x][y][z].forbiddenDownViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       1st cell in via-stack is forbiddenDownViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of else-block for (via_direction == VIA_DOWN)
          }  // End of if-block for (z == pseudoCoordsBeforeVia.Z)

          // Secondly, we handle the case where the segment is the last segment in the via-stack:
          else if (z == pseudoCoordsAfterVia.Z)  {
            if (via_direction == VIA_UP)  {
              // We got here, so via is an upward-going via. But since this is the last segment in the via-stack, we must
              // confirm that there's no barrier to DOWNWARD-going vias:
              if (   cellInfo[x][y][z].forbiddenDownViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_antiDirection)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       Last cell in via-stack is forbiddenDownViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of if-block for (via_direction == VIA_UP)
            else  {
              // We got here, so via is a downward-going via. But since this is the last segment in the via-stack, we must
              // confirm that there's no barrier to UPWARD-going vias.
              if (   cellInfo[x][y][z].forbiddenUpViaBarrier
                  || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_antiDirection)) {

                #ifdef DEBUG_calcDiffPairViaCoordinates
                if (DEBUG_ON)  {
                  printf("DEBUG: (thread %2d)       Last cell in via-stack is forbiddenUpViaBarrier.\n", omp_get_thread_num());
                }
                #endif

                valid_XY_via_location = FALSE;
                break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
              }
            }  // End of else-block for (via_direction == VIA_DOWN)
          }  // End of if-block for (z == pseudoCoordsAfterVia.Z)

          // Finally, we handle the case where the segment is an intermediate segment in the via-stack. It's
          // neither the first nor the last segment in the via-stack:
          else  {
            // We got here, so we must confirm that both upward- and downward-going vias are allowed. If there
            // are barriers to such vias, then we break out of the innermost for-loop (for index 'z') and move
            // on to the next (x,y) coordinate:
            if (cellInfo[x][y][z].forbiddenUpViaBarrier || cellInfo[x][y][z].forbiddenDownViaBarrier
                || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)
                || get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_antiDirection)) {

              #ifdef DEBUG_calcDiffPairViaCoordinates
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d)       Intermediate cell in via-stack is in/near an up- or down-forbidden zone.\n", omp_get_thread_num());
              }
              #endif

              valid_XY_via_location = FALSE;
              break;  // Break out of the innermost for-loop (for index 'z') and continue on to the next (x,y) coordinate.
            }
          }  // End of else-block for z being an intermediate segment in the via-stack

          // For the 2nd and subsequent cells in the current via-stack, confirm that the cell is not in or near a pin-swap
          // zone unless the previous cell was in or near a pin-swap zone. If this condition exists, then break out of the
          // innermost for-loop (for index 'z') and move on to the next (x,y) coordinate:
          if (z != pseudoCoordsBeforeVia.Z)  {
            if (get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction)
                 && (! prev_pinSwap_zone) && (! prev_pinSwap_proximity))  {

              #ifdef DEBUG_calcDiffPairViaCoordinates
              if (DEBUG_ON)  {
                printf("DEBUG: (thread %2d)       Cell is illegally in a pinSwap proximity zone.\n", omp_get_thread_num());
              }
              #endif

              valid_XY_via_location = FALSE;
              break;

            }  // End of if-block for finding cell near swap-zone but previous cell was not in/near a swap-zone
          }  // End of if-block for z != pseudoCoordsBeforeVia.Z

          // Re-populate the pin-swap zone and pin-swap-proximity status for the next time through this loop:
          prev_pinSwap_zone = cellInfo[x][y][z].swap_zone;
          prev_pinSwap_proximity = get_unwalkable_pinSwap_proximity_by_path(cellInfo, user_inputs, x, y, z, path_1_number, via_direction);

        }  // End of for-loop for index 'z'

        // Check if 'valid_XY_via_location' is still TRUE. If so, then save the coordinates and re-define the
        // closest_distance_via_* values:
        if (valid_XY_via_location)  {

          #ifdef DEBUG_calcDiffPairViaCoordinates
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d)         (%d,%d) is a valid candidate for a via.\n", omp_get_thread_num(), x, y);
          }
          #endif

          // We got here, so via 'B' needs a new location, and the current (x,y) location is a candidate.
          // Check if the (x,y) candidate is closer than previous candidates:
          if (distance_via_B < closest_distance_via_B)  {

            // We got here, so the current candidate for via 'B' is closer than previous candidates
            // (and is not too close to via 'A'). Save the (x,y) location of this candidate, as well
            // as its distance from the proposed via 'B' location:
            closest_distance_via_B = distance_via_B;

            // Save the new, alternate coordinates for via 'B':
            alternate_via_B_X = x;
            alternate_via_B_Y = y;
          }  // End of if-block for (distance_via_B < closest_distance_via_B)
        }  // End of if-block for valid_XY_via_location
      }  // End of for-loop for index 'y'
    }  // End of for-loop for index 'x'


    // We got here after rastering over the area around the pseudo-via to find valid locations
    // for alternate vias. Confirm that new via locations were indeed found.
    if ((*via_B_X == alternate_via_B_X) && (*via_B_Y == alternate_via_B_Y))  {
      printf("\nERROR: In function calcDiffPairViaCoordinates, the calculated location for via 'B' at (%.2f, %.2f) microns\n",
             *via_B_X * user_inputs->cell_size_um, *via_B_Y  * user_inputs->cell_size_um);
      printf(  "       had route-direction restrictions that prevented up/down vias between layers '%s'\n",
             user_inputs->layer_names[pseudoCoordsBeforeVia.Z]);
      printf(  "       and '%s'. No valid, alternate locations were found for paths '%s' and\n",
             user_inputs->layer_names[pseudoCoordsAfterVia.Z], user_inputs->net_name[path_1_number]);
      printf(  "       '%s', despite searching a zone from %d to %d cells from the pseudo-via at\n",
             user_inputs->net_name[path_2_number], (int)sqrt(min_radius_squared), max_radius_from_pseudoVia);
      printf(  "       (%.2f, %.2f) microns. Review the input text file to ensure that DR_zone, BLOCK, and\n",
             pseudoCoordsAfterVia.X * user_inputs->cell_size_um, pseudoCoordsAfterVia.Y * user_inputs->cell_size_um);
      printf(  "       UNBLOCK statements allow the necessary route-directions.\n\n");
      exit(1);
    }  // End of error condition for via 'B'
    else  {
      #ifdef DEBUG_calcDiffPairViaCoordinates
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) In function createDiffPairVias, the calculated location for via 'B' of (%d,%d)\n", omp_get_thread_num(), *via_B_X, *via_B_Y);
        printf("       (thread %2d) was replaced with alternate location (%d,%d).\n", omp_get_thread_num(), alternate_via_B_X, alternate_via_B_Y);
      }
      #endif
      *via_B_X = alternate_via_B_X;
      *via_B_Y = alternate_via_B_Y;
    }  // End of else-block for replacing via-B's location with alternate location
  }  // End of if-block for *via_B_routeDir_violation == TRUE

}  // End of function 'calcDiffPairViaCoordinates'


//-----------------------------------------------------------------------------
// Name: matchViasToShoulderPaths
// Desc: Determine which shoulder-paths (1 or 2) to match diff-pair vias
//       'A' and 'B' to. The matching attempts to minimize the distance
//       between the vias and the shoulder-paths.
//-----------------------------------------------------------------------------
static unsigned char matchViasToShoulderPaths(int via_A_X, int via_A_Y, int via_B_X, int via_B_Y,
                                              int path_1_number, int path_2_number,
                                              ViaStack_t layerTransition_1, ViaStack_t layerTransition_2,
                                              Coordinate_t *pathCoords[])  {

  // Determine whether to assign via 'A' to diff-pair path 1 or 2 by calculating the distances
  // between the newly calculated via coordinates and the shoulder-points for diff-pair
  // paths #1 and #2. This requires the calculation of 8 distances:
  //    1. Via A to layerTransition_1.startSegment
  //    2. Via A to layerTransition_1.endSegment
  //    3. Via A to layerTransition_2.startSegment
  //    4. Via A to layerTransition_2.endSegment
  //    5. Via B to layerTransition_1.startSegment
  //    6. Via B to layerTransition_1.endSegment
  //    7. Via B to layerTransition_2.startSegment
  //    8. Via B to layerTransition_2.endSegment
  //
  // From the above eight distances, we calculate two quantities:
  //       o   D(A-1,B-2) = #1 + #2 + #7 + #8
  //       o   D(A-2,B-1) = #3 + #4 + #5 + #6
  // If D(A-1,B-2) < D(A-2,B-1), then via A should be associated with shoulder-path #1.
  // Otherwise, via A should be associated with shoulder-path #2.
  // Distance #1:
  float via_A_to_shoulder_1_before_via
       = calc_2D_Pythagorean_distance_ints(via_A_X, via_A_Y,
                         pathCoords[path_1_number][layerTransition_1.startSegment].X,
                         pathCoords[path_1_number][layerTransition_1.startSegment].Y);

  // Distance #2:
  float via_A_to_shoulder_1_after_via
       = calc_2D_Pythagorean_distance_ints(via_A_X, via_A_Y,
                         pathCoords[path_1_number][layerTransition_1.endSegment].X,
                         pathCoords[path_1_number][layerTransition_1.endSegment].Y);

  // Distance #3:
  float via_A_to_shoulder_2_before_via
       = calc_2D_Pythagorean_distance_ints(via_A_X, via_A_Y,
                         pathCoords[path_2_number][layerTransition_2.startSegment].X,
                         pathCoords[path_2_number][layerTransition_2.startSegment].Y);

  // Distance #4:
  float via_A_to_shoulder_2_after_via
       = calc_2D_Pythagorean_distance_ints(via_A_X, via_A_Y,
                         pathCoords[path_2_number][layerTransition_2.endSegment].X,
                         pathCoords[path_2_number][layerTransition_2.endSegment].Y);

  // Distance #5:
  float via_B_to_shoulder_1_before_via
       = calc_2D_Pythagorean_distance_ints(via_B_X, via_B_Y,
                         pathCoords[path_1_number][layerTransition_1.startSegment].X,
                         pathCoords[path_1_number][layerTransition_1.startSegment].Y);

  // Distance #6:
  float via_B_to_shoulder_1_after_via
       = calc_2D_Pythagorean_distance_ints(via_B_X, via_B_Y,
                         pathCoords[path_1_number][layerTransition_1.endSegment].X,
                         pathCoords[path_1_number][layerTransition_1.endSegment].Y);

  // Distance #7:
  float via_B_to_shoulder_2_before_via
       = calc_2D_Pythagorean_distance_ints(via_B_X, via_B_Y,
                         pathCoords[path_2_number][layerTransition_2.startSegment].X,
                         pathCoords[path_2_number][layerTransition_2.startSegment].Y);

  // Distance #8:
  float via_B_to_shoulder_2_after_via
       = calc_2D_Pythagorean_distance_ints(via_B_X, via_B_Y,
                         pathCoords[path_2_number][layerTransition_2.endSegment].X,
                         pathCoords[path_2_number][layerTransition_2.endSegment].Y);

  // If the distance (#1 + #2 + #7 + #8) is less than (#3 + #4 + #5 + #6), then assign
  // via 'A' to diff-pair path 1. Otherwise, assign via 'A' to diff-pair path 2.
  int via_A_is_path_1 = FALSE;  // Boolean flag to specify whether via 'A' is part of diff-pair path #1.
  if ( (via_A_to_shoulder_1_before_via + via_A_to_shoulder_1_after_via + via_B_to_shoulder_2_before_via + via_B_to_shoulder_2_after_via)
     < (via_A_to_shoulder_2_before_via + via_A_to_shoulder_2_after_via + via_B_to_shoulder_1_before_via + via_B_to_shoulder_1_after_via))  {

    via_A_is_path_1 = TRUE;

  }  // End of if-block for comparing sums of 4 distances.

  // Return the result of this function:
  return(via_A_is_path_1);

}  // End of function 'matchViasToShoulderPaths'


//-----------------------------------------------------------------------------
// Name: insertViasInShoulderPaths
// Desc: For each segment in pseudo-path 'pseudoPathNum' from segment 'pseudoViaStartSeg'
//       to segment 'pseudoViaEndSegment' insert a vis in the two shoulder paths
//       with coordinates given by via_A_X, via_A_Y, via_B_X, and via_B_Y. This
//       function modifies the path-coordinates in array 'pathCoords[][]' and
//       the path-lengths in array 'pathLenghts[]'.
//-----------------------------------------------------------------------------
static void insertViasInShoulderPaths(int pseudoPathNum, int path_1_number, int path_2_number,
                                      int pseudoViaStartSeg, int pseudoViaEndSegment, unsigned char via_A_is_path_1,
                                      int via_A_X, int via_A_Y, int via_B_X, int via_B_Y,
                                      unsigned char via_A_is_in_forbidden_zone, unsigned char via_B_is_in_forbidden_zone,
                                      unsigned char via_A_routeDir_violation, unsigned char via_B_routeDir_violation,
                                      ViaStack_t layerTransition_1, ViaStack_t layerTransition_2,
                                      Coordinate_t *pathCoords[], int pathLengths[], const MapInfo_t *mapInfo)  {

  // Calculate the number of segments to be inserted:
  int num_inserted_segments = pseudoViaEndSegment - pseudoViaStartSeg + 1;

  // The value of Boolean flag 'via_A_is_path_1' tells us whether to associate via 'A' with
  // path '1' and via 'B' with path '2', or vice-versa. Based on this variable, set
  // variables appropriately that will be used later in this function:
  int path_1_via_X, path_1_via_Y, path_2_via_X, path_2_via_Y;
  unsigned char path_1_via_forbidden_zone, path_2_via_forbidden_zone;
  unsigned char path_1_routeDir_violation, path_2_routeDir_violation;
  if (via_A_is_path_1)  {
    path_1_via_X = via_A_X;
    path_1_via_Y = via_A_Y;
    path_2_via_X = via_B_X;
    path_2_via_Y = via_B_Y;
    path_1_via_forbidden_zone = via_A_is_in_forbidden_zone;
    path_2_via_forbidden_zone = via_B_is_in_forbidden_zone;
    path_1_routeDir_violation = via_A_routeDir_violation;
    path_2_routeDir_violation = via_B_routeDir_violation;
  }
  else  {
    path_1_via_X = via_B_X;
    path_1_via_Y = via_B_Y;
    path_2_via_X = via_A_X;
    path_2_via_Y = via_A_Y;
    path_1_via_forbidden_zone = via_B_is_in_forbidden_zone;
    path_2_via_forbidden_zone = via_A_is_in_forbidden_zone;
    path_1_routeDir_violation = via_B_routeDir_violation;
    path_2_routeDir_violation = via_A_routeDir_violation;
  }  // End of if/else-block for (via_A_is_path_1 == TRUE)

  //
  // If the via for path #1 is not located in a forbidden zone, insert it
  // into path #1:
  //
  if (! path_1_via_forbidden_zone)  {
    // Increase the path-length for path #1 to accommodate the extra via-segments:
    pathLengths[path_1_number] += num_inserted_segments;

    // Reallocate memory for the longer path #1:
    pathCoords[path_1_number] = realloc(pathCoords[path_1_number], pathLengths[path_1_number] * sizeof(Coordinate_t));

    //
    // Displace the segments in pathCoords for path #1 that follow the segment where we'll next
    // insert the via-segments. We displace each coordinate by 'num_inserted_segments' segments.
    //
    for (int segment = pathLengths[path_1_number] - 1; segment > layerTransition_1.startSegment + num_inserted_segments; segment--)  {
      pathCoords[path_1_number][segment] = copyCoordinates(pathCoords[path_1_number][segment - num_inserted_segments]);
    }  // End of for-loop for index 'segment'

    //
    // Iterate over the pseudo-via segments, and insert via-segments into
    // the shoulder-path array #1 for each pseudo-via segment:
    //
    int j = 0;  // Number of segments that have successfully been inserted
    for (int i = pseudoViaStartSeg; i <= pseudoViaEndSegment; i++)  {
      // Get the Z-coordinate of the current pseudo-path segment and next segment in the pseudo-path:
      int via_Z;
      if (i >= 0)  {
        via_Z = pathCoords[pseudoPathNum][i].Z;
      }
      else {
        // Segment 'i' refers to the start-terminal:
        via_Z = mapInfo->start_cells[pseudoPathNum].Z;
      }

      // Insert the shoulder-path via:
      pathCoords[path_1_number][layerTransition_1.startSegment + 1 + j].X = path_1_via_X;
      pathCoords[path_1_number][layerTransition_1.startSegment + 1 + j].Y = path_1_via_Y;
      pathCoords[path_1_number][layerTransition_1.startSegment + 1 + j].Z = via_Z;

      // If the new via was located by exhaustively searching for cells that allowed up/down
      // route-directions, then set the 'flag' element for this via's coordinates. This flag will
      // signal to downstream functions that these via segments should not be deleted.
      if (path_1_routeDir_violation)  {
        pathCoords[path_1_number][layerTransition_1.startSegment + 1 + j].flag = TRUE;
      }
      else  {
        pathCoords[path_1_number][layerTransition_1.startSegment + 1 + j].flag = FALSE;
      }

      // Increment the number of segments that have so far been inserted into the shoulder-path:
      j++;

    }  // End of for-loop for index 'i' (

  }  // End of if-block for (! path_1_via_forbidden_zone)

  //
  // Now we repeat the above process for path #2. If the via for path #2 is not
  // located in a forbidden zone, insert it into path #2:
  //
  if (! path_2_via_forbidden_zone)  {
    // Increase the path-length for path #2 to accommodate the extra via-segments:
    pathLengths[path_2_number] += num_inserted_segments;

    // Reallocate memory for the longer path #2:
    pathCoords[path_2_number] = realloc(pathCoords[path_2_number], pathLengths[path_2_number] * sizeof(Coordinate_t));

    //
    // Displace the segments in pathCoords for path #2 that follow the segment where we'll next
    // insert the via-segments. We displace each coordinate by 'num_inserted_segments' segments.
    //
    for (int segment = pathLengths[path_2_number] - 1; segment > layerTransition_2.startSegment + num_inserted_segments; segment--)  {
      pathCoords[path_2_number][segment] = copyCoordinates(pathCoords[path_2_number][segment - num_inserted_segments]);
    }  // End of for-loop for index 'segment'

    //
    // Iterate over the pseudo-via segments, and insert via-segments into
    // the shoulder-path array #2 for each pseudo-via segment:
    //
    int j = 0;  // Number of segments that have successfully been inserted
    for (int i = pseudoViaStartSeg; i <= pseudoViaEndSegment; i++)  {
      // Get the Z-coordinate of the current pseudo-path segment and next segment in the pseudo-path:
      int via_Z;
      if (i >= 0)  {
        via_Z = pathCoords[pseudoPathNum][i].Z;
      }
      else {
        // Segment 'i' refers to the start-terminal:
        via_Z = mapInfo->start_cells[pseudoPathNum].Z;
      }

      // Insert the shoulder-path via:
      pathCoords[path_2_number][layerTransition_2.startSegment + 1 + j].X = path_2_via_X;
      pathCoords[path_2_number][layerTransition_2.startSegment + 1 + j].Y = path_2_via_Y;
      pathCoords[path_2_number][layerTransition_2.startSegment + 1 + j].Z = via_Z;

      // If the new via was located by exhaustively searching for cells that allowed up/down
      // route-directions, then set the 'flag' element for this via's coordinates. This flag will
      // signal to downstream functions that these via segments should not be deleted.
      if (path_2_routeDir_violation)  {
        pathCoords[path_2_number][layerTransition_2.startSegment + 1 + j].flag = TRUE;
      }
      else  {
        pathCoords[path_2_number][layerTransition_2.startSegment + 1 + j].flag = FALSE;
      }

      // Increment the number of segments that have so far been inserted into the shoulder-path:
      j++;

    }  // End of for-loop for index 'i' (from pseudoViaStartSeg to pseudoViaEndSegment)

  }  // End of if-block for (! path_1_via_forbidden_zone)

}  // End of function 'insertViasInShoulderPaths'


//-----------------------------------------------------------------------------
// Name: createDiffPairVias
// Desc: Create diff-pair vias for the shoulder points alongside pseudo-net
//       'pseudoPathNum. The two diff-pair paths are 'path_1_number' and
//       'path_2_number'. This function modifies the structure that contains
//       the path segments, 'pathCoords', as well as the array that contains
//       the path lengths, 'pathLengths'.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_createDiffPairVias' and re-compile if you want verbose debugging
// print-statements enabled:
//
// #define DEBUG_createDiffPairVias 1
#undef DEBUG_createDiffPairVias

void createDiffPairVias(const int pseudoPathNum, const int path_1_number, const int path_2_number,
                        Coordinate_t *pathCoords[], int pathLengths[], const InputValues_t *user_inputs,
                        CellInfo_t ***cellInfo, const MapInfo_t *mapInfo) {

  #ifdef DEBUG_createDiffPairVias
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration >= 84) && (mapInfo->current_iteration <= 84) && (pseudoPathNum == 2))  {
    printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in createDiffPairVias() because specific requirements were met.\n\n", omp_get_thread_num());
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE


  if (DEBUG_ON)  {
    printf("\nDEBUG: (thread %2d) Entered function 'createDiffPairVias' with pseudoPathNum=%d and diff-pair paths %d and %d in iteration %d.\n",
           omp_get_thread_num(), pseudoPathNum, path_1_number, path_2_number, mapInfo->current_iteration);
  }  // End of if-block for DEBUG_ON
  #endif

  // Get the start-location of the pseudo-path:
  int pseudoStartX = mapInfo->start_cells[pseudoPathNum].X;
  int pseudoStartY = mapInfo->start_cells[pseudoPathNum].Y;
  int pseudoStartZ = mapInfo->start_cells[pseudoPathNum].Z;


  // Temporary variable to hold coordinates of previous segment of pseudo-path
  Coordinate_t prevPseudoSegmentCoords;
  Coordinate_t pseudoCoordsBeforeVia;

  int numVias = 0; // Maximum number of vias in pseudo-path

  // Count the maximum number of vias necessary to add to the shoulder path by iterating
  // through the segments of the pseudo-path array. Do not include the start- and
  // end-segments because a different algorithm will handle vias at these locations.
  prevPseudoSegmentCoords.X = pseudoStartX;
  prevPseudoSegmentCoords.Y = pseudoStartY;
  prevPseudoSegmentCoords.Z = pseudoStartZ;
  pseudoCoordsBeforeVia     = copyCoordinates(mapInfo->start_cells[pseudoPathNum]);

  for (int pseudoPathSegment = 0; pseudoPathSegment < pathLengths[pseudoPathNum]; pseudoPathSegment++)  {

    #ifdef DEBUG_createDiffPairVias
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) Checking for layer-transition at pseudo-path between segment %d (%d,%d,%d) and %d (%d,%d,%d).\n", omp_get_thread_num(),
             pseudoPathSegment-1, prevPseudoSegmentCoords.X, prevPseudoSegmentCoords.Y, prevPseudoSegmentCoords.Z,
             pseudoPathSegment, pathCoords[pseudoPathNum][pseudoPathSegment].X, pathCoords[pseudoPathNum][pseudoPathSegment].Y,
             pathCoords[pseudoPathNum][pseudoPathSegment].Z );
    }  // End of if-block for DEBUG_ON
    #endif

    // If current segment of pseudo-net is on a different layer (Z-coordinate)
    // than the previous segment, then a via is needed in the shoulder path.
    if (pathCoords[pseudoPathNum][pseudoPathSegment].Z != prevPseudoSegmentCoords.Z)  {

      // Increment the number of vias:
      numVias++;

      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Found via between segment %d (%d,%d,%d) and %d (%d,%d,%d).\n", omp_get_thread_num(),
                pseudoPathSegment-1, prevPseudoSegmentCoords.X, prevPseudoSegmentCoords.Y, prevPseudoSegmentCoords.Z,
                pseudoPathSegment,   pathCoords[pseudoPathNum][pseudoPathSegment].X, pathCoords[pseudoPathNum][pseudoPathSegment].Y,
                                     pathCoords[pseudoPathNum][pseudoPathSegment].Z);
        }
      #endif

    }  // End of if-block for reaching a different layer than previous segment


    // In preparation for the next run through this loop, copy the current segment's
    // coordinates into the 'prevPseudoSegmentCoords' variable:
    prevPseudoSegmentCoords = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);
  }  // End of for-loop for index 'pseudoPathSegment'

  #ifdef DEBUG_createDiffPairVias
  if (DEBUG_ON)  {
    printf("DEBUG: (thread %2d) Found %d layer-transitions in pseudo-path %d, including vias in swap-zones (if applicable).\n\n", omp_get_thread_num(),
           numVias, pseudoPathNum);
  }
  #endif


  // Iterate through all segments of the pseudo-path to locate each via. Do not
  // include the start- and end-segments (terminals) because a different algorithm
  // will handle vias at these locations.
  prevPseudoSegmentCoords.X = pseudoStartX;
  prevPseudoSegmentCoords.Y = pseudoStartY;
  prevPseudoSegmentCoords.Z = pseudoStartZ;
  int pseudoSegmentBeforeVia     = -1;  // Keeps track of segment before most recent via in pseudo-path
  int via_starts_in_swap_zone    = FALSE;  // Boolean variable that tracks whether pseudo-path via-stack starts in a swap-zone
  for (int pseudoPathSegment = 0; pseudoPathSegment < pathLengths[pseudoPathNum]; pseudoPathSegment++)  {

    #ifdef DEBUG_createDiffPairVias
    if (DEBUG_ON)  {
      printf("DEBUG: (thread %2d) In createDiffPairVias, pseudo-path #%d, segment %d: (%d,%d,%d) in swap-zone #%d. Previous segment = (%d,%d,%d). pseudoSegmentBeforeVia = %d (%d,%d,%d)\n",
             omp_get_thread_num(), pseudoPathNum, pseudoPathSegment,
             pathCoords[pseudoPathNum][pseudoPathSegment].X, pathCoords[pseudoPathNum][pseudoPathSegment].Y, pathCoords[pseudoPathNum][pseudoPathSegment].Z,
             cellInfo[pathCoords[pseudoPathNum][pseudoPathSegment].X][pathCoords[pseudoPathNum][pseudoPathSegment].Y][pathCoords[pseudoPathNum][pseudoPathSegment].Z].swap_zone,
             prevPseudoSegmentCoords.X, prevPseudoSegmentCoords.Y, prevPseudoSegmentCoords.Z, pseudoSegmentBeforeVia, pseudoCoordsBeforeVia.X, pseudoCoordsBeforeVia.Y, pseudoCoordsBeforeVia.Z);
    }
    #endif

    // If current segment is on the same routing layer (Z-value) as the previous segment, then save
    // this segment number as the last segment before the via. (This will be updated/overwritten each
    // time through the loop, until a via is actually discovered.)
    if (pathCoords[pseudoPathNum][pseudoPathSegment].Z == prevPseudoSegmentCoords.Z)  {

      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) In createDiffPairVias, updating pseudoSegmentBeforeVia from %d (on layer %d) to %d (on layer %d)\n",
               omp_get_thread_num(), pseudoSegmentBeforeVia, pseudoCoordsBeforeVia.Z, pseudoPathSegment, pathCoords[pseudoPathNum][pseudoPathSegment].Z);
      }
      #endif

      pseudoSegmentBeforeVia = pseudoPathSegment;
      pseudoCoordsBeforeVia = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);
    }  // End of if-block for recording 'pseudoSegmentBeforeVia'


    //
    // Check the rare case in which the starting terminal is in a swap-zone and is
    // also the first segment of a via-stack:
    //
    if ((pseudoPathSegment == 0) && (pseudoStartZ != pathCoords[pseudoPathNum][pseudoPathSegment].Z))  {

      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("DEBUG:(thread %2d)    Beginning of a via-stack was detected at the pseudo-net's start-terminal (%d,%d,%d)\n",
               omp_get_thread_num(), pseudoStartX, pseudoStartY, pseudoStartZ);
      }
      #endif

      // We found the beginning of a via-stack. Now check whether the first segment is located in a swap-zone.
      // For such via-stacks, no shoulder-path vias will be created later on:
      if (cellInfo[pseudoStartX][pseudoStartY][pseudoStartZ].swap_zone)  {
        via_starts_in_swap_zone = TRUE;
        #ifdef DEBUG_createDiffPairVias
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      Via-stack starts in swap-zone #%d.\n", omp_get_thread_num(),
                    cellInfo[pseudoStartX][pseudoStartY][pseudoStartZ].swap_zone);
        }
        #endif
      }
      else  {
        via_starts_in_swap_zone = FALSE;
        #ifdef DEBUG_createDiffPairVias
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
             && (pseudoPathSegment < pathLengths[pseudoPathNum] - 1)
             && (pathCoords[pseudoPathNum][pseudoPathSegment].Z != pathCoords[pseudoPathNum][pseudoPathSegment+1].Z))  {

      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d)   Beginning of a via-stack was detected at (%d,%d,%d)\n", omp_get_thread_num(),
               pathCoords[pseudoPathNum][pseudoPathSegment].X, pathCoords[pseudoPathNum][pseudoPathSegment].Y,
               pathCoords[pseudoPathNum][pseudoPathSegment].Z);
      }
      #endif

      // We found the beginning of a via-stack. Now check whether the first segment is located in a swap-zone.
      // For such via-stacks, no shoulder-path vias will be created later on:
      if (cellInfo[pathCoords[pseudoPathNum][pseudoPathSegment].X][pathCoords[pseudoPathNum][pseudoPathSegment].Y][pathCoords[pseudoPathNum][pseudoPathSegment].Z].swap_zone)  {
        via_starts_in_swap_zone = TRUE;

        #ifdef DEBUG_createDiffPairVias
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      Via-stack starts in swap-zone #%d.\n", omp_get_thread_num(),
                  cellInfo[pathCoords[pseudoPathNum][pseudoPathSegment].X][pathCoords[pseudoPathNum][pseudoPathSegment].Y][pathCoords[pseudoPathNum][pseudoPathSegment].Z].swap_zone);
        }
        #endif

      }
      else  {
        via_starts_in_swap_zone = FALSE;
        #ifdef DEBUG_createDiffPairVias
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d)      Via-stack does NOT start in a swap-zone.\n", omp_get_thread_num());
        }
        #endif
      }
    }  // End of if-block for locating the beginning of a via-stack in the pseudo-path

    // If current segment of pseudo-net satisfies the following criteria, then a via is needed in the shoulder paths:
    //   (a) pseudo-path segment is on a different layer (Z-coordinate) than the previous segment, AND
    //   (b) the current segment is the last segment in the pseudo-path, or the next segment is on the same layer, AND
    //   (c) the via-stack did not begin in a swap-zone:
    if (   (pathCoords[pseudoPathNum][pseudoPathSegment].Z != prevPseudoSegmentCoords.Z)
        && ((pseudoPathSegment == pathLengths[pseudoPathNum]-1)
               || (pathCoords[pseudoPathNum][pseudoPathSegment].Z == pathCoords[pseudoPathNum][pseudoPathSegment+1].Z))
        && (! via_starts_in_swap_zone) )  {

      // Make temporary copies of the coordinates at end of the pseudo-via stack:
      Coordinate_t pseudoCoordsAfterVia = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);

      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) We got to end of a via-stack at pseudoPathSegment %d located at (%d,%d,%d).\n", omp_get_thread_num(),
               pseudoPathSegment, pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y, pseudoCoordsAfterVia.Z);
      }
      #endif

      // We found a layer-transition in the pseudo-net. Now find the corresponding layer-
      // transitions in the two shoulder-paths. These transitions will be captured in variables
      // layerTransition_1 and layerTransition_2:
      ViaStack_t layerTransition_1, layerTransition_2;

      // First, find the layer-transtion in shoulder-path #1:
      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Calling findNearbyLayerTransition_wrapper from createDiffPairVias with pathNum = %d, startLayer = %d and endLayer = %d\n",
                omp_get_thread_num(), path_1_number, pseudoCoordsBeforeVia.Z, pseudoCoordsAfterVia.Z);
      }
      #endif
      layerTransition_1 = findNearbyLayerTransition_wrapper(path_1_number, pathLengths, pathCoords, pseudoCoordsBeforeVia.Z, pseudoCoordsAfterVia.Z,
                                                            pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y, mapInfo, user_inputs);

      // Confirm that a layer-transition was found in shoulder-path 1:
      if (layerTransition_1.error == TRUE)  {
        printf("\nERROR: (thread %2d) Function 'findNearbyLayerTransition_wrapper' failed to find a layer-transition in diff-pair path %d (%s) corresponding\n",
               omp_get_thread_num(), path_1_number, user_inputs->net_name[path_1_number]);
        printf(  "ERROR: (thread %2d) to the pseudo-via at coordinates (%d,%d) from layer %d (%s) to layer %d (%s). Please inform the\n", omp_get_thread_num(),
               pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y, pseudoCoordsBeforeVia.Z, user_inputs->routingLayerNames[pseudoCoordsBeforeVia.Z],
               pseudoCoordsAfterVia.Z, user_inputs->routingLayerNames[pseudoCoordsAfterVia.Z]);
        printf(  "ERROR: (thread %2d) software developer of this fatal error message.\n\n", omp_get_thread_num());
        exit(1);
      }

      // Repeat for shoulder-path #2:
      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("DEBUG: (thread %2d) Calling findNearbyLayerTransition_wrapper from createDiffPairVias with pathNum = %d, startLayer = %d and endLayer = %d\n",
                omp_get_thread_num(), path_2_number, pseudoCoordsBeforeVia.Z, pseudoCoordsAfterVia.Z);
      }
      #endif
      layerTransition_2 = findNearbyLayerTransition_wrapper(path_2_number, pathLengths, pathCoords, pseudoCoordsBeforeVia.Z, pseudoCoordsAfterVia.Z,
                                                            pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y, mapInfo, user_inputs);

      // Confirm that a layer-transition was found in shoulder-path 2:
      if (layerTransition_2.error == TRUE)  {
        printf("\nERROR: (thread %2d) Function 'findNearbyLayerTransition_wrapper' failed to find a layer-transition in diff-pair path %d (%s) corresponding\n",
               omp_get_thread_num(), path_2_number, user_inputs->net_name[path_2_number]);
        printf(  "ERROR: (thread %2d) to the pseudo-via at coordinates (%d,%d) from layer %d (%s) to layer %d (%s). Please inform the\n", omp_get_thread_num(),
               pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y, pseudoCoordsBeforeVia.Z, user_inputs->routingLayerNames[pseudoCoordsBeforeVia.Z],
               pseudoCoordsAfterVia.Z, user_inputs->routingLayerNames[pseudoCoordsAfterVia.Z]);
        printf(  "ERROR: (thread %2d) software developer of this fatal error message.\n\n", omp_get_thread_num());
        exit(1);
      }

      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        int x, y, z;
        if (pseudoPathSegment == 0)  {
          x = mapInfo->start_cells[pseudoPathNum].X;
          y = mapInfo->start_cells[pseudoPathNum].Y;
          z = mapInfo->start_cells[pseudoPathNum].Z;
        }
        else  {
          x = pseudoCoordsBeforeVia.X;
          y = pseudoCoordsBeforeVia.Y;
          z = pseudoCoordsBeforeVia.Z;
        }
        printf("DEBUG: (thread %2d) Pseudo-path #%d transitioned layers from segment %d (%d,%d,%d) to %d (%d,%d,%d).\n",
               omp_get_thread_num(), pseudoPathNum, pseudoPathSegment - 1, x, y, z,
               pseudoPathSegment, pseudoCoordsAfterVia.X, pseudoCoordsAfterVia.Y, pseudoCoordsAfterVia.Z);

        if (layerTransition_1.endSegment == 0)  {
          x = mapInfo->start_cells[path_1_number].X;
          y = mapInfo->start_cells[path_1_number].Y;
          z = mapInfo->start_cells[path_1_number].Z;
        }
        else  {
          x = pathCoords[path_1_number][layerTransition_1.endSegment-1].X;
          y = pathCoords[path_1_number][layerTransition_1.endSegment-1].Y;
          z = pathCoords[path_1_number][layerTransition_1.endSegment-1].Z;
        }
        printf("       (thread %2d)   Shoulder path %d transitioned from segment %d (%d,%d,%d) to %d (%d,%d,%d). shoulderSegmentBeforeVia = %d.\n",
               omp_get_thread_num(), path_1_number, layerTransition_1.endSegment - 1, x, y, z,
               layerTransition_1.endSegment, pathCoords[path_1_number][layerTransition_1.endSegment].X,
               pathCoords[path_1_number][layerTransition_1.endSegment].Y, pathCoords[path_1_number][layerTransition_1.endSegment].Z,
               layerTransition_1.startSegment);

        if (layerTransition_2.endSegment == 0)  {
          x = mapInfo->start_cells[path_2_number].X;
          y = mapInfo->start_cells[path_2_number].Y;
          z = mapInfo->start_cells[path_2_number].Z;
        }
        else  {
          x = pathCoords[path_2_number][layerTransition_2.endSegment-1].X;
          y = pathCoords[path_2_number][layerTransition_2.endSegment-1].Y;
          z = pathCoords[path_2_number][layerTransition_2.endSegment-1].Z;
        }
        printf("       (thread %2d)   Shoulder path %d transitioned from segment %d (%d,%d,%d) to %d (%d,%d,%d). shoulderSegmentBeforeVia = %d.\n",
               omp_get_thread_num(), path_2_number, layerTransition_2.endSegment - 1, x, y, z,
               layerTransition_2.endSegment, pathCoords[path_2_number][layerTransition_2.endSegment].X,
               pathCoords[path_2_number][layerTransition_2.endSegment].Y, pathCoords[path_2_number][layerTransition_2.endSegment].Z,
               layerTransition_2.startSegment);
      }
      #endif

      //
      // Call function that calculates the coordinates of the two diff-pair vias:
      //
      int via_A_X, via_A_Y, via_B_X, via_B_Y;  // Coordinates of two vias
      unsigned char via_A_is_in_forbidden_zone, via_B_is_in_forbidden_zone;  // Boolean flags to specify if via coordinates are in forbidden zones
      unsigned char via_A_routeDir_violation, via_B_routeDir_violation;      // Boolean flags to specify if via coordinates were determined by
                                                                             // exhaustively searching for cells that allow up/down routing
      calcDiffPairViaCoordinates(pseudoPathNum, pseudoSegmentBeforeVia, pseudoPathSegment,
                                 pseudoCoordsBeforeVia, pseudoCoordsAfterVia, path_1_number, path_2_number,
                                 &via_A_X, &via_A_Y, &via_B_X, &via_B_Y, &via_A_is_in_forbidden_zone, &via_B_is_in_forbidden_zone,
                                 &via_A_routeDir_violation, &via_B_routeDir_violation,
                                 pathCoords, pathLengths, user_inputs, cellInfo, mapInfo);

      //
      // Determine whether to assign via 'A' to diff-pair path 1 or 2 by calculating the distances
      // between the newly calculated via coordinates and the shoulder-points for diff-pair
      // paths #1 and #2.
      //
      unsigned char via_A_is_path_1 = matchViasToShoulderPaths(via_A_X, via_A_Y, via_B_X, via_B_Y,
                                            path_1_number, path_2_number, layerTransition_1, layerTransition_2, pathCoords);


      //
      // Insert the newly defined shoulder-path vias into the shoulder-path arrays:
      //
      insertViasInShoulderPaths(pseudoPathNum, path_1_number, path_2_number, pseudoSegmentBeforeVia, pseudoPathSegment,
                                via_A_is_path_1, via_A_X, via_A_Y, via_B_X, via_B_Y, via_A_is_in_forbidden_zone, via_B_is_in_forbidden_zone,
                                via_A_routeDir_violation, via_B_routeDir_violation, layerTransition_1, layerTransition_2,
                                pathCoords, pathLengths, mapInfo);

      #ifdef DEBUG_createDiffPairVias
      if (DEBUG_ON)  {
        printf("\nDEBUG: After insertViasInShoulderPaths within createDiffPairVias in iteration %d\n", mapInfo->current_iteration);

        printf("\nDEBUG: (thread %2d) Path number %d:\n", omp_get_thread_num(), path_1_number);
        printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_1_number, mapInfo->start_cells[path_1_number].X,
                mapInfo->start_cells[path_1_number].Y, mapInfo->start_cells[path_1_number].Z, mapInfo->start_cells[path_1_number].flag);
        for (int segment = 0; segment < pathLengths[path_1_number]; segment++)  {
          printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_1_number, segment,
                  pathCoords[path_1_number][segment].X, pathCoords[path_1_number][segment].Y, pathCoords[path_1_number][segment].Z,
                  pathCoords[path_1_number][segment].flag);
        }  // End of for-loop for index 'segment'
        printf("\nDEBUG: (thread %2d) Path number %d:\n", omp_get_thread_num(), path_2_number);
        printf("DEBUG: (thread %2d)   Path %d, start-terminal (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_2_number,
               mapInfo->start_cells[path_2_number].X, mapInfo->start_cells[path_2_number].Y,
               mapInfo->start_cells[path_2_number].Z, mapInfo->start_cells[path_2_number].flag);
        for (int segment = 0; segment < pathLengths[path_2_number]; segment++)  {
          printf("DEBUG: (thread %2d)   Path %d, segment %d: (%d,%d,%d), flag = %d\n", omp_get_thread_num(), path_2_number, segment,
                  pathCoords[path_2_number][segment].X, pathCoords[path_2_number][segment].Y,
                  pathCoords[path_2_number][segment].Z, pathCoords[path_2_number][segment].flag);
        }  // End of for-loop for index 'segment'
      }
      #endif

    }  // End of if-block for reaching end of a via-stack
    else  {
      // The current pseudo-path segment is not at the end of a via-stack. If the current
      // segment is on same layer as previous segment, then record this segment number
      // in 'pseudoSegmentBeforeVia', which we'll later use when we indeed reach the
      // end of a via-stack:
      if (pathCoords[pseudoPathNum][pseudoPathSegment].Z == prevPseudoSegmentCoords.Z)  {
        pseudoSegmentBeforeVia = pseudoPathSegment;

        #ifdef DEBUG_createDiffPairVias
        if (DEBUG_ON)  {
          printf("DEBUG: (thread %2d) We're not at the end of a via-stack that started in a non-swap-zone, so setting pseudoSegmentBeforeVia to %d.\n",
                 omp_get_thread_num(), pseudoSegmentBeforeVia);
        }
        #endif
      }
    }  // End of else-block (segment is not at end of via-stack)

    // In preparation for the next run through this loop, copy the current segment's
    // coordinates into the 'prevPseudoSegmentCoords' variable:
    prevPseudoSegmentCoords = copyCoordinates(pathCoords[pseudoPathNum][pseudoPathSegment]);

  }  //  End of for-loop for index 'pseudoPathSegment'

  #ifdef DEBUG_createDiffPairVias
  if (DEBUG_ON)  {
    printf("\n\nDEBUG: (thread %2d) At end of createDiffPairVias, path %d has %d segments:\n", omp_get_thread_num(), path_1_number, pathLengths[path_1_number]);
    for (int i = 0; i < pathLengths[path_1_number]; i++)  {
      printf("DEBUG: (thread %2d) Path %d, segment %d: (%d,%d,%d)\n", omp_get_thread_num(), path_1_number, i,
             pathCoords[path_1_number][i].X, pathCoords[path_1_number][i].Y, pathCoords[path_1_number][i].Z);
    }
    printf("DEBUG: (thread %2d)\n\n", omp_get_thread_num());
    printf("\n\nDEBUG: (thread %2d) At end of createDiffPairVias, path %d has %d segments:\n", omp_get_thread_num(),
           path_2_number, pathLengths[path_2_number]);
    for (int i = 0; i < pathLengths[path_2_number]; i++)  {
      printf("DEBUG: (thread %2d) Path %d, segment %d: (%d,%d,%d)\n", omp_get_thread_num(), path_2_number, i,
             pathCoords[path_2_number][i].X, pathCoords[path_2_number][i].Y, pathCoords[path_2_number][i].Z);
    }
    printf("DEBUG: (thread %2d)\n\n", omp_get_thread_num());
  }
  #endif

}  // End of function 'createDiffPairVias'

