#ifndef GLOBAL_HEADER_H

#define GLOBAL_HEADER_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <regex.h>
#include <stddef.h>
#include <stdbool.h>
#include <libgen.h>  // Library that contains function 'basename()' for extracting base file name
#include <locale.h>  // Included to enable 'setlocale' for formatting "9,876,543" integers
#include <png.h>     // C library for reading/writing PNG graphics files
#include "omp.h"     // OpenMP for parallel processing

// Define value of PI if it's not already defined by 'math.h':
#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif /* M_PI */

// Define value of NULL if it's not already defined by 'stddef.h':
#ifndef NULL
#define NULL   ((void *) 0)
#endif

// Define following macros for 'max(a,b)' and 'min(a,b)', which are intended
// to compare two integers and return the larger/smaller of the two:
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )


// Declare constants
enum {

  // Parameters that start with 'default...' are default values that can be overridden
  // by the user via the input text file.
  defaultPreEvaporationIterations = 2, // Number of iterations before beginning evaporation of congestion
  defaultEvapRate = 10,         // Evaporation rate (in percent). This percentage of congestion is evaporated each iteration.
  defaultDRCfreeThreshold = 10, // Minimum number of DRC-free solutions before program will end
  defaultRunsPerPngMap =  1, // Number of iterations to run between writing PNG maps
  defaultMaxIterations = 2000, // Maximum number of iterations to find a DRC-free solution
  subMap_maxIterations = 10,   // Number of iterations to use for path-finding for diff-pair connections
  defaultCellCost  =   10,   // Cost of moving to adjacent cell (north, S, E, or W)
  defaultDiagCost  =   14,   // Cost of moving to diagonal cell (NW, NE, SW, or SE)
  defaultKnightCost =  22,   // Cost of moving 2 cells (N/S/E/W), and one sideways,
                             // like a knight in the game of chess
  defaultVertCost  =   0,    // Cost of moving up or down to a different routing layer.

  maxNets   = 1023,   // Maximum number of user-defined nets allowed, limited by 10-bit field
                      // (1023 = 2^10 - 1), and use of an Acorn-defined path used as a 'global
                      // repellent to all other paths.
  maxWidthCells = 8192,      // Maximum number of cells in the X-direction of the map, limited
                             // by the 13 bits dedicated to X-coordinates in structure 'Coordinate_t'.
  maxHeightCells = 8192,     // Maximum number of cells in the Y-direction of the map, limited
                             // by the 13 bits dedicated to Y-coordinates in structure 'Coordinate_t'.
  maxRoutingLayers = 10,       // Maximum number of routing layers allowed (excluding via layers)
  maxTraversingShapes = 4095,  // Maximum number of unique path numbers, design-rule subsets, and
                               // shape-types that can traverse a cell, limited by the number of
                               // bits (12 bits) allocated in structure 'CellInfo_t' for member
                               // 'numTraversingPaths'.
  maxSwapZones = 255, // Maximum number of unique, non-contiguous pin-swapping zones in the map

  maxNetNameLength = 32,// Maximum number of characters in net names
  maxLayerNameLength = 16,// Maximum number of characters in layer names
  maxDesignRuleSets = 16, // Maximum number of design-rule sets
  maxDesignRuleSubsets = 16, // Maximum number of net-specific groups within a design-rule set
  maxDesRuleSetNameLength = 32, // Max length for name of design-rule set or diff-pair rule
  maxDesRuleSetDescriptionLength = 80, // Max length for description of design-rule set or diff-pair rule
  maxTraceCostMultipliers = 16, // Max number of cost-multipliers for lateral routing
  maxViaCostMultipliers = 8, // Max number of cost-multipliers for vias
  maxDiffPairRules = 8, // Maximum number of rules for differential pairs

  maxBlockInstructions = 3000, // Maximum number of BLOCK and UNBLOCK statements in input file
  maxBlockInstructionLength = 16, // Maximum number of characters in BLOCK/UNBLOCK command ('UNBLOCK RECT')
  maxBlockParameters = 6, // Maximum number of floating-point parameters in BLOCK/UNBLOCK command 

  maxDRzoneShapeLength = 5, // Maximun number of characters in a shape for DR_zone commands, e.g., 'RECT'
  maxDRzoneParameters = 6, // Maximum number of floating-point parameters in DR_zone command 

  maxCostZones = 1000, // Maximum number of trace_cost_zone statements in input file, i.e., max number of trace-cost zones in map
                       // Also the max number of via_cost_zone statements in input file.
  maxCostShapeLength = 5, // Maximun number of characters in a shape for trace_cost_zone commands
                          // and in via_cost_zone commands, e.g., 'RECT'
  maxCostParameters = 6, // Maximum number of floating-point parameters in trace_cost_zone command.
                         // Also the max number of parameters in via_cost_zone command.

  maxPinSwapInstructionLength = 18, // Maximum number of characters in PIN_SWAP/NO_PIN_SWAP command ('NO_PIN_SWAP RECT')
  maxPinSwapParameters = 6, // Maximum number of floating-point parameters in PIN_SWAP/NO_PIN_SWAP command 
  maxPinSwapShapeLength = 5, // Maximun number of characters in a shape for pin_swap and 
                             // no_pin_swap commands, e.g., 'RECT'
 
  maxCongestion = 16777215, // Largest allowed value of congestion, limited by the 24-bit field in the
                            // congestion matrix. (2^24 - 1 = 16,777,215)
  maxRecordedDRCs = 10,     // Maximum number of DRC violations that the program records for each iteration
  maxPrintedDRCs = 200,     // Maximum number of DRC violations that the program prints to STDOUT for
                            // each iteration.

  numIterationsToReEquilibrate  = 20, // Number of iterations to achieve a new equilibrium in routing
                                      // metrics after a change to the routing algorithm.

  // Define 3 shape-types and their associated indices in various arrays:
  NUM_SHAPE_TYPES = 3, 
  TRACE           = 0,
  VIA_UP          = 1,
  VIA_DOWN        = 2,
 
  // Boolean masks for the 'pathMap' matrix:
  pathMap_noPaths    = 0x00, // Binary mask for checking whether a cell in the pathMap 3D array has
                             // no paths that traverse it.
  pathMap_onePath    = 0x01, // Binary mask for bit #0 of 3D pathMap array. If set, cell is traversed
                             // by one path.
  pathMap_multiPath  = 0x02, // Binary mask for bit #1 of 3D pathMap array. If set, cell is traversed
                             // by 2 or more paths
  pathMap_onePitch   = 0x04, // Binary mask for bit #2 of 3D pathMap array. If set, the cell is 
                             // within 1 radius of 1 (and only 1) path's centerline, with radius defined
                             // as 1 pitch minus half a linewidth.
  pathMap_multiPitch = 0x08, // Binary mask for bit #3 of 3D pathMap array. If set, the cell is 
                             // within 1 radius of 2 (or more) paths' centerlines, with radius defined
                             // as 1 pitch minus half a linewidth.
  pathMap_startPoint = 0x10, // Binary mask for bit #4 of 3D pathMap array. If set, cell is the starting-
                             // point of a path.
  pathMap_endPoint   = 0x20, // Binary mask for bit #5 of 3D pathMap array. If set, cell is the ending-
                             // point of a path.
  pathMap_widthFlag  = 0x40, // Binary mask for bit #6 of 3D pathMap array. This bit is used as a
                             // temporary variable to determine whether the cell has been identified as
                             // within a half linewidth of a path's centerline.
  pathMap_pitchFlag  = 0x80, // Binary mask for bit #7 of 3D pathMap array. This bit is used as a 
                             // temporary variable to determine whether the cell has been identified as
                             // within 1 pitch of a path's centerline.
  pathMap_unwalkable = 0xFF, // Binary mask for all bits of 3D pathMap array. If all 8 bits are set,
                             // then cell is unwalkable.

  ONE_TRAVERSAL = 100,       // Value to add to congestion matrix to represent a net traversing a cell. This
                             // value is not simply '1' so that 'evaporateCongestion()' can reduce the
                             // congestion value by a percentage without introducing large rounding errors.

  // Define constant used for invoking function calcRoutabilityMetrics() to instruct this function
  // to deposit congestion for all nets.
  ADD_CONGESTION_FOR_ALL_NETS = -1,

  // Define the multiplier used to increase the G-, H-, and F-costs in non-pin-swap zones, relative to
  // pin-swap zones. This multiplier is equal to 2^(NON_PIN_SWAP_EXPONENT):
  NON_PIN_SWAP_EXPONENT = 30,

  // Factor by which we increase congestion-related G-cost for for diff-pair partner
  // nets, relative to the values that we use for for non-diff-pair partners:
  DIFF_PAIR_PARTNER_VIA_CONGESTION_FACTOR   = 16,
  DIFF_PAIR_PARTNER_TRACE_CONGESTION_FACTOR =  1,

  // Parameters for changing the via congestion sensitivity or for randomly changing congestion-related G-cost:
  NO_CHANGE = 0,
  DECREASE  = 1,
  INCREASE  = 2,

  //
  // Parameters for routing directions:
  //
  NUM_ROUTE_DIRECTIONS = 9, // Number of routing directions, listed below:
                            //                Bit-fields: | Up   Dn | N    S    E    W  | NE   SE   SW   NW |NxNE ExNE ExSE SxSE|SxSW WxSW WxNW NxNW|
                            //                            |---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|
  ANY         = 0x03FFFF,   // Any direction is allowed   | 1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  |
  NONE        = 0x000000,   // No direction is allowed    | 0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
  ANY_LATERAL = 0x00FFFF,   // Any lateral direction      | 0    0  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  |
  MANHATTAN   = 0x03F000,   // Manhattan & up/down        | 1    1  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
  X_ROUTING   = 0x030F00,   // X-route/up/down            | 1    1  | 0    0    0    0  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  |
  NORTH_SOUTH = 0x03C000,   // North/south/up/down        | 1    1  | 1    1    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
  EAST_WEST   = 0x033000,   // East/west/up/down          | 1    1  | 0    0    1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
  MANHATTAN_X = 0x03FF00,   // Manhattan/X-route/up/down  | 1    1  | 1    1    1    1  | 1    1    1    1  | 0    0    0    0  | 0    0    0    0  |
  UP_DOWN     = 0x030000,   // Up/down only               | 1    1  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |

  //
  // Parameters for transitions in the A* algorithm from the parent cell to a child cell:
  //
  E    = 0,
  N    = 1,
  W    = 2,
  S    = 3,
  NE   = 4,
  SE   = 5,
  NW   = 6,
  SW   = 7,
  NxNE = 8,
  ExNE = 9,
  ExSE = 10,
  SxSE = 11,
  SxSW = 12,
  WxSW = 13,
  WxNW = 14,
  NxNW = 15,
  Up   = 16,
  Down = 17,

  // Constants for coding the locations of terminals in function makePngPathMaps():
  START_TERM  = 1,   // start-terminal of a non-pseudo-net
  END_TERM    = 2,   // end-terminal of a non-pseudo-net
  PSEUDO_TERM = 3,   // start- or end-terminal of a pseudo-net

  NUM_CONG_SENSITIVITES = 11, // Number of congestion sensitivity percentages: 100, 141, 200, 283, 400, 566, 800, 1131, 1600, 2263, and 3200.

  NUM_PSEUDO_VIA_CONGESTION_STATES = 2, // Number of states for applying (or not applying) extra
                                        // TRACE congestion near crowded pseudo-vias.

  // Define constants for comparing routing metrics:
  WORSE = -1,
  EQUIVALENT = 0,
  BETTER = 1,

  // Define constants that describe the wiring configuration of diff-pair connections:
  NOT_SWAPPED = 0,
  SWAPPED     = 1,

  // Define Boolean values 'TRUE' and 'FALSE':
  TRUE  = 1,
  FALSE = 0

};  // End of enumerated constants


//
//  Define structure that holds an X/Y/Z coordinate in the map:
//
typedef struct Coordinate_t  {
  unsigned int X    : 13;  // X-coorinate, from 0 to 8191 cells
  unsigned int Y    : 13;  // Y-coorinate, from 0 to 8191 cells
  unsigned int Z    :  5;  // Z-coorinate, from 0 to 31 routing layers
  unsigned int flag :  1;  // Bit used as a Boolean flag for various purposes.
} Coordinate_t;  // End of struct definition 'Coordinate_t'


//
// Declare data structure for path-finding arrays:
//
typedef struct PathFinding_t  {
  unsigned char ***whichList;  // 3D array that specifies which path-finding list a cell is in.
                               // All elements are initialized to 'notOpenOrClosedList'.
                               //   whichList[x][y][z] = notOpenOrClosedList
                               //                         or onOpenList
                               //                          or onClosedList
  Coordinate_t *openListCoords;  // 1D array that stores the X/Y/Z coordinates of an
                                 // item on the Open List.
  Coordinate_t ***parentCoords;  // 3D array that contains the parent locations for each
                                 // cell in the 3D matrix.
                                 //     parentCoords[x][y][z] = coordinates of parent cell
  unsigned long *Fcost;   // 1D array that stores the F-cost of a cell on the Open list.
  unsigned long ***Gcost; // 3D array that stores the G-cost of each cell in the 3D matrix.
  unsigned long *Hcost;   // 1D array that stores the H-cost of a cell on the open list.
  int *openList;          // 1D array that contains the ID number of open-list items
  int ***sortNumber;  // 3D array that stores the current sort sequence (from binary heap)
                      // for each (x,y,z) cell on the Open List. This array helps the A*
                      // algorithm quickly find the sort number for an arbitrary cell at
                      // (x,y,z) in the map. This lookup feature can reduce the run-time
                      // significantly when the Open List is very large.
} PathFinding_t;



//
// Declare data structure for recording routing metrics associated with
// each value of the dynamic routing conditions, such as evaporation rate
// or the addition of TRACE congestion around crowded pseudo-vias:
//
typedef struct DynamicAlgorithmMetrics_t  {

  unsigned short iterationOfMeasuredMetrics; // Iteration number when the routing metrics were last measured. Zero if never measured.
                                             //   evapRateMetrics[dynamic_state_index].iterationOfMeasuredMetrics
                                             //          = iteration of last measurement for dynamic_state_index

  unsigned int dynamicParameter;   // Dynamic state value. For example, the congestion sensitivities would be:
                                   //   congSensitivity[0].dynamicParameter = 100,
                                   //   congSensitivity[1].dynamicParameter = 141, ...,
                                   //   congSensitivity[10].dynamicParameter = 3200

  float fractionIterationsWithoutDRCs; // Average DRC-free iteration rates that Acorn measures for each dynamic state.
                                       //   evapRateMetrics[dynamic_state_index].fractionIterationsWithoutDRCs
                                       //          = fraction of iterations without DRCs for dynamic_state_index

  float avgNonPseudoNetsWithDRCs;      // Average number of non-pseudo nets without DRCs that Acorn measures for each dynamic state.
                                       //   evapRateMetrics[dynamic_state_index].avgNonPseudoNetsWithDRCs
                                       //          = average number of nets with DRCs for dynamic_state_index

  float stdErrNonPseudoNetsWithDRCs;   // Standard error in the number of non-pseudo nets without DRCs that Acorn measures
                                       // for each dynamic state.
                                       //   evapRateMetrics[dynamic_state_index].stdErrNonPseudoNetsWithDRCs
                                       //          = std. error in the number of nets with DRCs for dynamic_state_index

  float avgNonPseudoRoutingCost;       // Average cost of non-pseudo routing that Acorn measures for each dynamic state.
                                       //   evapRateMetrics[dynamic_state_index].avgNonPseudoRoutingCost
                                       //          = average routing cost for dynamic_state_index

  float stdErrNonPseudoRoutingCost;    // Standard error in the cost of non-pseudo routing that Acorn measures for each
                                       // dynamic state.
                                       //   evapRateMetrics[dynamic_state_index].stdErrNonPseudoRoutingCost
                                       //          = std. error in the routing cost for dynamic_state_index

} DynamicAlgorithmMetrics_t;


//
// Declare data structure for routing metrics:
//
typedef struct RoutingMetrics_t  {
  int num_nonPseudo_DRC_cells;  // Number of cells with design-rule violations in the entire map, excluding pseudo-DRCs
  int num_pseudo_DRC_cells; // Number of cells with pseudo-design-rule violations in entire map.
  int total_num_DRC_cells;  // Number of cells with design-rule violations in entire map, including pseudo- and non-pseudo-nets.
  int num_DRCfree_paths;    // Number of user-defined paths that have no design-rule violations. (Pseudo-paths not counted.)
  int num_paths_with_DRCs;  // Number of user-defined paths that have design-rule violations. (Pseudo-paths not counted.)

  float *nonPseudoPathLengths;       // Total non-pseudo path length for each iteration. nonPseudoPathLengths[iteration_num] = aggregate path length
  int *nonPseudo_num_DRC_cells;      // Number of cells with non-pseudo-design-rule violations for each iteration. nonPseudo_num_DRC_cells[iteration_num] = number of DRC cells

  int *nonPseudo_num_via2via_DRC_cells;     // Number cells with non-pseudo-design-rule violations between vias for each iteration.
                                            //   nonPseudo_num_via2via_DRC_cells[iteration_num] = number of DRC cells with via-to-via spacing violations
  int *nonPseudo_num_trace2trace_DRC_cells; // Number cells with non-pseudo-design-rule violations between traces for each iteration.
                                            //   nonPseudo_num_trace2trace_DRC_cells[iteration_num] = number of DRC cells with trace-to-trace spacing violations
  int *nonPseudo_num_trace2via_DRC_cells;   // Number cells with non-pseudo-design-rule violations between traces and vias for each iteration.
                                            //   nonPseudo_num_trace2via_DRC_cells[iteration_num] = number of DRC cells with trace-to-via spacing violations

  int *nonPseudoViaCounts;           // Total non-pseudo via count for each iteration. nonPseudoViaCounts[iteration_num] = number of vias
  unsigned long *nonPseudoPathCosts; // Total non-pseudo path cost for each iteration. nonPseudoPathCosts[iteration_num] = aggregate path cost
  int *numNonPseudoDRCnets;          // Number of non-pseudo paths with DRCs for each iteration. numNonPseudoDRCnets[iteration_num] = number of nets with DRCs

  double *nonPseudoPathCosts_stdDev_trailing_10_iterations;  // Standard deviation of the non-pseudo path costs from the most recent 10 iterations
                                                             //  nonPseudoPathCosts_stdDev_trailing_10_iterations[iteration_num] = standard deviation
  double *nonPseudoPathCosts_slope_trailing_10_iterations;   // Slope of the the non-pseudo path costs from the most recent 10 iterations
                                                             //  nonPseudoPathCosts_slope_trailing_10_iterations[iteration_num] = slope
  unsigned char *inMetricsPlateau;           // Boolean value that specifies whether Acorn's routing metrics are currently in a plateau for a given iteration.
                                             //    inMetricsPlateau[iteration_num] = TRUE or FALSE
  unsigned char *swapStartAndEndTerms;       // Boolean value that specifies whether Acorn swaps the start/end-terminals of nets with DRCs.
                                             //    swapStartAndEndTerms[iteration_num] = TRUE or FALSE
  unsigned char *changeViaCongSensitivity;   // Boolean value that specifies whether Acorn changes the congestion sensitivity for vias
                                             // ('viaCongestionMultiplier').   changeViaCongSensitivity[iteration_num] = TRUE or FALSE
  unsigned char *changeTraceCongSensitivity; // Boolean value that specifies whether Acorn changes the congestion sensitivity for traces
                                             // ('traceCongestionMultiplier').   changeTraceCongSensitivity[iteration_num] = TRUE or FALSE
  unsigned char *enablePseudoTraceCongestion;  // Boolean value that specifies whether Acorn enables deposition of TRACE pseudo-congestion on more paths/layers
                                               // around pseudo-vias for nets with DRCs. enablePseudoTraceCongestion[iteration_num] = TRUE or FALSE
  int *cumulative_DRCfree_iterations; // Cumulative number of DRC-free iterations.
                                      //    cumulative_DRCfree_iterations[iteration_num] = cumulative number of DRC-free iterations


  unsigned long int total_cost;           // Total cost of all nets for an iteration, measured in units defined within map
  unsigned long int total_pseudo_cost;    // Total cost of all pseudo-nets for an iteration, measured in units defined within map
  unsigned long int total_nonPseudo_cost; // Total cost of all non-pseudo-nets for an iteration, measured in units defined within map
  int total_vias;            // Total number of vias in all nets for an iteration
  int total_pseudo_vias;     // Total number of vias in pseudo-nets for an iteration
  int total_nonPseudo_vias;  // Total number of vias in non-pseudo-nets for an iteration
  float total_lateral_length_mm;  // Total lateral length of all paths for an iteration, measured in millimeters
  float total_lateral_pseudo_length_mm;  // Total lateral length of pseudo-paths for an iteration, measured in millimeters
  float total_lateral_nonPseudo_length_mm;  // Total lateral length of non-pseudo-paths for an iteration, measured in millimeters
  unsigned long int *path_cost;        // 1D array containing length of each path, measured in units defined within map
  int *num_adjacent_steps; // 1D array containing number of adjacent steps in each path
  int *num_diagonal_steps; // 1D array containing number of diagonal steps in each path
  int *num_knights_steps;  // 1D array containing number of "knights" steps in each path (e.g., 2 up, 1 over)
  float *lateral_path_lengths_mm; // 1D array containing lateral length of each path, measured in millimeters
  int *path_DRC_cells;        // 1D array containing number of DRC cells for each path, excluding pseudo-DRCs
  int *layer_DRC_cells;       // 1D array containing number of DRC cells for each layer, excluding pseudo-DRCs
  int **path_DRC_cells_by_layer;  // 2D array containing number of DRC cells for each path and each routing layer
                                  // in latest iteration, excluding pseudo-DRCs.
                                  //   path_DRC_cells_by_layer[path_num][layer_num] = number of DRC cells for
                                  //                                                  path 'path_num' on routing
                                  //                                                  layer number 'layer_num'

  uint32_t **recent_DRC_flags_by_pseudoPath_layer;  // Pseudo 3D array containing Boolean flags to indicate whether
                                                    // each pseudo-path and each routing layer contains any DRCs
                                                    // of the most recent 32 iterations. The flags for the most recent
                                                    // 32 iterations are encoded as bits in the 32-bit element, with
                                                    // the most recent iteration encoded in the least-significant bit.
                                                    //   recent_DRC_flags_by_pseudoPath_layer[pseudoPath - numPaths][layerNum]
                                                    //                                        ^^^^^^^^^^^^^^^^^^^^^
                                                    //     Subtract numPaths from pseudo-path number to calculate zero-based index

  unsigned int **recent_path_DRC_cells;  // recent_path_DRC_cells[path][i] = number of DRC cells in path 'path' in the ith iteration
                                         // prior to the current iteration. The maximum value of i is numIterationsToReEquilibrate.
  float *recent_path_DRC_fraction;  // Fraction from 0 to 1.0 of the DRC cells that are in a given path over the recent iterations.
                                    //    recent_path_DRC_fraction[path] = (# of DRC cells in 'path')/(# of DRC cells in all paths)
  int *recent_path_DRC_iterations; // Number of recent iterations for which a path contained *any* DRC cells.
                                   //     recent_path_DRC_iterations[path] = number of iterations with any DRC violations within the
                                   //                                        last 'numIterationsToReEquilibrate' iterations
  float *fractionRecentIterationsWithoutPathDRCs; // Ratio (from 0 to 1) of recent iterations that had no DRC violations for a given
                                                  // path. For paths whose previous iterations had no design-rule violations, this
                                                  // ratio will be 1.  For paths whose previous iterations all had violations, this
                                                  // ratio will be 0. We multiply the A* heuristic by this ratio.
                                                  //   fractionRecentIterationsWithoutPathDRCs[path_num]
                                                  //      = 1.0 - (routability->recent_path_DRC_iterations[pathNum]) / min(numIterationsToReEquilibrate, pathFinderRun)
  float fractionRecentIterationsWithoutMapDRCs;   // Ratio (from 0 to 1) of recent iterations that had no DRC violations on any
                                                  // path in the map. We use this ratio to multiply the A* heuristic by. If no
                                                  // user-defined paths had recent design-rule violations, this ratio is 1. If
                                                  // every recent iteration had at least 1 violation (for any path), this ratio is 0.
                                                  //   fractionRecentIterationsWithoutMapDRCs
                                                  //      = 1.0 - (# of recent iterations that had any DRC) / min(numIterationsToReEquilibrate, pathFinderRun)
  unsigned char *randomize_congestion; // 1D array containing Boolean flags to tell findPath() whether to treat the congestion-related
                                       // G-cost differently for paths that had DRC violations. randomize_congestion[path] can take 3 values:
                                       //   0 = FALSE = do not change the congestion-related G-cost
                                       //   1 = DECREASE = decrease the congestion-related G-cost
                                       //   2 = INCREASE = increase the congestion-related G-cost
  unsigned short *one_path_traversal; // Quantity of congestion to deposit from a given path into a traversed cell after each iteration.
                                      // Value depends on the path. one_path_traversal[path_num] = ONE_TRAVERSAL * (path-specific fraction)
  int *num_vias;         // 1D array containing number of vias in each path 
  int **crossing_matrix; // 2D matrix describing which paths have DRC violations with other paths.
                         // Zero denotes no DRC violation. 1 denotes a single cell with a DRC violation, etc.
  int *path_elapsed_time;  // path_elapsed_time[i] is number of elapsed (wall-clock) seconds to find the path #i during most recent iteration.
                           // This time includes only the time in function 'findPath', and not any DRC checking or map-drawing.
  int *iteration_elapsed_time; // Number of elapsed (wall-clock) seconds to find all paths within iteration #i,
                               // including DRC-checking, but excluding the creation of PNG map-files.
  int total_elapsed_time;      // Number of elapsed (wall-clock) seconds before the job found a
                               // solution, or gave up trying. This is the sum of 'iteration_elapsed_time'
  unsigned long int *path_explored_cells;      // path_explored_cells[i] is number of cells explored to find path #i in most recent iteration.
  unsigned long int *iteration_explored_cells; // Number of cells explored to find all paths in iteration #j
                                               // (sum of path_explored_cells).
  unsigned long int total_explored_cells;      // Number of cells explored to find all paths across all iterations
                                               // (sum of iteration_explored_cells).
  unsigned short best_iteration;  // The iteration number that has the 'best' routing metrics. This iteration is the one with (a) the
                                  // lowest number of DRC cells, or (b) if multiple DRC-freeiterations exist, the number of the DRC-free
                                  // iteration with the lowest routing cost (including lateral path cost, via cost, including the effects
                                  // of cost-zones.

  unsigned short latestAlgorithmChange; // The most recent iteration number for which the routing algorithm was changed. Such
                                        // changes include (a) swapping start-/end-terminals, (b) changing the evaporation
                                        // rate, (c) starting to apply TRACE congestion near crowded pseudo-vias, or (d) increasing
                                        // the congestion multiplier.

  unsigned short num_startEnd_terminal_swaps; // The number of times the autorouter has swapped the start/end-terminals
                                              // of nets that contain design-rule violations (in order to improve routing).

  unsigned short num_viaCongSensitivity_changes; // The number of times the autorouter has changed the via congestion
                                                 // sensitivity in order to improve the routing.

  unsigned short num_viaCongSensitivity_reductions; // The number of times the autorouter has reduced the via
                                                    // congestion sensitivity in order to improve the routing.

  unsigned short num_viaCongSensitivity_stableRoutingMetrics;  // The number of times the autorouter found stable routing
                                                               // metrics when comparing the current via congestion
                                                               // sensitivity to higher (and sometimes lower)
                                                               // sensitivities.

  unsigned short num_traceCongSensitivity_changes; // The number of times the autorouter has changed the trace congestion
                                                   // sensitivity in order to improve the routing.

  unsigned short num_traceCongSensitivity_reductions; // The number of times the autorouter has reduced the trace
                                                      // congestion sensitivity in order to improve the routing.

  unsigned short num_traceCongSensitivity_stableRoutingMetrics;  // The number of times the autorouter found stable routing
                                                                 // metrics when comparing the current trace congestion
                                                                 // sensitivity to higher (and sometimes lower)
                                                                 // sensitivities.

  unsigned short DRC_free_threshold_achieved; // The iteration at which the auto-router achieved the necessary number
                                              // of DRC-free iterations ['DRC_free_threshold' in main()].

  // Arrays of routing metrics for each level of the congestion sensitivities:
  DynamicAlgorithmMetrics_t traceCongSensitivityMetrics[NUM_CONG_SENSITIVITES];
  DynamicAlgorithmMetrics_t viaCongSensitivityMetrics[NUM_CONG_SENSITIVITES];

} RoutingMetrics_t ;  // End of struct 'RoutingMetrics_t'

//
// Define data structure that contains details of design-rule violations:
//
typedef struct DRC_details_t  {
  int x;                        // X-location of DRC violation
  int y;                        // Y-location of DRC violation
  int z;                        // Z-location of DRC violation
  int pathNum;                  // Path-number at location (x,y,z)
  int shapeType;                // Shape-type at location (x,y,z)
  int offendingPathNum;         // Path-number of offending net
  int offendingShapeType;       // Shape-type of offending net
  float minimumAllowedDistance; // Minimum allowed separation between edge of shape-type 'shapeType'
                                // and center of shape-type 'offendingShapeType' for the design-rule
                                // set appropriate at location (x,y,z), in units of microns.
  float minimumAllowedSpacing;  // Minimum allowed spacing between edges of shape-type 'shapeType'
                                // and center of shape-type 'offendingShapeType' for the design-rule
                                // set appropriate at location (x,y,z), in units of microns.

} DRC_details_t ;  // End of struct 'DRC_details_t'


//
// Define data structure that contains design rules for a given design-rule subset:
//
typedef struct DesignRuleSubset_t  {

  // Name of design-rule subset.
  char *subsetName;

  // Design rules in microns, as provided by user:
  float lineWidthMicrons;               // Line width in microns
  float viaUpDiameterMicrons;           // Diameter of upward-going via, in microns
  float viaDownDiameterMicrons;         // Diameter of downward-going via, in microns
  float lineSpacingMicrons;             // Trace-to-trace spacing in microns
  float viaUpToTraceSpacingMicrons;     // Spacing between upward-going via and adjacent traces
  float viaDownToTraceSpacingMicrons;   // Spacing between downward-going via and adjacent traces
  float viaUpToViaUpSpacingMicrons;     // Spacing between adjacent, upward-going vias
  float viaDownToViaDownSpacingMicrons; // Spacing between adjacent, downward-going vias
  float viaUpToViaDownSpacingMicrons;   // Spacing between adjacent up- and down-ward going vias

  // Design rules that are specific to differential pairs:
  unsigned char  isDiffPairSubset;  // Boolean flag to specify if subset is dedicated to differential pairs
  unsigned char  isPseudoNetSubset; // Boolean flag to specify if subset is used for routing and
                                    // design-rule-checking pseudo-nets, which Acorn uses as proxies
                                    // for diff-pair nets
  float traceDiffPairPitchMicrons;            // Pitch of differential pairs in microns, as provided by user
  float diffPairPitchCells[NUM_SHAPE_TYPES];  // Pitch of differential pairs, in cells, for each shape-type
  // Following are used for calculating design-rules of pseudo-nets:
  float copy_lineWidthMicrons;       // Copy of lineWidthMicrons
  float copy_viaUpDiameterMicrons;   // Copy of viaUpDiameterMicrons
  float copy_viaDownDiameterMicrons; // Copy of viaDownDiameterMicrons

  // The following 2 arrays are redundant with the above values, but these
  // arrays simplify subsequent calculations:
  float width_um[NUM_SHAPE_TYPES];                  // Linewidth/diameter of shapes in microns:
                                                    //   width_um[TRACE]    = trace linewidth
                                                    //   width_um[VIA_UP]   = diameter of upward-via
                                                    //   width_um[VIA_DOWN] = diameter of downward-via
  float space_um[NUM_SHAPE_TYPES][NUM_SHAPE_TYPES]; // Minimum spacing between adjacent shapes, in microns:
                                                    //   space_um[TRACE][TRACE] = trace-to-trace spacing
                                                    //   space_um[TRACE][VIA_UP] = trace-to-upVia spacing
                                                    //      etc, etc.

  // Design rules converted to number of cells:
  float radius[NUM_SHAPE_TYPES];    // = radius (or half-width) (in cells) of trace (radius[0]),
                                    //   via-up (radius[1]), and via-down (radius[2]).
  float spacing[NUM_SHAPE_TYPES][NUM_SHAPE_TYPES];  // = inter-shape spacing (in cells) among (1) traces,
                                                    //   (2) upward vias, and (3) downward vias.

  // Derived values that are frequently used in path-finding operations:
  float radius_squared[NUM_SHAPE_TYPES]; // = 0.25 * lineWidthMicrons^2 (or via diameter) / (microns per cell)^2

  int routeDirections;  // = allowed routing directions. The values for this integer are derived from the 18 last-significant bits, as follows:
                        //      Bit-fields: | Up   Dn | N    S    E    W  | NE   SE   SW   NW |NxNE ExNE ExSE SxSE|SxSW WxSW WxNW NxNW|
                        //                  |---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|---- ---- ---- ----|
                        // Any direction:   | 1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  | 1    1    1    1  |
                        // No direction:    | 0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  | 0    0    0    0  |
                        // Et cetra, et cetra.

} DesignRuleSubset_t;  // End of struct 'DesignRuleSubset_t'


//
// Define data structure that contains values parsed from user's input file
//
typedef struct InputValues_t  {
  // General input parameters:
  float cell_size_um;  // Dimension in microns of the grid that the auto-router will use
  float map_width_mm;  // Map width in millimeters
  float map_height_mm; // Map height in millimeters
  int   num_routing_layers;    // Number of routing layers (excluding via layers)
  char *layer_names[2*maxRoutingLayers-1]; 
                       // Array of user-defined names for routing layers and intervening 
                       // vias. Number of elements is (2*num_routing_layers - 1).
  char *routingLayerNames[maxRoutingLayers]; // Array of user-defined names for routing layers

  char *origin;        // Location of origin: center, lower_left, upper_left, lower_right, or upper_right

  // Netlist parameters:
  int   num_nets;      // Number of nets specified by user.
  int   num_diff_pair_nets;   // Number of nets that are part of a differential pair.
  int   num_pseudo_nets;      // Number of pseudo nets to be routed: 1 pseudo net for each differential pair of nets.
  int   num_special_nets;     // Number of nets that have net-specific design rules, excluding diff-pair nets.
  char **net_name;    // Array of user-defined net names: net_name[net_number] = character string
  char **start_layer; // Array of routing layer names that each net starts at, e.g., start_layer[net_number] = 'M1' or 'M4', etc
  char **end_layer;   // Array of routing layer names that each net ends at, e.g., end_layer[net_number] = 'M1' or 'M4', etc)
  float *start_X_um;  // Array of starting x-positions (in microns): start_X_um[net_number] = X-position in microns
  float *start_Y_um;  // Array of starting y-positions (in microns): start_Y_um[net_number] = Y-position in microns
  float *end_X_um;    // Array of ending x-positions (in microns): end_X_um[net_number] = X-position in microns
  float *end_Y_um;    // Array of ending y-positions (in microns): end_Y_um[net_number] = Y-position in microns
  float *rats_nest_length_um;  // Array of lateral distances (in microns) between the start- and end-terminals for
                               // each path: rats_nest_length_um[net_number] = distance in microns
  float avg_rats_nest_length_um; // Average of all rat's nest lengths, in microns.

  // Parameters for differential pairs:
  unsigned char *isDiffPair;    // Array of Boolean flags indicating whether net is part of diff pair: isDiffPair[net_number] = TRUE or FALSE
  unsigned char *isPNswappable; // Array of Boolean flags indicating whether net is part of a diff-pair whose terminals may
                                // be swapped to improve routing: isPNswappable[net_number] = TRUE or FALSE
  unsigned char *isPseudoNet;   // Array of Boolean flags indicating whether net is a pseudo net for diff pairs: isPseudoNet[net_number] = TRUE or FALSE
  short int  *diffPairPartner; // Array of integers specifying the partner net number of a diff pair net: diffPairPartner[net_number] = net number of partner net
  char      **diffPairPartnerName; // diffPairPartnerName[i] is net name of diff-pair partner to net #i
  float     **diffPairPitchCells; // diffPairPitchCells[i][j] = pitch (in cells) for net #i in design-rule set #j
  float     **diffPairPitchMicrons; // diffPairPitchMicrons[i][j] = pitch (in microns) for net #i in design-rule set #j
  short int  *diffPairToPseudoNetMap; // Array of integers that maps diff-pair net numbers to the pseudo-net
                                      // used for routing the pair: diffPairToPseudoNetMap[net_number] = pseudo_net_number.
  short int  *pseudoNetToDiffPair_1;  // Arrays of integers that maps pseudo-nets to net #1 and net #2
  short int  *pseudoNetToDiffPair_2;  // of the original diff-pair nets.
  float      *diffPairStartTermPitchMicrons; // diffPairStartTermPitchMicrons[i] = Pitch (in um) of the starting terminals of net #i.
  float      *diffPairEndTermPitchMicrons;   // diffPairEndTermPitchMicrons[i]   = Pitch (in um) of the ending terminals of net #i.
  short int  *diffPairStartTermPitch; // diffPairStartTermPitch[i] = Pitch (in cells) of the starting terminals of net #i.
  short int  *diffPairEndTermPitch;   // diffPairEndTermPitch[i]   = Pitch (in cells) of the ending terminals of net #i.

  // Parameters for nets with special design rules:
  unsigned char *usesSpecialRule; // Array of Boolean flags indicating whether net has a net-specific design rule:
                                  // usesSpecialRule[i] = TRUE or FALSE for net #i. Array is dimensioned to include pseudo nets.
  char **netSpecificRuleName;     // netSpecificRuleName[i] is name of net-specific rule for net #i

  // Keep-out zone parameters:
  int  num_block_instructions;  // Number of BLOCK/UNBLOCK instructions in input file
  char  **block_command;    // block_command[i] = 'BLOCK' or 'UNBLOCK' for BLOCK instruction #i
  char  **block_type;       // block_type[i] = block type ('RECT', 'TRI', 'CIR', etc) for BLOCK instruction #i
  char  **block_layer;      // block_layer[i] = layer_name  (e.g., M1, M2, M3, ...) for BLOCK instruction #i
  unsigned char *block_num_params; // block_num_params[i] = number of parameters for BLOCK/UNBLOCK statement #i (0 to 6)
  float **block_parameters; // block_parameters[i][j] = floating-point parameter #j in BLOCK instruction #i

  // Parameters that affect the solution and run-time. 
  int maxIterations;  // Maximum number of iterations the program will make to find a DRC-free solution.
  int userDRCfreeThreshold; // Minimum number of DRC-free solutions that program will find before
                            // deciding that it has found the optimal solution. Program may add to
                            // this threshold, based on the complexity of the design.
  int runsPerPngMap; // Number of iterations to run between writing PNG versions of the routing maps.



  float baseVertCostMicrons; // Base-cost of routing up or down to a different routing layer, i.e., the 
                             // cost of a via expressed as an equivalent trace length, in microns.
  unsigned int baseVertCostCells;  // Base-cost of routing up or down to a different routing layer, i.e., the
                                   // cost of a via expressed as an equivalent trace length, in units of cells.
  long unsigned baseVertCost; // Base-cost of routing up or down to a different routing layer, i.e., the cost of a via
                              // expressed as an equivalent trace length, in same units of cost as lateral routing.
  long unsigned pinSwapVertCost;  // Routing cost of moving vertically by 1 layer in a pin-swap zone.
  long unsigned baseCellCost;    // Routing cost to move to an adjacent cell in the north, south, east, or west directions.
  long unsigned pinSwapCellCost; // Routing cost to move to an adjacent cell in the north, south, east, or west directions in pin-swap zone.
  long unsigned baseDiagCost;   // Routing cost of moving to a diagonal cell (NW, NE, SW, or SE).
  long unsigned pinSwapDiagCost;   // Routing cost of moving to a diagonal cell (NW, NE, SW, or SE) in pin-swap zone.
  long unsigned baseKnightCost; // Routing cost of moving 2 cells north, south, east, or west, followed
                                // by 1 cell sideways (like a knight moves in the game of chess).
  long unsigned pinSwapKnightCost; // Routing cost of moving 2 cells north, south, east, or west, followed
                                  // by 1 cell sideways (like a knight moves in the game of chess) in pin-swap zone.




  // The following parameter affects the evaporation of congestion:
  int preEvaporationIterations; // Number of iterations before beginning evaporation of congestion.
                                // The minimum value is 2.


  // Design-rule parameters:
  int numDesignRuleSets;                 // Number of design-rule sets
  char *designRuleSetName[maxDesignRuleSets];  // Name of each design-rule set
  char *designRuleSetDescription[maxDesignRuleSets];  // Description of each design-rule set
  int numDesignRuleSubsets[maxDesignRuleSets];  // Number of net-specific subsets in each design-rule set
  unsigned char usedOnLayers[maxDesignRuleSets][maxRoutingLayers]; // 0 if this design-rule set is not used on a layer.
                                                                   // 1 if this design-rule set is used on a layer.
                                                                   // 2 if this design-rule set is used on a layer, but
                                                                   // conflicts with design-rules on above or below layer.
  unsigned char *designRuleUsed;  // 1-dimensional array that specifies whether a given design-rule set is used anywhere
                                  // in the map: user_inputs->designRuleUsed[DR_num] = 1 (TRUE) or 0 (FALSE)
  unsigned char **DR_subsetUsed;  // 2-dimensional array that specifies whether a given design-rule subset is used anywhere
                                  // in the map: user_inputs->DR_subsetUsed[DR_num][DR_subset_num] = 1 (TRUE) or 0 (FALSE)
  float maxInteractionRadiusCellsInDR[maxDesignRuleSets];    // = max of all linewidths/diameters, plus max of all spacing values (in cell units) within a design-rule set.
  float maxInteractionRadiusSquaredInDR[maxDesignRuleSets];  // = Square of 'maxInteractionRadiusCellsInDR', in units of cells squared, within a design-rule set.

  DesignRuleSubset_t **designRules;  // 2-dimensional array: input_value->designRules[i][j] points to DesignRuleSubset_t
                                     // structure for design-rule subset #j of design-rule set #i.

  unsigned char **designRuleSubsetMap;  // 2-dimensional array: input_value->designRuleSubsetMap[i][j] = DR subset number for net #i
                                        // and DR set #j. The first dimension (i) is dimensioned to include pseudo nets.

  int ***foreign_DR_subset;    // 3-dimensional array that maps design-rule subsets to each other based on their names:
                               //      user_inputs->foreign_DR_subset[native_DR_set][native_DR_subset][foreign_DR_set] = foreign_DR_subset

  float ****cong_radius;       // 4-dimensional array: cong_radius[i][m][j][n] applies to design-rule sets #i and j, and provides the m-by-n matrix
                               // of congestion radii in cell units, where m ranges from 0 to NUM_SHAPE_TYPES * user_inputs->numDesignRuleSubsets[i],
                               // and n ranges from 0 to NUM_SHAPE_TYPES * user_inputs->numDesignRuleSubsets[j]. Each element
                               // at (m,n) equals (in cell units):  radius[n] + spacing[m][n] + radius[m]

  float ****cong_radius_squared; // 4-dimensional array, where elements are the square of those in 'cong_radius'.

  float ****DRC_radius;        // 4-dimensional array: DRC_radius[i][m][j][n] applies to design-rule sets #i and j, and provides the m-by-n matrix
                               // of DRC radii in cell units, where m and n = NUM_SHAPE_TYPES * user_inputs->numDesignRuleSubsets[]. Each element
                               // at (m,n) equals (in cell units):  radius[n] + spacing[m][n].

  float ****DRC_radius_squared;  // 4-dimensional array, where elements are the square of those in 'DRC_radius'.

  float ****detour_distance;   // 4-dimensional array: detour_distance[i][m][j][n] applies to design-rule sets #i and j, and
                               // provides the detour distance (in cell units) for routing subset/shapeType 'm' in the presence of
                               // congestion of subset/shapeType 'n'.


  // Design-rule zone parameters:
  int     num_DR_zones;       // Number of DR_zone instructions in input file
  char  **DR_zone_name;       // DR_zone_name[i] = name of design-rule set used for zone #i
  char  **DR_zone_layer;      // DR_zone_layer[i] = layer name for DR zone #i (e.g., M1, M2, M3, ...)
  char  **DR_zone_shape;      // DR_zone_shape[i] = shape ('RECT', 'TRI', 'CIR', etc) for DR zone #i
  unsigned char *DR_zone_num_params; // DR_zone_num_params[i] = number of parameters for DR_zone statement #i (0 to 6)
  float **DR_zone_parameters; // DR_zone_parameters[i]j] = floating-point parameter #j for DR zone #i

  // Trace-cost multipliers:
  int num_trace_cost_zone_instructions;  // Number of trace_cost_zone statements in the input file
  int num_via_cost_zone_instructions;    // Number of via_cost_zone statements in the input file

  unsigned char numTraceMultipliersInvoked; // Number of trace-cost multipliers that were invoked in trace_cost_zone statements
  unsigned char numTraceMultipliersUsed;    // Number of trace-cost multipliers that were actually used in the final map
  int traceCostMultiplier[maxTraceCostMultipliers]; // Cost multiplier for traces
  unsigned char traceCostMultiplierInvoked[maxTraceCostMultipliers]; // Flag whether this multiplier is invoked
                                                                     // in any trace_cost_zone statement
  unsigned char traceCostMultiplierUsed[maxTraceCostMultipliers]; // Flag whether this multiplier is actually
                                                                  // used anywhere in the map.
  unsigned char costUsedOnLayer[maxTraceCostMultipliers][2*maxRoutingLayers-1]; // costUsedOnLayer[i][j] is
                                                                                // Boolean flag indicating if
                                                                                // cost index i is used on
                                                                                // PNG layer j.
  unsigned long cellCost[maxTraceCostMultipliers];    // Product of baseCellCost and traceCostMultiplier
  unsigned long diagCost[maxTraceCostMultipliers];    // Product of baseDiagCost and traceCostMultiplier
  unsigned long knightCost[maxTraceCostMultipliers];  // Product of baseKnightCost and traceCostMultiplier

  // Via-cost multipliers:
  unsigned char numViaMultipliersInvoked;       // Number of via-cost multipliers that were invoked in via_cost_zone statements
  unsigned char numViaMultipliersUsed;          // Number of via-cost multipliers that were actually used in the final map
  int viaCostMultiplier[maxViaCostMultipliers]; // Cost multiplier for vias
  unsigned char viaCostMultiplierInvoked[maxViaCostMultipliers]; // Flag whether this multiplier is invoked
                                                                 // in any via_cost_zone statement
  unsigned char viaCostMultiplierUsed[maxViaCostMultipliers]; // Flag whether this multiplier is actually
                                                     // used anywhere in the map.
  unsigned long vertCost[maxViaCostMultipliers]; // Product of baseVertCost and viaCostMultiplier

  // Trace-cost multiplier zones:
  int num_trace_cost_zones;             // Number of trace_cost_zone instructions in input file
  unsigned char *trace_cost_zone_index; // trace_cost_zone_index[i] = index (0 to 15) of trace cost-multiplier used for trace-cost zone #i
  char **trace_cost_zone_layer;         // trace_cost_zone_layer[i] = layer name for trace_cost_zone #i (e.g., M1, M2, M3, ...)
  char **trace_cost_zone_shape;         // trace_cost_zone_shape[i] = shape name for trace_cost_zone #i ('RECT', 'TRI', 'CIR', etc)
  unsigned char *trace_cost_num_params; // trace_cost_num_params[i] = number of parameters for trace_cost_zone statement #i (0 to 6)
  float **trace_cost_zone_parameters;   // trace_cost_zone_parameters[i][j] = floating-point parameter #j for trace_cost_zone statement #i

  // Via-cost multiplier zones:
  int num_via_cost_zones;             // Number of via_cost_zone instructions in input file
  unsigned char *via_cost_zone_index; // via_cost_zone_index[i] = index (0 to 7) of via cost-multiplier used for via-cost zone #i
  char **via_cost_zone_layer;         // via_cost_zone_layer[i] = layer name for via_cost_zone #i (e.g., Via_1-2, Via_2-3, ...)
  char **via_cost_zone_shape;         // via_cost_zone_shape[i] = shape name for via_cost_zone #i ('RECT', 'TRI', 'CIR', etc)
  unsigned char *via_cost_num_params; // via_cost_num_params[i] = number of parameters for this via_cost_zone statement (0 to 6)
  float **via_cost_zone_parameters; // via_cost_zone_parameters[i][j] = floating-point parameter #j for via_cost_zone #i
  
  // Pin-swap zone parameters:
  int num_swap_instructions;  // Number of PIN_SWAP/NO_PIN_SWAP instructions in input file
  char **swap_command;  // swap_command[i] = 'PIN_SWAP' or 'NO_PIN_SWAP' for pin-swap instruction #i
  char **swap_shape;    // swap_shape[i] = shape for swap instruction #i ('RECT', 'TRI', 'CIR', etc)
  char **swap_layer;    // swap_layer[i] = layer_name for swap instruction #i (e.g., M1, M2, M3, ...)
  unsigned char *swap_num_params; // swap_num_params[i] = number of parameters for PIN_SWAP/NO_PIN_SWAP statement #i (0 to 6)
  float **swap_parameters; // swap_parameters[i][j] = floating-point parameter #j for pin-swap instruction #i

} InputValues_t ;  // End of struct 'InputValues_t'

//
// Define structure that holds congestion information for each cell in the map:
//
typedef struct Congestion_t  {
  unsigned int   pathTraversalsTimes100:  24 ; // pathTraversalsTimes100 values can range from 0 to 16,777,215
  unsigned int   pathNum:                 10 ; // PathNum values can range from 0 to 1023
  unsigned int   DR_subset:                4 ; // Design-rule subset can range from 0 to 15
  unsigned int   shapeType:                2 ; // 0=trace, 1=up-via, 2=down-via
} Congestion_t; // End of struct definition 'Congestion_t'


//
// Define structure for recording the path number
// and shape-type of paths at a given x/y/z location:
//
typedef struct PathAndShapeInfo_t  {
  unsigned pathNum     : 10  ; // Path number of traversing path (0 - 1023)
  unsigned shapeType   :  2  ; // 0=trace, 1=viaUp, 2=viaDown
} PathAndShapeInfo_t;


//
// Define structure that holds information about each individual cell in the map:
//
typedef struct CellInfo_t {

  // 8 bytes for following pointer (on 64-bit architecture):
  Congestion_t  *congestion; // Pointer to 1-dimensional array, with each
                             // element containing (1) pathTraversalsTimes100,
                             // (2) pathNum, (3) design-rule subset, and (4) shapeType.

  // 8 bytes for following pointer (on 64-bit architecture):
  PathAndShapeInfo_t *pathCenters ;   // Pointer to 1-dimensional array, with each
                                      // element containing (1) pathNum of 
                                      // traversing center-line, and (2)  the
                                      // shape-type of traversing shape (trace, up-via, down-via)



  // 6 bytes for the following 48-bit field:
  unsigned long forbiddenProximityBarrier : 48 ; // 48 bits that specify whether a cell is
                                                 // unwalkable due to proximity to the map's
                                                 // edge or a user-defined barrier.

  // 2 bytes for the following 12- and 4-bit fields:
  unsigned int numTraversingPaths         : 12 ; // 0 to 4095. This value
                                                // specifies how many paths and
                                                // shape-types traverse the cell,
                                                // including cells within a
                                                // half-linewidth of path's
                                                // center-line.

  unsigned int designRuleSet              :  4 ; // 0 to 15

  // 6 bytes for the following 48-bit field:
  unsigned long forbiddenProximityPinSwap : 48 ; // 48 bits that specify whether a cell is
                                                 // unwalkable due to proximity to a user-
                                                 // defined pin-swap zone.

  // 2 bytes for the following 12- and 4-bit fields:
  unsigned int   numTraversingPathCenters    : 12 ; // 0 to 4095. This value
                                              // specifies how many path
                                              // center-lines traverse this 
                                              // cell, including traces, up-vias,
                                              // and down-vias.

  unsigned int   traceCostMultiplierIndex   :  4 ; // 0 to 15

  // 1 byte for 8-bit 'swap_zone' field:
  unsigned char swap_zone;                            // Unique swap zone (1 to 255)

  // 24 bits (3 bytes) for remaining members:
  unsigned int viaUpCostMultiplierIndex        :  3 ; // 0 to 7

  _Bool        forbiddenTraceBarrier           :  1 ; // Unwalkable barrier for traces
  _Bool        forbiddenUpViaBarrier           :  1 ; // Unwalkable via above
  _Bool        forbiddenDownViaBarrier         :  1 ; // Unwalkable via below

  _Bool        routing_layer_metal_fill        :  1 ; // 1 if traversed by a trace, via-up, or via-down, excluding pseudo-nets
  _Bool        pseudo_routing_layer_metal_fill :  1 ; // 1 if traversed by a pseudo-net (trace or via)

  unsigned int viaDownCostMultiplierIndex      :  3 ; // 0 to 7

  _Bool        DRC_flag                        :  1 ; // 1 if cell represents a design-
                                                      // rule violation.
  _Bool        via_above_metal_fill            :  1 ; // 1 if the via layer above the cell is traversed
                                                      // by a via, excluding pseudo-vias
  _Bool        via_below_metal_fill            :  1 ; // 1 if the via layer below the cell is traversed
                                                      // by a via, excluding pseudo-vias
  _Bool        pseudo_via_above_metal_fill     :  1 ; // 1 if the via layer above the cell is traversed
                                                      // by a pseudo-via
  _Bool        pseudo_via_below_metal_fill     :  1 ; // 1 if the via layer below the cell is traversed
                                                      // by a pseudo-via
  _Bool        via_above_DRC_flag              :  1 ; // 1 if the via layer above the cell
                                                      // represents a design-rule violation.
                 
  _Bool        center_line_flag                :  1 ; // 1 if the cell is part of a (sparse)
                                                      // path. Flag is used to mark the center-
                                                      // line of path in PNG maps.

  _Bool        center_viaUp_flag               :  1 ; // 1 if the cell is at center of a VIA_UP
                                                      // path. Flag is used to mark the center-
                                                      // line of vias in PNG maps.

  _Bool        center_viaDown_flag             :  1 ; // 1 if the cell is at center of a VIA_DOWN
                                                      // path. Flag is used to mark the center-
                                                      // line of vias in PNG maps.

  _Bool        near_a_net                      :  1 ; // 1 if the cell is 'near' a net. 'Near'
                                                      // is defined as within 'maxInteractionRadiusCellsOnLayer'
                                                      // of the contiguous path's center-line.

  _Bool        explored                        :  1 ; // 1 if cell was explored during most recent A*
                                                      // path-finding iteration; zero otherwise.

  _Bool        explored_PP                     :  1 ; // 1 if cell was explored during Post-Processing (PP)
                                                      // of most recent iteration; zero otherwise.

  _Bool        flag                            :  1 ; // Flag bit used to temporarily mark which cells have been
                                                      // processed during certain operations.

} CellInfo_t;  // End of struct definition 'CellInfo_t'


//
//  Define structure that holds information about the overall map:
//
typedef struct MapInfo_t  {
  unsigned int mapHeight;        // Height of map, as measured in cells
  unsigned int mapWidth;         // Width of map, as measured in cells
  float mapDiagonal;             // Diagonal extent in map, as measured in cells
  unsigned int numLayers;        // Number of routing layers, excluding vias
  unsigned int numPaths;         // Number of non-pseudo paths to route (= number of nets).
  unsigned int numPseudoPaths;   // Number of pseudo paths to route (= half the number of diff-pair paths)
  Coordinate_t *start_cells;     // Array of starting x/y/z coordinates (in cell units): start_cells[net_number].X = X-position in cells
  Coordinate_t *end_cells;       // Array of ending x/y/z coordinates (in cell units): end_cells[net_number].X = X-position in cells
  float *diffPairStartTermPitchMicrons; // diffPairStartTermPitchMicrons[i] = Pitch (in um) of the starting terminals of net #i.
  float *diffPairEndTermPitchMicrons;   // diffPairEndTermPitchMicrons[i]   = Pitch (in um) of the ending terminals of net #i.
  short unsigned *swapZone;      // The pin-swap zone where the start-terminal is located. It is zero if net/path's start-terminal
                                 // is not located in a swap zone.  swapZone[net_number] = swap_zone_number
  unsigned char *diff_pair_terms_swapped; // Array of Boolean flags that specify whether Acorn has swapped the start-cells to optimize
                                          // routability.   diff_pair_terms_swapped[net_number] = TRUE (1) or FALSE (0).
  unsigned char *start_end_terms_swapped; // Array of Boolean flags that specify whether the start- and end-termainsl were swapped to
                                          // optimize path-finding. start_end_terms_swapped[net_number] = TRUE (1) or FALSE (0).
  unsigned char currentTraceCongSensIndex; // Index of the congestion sensitivity for trace routing, which is the percentage of the
                                           // nominal congestion multiplier to use when calculating congestion-related G-cost, and
                                           // can be modified dynamically by Acorn.
  unsigned char currentViaCongSensIndex;   // Index of the congestion sensitivity for via routing, which is the percentage of the
                                           // nominal congestion multiplier to use when calculating congestion-related G-cost, and
                                           // can be modified dynamically by Acorn.

  // 2D matrix of Boolean flags to specify which paths and which routing layers should have pseudo-TRACE congestion
  // deposited around pseudo-vias.   addPseudoTraceCongestionNearVias[path][layer] = TRUE or FALSE
  unsigned char **addPseudoTraceCongestionNearVias;

  float traceCongestionMultiplier;  // This value starts as the product:
                                    //   (defaultCellCost)*(currentEvaporationRate/100)/(1-currentEvaporationRate/100) / 100.
                                    // This value is used for calculating congestion-related G-costs in function findPath(), and
                                    // can be adjusted dynamically based on routing metrics.
  float viaCongestionMultiplier;    // This value starts as the product:
                                    //   (defaultCellCost)*(currentEvaporationRate/100)/(1-currentEvaporationRate/100) / 100.
                                    // This value is used for calculating congestion-related G-costs in function findPath(), and
                                    // can be adjusted dynamically based on routing metrics.
  int current_iteration;         // The number of the current iteration, starting with 1.
  int max_iterations;            // Maximum number of iterations allowed for the map.
  float maxInteractionRadiusCellsOnLayer[maxRoutingLayers];   // = max of all maxInteractionRadiusCellsInDR (in cell units) for all DR sets used on a layer.
  float maxInteractionRadiusSquaredOnLayer[maxRoutingLayers]; // = Square of 'maxInteractionRadiusCellsOnLayer', in units of cells squared, for a layer.
  float iterationDependentRatio; // This ratio will be initialized to 0.20 at the first iteration, and eventually reach 1.00 after an appropriate number
                                 // of iterations. It is used for slowly scaling up congestions and congestion sensitivities.

} MapInfo_t;  // End of struct definition 'MapInfo_t'


//
// Define structure that holds floating-point, two-dimensional vector:
//
typedef struct Vector2dFloat_t  {
  float X;  // X-coordinate of vector
  float Y;  // Y-coordinate of vector
} Vector2dFloat_t;  // End of struct definitionn 'Vector2dFloat_t'


//
// Define structure to hold certain input data for function findPath(),
// restricting the routing to a radius about a given X/Y coordinate
//
typedef struct RoutingRestriction_t  {
  unsigned int centerX; // X-coordinate of center-point for limiting routing.
  unsigned int centerY; // Y-coordinate of center-point for limiting routing.

  // Maximum radius (in microns) from X/Y position (centerX, centerY) that routing
  // is allowed. If value is zero, then allowed radius is infinite.
  float allowedRadiiMicrons[maxRoutingLayers];

  // Maximum radius (in cell units) from X/Y position (centerX, centerY) that
  // routing is allowed. If value is zero, then allowed radius is infinite.
  float allowedRadiiCells[maxRoutingLayers];

  unsigned char allowedLayers[maxRoutingLayers]; // Boolean flag that specifies whether routing
                                                 // is allowed on a given layer
  unsigned char restrictionFlag;  // Boolean value that specifies whether routing is restricted
                                  // on any layer.

} RoutingRestriction_t;  // End of struct definition 'RoutingRestriction_t'

//
// Define structure to describe a via-stack, which is used in multiple functions.
//
typedef struct ViaStack_t  {
  unsigned short pathNum;      // Path number that contains the via
  unsigned char  endShapeType; // Shape-type of the via's end-segment (either VIA_UP or VIA_DOWN)
  int startSegment;            // Segment number in the non-contiguous path where the
                               // via begins. If via begins at the start terminal, then
                               // this value is -1.
  int endSegment;           // Segment number where the via ends.
  Coordinate_t startCoord;  // (x,y,z) coordinate of start of via
  Coordinate_t endCoord;    // (x,y,z) coordinate of end of via
  unsigned char isVertical; // Boolean flag specifying whether all segments in the via-stack are
                            // vertically aligned.
  unsigned char error;      // Boolean flag specifying whether this structure does not contain
                            // a via because of an error in locating a via that met the constraints.
} ViaStack_t;  // End of struct definition 'ViaStack_t'

//
// Define structure to describe connections between a pair of diff-pair
// shoulder-paths and a pair of vias or terminals of these diff-pair paths.
// Note that this structure describes a single pair of connections, in
// contrast to structure 'ShoulderConnections_t' (plural), which describes
// an array of such connections.
//
typedef struct ShoulderConnection_t  {
  Coordinate_t startCoord_1;     // (x,y,z) coordinate at start of connection for path #1.
  Coordinate_t startCoord_2;     // (x,y,z) coordinate at start of connection for path #2.
  Coordinate_t endCoord_1;       // (x,y,z) coordinate at end of connection for path #1.
  Coordinate_t endCoord_2;       // (x,y,z) coordinate at end of connection for path #2.
  int startSegment_1; // Segment number of diff-pair path at start of connection for path #1.
                      // Value is -1 if the start-segment is the path's start-terminal.
  int startSegment_2; // Segment number of diff-pair path at start of connection for path #2.
                      // Value is -1 if the start-segment is the path's start-terminal.
  int endSegment_1;   // Segment number of diff-pair path at end of connection for path #1.
  int endSegment_2;   // Segment number of diff-pair path at end of connection for path #2.
  unsigned char startShapeType_1;  // Shape-time of the start-segment for path #1.
  unsigned char startShapeType_2;  // Shape-time of the start-segment for path #2.
  unsigned char endShapeType_1;    // Shape-time of the end-segment for path #1.
  unsigned char endShapeType_2;    // Shape-time of the end-segment for path #2.

  Coordinate_t minCoord;  // Minimum and maximum (x,y,z) coordinates to be used when running the
  Coordinate_t maxCoord;  // auto-router to connect diff-pair traces to vias and terminals.

  unsigned int optimizedConnectionLength_1[2];  // Length of optimized path #1 for unswapped (0)
                                                // and swapped (1) wiring configuration.
  unsigned int optimizedConnectionLength_2[2];  // Length of optimized path #2 for unswapped (0)
                                                // and swapped (1) wiring configuration.
  Coordinate_t *optimizedConnectionCoords_1[2]; // Pointers to array of optimized path coordinates
                                                // for path #1:
                                                // optimizedConnectionCoords_1[0][seg_num] = (x,y,z)
                                                //  coordinate of segment 'seg_num' for unswapped wire #1
  Coordinate_t *optimizedConnectionCoords_2[2]; // Pointers to array of optimized path coordinates
                                                // for path #2:
                                                // optimizedConnectionCoords_2[0][seg_num] = (x,y,z)
                                                //  coordinate of segment 'seg_num' for unswapped wire #2
  double symmetryRatio; // 'symmetryRatio' is <0.5 if existing connection has better routing metrics than swapping
                        // the connection, and >0.5 if swapping the connection would degrade the routing metrics.
  unsigned char swap;   // Boolean flag that specifies whether the connections should be swapped, i.e., connect
                        // start-segment from path #1 to end-segment of path #2.
  unsigned char DRC_free[2]; // Boolean flag that specifies whether DRC-free routing was found between diff-pair
                             // partner-paths in a given diff-pair connection for unswapped (0) and swapped (1)
                             // wiring configurations.
  unsigned char sameLayerTerminals; // Boolean flag that's true if (a) the connection's two start-terminals
                                    // are on the same layer, and (b) the connection's two end-terminals are
                                    // on the same layer. Only if this flag is TRUE will Acorn use the path-
                                    // finding results from sub-iterations to create low-cost paths in
                                    // the main map.
} ShoulderConnection_t;


//
// Define structure to describe the array of connections between diff-pair
// shoulder-paths and the vias or terminals along a diff-pair path. Note that
// this structure describes an array of such connections, in contrast to
// structure 'ShoulderConnection_t' (singular), which describes only
// a pair of such connections.
//
typedef struct ShoulderConnections_t  {
  ShoulderConnection_t *connection;  // Pointer to array of connections -- one for each connection between
                                     // shoulder-paths and diff-pair vias/terminals.
  unsigned short pseudoPath;      // The path number of the pseudo-path.
  unsigned short diffPairPath_1;  // The path number of diff-pair path #1.
  unsigned short diffPairPath_2;  // The path number of diff-pair path #2.
  unsigned short numPseudoVias;   // The number of pseudo-vias along the pseudo-path.
  unsigned short numConnections;  // The sum of the number of trace-to-terminal and trace-to-via
                                  // connections. This value is 2  +  2*numPsuedoVias.
  unsigned short numSwaps;        // Number of connections that should be swapped to optimize
                                  // connectivity while respecting P/N-swappability status.
  unsigned char PN_swappable;     // Boolean flag specifying whether the user defined the
                                  // diff-pair as P/N-swappable.
} ShoulderConnections_t;


//-----------------------------------------------------------------------------
// Name: copyCoordinates
// Desc: Copy the (x,y,z) coordinates from one variable to another, in addition
//       to the Boolean 'flag' element. Both variables must be of type
//       'Coordinate_t'.
//-----------------------------------------------------------------------------
Coordinate_t copyCoordinates(const Coordinate_t sourceCoordinate);


//-----------------------------------------------------------------------------
// Name: XYZpointIsOutsideOfMap
// Desc: Check if the point 'point' is within the map. If not, return TRUE.
//       Return FALSE otherwise.
//-----------------------------------------------------------------------------
int XYZpointIsOutsideOfMap(int x, int y, int z, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: delay
// Desc: Delay execution for 'microSecs' microseconds. This function is used
//       only for debugging, and was copied from the following web page:
//       https://c-for-dummies.com/blog/?p=69.
//-----------------------------------------------------------------------------
void delay(int microSecs);


//-----------------------------------------------------------------------------
// Name: getMemory
// Desc: Measures the current (and peak) resident and virtual memory usage of
//       the linux C process, in kB. This function is used only for debugging,
//       and was copied from the following web page:
// https://stackoverflow.com/questions/1558402/memory-usage-of-current-process-in-c
//-----------------------------------------------------------------------------
void getMemory(int* currRealMem, int* peakRealMem, int* currVirtMem, int* peakVirtMem);


//-----------------------------------------------------------------------------
// Name: printRoutabilityMetrics
// Desc: Print routability metrics to file defined by pointer 'fp' (e.g.,
//       'stdout' or a previously opened file). If there are more nets than
//       maxNets, then the crossing matrix will not be printed out.
//-----------------------------------------------------------------------------
void printRoutabilityMetrics(FILE *fp, const RoutingMetrics_t *routability, const InputValues_t *user_inputs,
                             const MapInfo_t *mapInfo, int numPaths, int maxNets);


//-----------------------------------------------------------------------------
// Name: get_unwalkable_barrier_proximity_by_path
// Desc: Reads the 'forbiddenProximityBarrier' element of the 3D cellInfo
//       matrix at location (x,y,z). This function returns whether this cell is
//       unwalkable due to proximity to a nearby, user-defined obstacle/barrier.
//       Whether the cell is unwalkable depends on the design-rule subset
//       and the shape type ('shape_type'). The design-rule subset is
//       calculated in this function based on the path number ('path_num').
//
//       This function assumes that (x,y,z) is a valid coordinate within the map.
//-----------------------------------------------------------------------------
int get_unwalkable_barrier_proximity_by_path(CellInfo_t ***const  cellInfo, const InputValues_t *user_inputs,
                                             const int x, const int y, const int z, const int path_num,
                                             const int shape_type);


//-----------------------------------------------------------------------------
// Name: pointIsOutsideOfMap
// Desc: Check if the point 'point' is within the map. If not, return TRUE.
//       Return FALSE otherwise.
//-----------------------------------------------------------------------------
int pointIsOutsideOfMap(const Coordinate_t point, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: XY_coords_are_outside_of_map
// Desc: Check of the (x,y) coordinate is within the map. If not, return TRUE.
//       Return FALSE otherwise.
//-----------------------------------------------------------------------------
int XY_coords_are_outside_of_map(const int x, const int y, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: assignCongestionByPathIndex
// Desc: Assign the pathTraversalsTimes100 value to the 'cellInfo' cell for path
//       index 'pathIndex'. If congestion_value exceeds 2^24-1, or 16,777,215
//       (aka 'maxCongestion), then re-define congestion value to this value,
//       which is the largest value that can fit in the 24-bit field.
//-----------------------------------------------------------------------------
void assignCongestionByPathIndex(CellInfo_t *cellInfo, int pathIndex, unsigned congestion_value);


//-----------------------------------------------------------------------------
// Name: get_unwalkable_pinSwap_proximity_by_path
// Desc: Reads the 'forbiddenProximityPinSwap' element of the 3D cellInfo
//       matrix at location (x,y,z). This function returns whether this cell is
//       unwalkable due to proximity to a nearby, user-defined pin-swap zone.
//       Whether the cell is unwalkable depends on the design-rule subset
//       and the shape type ('shape_type'). The design-rule subset is
//       calculated in this function based on the path number ('path_num').
//
//       This function assumes that (x,y,z) is a valid coordinate within the map.
//-----------------------------------------------------------------------------
int get_unwalkable_pinSwap_proximity_by_path(CellInfo_t ***const  cellInfo, const InputValues_t *user_inputs,
                                             const int x, const int y, const int z, const int path_num,
                                             const int shape_type);


//-----------------------------------------------------------------------------
// Name: calc_2D_Pythagorean_distance_ints
// Desc: Calculate the the distance between (x1, y1) and (x2, y2) using the
//       Pythagorean formula. This function does not account for the separation
//       in the z-dimension, and is used for calculating the distance between
//       integer-based coordinates.
//-----------------------------------------------------------------------------
float calc_2D_Pythagorean_distance_ints(const int x1, const int y1, const int x2, const int y2);


//-----------------------------------------------------------------------------
// Name: findCloserVia
// Desc: Determine whether the current via that ends at 'current_end_via' is
//       closer to (x,y) than the via at segment 'end_via', which is located
//       a distance 'closest_distance' from (x,y). If so, then update
//       the values of 'start_via', 'end_via', and 'closest_distance' with the
//       values from 'current_start_via' and 'current_end_via' (respectively)
//       and the new 'closest_distance' value.
//-----------------------------------------------------------------------------
void findCloserVia(const int current_start_via, const int current_end_via, int const pathNum, const int num_vias,
                   int *start_via, int *end_via, float *closest_distance, const int x, const int y,
                   Coordinate_t *pathCoords[], const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: findNearbyLayerTransition
// Desc: Locate layer-transitions (or 'vias') in path pathNum that start on
//       routing layer 'startLayer' and end on layer 'endLayer'. If more than
//       one layer-transition satisfies these requirements, then return the
//       one that is closest to coordinates (x,y). The function returns the
//       start- and end-segments of the via in the non-contiguous path array.
//       If the via begins at the start-terminal, then '-1' is returned for
//       the via's starting segment.
//
//       If pathNum is not a diff-pair path, then this function searches for
//       vertically stacked via-segments. (Pseudo-paths are an example.) If
//       pathNum is a diff-pair path, then the 'via' is any layer-transition,
//       vertically stacked or otherwise.
//
//       If 'enforceStartLayerOnly' is TRUE, then the function:
//         a) does not enforce the criterion for the end-layer, but
//         b) populates the 'end_via' output with the segment number
//            on layer 'endLayer' closest to the via-stack.
//       If 'enforceEndLayerOnly' is TRUE, then the function:
//         a) does not enforce the criterion for the start-layer, but
//         b) populates the 'start_via' output with the segment number
//            on layer 'startLayer' closest to the via-stack.
//
//       The parameters 'enforceStartLayerOnly' and 'enforceEndLayerOnly'
//       must not both be TRUE.
//
//       If no via satisfies the start-/end-layer constraints, then the
//       function returns -1 for the 'startLayer' and 'endLayer' values,
//       and sets the 'error' flag to TRUE.
//-----------------------------------------------------------------------------
ViaStack_t findNearbyLayerTransition(const int pathNum, int pathLengths[], Coordinate_t *pathCoords[],
                                     const int startLayer, const int endLayer, const int x, const int y,
                                     const int enforceStartLayerOnly, const int enforceEndLayerOnly,
                                     const MapInfo_t *mapInfo, const InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: findNearbyLayerTransition_wrapper
// Desc: Call function findNearbyLayerTransition() up to 3 times to locate
//       a via or layer-transition near coordinate (x,y), and which starts
//       on routing layer 'startLayer' and ends on routing layer 'endLayer'.
//       The first call to findNearbyLayerTransition() requires that both
//       the start- and end-layers match. If no such via is located, the
//       second call requires that only the start-layer match. If no such
//       vias are found, a final call is made that requires that only the
//       end-layer matches. If none of these attempts result in a via,
//       the function issues a non-fatal warning message.
//-----------------------------------------------------------------------------
ViaStack_t findNearbyLayerTransition_wrapper(const int pathNum, int pathLengths[], Coordinate_t *pathCoords[],
                                             const int startLayer, const int endLayer, const int x, const int y,
                                             const MapInfo_t *mapInfo, const InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: add_path_center_info
// Desc: Add information about a path #pathNum that traverses cell 'cellInfo'
//       in the 'pathCenters' array. Also increment the 'numTraversingPathCenters'
//       variable.
//-----------------------------------------------------------------------------
void add_path_center_info(CellInfo_t *cellInfo, int pathNum, int shape_type);


//-----------------------------------------------------------------------------
// Name: getIndexOfTraversingPath
// Desc: Determine whether path 'pathNum' with shape type 'shapeType' and design-
//       rule subset 'DR_subset' traverses cell 'cellInfo'. If so, return the
//       index number. If path does not explicitly traverse cell, then return -1.
//-----------------------------------------------------------------------------
int getIndexOfTraversingPath(CellInfo_t *cellInfo, const int pathNum, const unsigned short DR_subset,
                             const unsigned short shapeType);


//-----------------------------------------------------------------------------
// Name: swapStartAndEndTerminals
// Desc: Swap the starting and ending terminals of path number 'pathNum',
//       including the coordinates in cell units (but not in micron units).
//       If the net is a diff-pair net or a pseudo-path, then swap the
//       start- and end-pitch of the terminals (in microns, not in cells).
//
//       Finally, toggle the Boolean flag in the 'start_end_terms_swapped'
//       element for the given path number.
//-----------------------------------------------------------------------------
void swapStartAndEndTerminals(int pathNum, MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: createContiguousPaths
// Desc: For each path in the pathCoords array, generate a contiguous path that
//       contains no skipped cells. The resulting paths are stored in the
//       contigPathCoords array.
//-----------------------------------------------------------------------------
void createContiguousPaths(int numPaths, int pathLengths[], MapInfo_t *mapInfo,
                          Coordinate_t **pathCoords, Coordinate_t **contigPathCoords,
                          int contiguousPathLengths[], InputValues_t *user_inputs,
                          CellInfo_t ***cellInfo);


//-----------------------------------------------------------------------------
// Name: createOneContiguousPath
// Desc: Generate a contiguous path from path given by 'pathCoords[]' that
//       contains no skipped cells. The resulting path is stored in the
//       contigPathCoords[] array.
//
//       In design-rule zones for which the path's line width is at least two
//       cells, do not create intermediate path-segments. Otherwise, create
//       intermediate path-segments to ensure that the path has no gaps.
//-----------------------------------------------------------------------------
void createOneContiguousPath(int pathNum, Coordinate_t start_cells, MapInfo_t *mapInfo, int pathLength,
                             Coordinate_t pathCoords[], Coordinate_t *contigPathCoords[],
                             int *contiguousPathLength, InputValues_t *user_inputs, CellInfo_t ***cellInfo);


//-----------------------------------------------------------------------------
// Name: addCongestionAroundPoint_withSubsetAndShapeType
// Desc: Add a given amount of congestion in the 'cellInfo' 3D matrix with a given
//       path-number, design-rule subset, and shape-type within a given radius
//       about a given (x,y) location on a given routing layer. The amount of
//       congestion is given by 'max_congestion_amount', and represents the amount
//       of congestion deposited at/near the centerPoint coordinate. The amount
//       of congestion decreases linearly with the distance from this centerPoint
//       down to half the 'max_congestion_amount' at the distance of 'radius'.
//       The path-number, design-rule subset, and shape-type are given
//       (respectively) by 'pathNum', 'DR_subset', and 'shapeType'. The
//       radius is given by 'radius'. The square of this value must also be
//       provided as 'radius_squared'.
//-----------------------------------------------------------------------------
void addCongestionAroundPoint_withSubsetAndShapeType(const int pathNum, const int DR_set, const int DR_subset, const char shapeType,
                                                     const Coordinate_t centerPoint, const int radius, const int radius_squared,
                                                     const int max_congestion_amount, const InputValues_t *user_inputs,
                                                     const MapInfo_t *mapInfo, CellInfo_t ***cellInfo);


//-----------------------------------------------------------------------------
// Name: addCongestionAroundTerminal
// Desc: Add congestion (in the 'cellInfo' 3D matrix) at each cell around the
//       point 'centerPoint' that has shape-type 'centerShapeType.
//-----------------------------------------------------------------------------
void addCongestionAroundTerminal(const int pathNum, const Coordinate_t centerPoint, const char centerShapeType,
                                 const InputValues_t *user_inputs, const MapInfo_t *mapInfo, CellInfo_t ***cellInfo);


//-----------------------------------------------------------------------------
// Name: findPath
// Desc: Finds a path using A* algorithm. The information in structure
//       'routingRestrictions' is used to limit the lateral search within a
//       given distance of a given (x,y) coordinate on a given layer. This
//       function returns the G-cost of the path, which will be zero if no
//       path was found. The 'record_explored_cells' parameter tells function
//       which cells to flag as having been explored:
//
//       record_explored_cells   Action
//       ---------------------   ---------------------------------------------
//                 0             Record no explored cells
//                 1             Record explored cells in '.explored' variable
//                 2             Record explored cells in '.explored_PP' variable
//                 3             Record explored cells in '.explored' and
//                               in '.explored_PP' variables
//
//       If Boolean parameter 'record_elapsed_time' is TRUE, then the elapsed
//       (wall-clock) time for finding the path will be saved in variable:
//       routability->path_elapsed_time[pathNum].
//
//       If Boolean input parameter 'useDijkstra' is TRUE, then this function
//       uses a heuristic value of zero. Otherwise, it uses a heuristic that
//       depends on the distance to the target and the DRC histories of the
//       routed path and of other paths in the map.
//
//       If Boolean parameter 'disableRandomCosts' is TRUE, then findPath()
//       disables any randomized changes to the congestion-related G-cost.
//
//-----------------------------------------------------------------------------
unsigned long findPath(const MapInfo_t *mapInfo, CellInfo_t ***const cellInfo,
                       int pathNum, const Coordinate_t startCoord, const Coordinate_t endCoord,
                       Coordinate_t *pathCoords[], int *pathLength, InputValues_t *user_inputs,
                       RoutingMetrics_t *routability, PathFinding_t *pathFinding, const int record_explored_cells,
                       const int record_elapsed_time, const int useDijkstra, const RoutingRestriction_t *routingRestrictions,
                       const int disableRandomCosts, const int recognizeSelfCongestion);


//-----------------------------------------------------------------------------
// Name: calcRoutabilityMetrics
// Desc: Perform design-rule-check (DRC), with results stored in 'DRC_details' and
//       'routability.' Function also calculates the path lengths and via counts
//       for each path, storing these in 'routability.' Function also updates the
//       'cellInfo' matrix with the locations of traces and vias, for use in generating/
//       displaying maps of the layouts. Function also updates the congestion
//       (in 'cellInfo') at each cell if the 'addCongestion' flag is set. This
//       congestion affects subsequent path-finding.
//
//       Path lengths are calculated based on the sparse (non-contiguous) paths.
//       Design-rule violations are calculated based on the contiguous paths.
//
//       If the Boolean flag 'exitIfInvalidJump' is TRUE, then this function will cause
//       program to exit if it detects an illegal jump between two adjacent segments.
//       Set this to FALSE for sub-maps, in which the start-terminals may be outside
//       of the sub-map's boundaries, and the path might exit and re-enter the sub-map.
//
//       If the Boolean flag 'beQuiet' is TRUE, then this function outputs nothing
//       to STDOUT. This can be useful when calculating routability metrics for
//       many small sub-maps. If 'parallelProcessing' is TRUE, then processing is
//       performed in multiple threads.
//-----------------------------------------------------------------------------
void calcRoutabilityMetrics(const MapInfo_t *mapInfo, const int pathLength[],
                            Coordinate_t *pathCoords[], int contiguousPathLength[],
                            Coordinate_t *contigPathCoords[], RoutingMetrics_t *routability,
                            const InputValues_t *user_inputs, CellInfo_t ***cellInfo,
                            DRC_details_t DRC_details[], int addCongestionFlag,
                            int addCongOnlyForDiffPair, int exitIfInvalidJump, int beQuiet,
                            int parallelProcessing);



#endif
