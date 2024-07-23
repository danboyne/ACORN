#ifndef PROCESS_DIFF_PAIRS_H

#define PROCESS_DIFF_PAIRS_H


//-----------------------------------------------------------------------------
// Name: calcGapRoutingRestrictions
// Desc: Calculate the routing restrictions for function findPath() to use when
//       routing across a gap of a diff-pair path. The restrictions depend on
//       the number of layers spanned by the gap, and whether a pseudo-via can
//       be associated with the gap. This function modifies the variable
//       routeRestrictions.
//-----------------------------------------------------------------------------
void calcGapRoutingRestrictions(RoutingRestriction_t *routeRestrictions,
                                Coordinate_t startCoordinate, Coordinate_t endCoordinate,
                                int pathNum, Coordinate_t *pathCoords[], int *pathLengths,
                                unsigned char pseudoViaKnown, int pseudoVia_X, int pseudoVia_Y,
                                CellInfo_t ***cellInfo, MapInfo_t *mapInfo, InputValues_t *user_inputs);


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
int detectDiffPairConnections(ShoulderConnections_t * connections, MapInfo_t *mapInfo,
                              InputValues_t *user_inputs, Coordinate_t *pathCoords[],
                              int pathLengths[]);


//-----------------------------------------------------------------------------
// Name: deleteDuplicatePoints
// Desc: For path number 'pathNum' iterate through the segments sequentially
//       and delete any segments that are adjacent in the path and also have
//       the same coordinates. By deleting points, this function changes the
//       'pathCoords' array and the 'pathLengths' array.
//-----------------------------------------------------------------------------
void deleteDuplicatePoints(const int pathNum, Coordinate_t *pathCoords[],
                           int pathLengths[], MapInfo_t *mapInfo);


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
                                MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: postProcessDiffPairs
// Desc: Create diff-pair nets using the pseudo-nets routed by the auto-router.
//-----------------------------------------------------------------------------
void postProcessDiffPairs(Coordinate_t *pathCoords[], int pathLengths[], InputValues_t *user_inputs,
                          CellInfo_t ***cellInfo, MapInfo_t *mapInfo, RoutingMetrics_t *routability,
                          PathFinding_t *pathFinding, RoutingMetrics_t subMapRoutability[2],
                          RoutingRestriction_t *noRoutingRestrictions, int num_threads);



#endif
