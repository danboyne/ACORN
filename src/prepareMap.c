#include "global_defs.h"



//-----------------------------------------------------------------------------
// Name: set_unwalkable_barrier_proximity
// Desc: Modifies the 'forbiddenProximityBarrier' elements of the 3D cellInfo
//       matrix. This function flags this cell to be unwalkable due to
//       proximity to a nearby, user-defined obstacle/barrier. The shape-type
//       and design-rule set that will be forbidden from routing through this
//       cell are given by parameters 'shape_type' and 'DR_subset'.
//-----------------------------------------------------------------------------
static void set_unwalkable_barrier_proximity(CellInfo_t *cellInfo, const int DR_subset, const int shape_type)  {

  // Calculate bit-offset from base address, based on the design-rule subset
  // and the shape type. The largest possible value of offset is 47, because
  // the maximum DR_subset is 15, and the maximum shape_type is 2.
  int offset = DR_subset * NUM_SHAPE_TYPES   +   shape_type;

  // Initialize 'mask' as 64-bit integer with binary '1' in the right-most position, i.e.:
  // binary '0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001'.
  // Then shift the '1' to the left by 'offset' bits:
  long unsigned int mask = 1 << offset;

  // Logically OR the mask with with the existing value of 'forbiddenProximityBarrier'. The
  // 'pragma' line was intended to prevent the 'cellInfo' matrix from becoming corrupted if this function
  // is called simultaneously from multiple threads. However, using this 'pragma' statement causes
  // compile-time errors on AWS Graviton instances.
  // #pragma omp atomic update
  cellInfo->forbiddenProximityBarrier = mask  |  cellInfo->forbiddenProximityBarrier;

  // printf("DEBUG: Exiting function 'set_unwalkable_barrier_proximity';\n");
  // printf("DEBUG:   offset = %d, mask = %lu, forbiddenProximityBarrier = %lu\n",
  //         offset, mask, (long unsigned)cellInfo[x][y][z].forbiddenProximityBarrier);

}  // End of function 'set_unwalkable_barrier_proximity'


//-----------------------------------------------------------------------------
// Name: clear_unwalkable_barrier_proximity
// Desc: Modifies the 'forbiddenProximityBarrier' element of the 3D cellInfo
//       matrix. This function makes this cell walkable by clearing the flag
//       that causes the cell to be unwalkable due to proximity to a nearby,
//       user-defined obstacle/barrier. The shape-type and design-rule set
//       that will be permitted to be routed through this cell are given by
//       parameters 'shape_type' and 'DR_subset'.
//-----------------------------------------------------------------------------
static void clear_unwalkable_barrier_proximity(CellInfo_t *cellInfo, const int DR_subset, const int shape_type)  {

  // Calculate bit-offset from base address, based on the design-rule subset
  // and the shape type. The largest possible value of offset is 47, because
  // the maximum DR_subset is 15, and the maximum shape_type is 2.
  int offset = DR_subset * NUM_SHAPE_TYPES   +   shape_type;

  // Initialize 'mask' as 64-bit integer with binary '1' in the right-most position, i.e.:
  // binary '0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001'.
  // Then shift the '1' to the left by 'offset' bits:
  long unsigned int mask = 1 << offset;

  // Using the bitwise complement operator, invert the 1's to 0's and the 0's to 1's:
  mask = ~mask;

  // Logically AND the mask with with the existing value of 'forbiddenProximityBarrier'. The
  // 'pragma' line was intended to prevent the 'cellInfo' matrix from becoming corrupted if this function
  // is called simultaneously from multiple threads. However, this 'pragma' statement causes fatal
  // compile-time errors on AWS Graviton instances.
  // #pragma omp atomic update
  cellInfo->forbiddenProximityBarrier = mask  &  cellInfo->forbiddenProximityBarrier;

  // printf("DEBUG: Exiting function 'clear_unwalkable_barrier_proximity';\n");
  // printf("DEBUG:   offset = %d, mask = %lu, forbiddenProximityBarrier = %lu\n",
  //         offset, mask, (long unsigned)cellInfo->forbiddenProximityBarrier);

}  // End of function 'clear_unwalkable_barrier_proximity'


//-----------------------------------------------------------------------------
// Name: set_unwalkable_pinSwap_proximity
// Desc: Modifies the 'forbiddenProximityPinSwap' elements of the 3D cellInfo
//       matrix. This function flags this cell to be unwalkable due to
//       proximity to a nearby, user-defined pin-swap zone. The shape-type
//       and design-rule set that will be forbidden from routing through this
//       cell are given by parameters 'shape_type' and 'DR_subset'.
//-----------------------------------------------------------------------------
static void set_unwalkable_pinSwap_proximity(CellInfo_t *cellInfo, const int DR_subset,
                                             const int shape_type)  {

  // Calculate bit-offset from base address, based on the design-rule subset
  // and the shape type. The largest possible value of offset is 47, because
  // the maximum DR_subset is 15, and the maximum shape_type is 2.
  int offset = DR_subset * NUM_SHAPE_TYPES   +   shape_type;

  // Initialize 'mask' as 64-bit integer with binary '1' in the right-most position, i.e.:
  // binary '0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001'.
  // Then shift the '1' to the left by 'offset' bits:
  long unsigned int mask = 1 << offset;

  // Logically OR the mask with with the existing value of 'forbiddenProximityPinSwap'. The
  // 'pragma' line was intended to prevent the 'cellInfo' matrix from becoming corrupted if this function
  // is called simultaneously from multiple threads. However, using this 'pragma' statement causes
  // compile-time errors on AWS Graviton instances.
  // #pragma omp atomic update
  cellInfo->forbiddenProximityPinSwap = mask  |  cellInfo->forbiddenProximityPinSwap;

  // printf("DEBUG: Exiting function 'set_unwalkable_pinSwap_proximity';\n");
  // printf("DEBUG:   offset = %d, mask = %lu, forbiddenProximityPinSwap = %lu\n",
  //         offset, mask, (long unsigned)cellInfo->forbiddenProximityPinSwap);

}  // End of function 'set_unwalkable_pinSwap_proximity'


//-----------------------------------------------------------------------------
// Name: defineBarriers
// Desc: Modifies the 'cellInfo' 3D matrix based on the BLOCK/UNBLOCK
//       statements described in the 'user_inputs' data structure.
//-----------------------------------------------------------------------------
void defineBarriers(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                           InputValues_t *user_inputs)  {

  int num_block_statements = user_inputs->num_block_instructions;
  char command[maxBlockInstructionLength];
  char type[maxBlockInstructionLength];
  char layer_name[maxLayerNameLength];
  int params[maxBlockParameters]; // Array of parameters for a given BLOCK/UNBLOCK
                                  // statement, with values converted from microns to
                                  // to integer cell coordinates.

  int x, y, routing_layer_num; // Coordinates in 3D 'cellInfo' matrix
  int all_layer_num; // Z-coordinate in 3D space that includes all layers (routing and via layers)
  int isViaLayer; // = 1 if layer contains vias; =0 if layer is for routing
  int routingLayerAbove; // Index of routing layer above a given via layer
  int routingLayerBelow; // Index of routing layer below a given via layer
  int i;

  // printf("DEBUG: In function defineBarriers, num_block_statements=%d\n", num_block_statements);

  //
  // Cycle through the BLOCK/UNBLOCK statements and modify the 'cellInfo' matrix
  // accordingly. Note that the order of the BLOCK/UNBLOCK statements matters!!
  //
  for (int block_statement = 0; block_statement < num_block_statements; block_statement++)  {

    // To simplify coding, copy the command name, type, and layer to temporary variables.
    strcpy(command,    user_inputs->block_command[block_statement]);
    strcpy(type,       user_inputs->block_type[block_statement]);
    strcpy(layer_name, user_inputs->block_layer[block_statement]);
    // printf("DEBUG: In function defineBarriers, block_command[%d]=%s\n", block_statement, command);

    // If BLOCK/UNBLOCK statement contains numeric parameters, convert them from
    // floating-point values to integer cell coordinates:
    for (i = 0; i < user_inputs->block_num_params[block_statement]; i++)  {
      params[i] = (int)roundf(user_inputs->block_parameters[block_statement][i] / user_inputs->cell_size_um);
      // printf("DEBUG:   Parameter #%d is %d\n", i, params[i]);
    }

    //
    // Determine layer number from layer name:
    //
    all_layer_num = -1;
    for (i = 0; i < 2 * mapInfo->numLayers - 1; i++)  {
      if (strcasecmp(layer_name, user_inputs->layer_names[i]) == 0)  {
        all_layer_num = i;
        break;
      }  // End of if-statement
    }  // End of for-loop for index 'all_layer_num'

    // Confirm that layer name is a valid layer name:
    if (all_layer_num == -1)  {
      printf("\nERROR: Statement '%s %s %s...' in input file references layer '%s', which is\n",
              command, type, layer_name, layer_name);
      printf("       not defined as one of the valid layer names. Please fix input file.\n\n");
      exit(1);
    }
    // printf("DEBUG: Layer '%s' in BLOCK/UNBLOCK statement is mapped to layer '%d'\n", layer_name, all_layer_num);

    isViaLayer = all_layer_num % 2; // Odd-number layers are via layers; even-numbered are routing layers

    //
    // Handle 'BLOCK ALL', 'UNBLOCK ALL', 'BLOCK RECT', and 'UNBLOCK RECT' commands:
    //
    if ((strcasecmp(type, "ALL") == 0) || (strcasecmp(type, "RECT") == 0))  {
      int x1, y1, x2, y2;
      if (strcasecmp(type, "ALL") == 0) {
        x1 = 0;
        y1 = 0;
        x2 = mapInfo->mapWidth;
        y2 = mapInfo->mapHeight;
        // printf("DEBUG: Handling 'BLOCK ALL' statement...\n");
      }
      else {
        x1 = min(params[0], params[2]);  // x1 is x-coordinate of lower-left RECT corner
        y1 = min(params[1], params[3]);  // y1 is y-coordinate of lower-left RECT corner
        x2 = max(params[0], params[2]);  // x2 is x-coordinate of upper-right RECT corner
        y2 = max(params[1], params[3]);  // y2 is y-coordinate of upper-right RECT corner
        // printf("DEBUG: Handling 'BLOCK RECT' statement from (%d, %d) to (%d, %d), with all_layer_num = %d...\n",
        //         x1, y1, x2, y2, all_layer_num);
      }

      //
      // Raster over the rectangle of interest:
      //
      for (x = x1; x <= x2; x++)  {
        for (y = y1; y <= y2; y++)  {

          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // printf("DEBUG: Handling RECT shape at location (%d, %d).\n", x, y);

          if (isViaLayer)  {
            // Layer is a via layer, so modify the 'cellInfo' of routing layers directly
            // above and below the via layer.
            routingLayerAbove = (all_layer_num + 1) / 2;
            routingLayerBelow = (all_layer_num + 1) / 2  - 1;

            if (strcasecmp(command, "BLOCK") == 0)  {
              // printf("DEBUG: Prohibiting via-down and via-up at (%d,%d,%d) and (%d,%d,%d).\n",
              //              x, y, routingLayerBelow, x, y, routingLayerAbove);
              cellInfo[x][y][routingLayerBelow].forbiddenUpViaBarrier   = TRUE;
              cellInfo[x][y][routingLayerAbove].forbiddenDownViaBarrier = TRUE;
            }
            else if (strcasecmp(command, "UNBLOCK") == 0)  {
              //printf("DEBUG: Allowing via-down and via-up at (%d,%d,%d) and (%d,%d,%d).\n",
              //             x, y, routingLayerBelow, x, y, routingLayerAbove);
              cellInfo[x][y][routingLayerBelow].forbiddenUpViaBarrier   = FALSE;
              cellInfo[x][y][routingLayerAbove].forbiddenDownViaBarrier = FALSE;
            }
            else {
              printf("\nERROR: An unexpected BLOCK/UNBLOCK command of '%s' was encountered. Program is exiting.\n\n",
                      command);
              exit(1);
            }  // End of final else-clause for command not equal to BLOCK or UNBLOCK

          }  // End of if-clause for (isVia) in if/else statement

          else {
            // Layer is a routing layer (not a via layer):

            routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

            // Based on whether command is BLOCK or UNBLOCK, make the cell walkable or
            // unwalkable.
            if (strcasecmp(command, "BLOCK") == 0)  {
              // printf("DEBUG: Setting forbiddenTraceBarrier to TRUE at (%d,%d,%d).\n",
              //              x, y, routing_layer_num );
              cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier   = TRUE;
              cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier   = TRUE;
              cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier = TRUE;

              // Also modify the via-down/via-up flags for the cells above/beneath the current cell:
              if (routing_layer_num - 1 >= 0)
                cellInfo[x][y][routing_layer_num - 1].forbiddenUpViaBarrier   = TRUE;
              if (routing_layer_num + 1 < mapInfo->numLayers)
                cellInfo[x][y][routing_layer_num + 1].forbiddenDownViaBarrier = TRUE;
            }
            else if (strcasecmp(command, "UNBLOCK") == 0)  {
              // printf("DEBUG: Setting forbiddenTraceBarrier to FALSE at (%d,%d,%d).\n",
              //              x, y, routing_layer_num );
              cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier       = FALSE;
              cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier   = FALSE;
              cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier = FALSE;

              // Also modify the via-down/via-up flags for the cells above/beneath the current cell:
              if (routing_layer_num - 1 >= 0)
                cellInfo[x][y][routing_layer_num - 1].forbiddenUpViaBarrier   = FALSE;
              if (routing_layer_num + 1 < mapInfo->numLayers)
                cellInfo[x][y][routing_layer_num + 1].forbiddenDownViaBarrier = FALSE;
            }  // End of if/else-clause for command == 'UNBLOCK'
            else {
              printf("\nERROR: An unexpected BLOCK/UNBLOCK command of '%s' was encountered. Program is exiting.\n\n",
                      command);
              exit(1);
            }  // End of final else-clause for command not equal to BLOCK or UNBLOCK
          }  // End of else-clause
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'
    }  // End of if-clause for command type == 'ALL' or 'RECT'

    //
    // Handle 'BLOCK CIR' and 'UNBLOCK CIR' commands:
    //
    else if (strcasecmp(type, "CIR") == 0)  {
      // printf("DEBUG: Found CIR statement with parameters %d, %d, and %d\n",
      //        params[0], params[1], params[2]);

      int x_cent = params[0]; // X-coordinate of circle's center
      int y_cent = params[1]; // Y-coordinate of circle's center
      int radius = params[2]; // Radius of circle
      int radius_squared = radius * radius;

      // Raster over the square that circumscribes the circle:
      int x_min, y_min, x_max, y_max;
      x_min = x_cent - radius; // = Xo - R
      y_min = y_cent - radius; // = Yo - R
      x_max = x_cent + radius; // = Xo + R
      y_max = y_cent + radius; // = Yo + R
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Only proceed if X/Y point is within the circle: delta-X^2 + delta-Y^2 < R^2
          if ((x - x_cent)*(x - x_cent) + (y - y_cent)*(y - y_cent) <= radius_squared)  {

            if (isViaLayer)  {
              // Layer is a via layer, so modify the 'cellInfo' of routing layers directly
              // above and below the via layer.
              routingLayerAbove = (all_layer_num + 1) / 2;
              routingLayerBelow = (all_layer_num + 1) / 2  - 1;

              if (strcasecmp(command, "BLOCK") == 0)  {
                cellInfo[x][y][routingLayerBelow].forbiddenUpViaBarrier   = TRUE;
                cellInfo[x][y][routingLayerAbove].forbiddenDownViaBarrier = TRUE;
              }
              else if (strcasecmp(command, "UNBLOCK") == 0)  {
                cellInfo[x][y][routingLayerBelow].forbiddenUpViaBarrier   = FALSE;
                cellInfo[x][y][routingLayerAbove].forbiddenDownViaBarrier = FALSE;
              }  // End of if/else-clause for command == 'UNBLOCK'
              else {
                printf("\nERROR: An unexpected BLOCK/UNBLOCK command of '%s' was encountered. Program is exiting.\n\n",
                        command);
                exit(1);
              }  // End of final else-clause for command not equal to BLOCK or UNBLOCK

            }  // End of if-clause in if/else statement

            else {
              // Layer is a routing layer (not a via layer):

              routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

              // Based on whether command is BLOCK or UNBLOCK, make the cell walkable or
              // unwalkable.
              if (strcasecmp(command, "BLOCK") == 0)  {
                cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier   = TRUE;
                cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier   = TRUE;
                cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier = TRUE;

                // Also modify the via-down/via-up flags for the cells above/beneath the current cell:
                if (routing_layer_num - 1 >= 0)
                  cellInfo[x][y][routing_layer_num - 1].forbiddenUpViaBarrier   = TRUE;
                if (routing_layer_num + 1 < mapInfo->numLayers)
                  cellInfo[x][y][routing_layer_num + 1].forbiddenDownViaBarrier = TRUE;
              }
              else if (strcasecmp(command, "UNBLOCK") == 0)  {
                cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier   = FALSE;
                cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier   = FALSE;
                cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier = FALSE;

                // Also modify the via-down/via-up flags for the cells above/beneath the current cell:
                if (routing_layer_num - 1 >= 0)
                  cellInfo[x][y][routing_layer_num - 1].forbiddenUpViaBarrier   = FALSE;
                if (routing_layer_num + 1 < mapInfo->numLayers)
                  cellInfo[x][y][routing_layer_num + 1].forbiddenDownViaBarrier = FALSE;
              }  // End of if/else-clause for command == 'UNBLOCK'
              else {
                printf("\nERROR: An unexpected BLOCK/UNBLOCK command of '%s' was encountered. Program is exiting.\n\n",
                        command);
                exit(1);
              }  // End of final else-clause for command not equal to BLOCK or UNBLOCK

            }  // End of else-clause
          }  // End of if-block for X/Y point within circle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'CIR'

    //
    // Handle 'BLOCK TRI' and 'UNBLOCK TRI' commands:
    //
    else if (strcasecmp(type, "TRI") == 0)  {
      // printf("DEBUG: Found TRI statement with parameters %d, %d, %d, %d, %d, %d\n",
      //        params[0], params[1], params[2], params[3], params[4], params[5]);

      // Capture parameters in variables that describe the X/Y coordinates
      // of the triangle's 3 vertices: A, B, and C
      int x_A = params[0]; // X-coordinate of vertex 'A'
      int y_A = params[1]; // Y-coordinate of vertex 'A'
      int x_B = params[2]; // X-coordinate of vertex 'B'
      int y_B = params[3]; // Y-coordinate of vertex 'B'
      int x_C = params[4]; // X-coordinate of vertex 'C'
      int y_C = params[5]; // Y-coordinate of vertex 'C'

      // Determine the minimum and maximum X- and Y-values,
      // (x_min, y_min) and (x_max, y_max), respectively:
      int x_min = x_A; int y_min = x_A;
      int x_max = x_A; int y_max = x_A;
      if (x_B < x_min) x_min = x_B;
      if (x_C < x_min) x_min = x_C;
      if (x_B > x_max) x_max = x_B;
      if (x_C > x_max) x_max = x_C;
      if (y_B < y_min) y_min = y_B;
      if (y_C < y_min) y_min = y_C;
      if (y_B > y_max) y_max = y_B;
      if (y_C > y_max) y_max = y_C;
      // printf("DEBUG: Lower-left point is (%d,%d). Upper-right point is (%d,%d)\n",
      //         x_min, y_min, x_max, y_max);

      // Use Barycentric Technique to determine whether points are within
      // the triangle defined by vertices A, B, and C. This technique is
      // described at following web page:
      //    http://www.blackpawn.com/texts/pointinpoly/default.html
      int X_c_a = x_C - x_A; // X-component of vector from A to C
      int Y_c_a = y_C - y_A; // Y-component of vector from A to C
      int X_b_a = x_B - x_A; // X-component of vector from A to B
      int Y_b_a = y_B - y_A; // Y-component of vector from A to B

      // Calculate dot-products of vectors. Use long long integers to avoid
      // truncating. The dot-product values can get large.
      long long dot_ca_ca = (X_c_a * X_c_a) + (Y_c_a * Y_c_a); // Dot-product of vector CA with itself
                                                          // ('dot00' from above web page)
      long long dot_ca_ba = (X_c_a * X_b_a) + (Y_c_a * Y_b_a); // Dot-product of vector CA with vector
                                                          // BA ('dot01' from above web page)
      long long dot_ba_ba = (X_b_a * X_b_a) + (Y_b_a * Y_b_a); // Dot-product of vector BA with itself
                                                         // ('dot11' from above web page)
      int X_p_a, Y_p_a; // X- and Y-components of vector from A to point P
      long long dot_ca_pa;    // Dot product of vector CA with vector PA
      long long dot_ba_pa;    // Dot product of vector BA with vector PA
      float u, v, denominator; // Vectors u and v, as defined in above web page.
                               // 'denominator is temporary variable.

      // Raster over the rectangle that circumscribes the triangle:
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Calculate more vector quantities necessary to determine whether point P
          // is inside of triangle:
          X_p_a = x - x_A; // X-component of vector from P to A
          Y_p_a = y - y_A; // Y-component of vector from P to A
          dot_ca_pa = (X_c_a * X_p_a) + (Y_c_a * Y_p_a); // Dot product of vector CA with vector PA
                                                         // ('dot02' from above web page)
          dot_ba_pa = (X_b_a * X_p_a) + (Y_b_a * Y_p_a); // Dot product of vector BA with vector PA
                                                         // ('dot12' from above web page)
          // printf("DEBUG: dot_ca_ca=%lli, dot_ba_ba=%lli, dot_ca_ba=%lli, dot_ca_ba^2=%lli\n",
          //        dot_ca_ca, dot_ba_ba, dot_ca_ba, dot_ca_ba*dot_ca_ba);

          denominator = (float)(dot_ca_ca * dot_ba_ba - dot_ca_ba * dot_ca_ba);
          u = (float)((dot_ba_ba * dot_ca_pa) - (dot_ca_ba * dot_ba_pa)) / denominator;
          v = (float)((dot_ca_ca * dot_ba_pa) - (dot_ca_ba * dot_ca_pa)) / denominator;
          // printf("DEBUG: At point (%d,%d), denominator=%5.2f, u=%5.2f, v=%5.2f, u+v=%5.2f\n",
          //        x, y, denominator, u, v, u+v);

          // Only proceed if point P at (x,y) is within the triangle. This is true only
          // if (u >= 0) and (v >= 0) and (u + v < 1).
          if ((u >= 0.0) && (v >= 0.0) && (u + v < 1.0)) {
            // printf("DEBUG: Point (%d,%d) is within triangle.\n", x, y);

            if (isViaLayer)  {
              // Layer is a via layer, so modify the 'cellInfo' of routing layers directly
              // above and below the via layer.
              routingLayerAbove = (all_layer_num + 1) / 2;
              routingLayerBelow = (all_layer_num + 1) / 2  - 1;

              if (strcasecmp(command, "BLOCK") == 0)  {
                cellInfo[x][y][routingLayerBelow].forbiddenUpViaBarrier   = TRUE;
                cellInfo[x][y][routingLayerAbove].forbiddenDownViaBarrier = TRUE;
              }
              else if (strcasecmp(command, "UNBLOCK") == 0)  {
                cellInfo[x][y][routingLayerBelow].forbiddenUpViaBarrier   = FALSE;
                cellInfo[x][y][routingLayerAbove].forbiddenDownViaBarrier = FALSE;
              }  // End of if/else-clause for command == 'UNBLOCK'
              else {
                printf("\nERROR: An unexpected BLOCK/UNBLOCK command of '%s' was encountered. Program is exiting.\n\n",
                        command);
                exit(1);
              }  // End of final else-clause for command not equal to BLOCK or UNBLOCK

            }  // End of if-clause in if/else statement

            else {
              // Layer is a routing layer (not a via layer):

              routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

              // Based on whether command is BLOCK or UNBLOCK, make the cell walkable or
              // unwalkable.
              if (strcasecmp(command, "BLOCK") == 0)  {
                cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier   = TRUE;
                cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier   = TRUE;
                cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier = TRUE;

                // Also modify the via-down/via-up flags for the cells above/beneath the current cell:
                if (routing_layer_num - 1 >= 0)
                  cellInfo[x][y][routing_layer_num - 1].forbiddenUpViaBarrier   = TRUE;
                if (routing_layer_num + 1 < mapInfo->numLayers)
                  cellInfo[x][y][routing_layer_num + 1].forbiddenDownViaBarrier = TRUE;
              }
              else if (strcasecmp(command, "UNBLOCK") == 0)  {
                cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier   = FALSE;
                cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier   = FALSE;
                cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier = FALSE;

                // Also modify the via-down/via-up flags for the cells above/beneath the current cell:
                if (routing_layer_num - 1 >= 0)
                  cellInfo[x][y][routing_layer_num - 1].forbiddenUpViaBarrier   = FALSE;
                if (routing_layer_num + 1 < mapInfo->numLayers)
                  cellInfo[x][y][routing_layer_num + 1].forbiddenDownViaBarrier = FALSE;
              }  // End of if/else-clause for command == 'UNBLOCK'
              else {
                printf("\nERROR: An unexpected BLOCK/UNBLOCK command of '%s' was encountered. Program is exiting.\n\n",
                        command);
                exit(1);
              }  // End of final else-clause for command not equal to BLOCK or UNBLOCK

            }  // End of else-clause
          }  // End of if-block for X/Y point within triangle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'TRI'

    else {
      printf("\n\nERROR: Program encountered a BLOCK/UNBLOCK command of type '%s' that is not\n", type);
      printf("       recognized. Allowed types are ALL, RECT, CIR, and TRI (case insensitive).\n");
      printf("       Please fix input file. Program is exiting.\n\n");
      exit(1);
    }

  }  // End of for-loop for index 'block_statement'

  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);
  printf("INFO: Completed process of identifying cells near unwalkable zones at %02d-%02d-%d, %02d:%02d.\n",
         now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min);

}  // End of function 'defineBarriers'


//-----------------------------------------------------------------------------
// Name: defineProximityZones
// Desc: Cells are made unwalkable a half-linewidth or via radius away from
//       (1) user-defined barriers, and (2) the perimeter of the map, and
//       (3) pin-swap zones. Cells in pin-swap zones are never part of a
//       proximity zone.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_defineProximityZones' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_defineProximityZones 1
#undef DEBUG_defineProximityZones

void defineProximityZones(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                          InputValues_t *user_inputs)  {

  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);
  printf("\nINFO: Starting process of identifying cells near unwalkable zones at %02d-%02d-%d, %02d:%02d.\n",
         now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min);

  //
  // Raster over all points in the 3D matrix to determine which should be defined
  // as unwalkable:
  //
  for (int routing_layer_num = 0; routing_layer_num < mapInfo->numLayers; routing_layer_num++)  {
    //
    // The following for-loops are performed in multiple, parallel threads if the hardware
    // permits. Collapse the nested loops for indices y and x to apply parallel threading:
    //
    #pragma omp parallel for collapse(2) schedule(dynamic, 1)
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      for (int x = 0; x < mapInfo->mapWidth; x++)  {


        #ifdef DEBUG_defineProximityZones
        //
        // Check if the current parameters satisfy specific requirements. If so, then set the
        // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
        int DEBUG_ON = FALSE;
        if ((x == 40) && (y == 85) && (routing_layer_num == 4))  {
          printf("\n\nDEBUG: (thread %2d) Setting DEBUG_ON to TRUE in defineProximityZones() because specific requirements were met.\n\n", omp_get_thread_num());
          DEBUG_ON = TRUE;
        }  // End of if-block for determining whether to set DEBUG_ON to TRUE
        #endif

        // The 'pragma' statement was intended to avoid problems with simultaneously reading the
        // cellInfo matrix from multiple threads. However, this 'pragma' statement causes fatal
        // compile-time errors on AWS Graviton instances:
        // #pragma omp atomic read
        int already_unwalkable = cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier;
        int in_swap_zone       = cellInfo[x][y][routing_layer_num].swap_zone;
        if (already_unwalkable || in_swap_zone) {
          #ifdef DEBUG_defineProximityZones
          if (DEBUG_ON)  {
            printf("\nDEBUG: (thread %2d) Skipping (%d, %d, %d) because it's an unwalkable barrier or a pin-swap zone.\n", omp_get_thread_num(), x, y, routing_layer_num);
            printf("DEBUG: (thread %2d)           forbiddenTraceBarrier = %d\n", omp_get_thread_num(), cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier);
            printf("DEBUG: (thread %2d)           forbiddenUpViaBarrier = %d\n", omp_get_thread_num(), cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier);
            printf("DEBUG: (thread %2d)         forbiddenDownViaBarrier = %d\n", omp_get_thread_num(), cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier);
            printf("DEBUG: (thread %2d)                       swap_zone = %d\n\n", omp_get_thread_num(), cellInfo[x][y][routing_layer_num].swap_zone);
          }
          #endif

          continue;
        }
        else  {

          // Get design-rule number for this (x,y,z):
          int DR_num = cellInfo[x][y][routing_layer_num].designRuleSet;

          #ifdef DEBUG_defineProximityZones
          if (DEBUG_ON)  {
            printf("DEBUG: (thread %2d) At (%d, %d, %d), design-rule set number is %d.\n", omp_get_thread_num(), x, y,
                   routing_layer_num, DR_num);
          }
          #endif

          // Iterate over the design-rule subsets for this design rule:
          for (int DR_subset = 0; DR_subset < user_inputs->numDesignRuleSubsets[DR_num]; DR_subset++ )  {

            #ifdef DEBUG_defineProximityZones
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) Defining proximity barriers in function 'defineProximityZones'...\n", omp_get_thread_num());
              printf("DEBUG: (thread %2d)   DR_num = %d ('%s'), DR_subset = %d ('%s')\n", omp_get_thread_num(), DR_num,
                      user_inputs->designRuleSetName[DR_num], DR_subset, user_inputs->designRules[DR_num][DR_subset].subsetName);
            }
            #endif


            // Calculate the user-defined half-width and via radius values for this cell and design-rule
            // subset. Note that the calculations depend on whether the design-rule subset is used for pseudo-paths:
            int half_width, via_up_radius, via_down_radius, half_width_squared, via_up_radius_squared, via_down_radius_squared;
            if (! user_inputs->designRules[DR_num][DR_subset].isPseudoNetSubset)  {
              // We got here, so the design-rule subset is not used for pseudo-paths:

              half_width      = (int)floor(0.5 * user_inputs->designRules[DR_num][DR_subset].lineWidthMicrons       / user_inputs->cell_size_um);
              via_up_radius   = (int)floor(0.5 * user_inputs->designRules[DR_num][DR_subset].viaUpDiameterMicrons   / user_inputs->cell_size_um);
              via_down_radius = (int)floor(0.5 * user_inputs->designRules[DR_num][DR_subset].viaDownDiameterMicrons / user_inputs->cell_size_um);

            }  // End of if-block for handling a design-rule subset that's NOT used for pseudo-paths
            else  {
              // We got here, so the design-rule subset is used for pseudo-paths. Special calculations
              // therefore go into calculating the half-widths and radius values of the pseudo-path:
              //          Pseudo half-width = 0.5 * diff-pair pitch + linewidth of a single trace
              //       Pseudo via-up radius = 0.5 * max(2*Dvu + Svu, diff-pair pitch + linewidth of a single trace)
              //     Pseudo via-down radius = 0.5 * max(2*Dvd + Svd, diff-pair pitch + linewidth of a single trace)

              //
              // Calculate the half-width of a pseudo-trace:
              //
              // If the diff-pair linewidth is non-zero, then define the pseudo-net's radius as half the
              // diff-pair linewidth plus half the diff-pair pitch:
              if (user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons > 0)  {
                half_width = (int)floor( 0.5 * (  user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons
                                         + user_inputs->designRules[DR_num][DR_subset].traceDiffPairPitchMicrons)
                                         / user_inputs->cell_size_um);
              }
              // If the diff-pair linewidth is zero (not realistic), then define the pseudo-net's radius
              // as half the the diff-pair pitch plus one cell.
              else  {
                half_width = 1 + (int)floor(0.5 * user_inputs->designRules[DR_num][DR_subset].traceDiffPairPitchMicrons / user_inputs->cell_size_um);
              }

              //
              // Calculate the radius of an upward-going pseudo-via:
              //
              // Recall that the pseudo via-up diameter = max(2*Dvu + Svu, diff-pair pitch + linewidth).
              //
              // Find the maximum of (2*Dvu + Svu) and (diff-pair pitch + linewidth of a single trace)
              {
                float temp_1 = 2.0 * user_inputs->designRules[DR_num][DR_subset].copy_viaUpDiameterMicrons
                                   + user_inputs->designRules[DR_num][DR_subset].viaUpToViaUpSpacingMicrons;
                float temp_2 =   user_inputs->designRules[DR_num][DR_subset].traceDiffPairPitchMicrons
                               + user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons;
                via_up_radius = (int)floor(0.5 * max(temp_1, temp_2) / user_inputs->cell_size_um);
              }


              //
              // Calculate the radius of a downward-going pseudo-via:
              //
              // Recall that the pseudo via-down diameter = max(2*Dvd + Svd, diff-pair pitch + linewidth).
              //
              // Find the maximum of (2*Dvd + Svd) and (diff-pair pitch + linewidth of a single trace)
              {
                float temp_1 = 2.0 * user_inputs->designRules[DR_num][DR_subset].copy_viaDownDiameterMicrons
                                   + user_inputs->designRules[DR_num][DR_subset].viaDownToViaDownSpacingMicrons;
                float temp_2 =   user_inputs->designRules[DR_num][DR_subset].traceDiffPairPitchMicrons
                               + user_inputs->designRules[DR_num][DR_subset].copy_lineWidthMicrons;
                via_down_radius = (int)floor(0.5 * max(temp_1, temp_2) / user_inputs->cell_size_um);
              }

            }  // End of else-block for handling a design-rule subset that's used for pseudo-paths

            // Calculate the squares of the half-widths and radius values:
            half_width_squared      = half_width      * half_width;
            via_up_radius_squared   = via_up_radius   * via_up_radius;
            via_down_radius_squared = via_down_radius * via_down_radius;


            #ifdef DEBUG_defineProximityZones
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) At (x,y,z) = (%d, %d, %d):\n", omp_get_thread_num(), x, y, routing_layer_num);
              printf("       (thread %2d) half_width      = %d cells\n", omp_get_thread_num(), half_width);
              printf("       (thread %2d) via_up_radius   = %d cells\n", omp_get_thread_num(), via_up_radius);
              printf("       (thread %2d) via_down_radius = %d cells\n", omp_get_thread_num(), via_down_radius);

              printf("DEBUG: (thread %2d) At (x,y,z) = (%d, %d, %d):\n", omp_get_thread_num(), x, y, routing_layer_num);
              printf("       (thread %2d) half_width_squared      = %d cells^2\n", omp_get_thread_num(), half_width_squared);
              printf("       (thread %2d) via_up_radius_squared   = %d cells^2\n", omp_get_thread_num(), via_up_radius_squared);
              printf("       (thread %2d) via_down_radius_squared = %d cells^2\n", omp_get_thread_num(), via_down_radius_squared);
            }
            #endif

            // Calculate the maximum distance value among the half_width, via_up_radius,
            // and via_down_radius:
            float radius = half_width;
            if (via_up_radius   > radius)
              radius = via_up_radius;
            if (via_down_radius > radius)
              radius = via_down_radius;
            int radius_squared = radius * radius;

            //
            // Raster over the areas around (x,y). The variables 'x_prime' and
            // 'y_prime' represent the coordinates of the points around (x,y).
            //
            for (int y_prime = y - radius; y_prime <= y + radius; y_prime++)  {
              int delta_y_squared = (y_prime-y)*(y_prime-y);
              for (int x_prime = x - radius; x_prime <= x + radius; x_prime++)  {

                // Define square of distance between (x,y) and (x_prime, y_prime):
                int distance_squared = (x_prime-x)*(x_prime-x) + delta_y_squared;

                // Skip this point if it's farther from (x,y) than distance 'radius':
                if (distance_squared > radius_squared)
                  continue;

                // Check if (x',y') is outside of the map:
                if (XY_coords_are_outside_of_map(x_prime, y_prime, mapInfo))  {

                  // If out-of-map cell is within a distance 'half_width' of (x,y)
                  // then make (x,y) 'unWalkableProximity':
                  if (distance_squared <= half_width_squared)  {
                    set_unwalkable_barrier_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, TRACE);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                    	printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for TRACE shape-types due to proximity to map edge for design-rule subset %d\n",
                                omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                    }
                    #endif
                  }  // End of if-block for dist^2 < half_width_squared

                  // If out-of-map cell is  within a distance 'viaUpRadiusCells' of (x,y)
                  // then make (x,y) 'forbiddenUpViaProximity':
                  if (distance_squared <= via_up_radius_squared)  {
                    set_unwalkable_barrier_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, VIA_UP);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
	                  printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for VIA_UP shape-types due to proximity to map edge for design-rule subset %d\n",
                              omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                    }
                    #endif
                  }  // End of if-block for dist^2 < via_up_radius_squared

                  // If out-of-map cell is  within a distance 'viaDownRadiusCells' of (x,y)
                  // then make (x,y) 'forbiddenDownViaProximity':
                  if (distance_squared <= via_down_radius_squared)  {
                    set_unwalkable_barrier_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, VIA_DOWN);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for VIA_DOWN shape-types due to proximity to map edge for design-rule subset %d\n",
                             omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                    }
                    #endif
                  }  // End of if-block for dist^2 < via_down_radius_squared

                  continue; // Move on to next (x',y').
                }  // End of if-block for (x',y') being outside of map

                // If (x,y) is within distance 'half_width' of (x',y'), and if (x',y') is forbiddenTraceBarrier
                // or in a swap-zone, then make (x,y) unwalkable due to proximity to these zones:
                if (distance_squared <= half_width_squared)  {
                  if (cellInfo[x_prime][y_prime][routing_layer_num].forbiddenTraceBarrier)  {
                    set_unwalkable_barrier_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, TRACE);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for TRACE shape-types due to proximity to user-defined barrier for design-rule subset %d\n",
                             omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                    }
                    #endif
                  }  // End of if-block for (x',y') being in forbiddenTraceBarrier zone

                  if (cellInfo[x_prime][y_prime][routing_layer_num].swap_zone)  {
                    set_unwalkable_pinSwap_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, TRACE);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for TRACE shape-types due to proximity to user-defined swap-zone for design-rule subset %d\n",
                             omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                    }
                    #endif
                  }  // End of if-block for (x',y') being in pin-swap zone
                }  // End of if-block for dist^2 < half_width^2

                // If (x,y) is within distance 'via_up_radius' of (x',y'), and if (x',y') is forbiddenUpViaBarrier
                // or in a swap-zone, then make (x,y) unwalkable for VIA_UP due to proximity to these zones.
                if (distance_squared <= via_up_radius_squared)  {
                  if (cellInfo[x_prime][y_prime][routing_layer_num].forbiddenUpViaBarrier)  {
                    set_unwalkable_barrier_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, VIA_UP);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for VIA_UP shape-types due to proximity to user-defined barrier for design-rule subset %d\n",
                             omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                      printf("DEBUG: (thread %2d)    distance_squared = %d to (%d,%d), via_up_radius_squared = %.3f\n",
                             omp_get_thread_num(), distance_squared, x_prime, y_prime, via_up_radius_squared);
                    }
                    #endif
                  }  // End of if-block for (x',y') being in forbiddenUpViaBarrier zone

                  if (cellInfo[x_prime][y_prime][routing_layer_num].swap_zone)  {
                    set_unwalkable_pinSwap_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, VIA_UP);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for VIA_UP shape-types due to proximity to user-defined swap-zone for design-rule subset %d\n",
                              omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                    }
                    #endif
                  }  // End of if-block for (x',y') being in pin-swap zone
                }  // End of if-block for dist^2 < via_up_radius^2

                // If (x,y) is within distance 'via_down_radius' of (x',y'), and if (x',y') is forbiddenDownViaBarrier
                // or in a swap-zone, then make (x,y) unwalkable for VIA_DOWN due to proximity to these zones:
                if (distance_squared <= via_down_radius_squared)  {
                  if (cellInfo[x_prime][y_prime][routing_layer_num].forbiddenDownViaBarrier)  {
                    set_unwalkable_barrier_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, VIA_DOWN);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for VIA_DOWN shape-types due to proximity to user-defined barrier for design-rule subset %d\n",
                             omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                      // printf("DEBUG: (thread %2d)    distance_squared = %d to (%d,%d), via_down_radius_squared = %.3f\n",
                      //        omp_get_thread_num(), distance_squared, x_prime, y_prime, via_down_radius_squared);
                    }
                    #endif
                  }  // End of if-block for (x',y') being in forbiddenDownViaBarrier zone

                  if (cellInfo[x_prime][y_prime][routing_layer_num].swap_zone)  {
                    set_unwalkable_pinSwap_proximity(&(cellInfo[x][y][routing_layer_num]), DR_subset, VIA_DOWN);

                    #ifdef DEBUG_defineProximityZones
                    if (DEBUG_ON)  {
                      printf("DEBUG: (thread %2d) Cell at (%d,%d,%d) is set to unwalkable for VIA_DOWN shape-types due to proximity to user-defined swap-zone for design-rule subset %d\n",
                             omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
                    }
                    #endif
                  }  // End of if-block for (x',y') being in pin-swap zone
                }  // End of if-block for dist^2 < via_down_radius^2

              }  // End of for-loop for variable 'x_prime'
            }  // End of for-loop for variable 'y_prime'

            #ifdef DEBUG_defineProximityZones
            if (DEBUG_ON)  {
              printf("DEBUG: (thread %2d) At (%d, %d, %d) for design-rule subset %d:\n", omp_get_thread_num(), x, y, routing_layer_num, DR_subset);
              printf("DEBUG: (thread %2d)                 forbiddenTraceBarrier = %d\n", omp_get_thread_num(), cellInfo[x][y][routing_layer_num].forbiddenTraceBarrier);
              printf("DEBUG: (thread %2d)                 forbiddenUpViaBarrier = %d\n", omp_get_thread_num(), cellInfo[x][y][routing_layer_num].forbiddenUpViaBarrier);
              printf("DEBUG: (thread %2d)               forbiddenDownViaBarrier = %d\n", omp_get_thread_num(), cellInfo[x][y][routing_layer_num].forbiddenDownViaBarrier);
            }
            #endif

          }  // End of for-loop for index 'DR_subset'
        }  // End of if/else-block for (already_unwalkable)
      }  // End of for-loop for coordinate 'x'
    }  // End of for-loop for coordinate 'y'
    //
    // End of parallel processing.
    //
    tim = time(NULL);
    now = localtime(&tim);
    printf("INFO: Done with layer #%d of %d ('%s') at %02d-%02d-%d, %02d:%02d:%02d.\n", routing_layer_num, mapInfo->numLayers - 1,
           user_inputs->routingLayerNames[routing_layer_num], now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  }  // End of for-loop for routing_layer_num

  tim = time(NULL);
  now = localtime(&tim);
  printf("INFO: Completed process of identifying cells near unwalkable zones at %02d-%02d-%d, %02d:%02d.\n",
         now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min);

}  // End of function 'defineProximityZones'


//-----------------------------------------------------------------------------
// Name: defineCellDesignRules
// Desc: Modifies the 'cellInfo' 3D matrix based on the DR_zone
//       statements described in the 'user_inputs' data structure. 
//
//       This function defines the design-rule set number for each cell
//       in the map. If no design-rule zones were defined by the user,
//       then all cells are assigned the default design-rule set
//       of zero.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_defineCellDesignRules' and re-compile if you want verbose
// debugging print-statements enabled:
//
// #define DEBUG_defineCellDesignRules 1
#undef DEBUG_defineCellDesignRules

void defineCellDesignRules(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                           InputValues_t *user_inputs)  {

  int num_DR_zones = user_inputs->num_DR_zones;
  char DR_name[maxDesRuleSetNameLength];  // Name of design-rule set
  char shape[maxDRzoneShapeLength];       // Shape for a DR zone (e.g., CIR, RECT)
  char layer_name[maxLayerNameLength];    // Routing layer name in a DR_zone command
  int params[maxBlockParameters]; // Array of parameters for a given DR_zone
                                  // statement, with values converted from microns to
                                  // to integer cell coordinates.

  int x, y, routing_layer_num; // Coordinates in 3D 'cellInfo' matrix
  int all_layer_num; // Z-coordinate in 3D space that includes all layers (routing and via layers)
  int isViaLayer; // = 1 if layer contains vias; =0 if layer is for routing
  int i;

  #ifdef DEBUG_defineCellDesignRules
  printf("DEBUG: In function defineCellDesignRules, num_DR_zones = %d\n", num_DR_zones);
  #endif

  // Handle case where user supplied a design-rule set, but did not define
  // any design-rule zones. In this case, all cells should be assigned
  // to design-rule set #0:
  if (num_DR_zones == 0)  {
    #ifdef DEBUG_defineCellDesignRules
    printf("DEBUG:  Setting ALL cells' design-rule number to zero...\n\n");
    #endif
    for (routing_layer_num = 0; routing_layer_num < mapInfo->numLayers; routing_layer_num++)  {
      for (x = 0; x < mapInfo->mapWidth; x++)  {
        for (y = 0; y < mapInfo->mapHeight; y++)  {
          cellInfo[x][y][routing_layer_num].designRuleSet = 0;
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'
    }  // End of for-loop for index 'routing_layer_num'
    return;
  }  // End of if-block for (num_DR_zones == 0)
  

  // 
  // Cycle through the DR_zone statements and modify the 'cellInfo' matrix
  // accordingly. Note that the order of the DR_zone statements matters!!
  //
  for (int DR_zone = 0; DR_zone < num_DR_zones; DR_zone++)  {

    // To simplify coding, copy the design-rule name, shape, and layer to temporary variables.
    strcpy(DR_name,    user_inputs->DR_zone_name[DR_zone]);
    strcpy(shape,      user_inputs->DR_zone_shape[DR_zone]);
    strcpy(layer_name, user_inputs->DR_zone_layer[DR_zone]);

    // If DR_zone statement contains numeric parameters, convert them from  
    // floating-point values to integer cell coordinates:
    for (i = 0; i < user_inputs->DR_zone_num_params[DR_zone]; i++)  {
      #ifdef DEBUG_defineCellDesignRules
      printf("DEBUG:   user_inputs->DR_zone_parameters[%d][%d] is %5.2f\n", DR_zone, i,
              user_inputs->DR_zone_parameters[DR_zone][i]);
      #endif
      params[i] = (int)roundf(user_inputs->DR_zone_parameters[DR_zone][i] / user_inputs->cell_size_um);
      #ifdef DEBUG_defineCellDesignRules
      printf("DEBUG:   Parameter #%d is %d\n", i, params[i]);
      #endif
    }

    //
    // Determine layer number from layer name:
    //
    all_layer_num = -1;
    for (i = 0; i < 2 * mapInfo->numLayers - 1; i++)  {
      if (strcasecmp(layer_name, user_inputs->layer_names[i]) == 0)  {
        all_layer_num = i;
        break;
      }  // End of if-statement
    }  // End of for-loop for index 'all_layer_num'

    // Confirm that layer name is a valid layer name:
    if (all_layer_num == -1)  {
      printf("\nERROR: Statement 'DR_zone %s %s %s...' in input file references layer '%s', which is\n",
              DR_name, layer_name, shape, layer_name);
      printf("       not defined as one of the valid layer names. Please fix input file.\n\n");
      exit(1);
    }
    #ifdef DEBUG_defineCellDesignRules
    printf("DEBUG: Layer '%s' in DR_zone statement is mapped to layer '%d'\n", layer_name, all_layer_num);
    #endif

    // Confirm that the DR_zone statement refers to a routing layer rather than a via layer. If it refers
    // to a via layer, then issue an error message and terminate.
    isViaLayer = all_layer_num % 2; // Odd-number layers are via layers; even-numbered are routing layers
    if (isViaLayer)  {
      printf("\nERROR: A 'DR_zone' statement in the input file refers to a via layer. DR_zone statements\n");
      printf("       may only refer to routing layers. Fix the input file and re-start the program.\n\n");
      exit(1);
    }  // End of if-block for (isViaLayer)


    //
    // Determine design-rule set number from design-rule name:
    //
    int DR_set_number = -1;
    for (i = 0; i < user_inputs->numDesignRuleSets; i++)  {
      // printf("DEBUG: Comparing '%s' to '%s'...\n", DR_name, user_inputs->designRuleSetName[i]);
      if (strcasecmp(DR_name, user_inputs->designRuleSetName[i]) == 0)  {
        DR_set_number = i;
        // printf("DEBUG:      ...MATCH!\n");
        break;
      }  // End of if-statement
      else  {
        // printf("DEBUG:      ...Did NOT match!\n");
      }
    }  // End of for-loop for index 'i'

    // Confirm that design-rule name from 'DR_zone' statement is a valid design-rule name:
    if (DR_set_number == -1)  {
      printf("\nERROR: Statement 'DR_zone %s %s %s...' in input file references design-rule set '%s', which is\n",
              DR_name, layer_name, shape, DR_name);
      printf("       not defined as one of the valid design-rule set names. Please fix input file.\n\n");
      exit(1);
    }
    #ifdef DEBUG_defineCellDesignRules
    printf("DEBUG: Design-rule set name '%s' in DR_zone statement is mapped to design-rule set #%d\n", DR_name, DR_set_number);
    #endif


    //
    // Handle 'DR_zone <DR name> <layer> ALL' and 'DR_zone <DR name> <layer> RECT' commands:
    //
    if ((strcasecmp(shape, "ALL") == 0) || (strcasecmp(shape, "RECT") == 0))  {
      int x1, y1, x2, y2;
      if (strcasecmp(shape, "ALL") == 0) {
        x1 = 0;
        y1 = 0;
        x2 = mapInfo->mapWidth;
        y2 = mapInfo->mapHeight;
      }
      else {
        x1 = min(params[0], params[2]);  // x1 is x-coordinate of lower-left RECT corner
        y1 = min(params[1], params[3]);  // y1 is y-coordinate of lower-left RECT corner 
        x2 = max(params[0], params[2]);  // x2 is x-coordinate of upper-right RECT corner
        y2 = max(params[1], params[3]);  // y2 is y-coordinate of upper-right RECT corner
      }


      //
      // Raster over the rectangle of interest:
      //
      routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number
      // printf("DEBUG: About to raster over RECT shape from (%d, %d, %d) to (%d, %d, %d).\n", x1, y1, routing_layer_num, x2, y2, routing_layer_num);
      for (x = x1; x <= x2; x++)  {
        for (y = y1; y <= y2; y++)  {

          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // printf("DEBUG: About to assign designRuleSet #%d to location (%d, %d, %d) within a rectangle.\n", DR_set_number, x, y, routing_layer_num);

          // Define this cell's desgn-rule set number:
          cellInfo[x][y][routing_layer_num].designRuleSet = DR_set_number;

        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'
    }  // End of if-clause for command type == 'ALL' or 'RECT'

    //
    // Handle 'DR_zone <DR name> <layer> CIR' command:
    //
    else if (strcasecmp(shape, "CIR") == 0)  {
      #ifdef DEBUG_defineCellDesignRules
      printf("DEBUG: Found CIR statement with parameters %d, %d, and %d\n",
             params[0], params[1], params[2]);
      #endif

      int x_cent = params[0]; // X-coordinate of circle's center
      int y_cent = params[1]; // Y-coordinate of circle's center
      int radius = params[2]; // Radius of circle
      int radius_squared = radius * radius;

      // Raster over the square that circumscribes the circle:
      int x_min, y_min, x_max, y_max;
      x_min = x_cent - radius; // = Xo - R
      y_min = y_cent - radius; // = Yo - R
      x_max = x_cent + radius; // = Xo + R
      y_max = y_cent + radius; // = Yo + R
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Only proceed if X/Y point is within the circle: delta-X^2 + delta-Y^2 < R^2
          if ((x - x_cent)*(x - x_cent) + (y - y_cent)*(y - y_cent) <= radius_squared)  {

            // Layer is a routing layer (not a via layer):
            routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

            // Define this cell's desgn-rule set number:
            cellInfo[x][y][routing_layer_num].designRuleSet = DR_set_number;

            #ifdef DEBUG_defineCellDesignRules
            int x_window_min = 114;
            int x_window_max = 116;
            int y_window_min = 104;
            int y_window_max = 106;
            int z_window_min = 0;
            int z_window_max = 2;
            if (   (x >= x_window_min) && (y >= y_window_min) && (routing_layer_num >= z_window_min)
                && (x <= x_window_max) && (y <= y_window_max) && (routing_layer_num <= z_window_max))  {
              printf("DEBUG: Assigned designRuleSet #%d to location (%d,%d,%d) within a circle.\n", DR_set_number, x, y, routing_layer_num);
            }
            #endif

          }  // End of if-block for X/Y point within circle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'CIR'

    //
    // Handle 'DR_zone <DR name> <layer> TRI' command:
    //
    else if (strcasecmp(shape, "TRI") == 0)  {
      #ifdef DEBUG_defineCellDesignRules
      printf("DEBUG: Found TRI statement with parameters %d, %d, %d, %d, %d, %d\n",
             params[0], params[1], params[2], params[3], params[4], params[5]);
      #endif

      // Capture parameters in variables that describe the X/Y coordinates
      // of the triangle's 3 vertices: A, B, and C
      int x_A = params[0]; // X-coordinate of vertex 'A'
      int y_A = params[1]; // Y-coordinate of vertex 'A'
      int x_B = params[2]; // X-coordinate of vertex 'B'
      int y_B = params[3]; // Y-coordinate of vertex 'B'
      int x_C = params[4]; // X-coordinate of vertex 'C'
      int y_C = params[5]; // Y-coordinate of vertex 'C'

      // Determine the minimum and maximum X- and Y-values,
      // (x_min, y_min) and (x_max, y_max), respectively:
      int x_min = x_A; int y_min = x_A;
      int x_max = x_A; int y_max = x_A;
      if (x_B < x_min) x_min = x_B;
      if (x_C < x_min) x_min = x_C;
      if (x_B > x_max) x_max = x_B;
      if (x_C > x_max) x_max = x_C;
      if (y_B < y_min) y_min = y_B;
      if (y_C < y_min) y_min = y_C;
      if (y_B > y_max) y_max = y_B;
      if (y_C > y_max) y_max = y_C;
      #ifdef DEBUG_defineCellDesignRules
      printf("DEBUG: Lower-left point is (%d,%d). Upper-right point is (%d,%d)\n",
              x_min, y_min, x_max, y_max);
      #endif

      // Use Barycentric Technique to determine whether points are within
      // the triangle defined by vertices A, B, and C. This technique is
      // described at following web page: 
      //    http://www.blackpawn.com/texts/pointinpoly/default.html
      int X_c_a = x_C - x_A; // X-component of vector from A to C
      int Y_c_a = y_C - y_A; // Y-component of vector from A to C
      int X_b_a = x_B - x_A; // X-component of vector from A to B
      int Y_b_a = y_B - y_A; // Y-component of vector from A to B

      // Calculate dot-products of vectors. Use long long integers to avoid
      // truncating. The dot-product values can get large.
      long long dot_ca_ca = (X_c_a * X_c_a) + (Y_c_a * Y_c_a); // Dot-product of vector CA with itself
                                                          // ('dot00' from above web page)
      long long dot_ca_ba = (X_c_a * X_b_a) + (Y_c_a * Y_b_a); // Dot-product of vector CA with vector
                                                          // BA ('dot01' from above web page)
      long long dot_ba_ba = (X_b_a * X_b_a) + (Y_b_a * Y_b_a); // Dot-product of vector BA with itself 
                                                         // ('dot11' from above web page)
      int X_p_a, Y_p_a; // X- and Y-components of vector from A to point P
      long long dot_ca_pa;    // Dot product of vector CA with vector PA
      long long dot_ba_pa;    // Dot product of vector BA with vector PA
      float u, v, denominator; // Vectors u and v, as defined in above web page.
                               // 'denominator is temporary variable.

      // Raster over the rectangle that circumscribes the triangle:
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Calculate more vector quantities necessary to determine whether point P
          // is inside of triangle:
          X_p_a = x - x_A; // X-component of vector from P to A
          Y_p_a = y - y_A; // Y-component of vector from P to A
          dot_ca_pa = (X_c_a * X_p_a) + (Y_c_a * Y_p_a); // Dot product of vector CA with vector PA
                                                         // ('dot02' from above web page)
          dot_ba_pa = (X_b_a * X_p_a) + (Y_b_a * Y_p_a); // Dot product of vector BA with vector PA 
                                                         // ('dot12' from above web page)
          #ifdef DEBUG_defineCellDesignRules
          printf("DEBUG: dot_ca_ca=%lli, dot_ba_ba=%lli, dot_ca_ba=%lli, dot_ca_ba^2=%lli\n",
                 dot_ca_ca, dot_ba_ba, dot_ca_ba, dot_ca_ba*dot_ca_ba);
          #endif

          denominator = (float)(dot_ca_ca * dot_ba_ba - dot_ca_ba * dot_ca_ba);
          u = (float)((dot_ba_ba * dot_ca_pa) - (dot_ca_ba * dot_ba_pa)) / denominator;
          v = (float)((dot_ca_ca * dot_ba_pa) - (dot_ca_ba * dot_ca_pa)) / denominator;
          #ifdef DEBUG_defineCellDesignRules
          printf("DEBUG: At point (%d,%d), denominator=%5.2f, u=%5.2f, v=%5.2f, u+v=%5.2f\n",
                 x, y, denominator, u, v, u+v);
          #endif

          // Only proceed if point P at (x,y) is within the triangle. This is true only
          // if (u >= 0) and (v >= 0) and (u + v < 1).
          if ((u >= 0.0) && (v >= 0.0) && (u + v < 1.0)) {
            #ifdef DEBUG_defineCellDesignRules
            printf("DEBUG: Point (%d,%d) is within triangle.\n", x, y);
            #endif

            routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number
 
            // Define this cell's desgn-rule set number:

            #ifdef DEBUG_defineCellDesignRules
            printf("DEBUG: About to assign designRuleSet #%d to location (%d, %d, %d) within a triangle.\n", DR_set_number, x, y, routing_layer_num);
            #endif

            cellInfo[x][y][routing_layer_num].designRuleSet = DR_set_number;

          }  // End of if-block for X/Y point within triangle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'TRI'

    else {
      printf("\n\nERROR: Program encountered a DR_zone command with shape-type '%s' that is not\n", shape);
      printf("       recognized. Allowed types are ALL, RECT, CIR, and TRI (case insensitive).\n");
      printf("       Please fix input file. Program is exiting.\n\n");
      exit(1);
    }
      
  }  // End of for-loop for index 'DR_zone'

  //
  // DEBUG code follows:
  //
  #ifdef DEBUG_defineCellDesignRules
  printf("\nDEBUG: At end of defineCellDesignRules:\n");
  for (routing_layer_num = 0; routing_layer_num <= 2; routing_layer_num++)  {
    for (y = 103; y <= 107; y++)  {
      for (x = 113; x <= 117; x++)  {
        printf("DEBUG:   location (%d,%d,%d) has design-rule set %d\n",
               x, y, routing_layer_num, cellInfo[x][y][routing_layer_num].designRuleSet);
      }  // End of for-loop for index 'x'
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'z'
  printf("DEBUG:\n");
  printf("DEBUG:\n");
  //
  // End of DEBUG code.
  //
  #endif

  return;

}  // End of function 'defineCellDesignRules'


//-----------------------------------------------------------------------------
// Name: defineCellCosts
// Desc: Modifies the 'cellInfo' 3D matrix based on the 'trace_cost_multiplier'
//       and 'via_cost_multiplier' statements described in the 'user_inputs' 
//       data structure. 
//
//       This function defines the cost-zone number for each cell
//       in the map. If no cost-zones were defined by the user,
//       then all cells are assigned the default design-rule set
//       of zero, whose multiplier is one.
//-----------------------------------------------------------------------------
void defineCellCosts(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                           InputValues_t *user_inputs)  {

  int num_trace_zones = user_inputs->num_trace_cost_zones;
  int num_via_zones = user_inputs->num_via_cost_zones;
  int zone_index;  // Index of cost zone specified by user in trace/via cost-zone statement
  char shape[maxCostShapeLength];  // Shape for a cost zone (e.g., CIR, RECT)
  char layer_name[maxLayerNameLength];  // Routing layer name in a cost-zone command
  int params[maxCostParameters]; // Array of parameters for a given trace/via cost-
                                 // zone statement, with values converted from
                                 // microns to integer cell coordinates.

  int x, y, routing_layer_num, routing_layer_above, routing_layer_below; // Coordinates in 3D 'cellInfo' matrix
  int all_layer_num; // Z-coordinate in 3D space that includes all layers (routing and via layers)
  int isViaLayer; // = 1 if layer contains vias; =0 if layer is for routing
  int i;

  // printf("DEBUG: In function defineCellCosts, num_trace_zones = %d\n", num_trace_zones);
  // printf("DEBUG: In function defineCellCosts, num_via_zones   = %d\n", num_via_zones);

  // Initially, define each cell's cost-index as zero, whose cost-multiplier
  // is 1. Consequently, the costs are equal to the base-costs.
  // printf("DEBUG:  Setting ALL cells' costs to default (base) values...\n\n");
  for (routing_layer_num = 0; routing_layer_num < mapInfo->numLayers; routing_layer_num++)  {
    for (x = 0; x < mapInfo->mapWidth; x++)  {
      for (y = 0; y < mapInfo->mapHeight; y++)  {
        cellInfo[x][y][routing_layer_num].traceCostMultiplierIndex   = 0;
        cellInfo[x][y][routing_layer_num].viaUpCostMultiplierIndex   = 0;
        cellInfo[x][y][routing_layer_num].viaDownCostMultiplierIndex = 0;
      }  // End of for-loop for index 'y'
    }  // End of for-loop for index 'x'
  }  // End of for-loop for index 'routing_layer_num'
  

  // 
  // Cycle through the trace_cost_zone statements and modify the 'cellInfo' matrix
  // accordingly. Note that the order of the cost-zone statements matters!!
  //
  for (int cost_zone = 0; cost_zone < num_trace_zones; cost_zone++)  {

    // To simplify coding, copy the cost zone's index number, shape, and layer to temporary variables.
    zone_index = user_inputs->trace_cost_zone_index[cost_zone];
    strcpy(shape,      user_inputs->trace_cost_zone_shape[cost_zone]);
    strcpy(layer_name, user_inputs->trace_cost_zone_layer[cost_zone]);

    // If cost-zone statement contains numeric parameters, convert them from  
    // floating-point values to integer cell coordinates:
    for (i = 0; i < user_inputs->trace_cost_num_params[cost_zone]; i++)  {
      // printf("DEBUG:   user_inputs->trace_cost_zone_parameters[%d][%d] is %5.2f\n", cost_zone, i,
      //            user_inputs->trace_cost_zone_parameters[cost_zone][i]);
      params[i] = (int)roundf(user_inputs->trace_cost_zone_parameters[cost_zone][i] / user_inputs->cell_size_um);
      // printf("DEBUG:   Parameter #%d is %d\n", i, params[i]);
    }

    //
    // Determine layer number from layer name:
    //
    all_layer_num = -1;
    for (i = 0; i < 2 * mapInfo->numLayers - 1; i++)  {
      if (strcasecmp(layer_name, user_inputs->layer_names[i]) == 0)  {
        all_layer_num = i;
        break;
      }  // End of if-statement
    }  // End of for-loop for index 'all_layer_num'

    // Confirm that layer name is a valid layer name:
    if (all_layer_num == -1)  {
      printf("\nERROR: Statement 'trace_cost_zone %d %s %s...' in input file references layer '%s', which is\n",
              zone_index, layer_name, shape, layer_name);
      printf("       not defined as one of the valid layer names. Please fix input file.\n\n");
      exit(1);
    }
    // printf("DEBUG: Layer '%s' in trace_cost_zone statement is mapped to layer '%d'\n", layer_name, all_layer_num);

    // Confirm that the trace_cost_zone statment refers to a routing layer rather than a via layer. If it refers
    // to a via layer, then issue an error message and terminate.
    isViaLayer = all_layer_num % 2; // Odd-number layers are via layers; even-numbered are routing layers
    if (isViaLayer)  {
      printf("\nERROR: A 'trace_cost_zone' statement in the input file refers to a via layer. trace_cost_zone statements\n");
      printf("       may only refer to routing layers. Fix the input file and re-start the program.\n\n");
      exit(1);
    }  // End of if-block for (isViaLayer)


    //
    // Handle 'trace_cost_zone <cost index>  <layer> ALL' and 'trace_cost_zone <cost index> <layer> RECT' commands:
    //
    if ((strcasecmp(shape, "ALL") == 0) || (strcasecmp(shape, "RECT") == 0))  {
      int x1, y1, x2, y2;
      if (strcasecmp(shape, "ALL") == 0) {
        x1 = 0;
        y1 = 0;
        x2 = mapInfo->mapWidth;
        y2 = mapInfo->mapHeight;
      }
      else {
        x1 = min(params[0], params[2]);  // x1 is x-coordinate of lower-left RECT corner
        y1 = min(params[1], params[3]);  // y1 is y-coordinate of lower-left RECT corner 
        x2 = max(params[0], params[2]);  // x2 is x-coordinate of upper-right RECT corner
        y2 = max(params[1], params[3]);  // y2 is y-coordinate of upper-right RECT corner
      }

      //
      // Raster over the rectangle of interest:
      //
      for (x = x1; x <= x2; x++)  {
        for (y = y1; y <= y2; y++)  {

          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // printf("DEBUG: Handling RECT shape at location (%d, %d).\n", x, y);

          routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

          // Define this cell's trace-cost index:
          cellInfo[x][y][routing_layer_num].traceCostMultiplierIndex = zone_index;

        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'
    }  // End of if-clause for command type == 'ALL' or 'RECT'

    //
    // Handle 'DR_zone <DR name> <layer> CIR' command:
    //
    else if (strcasecmp(shape, "CIR") == 0)  {
      // printf("DEBUG: Found CIR statement with parameters %d, %d, and %d\n",
      //        params[0], params[1], params[2]);

      int x_cent = params[0]; // X-coordinate of circle's center
      int y_cent = params[1]; // Y-coordinate of circle's center
      int radius = params[2]; // Radius of circle
      int radius_squared = radius * radius;

      // Raster over the square that circumscribes the circle:
      int x_min, y_min, x_max, y_max;
      x_min = x_cent - radius; // = Xo - R
      y_min = y_cent - radius; // = Yo - R
      x_max = x_cent + radius; // = Xo + R
      y_max = y_cent + radius; // = Yo + R
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Only proceed if X/Y point is within the circle: delta-X^2 + delta-Y^2 < R^2
          if ((x - x_cent)*(x - x_cent) + (y - y_cent)*(y - y_cent) <= radius_squared)  {

            // Layer is a routing layer (not a via layer):
            routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

            // Define this cell's trace-cost index:
            cellInfo[x][y][routing_layer_num].traceCostMultiplierIndex = zone_index;

          }  // End of if-block for X/Y point within circle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'CIR'

    //
    // Handle 'DR_zone <DR name> <layer> TRI' command:
    //
    else if (strcasecmp(shape, "TRI") == 0)  {
      // printf("DEBUG: Found TRI statement with parameters %d, %d, %d, %d, %d, %d\n",
      //        params[0], params[1], params[2], params[3], params[4], params[5]);

      // Capture parameters in variables that describe the X/Y coordinates
      // of the triangle's 3 vertices: A, B, and C
      int x_A = params[0]; // X-coordinate of vertex 'A'
      int y_A = params[1]; // Y-coordinate of vertex 'A'
      int x_B = params[2]; // X-coordinate of vertex 'B'
      int y_B = params[3]; // Y-coordinate of vertex 'B'
      int x_C = params[4]; // X-coordinate of vertex 'C'
      int y_C = params[5]; // Y-coordinate of vertex 'C'

      // Determine the minimum and maximum X- and Y-values,
      // (x_min, y_min) and (x_max, y_max), respectively:
      int x_min = x_A; int y_min = x_A;
      int x_max = x_A; int y_max = x_A;
      if (x_B < x_min) x_min = x_B;
      if (x_C < x_min) x_min = x_C;
      if (x_B > x_max) x_max = x_B;
      if (x_C > x_max) x_max = x_C;
      if (y_B < y_min) y_min = y_B;
      if (y_C < y_min) y_min = y_C;
      if (y_B > y_max) y_max = y_B;
      if (y_C > y_max) y_max = y_C;
      // printf("DEBUG: Lower-left point is (%d,%d). Upper-right point is (%d,%d)\n",
      //         x_min, y_min, x_max, y_max);

      // Use Barycentric Technique to determine whether points are within
      // the triangle defined by vertices A, B, and C. This technique is
      // described at following web page: 
      //    http://www.blackpawn.com/texts/pointinpoly/default.html
      int X_c_a = x_C - x_A; // X-component of vector from A to C
      int Y_c_a = y_C - y_A; // Y-component of vector from A to C
      int X_b_a = x_B - x_A; // X-component of vector from A to B
      int Y_b_a = y_B - y_A; // Y-component of vector from A to B

      // Calculate dot-products of vectors. Use long long integers to avoid
      // truncating. The dot-product values can get large.
      long long dot_ca_ca = (X_c_a * X_c_a) + (Y_c_a * Y_c_a); // Dot-product of vector CA with itself
                                                          // ('dot00' from above web page)
      long long dot_ca_ba = (X_c_a * X_b_a) + (Y_c_a * Y_b_a); // Dot-product of vector CA with vector
                                                          // BA ('dot01' from above web page)
      long long dot_ba_ba = (X_b_a * X_b_a) + (Y_b_a * Y_b_a); // Dot-product of vector BA with itself 
                                                         // ('dot11' from above web page)
      int X_p_a, Y_p_a; // X- and Y-components of vector from A to point P
      long long dot_ca_pa;    // Dot product of vector CA with vector PA
      long long dot_ba_pa;    // Dot product of vector BA with vector PA
      float u, v, denominator; // Vectors u and v, as defined in above web page.
                               // 'denominator is temporary variable.

      // Raster over the rectangle that circumscribes the triangle:
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Calculate more vector quantities necessary to determine whether point P
          // is inside of triangle:
          X_p_a = x - x_A; // X-component of vector from P to A
          Y_p_a = y - y_A; // Y-component of vector from P to A
          dot_ca_pa = (X_c_a * X_p_a) + (Y_c_a * Y_p_a); // Dot product of vector CA with vector PA
                                                         // ('dot02' from above web page)
          dot_ba_pa = (X_b_a * X_p_a) + (Y_b_a * Y_p_a); // Dot product of vector BA with vector PA 
                                                         // ('dot12' from above web page)
          // printf("DEBUG: dot_ca_ca=%lli, dot_ba_ba=%lli, dot_ca_ba=%lli, dot_ca_ba^2=%lli\n",
          //        dot_ca_ca, dot_ba_ba, dot_ca_ba, dot_ca_ba*dot_ca_ba);

          denominator = (float)(dot_ca_ca * dot_ba_ba - dot_ca_ba * dot_ca_ba);
          u = (float)((dot_ba_ba * dot_ca_pa) - (dot_ca_ba * dot_ba_pa)) / denominator;
          v = (float)((dot_ca_ca * dot_ba_pa) - (dot_ca_ba * dot_ca_pa)) / denominator;
          // printf("DEBUG: At point (%d,%d), denominator=%5.2f, u=%5.2f, v=%5.2f, u+v=%5.2f\n", 
          //        x, y, denominator, u, v, u+v);

          // Only proceed if point P at (x,y) is within the triangle. This is true only
          // if (u >= 0) and (v >= 0) and (u + v < 1).
          if ((u >= 0.0) && (v >= 0.0) && (u + v < 1.0)) {
            // printf("DEBUG: Point (%d,%d) is within triangle.\n", x, y);

            routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number
 
            // Define this cell's trace-cost index:
            cellInfo[x][y][routing_layer_num].traceCostMultiplierIndex = zone_index;

          }  // End of if-block for X/Y point within triangle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'TRI'

    else {
      printf("\n\nERROR: Program encountered a trace_cost_zone command with shape-type '%s' that is not\n", shape);
      printf("       recognized. Allowed types are ALL, RECT, CIR, and TRI (case insensitive).\n");
      printf("       Please fix input file. Program is exiting.\n\n");
      exit(1);
    }
      
  }  // End of for-loop for index 'cost_zone'



  // 
  // Now do the same thing for the VIA_cost_zones. That is, cycle through the 
  // via_cost_zone statements and modify the 'cellInfo' matrix
  // accordingly. Note that the order of the cost-zone statements matters!!
  //
  for (int cost_zone = 0; cost_zone < num_via_zones; cost_zone++)  {

    // To simplify coding, copy the cost zone's index number, shape, and layer to temporary variables.
    zone_index = user_inputs->via_cost_zone_index[cost_zone];
    strcpy(shape,      user_inputs->via_cost_zone_shape[cost_zone]);
    strcpy(layer_name, user_inputs->via_cost_zone_layer[cost_zone]);

    // If cost-zone statement contains numeric parameters, convert them from  
    // floating-point values to integer cell coordinates:
    for (i = 0; i < user_inputs->via_cost_num_params[cost_zone]; i++)  {
      // printf("DEBUG:   user_inputs->via_cost_zone_parameters[%d][%d] is %5.2f\n", cost_zone, i,
      //            user_inputs->via_cost_zone_parameters[cost_zone][i]);
      params[i] = (int)roundf(user_inputs->via_cost_zone_parameters[cost_zone][i] / user_inputs->cell_size_um);
      // printf("DEBUG:   Parameter #%d is %d\n", i, params[i]);
    }

    //
    // Determine layer number from layer name:
    //
    all_layer_num = -1;
    for (i = 0; i < 2 * mapInfo->numLayers - 1; i++)  {
      // printf("DEBUG: In defineCellCosts, comparing '%s' layer name from via_cost_zone statement with layer name #%d: '%s'\n...",
      //        layer_name, i, user_inputs->layer_names[i]);
      if (strcasecmp(layer_name, user_inputs->layer_names[i]) == 0)  {
        all_layer_num = i;
        break;
      }  // End of if-statement
    }  // End of for-loop for index 'all_layer_num'

    // Confirm that layer name is a valid layer name:
    if (all_layer_num == -1)  {
      printf("\nERROR: Statement 'via_cost_zone %d %s %s...' in input file references layer '%s', which is\n",
              zone_index, layer_name, shape, layer_name);
      printf("       not defined as one of the valid layer names. Please fix input file.\n\n");
      exit(1);
    }
    // printf("DEBUG: Layer '%s' in via_cost_zone statement is mapped to layer '%d'\n", layer_name, all_layer_num);

    // Confirm that the via_cost_zone statment refers to a via layer rather than a routing layer. If it refers
    // to a routing layer, then issue an error message and terminate.
    isViaLayer = all_layer_num % 2; // Odd-number layers are via layers; even-numbered are routing layers
    if (! isViaLayer)  {
      printf("\nERROR: A 'via_cost_zone' statement in the input file refers to a routing  layer. via_cost_zone statements\n");
      printf("       may only refer to via layers. Fix the input file and re-start the program.\n\n");
      exit(1);
    }  // End of if-block for (! isViaLayer)


    //
    // Handle 'via_cost_zone <cost index>  <layer> ALL' and 'via_cost_zone <cost index> <layer> RECT' commands:
    //
    if ((strcasecmp(shape, "ALL") == 0) || (strcasecmp(shape, "RECT") == 0))  {
      int x1, y1, x2, y2;
      if (strcasecmp(shape, "ALL") == 0) {
        x1 = 0;
        y1 = 0;
        x2 = mapInfo->mapWidth;
        y2 = mapInfo->mapHeight;
      }
      else {
        x1 = min(params[0], params[2]);  // x1 is x-coordinate of lower-left RECT corner
        y1 = min(params[1], params[3]);  // y1 is y-coordinate of lower-left RECT corner 
        x2 = max(params[0], params[2]);  // x2 is x-coordinate of upper-right RECT corner
        y2 = max(params[1], params[3]);  // y2 is y-coordinate of upper-right RECT corner
      }

      //
      // Raster over the rectangle of interest:
      //
      for (x = x1; x <= x2; x++)  {
        for (y = y1; y <= y2; y++)  {

          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // printf("DEBUG: Handling RECT shape at location (%d, %d).\n", x, y);

          // Routing layer numbers of affected layers are half the via-layer number, +/- 1:
          routing_layer_above = (all_layer_num + 1) / 2;
          routing_layer_below = (all_layer_num - 1) / 2;
         
          // Define the affected cells' via-cost indices:
          cellInfo[x][y][routing_layer_below].viaUpCostMultiplierIndex   = zone_index;
          cellInfo[x][y][routing_layer_above].viaDownCostMultiplierIndex = zone_index;

        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'
    }  // End of if-clause for command type == 'ALL' or 'RECT'

    //
    // Handle 'DR_zone <DR name> <layer> CIR' command:
    //
    else if (strcasecmp(shape, "CIR") == 0)  {
      // printf("DEBUG: Found CIR statement with parameters %d, %d, and %d\n",
      //        params[0], params[1], params[2]);

      int x_cent = params[0]; // X-coordinate of circle's center
      int y_cent = params[1]; // Y-coordinate of circle's center
      int radius = params[2]; // Radius of circle
      int radius_squared = radius * radius;

      // Raster over the square that circumscribes the circle:
      int x_min, y_min, x_max, y_max;
      x_min = x_cent - radius; // = Xo - R
      y_min = y_cent - radius; // = Yo - R
      x_max = x_cent + radius; // = Xo + R
      y_max = y_cent + radius; // = Yo + R
      for (x = x_min; x <= x_max; x++)  {

        int delta_x_squared = (x - x_cent)*(x - x_cent);

        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Only proceed if X/Y point is within the circle: deltaX^2 + deltaY^2 < R^2
          if (delta_x_squared + (y - y_cent)*(y - y_cent) <= radius_squared)  {

            // Routing layer numbers of affected layers are half the via-layer number, +/- 1:
            routing_layer_above = (all_layer_num + 1) / 2;
            routing_layer_below = (all_layer_num - 1) / 2;
         
            // Define the affected cells' via-cost indices:
            cellInfo[x][y][routing_layer_below].viaUpCostMultiplierIndex   = zone_index;
            cellInfo[x][y][routing_layer_above].viaDownCostMultiplierIndex = zone_index;

          }  // End of if-block for X/Y point within circle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'CIR'

    //
    // Handle 'DR_zone <DR name> <layer> TRI' command:
    //
    else if (strcasecmp(shape, "TRI") == 0)  {
      // printf("DEBUG: Found TRI statement with parameters %d, %d, %d, %d, %d, %d\n",
      //        params[0], params[1], params[2], params[3], params[4], params[5]);

      // Capture parameters in variables that describe the X/Y coordinates
      // of the triangle's 3 vertices: A, B, and C
      int x_A = params[0]; // X-coordinate of vertex 'A'
      int y_A = params[1]; // Y-coordinate of vertex 'A'
      int x_B = params[2]; // X-coordinate of vertex 'B'
      int y_B = params[3]; // Y-coordinate of vertex 'B'
      int x_C = params[4]; // X-coordinate of vertex 'C'
      int y_C = params[5]; // Y-coordinate of vertex 'C'

      // Determine the minimum and maximum X- and Y-values,
      // (x_min, y_min) and (x_max, y_max), respectively:
      int x_min = x_A; int y_min = x_A;
      int x_max = x_A; int y_max = x_A;
      if (x_B < x_min) x_min = x_B;
      if (x_C < x_min) x_min = x_C;
      if (x_B > x_max) x_max = x_B;
      if (x_C > x_max) x_max = x_C;
      if (y_B < y_min) y_min = y_B;
      if (y_C < y_min) y_min = y_C;
      if (y_B > y_max) y_max = y_B;
      if (y_C > y_max) y_max = y_C;
      // printf("DEBUG: Lower-left point is (%d,%d). Upper-right point is (%d,%d)\n",
      //         x_min, y_min, x_max, y_max);

      // Use Barycentric Technique to determine whether points are within
      // the triangle defined by vertices A, B, and C. This technique is
      // described at following web page: 
      //    http://www.blackpawn.com/texts/pointinpoly/default.html
      int X_c_a = x_C - x_A; // X-component of vector from A to C
      int Y_c_a = y_C - y_A; // Y-component of vector from A to C
      int X_b_a = x_B - x_A; // X-component of vector from A to B
      int Y_b_a = y_B - y_A; // Y-component of vector from A to B

      // Calculate dot-products of vectors. Use long long integers to avoid
      // truncating. The dot-product values can get large.
      long long dot_ca_ca = (X_c_a * X_c_a) + (Y_c_a * Y_c_a); // Dot-product of vector CA with itself
                                                          // ('dot00' from above web page)
      long long dot_ca_ba = (X_c_a * X_b_a) + (Y_c_a * Y_b_a); // Dot-product of vector CA with vector
                                                          // BA ('dot01' from above web page)
      long long dot_ba_ba = (X_b_a * X_b_a) + (Y_b_a * Y_b_a); // Dot-product of vector BA with itself 
                                                         // ('dot11' from above web page)
      int X_p_a, Y_p_a; // X- and Y-components of vector from A to point P
      long long dot_ca_pa;    // Dot product of vector CA with vector PA
      long long dot_ba_pa;    // Dot product of vector BA with vector PA
      float u, v, denominator; // Vectors u and v, as defined in above web page.
                               // 'denominator is temporary variable.

      // Raster over the rectangle that circumscribes the triangle:
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Calculate more vector quantities necessary to determine whether point P
          // is inside of triangle:
          X_p_a = x - x_A; // X-component of vector from P to A
          Y_p_a = y - y_A; // Y-component of vector from P to A
          dot_ca_pa = (X_c_a * X_p_a) + (Y_c_a * Y_p_a); // Dot product of vector CA with vector PA
                                                         // ('dot02' from above web page)
          dot_ba_pa = (X_b_a * X_p_a) + (Y_b_a * Y_p_a); // Dot product of vector BA with vector PA 
                                                         // ('dot12' from above web page)
          // printf("DEBUG: dot_ca_ca=%lli, dot_ba_ba=%lli, dot_ca_ba=%lli, dot_ca_ba^2=%lli\n",
          //        dot_ca_ca, dot_ba_ba, dot_ca_ba, dot_ca_ba*dot_ca_ba);

          denominator = (float)(dot_ca_ca * dot_ba_ba - dot_ca_ba * dot_ca_ba);
          u = (float)((dot_ba_ba * dot_ca_pa) - (dot_ca_ba * dot_ba_pa)) / denominator;
          v = (float)((dot_ca_ca * dot_ba_pa) - (dot_ca_ba * dot_ca_pa)) / denominator;
          // printf("DEBUG: At point (%d,%d), denominator=%5.2f, u=%5.2f, v=%5.2f, u+v=%5.2f\n", 
          //        x, y, denominator, u, v, u+v);

          // Only proceed if point P at (x,y) is within the triangle. This is true only
          // if (u >= 0) and (v >= 0) and (u + v < 1).
          if ((u >= 0.0) && (v >= 0.0) && (u + v < 1.0)) {
            // printf("DEBUG: Point (%d,%d) is within triangle.\n", x, y);

            // Routing layer numbers of affected layers are half the via-layer number, +/- 1:
            routing_layer_above = (all_layer_num + 1) / 2;
            routing_layer_below = (all_layer_num - 1) / 2;
         
            // Define the affected cells' via-cost indices:
            cellInfo[x][y][routing_layer_below].viaUpCostMultiplierIndex   = zone_index;
            cellInfo[x][y][routing_layer_above].viaDownCostMultiplierIndex = zone_index;

          }  // End of if-block for X/Y point within triangle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command type == 'TRI'

    else {
      printf("\n\nERROR: Program encountered a via_cost_zone command with shape-type '%s' that is not\n", shape);
      printf("       recognized. Allowed types are ALL, RECT, CIR, and TRI (case insensitive).\n");
      printf("       Please fix input file. Program is exiting.\n\n");
      exit(1);
    }
      
  }  // End of for-loop for index 'cost_zone'

  return;

}  // End of function 'defineCellCosts'


//-----------------------------------------------------------------------------
// Name: prior_neighbors
// Desc: This function returns the set of neighboring, pin-swappable cells
//       at 3 neighboring locations to (x,y,z): (x-1,y,z), (x,y-1,z), and (x,y,z-1).
//       This function is called by function 'definePinSwapZones', and is based
//       on the procedure by the same name described in University of Washington
//       computer science course:
//            https://courses.cs.washington.edu/courses/cse576/book/ch3.pdf
//
//       The returned integer array has the following structure:
//          array[0]   = number of neighbors that are pin-swappable (0 to 3)
//          array[1-3] = (x,y,z) values of 1st pin-swappable cell (if it exists)
//          array[4-6] = (x,y,z) values of 2nd pin-swappable cell (if it exists)
//          array[7-9] = (x,y,z) values of 3rd pin-swappable cell (if it exists)
//-----------------------------------------------------------------------------
static int * prior_neighbors(int x, int y, int z, char ***inPinSwapZone)  {

  static int swappable_neighbors[10];  // Array to be returned
  int num_swappable_neighbors = 0;

  // Check the cell at (x-1,y,z), but only if (x-1) is not negative:
  if ((x - 1 >= 0) && (inPinSwapZone[x-1][y][z]))  {
    num_swappable_neighbors++;
    swappable_neighbors[1] = x - 1;
    swappable_neighbors[2] = y;
    swappable_neighbors[3] = z;
  }  // End of if-block 

  // Check the cell at (x,y-1,z), but only if (y-1) is not negative:
  if ((y - 1 >= 0) && (inPinSwapZone[x][y-1][z]))  {
    num_swappable_neighbors++;
    swappable_neighbors[4] = x;
    swappable_neighbors[5] = y - 1;
    swappable_neighbors[6] = z;
  }  // End of if-block 

  // Check the cell at (x,y,z-1), but only if (z-1) is not negative:
  if ((z - 1 >= 0) && (inPinSwapZone[x][y][z-1]))  {
    num_swappable_neighbors++;
    swappable_neighbors[7] = x;
    swappable_neighbors[8] = y;
    swappable_neighbors[9] = z - 1;
  }  // End of if-block 

  // Set the 0th element of returned array to the number of swappable neighbors (0 to 3):
  swappable_neighbors[0] = num_swappable_neighbors;

  // Return pointer to the static array of swappable neighbors:
  return(swappable_neighbors);

}  // End of function 'prior_neighbors'

//-----------------------------------------------------------------------------
// Name: merge_labels
// Desc: This function is called by function 'definePinSwapZones'. This 'merge_labels'
//       function takes two labels, label_1 and label_2, and the parent 
//       array 'parents'. The function modifies the 'parents' structure (if necessary) 
//       to merge the set containing label_1 with the set containing label_2.
//       It starts at labels 'label_1' and 'label_2' and follows the 'parents'
//       pointers up the tree until it reaches the roots of the two sets. If the
//       roots are not the same, one label is made the parent of the other.
//
//       This function was taken from University of Washington computer science
//       class, as described here:
//          https://courses.cs.washington.edu/courses/cse576/book/ch3.pdf
//       In the above PDF file, this function is called 'union'.
//-----------------------------------------------------------------------------
static void merge_labels(int label_1, int label_2, int *parents)  {

  int j = label_1;
  int k = label_2;

  while (parents[j])
    j = parents[j];

  while (parents[k])
    k = parents[k];

  if (j != k)
    // The roots are not the same, so make one label the parent of the other.
    // (Here, label_1 is arbitrarily made the parent of label_2.)
    parents[k] = j;

}  // End of function 'merge_labels'


//-----------------------------------------------------------------------------
// Name: find
// Desc: This function is called by function 'definePinSwapZones'. This 'find'
//       function takes a label 'label' and the parent array 'parents.' It
//       follows the parent points up the tree to find the label of the root 
//       node of the tree that 'label' is in .
//
//       This function was taken from University of Washington computer science
//       class, as described here:
//          https://courses.cs.washington.edu/courses/cse576/book/ch3.pdf
//-----------------------------------------------------------------------------
static int find(int label, int *parents)  {

  int j = label;

  while (parents[j])
    j = parents[j];

  return(j);

}  // End of function 'find'


//-----------------------------------------------------------------------------
// Name: definePinSwapZones
// Desc: Modifies the 'cellInfo' 3D matrix based on the PIN_SWAP/NO_PIN_SWAP
//       statements described in the 'user_inputs' data structure. Each cell
//       is flagged as either being in a pin-swappable zone, or not being in 
//       such a zone. For those cells in a pin-swappable zone, a unique number for
//       that zone is defined in this function. A pin-swap zone is defined as
//       all the cells that share a common side or face (in all 3 dimensions).
//
//       Function also maps the swap-zone numbers to the starting-coordinates of
//       each path that is within each pin-swap zone. If a path's ending-
//       coordinates are within a pin-swap zone, then the function swaps the
//       path's starting and ending coordinates, and ensures that that the
//       isPNswappable flag is set for the nets, and the start-terminal's
//       pitch values are set to zero.
//-----------------------------------------------------------------------------
void definePinSwapZones(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                        InputValues_t *user_inputs)  {

  int num_swap_statements = user_inputs->num_swap_instructions;
  char command[maxPinSwapInstructionLength];
  char shape[maxPinSwapInstructionLength];
  char layer_name[maxLayerNameLength];
  int params[maxPinSwapParameters]; // Array of parameters for a given PIN_SWAP/NO_PIN_SWAP
                                    // statement, with values converted from microns to
                                    // to integer cell coordinates.

  int x, y, z, routing_layer_num; // Coordinates in 3D 'cellInfo' matrix
  int all_layer_num; // Z-coordinate in 3D space that includes all layers (routing and via layers)
  int isViaLayer; // = 1 if layer contains vias; =0 if layer is for routing
  int i;
  int *swappable_neighbors;  // Pointer to integer array containing number of pin-swappable
                             // neighbors, as well as their x/y/z coordinates.
  int num_swappable_neighbors;
  int x_neighbor, y_neighbor, z_neighbor; // Coordinates of pin-swappable neighbors
  int minimum_swap_label; // Minimum swap-zone label of neighbors

  // Create two temporary 3-dimensional matrices. 'temp_swap_labels' is matrix of integers to
  // track the temporary pin-swap labels, which can temporarily exceed the 'maxSwapZones' value
  // of 255 that can be handled in the cellInfo structure. The second matrix, inPinSwapZone, is
  // a matrix of char's acting as Boolean variables to specify whether the cell is in a pin-swap
  // zone:
  int ***temp_swap_labels = malloc(sizeof(int **) * mapInfo->mapWidth);
  if (temp_swap_labels == 0) {
    printf("\nERROR: Failed to allocate memory for 'temp_swap_labels' matrix.\n\n");
    exit (1);
  }
  char ***inPinSwapZone = malloc(sizeof(char **) * mapInfo->mapWidth);
  if (inPinSwapZone == 0) {
    printf("\nERROR: Failed to allocate memory for 'inPinSwapZone' matrix.\n\n");
    exit (1);
  }

  // Allocate space for a 2nd dimension:
  for (x = 0; x < mapInfo->mapWidth; x++)  {
    temp_swap_labels[x] = malloc(mapInfo->mapHeight * sizeof(int *));
    if (temp_swap_labels[x] == 0) {
      printf("\nERROR: Failed to allocate memory for 'temp_swap_labels[%d]' array.\n\n", x);
      exit (1);
    }

    inPinSwapZone[x] = malloc(mapInfo->mapHeight * sizeof(char *));
    if (inPinSwapZone[x] == 0) {
      printf("\nERROR: Failed to allocate memory for 'inPinSwapZone[%d]' array.\n\n", x);
      exit (1);
    }

    // Allocate space for a 3rd dimension:
    for (y = 0; y < mapInfo->mapHeight; y++ )  {
      temp_swap_labels[x][y] = malloc((mapInfo->numLayers) * sizeof(int));
      if (temp_swap_labels[x][y] == 0) {
        printf("\nERROR: Failed to allocate memory for 'temp_swap_labels[%d][%d]' array.\n\n", x, y);
        exit (1);
      }

      inPinSwapZone[x][y] = malloc((mapInfo->numLayers) * sizeof(char));
      if (inPinSwapZone[x][y] == 0) {
        printf("\nERROR: Failed to allocate memory for 'inPinSwapZone[%d][%d]' array.\n\n", x, y);
        exit (1);
      }

      // Initialize labels for all x/y/z locations to zero, and set all Boolean values to FALSE:
      for (z = 0; z < mapInfo->numLayers; z++)  {
        temp_swap_labels[x][y][z] = 0;
        inPinSwapZone[x][y][z] = FALSE;
      }  // End of 'z' for-loop
    }  // End of 'y' for-loop
  }  // End of 'x' for-loop

  // printf("DEBUG: In function definePinSwapZones, num_swap_statements=%d\n", num_swap_statements);

  // 
  // Cycle through the PIN_SWAP/NO_PIN_SWAP statements and modify the 'cellInfo' matrix
  // accordingly. Note that the order of the PIN_SWAP/NO_PIN_SWAP statements matters!!
  //
  for (int swap_statement = 0; swap_statement < num_swap_statements; swap_statement++)  {

    // To simplify coding, copy the command name, type, and layer to temporary variables.
    strcpy(command,    user_inputs->swap_command[swap_statement]);
    strcpy(layer_name, user_inputs->swap_layer[swap_statement]);
    strcpy(shape,      user_inputs->swap_shape[swap_statement]);
    // printf("DEBUG: In function definePinSwapZones, swap_command[%d]=%s\n", swap_statement, command);

    // If PIN_SWAP/NO_PIN_SWAP statement contains numeric parameters, convert them from  
    // floating-point values to integer cell coordinates:
    for (i = 0; i < user_inputs->swap_num_params[swap_statement]; i++)  {
      params[i] = (int)roundf(user_inputs->swap_parameters[swap_statement][i] / user_inputs->cell_size_um);
      // printf("DEBUG:   Parameter #%d is %d\n", i, params[i]);
    }

    //
    // Determine layer number from layer name:
    //
    all_layer_num = -1;
    for (i = 0; i < 2 * mapInfo->numLayers - 1; i++)  {
      if (strcasecmp(layer_name, user_inputs->layer_names[i]) == 0)  {
        all_layer_num = i;
        break;
      }  // End of if-statement
    }  // End of for-loop for index 'all_layer_num'

    isViaLayer = all_layer_num % 2; // Odd-number layers are via layers; even-numbered are routing layers

    // Confirm that layer name is a valid layer name:
    if ((all_layer_num == -1) || (isViaLayer))  {
      printf("\nERROR: Statement '%s %s %s...' in input file references layer '%s', which is\n",
              command, layer_name, shape, layer_name);
      printf("       not defined as one of the valid routing layer names. Please fix input file.\n\n");
      exit(1);
    }
    // printf("DEBUG: Layer '%s' in PIN_SWAP/NO_PIN_SWAP statement is mapped to layer '%d'\n", layer_name, all_layer_num);


    //
    // Handle 'PIN_SWAP ALL', 'NO_PIN_SWAP ALL', 'PIN_SWAP RECT', and 'NO_PIN_SWAP RECT' commands:
    //
    if ((strcasecmp(shape, "ALL") == 0) || (strcasecmp(shape, "RECT") == 0))  {
      int x1, y1, x2, y2;
      if (strcasecmp(shape, "ALL") == 0) {
        x1 = 0;
        y1 = 0;
        x2 = mapInfo->mapWidth;
        y2 = mapInfo->mapHeight;
      }
      else {
        x1 = min(params[0], params[2]);  // x1 is x-coordinate of lower-left RECT corner
        y1 = min(params[1], params[3]);  // y1 is y-coordinate of lower-left RECT corner 
        x2 = max(params[0], params[2]);  // x2 is x-coordinate of upper-right RECT corner
        y2 = max(params[1], params[3]);  // y2 is y-coordinate of upper-right RECT corner
      }

      //
      // Raster over the rectangle of interest:
      //
      for (x = x1; x <= x2; x++)  {
        for (y = y1; y <= y2; y++)  {

          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // printf("DEBUG: Handling RECT shape at location (%d, %d).\n", x, y);

          routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

          // Based on whether command is PIN_SWAP or NO_PIN_SWAP, make the cell swappable or
          // unswappable. 
          if (strcasecmp(command, "PIN_SWAP") == 0)  {
            inPinSwapZone[x][y][routing_layer_num] = TRUE;
          }
          else if (strcasecmp(command, "NO_PIN_SWAP") == 0)  {
            inPinSwapZone[x][y][routing_layer_num] = FALSE;
          }  // End of if/else-clause for command == 'PIN_SWAP'
          else {
            printf("\nERROR: An unexpected PIN_SWAP/NO_PIN_SWAP command of '%s' was encountered. Program is exiting.\n\n",
                    command);
            exit(1);
          }  // End of final else-clause for command not equal to PIN_SWAP or NO_PIN_SWAP
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'
    }  // End of if-clause for command shape == 'ALL' or 'RECT'

    //
    // Handle 'PIN_SWAP CIR' and 'NO_PIN_SWAP CIR' commands:
    //
    else if (strcasecmp(shape, "CIR") == 0)  {
      // printf("DEBUG: Found CIR statement with parameters %d, %d, and %d\n",
      //        params[0], params[1], params[2]);

      int x_cent = params[0]; // X-coordinate of circle's center
      int y_cent = params[1]; // Y-coordinate of circle's center
      int radius = params[2]; // Radius of circle
      int radius_squared = radius * radius;

      // Raster over the square that circumscribes the circle:
      int x_min, y_min, x_max, y_max;
      x_min = x_cent - radius; // = Xo - R
      y_min = y_cent - radius; // = Yo - R
      x_max = x_cent + radius; // = Xo + R
      y_max = y_cent + radius; // = Yo + R
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Only proceed if X/Y point is within the circle: delta-X^2 + delta-Y^2 < R^2
          if ((x - x_cent)*(x - x_cent) + (y - y_cent)*(y - y_cent) <= radius_squared)  {

            routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

            // Based on whether command is PIN_SWAP or NO_PIN_SWAP, make the cell swappable or
            // unswappable. 
            if (strcasecmp(command, "PIN_SWAP") == 0)  {
              inPinSwapZone[x][y][routing_layer_num] = TRUE;
            }
            else if (strcasecmp(command, "NO_PIN_SWAP") == 0)  {
              inPinSwapZone[x][y][routing_layer_num] = FALSE;
            }  // End of if/else-clause for command == 'NO_PIN_SWAP'
            else {
              printf("\nERROR: An unexpected PIN_SWAP/NO_PIN_SWAP command of '%s' was encountered. Program is exiting.\n\n",
                      command);
              exit(1);
            }  // End of final else-clause for command not equal to PIN_SWAP or NO_PIN_SWAP

          }  // End of if-block for X/Y point within circle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command shape == 'CIR'

    //
    // Handle 'BLOCK TRI' and 'UNBLOCK TRI' commands:
    //
    else if (strcasecmp(shape, "TRI") == 0)  {
      // printf("DEBUG: Found TRI statement with parameters %d, %d, %d, %d, %d, %d\n",
      //        params[0], params[1], params[2], params[3], params[4], params[5]);

      // Capture parameters in variables that describe the X/Y coordinates
      // of the triangle's 3 vertices: A, B, and C
      int x_A = params[0]; // X-coordinate of vertex 'A'
      int y_A = params[1]; // Y-coordinate of vertex 'A'
      int x_B = params[2]; // X-coordinate of vertex 'B'
      int y_B = params[3]; // Y-coordinate of vertex 'B'
      int x_C = params[4]; // X-coordinate of vertex 'C'
      int y_C = params[5]; // Y-coordinate of vertex 'C'

      // Determine the minimum and maximum X- and Y-values,
      // (x_min, y_min) and (x_max, y_max), respectively:
      int x_min = x_A; int y_min = x_A;
      int x_max = x_A; int y_max = x_A;
      if (x_B < x_min) x_min = x_B;
      if (x_C < x_min) x_min = x_C;
      if (x_B > x_max) x_max = x_B;
      if (x_C > x_max) x_max = x_C;
      if (y_B < y_min) y_min = y_B;
      if (y_C < y_min) y_min = y_C;
      if (y_B > y_max) y_max = y_B;
      if (y_C > y_max) y_max = y_C;
      // printf("DEBUG: Lower-left point is (%d,%d). Upper-right point is (%d,%d)\n",
      //         x_min, y_min, x_max, y_max);

      // Use Barycentric Technique to determine whether points are within
      // the triangle defined by vertices A, B, and C. This technique is
      // described at following web page: 
      //    http://www.blackpawn.com/texts/pointinpoly/default.html
      int X_c_a = x_C - x_A; // X-component of vector from A to C
      int Y_c_a = y_C - y_A; // Y-component of vector from A to C
      int X_b_a = x_B - x_A; // X-component of vector from A to B
      int Y_b_a = y_B - y_A; // Y-component of vector from A to B

      // Calculate dot-products of vectors. Use long long integers to avoid
      // truncating. The dot-product values can get large.
      long long dot_ca_ca = (X_c_a * X_c_a) + (Y_c_a * Y_c_a); // Dot-product of vector CA with itself
                                                          // ('dot00' from above web page)
      long long dot_ca_ba = (X_c_a * X_b_a) + (Y_c_a * Y_b_a); // Dot-product of vector CA with vector
                                                          // BA ('dot01' from above web page)
      long long dot_ba_ba = (X_b_a * X_b_a) + (Y_b_a * Y_b_a); // Dot-product of vector BA with itself 
                                                         // ('dot11' from above web page)
      int X_p_a, Y_p_a; // X- and Y-components of vector from A to point P
      long long dot_ca_pa;    // Dot product of vector CA with vector PA
      long long dot_ba_pa;    // Dot product of vector BA with vector PA
      float u, v, denominator; // Vectors u and v, as defined in above web page.
                               // 'denominator is temporary variable.

      // Raster over the rectangle that circumscribes the triangle:
      for (x = x_min; x <= x_max; x++)  {
        for (y = y_min; y <= y_max; y++)  {
          // Skip this point if it's outside the map area:
          if (XY_coords_are_outside_of_map(x, y, mapInfo))
            continue;

          // Calculate more vector quantities necessary to determine whether point P
          // is inside of triangle:
          X_p_a = x - x_A; // X-component of vector from P to A
          Y_p_a = y - y_A; // Y-component of vector from P to A
          dot_ca_pa = (X_c_a * X_p_a) + (Y_c_a * Y_p_a); // Dot product of vector CA with vector PA
                                                         // ('dot02' from above web page)
          dot_ba_pa = (X_b_a * X_p_a) + (Y_b_a * Y_p_a); // Dot product of vector BA with vector PA 
                                                         // ('dot12' from above web page)
          // printf("DEBUG: dot_ca_ca=%lli, dot_ba_ba=%lli, dot_ca_ba=%lli, dot_ca_ba^2=%lli\n",
          //        dot_ca_ca, dot_ba_ba, dot_ca_ba, dot_ca_ba*dot_ca_ba);

          denominator = (float)(dot_ca_ca * dot_ba_ba - dot_ca_ba * dot_ca_ba);
          u = (float)((dot_ba_ba * dot_ca_pa) - (dot_ca_ba * dot_ba_pa)) / denominator;
          v = (float)((dot_ca_ca * dot_ba_pa) - (dot_ca_ba * dot_ca_pa)) / denominator;
          // printf("DEBUG: At point (%d,%d), denominator=%5.2f, u=%5.2f, v=%5.2f, u+v=%5.2f\n", 
          //        x, y, denominator, u, v, u+v);

          // Only proceed if point P at (x,y) is within the triangle. This is true only
          // if (u >= 0) and (v >= 0) and (u + v < 1).
          if ((u >= 0.0) && (v >= 0.0) && (u + v < 1.0)) {
            // printf("DEBUG: Point (%d,%d) is within triangle.\n", x, y);

            routing_layer_num = all_layer_num / 2; // Routing layer number is half the value of all-layer number

            // Based on whether command is PIN_SWAP or NO_PIN_SWAP, make the cell 
            // swappable or unswappable.
            if (strcasecmp(command, "PIN_SWAP") == 0)  {
              inPinSwapZone[x][y][routing_layer_num] = TRUE;
              // printf("DEBUG: pin_swappable = TRUE for (%d,%d,%d)\n", x, y, routing_layer_num);
            }
            else if (strcasecmp(command, "NO_PIN_SWAP") == 0)  {
              inPinSwapZone[x][y][routing_layer_num] = FALSE;
              // printf("DEBUG: pin_swappable = FALSE for (%d,%d,%d)\n", x, y, routing_layer_num);
            }  // End of if/else-clause for command == 'NO_PIN_SWAP'
            else {
              printf("\nERROR: An unexpected PIN_SWAP/NO_PIN_SWAP command of '%s' was encountered. Program is exiting.\n\n",
                      command);
              exit(1);
            }  // End of final else-clause for command not equal to PIN_SWAP or NO_PIN_SWAP

          }  // End of if-block for X/Y point within triangle
        }  // End of for-loop for index 'y'
      }  // End of for-loop for index 'x'

    }  // End of else-clause for command shape == 'TRI'

    else {
      printf("\n\nERROR: Program encountered a PIN_SWAP/NO_PIN_SWAP command of shape '%s' that is not\n", shape);
      printf("       recognized. Allowed shapes are ALL, RECT, CIR, and TRI (case insensitive).\n");
      printf("       Please fix input file. Program is exiting.\n\n");
      exit(1);
    }
      
  }  // End of for-loop for index 'swap_statement'

  //
  // Now that the 'cellInfo' matrix has been updated with locations of the user-defined
  // pin-swappable cells, we next need to determine which pin-swappable cells are
  // contiguous (touch each other), and then label each set of contiguous zones with a
  // unique numeric label (1, 2, 3, ...). The algorithm uses the 'union-find' approach
  // outlined by University of Washington computer science course, but extended to
  // three dimensions:
  //
  //   https://courses.cs.washington.edu/courses/cse576/book/ch3.pdf
  //
  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);
  printf("\nDEBUG:  Starting process of identifying contiguous pin-swappable cells at %02d-%02d-%d, %02d:%02d.\n",
         now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min);


  // Initialize 'swap_zone_label' variable to 1:
  int swap_zone_label = 1; 

  // Initialize the 'parents' array used by the union-find algorithm.
  // The value of parents[i] is the label of the parent of i, or zero if i 
  // is a root node and has no parent. The size of the 'parents' array may grow
  // beyond 'maxSwapZones' before contiguous swap-zones are merged, so provide
  // a 10-fold safety buffer.
  int max_parents_elements = 10 * maxSwapZones;
  int *parents = malloc( max_parents_elements * sizeof(int));
  // printf("DEBUG: 'parents' array allocated to have %d elements (10 times 'maxSwapZones').\n", max_parents_elements);
  for (int i = 0; i < max_parents_elements; i++)  {
    parents[i] = 0;
  }

  //
  // Pass #1 assigns initial labels to each row 
  //
  // printf("DEBUG: Starting Pass #1...\n");
  for (z = 0; z < mapInfo->numLayers; z++)  {
    for (y = 0; y < mapInfo->mapHeight; y++)  {

      // Process row y:
      for (x = 0; x < mapInfo->mapWidth; x++)  {
        if (inPinSwapZone[x][y][z])  {

          // Check whether the neighbors of this cell were also pin-swappable:
          swappable_neighbors = prior_neighbors(x, y, z, inPinSwapZone);

          // 0th element is number of pin-swappable neighbors:
          num_swappable_neighbors = swappable_neighbors[0];

          // printf("DEBUG: Cell at (%d, %d %d) is originally in pin-swappable zone #%d.\n", x, y, z,
          //        temp_swap_labels[x][y][z]);
          // printf("       It has %d pin-swappable neighbors.\n", num_swappable_neighbors);

          if (num_swappable_neighbors == 0)  {  
            // No neighbors are pin-swappable, so assign the current cell's label to
            // 'swap_zone_label' and then increment this label:
            temp_swap_labels[x][y][z] = swap_zone_label;
            // printf("          cell assigned to swap zone %d\n", temp_swap_labels[x][y][z]);
            swap_zone_label++;
            // printf("          swap_zone_label incremented to %d\n", swap_zone_label);

            // Check that the swap label doesn't exceed the dimension of the 'parents[]' array:
            if (swap_zone_label >= max_parents_elements)  {
              printf("\nERROR: The number of pin-swap labels exceeded the anticipated maximum (%d)\n", max_parents_elements);
              printf("       while calculating which swap-zones are contiguous. This represents an error\n");
              printf("       in the software. Alert the software developer of this bug. Program is exiting.\n\n");
              exit(1);
            }  // End of if-block to compare minimum_swap_label and max_parents_elements

          }
          else {
            // Neighbors were found that are pin-swappable, so assign the current cell's
            // label to the minimum label number of the pin-swappable neighbors:

            // Initialize the 'minimum' value to a number that's larger than any current swap zone
            minimum_swap_label = swap_zone_label + 1; 

            // Check that the swap label doesn't exceed the dimension of the 'parents[]' array:
            if (minimum_swap_label >= max_parents_elements)  {
              printf("\nERROR: The number of pin-swap labels exceeded the anticipated maximum (%d)\n", max_parents_elements);
              printf("       while calculating which swap-zones are contiguous. This represents an error\n");
              printf("       in the software. Alert the software developer of this bug. Program is exiting.\n\n");
              exit(1);
            }  // End of if-block to compare minimum_swap_label and max_parents_elements

            for (i = 0; i < num_swappable_neighbors; i++)  {
              x_neighbor = swappable_neighbors[i*3 + 1];
              y_neighbor = swappable_neighbors[i*3 + 2];
              z_neighbor = swappable_neighbors[i*3 + 3];
              // printf("DEBUG:  Neighbor #%d: cell at (%d, %d, %d) is part of swap zone %d.\n",
              //         i+1, x_neighbor, y_neighbor, z_neighbor, temp_swap_labels[x_neighbor][y_neighbor][z_neighbor]);
              if (temp_swap_labels[x_neighbor][y_neighbor][z_neighbor] < minimum_swap_label)
                minimum_swap_label = temp_swap_labels[x_neighbor][y_neighbor][z_neighbor];
            }  // End of for-loop for index 'i'

            // Assign current cell's label to the minimum label number of pin-swappable neighbors:
            temp_swap_labels[x][y][z] = minimum_swap_label;

            // printf("          cell assigned to swap zone %d (minimum of neighbors' swap-zones).\n", temp_swap_labels[x][y][z]);

          }  // End of if/else block 

          // Update the 'parents' array:
          int current_swap_label = temp_swap_labels[x][y][z];
          int neighbor_swap_label;
          for (i = 0; i < num_swappable_neighbors; i++)  {
            x_neighbor = swappable_neighbors[i*3 + 1];
            y_neighbor = swappable_neighbors[i*3 + 2];
            z_neighbor = swappable_neighbors[i*3 + 3];
            neighbor_swap_label = temp_swap_labels[x_neighbor][y_neighbor][z_neighbor];

            if (neighbor_swap_label != current_swap_label)  { 
              // printf("DEBUG:    Updating 'parents' array by merging current swap label %d and neighbor swap label %d.\n",
              //        current_swap_label, neighbor_swap_label);
              merge_labels(current_swap_label, neighbor_swap_label, parents);
            }  // End of if-block
          }  // End of for-loop for index 'i'

        }  // End of if-block for inPinSwapZone == TRUE

      }  // End of for-loop for index 'x'
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'z'
  // printf("DEBUG: Done with Pass #1.\n");

  // Define 'label_mapping' integer array that we'll use for re-mapping
  // labels to a consecutive sequence (1, 2, 3, etc):
  int *label_mapping = malloc(maxSwapZones * sizeof(int));
  // printf("DEBUG: Address of 'label_mapping' is %p.\n", label_mapping);
  for (i = 0; i < maxSwapZones; i++)  {
    label_mapping[i] = 0;
    // printf("DEBUG  Address of 'label_mapping[%d]' is %p.\n", i, &label_mapping[i]);
  }

  //
  // Pass #2 replaces Pass-1 labels with equivalence class labels:
  //
  // printf("DEBUG: Starting Pass #2...\n");
  #pragma omp parallel for collapse(3) schedule (dynamic, 1)
  for (z = 0; z < mapInfo->numLayers; z++)  {
    for (y = 0; y < mapInfo->mapHeight; y++)  {
      for (x = 0; x < mapInfo->mapWidth; x++)  {

        if (inPinSwapZone[x][y][z])  {
          // printf("DEBUG: Cell at (%d, %d, %d) is pin-swappable in swap-zone %d, so about to find the equivalent swap_zone.\n",
          //         x, y, z, temp_swap_labels[x][y][z]);
          temp_swap_labels[x][y][z] = find(temp_swap_labels[x][y][z], parents);
          // printf("DEBUG: ...equivalent swap_zone defined as %d.\n", temp_swap_labels[x][y][z]);
          label_mapping[temp_swap_labels[x][y][z]] = 1; // Flag this swap-zone label as being used
        }

      }  // End of for-loop for index 'x'
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'z'

  //
  // Pass #3 replaces Pass-2 labels with label numbers that are sequential,
  // without any missing gaps (i.e., 1, 2, 3, 4, ...).
  //
  // printf("DEBUG: Starting Pass #3...\n");
  // First, re-map the labels to a set of sequential integers, starting with 1:
  int num_swap_zones = 0;
  // printf("DEBUG: About to re-map swap zones to a consecutive set of integers...\n");
  for (i = 1; i < maxSwapZones; i++)  {
    if (label_mapping[i])  {
      num_swap_zones++;
      // Label 'i' is used, so assign the 'label_mapping' value to 
      // the (potentially) new label number that's sequential:
      // printf("DEBUG: Re-mapping swap-label '%d' to '%d'.\n", i, num_swap_zones);
      label_mapping[i] = num_swap_zones;
    }
  }  // End of for-loop

  // Confirm that the number of swap zones is less than 255 ('maxSwapZones'), so that
  // the swap-zone label will fit within the 8-bit field in the cellInfo structure:
  if (num_swap_zones > maxSwapZones)  {
    printf("\nERROR: The number of discrete pin-swappable zones (%d) exceeds the allowed number (%d).\n",
            num_swap_zones, maxSwapZones - 1);
    printf("       Modify the PIN_SWAP and NO_PIN_SWAP statements in the input file and re-start the program.\n\n");
    exit(1);
  }

  // Re-map the labels to sequential values at every cell. Also eliminate
  // un-walkability due to proximity to user-defined barriers:
  // printf("DEBUG: Starting Pass 3...\n");
  #pragma omp parallel for collapse(3) schedule(dynamic, 1)
  for (z = 0; z < mapInfo->numLayers; z++)  {
    for (y = 0; y < mapInfo->mapHeight; y++)  {
      for (x = 0; x < mapInfo->mapWidth; x++)  {
        if (inPinSwapZone[x][y][z])  {
          // Re-map labels:
          cellInfo[x][y][z].swap_zone = label_mapping[temp_swap_labels[x][y][z]];
          // printf("DEBUG: At (%d,%d,%d), swap-label %d was replaced with %d\n", x, y, z,
          //        temp_swap_labels[x][y][z], cellInfo[x][y][z].swap_zone);

          // Remove routing barriers due to proximity to user-defined barriers in
          // pin-swappable regions:
          for (int DR_subset = 0; DR_subset < maxDesignRuleSubsets; DR_subset++)  {
            for (int shape_type = 0; shape_type < NUM_SHAPE_TYPES; shape_type++)  {
              clear_unwalkable_barrier_proximity(&(cellInfo[x][y][z]), DR_subset, shape_type);
            }  // End of for-loop for index 'shape_type'
          }  // End of for-loop for index 'DR_subset'
        }  // End of if-block for inPinSwapZone == TRUE

      }  // End of for-loop for index 'x'
    }  // End of for-loop for index 'y'
  }  // End of for-loop for index 'z'

  //
  // Free the memory used by the temporary 3-dimensional matrices 'temp_swap_labels'
  // and 'inPinSwapZone':
  //
  for (x = 0; x < mapInfo->mapWidth; x++){
    for (int y = 0; y < mapInfo->mapHeight; y++)  {
      free(temp_swap_labels[x][y]);
      temp_swap_labels[x][y] = NULL;  // Set pointer to NULL as a precaution

      free(inPinSwapZone[x][y]);
      inPinSwapZone[x][y] = NULL;  // Set pointer to NULL as a precaution
    }
    free(temp_swap_labels[x]);
    temp_swap_labels[x] = NULL;  // Set pointer to NULL as a precaution

    free(inPinSwapZone[x]);
    inPinSwapZone[x] = NULL;  // Set pointer to NULL as a precaution
  }
  free(temp_swap_labels);
  temp_swap_labels = NULL;  // Set pointer to NULL as a precaution

  free(inPinSwapZone);
  inPinSwapZone = NULL;  // Set pointer to NULL as a precaution


  // Free the 'parents' array:
  free(parents);
  parents = NULL;  // Set pointer to NULL as a precaution

  // Free the 'label_mapping' array:
  free(label_mapping);
  label_mapping = NULL;  // Set pointer to zero as a precaution

  tim = time(NULL);
  now = localtime(&tim);
  printf("INFO: Completed process of identifying contiguous pin-swappable cells at %02d-%02d-%d, %02d:%02d.\n",
         now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min);

  printf("INFO: Number of non-contiguous, pin-swappable zones is %d.\n", num_swap_zones);

  //
  // Finally, map the swap-zone number to each net contained in the swap zone. This
  // is done by adding the swap-zone number to each path's element in the 
  // 'swapZone' array:
  //
  int swap_zone_start, swap_zone_target;
  int max_routed_nets = user_inputs->num_nets + user_inputs->num_pseudo_nets;
  for (i = 0; i < max_routed_nets; i++)  {
    swap_zone_start  = cellInfo[mapInfo->start_cells[i].X ][mapInfo->start_cells[i].Y ][mapInfo->start_cells[i].Z ].swap_zone;
    swap_zone_target = cellInfo[mapInfo->end_cells[i].X][mapInfo->end_cells[i].Y][mapInfo->end_cells[i].Z].swap_zone;

    // printf("DEBUG: For path %d, swap zone = %d at origin\n", i, swap_zone_start);
    // printf("                    swap zone = %d at target\n", swap_zone_target);

    // Issue error message and exit if a path's start- and end-locations are located 
    // in pin-swappable zones:
    if ((swap_zone_start) && (swap_zone_target))  {
      printf("\nERROR: The starting and ending coordinates of path #%d ('%s')\n", i, user_inputs->net_name[i]);
      printf("       occupy pin-swappable zones. This is not allowed; only one\n");
      printf("       of each path's terminals may be in a pin-swappable zone. The coordinates are:\n");
      printf("         Start: (%6.3f, %6.3f) microns on layer %s\n", user_inputs->start_X_um[i],
              user_inputs->start_Y_um[i], user_inputs->layer_names[2*mapInfo->start_cells[i].Z] );
      printf("           End: (%6.3f, %6.3f) microns on layer %s\n", user_inputs->end_X_um[i],
              user_inputs->end_Y_um[i], user_inputs->layer_names[2*mapInfo->end_cells[i].Z] );
      printf("       Fix the input file and re-start the program.\n\n");
      printf("DEBUG:  Start: cellInfo[%d][%d][%d].swap_zone = %d.\n",
               mapInfo->start_cells[i].X, mapInfo->start_cells[i].Y, mapInfo->start_cells[i].Z,
               cellInfo[mapInfo->start_cells[i].X][mapInfo->start_cells[i].Y][mapInfo->start_cells[i].Z].swap_zone);
      printf("DEBUG: Target: cellInfo[%d][%d][%d].swap_zone = %d.\n",
               mapInfo->end_cells[i].X, mapInfo->end_cells[i].Y, mapInfo->end_cells[i].Z,
               cellInfo[mapInfo->end_cells[i].X][mapInfo->end_cells[i].Y][mapInfo->end_cells[i].Z].swap_zone);
      exit(1);
    }  // End of if-block for start- and end-points occupying same pin-swap zone

    // If the path's **target** coordinates are in a pin-swappable zone, then
    // swap the starting coordinates with the target coordinates so that only the
    // starting coordinates are in the pin-swappable zone:
    if (swap_zone_target)  {
      swapStartAndEndTerminals(i, mapInfo);
      printf("INFO: The start- and end-terminals have been swapped for net #%d ('%s') because end-terminals may not be located in a swap-zone.\n",
              i, user_inputs->net_name[i]);
      // printf("DEBUG:   Net #%d's start-terminal is at (%d,%d,%d), and end-terminal is at (%d,%d,%d)\n", i,
      //        mapInfo->start_cells[i].X, mapInfo->start_cells[i].Y, mapInfo->start_cells[i].Z,
      //        mapInfo->end_cells[i].X,   mapInfo->end_cells[i].Y,   mapInfo->end_cells[i].Z);

      // Assign swap-zone number to the 'swapZone' array element for this path:
      mapInfo->swapZone[i]  = swap_zone_target;
    }
    else {
      // Assign swap-zone number to the 'swapZone' array element for this path:
      mapInfo->swapZone[i]  = swap_zone_start;
    }

    // If the current path's starting terminal is in a non-zero swap zone, then report
    // this information to the log file:
    if (mapInfo->swapZone[i])  {


      // If the the current path is a differential pair, then:
      //   a.) ensure that the 'isPNswappable' flag is set to TRUE for all associated paths, and
      //   b.) set to zero the pitch of the start-terminals.
      if (user_inputs->isDiffPair[i])  {

        printf("INFO: Starting location (%d, %d, %d) of diff-pair path %d ('%s') is mapped to pin-swappable zone %d.\n",
               mapInfo->start_cells[i].X, mapInfo->start_cells[i].Y, mapInfo->start_cells[i].Z, i, user_inputs->net_name[i], mapInfo->swapZone[i]);

        // Net is a differential pair, so set isPNswappable to TRUE for this net:
        user_inputs->isPNswappable[i] = TRUE;
        user_inputs->isPNswappable[user_inputs->diffPairPartner[i]] = TRUE;  // Do the same for its partner net
        user_inputs->isPNswappable[user_inputs->diffPairToPseudoNetMap[i]] = TRUE;  // Do the same for its "parent" pseudo-net

        // Next, set the diff-pair's start-terminal pitch to zero because the start-terminals
        // are in a pin-swap zone:
        user_inputs->diffPairStartTermPitch[i] = 0;           // Set pitch to zero for the diff-pair
        user_inputs->diffPairStartTermPitchMicrons[i] = 0.0;

        user_inputs->diffPairStartTermPitch[user_inputs->diffPairPartner[i]] = 0;  // Set pitch to zero for the diff-pair's partner
        user_inputs->diffPairStartTermPitchMicrons[user_inputs->diffPairPartner[i]] = 0.0;

        user_inputs->diffPairStartTermPitch[user_inputs->diffPairToPseudoNetMap[i]] = 0;  // Set pitch to zero for the diff-pair's pseudo-net
        user_inputs->diffPairStartTermPitchMicrons[user_inputs->diffPairToPseudoNetMap[i]] = 0.0;

      }  // End of if-block for isDiffPair[i] == TRUE

      // If the the current path is a pseudo-net, then:
      //   a.) ensure that the 'isPNswappable' flag is set to TRUE for all associated paths, and
      //   b.) set to zero the pitch of the start-terminals.
      if (user_inputs->isPseudoNet[i])  {

        printf("INFO: Starting location (%d, %d, %d) of diff-pair pseudo-path %d ('%s') is mapped to pin-swappable zone %d.\n",
               mapInfo->start_cells[i].X, mapInfo->start_cells[i].Y, mapInfo->start_cells[i].Z, i, user_inputs->net_name[i], mapInfo->swapZone[i]);

        // Net is a pseudo-net, so set isPNswappable to TRUE for this net:
        user_inputs->isPNswappable[i] = TRUE;
        user_inputs->isPNswappable[user_inputs->pseudoNetToDiffPair_1[i]] = TRUE;  // Do the same for its two
        user_inputs->isPNswappable[user_inputs->pseudoNetToDiffPair_2[i]] = TRUE;  //   children (diff-pair) nets

        // Next, set the pseudo-net's start-terminal pitch to zero because the start-terminals
        // are in a pin-swap zone:
        user_inputs->diffPairStartTermPitch[i] = 0;           // Set pitch to zero for the pseudo-path
        user_inputs->diffPairStartTermPitchMicrons[i] = 0.0;

        user_inputs->diffPairStartTermPitch[user_inputs->pseudoNetToDiffPair_1[i]] = 0;           // Do the same for its
        user_inputs->diffPairStartTermPitchMicrons[user_inputs->pseudoNetToDiffPair_1[i]] = 0.0;  //   two (diff-pair) nets.
        user_inputs->diffPairStartTermPitch[user_inputs->pseudoNetToDiffPair_2[i]] = 0;
        user_inputs->diffPairStartTermPitchMicrons[user_inputs->pseudoNetToDiffPair_2[i]] = 0.0;

      }  // End of if-block for isPseudoNet[i] == TRUE

    }  // End of if-block for swapZone[i] > 0

  }  // End of for-loop for index 'i' (0 to max_routed_nets)

}  // End of function 'definePinSwapZones'

