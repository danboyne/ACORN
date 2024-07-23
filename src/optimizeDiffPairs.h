#ifndef OPTIMIZE_DIFF_PAIRS_H

#define OPTIMIZE_DIFF_PAIRS_H



//-----------------------------------------------------------------------------
// Name: optimizeDiffPairConnections
// Desc: Optimize the connections between diff-pair shoulder-paths and the
//       corresponding diff-pair vias and terminals, respecting whether the
//       diff-pair is P/N-swappable. This function modifies the pathCoords[][]
//       array and the pathLengths[] array. For P/N-swappable diff-pairs, this
//       function may also modify mapInfo->start_cells[] and
//       mapInfo->diff_pair_terms_swapped[].
//-----------------------------------------------------------------------------
void optimizeDiffPairConnections(Coordinate_t *pathCoords[], int pathLengths[], CellInfo_t ***cellInfo,
                                 MapInfo_t *mapInfo, InputValues_t *user_inputs, RoutingMetrics_t *routability,
                                 RoutingMetrics_t subMapRoutability[2], RoutingRestriction_t *noRoutingRestrictions,
                                 int num_threads);


#endif
