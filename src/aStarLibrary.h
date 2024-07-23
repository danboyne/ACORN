#ifndef A_STAR_LIBRARY_H

#define A_STAR_LIBRARY_H

//-----------------------------------------------------------------------------
// Name: createNoRoutingRestrictions
// Desc: Initialize elements of variable 'routingRestrictions' such that
//       this variable can be fed into function 'findPath()' for general routing
//       without any restrictions.
//-----------------------------------------------------------------------------
void createNoRoutingRestrictions(RoutingRestriction_t * routingRestrictions);


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
                         const float evaporationRate, int num_threads);


//-----------------------------------------------------------------------------
// Name: directionToText
// Desc: Given a direction, routeDir, this function generates a text string
//       that corresponds to that routing direction. See the definitions of
//       routing directions in file 'global_defs.h' for which text string is
//       appropriate for which value of the routing direction. The text string
//       is written to the character buffer starting at routeDescription.
//-----------------------------------------------------------------------------
void directionToText(const int routeDir, char * routeDescription);


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
                   unsigned short shapeType, unsigned congestionPenalty);


//-----------------------------------------------------------------------------
// Name: initializePathFindingArrays
// Desc: Initialize all elements in the various path-finding arrays to values
//       appropriate for the beginning of function 'findPath()'.
//-----------------------------------------------------------------------------
void initializePathFindingArrays(PathFinding_t *pathFinding, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: calcMinimumAllowedDirection
// Desc: Given two routing directions, routeDir_1 and routeDir_2, this function
//       returns the more restrictive routing direction. For cases where there
//       is zero overlap between the two directions, we return the logical OR
//       of the two directions (the superset) -- unless one of the directions
//       is 'NONE', in which case we return 'NONE'.
//-----------------------------------------------------------------------------
int calcMinimumAllowedDirection(const int routeDir_1, const int routeDir_2);


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
int allowedDirection(const int deltaX, const int deltaY, const int deltaZ, const int allowedDir);


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
                                 CellInfo_t ***const  cellInfo);


//-----------------------------------------------------------------------------
// Name: record_DRC_by_paths
// Desc: Record the design-rule violation between path number (path_1) and its
//       shape-type (shapeType_1) and offending path number (path_2) and its
//       shape-type (shapeType_2).
//-----------------------------------------------------------------------------
void record_DRC_by_paths(int numPaths, char DRCs[], int path_1, int shapeType_1,
                         int path_2, int shapeType_2);


//-----------------------------------------------------------------------------
// Name: check_for_DRC
// Desc: Read the 'DRCs[]' array to determine whether a design-rule violation
//       has previously been recorded between path number (path_1) and its
//       shape-type (shapeType_1), and offending path number (path_2) and its
//       shape-type (shapeType_2).
//-----------------------------------------------------------------------------
unsigned char check_for_DRC(char DRCs[], unsigned int numPaths, int path_1, int shapeType_1,
                            int path_2, int shapeType_2);


//-----------------------------------------------------------------------------
// Name: calcPathMetrics
// Desc: Calculate path-specific metrics like path length, via count, etc. Also
//       mark the centerlines of each path and via. If Boolean flag
//       'exitIfInvalidJump' is TRUE, the program will die if an invalid
//       jump is detected between segments. This should be the default behavior
//       when checking most paths, but not for sub-maps of diff-pair connections,
//       in which paths may exit and re-enter the sub-map.
//-----------------------------------------------------------------------------
void calcPathMetrics(const int total_nets, const InputValues_t *user_inputs,
                     const MapInfo_t *mapInfo, const int pathLengths[],
                     Coordinate_t *pathCoords[], CellInfo_t ***cellInfo,
                     RoutingMetrics_t *routability, int exitIfInvalidJump);


//-----------------------------------------------------------------------------
// Name: markCellsNearCenterlinesInMap
// Desc: Flag cells that are near the centers of (contiguous) paths, so we can avoid
//       other cells when checking design rules. The 'near_a_net' element in the
//       cellInfo 3D matrix is set to TRUE for cells that are near a path-center.
//
//       Note: this function can be parallelized into multiple threads if the CPU
//       architecture allows atomic writes to single-bit elements of a structure.
//-----------------------------------------------------------------------------
void markCellsNearCenterlinesInMap(const int total_nets, const MapInfo_t *mapInfo, int contiguousPathLength[],
                                   Coordinate_t *contigPathCoords[], const InputValues_t *user_inputs,
                                   CellInfo_t ***cellInfo);


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
void markPathCenterlinesInMap(const int total_nets, int contiguousPathLength[],
                              Coordinate_t *contigPathCoords[], CellInfo_t ***cellInfo,
                              const MapInfo_t *mapInfo, const RoutingMetrics_t *routability,
                              const InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: calc_fraction_of_recent_iterations_with_DRCs
// Desc: Return the floating-point fraction (from 0 to 1.0) of the number of
//       recent iterations that contained any design-rule violations for
//       the path whose 'recent_path_DRC_cells' array is given among the input
//       parameters over 'num_iterations' iterations. The maximum allowed
//       value for 'num_iterations' is numIterationsToReEquilibrate.
//-----------------------------------------------------------------------------
float calc_fraction_of_recent_iterations_with_DRCs(unsigned int * recent_path_DRC_cells,
                                                   int num_iterations);


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
void determineIfMetricsPlateaued(const MapInfo_t *mapInfo, RoutingMetrics_t *routability);


//-----------------------------------------------------------------------------
// Name: calc_distance_G_cost
// Desc: Calculate the distance component of the G-cost between target
//       point (x,y,z) and parent point (parentX, parentY, parentZ). Include
//       effects of cost-multipliers if (x,y,z) is not in a pin-swappable zone.
//       The parent- and target-points must both be walkable. If there are
//       corner-cells between the parent and target, these cell must also
//       be walkable.
//-----------------------------------------------------------------------------
unsigned long calc_distance_G_cost(int x, int y, int z, int parentX, int parentY, int parentZ,
                                   InputValues_t *user_inputs, CellInfo_t ***const  cellInfo,
                                   const MapInfo_t *mapInfo, int pathNum);


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
int calc_via_congestion(const int path, const unsigned short target_DR_num, const unsigned short target_DR_subset,
                        const unsigned short parent_DR_num, const unsigned short parent_DR_subset,
                        CellInfo_t ***const  cellInfo, const InputValues_t *user_inputs, const MapInfo_t *mapInfo,
                        const int x, const int y, const int parentZ, const int targetZ,
                        const int excludeCongestion, const int excludePathNum, const int recognizeSelfCongestion);


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
int calc_corner_congestion(const int path, const unsigned short shapeType,
                           CellInfo_t ***const cellInfo, const InputValues_t *user_inputs,
                           const MapInfo_t *mapInfo, const int parentX, const int parentY, const int z,
                           const int x, const int y, const int criteria_X_delta, const int criteria_Y_delta,
                           const int corn1_X_delta,    const int corn1_Y_delta,
                           const int corn2_X_delta,    const int corn2_Y_delta,
                           const int excludeCongestion, const int excludePathNum, const int recognizeSelfCongestion);


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
long calc_congestion_penalty(const int x, const int y, const int z,
                             const int parentX, const int parentY, const int parentZ,
                             const int pathNum, const unsigned short shapeType,
                             CellInfo_t ***const cellInfo, const InputValues_t *user_inputs,
                             const MapInfo_t *mapInfo, const int excludeCongestion,
                             const int excludePathNum, const int recognizeSelfCongestion);


//-----------------------------------------------------------------------------
// Name: allocateMapInfo
// Desc: Allocate memory for variable of type MapInfo_t.
//-----------------------------------------------------------------------------
void allocateMapInfo(MapInfo_t *mapInfo, int num_nonPseudo_nets, int num_pseudo_nets,
                     int num_routing_layers);


//-----------------------------------------------------------------------------
// Name: initializePathfinder
// Desc: Allocates small amount of memory for the arrays used to store paths.
//       Because we don't know the eventual lengths of each path, only space for
//       one (x,y,z) location is allocated in each of the two arrays:
//       (1) pathCoords and (2) contigPathCoords.
//-----------------------------------------------------------------------------
void initializePathfinder(int numPaths, int *pathLengths, Coordinate_t *pathCoords[],
                          int *contiguousPathLengths, Coordinate_t *contigPathCoords[]);


//-----------------------------------------------------------------------------
// Name: allocateCellInfo
// Desc: Allocates memory for the 3D cellInfo array.
//-----------------------------------------------------------------------------
CellInfo_t *** allocateCellInfo(MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: initializeCellInfo
// Desc: Initialize the 3D cellInfo array. The format of each element is
//       provided in the header file.
//-----------------------------------------------------------------------------
void initializeCellInfo(CellInfo_t ***cellInfo, MapInfo_t *mapInfo);


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
void reInitializeCellInfo(const MapInfo_t *mapInfo, CellInfo_t ***cellInfo);


//-----------------------------------------------------------------------------
// Name: initializeRoutability
// Desc: Initialize elements in variable 'routability'.
//-----------------------------------------------------------------------------
void initializeRoutability(RoutingMetrics_t *routability, const MapInfo_t *mapInfo,
                           unsigned char initialize_ALL_elements);


//-----------------------------------------------------------------------------
// Name: allocatePathFindingArrays
// Desc: Allocates memory for the large arrays used by the path-finding
//       function, findPath().
//-----------------------------------------------------------------------------
void allocatePathFindingArrays(PathFinding_t *pathFinding, MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: endPathfinder
// Desc: Free memory in the arrays that are used for storing path coordinates.
//       This function frees the memory that was allocated in function
//       'initializePathfinder'.
//-----------------------------------------------------------------------------
void endPathfinder(int numPaths, Coordinate_t *pathCoords[],
                   Coordinate_t *contigPathCoords[]);


//-----------------------------------------------------------------------------
// Name: freeMemory_cellInfo
// Desc: Free memory that was allocated dynamically in function
//       'allocateCellInfo'.
//-----------------------------------------------------------------------------
void freeMemory_cellInfo(MapInfo_t *mapInfo, CellInfo_t ***cellInfo);


//-----------------------------------------------------------------------------
// Name: freePathFindingArrays
// Desc: Free memory that was allocated dynamically in function
//       'allocatePathFindingArrays'.
//-----------------------------------------------------------------------------
void freePathFindingArrays(PathFinding_t *pathFinding, MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: freeMemory_mapInfo
// Desc: Free the memory that was allocated in function 'allocateMapInfo'.
//-----------------------------------------------------------------------------
void freeMemory_mapInfo(MapInfo_t *mapInfo);




#endif
