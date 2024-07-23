//-----------------------------------------------------------------------------
// Name: findShortPathHeuristically
// Desc: Insert extra steps within path number 'pathNum' between points
//       mapInfo->start_cells[pathNum] and mapInfo->end_cells[pathNum].
//       These points must be within 5 cells of each other, or else a fatal
//       error is issued. This function modifies the variables at addresses
//       &insertedCoords and &num_inserted_segments_in_gap. Depending on the
//       arrangement of the start- and end-cells, the function attempts to find
//       a low-cost path between them.
//-----------------------------------------------------------------------------
int findShortPathHeuristically(const Coordinate_t startCoord, const Coordinate_t endCoord, CellInfo_t ***cellInfo,
                               int pathNum, Coordinate_t *insertedCoords[], int *num_inserted_segments_in_gap,
                               InputValues_t *user_inputs, const MapInfo_t *mapInfo);

