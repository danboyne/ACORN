//-----------------------------------------------------------------------------
// Name: createRoutability
// Desc: Allocate memory for the routing metrics data structure. This function
//       does not initialize elements to known values.
//-----------------------------------------------------------------------------
void createRoutability(RoutingMetrics_t *routability, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: freeMemory_routability
// Desc: Free memory that was allocated in function 'initializeRoutability'.
//-----------------------------------------------------------------------------
void freeMemory_routability(RoutingMetrics_t *routability, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: print_cell_congestion
// Desc: Print out the congestion at a given cell in the cellInfo matrix. This
//       function is intended only for debugging.
//-----------------------------------------------------------------------------
void print_cell_congestion(CellInfo_t *cellInfo);


//-----------------------------------------------------------------------------
// Name: addCongestionAroundAllTerminals
// Desc: Add congestion (in the 'cellInfo' 3D matrix) at each start- and end-terminal
//       of all non-pseudo-paths.
//-----------------------------------------------------------------------------
void addCongestionAroundAllTerminals(const InputValues_t *user_inputs, const MapInfo_t *mapInfo,
                                     CellInfo_t ***cellInfo, Coordinate_t *contigPathCoords[],
                                     int contiguousPathLength[]);


//-----------------------------------------------------------------------------
// Name: update_iterationDependent_parameters
// Desc: Update the 'traceCongestionMultiplier' and 'viaCongestionMultiplier' elements
//       in structure mapInfo. These elements depend on the iteration number like so:
//
//       1. For iterations 1 through 20*log10(num_paths), the congestion multipliers are
//          unchanged from their default values of:
//          traceCongestionMultiplier = 0.20
//                 * (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter / 100.0)
//                 * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
//          viaCongestionMultiplier = 0.20
//                 * (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter / 100.0)
//                 * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
//
//       2. For iterations 20*log10(num_paths) + 1 through 100*log10(num_paths), the congestion
//          multipliers increase linearly as:
//          traceCongestionMultiplier = (current_iteration / 5.0 / 20*log10(num_paths))
//                 * (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter / 100.0)
//                 * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
//          viaCongestionMultiplier = (current_iteration / 5.0 / 20*log10(num_paths))
//                 * (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter / 100.0)
//                 * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
//
//       3. For iterations after 100*log10(num_paths), the congestion multipliers are not
//          changed by this function, retaining the following values. Note that these values
//          might be changed by other functions to optimize the routing:
//          traceCongestionMultiplier =
//                 * (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter / 100.0)
//                 * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
//          viaCongestionMultiplier =
//                 * (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter / 100.0)
//                 * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
//-----------------------------------------------------------------------------
void update_iterationDependent_parameters(MapInfo_t *mapInfo, RoutingMetrics_t *routability, FILE * fp);


//-----------------------------------------------------------------------------
// Name: determineBestIterations
// Desc: Determine the iteration with the best routing metrics. The best
//       iteration is the one with the lowest number of cells with DRCs. If
//       multiple iterations contain zero DRC cells, then the best iteration
//       is the DRC-free iteration with the lowest routing cost.
//
//       The first iteration will not be reported if the user has
//       defined cost-zones in the map. Such cost-zones increase the cost
//       of routing, but Acorn disregards these zones only for iteration
//       #1 in order to display a "rat's nest" routing.
//-----------------------------------------------------------------------------
void determineBestIterations(const MapInfo_t *mapInfo, RoutingMetrics_t *routability,
                             int cost_multipliers_used);


//-----------------------------------------------------------------------------
// Name: swap_start_and_end_terminals_of_DRC_paths
// Desc: Swap the start- and end-terminals of nets that have DRCs. The function
//       returns the number of nets whose terminals were swapped, or were
//       eligible to be swapped.
//
//       If the input parameter 'countOnly' is TRUE, then this function merely
//       counts the number of nets that are eligible for having their start-
//       and end-terminals swapped, without actually swapping the terminals.
//-----------------------------------------------------------------------------
int swap_start_and_end_terminals_of_DRC_paths(const int max_routed_nets, MapInfo_t *mapInfo,
                                              RoutingMetrics_t *routability, const InputValues_t *user_inputs,
                                              const int countOnly);


//-----------------------------------------------------------------------------
// Name: determineAlgorithmChanges
// Desc: Determine which one, if any, changes should be made to the routing
//       algorithm. The three possible changes are listed below, along with
//       the criteria for invoking them:
//
//       (1) Start or stop applying pseudo-TRACE congestion near pseudo-vias of
//           nets with DRCs on layer(s) that contain these DRCs
//           Criteria: (a) Number of routing layers > 1, and
//                     (b) Netlist contains pseudo-nets associated with diff-pair nets that
//                         have had DRCs for every one of the most recent 20 consecutive
//                         iterations, and
//                     (c) Acorn is currently in a metric plateau, and
//                     (d) The required number of DRC-free iterations has not
//                         been achieved, and
//                     (e) 0% of the most recent 20 iterations were DRC-free, and
//                     (f) No algorithm changes have been made for at least 60
//                         iterations, including the swapping of start-/end-terminals,
//                         or changing the congestion sensitivity.
//                     (g) Reducing the congestion sensitivity has been done at
//                         least 1 time.
//       (2) Change congestion sensitivity ('congestionMultiplier')
//           Criteria: (a) Acorn is currently in a metric plateau, and
//                     (b) The required number of DRC-free iterations has not
//                         been achieved, and
//                     (c) <=20% of the most recent 20 iterations were DRC-free, and
//                     (d) No algorithm changes have been made for at least 60
//                         iterations, including the swapping of start-/end-terminals,
//                         changing the congestion sensitivity, or starting to apply
//                         TRACE congestion near crowded pseudo-vias.
//                     (e) Swapping start/end-terminals has been done at least 3 times,
//                         or else zero nets with DRCs are currently eligible for swapping
//                         their start/end terminals. (Ineligible nets are those with
//                         terminals in pin-swap zones.)
//       (3) Swap start- and end-terminals of nets with DRCs
//           Criteria: (a) At least one net with DRCs is currently eligible for swapping
//                         its start/end terminals. (Ineligible nets are those with
//                         terminals in pin-swap zones.)
//                     (b) Acorn is currently in a metric plateau, and
//                     (c) The required number of DRC-free iterations has not
//                         been achieved, and
//                     (d) <=60% of the most recent 20 iterations were DRC-free, and
//                     (e) No algorithm changes have been made for at least 60
//                         iterations, including the swapping of start-/end-terminals,
//                         changing the congestion sensitivity, or starting to apply
//                         TRACE congestion near crowded pseudo-vias.
//
//       We assess which one (if any) changes to make in the following order, which is the
//       reverse order of their likelihood to occur:
//         (1) adding pseudo-TRACE congestion near pseudo-vias,
//         (2) changing congestion sensitivity, and
//         (3) swapping start/end-terminals.
//
//       This function modifies the following three elements of
//       structure RoutingMetrics_t:
//            swapStartAndEndTerms[iteration_num]
//            changeViaCongSensitivity[iteration_num]
//            changeTraceCongSensitivity[iteration_num]
//            enablePseudoTraceCongestion[iteration_num]
//-----------------------------------------------------------------------------
void determineAlgorithmChanges(MapInfo_t *mapInfo, int DRC_free_threshold,
                                 RoutingMetrics_t *routability, const InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: addTraceCongestionNearPseudoViasWithDRCs
// Desc: For diff-pair nets that have DRCs, add congestion near pseudo-vias
//       intended to repel pseudo-TRACE routing on routing layers for which the
//       diff-pair net contains DRCs. The location of the added TRACE congestion
//       is limited to a circle around the pseudo-via's center with a radius
//       of half a line-width of the the associated pseudo-path. The amount of
//       deposited congestion is ONE_TRAVERSAL * baseVertCostCells / cong_radius
//       (scaled by the user-defined cost-multipliers, if appropriate).
//-----------------------------------------------------------------------------
void addTraceCongestionNearPseudoViasWithDRCs(const MapInfo_t *mapInfo, const int pathLengths[],
                                              Coordinate_t *pathCoords[], CellInfo_t ***cellInfo,
                                              RoutingMetrics_t *routability, const InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: determineIfSolved
// Desc: Calculate whether a suitable routing solution is found. The following
//       criteria must be satisfied:
//         (1) The required number of DRC-free iterations has been achieved,
//
//        And any of the following:
//
//           (2a) The number of routed paths is only 1 (note that a diff-pair
//                constitutes a single routed path).
//        Or:
//           (2b) The value of routability->inMetricsPlateau must be TRUE for
//                the current iteration.
//        Or:
//           (2c) The current iteration is >=20 iterations beyond the iteration
//                when Acorn achieved the required number of DRC-free iterations.
//        Or:
//           (2d) The current iteration equals the maximum allowed number of
//                iterations.
//-----------------------------------------------------------------------------
int determineIfSolved(int iteration, int DRCfree_threshold, int num_routed_paths,
                      int maxIterations, RoutingMetrics_t *routability);




