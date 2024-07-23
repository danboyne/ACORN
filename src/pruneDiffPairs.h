#ifndef PRUNE_DIFF_PAIRS_H

#define PRUNE_DIFF_PAIRS_H


//-----------------------------------------------------------------------------
// Name: deleteSelectedDiffPairSegments
// Desc: Delete selected diff-pair segments associated with pseudo-path
//       'pseudoPathNum'. The selected segments are (a) near pseudo-vias,
//       (b) near design-rule boundaries, and (c) near terminals.
//-----------------------------------------------------------------------------
void deleteSelectedDiffPairSegments(const int pseudoPathNum, Coordinate_t *pathCoords[],
                                    int pathLengths[], InputValues_t *user_inputs,
                                    CellInfo_t ***cellInfo, MapInfo_t *mapInfo);


#endif
