#include "global_defs.h"
#include "aStarLibrary.h"


//-----------------------------------------------------------------------------
// Name: createRoutability
// Desc: Allocate memory for the routing metrics data structure. This function
//       does not initialize elements to known values.
//-----------------------------------------------------------------------------
void createRoutability(RoutingMetrics_t *routability, const MapInfo_t *mapInfo)  {

  int i;
  int max_routed_nets = mapInfo->numPaths + mapInfo->numPseudoPaths;

  // Allocate memory for 1-dimensional 'path_cost' array
  routability->path_cost = malloc(sizeof(unsigned long) * max_routed_nets);
  if (routability->path_cost == 0)  {
    printf("Error: Failed to allocate memory for 'path_cost' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'lateral_path_lengths_mm' array
  routability->lateral_path_lengths_mm = malloc(sizeof(float) * max_routed_nets);
  if (routability->lateral_path_lengths_mm == 0)  {
    printf("Error: Failed to allocate memory for 'lateral_path_lengths_mm' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'num_adjacent_steps' array
  routability->num_adjacent_steps = malloc(sizeof(int) * max_routed_nets);
  if (routability->num_adjacent_steps == 0)  {
    printf("Error: Failed to allocate memory for 'num_adjacent_steps' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'num_diagonal_steps' array
  routability->num_diagonal_steps = malloc(sizeof(int) * max_routed_nets);
  if (routability->num_diagonal_steps == 0)  {
    printf("Error: Failed to allocate memory for 'num_diagonal_steps' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'num_knights_steps' array
  routability->num_knights_steps = malloc(sizeof(int) * max_routed_nets);
  if (routability->num_knights_steps == 0)  {
    printf("Error: Failed to allocate memory for 'num_knights_steps' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'num_vias' array
  routability->num_vias = malloc(sizeof(int) * max_routed_nets);
  if (routability->num_vias == 0)  {
    printf("Error: Failed to allocate memory for 'num_vias' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'path_DRC_cells' array to hold DRC violations
  routability->path_DRC_cells = malloc(sizeof(int) * max_routed_nets);
  if (routability->path_DRC_cells == 0)  {
    printf("Error: Failed to allocate memory for 'path_DRC_cells' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'layer_DRC_cells' array to hold DRC violations
  routability->layer_DRC_cells = malloc(sizeof(int) * mapInfo->numLayers);
  if (routability->layer_DRC_cells == 0)  {
    printf("Error: Failed to allocate memory for 'layer_DRC_cells' array.\n");
    exit (1);
  }

  // Allocate memory for 2-dimensional matrix 'path_DRC_cells_by_layer':
  routability->path_DRC_cells_by_layer = malloc(sizeof(int *) * mapInfo->numPaths);
  if (routability->path_DRC_cells_by_layer == 0)  {
    printf("Error: Failed to allocate memory for 'path_DRC_cells_by_layer' array.\n");
    exit (1);
  }
  for (i = 0; i < mapInfo->numPaths; i++)  {
    routability->path_DRC_cells_by_layer[i] = malloc(sizeof(int) * mapInfo->numLayers);
    if (routability->path_DRC_cells_by_layer[i] == 0)  {
      printf("Error: Failed to allocate memory for 'path_DRC_cells_by_layer[%d]' array.\n", i);
      exit (1);
    }  // End of if-block
  }  // End of for-loop

  // Allocate memory for 2-dimensional matrix 'recent_DRC_flags_by_pseudoPath_layer':
  routability->recent_DRC_flags_by_pseudoPath_layer = malloc(sizeof(uint32_t *) * mapInfo->numPseudoPaths);
  if (routability->recent_DRC_flags_by_pseudoPath_layer == 0)  {
    printf("Error: Failed to allocate memory for 'recent_DRC_flags_by_pseudoPath_layer' array.\n");
    exit (1);
  }
  for (i = 0; i < mapInfo->numPseudoPaths; i++)  {
    routability->recent_DRC_flags_by_pseudoPath_layer[i] = malloc(sizeof(uint32_t) * mapInfo->numLayers);
    if (routability->recent_DRC_flags_by_pseudoPath_layer[i] == 0)  {
      printf("Error: Failed to allocate memory for 'recent_DRC_flags_by_pseudoPath_layer[%d]' array.\n", i);
      exit (1);
    }  // End of if-block
  }  // End of for-loop

  // Allocate memory for first dimension of 2-dimensional 'crossing matrix' to hold DRC violations:
  routability->crossing_matrix = malloc(sizeof(int *) * max_routed_nets);
  if (routability->crossing_matrix == 0)  {
    printf("Error: Failed to allocate memory for 'crossing_matrix' array.\n");
    exit (1);
  }

  // Allocate memory for first dimension of the 2-dimensional 'recent_path_DRC_cells' matrix:
  routability->recent_path_DRC_cells = malloc(sizeof(int *) * max_routed_nets);
  if (routability->recent_path_DRC_cells == 0)  {
    printf("Error: Failed to allocate memory for 'recent_path_DRC_cells' array.\n");
    exit (1);
  }

  // Allocate memory for the 2nd dimension of 'crossing_matrix' and 'recent_path_DRC_cells' arrays:
  for (i = 0; i < max_routed_nets; i++)  {
    routability->crossing_matrix[i] = malloc(sizeof(int) * max_routed_nets);
    // printf("DEBUG: routability->crossing_matrix[%d] is allocated for %d integer elements.\n", i, max_routed_nets);
    if (routability->crossing_matrix[i] == 0)  {
      printf("Error: Failed to allocate memory for 'crossing_matrix[%d]' array.\n", i);
      exit (1);
    }  // End of if-block

    routability->recent_path_DRC_cells[i] = malloc(sizeof(int) * numIterationsToReEquilibrate);
    // printf("DEBUG: routability->recent_path_DRC_cells[%d] is allocated for %d integer elements.\n", i, numIterationsToReEquilibrate);
    if (routability->recent_path_DRC_cells[i] == 0)  {
      printf("Error: Failed to allocate memory for 'recent_path_DRC_cells[%d]' array.\n", i);
      exit (1);
    }  // End of if-block

  }  // End of for-loop

  // Allocate memory for 1-dimensional 'recent_path_DRC_fraction' array:
  routability->recent_path_DRC_fraction = malloc(sizeof(float) * max_routed_nets);
  if (routability->recent_path_DRC_fraction == 0)  {
    printf("Error: Failed to allocate memory for 'recent_path_DRC_fraction' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'recent_path_DRC_iterations' array:
  routability->recent_path_DRC_iterations = malloc(sizeof(int) * max_routed_nets);
  if (routability->recent_path_DRC_iterations == 0)  {
    printf("Error: Failed to allocate memory for 'recent_path_DRC_iterations' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'fractionRecentIterationsWithoutPathDRCs' array:
  routability->fractionRecentIterationsWithoutPathDRCs = malloc(sizeof(float) * max_routed_nets);
  if (routability->fractionRecentIterationsWithoutPathDRCs == 0)  {
    printf("Error: Failed to allocate memory for 'fractionRecentIterationsWithoutPathDRCs' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'one_path_traversal' array:
  routability->one_path_traversal = malloc(sizeof(short) * max_routed_nets);
  if (routability->one_path_traversal == 0)  {
    printf("Error: Failed to allocate memory for 'one_path_traversal' array.\n");
    exit (1);
  }

  // Allocate memory for 1-dimensional 'randomize_congestion' array:
  routability->randomize_congestion = malloc(sizeof(char) * max_routed_nets);
  if (routability->randomize_congestion == 0)  {
    printf("Error: Failed to allocate memory for 'randomize_congestion' array.\n");
    exit (1);
  }

  // Allocate memory for the 1-dimensional 'path_elapsed_time' array:
  // printf("DEBUG: About to allocate path_elapsed_time for %d max_routed_nets.\n", max_routed_nets);
  routability->path_elapsed_time = malloc(sizeof(int) * max_routed_nets);
  if (routability->path_elapsed_time == 0)  {
    printf("Error: Failed to allocate memory for 'path_elapsed_time' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'iteration_elapsed_time' array
  routability->iteration_elapsed_time = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->iteration_elapsed_time == 0)  {
    printf("Error: Failed to allocate memory for 'iteration_elapsed_time' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for the 1-dimensional 'path_explored_cells' array:
  routability->path_explored_cells = malloc(sizeof(long) * max_routed_nets);
  if (routability->path_explored_cells == 0)  {
    printf("Error: Failed to allocate memory for 'path_explored_cells' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'iteration_explored_cells' array
  routability->iteration_explored_cells = malloc(sizeof(long) * (mapInfo->max_iterations + 1));
  if (routability->iteration_explored_cells == 0)  {
    printf("Error: Failed to allocate memory for 'iteration_explored_cells' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudoPathLengths' array
  routability->nonPseudoPathLengths = malloc(sizeof(float) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudoPathLengths == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudoPathLengths' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudo_num_DRC_cells' array
  routability->nonPseudo_num_DRC_cells = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudo_num_DRC_cells == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudo_num_DRC_cells' array.\n");
    exit (1);
  }  // End of if-block


  // Allocate memory for 1-dimensional 'nonPseudo_num_via2via_DRC_cells' array
  routability->nonPseudo_num_via2via_DRC_cells = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudo_num_via2via_DRC_cells == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudo_num_via2via_DRC_cells' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudo_num_trace2trace_DRC_cells' array
  routability->nonPseudo_num_trace2trace_DRC_cells = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudo_num_trace2trace_DRC_cells == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudo_num_trace2trace_DRC_cells' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudo_num_trace2via_DRC_cells' array
  routability->nonPseudo_num_trace2via_DRC_cells = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudo_num_trace2via_DRC_cells == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudo_num_trace2via_DRC_cells' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudoViaCounts' array
  routability->nonPseudoViaCounts = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudoViaCounts == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudoViaCounts' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudoPathCosts' array
  routability->nonPseudoPathCosts = malloc(sizeof(long) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudoPathCosts == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudoPathCosts' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'numNonPseudoDRCnets' array
  routability->numNonPseudoDRCnets = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->numNonPseudoDRCnets == 0)  {
    printf("Error: Failed to allocate memory for 'numNonPseudoDRCnets' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudoPathCosts_stdDev_trailing_10_iterations' array
  routability->nonPseudoPathCosts_stdDev_trailing_10_iterations = malloc(sizeof(double) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudoPathCosts_stdDev_trailing_10_iterations == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudoPathCosts_stdDev_trailing_10_iterations' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'nonPseudoPathCosts_slope_trailing_10_iterations' array
  routability->nonPseudoPathCosts_slope_trailing_10_iterations = malloc(sizeof(double) * (mapInfo->max_iterations + 1));
  if (routability->nonPseudoPathCosts_slope_trailing_10_iterations == 0)  {
    printf("Error: Failed to allocate memory for 'nonPseudoPathCosts_slope_trailing_10_iterations' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'inMetricsPlateau' array
  routability->inMetricsPlateau = malloc(sizeof(char) * (mapInfo->max_iterations + 1));
  if (routability->inMetricsPlateau == 0)  {
    printf("Error: Failed to allocate memory for 'inMetricsPlateau' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'swapStartAndEndTerms' array
  routability->swapStartAndEndTerms = malloc(sizeof(char) * (mapInfo->max_iterations + 1));
  if (routability->swapStartAndEndTerms == 0)  {
    printf("Error: Failed to allocate memory for 'swapStartAndEndTerms' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'changeViaCongSensitivity' array
  routability->changeViaCongSensitivity = malloc(sizeof(char) * (mapInfo->max_iterations + 1));
  if (routability->changeViaCongSensitivity == 0)  {
    printf("Error: Failed to allocate memory for 'changeViaCongSensitivity' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'changeTraceCongSensitivity' array
  routability->changeTraceCongSensitivity = malloc(sizeof(char) * (mapInfo->max_iterations + 1));
  if (routability->changeTraceCongSensitivity == 0)  {
    printf("Error: Failed to allocate memory for 'changeTraceCongSensitivity' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'enablePseudoTraceCongestion' array
  routability->enablePseudoTraceCongestion = malloc(sizeof(char) * (mapInfo->max_iterations + 1));
  if (routability->enablePseudoTraceCongestion == 0)  {
    printf("Error: Failed to allocate memory for 'enablePseudoTraceCongestion' array.\n");
    exit (1);
  }  // End of if-block

  // Allocate memory for 1-dimensional 'cumulative_DRCfree_iterations' array
  routability->cumulative_DRCfree_iterations = malloc(sizeof(int) * (mapInfo->max_iterations + 1));
  if (routability->cumulative_DRCfree_iterations == 0)  {
    printf("Error: Failed to allocate memory for 'cumulative_DRCfree_iterations' array.\n");
    exit (1);
  }  // End of if-block

}  // End of function 'createRoutability'


//-----------------------------------------------------------------------------
// Name: freeMemory_routability
// Desc: Free memory that was allocated in function 'initializeRoutability'.
//-----------------------------------------------------------------------------
void freeMemory_routability(RoutingMetrics_t *routability, const MapInfo_t *mapInfo)  {

  int i;
  int max_routed_nets = mapInfo->numPaths + mapInfo->numPseudoPaths;

  // Free memory for 1-dimensional 'path_cost' array
  free(routability->path_cost);                       routability->path_cost = NULL;

  // Free memory for 1-dimensional 'lateral_path_lengths_mm' array
  free(routability->lateral_path_lengths_mm);          routability->lateral_path_lengths_mm = NULL;

  // Free memory for 1-dimensional 'num_adjacent_steps' array
  free(routability->num_adjacent_steps);              routability->num_adjacent_steps = NULL;

  // Free memory for 1-dimensional 'num_diagonal_steps' array
  free(routability->num_diagonal_steps);              routability->num_diagonal_steps = NULL;

  // Free memory for 1-dimensional 'num_knights_steps' array
  free(routability->num_knights_steps);               routability->num_knights_steps = NULL;

  // Free memory for 1-dimensional 'num_vias' array
  free(routability->num_vias);                        routability->num_vias = NULL;

  // Free memory for 1-dimensional 'path_DRC_cells' array
  free(routability->path_DRC_cells);                  routability->path_DRC_cells = NULL;

  // Free memory for 1-dimensional 'layer_DRC_cells' array
  free(routability->layer_DRC_cells);                 routability->layer_DRC_cells = NULL;

  // Free memory for 2-dimensional 'path_DRC_cells_by_layer' matrix
  for (i = 0; i < mapInfo->numPaths; i++)  {
    free(routability->path_DRC_cells_by_layer[i]);    routability->path_DRC_cells_by_layer[i] = NULL;
  }
  free(routability->path_DRC_cells_by_layer);

  // Free memory for 2-dimensional 'recent_DRC_flags_by_pseudoPath_layer' matrix
  for (i = 0; i < mapInfo->numPseudoPaths; i++)  {
    free(routability->recent_DRC_flags_by_pseudoPath_layer[i]);
    routability->recent_DRC_flags_by_pseudoPath_layer[i] = NULL;
  }
  free(routability->recent_DRC_flags_by_pseudoPath_layer);

  // Free memory for 2-dimensional 'crossing matrix' and 'recent_path_DRC_cells' matrices:
  for (i = 0; i < max_routed_nets; i++)  {
    free(routability->crossing_matrix[i]);                   routability->crossing_matrix[i] = NULL;
    free(routability->recent_path_DRC_cells[i]);             routability->recent_path_DRC_cells[i] = NULL;
  }  // End of for-loop
  free(routability->crossing_matrix);                   routability->crossing_matrix = NULL;
  free(routability->recent_path_DRC_cells);             routability->recent_path_DRC_cells = NULL;

  // Free memory for the 1-dimensional 'path_elapsed_time' array:
  free(routability->path_elapsed_time);               routability->path_elapsed_time = NULL;

  // Free memory for 1-dimensional 'nonPseudoPathLengths' array
  free(routability->nonPseudoPathLengths);            routability->nonPseudoPathLengths = NULL;

  // Free memory for 1-dimensional 'nonPseudo_num_DRC_cells' array
  free(routability->nonPseudo_num_DRC_cells);         routability->nonPseudo_num_DRC_cells = NULL;

  // Free memory for 1-dimensional 'nonPseudo_num_via2via_DRC_cells' array
  free(routability->nonPseudo_num_via2via_DRC_cells); routability->nonPseudo_num_via2via_DRC_cells = NULL;

  // Free memory for 1-dimensional 'nonPseudo_num_trace2trace_DRC_cells' array
  free(routability->nonPseudo_num_trace2trace_DRC_cells); routability->nonPseudo_num_trace2trace_DRC_cells = NULL;

  // Free memory for 1-dimensional 'nonPseudo_num_trace2via_DRC_cells' array
  free(routability->nonPseudo_num_trace2via_DRC_cells); routability->nonPseudo_num_trace2via_DRC_cells = NULL;

  // Free memory for 1-dimensional 'nonPseudoViaCounts' array
  free(routability->nonPseudoViaCounts);              routability->nonPseudoViaCounts = NULL;

  // Free memory for 1-dimensional 'nonPseudoPathCosts' array
  free(routability->nonPseudoPathCosts);              routability->nonPseudoPathCosts = NULL;

  // Free memory for 1-dimensional 'numNonPseudoDRCnets' array
  free(routability->numNonPseudoDRCnets);             routability->numNonPseudoDRCnets = NULL;

  // Free memory for 1-dimensional 'nonPseudoPathCosts_stdDev_trailing_10_iterations' array
  free(routability->nonPseudoPathCosts_stdDev_trailing_10_iterations);
  routability->nonPseudoPathCosts_stdDev_trailing_10_iterations = NULL;

  // Free memory for 1-dimensional 'nonPseudoPathCosts_slope_trailing_10_iterations' array
  free(routability->nonPseudoPathCosts_slope_trailing_10_iterations);
  routability->nonPseudoPathCosts_slope_trailing_10_iterations = NULL;

  // Free memory for 1-dimensional 'inMetricsPlateau' array
  free(routability->inMetricsPlateau);                routability->inMetricsPlateau = NULL;

  // Free memory for 1-dimensional 'swapStartAndEndTerms' array
  free(routability->swapStartAndEndTerms);            routability->swapStartAndEndTerms = NULL;

  // Free memory for 1-dimensional 'changeViaCongSensitivity' array
  free(routability->changeViaCongSensitivity);        routability->changeViaCongSensitivity = NULL;

  // Free memory for 1-dimensional 'changeTraceCongSensitivity' array
  free(routability->changeTraceCongSensitivity);      routability->changeTraceCongSensitivity = NULL;

  // Free memory for 1-dimensional 'enablePseudoTraceCongestion' array
  free(routability->enablePseudoTraceCongestion);
  routability->enablePseudoTraceCongestion = NULL;

  // Free memory for 1-dimensional 'cumulative_DRCfree_iterations' array
  free(routability->cumulative_DRCfree_iterations);   routability->cumulative_DRCfree_iterations = NULL;

  // Free memory for 1-dimensional 'recent_path_DRC_fraction' array
  free(routability->recent_path_DRC_fraction);        routability->recent_path_DRC_fraction = NULL;

  // Free memory for 1-dimensional 'fractionRecentIterationsWithoutPathDRCs' array
  free(routability->fractionRecentIterationsWithoutPathDRCs);     routability->fractionRecentIterationsWithoutPathDRCs = NULL;

  // Free memory for 1-dimensional 'recent_path_DRC_iterations' array
  free(routability->recent_path_DRC_iterations);      routability->recent_path_DRC_iterations = NULL;

  // Free memory for 1-dimensional 'one_path_traversal' array
  free(routability->one_path_traversal);      routability->one_path_traversal = NULL;

  // Free memory for 1-dimensional 'randomize_congestion' array
  free(routability->randomize_congestion);      routability->randomize_congestion = NULL;

  // Free memory for 1-dimensional 'iteration_elapsed_time' array
  free(routability->iteration_elapsed_time);          routability->iteration_elapsed_time = NULL;

  // Free memory for the 1-dimensional 'path_explored_cells' array:
  free(routability->path_explored_cells);             routability->path_explored_cells = NULL;

  // Free memory for 1-dimensional 'iteration_explored_cells' array
  free(routability->iteration_explored_cells);        routability->iteration_explored_cells = NULL;

}  // End of function 'freeMemory_routability'


//-----------------------------------------------------------------------------
// Name: print_cell_congestion
// Desc: Print out the congestion at a given cell in the cellInfo matrix. This
//       function is intended only for debugging.
//-----------------------------------------------------------------------------
void print_cell_congestion(CellInfo_t *cellInfo)  {

  // Get the number of paths that traverse this cell:
  int num_paths = cellInfo->numTraversingPaths;

  printf("DEBUG: (thread %2d) ----------------------------------------------------------------\n", omp_get_thread_num());
  if (num_paths)  {
    printf("DEBUG: (thread %2d) %d paths traverse the cell:\n", omp_get_thread_num(), num_paths);

    // Iterate over the paths whose congestion traverses this cell:
    for (int path_index = 0; path_index < num_paths; path_index++)  {
      printf("DEBUG: (thread %2d)   %3d: path=%d, subset=%d, shapeType=%d, pathTraversalsTimes100=%'9d\n",
             omp_get_thread_num(), path_index, cellInfo->congestion[path_index].pathNum, cellInfo->congestion[path_index].DR_subset,
             cellInfo->congestion[path_index].shapeType, cellInfo->congestion[path_index].pathTraversalsTimes100);
    }  // End of for-loop for index 'path_index'
  }
  else  {
    printf("DEBUG: (thread %2d) No paths traverse the cell.\n", omp_get_thread_num());
  }
  printf("DEBUG: (thread %2d) ----------------------------------------------------------------\n", omp_get_thread_num());
}  // End of function 'print_cell_congestion'


//-----------------------------------------------------------------------------
// Name: addCongestionAroundAllTerminals
// Desc: Add congestion (in the 'cellInfo' 3D matrix) at each start- and end-terminal
//       of all non-pseudo-paths.
//-----------------------------------------------------------------------------
// Define 'DEBUG_addCongestion' and re-compile if you want verbose debugging
// of functions that add congestion
// #define DEBUG_addCongestion
#undef DEBUG_addCongestion

void addCongestionAroundAllTerminals(const InputValues_t *user_inputs, const MapInfo_t *mapInfo,
                                     CellInfo_t ***cellInfo, Coordinate_t *contigPathCoords[],
                                     int contiguousPathLength[])  {

  // Iterate over all non-pseudo-paths:
  for (int pathNum = 0; pathNum < mapInfo->numPaths; pathNum++)  {

    // Define variables to hold coordinates of terminal and one adjacent segment
    // in the same path:
    Coordinate_t terminal, adjacentSegment;

    // Get the coordinate of the starting terminal:
    terminal = copyCoordinates(mapInfo->start_cells[pathNum]);

    // Skip this terminal if it's located in a pin-swappable zone:
    if (! cellInfo[terminal.X][terminal.Y][terminal.Z].swap_zone)  {

      // Determine the shape-type of the starting terminal by checking whether the next segment
      // in the contiguous path is on the same layer (a TRACE), the layer above (VIA_UP), or
      // the layer below (VIA_DOWN).
      adjacentSegment = copyCoordinates(contigPathCoords[pathNum][1]);

      // Confirm that the adjacent segment does not have the same x/y/z coordinates as the
      // terminal, which would indicate a serious error in the software:
      if ((terminal.X == adjacentSegment.X) && (terminal.Y == adjacentSegment.Y) && (terminal.Z == adjacentSegment.Z))  {
        printf("\n\nERROR: In function 'addCongestionAroundAllTerminals', an unexpected problem was detected for path #%d ('%s').\n",
                pathNum, user_inputs->net_name[pathNum]);
        printf(    "       The starting terminal at (%d,%d,%d) has the same coordinates as the adjacent segment (%d,%d,%d)\n",
                terminal.X, terminal.Y, terminal.Z, adjacentSegment.X, adjacentSegment.Y, adjacentSegment.Z);
        printf(    "       of the same path. Please inform the software developer of this fatal error message.\n\n");
        exit(1);
      }  // End of if-block for fatal error condition

      // Add congestion around the starting terminal as TRACE and VIA shape-types:
      #ifdef DEBUG_addCongestion
      addCongestionAroundTerminal(pathNum, terminal, TRACE,    user_inputs, mapInfo, cellInfo, FALSE);
      addCongestionAroundTerminal(pathNum, terminal, VIA_UP,   user_inputs, mapInfo, cellInfo, FALSE);
      addCongestionAroundTerminal(pathNum, terminal, VIA_DOWN, user_inputs, mapInfo, cellInfo, FALSE);
      #else
      addCongestionAroundTerminal(pathNum, terminal, TRACE,    user_inputs, mapInfo, cellInfo);
      addCongestionAroundTerminal(pathNum, terminal, VIA_UP,   user_inputs, mapInfo, cellInfo);
      addCongestionAroundTerminal(pathNum, terminal, VIA_DOWN, user_inputs, mapInfo, cellInfo);
      #endif

    }  // End of if-block for (! swap_zone)

    //
    // Now repeat the above steps for the ending terminal. First, get the
    // coordinate of the ending terminal:
    terminal = copyCoordinates(mapInfo->end_cells[pathNum]);

    // Note that we don't need to check if the end-terminal is in a pin-swappable zone, because
    // only start-terminals are allowed to be in pin-swap zones:

    // Determine the shape-type of the ending terminal by checking whether the previous segment
    // in the contiguous path is on the same layer (a TRACE), the layer above (VIA_UP), or
    // the layer below (VIA_DOWN).
    adjacentSegment = copyCoordinates(contigPathCoords[pathNum][contiguousPathLength[pathNum] - 2]);

    // Confirm that the adjacent segment does not have the same x/y/z coordinates as the
    // terminal, which would indicate a serious error in the software:
    if ((terminal.X == adjacentSegment.X) && (terminal.Y == adjacentSegment.Y) && (terminal.Z == adjacentSegment.Z))  {
      printf("\n\nERROR: In function 'addCongestionAroundAllTerminals', an unexpected problem was detected for path #%d ('%s').\n",
              pathNum, user_inputs->net_name[pathNum]);
      printf(    "       The ending terminal at (%d,%d,%d) has the same coordinates as the adjacent segment (%d,%d,%d)\n",
              terminal.X, terminal.Y, terminal.Z, adjacentSegment.X, adjacentSegment.Y, adjacentSegment.Z);
      printf(    "       of the same path. Please inform the software developer of this fatal error message.\n\n");
      exit(1);
    }  // End of if-block for fatal error condition


    // Add congestion around the ending terminal as TRACE and VIA shape-types:
    #ifdef DEBUG_addCongestion
    addCongestionAroundTerminal(pathNum, terminal, TRACE,    user_inputs, mapInfo, cellInfo, FALSE);
    addCongestionAroundTerminal(pathNum, terminal, VIA_UP,   user_inputs, mapInfo, cellInfo, FALSE);
    addCongestionAroundTerminal(pathNum, terminal, VIA_DOWN, user_inputs, mapInfo, cellInfo, FALSE);
    #else
    addCongestionAroundTerminal(pathNum, terminal, TRACE,    user_inputs, mapInfo, cellInfo);
    addCongestionAroundTerminal(pathNum, terminal, VIA_UP,   user_inputs, mapInfo, cellInfo);
    addCongestionAroundTerminal(pathNum, terminal, VIA_DOWN, user_inputs, mapInfo, cellInfo);
    #endif

  }  // End of for-loop for index 'pathNum'
}  // End of function 'addCongestionAroundAllTerminals'


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
void update_iterationDependent_parameters(MapInfo_t *mapInfo, RoutingMetrics_t *routability, FILE * fp)  {

  int time_constant_iterations = max(1, (int)(20.0 * log10(mapInfo->numPaths)));

  if (mapInfo->current_iteration <= time_constant_iterations)  {

    mapInfo->iterationDependentRatio = 0.20;

    mapInfo->traceCongestionMultiplier = 0.20 * (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter / 100.0)
                                         * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
    mapInfo->viaCongestionMultiplier   = 0.20 * (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter / 100.0)
                                         * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;

    printf("\nINFO: traceCongestionMultiplier remains %0.8f in iteration %d, which is less than time_constant_iterations (%d).\n",
           mapInfo->traceCongestionMultiplier, mapInfo->current_iteration, time_constant_iterations);
    printf("INFO: viaCongestionMultiplier remains %0.8f in iteration %d, which is less than time_constant_iterations (%d).\n",
           mapInfo->viaCongestionMultiplier, mapInfo->current_iteration, time_constant_iterations);
    printf("INFO: iterationDependentRatio remains %0.2f in iteration %d, which is less than time_constant_iterations (%d).\n\n",
           mapInfo->iterationDependentRatio, mapInfo->current_iteration, time_constant_iterations);

  }
  else if (mapInfo->current_iteration <= 5 * time_constant_iterations)  {

    mapInfo->iterationDependentRatio = mapInfo->current_iteration / 5.0 / time_constant_iterations;

    mapInfo->traceCongestionMultiplier = mapInfo->current_iteration / 5.0 / time_constant_iterations
                                         * (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter / 100.0)
                                         * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
    mapInfo->viaCongestionMultiplier   = mapInfo->current_iteration / 5.0 / time_constant_iterations
                                         * (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter / 100.0)
                                         * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;

    printf("\nINFO: traceCongestionMultiplier increased to %0.8f in iteration %d, which is between one and 5 times the time_constant_iterations (%d).\n",
           mapInfo->traceCongestionMultiplier, mapInfo->current_iteration, time_constant_iterations);
    printf("INFO: viaCongestionMultiplier increased to %0.8f in iteration %d, which is between one and 5 times the time_constant_iterations (%d).\n",
           mapInfo->viaCongestionMultiplier, mapInfo->current_iteration, time_constant_iterations);
    printf("INFO: iterationDependentRatio increased to %0.2f in iteration %d, which is between one and 5 times the time_constant_iterations (%d).\n\n",
           mapInfo->iterationDependentRatio, mapInfo->current_iteration, time_constant_iterations);

    // Update the 'latestAlgorithmChange' parameter with the current iteration, which signals to other functions
    // that the congestion multiplier was changed during this iteration:
    routability->latestAlgorithmChange = mapInfo->current_iteration;

  }
  else  {

    mapInfo->iterationDependentRatio = 1.00;

    mapInfo->traceCongestionMultiplier = (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter / 100.0)
                                         * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
    mapInfo->viaCongestionMultiplier   = (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter / 100.0)
                                         * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;

    printf("\nINFO: traceCongestionMultiplier remains %0.8f in iteration %d, which is greater than five times the time_constant_iterations (%d).\n",
           mapInfo->traceCongestionMultiplier, mapInfo->current_iteration, time_constant_iterations);
    printf("INFO: viaCongestionMultiplier remains %0.8f in iteration %d, which is greater than five times the time_constant_iterations (%d).\n",
           mapInfo->viaCongestionMultiplier, mapInfo->current_iteration, time_constant_iterations);
    printf("INFO: iterationDependentRatio remains %0.2f in iteration %d, which is greater than five times the time_constant_iterations (%d).\n\n",
           mapInfo->iterationDependentRatio, mapInfo->current_iteration, time_constant_iterations);
  }

  if (mapInfo->current_iteration == time_constant_iterations)  {
    fprintf(fp, "  <UL><LI><FONT color=\"#00CC00\">Trace and Via Congestion Sensitivities will increase linearly from 20%% to 100%% until iteration %d.</FONT></UL>\n",
            5 * time_constant_iterations);
    printf("\nINFO: Trace and Via Congestion Sensitivities will increase linearly from 20%% to 100%% until iteration %d.\n\n", 5 * time_constant_iterations);
  }
  else if (mapInfo->current_iteration == 5 * time_constant_iterations)  {
    fprintf(fp, "  <UL><LI><FONT color=\"#00CC00\">Trace and Via Congestion Sensitivities have reached their nominal values (100%%).</FONT></UL>\n");
    printf("\nINFO: Trace and Via Congestion Sensitivities have reached their nominal values (100%%).\n\n");
  }

}  // End of function 'update_iterationDependent_parameters'


//-----------------------------------------------------------------------------
// Name: determineBestIteration
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
//
// Define 'DEBUG_postProcess' and re-compile if you want verbose debugging print-statements enabled:
//
// #define DEBUG_determineBestIteration 1
#undef DEBUG_determineBestIteration

void determineBestIteration(const MapInfo_t *mapInfo, RoutingMetrics_t *routability,
                            int cost_multipliers_used)  {

  #ifdef DEBUG_determineBestIteration
  printf("\nDEBUG: Entered function determineBestIteration in iteration %d.\n", mapInfo->current_iteration);
  #endif

  // Define the iteration that this function will begin searching for optimal
  // routing metrics. If the user defined cost-multipliers in the routing
  // map, then we start at iteration #2 because Acorn disregards such
  // multipliers during iteration #1:
  unsigned int starting_iteration;
  if (cost_multipliers_used && (mapInfo->current_iteration > 1))  {
    starting_iteration = 2;
  }
  else  {
    starting_iteration = 1;
  }

  unsigned int iteration_with_fewest_DRC_cells = starting_iteration;
  unsigned int DRCfree_iteration_with_lowest_cost = starting_iteration;
  unsigned int best_iteration = starting_iteration;

  unsigned int num_DRCfree_iterations = routability->cumulative_DRCfree_iterations[mapInfo->current_iteration];

  // Initialize minimum routing cost to the largest possible unsigned long value:
  unsigned long min_routing_cost = ULONG_MAX;

  // Initialize minimum number of DRC cells to the largest possible integer value:
  // unsigned int min_DRC_cells = routability->nonPseudo_num_DRC_cells[1];
  unsigned int min_DRC_cells = INT_MAX;

  //
  // Iterate over all the iterations to find the one with the lowest number of
  // DRC cells, or the DRC-free iteration with the lowest routing costs:
  //
  #ifdef DEBUG_determineBestIteration
  printf("DEBUG: Checking all iterations for fewest DRC cells or lower routing cost:\n");
  #endif
  for (int iteration = starting_iteration; iteration <= mapInfo->current_iteration; iteration++)  {
    #ifdef DEBUG_determineBestIteration
    printf("  DEBUG: Iteration #%d:\n", iteration);
    #endif
    if (num_DRCfree_iterations == 0)  {
      if (routability->nonPseudo_num_DRC_cells[iteration] < min_DRC_cells)  {

        #ifdef DEBUG_determineBestIteration
        printf("    DEBUG: Iteration #%d has the fewest number of DRC cells (%'d).\n", iteration,
               routability->nonPseudo_num_DRC_cells[iteration]);
        #endif

        iteration_with_fewest_DRC_cells = iteration;
        min_DRC_cells = routability->nonPseudo_num_DRC_cells[iteration];

      }  // End of if-block for nonPseudo_num_DRC_cells <= min_DRC_cells
    }  // End of if-block for num_DRCfree_iterations == 0

    else  {
      if (    (routability->nonPseudo_num_DRC_cells[iteration] == 0)
           && (routability->nonPseudoPathCosts[iteration] < min_routing_cost))  {

        #ifdef DEBUG_determineBestIteration
        printf("    DEBUG: Iteration #%d has a lower DRC-free cost (%'ld) than iteration %d (%'ld).\n", iteration,
               routability->nonPseudoPathCosts[iteration], DRCfree_iteration_with_lowest_cost, min_routing_cost);
        #endif

        DRCfree_iteration_with_lowest_cost = iteration;
        min_routing_cost = routability->nonPseudoPathCosts[iteration];

      }  // End of if-block for nonPseudoPathCosts <= min_routing_cost
      #ifdef DEBUG_determineBestIteration
      else  {
        printf("    DEBUG: Iteration #%d has DRCs (%d), or has a higher DRC-free cost (%'ld) than iteration %d (%'ld).\n", iteration,
                routability->nonPseudo_num_DRC_cells[iteration], routability->nonPseudoPathCosts[iteration],
                DRCfree_iteration_with_lowest_cost, min_routing_cost);
      }  // End of else-block for nonPseudoPathCosts > min_routing_cost
      #endif
    }  // End of else-block for num_DRCfree_iterations > 0
  }  // End of for-loop for index 'iteration'


  //
  // Depending on whether any DRC-free iterations have been found, record the
  // 'best_iteration':
  //
  if (num_DRCfree_iterations == 0)  {
    best_iteration = iteration_with_fewest_DRC_cells;
    printf("INFO: After %d iteration(s), iteration %d has the best routing metrics because it has the fewest DRC cells (%'d).\n",
           mapInfo->current_iteration, iteration_with_fewest_DRC_cells, min_DRC_cells);
  }
  else  {
    best_iteration = DRCfree_iteration_with_lowest_cost;
    printf("INFO: After %d iteration(s), iteration %d has the best routing metrics because it has the lowest cost (%'lu) of all DRC-free iterations.\n",
           mapInfo->current_iteration, DRCfree_iteration_with_lowest_cost, min_routing_cost);
  }

  // Store the iteration with the best routing metrics into the 'routability' structure:
  routability->best_iteration = best_iteration;

}  // End of function 'determineBestIteration


//-----------------------------------------------------------------------------
// Name: swap_start_and_end_terminals_of_DRC_paths
// Desc: Swap the start- and end-terminals of nets that have DRCs. The function
//       returns the number of nets whose terminals were swapped.
//
//       If the input parameter 'countOnly' is TRUE, then this function merely
//       counts the number of nets that are eligible for having their start-
//       and end-terminals swapped, without actually swapping the terminals.
//-----------------------------------------------------------------------------
int swap_start_and_end_terminals_of_DRC_paths(const int max_routed_nets, MapInfo_t *mapInfo,
                                              RoutingMetrics_t *routability, const InputValues_t *user_inputs,
                                              const int countOnly)  {

  int num_nonPseudo_terminals_swapped = 0;  // Number of non-pseudo paths whose terminals were swapped

  // First, create and initialize an array with one Boolean element per path:
  char swap_path[max_routed_nets];
  for (int path = 0; path < max_routed_nets; path++)
    swap_path[path] = FALSE;

  // Second, cycle through each path to determine whether its terminals should be swapped:
  for (int path = 0; path < max_routed_nets; path++)  {

    // Skip paths whose start-terminals are in a pin-swap zone:
    if (mapInfo->swapZone[path])
      continue;

    // Skip pseudo-paths:
    if (user_inputs->isPseudoNet[path])
      continue;

    // Check whether path contains DRCs for majority of recent iterations:
    if (routability->fractionRecentIterationsWithoutPathDRCs[path] < 0.5)  {
      swap_path[path] = TRUE;  // Flag current net for terminal-swapping
      if (user_inputs->isDiffPair[path])  {
        swap_path[user_inputs->diffPairPartner[path]] = TRUE;  // Flag diff-pair's partner net
        swap_path[user_inputs->diffPairToPseudoNetMap[path]] = TRUE;  // Flag diff-pair's 'parent' pseudo-net
        printf("DEBUG: Flagging pseudo-path %d and diff-pair paths %d and %d for start/end terminal-swapping because diff-pair path %d has %.3f%% recent iterations without DRCs.\n",
                user_inputs->diffPairToPseudoNetMap[path], path, user_inputs->diffPairPartner[path], path, 100 * routability->fractionRecentIterationsWithoutPathDRCs[path]);
      }  // End of if block for flagging diff-pair nets and their associated nets
      else  {
        printf("DEBUG: Flagging standard path %d for start/end terminal-swapping because it has %.3f%% recent iterations without DRCs\n",
                path, 100 * routability->fractionRecentIterationsWithoutPathDRCs[path]);
      }
    }  // End of else/if-block for flagging non-pseudo nets.
  }  // End of for-loop for index 'path' for flagging nets to have their terminals swapped

  // Thirdly, swap the terminals of the nets that were flagged because of DRCs if
  // 'countOnly' is FALSE:
  if (! countOnly)  {
    printf("\nINFO: Swapping the start- and end-terminals for the following nets to improve routing:\n");
  }
  for (int path = 0; path < max_routed_nets; path++)  {
    // Check whether the path has terminals in a swap-zone. If so, skip this path:
    if (swap_path[path])  {

      if (! countOnly)  {
        swapStartAndEndTerminals(path, mapInfo);
        printf("INFO:   Net #%d ('%s')\n", path, user_inputs->net_name[path]);
      }

      // Count how many user-defined paths were flagged to be swapped
      if (!user_inputs->isPseudoNet[path])
        num_nonPseudo_terminals_swapped++;
    }  // End of if-block for path being flagged for terminal-swapping
  }  // End of for-loop for index 'path'

  // Return the number of nets whose terminals were swapped:
  return(num_nonPseudo_terminals_swapped);

}  // End of function 'swap_start_and_end_terminals_of_DRC_paths'


//-----------------------------------------------------------------------------
// Name: compareRoutingMetrics
// Desc: Compare the routing metrics associated with two dynamic parameter
//       indices, and return -1 (WORSE), 0 (EQUIVALENT), or +1 (BETTER). The
//       routing metrics are compared using the following rules:
//        (a) First, compare the fractions of DRC-free iterations. If
//            this metric is higher for the first index by 0.05, then return
//            return BETTER. If it's lower by 0.05, then return WORSE. If
//            the two fractions are within 0.05 of each other, then proceed
//            to step (b) below.
//        (b) Compare the number of non-pseudo nets that have DRCs. If this
//            metric is lower for the first index, then return BETTER. If it's
//            higher, then return WORSE. If the two numbers are within a standard
//            error in the mean of each other, then proceed to step (c) below.
//        (c) Compare the routing cost. If this metric is lower for the first
//            index, then return BETTER. If it's higher, then return WORSE. If
//            the costs are equivalent (within a standard error in their means),
//            then return EQUIVALENT.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_compareRoutingMetrics' and re-compile if you want verbose debugging print-statements enabled:
//
#define DEBUG_compareRoutingMetrics 1
/// #undef DEBUG_compareRoutingMetrics

static int compareRoutingMetrics(const unsigned char dynamicParameter_1,
                                 const unsigned char dynamicParameter_2,
                                 const DynamicAlgorithmMetrics_t routingMetrics[])  {

  // Check for the error-condition in which the routing metrics have not yet
  // been measured for one or both of the dynamic parameter indices:
  int undefined_inputs = FALSE;
  if (routingMetrics[dynamicParameter_1].iterationOfMeasuredMetrics == 0)  {
    printf("\nERROR: Function 'compareRoutingMetrics()' was called to compare the routing metrics for states\n");
    printf(  "       with dynamic values %d and %d. However the routing metrics have not yet been measured for value %d.\n",
            routingMetrics[dynamicParameter_1].dynamicParameter,
            routingMetrics[dynamicParameter_2].dynamicParameter,
            routingMetrics[dynamicParameter_1].dynamicParameter);
    undefined_inputs = TRUE;
  }  // End of if-block for undefined metrics for index #1
  if (routingMetrics[dynamicParameter_2].iterationOfMeasuredMetrics == 0)  {
    printf("\nERROR: Function 'compareRoutingMetrics()' was called to compare the routing metrics for states\n");
    printf(  "       with dynamic values %d and %d. However the routing metrics have not yet been measured for value %d.\n",
           routingMetrics[dynamicParameter_1].dynamicParameter,
           routingMetrics[dynamicParameter_2].dynamicParameter,
           routingMetrics[dynamicParameter_2].dynamicParameter);
    undefined_inputs = TRUE;
  }  // End of if-block for undefined metrics for index #2
  if (undefined_inputs)  {
    printf("\n       Inform the software developer of this fatal error message.\n\n");
    exit(1);
  }  // End of if-block for undefined input values

  // Calculate the uncertainty in the routing costs for the two parameters being compared. For each,
  // the uncertainty is the greater of (5% of the mean) and (the standard error in the mean).
  float cost_uncertainty_1 = max( 0.05 * routingMetrics[dynamicParameter_1].avgNonPseudoRoutingCost,
                                  routingMetrics[dynamicParameter_1].stdErrNonPseudoRoutingCost );
  float cost_uncertainty_2 = max( 0.05 * routingMetrics[dynamicParameter_2].avgNonPseudoRoutingCost,
                                  routingMetrics[dynamicParameter_2].stdErrNonPseudoRoutingCost );

  #ifdef DEBUG_compareRoutingMetrics
  printf("\nDEBUG: Function compareRoutingMetrics() was called to compare the routing metrics for states with dynamic indices\n");
  printf(  "DEBUG: '%d' and '%d', with dynamic values of '%d' and '%d', respectively.\n", dynamicParameter_1, dynamicParameter_2,
         routingMetrics[dynamicParameter_1].dynamicParameter, routingMetrics[dynamicParameter_2].dynamicParameter);
  printf("DEBUG:\n");
  printf("DEBUG:                 Dynamic Parameter:                  %4d                                 %4d\n",
         routingMetrics[dynamicParameter_1].dynamicParameter, routingMetrics[dynamicParameter_2].dynamicParameter);
  printf("DEBUG:    -------------------------------      ---------------------------           ---------------------------\n");
  printf("DEBUG:     fractionIterationsWithoutDRCs:               %6.3f                               %6.3f\n",
         routingMetrics[dynamicParameter_1].fractionIterationsWithoutDRCs,
         routingMetrics[dynamicParameter_2].fractionIterationsWithoutDRCs);
  printf("DEBUG:          avgNonPseudoNetsWithDRCs:               %6.3f                               %6.3f\n",
         routingMetrics[dynamicParameter_1].avgNonPseudoNetsWithDRCs,
         routingMetrics[dynamicParameter_2].avgNonPseudoNetsWithDRCs);
  printf("DEBUG:       stdErrNonPseudoNetsWithDRCs:               %6.3f                               %6.3f\n",
         routingMetrics[dynamicParameter_1].stdErrNonPseudoNetsWithDRCs,
         routingMetrics[dynamicParameter_2].stdErrNonPseudoNetsWithDRCs);
  printf("DEBUG:           avgNonPseudoRoutingCost:     %'26.3f             %'26.3f\n",
         routingMetrics[dynamicParameter_1].avgNonPseudoRoutingCost,
         routingMetrics[dynamicParameter_2].avgNonPseudoRoutingCost);
  printf("DEBUG:        stdErrNonPseudoRoutingCost:     %'26.3f             %'26.3f\n",
         routingMetrics[dynamicParameter_1].stdErrNonPseudoRoutingCost,
         routingMetrics[dynamicParameter_2].stdErrNonPseudoRoutingCost);
  printf("DEBUG:                  cost_uncertainty:     %'26.3f             %'26.3f\n",
         cost_uncertainty_1, cost_uncertainty_2);
  printf("DEBUG:\n");
  #endif


  //
  // Define variable that will be returned from this function. Allowed values
  // are 0 (EQUIVALENT), -1 (WORSE), and +1 (BETTER):
  //
  int comparison_result = EQUIVALENT;

  if (  routingMetrics[dynamicParameter_1].fractionIterationsWithoutDRCs
      - routingMetrics[dynamicParameter_2].fractionIterationsWithoutDRCs >= 0.05)  {

    // We got here, so the fraction of DRC-free iterations is more than 0.05 greater (better)
    // for the first index than the second. So we return 'BETTER':
    comparison_result = BETTER;

  }  // End of if-block for index_1 having better fraction of iterations without DRCs
  else if (  routingMetrics[dynamicParameter_1].fractionIterationsWithoutDRCs
           - routingMetrics[dynamicParameter_2].fractionIterationsWithoutDRCs <= -0.05)  {

    // We got here, so the fraction of DRC-free iterations is more than 0.05 lower (worse)
    // for the first index than the second. So we return 'WORSE':

    comparison_result = WORSE;

  }  // End of if-block for index_1 having worse fraction of iterations without DRCs
  else  {

    // We got here, so the fractions of DRC-free iterations are within 0.05 of each
    // other for the two indices. So we proceed to comparing the number of non-pseudo
    // nets that have DRCs.

    // First, define the average of the two values we're comparing:
    float average = 0.5 * (  routingMetrics[dynamicParameter_1].avgNonPseudoNetsWithDRCs
                           + routingMetrics[dynamicParameter_2].avgNonPseudoNetsWithDRCs);

    #ifdef DEBUG_compareRoutingMetrics
    printf("\nDEBUG: In function compareRoutingMetrics(), 'fractionIterationsWithoutDRCs' are EQUIVALENT.\n");
    printf("\nDEBUG: The average of the two avgNonPseudoNetsWithDRCs values is %6.3f\n", average);
    #endif


    // If the average is zero, it means that the two (positive) values are also zero, and therefore equal
    // to each other. This situation would require that we proceed to comparing the routing costs.
    if (   (average > 0.00001)
        && (   routingMetrics[dynamicParameter_1].avgNonPseudoNetsWithDRCs + max(0.5, routingMetrics[dynamicParameter_1].stdErrNonPseudoNetsWithDRCs)
             < routingMetrics[dynamicParameter_2].avgNonPseudoNetsWithDRCs - max(0.5, routingMetrics[dynamicParameter_2].stdErrNonPseudoNetsWithDRCs) ))  {


      // We got here, so index #1 is associated with fewer nets having DRC violations,
      // which is better.
      comparison_result = BETTER;
    }
    else if (   (average > 0.00001)
             && (   routingMetrics[dynamicParameter_1].avgNonPseudoNetsWithDRCs - max(0.5, routingMetrics[dynamicParameter_1].stdErrNonPseudoNetsWithDRCs)
                  > routingMetrics[dynamicParameter_2].avgNonPseudoNetsWithDRCs + max(0.5, routingMetrics[dynamicParameter_2].stdErrNonPseudoNetsWithDRCs) ))  {

      // We got here, so index #1 is associated with more nets having DRC violations,
      // which is worse.
      comparison_result = WORSE;
    }
    else  {
      // We got here, so the number of nets with DRCs are within the standard errors of
      // each other for the two indices. So we we proceed to comparing the routing costs.

      #ifdef DEBUG_compareRoutingMetrics
      printf("\nDEBUG: In function compareRoutingMetrics(), 'avgNonPseudoNetsWithDRCs' are EQUIVALENT.\n");
      #endif

      if (   routingMetrics[dynamicParameter_1].avgNonPseudoRoutingCost + cost_uncertainty_1
           < routingMetrics[dynamicParameter_2].avgNonPseudoRoutingCost - cost_uncertainty_2 )  {

        // We got here, so index #1 is associated with a lower routing cost,
        // which is better.
        comparison_result = BETTER;
      }

      else if (   routingMetrics[dynamicParameter_1].avgNonPseudoRoutingCost - cost_uncertainty_1
                > routingMetrics[dynamicParameter_2].avgNonPseudoRoutingCost + cost_uncertainty_2  )  {

        // We got here, so index #1 is associated with a higher routing cost,
        // which is worse.
        comparison_result = WORSE;
      }
      else  {
        // We got here, so indices #1 and #2 are associated with routing costs that are
        // statistically equivalent. We issue a WARNING message and return the
        // value 'EQUIVALENT', indicating that both indices are associated with
        // equivalent routing metrics.

        comparison_result = EQUIVALENT;
      }  // End of else-block for (exactly) equal values of the routing costs

    }  // End of else-block for (approximately) equal number of nets with DRCs.

  }  // End of else-block for both indices having equivalent fractions of iterations without DRCs

  // Check if the comparison result is 'EQUIVALENT'. If so, then issue a warning message:
  if (comparison_result == EQUIVALENT)  {
    // We got here, so indices #1 and #2 are associated with equivalent routing metrics.
    // We issue a WARNING message that lists the routing metrics from each index:
    printf("\nWARNING: Function 'compareRoutingMetrics()' concluded that the routing metrics for dynamic values\n");
    printf(  "         %d and %d are statistically equivalent. The metrics are:\n",
            routingMetrics[dynamicParameter_1].dynamicParameter, routingMetrics[dynamicParameter_2].dynamicParameter);
    printf(  "           %d%% rate: Fraction iterations w/o DRCs=%.3f, Avg nets w/ DRCs=%.3f +/- %.3f, Avg routing cost = %.2E +/- %.2E\n",
            routingMetrics[dynamicParameter_1].dynamicParameter,
            routingMetrics[dynamicParameter_1].fractionIterationsWithoutDRCs,
            routingMetrics[dynamicParameter_1].avgNonPseudoNetsWithDRCs,
            routingMetrics[dynamicParameter_1].stdErrNonPseudoNetsWithDRCs,
            routingMetrics[dynamicParameter_1].avgNonPseudoRoutingCost,
            cost_uncertainty_1);
    printf(  "           %d%% rate: Fraction iterations w/o DRCs=%.3f, Avg nets w/ DRCs=%.3f +/- %.3f, Avg routing cost = %.2E +/- %.2E\n\n",
            routingMetrics[dynamicParameter_2].dynamicParameter,
            routingMetrics[dynamicParameter_2].fractionIterationsWithoutDRCs,
            routingMetrics[dynamicParameter_2].avgNonPseudoNetsWithDRCs,
            routingMetrics[dynamicParameter_2].stdErrNonPseudoNetsWithDRCs,
            routingMetrics[dynamicParameter_2].avgNonPseudoRoutingCost,
            cost_uncertainty_2);
  }  // End of if-block for comparison_result == EQUIVALENT


  #ifdef DEBUG_compareRoutingMetrics
  printf("\nDEBUG: At end of function compareRoutingMetrics(), value of '%d' was returned ", comparison_result);
  if (comparison_result == WORSE)  {
    printf("(WORSE)\n");
  }
  else if (comparison_result == BETTER)  {
    printf("(BETTER)\n");
  }
  else {
    printf("(EQUIVALENT)\n");
  }
  printf("\n");
  #endif

  // Return the result of the comparison:
  return(comparison_result);

}  // End of function 'compareRoutingMetrics'


//-----------------------------------------------------------------------------
// Name: assessCongestionSensitivities
// Desc: Determine whether to increase, decrease, or maintain the same congestion
//       sensitivity based on routing metrics and other parameters associated with
//       the congestion sensitivities.
//-----------------------------------------------------------------------------
//
// Define 'DEBUG_assessCongestionSensitivities' and re-compile if you want verbose
// debugging print-statements enabled:
//
#define DEBUG_assessCongestionSensitivities 1
// #undef DEBUG_assessCongestionSensitivities

static void assessCongestionSensitivities(DynamicAlgorithmMetrics_t congSensitivityMetrics[], unsigned char *change_algorithm_during_this_iteration,
                                          unsigned char *changeCongSensitivity, unsigned short *num_congSensitivity_changes,
                                          unsigned short *num_congSensitivity_stableRoutingMetrics,
                                          unsigned short *num_congSensitivity_reductions, unsigned char currentCongSensIndex,
                                          int current_iteration, unsigned char inMetricsPlateau)  {

  // Determine whether to increase the sensitivity, decrease the sensitivity, or keep it the
  // same if it's the optimum congestion sensitivity. This decision depends on six unique variables:
  //   (a) current congestion sensitivity
  //   (b) whether the routing metrics are known for the next lower congestion sensitivity
  //   (c) whether the routing metrics are known for the next higher congestion sensitivity
  //   (d) current sensitivity's routing metrics compared to the next lower sensitivity's
  //   (e) current sensitivity's routing metrics compared to the next higher sensitivity's
  //   (f) next higher sensitivity's routing metrics compared to the next lower sensitivity's
  //
  // Items (d), (e), and (f) are tri-state variables: BETTER, WORSE, or EQUIVALENT.
  // Item (a) essentially takes 1 of 3 values: 100% (lowest), 500% (highest), or
  // an intermediate rate (150% to 450%). The truth-table contains 26 unique conditions.
  // In this table,
  //   "BET" means that current congestion sensitivity's metrics are better
  //   "WOR" means that current congestion sensitivity's metrics are worse
  //   "EQU" means that current congestion sensitivity's metrics are equivalent
  //     "x" means 'not applicable', i.e., the input value doesn't exist or doesn't matter
  //      ** means that the state is flagged as achieving a congestion sensitivity with
  //         stable routing metrics, relative to higher (and sometimes lower) sensitivities.
  //
  // No.  (a)           (b)  (c)  (d)  (e)  (f)   Action(s)                Comment
  // ---  ---           ---  ---  ---  ---  ---  -----------------------  -----------------------
  //  1   Lowest         x    No   x    x    x   Increase sensitivity     Re-measure at higher sensitivity
  //  2   Lowest         x   Yes   x   WOR   x   Increase sensitivity
  //  3   Lowest         x   Yes   x   BET   x   No change
  //  4   Lowest         x   Yes   x   EQU   x   Increase sensitivity **  Bias towards higher sensitivity
  //  5   Intermediate   No   No   x    x    x   Increase sensitivity     Bias towards higher sensitivity
  //  6   Intermediate   No  Yes   x   WOR   x   Increase sensitivity
  //  7   Intermediate   No  Yes   x   BET   x   Decrease sensitivity
  //  8   Intermediate   No  Yes   x   EQU   x   Increase sensitivity **  Bias towards higher sensitivity
  //  9   Intermediate  Yes   No  WOR   x    x   Decrease sensitivity
  // 10   Intermediate  Yes   No  BET   x    x   Increase sensitivity
  // 11   Intermediate  Yes   No  EQU   x    x   Increase sensitivity **  Bias towards higher sensitivity
  // 12   Intermediate  Yes  Yes  WOR  WOR  WOR  Decrease sensitivity
  // 13   Intermediate  Yes  Yes  WOR  WOR  BET  Increase sensitivity
  // 14   Intermediate  Yes  Yes  WOR  WOR  EQU  Increase sensitivity     Bias towards higher sensitivity
  // 15   Intermediate  Yes  Yes  WOR  BET   x   Decrease sensitivity
  // 16   Intermediate  Yes  Yes  WOR  EQU   x   Increase sensitivity **  Bias towards higher sensitivity
  // 17   Intermediate  Yes  Yes  BET  WOR   x   Increase sensitivity
  // 18   Intermediate  Yes  Yes  BET  BET   x   No change            **  At local maximum for metrics
  // 19   Intermediate  Yes  Yes  BET  EQU   x   Increase sensitivity **  Bias towards higher sensitivity
  // 20   Intermediate  Yes  Yes  EQU  WOR   x   Increase sensitivity
  // 21   Intermediate  Yes  Yes  EQU  BET   x   No change            **  Bias towards higher sensitivity
  // 22   Intermediate  Yes  Yes  EQU  EQU   x   Increase sensitivity **  Bias towards higher sensitivity
  // 23   Highest        No   x    x    x    x   Decrease sensitivity     Re-measure at lower sensitivity
  // 24   Highest       Yes   x   WOR   x    x   Decrease sensitivity
  // 25   Highest       Yes   x   BET   x    x   No change            **
  // 26   Highest       Yes   x   EQU   x    x   No change            **  Bias towards higher sensitivity
  //

  // First handle conditions #1 through #4:
  if (currentCongSensIndex == 0) {
    if (congSensitivityMetrics[1].iterationOfMeasuredMetrics == 0)  {
      // Condition #1 (Routing metrics are unknown for 141% congestion sensitivity)
      *change_algorithm_during_this_iteration = TRUE;
      *changeCongSensitivity = INCREASE;
      (*num_congSensitivity_changes)++;
      #ifdef DEBUG_assessCongestionSensitivities
      printf("DEBUG: Condition #1 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
             current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
      #endif
    }  // End of if-block for condition #1
    else  {  // Routing metrics are known for 141% congestion sensitivity:
      if (compareRoutingMetrics(0, 1, congSensitivityMetrics) == WORSE)  {
        // Condition #2 (100% metrics are worse than 150% metrics)
        *change_algorithm_during_this_iteration = TRUE;
        *changeCongSensitivity = INCREASE;
        (*num_congSensitivity_changes)++;
        #ifdef DEBUG_assessCongestionSensitivities
        printf("DEBUG: Condition #2 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
               current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
        #endif
      }  // End of if-block for condition #2
      else if (compareRoutingMetrics(0, 1, congSensitivityMetrics) == BETTER)  {
        // Condition #3 (100% metrics are better than 150% metrics)
        *changeCongSensitivity = NO_CHANGE;
        #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #3 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be UNCHANGED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
        #endif
      }  // End of else/if-block for condition #3
      else  {
        // Condition #4 (100% metrics are equivalent to 150% metrics)
        *change_algorithm_during_this_iteration = TRUE;
        *changeCongSensitivity = INCREASE;
        (*num_congSensitivity_changes)++;
        (*num_congSensitivity_stableRoutingMetrics)++;
        #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #4 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
        #endif
      }  // End of else-block for condition #4
    }  // End of else-block for conditions #2, 3, 4
  }  // End of if-block for conditions #1 through 4

  // Next, handle conditions #5 through #22:
  else if (currentCongSensIndex < NUM_CONG_SENSITIVITES - 1)  {
    if (congSensitivityMetrics[currentCongSensIndex - 1].iterationOfMeasuredMetrics == 0)  {
      // Conditions #5 through #8 (Routing metrics are unknown for next lower congestion sensitivity)
      if (congSensitivityMetrics[currentCongSensIndex + 1].iterationOfMeasuredMetrics == 0)  {
        // Condition #5 (Routing metrics are unknown for next higher congestion sensitivity)
        *change_algorithm_during_this_iteration = TRUE;
        *changeCongSensitivity = INCREASE;
        (*num_congSensitivity_changes)++;
        #ifdef DEBUG_assessCongestionSensitivities
        printf("DEBUG: Condition #5 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
               current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
        #endif
      }  // End of if-block for condition #5
      else  {
        // Conditions #6 through #8 (Routing metrics are known for next higher congestion sensitivity)
        int comparisonToHigherRate = compareRoutingMetrics(currentCongSensIndex, currentCongSensIndex + 1, congSensitivityMetrics);
        if (comparisonToHigherRate == WORSE)  {
          // Condition #6 (Routing metrics of current rate are worse than higher rate)
          *change_algorithm_during_this_iteration = TRUE;
          *changeCongSensitivity = INCREASE;
          (*num_congSensitivity_changes)++;
          #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #6 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
          #endif
        }  // End of if-block for condition #6
        else if (comparisonToHigherRate == BETTER)  {
          // Condition #7 (Routing metrics of current rate are better than higher congestion sensitivity)
          *change_algorithm_during_this_iteration = TRUE;
          *changeCongSensitivity = DECREASE;
          (*num_congSensitivity_changes)++;
          (*num_congSensitivity_reductions)++;
          #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #7 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be DECREASED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
          #endif
        }  // End of if-block for condition #7
        else  {
          // Condition #8 (Routing metrics of current rate are equivalent to higher congestion sensitivity)
          *change_algorithm_during_this_iteration = TRUE;
          *changeCongSensitivity = INCREASE;
          (*num_congSensitivity_changes)++;
          (*num_congSensitivity_stableRoutingMetrics)++;
          #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #8 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
          #endif
        }  // End of else-block for condition #8
      }  // End of else-block for conditions #6 through #8
    }  // End of if-block for conditions #5 through #8
    else  {
      // Conditions #9 through #22 (Routing metrics are known for next lower congestion sensitivity)
      if (congSensitivityMetrics[currentCongSensIndex + 1].iterationOfMeasuredMetrics == 0)  {
        // Conditions #9 through #11 (Routing metrics are unknown for next higher congestion sensitivity)
        int comparisonToLowerRate = compareRoutingMetrics(currentCongSensIndex, currentCongSensIndex - 1, congSensitivityMetrics);
        if (comparisonToLowerRate == BETTER)  {
          // Condition #10 (Routing metrics of current rate are better than lower congestion sensitivity)
          *change_algorithm_during_this_iteration = TRUE;
          *changeCongSensitivity = INCREASE;
          (*num_congSensitivity_changes)++;
          #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #10 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
          #endif
        }  // End of if-block for condition #10
        else if (comparisonToLowerRate == EQUIVALENT)  {
          // Condition 11 (Routing metrics of current rate are equivalent to the lower congestion sensitivity)
          *change_algorithm_during_this_iteration = TRUE;
          *changeCongSensitivity = INCREASE;
          (*num_congSensitivity_changes)++;
          (*num_congSensitivity_stableRoutingMetrics)++;
          #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #11 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
          #endif
        }  // End of if-block for condition #11
        else  {
          // Condition #9 (Routing metrics of current congestion sensitivity are worse than lower congestion sensitivity)
          *change_algorithm_during_this_iteration = TRUE;
          *changeCongSensitivity = DECREASE;
          (*num_congSensitivity_changes)++;
          (*num_congSensitivity_reductions)++;
          #ifdef DEBUG_assessCongestionSensitivities
          printf("DEBUG: Condition #9 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be DECREASED from %d%%.\n",
                 current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
          #endif
        }  // End of else-block for conditions #9
      }  // End of if-block for conditions #9 through #11
      else  {
        // Conditions #12 through #22 (Routing metrics are known for next higher congestion sensitivity)
        int comparisonToLowerRate = compareRoutingMetrics(currentCongSensIndex, currentCongSensIndex - 1, congSensitivityMetrics);
        if (comparisonToLowerRate == WORSE)  {
          // Conditions #12 through #16 (Routing metrics of current congestion sensitivity are worse than lower congestion sensitivity's)
          int comparisonToHigherRate = compareRoutingMetrics(currentCongSensIndex, currentCongSensIndex + 1, congSensitivityMetrics);
          if (comparisonToHigherRate == WORSE)  {
            // Conditions #12 through #14 (Routing metrics of current congestion sensitivity are worse than higher congestion sensitivity's)
            int compareHigherToLowerRate = compareRoutingMetrics(currentCongSensIndex + 1, currentCongSensIndex - 1, congSensitivityMetrics);
            if ((compareHigherToLowerRate == BETTER) || (compareHigherToLowerRate == EQUIVALENT))  {
              // Conditions #13 and #14 (Routing metrics of next higher congestion sensitivity are better than, or equivalent to, next lower congestion sensitivity's)
              *change_algorithm_during_this_iteration = TRUE;
              *changeCongSensitivity = INCREASE;
              (*num_congSensitivity_changes)++;
              #ifdef DEBUG_assessCongestionSensitivities
              printf("DEBUG: Condition #13 or #14 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                     current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
              #endif
            }  // End of if-block for condition #13 and #14
            else  {
              // Conditions #12 (Routing metrics of next higher congestion sensitivity are worse than next lower congestion sensitivity's)
              *change_algorithm_during_this_iteration = TRUE;
              *changeCongSensitivity = DECREASE;
              (*num_congSensitivity_changes)++;
              (*num_congSensitivity_reductions)++;
              #ifdef DEBUG_assessCongestionSensitivities
              printf("DEBUG: Condition #12 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be DECREASED from %d%%.\n",
                     current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
              #endif
            }  // End of if-block for condition #12
          }  // End of if-block for conditions #12 through #14
          else if (comparisonToHigherRate == BETTER)  {
            // Condition #15 (Routing metrics of current congestion sensitivity are better than higher congestion sensitivity's)
            *change_algorithm_during_this_iteration = TRUE;
            *changeCongSensitivity = DECREASE;
            (*num_congSensitivity_changes)++;
            (*num_congSensitivity_reductions)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #15 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be DECREASED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of if-block for condition #15
          else  {
            // Condition #16 (Routing metrics of current congestion sensitivity are equivalent to higher congestion sensitivity's)
            *change_algorithm_during_this_iteration = TRUE;
            *changeCongSensitivity = INCREASE;
            (*num_congSensitivity_changes)++;
            (*num_congSensitivity_stableRoutingMetrics)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #16 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of else-block for condition #16
        }  // End of if-block for conditions #12 through #16
        else if (comparisonToLowerRate == BETTER)  {
          // Conditions #17 through #19 (Routing metrics of current congestion sensitivity are better than lower sensitivity's)
          int comparisonToHigherRate = compareRoutingMetrics(currentCongSensIndex, currentCongSensIndex + 1, congSensitivityMetrics);
          if (comparisonToHigherRate == WORSE)  {
            // Condition #17 (Routing metrics of current congestion sensitivity are worse than higher sensitivity's)
            *change_algorithm_during_this_iteration = TRUE;
            *changeCongSensitivity = INCREASE;
            (*num_congSensitivity_changes)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #17 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of if-block for condition #17
          else if (comparisonToHigherRate == EQUIVALENT)  {
            // Condition #19 (Routing metrics of current congestion sensitivity are equivalent to higher sensitivity's)
            *change_algorithm_during_this_iteration = TRUE;
            *changeCongSensitivity = INCREASE;
            (*num_congSensitivity_changes)++;
            (*num_congSensitivity_stableRoutingMetrics)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #19 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of else/if-block for condition #19
          else  {
            // Condition #18 (Routing metrics of current congestion sensitivity are better than higher sensitivity's)
            *changeCongSensitivity = NO_CHANGE;
            (*num_congSensitivity_stableRoutingMetrics)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #18 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be UNCHANGED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of else-block for condition #18
        }  // End of if-block for conditions #17 through #19
        else  {
          // Conditions #20 through #22 (Routing metrics of current congestion sensitivity are equivalent to lower sensitivity's)
          int comparisonToHigherRate = compareRoutingMetrics(currentCongSensIndex, currentCongSensIndex + 1, congSensitivityMetrics);
          if (comparisonToHigherRate == WORSE)  {
            // Condition #20 (Routing metrics of current rate are worse than higher rate's)
            *change_algorithm_during_this_iteration = TRUE;
            *changeCongSensitivity = INCREASE;
            (*num_congSensitivity_changes)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #20 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of if-block for condition #20
          else if (comparisonToHigherRate == EQUIVALENT)  {
            // Condition #22 (Routing metrics of current rate are equivalent to higher rate's)
            *change_algorithm_during_this_iteration = TRUE;
            *changeCongSensitivity = INCREASE;
            (*num_congSensitivity_changes)++;
            (*num_congSensitivity_stableRoutingMetrics)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #22 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be INCREASED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of else/if-block for condition #22
          else  {
            // Condition #21 (Routing metrics of current congestion sensitivity are better than higher sensitivity's)
            *changeCongSensitivity = NO_CHANGE;
            (*num_congSensitivity_stableRoutingMetrics)++;
            #ifdef DEBUG_assessCongestionSensitivities
            printf("DEBUG: Condition #21 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be UNCHANGED from %d%%.\n",
                   current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
            #endif
          }  // End of else-block for conditions #21
        }  // End of else-block for conditions #20 through #22
      }  // End of else-block for conditions #12 through #22
    }  // End of else-block for conditions #9 through #22
  }  // End of if-block for conditions #5 through #22

  // Finally, handle conditions #23 through #26:
  else if (currentCongSensIndex == NUM_CONG_SENSITIVITES - 1)  {
    if (congSensitivityMetrics[currentCongSensIndex - 1].iterationOfMeasuredMetrics == 0)  {
      // Condition #23 (Routing metrics are unknown for next lower congestion sensitivity)
      *change_algorithm_during_this_iteration = TRUE;
      *changeCongSensitivity = DECREASE;
      (*num_congSensitivity_changes)++;
      (*num_congSensitivity_reductions)++;
      #ifdef DEBUG_assessCongestionSensitivities
      printf("DEBUG: Condition #23 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be DECREASED from %d%%.\n",
             current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
      #endif
    }  //  End of if-block for condition #23
    else  {
      // Conditions #24 through #26 (Routing metrics are known for next lower congestion sensitivity)
      int comparisonToLowerRate = compareRoutingMetrics(currentCongSensIndex, currentCongSensIndex - 1, congSensitivityMetrics);
      if ((comparisonToLowerRate == BETTER) || (comparisonToLowerRate == EQUIVALENT))  {
        // Conditions #25 and #26 (Routing metrics of current congestion sensitivity are better than, or equivalent to, lower sensitivity's)
        *changeCongSensitivity = NO_CHANGE;
        (*num_congSensitivity_stableRoutingMetrics)++;
        #ifdef DEBUG_assessCongestionSensitivities
        printf("DEBUG: Condition #25 or #26 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be UNCHANGED from %d%%.\n",
               current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
        #endif
      }  // End of if-block for conditions #25 and #26
      else  {
        // Condition #24 (outing metrics of current congestion sensitivity are worse than lower sensitivity's)
        *change_algorithm_during_this_iteration = TRUE;
        *changeCongSensitivity = DECREASE;
        (*num_congSensitivity_changes)++;
        (*num_congSensitivity_reductions)++;
        #ifdef DEBUG_assessCongestionSensitivities
        printf("DEBUG: Condition #24 detected in assessCongestionSensitivities during iteration %d. Congestion sensitivity will be DECREASED from %d%%.\n",
               current_iteration, congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
        #endif
      }  // End of else-block for condition #24
    }  // End of else-block for conditions #24 through #26
  }  // End of if-block for conditions #23 through #26

  // If we get to the following else-block, it means that the currentCongSensIndex variable has
  // an illegal value. Issue a fatal error message and exit the program:
  else  {
      printf("\nERROR: An unexpected condition was encountered in function 'assessCongestionSensitivities()' during iteration %d. Please report the\n",
             current_iteration);
      printf(  "       following information to the software developer:\n");
      printf(  "         Current congestion sensitivity index = %d\n", currentCongSensIndex);
      printf(  "               Current congestion sensitivity = %d%%\n", congSensitivityMetrics[currentCongSensIndex].dynamicParameter);
      printf(  "                             inMetricsPlateau = %d\n\n", inMetricsPlateau);
      for (int i = 0; i < NUM_CONG_SENSITIVITES; i++)  {
        printf(  "         Rate = %d%%: iterationOfMeasuredMetrics = %d, fractionIterationsWithoutDRCs = %.3f, avgNonPseudoNetsWithDRCs = %.3f +/- %.3f, avgNonPseudoRoutingCost = %.2E +/- %.2E\n",
               congSensitivityMetrics[i].dynamicParameter,
               congSensitivityMetrics[i].iterationOfMeasuredMetrics,
               congSensitivityMetrics[i].fractionIterationsWithoutDRCs,
               congSensitivityMetrics[i].avgNonPseudoNetsWithDRCs,
               congSensitivityMetrics[i].stdErrNonPseudoNetsWithDRCs,
               congSensitivityMetrics[i].avgNonPseudoRoutingCost,
               congSensitivityMetrics[i].stdErrNonPseudoRoutingCost);
      }  // End of for-loop for index 'i'
      printf("\n       Program is exiting.\n\n");
      exit(1);
  }  // End of else-block for ERROR condition


}  // End of function 'assessCongestionSensitivities'


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
//                     (g) The routing metrics have justified reducing the via congestion
//                         sensitivity at least 1 time, or else holding this sensitivity
//                         stable at the current level.
//                     (h) The routing metrics have justified reducing the trace congestion
//                         sensitivity at least 1 time, or else holding this sensitivity
//                         stable at the current level.
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
//
// Define 'DEBUG_determineAlgorithmChanges' and re-compile if you want verbose
// debugging print-statements enabled:
//
#define DEBUG_determineAlgorithmChanges 1
// #undef DEBUG_determineAlgorithmChanges

void determineAlgorithmChanges(MapInfo_t *mapInfo, int DRC_free_threshold,
                                 RoutingMetrics_t *routability, const InputValues_t *user_inputs)  {

  #ifdef DEBUG_determineAlgorithmChanges
  printf("\nDEBUG: Entered function determineAlgorithmChanges...\n");
  #endif

  // Initialize to FALSE the four Boolean variables that this function modifies:
  routability->swapStartAndEndTerms[mapInfo->current_iteration] = FALSE;
  routability->changeViaCongSensitivity[mapInfo->current_iteration] = FALSE;
  routability->changeTraceCongSensitivity[mapInfo->current_iteration] = FALSE;
  routability->enablePseudoTraceCongestion[mapInfo->current_iteration] = FALSE;

  // Initialize flag that is set if *any* changes are made during the current iteration:
  unsigned char change_algorithm_during_this_iteration = FALSE;

  // Calculate the total number of nets, including user-defined and pseudo-nets:
  int num_total_nets = mapInfo->numPaths + mapInfo->numPseudoPaths;

  // Over the last 20 iterations ('numIterationsToReEquilibrate'), calculate the following
  // routing metrics:
  //    1) fractionRecentIterationsWithoutMapDRCs
  //         => Fraction of recent iterations without any DRCs on non-pseudo nets
  //    2) avg_nonPseudo_nets_with_DRCs
  //         => Average number of non-pseudo nets that have DRCs
  //    3) avg_nonPseudo_routing_cost
  //         => Average routing cost of non-pseudo nets
  int num_recent_iterations_with_DRCs = 0;  // Number of recent iterations with non-pseudo DRCs
  int sum_nonPseudo_nets_with_DRCs    = 0;  // Sum of non-pseudo nets with DRCs from recent iterations.
  int sumSqu_nonPseudo_nets_with_DRCs = 0;  // Sum of squares of of non-pseudo nets with DRCs from recent iterations.
  unsigned long sum_routing_cost      = 0;  // Sum of non-pseudo routing costs from recent iterations.
  unsigned long sumSqu_routing_cost   = 0;  // Sum of squares of non-pseudo routing costs from recent iterations.
  float avg_nonPseudo_nets_with_DRCs  = 0.0; // Average number of non-pseudo nets with DRCs from recent iterations.
  float stdErr_nonPseudo_nets_with_DRCs = 0.0; // Standard error in number of non-pseudo nets with DRCs from recent iterations.
  float avg_nonPseudo_routing_cost    = 0.0;   // Average non-pseudo routing cost from recent iterations.
  float stdErr_nonPseudo_routing_cost = 0.0;   // Standard error in non-pseudo routing cost from recent iterations.
  for (int i = mapInfo->current_iteration; i > max(0, mapInfo->current_iteration - numIterationsToReEquilibrate); i--)  {
    if (routability->nonPseudo_num_DRC_cells[i] > 0) {
      num_recent_iterations_with_DRCs++;
      // printf("DEBUG:   Iteration %d had %d DRC cells, so increasing num_recent_iterations_with_DRCs to %d.\n",
      //         i, routability->nonPseudo_num_DRC_cells[i], num_recent_iterations_with_DRCs);

      sum_nonPseudo_nets_with_DRCs    += routability->numNonPseudoDRCnets[i];
      sumSqu_nonPseudo_nets_with_DRCs += routability->numNonPseudoDRCnets[i]  *  routability->numNonPseudoDRCnets[i];
      // printf("DEBUG:   Iteration %d had %d nets with DRCs, so increasing sum_nonPseudo_nets_with_DRCs to %d.\n",
      //         i, routability->numNonPseudoDRCnets[i], sum_nonPseudo_nets_with_DRCs);

    }  // End of if-block

    // Add the non-pseudo routing cost from iteration 'i':
    sum_routing_cost    += routability->nonPseudoPathCosts[i];
    sumSqu_routing_cost += routability->nonPseudoPathCosts[i]  *  routability->nonPseudoPathCosts[i];

  }  // End of for-loop for index 'i'

  // Using the sums from the loop above, calculate the averages and standard deviations
  // over the most recent iterations that are necessary later in this function:
  int interationsToAverage = min(numIterationsToReEquilibrate, mapInfo->current_iteration);
  routability->fractionRecentIterationsWithoutMapDRCs = 1.0 - (float)num_recent_iterations_with_DRCs / interationsToAverage;
  avg_nonPseudo_nets_with_DRCs    = (float)sum_nonPseudo_nets_with_DRCs / interationsToAverage;
  stdErr_nonPseudo_nets_with_DRCs
      = sqrt((double)(sumSqu_nonPseudo_nets_with_DRCs - (double)(sum_nonPseudo_nets_with_DRCs * sum_nonPseudo_nets_with_DRCs / interationsToAverage) ))
                      / interationsToAverage;
  avg_nonPseudo_routing_cost    = (float)sum_routing_cost / interationsToAverage;
  stdErr_nonPseudo_routing_cost
      = sqrt((double)(sumSqu_routing_cost - (double)(sum_routing_cost * sum_routing_cost / interationsToAverage) ))
                      / interationsToAverage;

  #ifdef DEBUG_determineAlgorithmChanges
  printf("DEBUG:        num_recent_iterations_with_DRCs = %d for iteration %d.\n", num_recent_iterations_with_DRCs, mapInfo->current_iteration);
  printf("DEBUG: fractionRecentIterationsWithoutMapDRCs = %6.3f for iteration %d.\n", routability->fractionRecentIterationsWithoutMapDRCs, mapInfo->current_iteration);
  printf("DEBUG:           avg_nonPseudo_nets_with_DRCs = %6.3f for iteration %d.\n", avg_nonPseudo_nets_with_DRCs, mapInfo->current_iteration);
  printf("DEBUG:        stdErr_nonPseudo_nets_with_DRCs = %6.3f for iteration %d.\n", stdErr_nonPseudo_nets_with_DRCs, mapInfo->current_iteration);
  printf("DEBUG:             avg_nonPseudo_routing_cost = %'6.3f for iteration %d.\n", avg_nonPseudo_routing_cost, mapInfo->current_iteration);
  printf("DEBUG:          stdErr_nonPseudo_routing_cost = %'6.3f for iteration %d.\n", stdErr_nonPseudo_routing_cost, mapInfo->current_iteration);
  #endif

  // Calculate the number of user-defined nets that currently have DRCs and
  // which are eligible to have their start- and end-terminals swapped. This value
  // is used to determine whether to (1) swap terminals, and (2) change the
  // congestion sensitivity. (Ineligible nets are those with terminals in pin-swap zones.)
  // The 'TRUE' in the parameter list of 'swap_start_and_end_terminals_of_DRC_paths()'
  // tells this function *not* to actually swap any terminals.
  int num_eligible_nets_for_startEnd_terminal_swaps
          = swap_start_and_end_terminals_of_DRC_paths(mapInfo->numPaths + mapInfo->numPseudoPaths, mapInfo, routability, user_inputs, TRUE);
  #ifdef DEBUG_determineAlgorithmChanges
  printf("DEBUG: In function determineAlgorithmChanges after iteration %d, %d net(s) is/are eligible to have their start/end-terminals swapped.\n",
         mapInfo->current_iteration, num_eligible_nets_for_startEnd_terminal_swaps);
  #endif

  // Calculate the number of pseudo-paths with associated diff-pair nets that
  // currently have DRCs on each layer. This value is used to deposit
  // TRACE pseudo-congestion around pseudo-vias:
  int num_eligible_pseudoNets_for_toggling_TRACE_pseudoCongestion = 0;
  for (int pseudoPath = mapInfo->numPaths; pseudoPath < num_total_nets; pseudoPath++)  {
    int diffPairPath_1 = user_inputs->pseudoNetToDiffPair_1[pseudoPath];
    int diffPairPath_2 = user_inputs->pseudoNetToDiffPair_2[pseudoPath];
    for (int layer = 0; layer < mapInfo->numLayers; layer++)  {

      // Shift the 'recent_DRC_flags_by_pseudoPath_layer' variable by 1 bit to the
      // left to make room for the current iteration's Boolean flag:
      routability->recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo->numPaths][layer]
            = routability->recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo->numPaths][layer] << 1;  // Shift bits to left by 1 bit

      // Check if either of the pseudo-path's two children have DRCs on the current
      // layer for the current iteration:
      if (routability->path_DRC_cells_by_layer[diffPairPath_1][layer] || routability->path_DRC_cells_by_layer[diffPairPath_2][layer])  {

        // In the current iteration, this combination of pseudo-path and layer had DRCs. So set the
        // least-significant bit, which is the Boolean flag for the current iteration:
        routability->recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo->numPaths][layer] |= 1; // Bitwise OR with '1'

        // If the 20 least-significant bits in recent_DRC_flags_by_pseudoPath_layer are '1', then the 20 most recent
        // iterations had DRCs for the current pseudo-path and layer. We therefore increment the number of
        // pseudo-path/layer combinations eligible for adding TRACE pseudo-congestion.
        //
        // Note: 0x000FFFFF = 0000 0000 0000 1111  1111 1111 1111 1111 (binary)
        if ((routability->recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo->numPaths][layer] & 0x000FFFFF) == 0x000FFFFF)  {
          num_eligible_pseudoNets_for_toggling_TRACE_pseudoCongestion++;

          #ifdef DEBUG_determineAlgorithmChanges
          printf("DEBUG: In determineAlgorithmChanges in iteration %d, pseudo-path %d on layer %d is eligible to have TRACE pseudo-congestion deposited.\n",
                 mapInfo->current_iteration, pseudoPath, layer);
          #endif

        }  // End of if-block for 20 consecutive iterations with DRCs

      }  // End of if-block for having DRCs on one of the diff-pair paths
      else  {
        // In the current iteration, this combination of pseudo-path and layer had no DRCs. So clear
        // the least-significant bit, which is the Boolean flag for the current iteration. To clear
        // the LSB, we do a bitwise AND with binary 1111 1111 1111 1111 1111 1111 1111 1110,
        // which is hexadecimal FFFFFFFE
        routability->recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo->numPaths][layer] &= 0xFFFFFFFE;
      }  // End of else-block

      #ifdef DEBUG_determineAlgorithmChanges
      printf("DEBUG: In determineAlgorithmChanges in iteration %d, recent_DRC_flags_by_pseudoPath_layer[%d][%d] = %08X (pseudo-path #%d)\n",
             mapInfo->current_iteration, pseudoPath - mapInfo->numPaths, layer,
             routability->recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo->numPaths][layer], pseudoPath);
      #endif

    }  // End of for-loop for index 'layer'
  }  // End of for-loop for index 'pseudoPath'
  #ifdef DEBUG_determineAlgorithmChanges
  printf("DEBUG: In function determineAlgorithmChanges after iteration %d, %d pseudo-net(s) is/are eligible to have their TRACE pseudo-congestion turned on.\n",
         mapInfo->current_iteration, num_eligible_pseudoNets_for_toggling_TRACE_pseudoCongestion);
  #endif


  //
  // Next, update the routing metrics for the current via and trace congestion sensitivities:
  //
  #ifdef DEBUG_determineAlgorithmChanges
  if (routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].iterationOfMeasuredMetrics == 0)  {
    printf("DEBUG: In determineAlgorithmChanges, routing metrics have not yet been defined for via congestion sensitivity %d%%.\n",
           routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter);
  }
  else  {
    printf("DEBUG: In determineAlgorithmChanges, routing metrics were previously measured for via congestion sensitivity %d%%:\n",
           routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter);
    printf("DEBUG:    fraction w/o DRCs=%.3f, nets w/ DRCs=%.3f +/- %.3f, routing cost = %.2E +/- %.2E\n",
           routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].fractionIterationsWithoutDRCs,
           routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].avgNonPseudoNetsWithDRCs,
           routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].stdErrNonPseudoNetsWithDRCs,
           routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].avgNonPseudoRoutingCost,
           routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].stdErrNonPseudoRoutingCost);
  }  // End if if/else-block for iterationOfMeasuredMetrics == 0

  if (routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].iterationOfMeasuredMetrics == 0)  {
    printf("DEBUG: In determineAlgorithmChanges, routing metrics have not yet been defined for trace congestion sensitivity %d%%.\n",
           routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter);
  }
  else  {
    printf("DEBUG: In determineAlgorithmChanges, routing metrics were previously measured for trace congestion sensitivity %d%%:\n",
           routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter);
    printf("DEBUG:    fraction w/o DRCs=%.3f, nets w/ DRCs=%.3f +/- %.3f, routing cost = %.2E +/- %.2E\n",
           routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].fractionIterationsWithoutDRCs,
           routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].avgNonPseudoNetsWithDRCs,
           routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].stdErrNonPseudoNetsWithDRCs,
           routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].avgNonPseudoRoutingCost,
           routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].stdErrNonPseudoRoutingCost);
  }  // End if if/else-block for iterationOfMeasuredMetrics == 0
  #endif

  // Save the metrics to the 'viaCongSensitivityMetrics' array for the recent iterations:
  routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].iterationOfMeasuredMetrics    = mapInfo->current_iteration;
  routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].fractionIterationsWithoutDRCs = routability->fractionRecentIterationsWithoutMapDRCs;
  routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].avgNonPseudoNetsWithDRCs      = avg_nonPseudo_nets_with_DRCs;
  routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].stdErrNonPseudoNetsWithDRCs   = stdErr_nonPseudo_nets_with_DRCs;
  routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].avgNonPseudoRoutingCost       = avg_nonPseudo_routing_cost;
  routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].stdErrNonPseudoRoutingCost    = stdErr_nonPseudo_routing_cost;

  // Also save the metrics to the 'traceCongSensitivityMetrics' array for the recent iterations:
  routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].iterationOfMeasuredMetrics    = mapInfo->current_iteration;
  routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].fractionIterationsWithoutDRCs = routability->fractionRecentIterationsWithoutMapDRCs;
  routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].avgNonPseudoNetsWithDRCs      = avg_nonPseudo_nets_with_DRCs;
  routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].stdErrNonPseudoNetsWithDRCs   = stdErr_nonPseudo_nets_with_DRCs;
  routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].avgNonPseudoRoutingCost       = avg_nonPseudo_routing_cost;
  routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].stdErrNonPseudoRoutingCost    = stdErr_nonPseudo_routing_cost;


  #ifdef DEBUG_determineAlgorithmChanges
  printf("DEBUG: fractionIterationsWithoutDRCs has been updated to %.3f for via congestion sensitivity %d%%.\n",
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].fractionIterationsWithoutDRCs,
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter);
  printf("DEBUG: avgNonPseudoNetsWithDRCs has been updated to %.3f for via congestion sensitivity %d%%.\n",
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].avgNonPseudoNetsWithDRCs,
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter);
  printf("DEBUG: stdErrNonPseudoNetsWithDRCs has been updated to %.3f for via congestion sensitivity %d%%.\n",
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].stdErrNonPseudoNetsWithDRCs,
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter);
  printf("DEBUG: avgNonPseudoRoutingCost has been updated to %'.3f for via congestion sensitivity %d%%.\n",
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].avgNonPseudoRoutingCost,
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter);
  printf("DEBUG: stdErrNonPseudoRoutingCost has been updated to %'.3f for via congestion sensitivity %d%%.\n\n",
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].stdErrNonPseudoRoutingCost,
         routability->viaCongSensitivityMetrics[mapInfo->currentViaCongSensIndex].dynamicParameter);

  printf("DEBUG: fractionIterationsWithoutDRCs has been updated to %.3f for trace congestion sensitivity %d%%.\n",
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].fractionIterationsWithoutDRCs,
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter);
  printf("DEBUG: avgNonPseudoNetsWithDRCs has been updated to %.3f for trace congestion sensitivity %d%%.\n",
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].avgNonPseudoNetsWithDRCs,
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter);
  printf("DEBUG: stdErrNonPseudoNetsWithDRCs has been updated to %.3f for trace congestion sensitivity %d%%.\n",
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].stdErrNonPseudoNetsWithDRCs,
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter);
  printf("DEBUG: avgNonPseudoRoutingCost has been updated to %'.3f for trace congestion sensitivity %d%%.\n",
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].avgNonPseudoRoutingCost,
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter);
  printf("DEBUG: stdErrNonPseudoRoutingCost has been updated to %'.3f for trace congestion sensitivity %d%%.\n\n",
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].stdErrNonPseudoRoutingCost,
         routability->traceCongSensitivityMetrics[mapInfo->currentTraceCongSensIndex].dynamicParameter);
  #endif


  // Next, reset the 'iterationOfMeasuredMetrics' variable to zero for any congestion sensitivity
  // levels that have not been measured in 240 or more iterations (12 x numIterationsToReEquilibrate).
  // This will cause the function to re-measure the metrics, since the previously measured metrics
  // are likely to be stale (no longer valid):
  for (int i = 0; i < NUM_CONG_SENSITIVITES; i++)  {

    // Start with the via congestion sensitivity:
    if (   (routability->viaCongSensitivityMetrics[i].iterationOfMeasuredMetrics > 0)
        && (mapInfo->current_iteration - routability->viaCongSensitivityMetrics[i].iterationOfMeasuredMetrics >= 12*numIterationsToReEquilibrate))  {

      #ifdef DEBUG_determineAlgorithmChanges
      printf("DEBUG: In iteration %d, for via congestion sensitivity %d%%, the routing metrics were last measured at iteration %d, which is more than\n",
             mapInfo->current_iteration, routability->viaCongSensitivityMetrics[i].dynamicParameter,
             routability->viaCongSensitivityMetrics[i].iterationOfMeasuredMetrics);
      printf("DEBUG: %d iterations ago. This via congestion sensitivity will therefore be flagged to have its routing metrics re-measured.\n",
             12*numIterationsToReEquilibrate);
      #endif

      routability->viaCongSensitivityMetrics[i].iterationOfMeasuredMetrics = 0;

      // Also reset the routing metrics to zero to help with debugging:
      routability->viaCongSensitivityMetrics[i].fractionIterationsWithoutDRCs = 0.0;
      routability->viaCongSensitivityMetrics[i].avgNonPseudoNetsWithDRCs      = 0.0;
      routability->viaCongSensitivityMetrics[i].stdErrNonPseudoNetsWithDRCs   = 0.0;
      routability->viaCongSensitivityMetrics[i].avgNonPseudoRoutingCost       = 0.0;
      routability->viaCongSensitivityMetrics[i].stdErrNonPseudoRoutingCost    = 0.0;
    }  // End of if-block for via congestion sensitivities

    // Next, check the trace congestion sensitivity:
    if (   (routability->traceCongSensitivityMetrics[i].iterationOfMeasuredMetrics > 0)
        && (mapInfo->current_iteration - routability->traceCongSensitivityMetrics[i].iterationOfMeasuredMetrics >= 12*numIterationsToReEquilibrate))  {

      #ifdef DEBUG_determineAlgorithmChanges
      printf("DEBUG: In iteration %d, for trace congestion sensitivity %d%%, the routing metrics were last measured at iteration %d, which is more than\n",
             mapInfo->current_iteration, routability->traceCongSensitivityMetrics[i].dynamicParameter,
             routability->traceCongSensitivityMetrics[i].iterationOfMeasuredMetrics);
      printf("DEBUG: %d iterations ago. This trace congestion sensitivity will therefore be flagged to have its routing metrics re-measured.\n",
             12*numIterationsToReEquilibrate);
      #endif

      routability->traceCongSensitivityMetrics[i].iterationOfMeasuredMetrics = 0;

      // Also reset the routing metrics to zero to help with debugging:
      routability->traceCongSensitivityMetrics[i].fractionIterationsWithoutDRCs = 0.0;
      routability->traceCongSensitivityMetrics[i].avgNonPseudoNetsWithDRCs      = 0.0;
      routability->traceCongSensitivityMetrics[i].stdErrNonPseudoNetsWithDRCs   = 0.0;
      routability->traceCongSensitivityMetrics[i].avgNonPseudoRoutingCost       = 0.0;
      routability->traceCongSensitivityMetrics[i].stdErrNonPseudoRoutingCost    = 0.0;
    }  // End of if-block for trace congestion sensitivities

  }  // End of for-loop for index 'i'


  //
  // Make no algorithm changes if any of the following are true:
  //  (1) Acorn is currently NOT in a metrics plateau.
  //  (2) Acorn has made algorithm changes during the last 60 iterations.
  //  (3) Acorn has achieved the required number of DRC-free iterations.
  //  (4) Acorn is within 20 iterations ('numIterationsToReEquilibrate') of the
  //      maximum allowed number of iterations (user_inputs->maxIterations).
  //
  if (   (routability->inMetricsPlateau[mapInfo->current_iteration])
      && (mapInfo->current_iteration >= routability->latestAlgorithmChange + 3 * numIterationsToReEquilibrate)
      && (routability->cumulative_DRCfree_iterations[mapInfo->current_iteration] < DRC_free_threshold)
      && (user_inputs->maxIterations - mapInfo->current_iteration > numIterationsToReEquilibrate)) {

    // We got here, so changes to the routing algorithm are possible because:
    //  (1) Acorn is currently in a metrics plateau.
    //  (2) Acorn has NOT made algorithm changes during the last 60 iterations.
    //  (3) Acorn has NOT achieved the required number of DRC-free iterations.

    //
    // We assess which one (if any) changes to make in the following order, which is the
    // reverse order of their likelihood to occur:
    //   (1) depositing TRACE pseudo-congestion near pseudo-vias,
    //   (2) changing via and/or trace congestion sensitivity, and
    //   (3) swapping start/end-terminals.
    //
    // Step 1: Assess whether to deposit TRACE pseudo-congestion near
    //         pseudo-vias on layer that contains DRCs:
    //
    if ((mapInfo->numLayers > 1) && (num_eligible_pseudoNets_for_toggling_TRACE_pseudoCongestion > 0)
        && (routability->fractionRecentIterationsWithoutMapDRCs <= 0.1)
        && (routability->num_viaCongSensitivity_reductions + routability->num_viaCongSensitivity_stableRoutingMetrics >= 1)
        && (routability->num_traceCongSensitivity_reductions + routability->num_traceCongSensitivity_stableRoutingMetrics >= 1))  {

      #ifdef DEBUG_determineAlgorithmChanges
      printf("DEBUG: In determineAlgorithmChanges, all criteria have been met to enable the\n");
      printf("       application of TRACE pseudo-congestion near pseudo-vias...\n");
      #endif

      // We got here, so TRACE pseudo-congestion around pseudo-vias should be enabled
      // on layers for which pseudo-paths have child-path with DRCs.
      routability->enablePseudoTraceCongestion[mapInfo->current_iteration] = TRUE;
      change_algorithm_during_this_iteration = TRUE;

    }  // End of if-block for assessing whether to enable the application of TRACE pseudo-congestion near pseudo-vias

    //
    // Step 2: Assess whether to change the via and/or trace congestion sensitivity if no changes
    //         are being made during this iteration to application of TRACE congestion
    //         near crowded pseudo-vias:
    //
    if (routability->enablePseudoTraceCongestion[mapInfo->current_iteration] == FALSE)  {
      // First, determine whether the criteria are satisfied for changing the congestion sensitivity:
      if (   (routability->fractionRecentIterationsWithoutMapDRCs <= 0.2)
          && (   (routability->num_startEnd_terminal_swaps >= 3)
              || (num_eligible_nets_for_startEnd_terminal_swaps == 0)))  {

        #ifdef DEBUG_determineAlgorithmChanges
        printf("DEBUG: In determineAlgorithmChanges, most criteria have been met to change the congestion sensitivity...\n");
        #endif


        //
        // Over the last 20 iterations, calculate the fraction of DRC-cells that are caused by trace-to-trace spacing
        // violations, and the fraction that are caused by via-to-via spacing violations:
        //
        int sum_DRCs = 0, sum_trace2trace_DRCs = 0, sum_via2via_DRCs = 0;
        for (int iter = mapInfo->current_iteration; iter >= max(1, mapInfo->current_iteration - numIterationsToReEquilibrate + 1); iter--)  {
          sum_trace2trace_DRCs += routability->nonPseudo_num_trace2trace_DRC_cells[iter];
          sum_via2via_DRCs    += routability->nonPseudo_num_via2via_DRC_cells[iter];
          sum_DRCs            += routability->nonPseudo_num_trace2via_DRC_cells[iter];
        }  // End of for-loop for index 'i'
        sum_DRCs += sum_trace2trace_DRCs + sum_via2via_DRCs;
        float fraction_trace2trace_DRCs = (float)sum_trace2trace_DRCs/sum_DRCs;
        float fraction_via2via_DRCs     = (float)sum_via2via_DRCs    /sum_DRCs;

        printf("\nDEBUG: In iteration %d in function determineAlgorithmChanges, the shape-types of DRCs over the last %d iterations are:\n",
               mapInfo->current_iteration, min(mapInfo->current_iteration, numIterationsToReEquilibrate));
        printf(  "DEBUG:     fraction_trace2trace_DRCs = %.3f\n", fraction_trace2trace_DRCs);
        printf(  "DEBUG:         fraction_via2via_DRCs = %.3f\n\n", fraction_via2via_DRCs);

        /// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


        //
        // Depending on the shape-types involved in recent DRCs, decide whether to increase via congestion
        // sensitivity, trace congestion sensitivity, or both:
        //
        if (fraction_via2via_DRCs > 0.8)  {
          //
          // If more than 80% of the recent DRCs are via-to-via spacing violations, then assess
          // whether to increase the via congestion sensitivity:
          //
          assessCongestionSensitivities(routability->viaCongSensitivityMetrics, &change_algorithm_during_this_iteration,
                                        &(routability->changeViaCongSensitivity[mapInfo->current_iteration]),
                                        &(routability->num_viaCongSensitivity_changes),
                                        &(routability->num_viaCongSensitivity_stableRoutingMetrics),
                                        &(routability->num_viaCongSensitivity_reductions), mapInfo->currentViaCongSensIndex,
                                        mapInfo->current_iteration, routability->inMetricsPlateau[mapInfo->current_iteration]);

          // Because the recent DRCs have been dominated (>80%) by via-to-via spacing violations, we don't
          // both assessing whether to modify the trace congestion sensitivity. We signal to Acorn that
          // trace congestion sensitivity is stable by incrementing 'num_traceCongSensitivity_stableRoutingMetrics':
          routability->num_traceCongSensitivity_stableRoutingMetrics++;

        }  // End of if-block for via-to-via DRCs dominating
        else if (fraction_trace2trace_DRCs > 0.8)  {
          //
          // If more than 80% of the recent DRCs are trace-to-trace spacing violations, then assess
          // whether to increase the trace congestion sensitivity:
          //
          assessCongestionSensitivities(routability->traceCongSensitivityMetrics, &change_algorithm_during_this_iteration,
                                        &(routability->changeTraceCongSensitivity[mapInfo->current_iteration]),
                                        &(routability->num_traceCongSensitivity_changes),
                                        &(routability->num_traceCongSensitivity_stableRoutingMetrics),
                                        &(routability->num_traceCongSensitivity_reductions), mapInfo->currentTraceCongSensIndex,
                                        mapInfo->current_iteration, routability->inMetricsPlateau[mapInfo->current_iteration]);

          // Because the recent DRCs have been dominated (>80%) by trace-to-trace spacing violations, we don't
          // both assessing whether to modify the via congestion sensitivity. We signal to Acorn that
          // via congestion sensitivity is stable by incrementing 'num_viaCongSensitivity_stableRoutingMetrics':
          routability->num_viaCongSensitivity_stableRoutingMetrics++;

        }  // End of if-block for trace-to-trace DRCs dominating
        else  {
          //
          // If the recent DRCs are not dominated by either trace-to-trace or via-to-via violations, then
          // assess whether to increase the trace *and* via congestion sensitivities:
          //
          assessCongestionSensitivities(routability->viaCongSensitivityMetrics, &change_algorithm_during_this_iteration,
                                        &(routability->changeViaCongSensitivity[mapInfo->current_iteration]),
                                        &(routability->num_viaCongSensitivity_changes),
                                        &(routability->num_viaCongSensitivity_stableRoutingMetrics),
                                        &(routability->num_viaCongSensitivity_reductions), mapInfo->currentViaCongSensIndex,
                                        mapInfo->current_iteration, routability->inMetricsPlateau[mapInfo->current_iteration]);

          assessCongestionSensitivities(routability->traceCongSensitivityMetrics, &change_algorithm_during_this_iteration,
                                        &(routability->changeTraceCongSensitivity[mapInfo->current_iteration]),
                                        &(routability->num_traceCongSensitivity_changes),
                                        &(routability->num_traceCongSensitivity_stableRoutingMetrics),
                                        &(routability->num_traceCongSensitivity_reductions), mapInfo->currentTraceCongSensIndex,
                                        mapInfo->current_iteration, routability->inMetricsPlateau[mapInfo->current_iteration]);
        }  // End of else-block for a mixture of trace-to-trace and via-to-via violations




        /// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


      }  // End of if-block for assessing whether to change the congestion sensitivity
    }  // End of if-block for enablePseudoTraceCongestion == FALSE

    //
    // Step 3: Assess whether to swap start- and end-terminals on nets with DRCs if no changes
    //         are being made this iteration to the congestion sensitivities or application of
    //         TRACE congestion near crowded pseudo-vias:
    //
    if (   (routability->changeViaCongSensitivity[mapInfo->current_iteration] == FALSE)
        && (routability->changeTraceCongSensitivity[mapInfo->current_iteration] == FALSE)
        && (routability->enablePseudoTraceCongestion[mapInfo->current_iteration] == FALSE))  {
      //
      // Determine whether the routing metrics meet the criteria for swapping
      // start- and end-terminals on nets with DRCs:
      //
      if ( (num_eligible_nets_for_startEnd_terminal_swaps)
        && (routability->fractionRecentIterationsWithoutMapDRCs <= 0.6))  {

        routability->swapStartAndEndTerms[mapInfo->current_iteration] = TRUE;
        change_algorithm_during_this_iteration = TRUE;
      }  // End of if-block for assessing whether to swap start/end-terminals
    }  // End of if-block for no changes being made to congestion sensitivity or applying TRACE congestion

  }  // End of if-block for inMetricsPlateau && (current_iteration >= 20) && (Acorn hasn't achieved the required number of DRC-free iterations)

  #ifdef DEBUG_determineAlgorithmChanges
  printf("\nDEBUG: At end of function 'determineAlgorithmChanges' after iteration %d:\n", mapInfo->current_iteration);
  printf("DEBUG:                           swapStartAndEndTerms = %d\n", routability->swapStartAndEndTerms[mapInfo->current_iteration]);
  printf("DEBUG:                       changeViaCongSensitivity = %d\n", routability->changeViaCongSensitivity[mapInfo->current_iteration]);
  printf("DEBUG:                     changeTraceCongSensitivity = %d\n", routability->changeTraceCongSensitivity[mapInfo->current_iteration]);
  printf("DEBUG:                    enablePseudoTraceCongestion = %d\n", routability->enablePseudoTraceCongestion[mapInfo->current_iteration]);
  printf("DEBUG:                          latestAlgorithmChange = iteration %d\n", routability->latestAlgorithmChange);
  printf("DEBUG:         change_algorithm_during_this_iteration = %d\n", change_algorithm_during_this_iteration);
  printf("DEBUG:                    num_startEnd_terminal_swaps = %d\n", routability->num_startEnd_terminal_swaps);
  printf("DEBUG:                 num_viaCongSensitivity_changes = %d\n", routability->num_viaCongSensitivity_changes);
  printf("DEBUG:               num_traceCongSensitivity_changes = %d\n", routability->num_traceCongSensitivity_changes);
  printf("DEBUG:              num_viaCongSensitivity_reductions = %d\n", routability->num_viaCongSensitivity_reductions);
  printf("DEBUG:            num_traceCongSensitivity_reductions = %d\n", routability->num_traceCongSensitivity_reductions);
  printf("DEBUG:    num_viaCongSensitivity_stableRoutingMetrics = %d\n", routability->num_viaCongSensitivity_stableRoutingMetrics);
  printf("DEBUG:  num_traceCongSensitivity_stableRoutingMetrics = %d\n", routability->num_traceCongSensitivity_stableRoutingMetrics);
  #endif

  //
  // If we've flagged any changes to the algorithm in this function, then reset
  // the 'latestAlgorithmChange' variable to the current iteration:
  //
  if (change_algorithm_during_this_iteration)  {
    routability->latestAlgorithmChange = mapInfo->current_iteration;
  }

}  // End of function 'determineAlgorithmChanges'


//-----------------------------------------------------------------------------
// Name: findAllPseudoVias
// Desc: Find all the pseudo-vias in the map and place their information in
//       the array 'pseudoVias'. Return the length of this array.
//-----------------------------------------------------------------------------
static int findAllPseudoVias(ViaStack_t *pseudoVias[], const MapInfo_t *mapInfo,
                             const int pathLengths[], Coordinate_t *pathCoords[])  {

  // Initially, allocate enough memory for the 'pseudoVias' array to
  // accommodate enough pseudo-vias for every pseudo-path to have twice
  // as many vias as there are routing layers:
  int max_pseudo_vias = 2 * mapInfo->numPseudoPaths * mapInfo->numLayers;
  int num_pseudo_vias = 0;  // Number of pseudo-vias found in map
  *pseudoVias = malloc(max_pseudo_vias * sizeof(ViaStack_t));

  // printf("DEBUG: At beginning of findAllPseudoVias, pseudoVias array is initially allocated for %d vias.\n", max_pseudo_vias);

  //
  // Iterate through each pseudo-path to locate all pseudo-vias, including
  // those at start- and end-terminals:
  //
  for (int pathNum = mapInfo->numPaths; pathNum < mapInfo->numPaths + mapInfo->numPseudoPaths; pathNum++)  {

    // Initialize the 'prevSegment' (previous segment) coordinates with the coordinates from the
    // start-terminal:
    Coordinate_t prevSegment = copyCoordinates(mapInfo->start_cells[pathNum]);

    // Initialize the 'via_start_segment' to -1, in case the start-terminal (at segment -1)
    // is the start of a pseudo-via:
    int via_start_segment = -1;
    Coordinate_t via_start_coords = copyCoordinates(mapInfo->start_cells[pathNum]);

    // Initialize the Boolean flag 'in_via_stack' to FALSE:
    int in_via_stack = FALSE;

    // Iterate through each segment of pseudo-path 'pathNum' to locate pseudo-vias:
    for (int segment = 0; segment < pathLengths[pathNum]; segment++)  {
      int current_Z = pathCoords[pathNum][segment].Z;

      // Compare routing layer of current segment with that of previous segment. There
      // are 4 possibilities:
      //    Case A: current Z == previous Z, in_via_stack == TRUE
      //    Case B: current Z == previous Z, in_via_stack == FALSE
      //    Case C: current Z != previous Z, in_via_stack == TRUE
      //    Case D: current Z != previous Z, in_via_stack == FALSE
      if (current_Z == prevSegment.Z)  {
        // We got here, so the layer did not change. Now check if we were inside
        // a via-stack:
        if (in_via_stack)  {
          // Case A:
          //   We were in a via-stack, but the previous segment was the last segment
          //   of the stack. Set 'in_via_stack' to FALSE:
          in_via_stack = FALSE;

          // Save the previous segment as the 'endSegment' of the via-stack:
          (*pseudoVias)[num_pseudo_vias].endSegment = segment - 1;
          (*pseudoVias)[num_pseudo_vias].endCoord = copyCoordinates(prevSegment);

          // Save the 'startSegment' of the via-stack:
          (*pseudoVias)[num_pseudo_vias].startSegment = via_start_segment;
          (*pseudoVias)[num_pseudo_vias].startCoord = copyCoordinates(via_start_coords);

          // Save the path-number of the via-stack:
          (*pseudoVias)[num_pseudo_vias].pathNum = pathNum;

          // Calculate whether the end-via segment has a shape-type of VIA_UP or VIA_DOWN:
          if (prevSegment.Z > via_start_coords.Z)  {
            (*pseudoVias)[num_pseudo_vias].endShapeType = VIA_DOWN;
          }
          else  {
            (*pseudoVias)[num_pseudo_vias].endShapeType = VIA_UP;
          }

          // Ensure the 'error' flag is FALSE for this via-stack:
          (*pseudoVias)[num_pseudo_vias].error = FALSE;

          // Increment the number of vias that have been located:
          num_pseudo_vias++;

          // If the number of pseudo-vias has exceeded the number for which memory
          // has been allocated, then allocate more memory:
          if (num_pseudo_vias >= max_pseudo_vias)  {
            // Increase the memory allocation by 10 pseudo-vias:
            max_pseudo_vias += 10;
            *pseudoVias = realloc(*pseudoVias, max_pseudo_vias * sizeof(ViaStack_t));
          }  // End of if-block for num_pseudo_vias >= max_pseudo_vias
        }  // End of if-block for in_via_stack
        else  {
          // Case B:
          //   We got here, so we're not inside a via-stack, and the current segment
          //   is on the same layer as the previous segment. There is no need
          //   to do anything.
        }  // End of else-block for not being in a via-stack

        // Because the layer did not change, we update the 'via_start_segment' to
        // the current segment, just in case the current segment is the start
        // of a via-stack:
        via_start_segment = segment;
        via_start_coords = copyCoordinates(pathCoords[pathNum][segment]);
      }  // End of if-block for current_Z = previous Z
      else  {
        // We got here, so the current_Z does not equal the previous Z. Now check
        // if we were inside a via-stack:
        if (in_via_stack)  {
          // Case C:
          //   We were in a via-stack, and the current segment is on a different
          //   layer than the previous segment. There is nothing to do in this
          //   case.
        }  // End of if-block for in_via_stack
        else  {
          // Case D:
          //   We got here, so the current Z does not equal the previous Z. But
          //   we were not yet in a via-stack. So we set in_via_stack to TRUE.
          in_via_stack = TRUE;
        }  // End of else-block for not being in a via-stack
      }  // End of else-block for current_Z != previous Z

      // Copy the current segment's coordinates into the 'prevSegment' variable
      // in preparation of repeating this loop on the next segment of the path:
      prevSegment = copyCoordinates(pathCoords[pathNum][segment]);

    }  // End of for-loop for index 'segment'

    // We've analyzed all the segments in path 'pathNum', but have not yet
    // handled the case where the end-terminal is also the end-segment of
    // a pseudo-via. This is true if the Boolean variable 'in_via_stack'
    // is TRUE.
    if (in_via_stack)  {
      // Save the end-terminal's segment as the 'endSegment' of the via-stack:
      (*pseudoVias)[num_pseudo_vias].endSegment = pathLengths[pathNum] - 1;
      (*pseudoVias)[num_pseudo_vias].endCoord = copyCoordinates(pathCoords[pathNum][pathLengths[pathNum] - 1]);

      // Save the 'startSegment' of the via-stack:
      (*pseudoVias)[num_pseudo_vias].startSegment = via_start_segment;
      (*pseudoVias)[num_pseudo_vias].startCoord = copyCoordinates(via_start_coords);

      // Save the path-number of the via-stack:
      (*pseudoVias)[num_pseudo_vias].pathNum = pathNum;

      // Calculate whether the end-via segment has a shape-type of VIA_UP or VIA_DOWN:
      if (pathCoords[pathNum][pathLengths[pathNum] - 1].Z > via_start_coords.Z)  {
        (*pseudoVias)[num_pseudo_vias].endShapeType = VIA_DOWN;
      }
      else  {
        (*pseudoVias)[num_pseudo_vias].endShapeType = VIA_UP;
      }

      // Ensure the 'error' flag is FALSE for this via-stack:
      (*pseudoVias)[num_pseudo_vias].error = FALSE;

      // Increment the number of vias that have been located:
      num_pseudo_vias++;

    }  // End of if-block for in_via_stack == TRUE

  }  // End of for-loop for index 'pathNum'

  // printf("DEBUG: At end of findAllPseudoVias, num_pseudo_vias is %d.\n", num_pseudo_vias);

  // Reduce the memory for array 'pseudoVias' based on the actual number of
  // pseudo-vias that were found. If no pseudo-vias were found, then allocate
  // only a single element for the array (rather than zero elements, which
  // is not a good coding practice).
  *pseudoVias = realloc(*pseudoVias, max(1, num_pseudo_vias) * sizeof(ViaStack_t));

  // Return the number of pseudo-vias to the calling program:
  return(num_pseudo_vias);

}  // End of function 'findAllPseudoVias'


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
//
// Define 'DEBUG_addTraceCongestionNearPseudoViasWithDRCs' and re-compile if you
// want verbose debugging print-statements enabled:
//
// #define DEBUG_addTraceCongestionNearPseudoViasWithDRCs 1
#undef DEBUG_addTraceCongestionNearPseudoViasWithDRCs

void addTraceCongestionNearPseudoViasWithDRCs(const MapInfo_t *mapInfo, const int pathLengths[],
                                              Coordinate_t *pathCoords[], CellInfo_t ***cellInfo,
                                              RoutingMetrics_t *routability, const InputValues_t *user_inputs)  {

  #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
  // DEBUG code follows:
  //
  // Check if the input parameters satisfy specific requirements. If so, then set the
  // 'DEBUG_ON' flag to TRUE so that this function will print out additional debug information:
  int DEBUG_ON = FALSE;
  if ((mapInfo->current_iteration >= 285) && (mapInfo->current_iteration <= 288))  {
    printf("\n\nDEBUG: Setting DEBUG_ON to TRUE in addTraceCongestionNearPseudoViasWithDRCs() because specific requirements were met.\n\n");
    DEBUG_ON = TRUE;
  }  // End of if-block for determining whether to set DEBUG_ON to TRUE
  #endif


  // If there are only 2 routing layers, or if there are zero pseudo-paths, then return
  // from this function without performing any actions:
  if ((mapInfo->numLayers < 3) || (user_inputs->num_pseudo_nets == 0))  {
    return;
  }

  // Create an array of via-stacks that holds all pseudo-vias in the entire map.
  ViaStack_t *pseudoVias;

  // Call function that searches all paths for all pseudo-vias, placing them in the
  // 'pseudoVias' array, which has 'num_pseudo_vias' elements:
  int num_pseudo_vias = findAllPseudoVias(&pseudoVias, mapInfo, pathLengths, pathCoords);

  #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
  if (DEBUG_ON)  {
    printf("\nDEBUG: Following %d pseudo-vias were found in addTraceCongestionNearPseudoViasWithDRCs:\n", num_pseudo_vias);
    for (int i = 0; i < num_pseudo_vias; i++)  {
      printf("DEBUG:   Pseudo-via #%d: pathNum = %d, startSegment = %d (%d,%d,%d), endSegment = %d (%d,%d,%d), endShapeType = %d.\n",
             i, pseudoVias[i].pathNum, pseudoVias[i].startSegment, pseudoVias[i].startCoord.X, pseudoVias[i].startCoord.Y, pseudoVias[i].startCoord.Z,
             pseudoVias[i].endSegment, pseudoVias[i].endCoord.X, pseudoVias[i].endCoord.Y, pseudoVias[i].endCoord.Z, pseudoVias[i].endShapeType);
    }  // End of for-loop for index 'i'
    printf("DEBUG: End of pseudo-via list.\n\n");
  }
  #endif

  // Define a variable to hold the (x,y,z) location of the pseudo-via's center a given routing layer:
  Coordinate_t via_centerPoint;

  //
  // Iterate through each of the pseudo-vias:
  //
  for (int i = 0; i < num_pseudo_vias; i++)  {

    #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
    if (DEBUG_ON)  {
      printf("DEBUG: Analyzing pseudo-via %d...\n", i);
    }
    #endif

    // Make a local copy of the path number for this pseudo-via:
    int pseudoPathNum = pseudoVias[i].pathNum;

    // Make local copies of the (x,y,z) coordinates of the start- and end-segments of the pseudo-via:
    Coordinate_t viaStartCoords, viaEndCoords;
    viaStartCoords = copyCoordinates(pseudoVias[i].startCoord);
    viaEndCoords   = copyCoordinates(pseudoVias[i].endCoord);
    via_centerPoint.X = pseudoVias[i].startCoord.X;
    via_centerPoint.Y = pseudoVias[i].startCoord.Y;

    // Find the minimum and maximum layer numbers.
    int min_Z, max_Z;
    if (viaStartCoords.Z < viaEndCoords.Z)  {
      min_Z = viaStartCoords.Z;
      max_Z = viaEndCoords.Z;
    }
    else  {
      min_Z = viaEndCoords.Z;
      max_Z = viaStartCoords.Z;
    }

    // Define the path number of the universal repellent net:
    const int universal_repellent_pathNum = mapInfo->numPaths + mapInfo->numPseudoPaths;

    // Iterate over each routing layer in the current pseudo-via:
    for (int layer = min_Z; layer <= max_Z; layer++)  {

      // Skip the current layer if it's either the top or bottom routing layer in the
      // map. There is no point in adding TRACE pseudo-congestion to these layers,
      // since there's no possibility of adding a via above or below the map:
      if ((layer == 0) || (layer == mapInfo->numLayers - 1))  {

        #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
        if (DEBUG_ON)  {
          printf("DEBUG:   For pseudo-via %d, we're skipping layer %d because it's an extremum layer.\n", i, layer);
        }
        #endif

        continue;
      }
      #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
      else {
        if (DEBUG_ON)  {
          printf("DEBUG:   For pseudo-via %d, we're checking for DRCs on layer %d because it's not an extremum layer.\n", i, layer);
        }
      }
      #endif

      // If 'addPseudoTraceCongestionNearVias' is TRUE for this combination of pseudo-path and routing-layer,
      // then add TRACE pseudo-congestion around this via on layer 'layer':
      if (mapInfo->addPseudoTraceCongestionNearVias[pseudoPathNum][layer])  {

        #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
        if (DEBUG_ON)  {
          printf("DEBUG: For pseudo-via %d and layer %d, addPseudoTraceCongestionNearVias[%d][%d] is TRUE.\n", i, layer, pseudoPathNum, layer);
        }
        #endif

        // Get the design-rule set at the (x,y,z) location of the pseudo-via:
        int DR_set = cellInfo[via_centerPoint.X][via_centerPoint.Y][layer].designRuleSet;

        // Get the design-rule subset of the pseudo-path in the design-rule set at the
        // pseudo-via's center:
        int DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][DR_set];

        // Get the z-coordinate of the pseudo-via's center on the current layer:
        via_centerPoint.Z = layer;

        // Get the half-width of a pseudo-trace on the current layer, and use this value (in cell units)
        // as the radius of the circle in which TRACE pseudo-path congestion will be deposited.
        int cong_radius = user_inputs->designRules[DR_set][DR_subset].radius[TRACE];

        // Calculate the square of the congestion radius, which is required by function
        // addCongestionAroundPoint_withSubsetAndShapeType():
        int cong_radius_squared = cong_radius * cong_radius;


        // Determine if the potentially new via would be a via-up or a via-down. Then get
        // the via cost-multiplier and the congestion-related G-cost of such a via:
        int viaCostMultiplier;
        int congestion_Gcost;
        if (layer == min_Z)  {
          // We got here, so we're on the lowest (least positive) routing layer number in
          // the via-stack. A potential extra via would therefore be a VIA_DOWN, so get the
          // cost-multiplier for a VIA_DOWN from the current layer:
          viaCostMultiplier = user_inputs->viaCostMultiplier[cellInfo[via_centerPoint.X][via_centerPoint.Y][layer].viaDownCostMultiplierIndex];

          // Next, get the congestion-related G-cost of creating a VIA_DOWN from the current
          // layer to the target layer:
          int target_layer = layer - 1;
          int target_DR_set = cellInfo[via_centerPoint.X][via_centerPoint.Y][target_layer].designRuleSet;
          int target_DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][target_DR_set];

          // Call 'calc_via_congestion' to get the congestion-related G-cost from the current layer
          // down to the target layer:
          congestion_Gcost = calc_via_congestion(pseudoPathNum, target_DR_set, target_DR_subset, DR_set,
                                                 DR_subset, cellInfo, user_inputs, mapInfo, via_centerPoint.X,
                                                 via_centerPoint.Y, layer, target_layer, FALSE, 0, FALSE);
        }
        else  {
          // We got here, so we're on the highest (most positive) routing layer number in
          // the via-stack. A potential extra via would therefore be a VIA_UP, so get the
          // cost-multiplier for a VIA_UP from the current layer:
          viaCostMultiplier = user_inputs->viaCostMultiplier[cellInfo[via_centerPoint.X][via_centerPoint.Y][layer].viaUpCostMultiplierIndex];

          // Next, get the congestion-related G-cost of creating a VIA_UP from the current
          // layer to the target layer:
          int target_layer = layer + 1;
          int target_DR_set = cellInfo[via_centerPoint.X][via_centerPoint.Y][target_layer].designRuleSet;
          int target_DR_subset = user_inputs->designRuleSubsetMap[pseudoPathNum][target_DR_set];

          // Call 'calc_via_congestion' to get the congestion-related G-cost from the current layer
          // up to the target layer:
          congestion_Gcost = calc_via_congestion(pseudoPathNum, target_DR_set, target_DR_subset, DR_set,
                                                 DR_subset, cellInfo, user_inputs, mapInfo, via_centerPoint.X,
                                                 via_centerPoint.Y, layer, target_layer, FALSE, 0, FALSE);

        }  // End of if/else block for layer == min_Z

        // Calculate the amount of congestion to be deposited in each cell. This value is intended
        // to exceed the cost of creating a via to another layer, thereby causing the autorouter to
        // create a via rather than routing laterally on the current layer. The amount of congestion
        // to deposit consists of two components, which are added together:
        //  A. Distance-related G-cost of the vertical via, which is the product of:
        //      (1) ONE_TRAVERSAL, which is a constant value of 100,
        //      (2) the user-defined vertical cost (in units of cells)
        //      (3) the cost-multiplier for a via-up or a via-down from the current layer.
        //  B. Congestion-related G-cost of the vertical via, which depends on the via's neighbors.
        int distance_Gcost    = ONE_TRAVERSAL * user_inputs->baseVertCostCells * viaCostMultiplier;

        // Add the distance-related G-cost of a new via to the congestion-related G-cost of such a via:
        int congestion_amount = (distance_Gcost + congestion_Gcost) / cong_radius;


        // We increase the congestion_amount by 50x to absolutely ensure that it repels traces from all
        // pseudo-paths, no matter how congested the nearby region becomes:
        congestion_amount *= 50;


        #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
        if (DEBUG_ON)  {
          printf("DEBUG: In function addTraceCongestionNearPseudoViasWithDRCs, about to deposit TRACE pseudo-congestion with following parameters:\n");
          printf("DEBUG:               pseudo-via number = %d\n", i);
          printf("DEBUG:                   pseudoPathNum = %d\n", pseudoPathNum);
          printf("DEBUG:     universal_repellent_pathNum = %d\n", universal_repellent_pathNum);
          printf("DEBUG:                          DR_set = %d\n", DR_set);
          printf("DEBUG:                       DR_subset = %d\n", DR_subset);
          printf("DEBUG:     via_centerPoint coordinates = (%d,%d,%d)\n", via_centerPoint.X, via_centerPoint.Y, via_centerPoint.Z);
          printf("DEBUG:                     cong_radius = %d cells\n", cong_radius);
          printf("DEBUG:             cong_radius_squared = %d cells^2\n", cong_radius_squared);
          printf("DEBUG:                  distance_Gcost = %d\n", distance_Gcost);
          printf("DEBUG:                congestion_Gcost = %d\n", congestion_Gcost);
          printf("DEBUG:               congestion_amount = %d\n", congestion_amount);
          printf("DEBUG:                        baseVertCostCells = %d cells\n", user_inputs->baseVertCostCells);
          printf("DEBUG:                 VIA_UP viaCostMultiplier = %d\n",
                  user_inputs->viaCostMultiplier[cellInfo[via_centerPoint.X][via_centerPoint.Y][layer].viaUpCostMultiplierIndex]);
          printf("DEBUG:               VIA_DOWN viaCostMultiplier = %d\n",
                  user_inputs->viaCostMultiplier[cellInfo[via_centerPoint.X][via_centerPoint.Y][layer].viaDownCostMultiplierIndex]);
        }
        #endif

        //
        // Deposit congestion on the current layer using the 'universal repellent' path:
        //
        #ifdef DEBUG_addCongestion
        addCongestionAroundPoint_withSubsetAndShapeType(universal_repellent_pathNum, DR_set, DR_subset, TRACE,
                                                        via_centerPoint, cong_radius, cong_radius_squared,
                                                        congestion_amount, user_inputs, mapInfo, cellInfo, FALSE);
        #else
        addCongestionAroundPoint_withSubsetAndShapeType(universal_repellent_pathNum, DR_set, DR_subset, TRACE,
                                                        via_centerPoint, cong_radius, cong_radius_squared,
                                                        congestion_amount, user_inputs, mapInfo, cellInfo);
        #endif

      }  // End of if-block for (addPseudoTraceCongestionNearVias == TRUE)
      #ifdef DEBUG_addTraceCongestionNearPseudoViasWithDRCs
      else {
        if (DEBUG_ON)  {
          printf("DEBUG: For pseudo-via %d and layer %d, addPseudoTraceCongestionNearVias[%d][%d] is FALSE.\n", i, layer, pseudoPathNum, layer);
        }
      }
      #endif


    }  // End of for-loop for index 'layer' (from min_Z to max_Z)

  }  // End of for-loop for index 'i' (0 to num_pseudo_vias)

  // Release the memory from the heap that was allocated during this function:
  free(pseudoVias);
  pseudoVias = NULL;  // Set pointer to NULL as a precaution

}  // End of function 'addTraceCongestionNearPseudoViasWithDRCs'


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
                      int maxIterations, RoutingMetrics_t *routability)  {

  // DEBUG code follows:
  printf("\nDEBUG: Entered function 'determineIfSolved' with following input values:\n");
  printf("DEBUG:      iteration = %d,   DRCfree_threshold = %d,   cumulative_DRCfree_iterations = %d,   inMetricsPlateau = %d,  maxIterations = %d\n",
         iteration, DRCfree_threshold, routability->cumulative_DRCfree_iterations[iteration], routability->inMetricsPlateau[iteration], maxIterations);
  // End of DEBUG code

  // Check whether the required number of DRC-free iterations were achieved during
  // the *current* iteration. If so, then record the current iteration number:
  if (   (routability->DRC_free_threshold_achieved == 0)
      && (routability->cumulative_DRCfree_iterations[iteration] == DRCfree_threshold)) {
    routability->DRC_free_threshold_achieved = iteration;
  }

  //
  // If the criteria are met, then return TRUE. Return FALSE otherwise:
  //
  if (   (routability->cumulative_DRCfree_iterations[iteration] >= DRCfree_threshold)
      && (   (num_routed_paths == 1)
          || (routability->inMetricsPlateau[iteration])
          || (iteration >= routability->DRC_free_threshold_achieved + numIterationsToReEquilibrate)
          || (iteration == maxIterations)))  {

    return(TRUE);

  }
  else  {
    return(FALSE);
  }

}  // End of function 'determineIfSolved'

