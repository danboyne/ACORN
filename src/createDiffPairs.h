#ifndef CREATE_DIFF_PAIRS_H

#define CREATE_DIFF_PAIRS_H

//-----------------------------------------------------------------------------
// Name: createDiffPairShoulderPoints
// Desc: Populate two arrays of path-coordinates: one for each diff-pair net on
//       either side of the pseudo-net contained in array 'pseudoPathCoords[]'.
//       This function places the coordinates of the two diff-pair nets in the
//       arrays pathCoords[p1] and pathCoords[p2], where p1 and p2 are the
//       diff-pair nets associated with the pseudo-path 'pseudoNetNumber' (in
//       the sequence that these nets fall in the user-defined netlist).
//-----------------------------------------------------------------------------
void createDiffPairShoulderPoints(const int pseudoNetNumber, Coordinate_t *pathCoords[],
                                  int pathLength[], const InputValues_t *user_inputs,
                                  CellInfo_t ***cellInfo, const MapInfo_t *mapInfo);


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
Vector2dFloat_t calcUnitVectorToDiffPairVia_wrapper(int pseudoPathNum, int viaStartSegment,
                                                    int viaEndSegment, int pathLengths[],
                                                    Coordinate_t *pathCoords[], const MapInfo_t *mapInfo,
                                                    CellInfo_t ***cellInfo, float pseudoVia_to_diffPairVia_distance);


//-----------------------------------------------------------------------------
// Name: createDiffPairVias
// Desc: Create diff-pair vias for the shoulder points alongside pseudo-net
//       'pseudoPathNum. The two diff-pair paths are 'path_1_number' and
//       'path_2_number'. This function modifies the structure that contains
//       the path segments, 'pathCoords', as well as the array that contains
//       the path lengths, 'pathLengths'.
//-----------------------------------------------------------------------------
void createDiffPairVias(const int pseudoPathNum, const int path_1_number, const int path_2_number,
                        Coordinate_t *pathCoords[], int pathLengths[], const InputValues_t *user_inputs,
                        CellInfo_t ***cellInfo, const MapInfo_t *mapInfo);


#endif
