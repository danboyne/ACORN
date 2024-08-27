#include "global_defs.h"
#include "aStarLibrary.h"
#include "drawMaps.h"
#include "routability.h"
#include "parse.h"
#include "prepareMap.h"
#include "processDiffPairs.h"




//----------------------------------------------------------------------------------
// Name: main
// Desc: Top-level C program
//----------------------------------------------------------------------------------  


int main(int argc, char *argv[])  {

  // Don't buffer STDOUT and STDERR so that it's easier to debug programming errors:
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  // Enables formatting integers with thousands-separators like "9,876,543". To find
  // valid arguments on your Linux system, type 'locale -a' at command line.
  setlocale(LC_ALL, "en_US.utf8");

  // Print version number 'VERSION', which is extracted from GIT repository at compile-time.
  printf("INFO: ACORN version %s\n\n", VERSION);

  // Capture the start-time so we can report the total elapsed time at end of each iteration:
  time_t start_autorouter = time(NULL); // Get number of seconds since the Epoch (Jan 1, 1970)

  //
  // Define 'mapInfo' object that contains general info about the map (width, height, etc). 
  //
  MapInfo_t mapInfo;
  // Initialize the iteration to 0. It will be incremented at start of each iteration:
  mapInfo.current_iteration = 0;

  // Print a time-stamp to STDOUT:
  time_t tim = time(NULL);
  struct tm *now = localtime(&tim);
  printf("Date-stamp: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
        now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

  // Define the default number of parallel threads as the number of processors on the computer:
  int num_threads = omp_get_num_procs();

  // Get input filename from command line and (optionally) the maximum number of threads:
  char *input_filename;
  input_filename = malloc(300 * sizeof(char)); // Handle files names with long paths

  // printf("DEBUG: argc is %d.\n", argc);
  // printf("DEBUG: argv[0] is %s.\n", argv[0]);
  // printf("DEBUG: argv[1] is %s.\n", argv[1]);
  // printf("DEBUG: argv[2] is %s.\n", argv[2]);
  // printf("DEBUG: argv[3] is %s.\n", argv[3]);
  if (argc == 2)  {
    // Input filename is last argument on command line.

    // Check if length of filename on command-line is too long to fit into string variable:
    if (strlen(argv[argc-1]) >= 300)  {
      // printf("DEBUG: length of argv[%d] is %ld.\n", argc-1, strlen(argv[argc-1]));
      printf("\nERROR: File name is too long (%ld characters).\n\n", strlen(argv[argc-1]));
      exit(1);
    }

    // Copy lone command-line argument (filename) to variable 'input_filename':
    strncpy(input_filename, argv[argc-1], 300);
  }
  else if ((argc == 4) && (strcmp(argv[1], "-t") == 0))  {
    int num_requested_threads = atoi(argv[2]);
    if (num_requested_threads <= num_threads)  {
      num_threads = num_requested_threads;
    }
    else {
      printf("INFO: Command-line specified %d threads, but there are only %d threads on the computer.\n",
              num_requested_threads, num_threads);
    }

    // Check if length of filename on command-line is too long to fit into string variable:
    if (strlen(argv[argc-1]) >= 300)  {
      // printf("DEBUG: length of argv[%d] is %ld.\n", argc-1, strlen(argv[argc-1]));
      printf("\nERROR: File name is too long (%ld characters).\n\n", strlen(argv[argc-1]));
      exit(1);
    }

    // Copy lone command-line argument (filename) to variable 'input_filename':
    strncpy(input_filename, argv[argc-1], 300);

  }
  else  {
    printf("ERROR: Usage is: %s [-t num_threads] input_filename.\n", argv[0]);
    exit(1);
  }

  // Define the number of threads for parallel processing:
  omp_set_num_threads(num_threads);
  printf("INFO: Number of threads is %d.\n", num_threads );

  // Enable nested parallelization in OpenMP:
  omp_set_nested(TRUE);
  // printf("DEBUG: omp_get_nested is %d.\n", omp_get_nested());

  //
  // Define the data structure for user-defined input data:
  //
  InputValues_t user_inputs; // 'user_inputs' is structure for data from input file

  //
  // Pre-parse the user's input file to determine the number of nets and various
  // other parameters, so that we can later allocate the appropriate amount of
  // memory for these data structures
  //
  pre_process_input_file(input_filename, &user_inputs);

  printf("DEBUG: Output of pre-processing the input file:\n");
  printf("                                                   Number of nets: %d\n", user_inputs.num_nets);
  printf("                               Number of nets that are diff-pairs: %d\n", user_inputs.num_diff_pair_nets);
  printf("                    Number of nets with net-specific design rules: %d\n", user_inputs.num_special_nets);
  printf("                             Number of block/unblock instructions: %d\n", user_inputs.num_block_instructions);
  printf("          Number of design-rule sets (design_rule_set statements): %d\n", user_inputs.numDesignRuleSets);
  for (int j = 0; j < user_inputs.numDesignRuleSets; j++)  {
    printf("                          Number of subsets for design-rule set %d: %d\n", j, user_inputs.numDesignRuleSubsets[j]);
  }
  printf("                 Number of design-rule zones (DR_zone statements): %d\n", user_inputs.num_DR_zones);
  printf("                             Number of trace_cost_zone statements: %d\n", user_inputs.num_trace_cost_zone_instructions);
  printf("                               Number of via_cost_zone statements: %d\n", user_inputs.num_via_cost_zone_instructions);
  printf("                                  Number of pin-swap instructions: %d\n", user_inputs.num_swap_instructions);


  //
  // Allocate memory and initialize the data structure for user-defined input data:
  //
  initialize_input_values(&user_inputs);  // Allocate memory for 'user_inputs' data structure
  allocateMapInfo(&mapInfo, user_inputs.num_nets, user_inputs.num_pseudo_nets, user_inputs.num_routing_layers);
  // printf("DEBUG: Finished initializing input values.\n");

  // Define user-friendly names of the 3 shape-types, associated with their indices:
  char *shapeTypeNames[NUM_SHAPE_TYPES];
  shapeTypeNames[TRACE]    = "TRACE\0";
  shapeTypeNames[VIA_UP]   = "VIA-UP\0";
  shapeTypeNames[VIA_DOWN] = "VIA-DOWN\0";

  //
  // Read input file and place data into 'user_inputs' data structure
  //
  parse_input_file(input_filename, &user_inputs, &mapInfo);
  // printf("DEBUG: Finished parsing input values.\n");


  // Create a 'pathFinding' array that contains num_threads elements:
  int num_simultaneous_pathFinding = num_threads;
  PathFinding_t pathFinding[num_simultaneous_pathFinding];
  for (int i = 0; i < num_simultaneous_pathFinding; i++)  {
    // Allocate memory for each element of the 'pathFinding' array. See definition of
    // structure 'PathFinding_t' structure for the contents/description.
    allocatePathFindingArrays(&(pathFinding[i]), &mapInfo);
  }  // End of for-loop for index 'i' (0 to num_simultaneous_pathFinding-1)
  // printf("DEBUG: pathFinding arrays were initialized for up to %d simultaneous path-finding threads.\n", num_simultaneous_pathFinding);


  // 'cellInfo' is a 3D array of CellInfo_t objects (see header file for description).
  // Dynamically allocate space for 'cellInfo' matrix and initialize elements
  // to have no traversing paths and zero unwalkable cells:
  CellInfo_t ***cellInfo = allocateCellInfo(&mapInfo);
  initializeCellInfo(cellInfo, &mapInfo);

  // Calculate the minimum number of DRC-free solutions that must be achieved before
  // the program ends. Value is derived by the minimum 'userDRCfreeThreshold' specified by
  // the user in the input file, plus 35 times the base-10 logarithm of the net-count.
  int DRC_free_threshold = user_inputs.userDRCfreeThreshold + 35*log10(user_inputs.num_nets);

  printf("INFO: Program requires at least %d DRC-free solutions before it terminates.\n",
          DRC_free_threshold);


  // Assign a layer number (starting with zero) for each routing layer:
  // 'numRoutingLayers' is the number of routing layers, excluding vias
  for (int i = 0; i < user_inputs.num_routing_layers; i++)  {
    strcpy(user_inputs.routingLayerNames[i], user_inputs.layer_names[2*i]);
    printf("DEBUG: Routing layer '%s' is mapped to layer number %d.\n", 
                user_inputs.layer_names[2*i], i);
  }  // End of for-loop

  // Calculate the total number of nets to route, including user-defined nets and
  // (if applicable) pseudo nets for differential pairs:
  int max_routed_nets = user_inputs.num_nets + user_inputs.num_pseudo_nets;

  // Report the number of paths to route (= number of nets):
  printf("INFO: Number of paths to route is %d, including %d pseudo nets for differential pairs.\n",
         max_routed_nets, user_inputs.num_pseudo_nets);

  //
  // Structure 'routability' contains elements that describe the 'goodness' of the
  // routed paths for the main map:
  //
  RoutingMetrics_t routability;
  createRoutability(&routability, &mapInfo);
  initializeRoutability(&routability, &mapInfo, TRUE);

  // Initialize 'adequateSolutionFound' to FALSE. It will be changed to TRUE only after
  // all criteria are met for an adequate solution:
  int adequateSolutionFound = FALSE;

  //
  // Create a preliminary version of the 'routingStatus.html' file:
  //
  if (create_routingStatus_HTML_file(input_filename, "routingStatus.html", &mapInfo, &routability, &user_inputs, shapeTypeNames, adequateSolutionFound, DRC_free_threshold, num_threads) == 0)  {
    printf("\nINFO: Output file 'routingStatus.html' was successfully created.\n");
  }
  else  {
    printf("\n\nERROR: The file 'routingStatus.html' was not successfully created in function main(). This behavior is not expected.\n");
    printf(    "       Please inform the software developer of this fatal error message.\n\n");
    exit(1);
  }

  //
  // Array 'subMapRoutability' contains elements that describe the 'goodness' of the routed
  // paths in the sub-maps used for diff-pairs. Two 'subMapRoutability' variables of type
  // RoutingMetrics_t are created: one for the non-swapped wiring configuration, and one
  // for the swapped wiring configuration.
  //
  RoutingMetrics_t subMapRoutability[2];

  // Create a MapInfo_t variable that's appropriate for creating the subMapRoutability
  // variables. The MapInfo_t variable must have the following elements defined:
  // numPaths, numPseudoPaths, numLayers, and max_iterations.
  MapInfo_t genericSubMapInfo;
  genericSubMapInfo.numPaths = mapInfo.numPaths;
  genericSubMapInfo.numPseudoPaths = mapInfo.numPseudoPaths;
  genericSubMapInfo.numLayers = mapInfo.numLayers;
  genericSubMapInfo.max_iterations = subMap_maxIterations;
  genericSubMapInfo.current_iteration = 1;
  // If the user defined any diff-pair nets, then allocate memory and initialize the
  // variables necessary for routing the diff-pair nets to their terminals and vias:
  if (user_inputs.num_diff_pair_nets > 0)  {
    for (int wire_config = NOT_SWAPPED; wire_config <= SWAPPED; wire_config++)  {
      createRoutability(&(subMapRoutability[wire_config]), &genericSubMapInfo);
      initializeRoutability(&(subMapRoutability[wire_config]), &genericSubMapInfo, TRUE);
    }  // End of for-loop for index 'wire_config' (0 to 1)
  }  // End of if-block for num_diff_pair_nets > 0


  // Define the initial congestion sensitivities for trace and via congestion as
  // the 0th value in the 'congSensitivity' array. Later, this value might be
  // changed by the algorithm to optimize the routing:
  mapInfo.currentTraceCongSensIndex = 0;
  mapInfo.currentViaCongSensIndex = 0;


  // Define the initial values for 'congestionMultiplier', which define how sensitivity the
  // path-finding algorithm is to congestion in the map. These multipliers can be modified
  // by the algorithm based on routing metrics:
  mapInfo.traceCongestionMultiplier = 0.20
                                      * (routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter / 100.0)
                                      * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;
  mapInfo.viaCongestionMultiplier = 0.20
                                      * (routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter / 100.0)
                                      * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;


  // Create needed arrays
  int *pathLengths;  // Stores length of the found path for each start/end pair
  pathLengths = malloc(max_routed_nets * sizeof(int));
  if (pathLengths == 0)  {
    printf("\nERROR: Unable to allocate %d elements for array pathLengths.\n\n", max_routed_nets);
    exit(1);
  }

  Coordinate_t **pathCoords;  // pathCoords is array of pointers to arrays:
                              //    pathCoords[path_number][cellPosition].X = X coordinate
                              //    pathCoords[path_number][cellPosition].Y = Y coordinate
                              //    pathCoords[path_number][cellPosition].Z = Z coordinate
                              //    pathCoords[path_number][cellPosition].flag = flag used for various purposes
                              // pathCoords stores the x/y/z locations of each path, EXCLUDING
                              // the starting location, which is stored in start_cells
  pathCoords = malloc(max_routed_nets * sizeof(Coordinate_t *));
  if (pathCoords == 0)  {
    printf("\nERROR: Unable to allocate %d elements for array pathCoords.\n\n", max_routed_nets);
    exit(1);
  }

  int *contiguousPathLengths;  //stores length of the corresponding contiguous paths
  contiguousPathLengths = malloc (max_routed_nets * sizeof(int));
  if (contiguousPathLengths == 0)  {
    printf("\nERROR: Unable to allocate %d elements for array contiguousPathLengths.\n\n", max_routed_nets);
    exit(1);
  }

  Coordinate_t **contigPathCoords;  // contigPathCoords is array of pointers to arrays:
                                    //    contigPathCoords[path_number][cellPosition].X = X coordinate
                                    //    contigPathCoords[path_number][cellPosition].Y = Y coordinate
                                    //    contigPathCoords[path_number][cellPosition].Z = Z coordinate
                                    //    contigPathCoords[path_number][cellPosition].flag = unused Boolean flag
                                    // contigPathCoords stores the x/y/z locations of each
                                    // contiguous path, INCLUDING the starting location.
  contigPathCoords = malloc(max_routed_nets * sizeof(Coordinate_t *));
  if (contigPathCoords == 0)  {
    printf("\nERROR: Unable to allocate %d elements for array contigPathCoords.\n\n", max_routed_nets);
    exit(1);
  }

  // Dynamically allocate small amount of memory from heap for 'pathCoords' and
  // 'contigPathCoords' arrays of arrays. Also initialize to zero the elements of
  // pathLengths and contiguousPathLengths arrays:
  initializePathfinder(max_routed_nets, pathLengths, pathCoords, contiguousPathLengths, contigPathCoords);


  // Create variable 'noRoutingRestrictions' that will be used by all calls to findPath() within
  // the 'main' function. This variable reflects zero routing restrictions. The same
  // variable can be used simultaneously by all threads:
  RoutingRestriction_t noRoutingRestrictions;

  // Initialize elements of variable noRoutingRestrictions so there are no restrictions
  // on routing.
  createNoRoutingRestrictions(&noRoutingRestrictions);

  //  Modify the 'cellInfo' 3D matrix based on the DR_zone statements in
  //  the 'user_inputs' data structure.
  tim = time(NULL); now = localtime(&tim);
  printf("\nDate-stamp before calling defineCellDesignRules: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  defineCellDesignRules(cellInfo, &mapInfo, &user_inputs);
  tim = time(NULL); now = localtime(&tim);
  printf("Date-stamp after returning from defineCellDesignRules: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


  //  Modify the 'cellInfo' 3D matrix based on the BLOCK/UNBLOCK statements in
  //  the 'user_inputs' data structure.
  tim = time(NULL); now = localtime(&tim);
  printf("\nDate-stamp before calling defineBarriers: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  defineBarriers(cellInfo, &mapInfo, &user_inputs);
  tim = time(NULL); now = localtime(&tim);
  printf("Date-stamp after returning from defineBarriers: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


  //  Modify the 'cellInfo' 3D matrix based on the 'trace_cost_zone' and 'via_cost_zone'
  //  statements in the 'user_inputs' data structure.
  tim = time(NULL); now = localtime(&tim);
  printf("\nDate-stamp before calling defineCellCosts: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  defineCellCosts(cellInfo, &mapInfo, &user_inputs);
  tim = time(NULL); now = localtime(&tim);
  printf("Date-stamp after returning from defineCellCosts: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

  //  Modify the 'cellInfo' 3D matrix based on the 'PIN_SWAP' and 'NO_PIN_SWAP'
  //  statements in the 'user_inputs' data structure.
  tim = time(NULL); now = localtime(&tim);
  printf("\nDate-stamp before calling definePinSwapZones: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  definePinSwapZones(cellInfo, &mapInfo, &user_inputs);
  tim = time(NULL); now = localtime(&tim);
  printf("Date-stamp after returning from definePinSwapZones: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


  // Modify the 'cellInfo' 3D matrix to identify cells *near* user-defined barriers,
  // edges of the map, and pin-swap zones.
  tim = time(NULL); now = localtime(&tim);
  printf("\nDate-stamp before calling defineProximityZones: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
  defineProximityZones(cellInfo, &mapInfo, &user_inputs);
  tim = time(NULL); now = localtime(&tim);
  printf("Date-stamp after returning from defineProximityZones: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
      now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


  // For each diff-pair, verify that the two starting terminals and two ending
  // terminals are on the same layer and within the same design-rule zone. Verify
  // that the two starting terminals and two ending terminals are within a reasonable
  // distance of each other. Verify that there are no other terminals between
  // the two starting and two ending terminals. Calculate the "rat's nest" distance
  // between the start- and end-terminal for each path.
  verifyDiffPairTerminals(&user_inputs, cellInfo, &mapInfo);


  // For each net, verify that its start- and end-terminals are not
  // too close to those of nets:
  verifyAllTerminals(&user_inputs, cellInfo, &mapInfo);

  //
  // Create PNG maps showing the design-rule (DR) zones, which are static and do not change
  // throughout the auto-routing process. Returned value 'designRuleConflicts' is zero if 
  // there are no conflicts in via diameters between different design-rule zones. 
  //
  int designRuleConflicts = makeDesignRulePngMaps(cellInfo, &mapInfo, &user_inputs);


  //
  // DEBUG CODE FOLLOWS:
  //
  printf("\n----------------------------------------------------------------\n");
  printf("DEBUG: Usage summary of design rules and design-rule subsets:\n");
  for (int DR_num = 0; DR_num < user_inputs.numDesignRuleSets; DR_num++)  {
    printf("DEBUG:    Design rule #%d usage is %d.\n", DR_num, user_inputs.designRuleUsed[DR_num]);
    for (int DR_subset = 0; DR_subset < user_inputs.numDesignRuleSubsets[DR_num]; DR_subset++)  {
      printf("DEBUG:      Design-rule subset #%d usage is %d.\n", DR_subset, user_inputs.DR_subsetUsed[DR_num][DR_subset]);
    }  // End of for-loop for index 'DR_subset'
  }  // End of for-loop for index 'DR_num'
  printf("----------------------------------------------------------------\n");

  // 
  // Create HTML page showing design rules:
  //
  makeDesignRuleReport(cellInfo, &user_inputs, &mapInfo);

  // If design-rule conflicts are detected, then reduce the 'maxIterations' variable to 1
  // so that program halts after drawing maps of problematic design-rule zones:
  if (designRuleConflicts)  {
    user_inputs.maxIterations = 1;
  }

  //
  // Create PNG maps showing the zones that have user-defined cost multipliers. These PNG
  // maps are static and do not change throughout the auto-routing process. The returned
  // value, 'cost_multipliers_used', is TRUE if any non-unity cost-multipliers are used
  // in the map (FALSE otherwise).
  //
  int cost_multipliers_used = makeCostZonePngMaps(cellInfo, &mapInfo, &user_inputs);

  // 
  // Create HTML page showing cost zones:
  //
  makeCostMapReport(cellInfo, &user_inputs, &mapInfo);

  //
  // Create HTML page showing map without any routing:
  //
  makeHtmlIterationSummary(0, &mapInfo, cellInfo, &user_inputs, &routability,
                           "Title", shapeTypeNames);


  //
  // Open the output HTML file that will contain important output info and
  // hyperlinks to maps:
  //
  FILE *fp_TOC = start_HTML_table_of_contents(input_filename, &user_inputs, &mapInfo, DRC_free_threshold, num_threads);

  // Initialize the 'sequence' array, which will determine the sequence of path-finding.
  // Path-finding is performed first on the path with the longest length, and then the
  // next shortest, etc.  This sequence optimizes the CPU utilization during 
  // parallel processing. The format of this array is:
  //
  //    sequence[order of path-finding] = path_number
  //
  int *sequence = malloc(max_routed_nets * sizeof(int));
  for (int path = 0; path < max_routed_nets; path++)  {
    sequence[path] = path;  // Initial sequence is simply the order of the path numbers
  }


  // Capture the cumulative time (in seconds) required to parse the user's input file and
  // prepare the routing map. Save this time as 'iteration number zero':
  routability.iteration_cumulative_time[0] = (int) (time(NULL) - start_autorouter);

  //
  // Run a maximum of 'maxIterations' iterations of the path-finding algorithm, updating 
  // the 'cellInfo' matrix after each run:
  //
  int addCongestion = TRUE; // Flag to add congestion after each iteration. Set to FALSE only
                            // for 1st iteration if non-unity cost-multipliers exist.

  while ((mapInfo.current_iteration < user_inputs.maxIterations) && (! adequateSolutionFound))  {

    mapInfo.current_iteration++;
    printf("\n---\nINFO: Starting iteration number %d...\n", mapInfo.current_iteration);

    // Update the 'congestionMultiplier' factor, which depends on the iteration number:
    update_iterationDependent_parameters(&mapInfo, &routability, fp_TOC);

    //
    // Reduce the congestion from previous iterations as long as we've already completed 
    // at least 'preEvaporationIterations' iterations.
    //
    if (mapInfo.current_iteration > user_inputs.preEvaporationIterations)  {
      printf("INFO: Evaporating %d percent of congestion from previous iterations.\n", defaultEvapRate);
      evaporateCongestion(&mapInfo, cellInfo, defaultEvapRate, num_threads);
    }

    // If there are non-unity cost-multipliers in the map, then run the first iteration
    // without these added costs. This provides the user with a baseline "rat's nest" of the routing.
    // For iteration #2 (and subsequent iterations), include the effects of the added costs:
    // printf("DEBUG: current_iteration = %d.  cost_multipliers_used = %d\n", mapInfo.current_iteration, cost_multipliers_used);
    if (! cost_multipliers_used)  {
      if (mapInfo.current_iteration == 1)  {
        // We got here, so the user didn't define cost-multipliers to zones in the map.
        // We therefore set the cell-costs to their base-costs during the first iteration:
        set_costs_to_base_values(&user_inputs);
        addCongestion = TRUE;
      }  // End of if-block for current_iteration == 1
    }  // End of if-block for cost_multipliers_used == FALSE
    else  {
      // We got here, so cost-multipliers are used in the map. For the first iteration,
      // we set the cell-costs to their base values:
      if (mapInfo.current_iteration == 1) {
        set_costs_to_base_values(&user_inputs);
        addCongestion = FALSE;
        printf("INFO: Because there are cost multipliers used in the map, the first iteration will be run\n");
        printf("      disregarding these added costs, thereby providing a \"rat's nest\" view of the routing\n");
        printf("      in the absence of added costs.\n");
      }  // End of if-block for current_iteration == 1
      if (mapInfo.current_iteration == 2) {
        // We got here, so cost-multipliers are used in the map. For the second (and subsequent
        // iterations, we set the cell-costs to their user-defined (higher) values:
        set_costs_to_userDefined_values(&user_inputs);
        addCongestion = TRUE;
        printf("INFO: Because there are cost multipliers used in the map, the second and subsequent iterations\n");
        printf("      will be run with these added costs.\n");
      }  // End of if-block for current_iteration == 2
    }  // End of else-block for cost_multipliers_used == TRUE

    //
    // For each start- and end-location, find the most efficient path:
    //
    #pragma omp parallel for schedule(dynamic, 1)
    for (int pathFindingSequence = 0; pathFindingSequence < max_routed_nets; pathFindingSequence++)  {

      int pathNum = sequence[pathFindingSequence];
      int thread_num = omp_get_thread_num();

      // Check if net is part of a diff-pair, in which case we don't run the path-finding algorithm:
      if (user_inputs.isDiffPair[pathNum])  {
        printf("INFO: Skipping diff-pair net #%d because it will be routed using pseudo net #%d.\n", pathNum, user_inputs.diffPairToPseudoNetMap[pathNum]);
      }
      else  {

        // We got here, so net is not a diff-pair net. Prepare to run the path-finding algorithm:

        tim = time(NULL); now = localtime(&tim);
        printf("INFO: Starting path %3d (sequence %3d) in thread %2d with %'d DRCs at %02d-%02d-%d, %02d:%02d:%02d.\n",
             pathNum, pathFindingSequence, thread_num, routability.path_DRC_cells[pathNum],
             now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
        // printf("DEBUG: Sequence %d: Path %d has %d path-overlaps...\n", pathFindingSequence,
        //        pathNum, routability.path_DRC_cells[pathNum]);

        // Run the 'FindPath algorithm to optimize this path:

        // printf("\nDEBUG: About to enter 'findPath' with pathFinderRun=%d, mapWidth=%d, mapHeight=%d, numLayers=%d\n",
        //         mapInfo.current_iteration, mapInfo.mapWidth, mapInfo.mapHeight, mapInfo.numLayers);
        // printf("       Starting point: (%d, %d, %d)\n", mapInfo.start_X_cells[pathNum], mapInfo.start_Y_cells[pathNum], mapInfo.start_Z[pathNum]);
        // printf("         Ending point: (%d, %d, %d)\n", mapInfo.end_X_cells[pathNum], mapInfo.end_Y_cells[pathNum], mapInfo.end_Z[pathNum]);


        //
        // Enter the 'findPath' function to find best path for path number 'pathNum'
        //
        unsigned long pathCost = findPath(&mapInfo, cellInfo, pathNum, mapInfo.start_cells[pathNum], mapInfo.end_cells[pathNum],
                                          &(pathCoords[pathNum]), &(pathLengths[pathNum]), &user_inputs, &routability, &pathFinding[thread_num],
                                          1, TRUE, FALSE, &noRoutingRestrictions, FALSE, FALSE);

        tim = time(NULL);
        now = localtime(&tim);
        printf("INFO:   Explored %'lu cells for path %d (sequence %d), requiring %'d seconds at %02d-%02d-%d, %02d:%02d.\n",
               routability.path_explored_cells[pathNum], pathNum, pathFindingSequence,
               routability.path_elapsed_time[pathNum], now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min);

        if (! pathCost)  {
          printf("\nERROR: No path was found for path %d ('%s'). Path cost = %lu. Exiting.\n\n",
                  pathNum, user_inputs.net_name[pathNum], pathCost);
          exit(1);
        }
        // printf("DEBUG:     done finding new path for path %d. F-cost was %'lu.\n\n", pathNum, pathCost);
        // printf("DEBUG:     done finding new path for path %d.\n\n", pathNum);

        // If the path starts in a pin-swap zone, then update the start-terminal so that it's
        // the last path-segment before the path exits the pin-swap zone:
        if (mapInfo.swapZone[pathNum])  {
          update_swapZone_startTerms(pathNum, &(pathCoords[pathNum]), &pathLengths[pathNum],
                                     &user_inputs, cellInfo, &mapInfo);
        }

      }  // End of if/else-block for running the path-finding function for nets that aren't diff-pairs

    }  // End of for-loop for variable 'pathFindingSequence'
    //
    // The previous line is the end of multi-threaded processing in this file.
    //

    printf("\nINFO: Completed findPath for all nets.\n\n");

    // If the map contains differential pairs, then create diff-pair nets using the pseudo-nets routed by the auto-router:
    if (user_inputs.num_pseudo_nets > 0)  {
      // printf("DEBUG: Before calling postProcessDiffPairs, omp_get_num_threads = %d\n", omp_get_num_threads());
      postProcessDiffPairs(pathCoords, pathLengths, &user_inputs, cellInfo, &mapInfo, &routability, pathFinding, subMapRoutability,
                           &noRoutingRestrictions, num_threads);
    }

    // Based on the paths found from the 'findPath' function, generate corresponding
    // contiguous paths (without any missing gaps or skipped cells):
    // printf("DEBUG: Entering createContiguousPath in thread %d\n", omp_get_thread_num());
    createContiguousPaths(max_routed_nets, pathLengths, &mapInfo, pathCoords, contigPathCoords, contiguousPathLengths, &user_inputs, cellInfo);
    // printf("DEBUG: Returned from function 'createContiguousPath' in thread %d.\n", omp_get_thread_num());
  
    // Print a time-stamp to STDOUT:
    tim = time(NULL);
    now = localtime(&tim);
    printf("\nINFO: Date-stamp before entering calcRoutabilityMetrics: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
        now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);


    // Call to function reInitializeCellInfo() was added 3/16/2024 after being removed from calcRoutabilityMetrics():
    reInitializeCellInfo(&mapInfo, cellInfo);

    // Calculate the 'goodness' of the solution (routability metrics), and add congestion to the map
    // at/near the nets and vias (if addCongestion is TRUE):
    calcRoutabilityMetrics(&mapInfo, pathLengths, pathCoords,
                           contiguousPathLengths, contigPathCoords, &routability,
                           &user_inputs, cellInfo, addCongestion,
                           ADD_CONGESTION_FOR_ALL_NETS, TRUE, FALSE, TRUE);

    // printf("DEBUG: Returned from function 'calcRoutabilityMetrics' in thread %d.\n", omp_get_thread_num());

    //
    // Print a time-stamp to STDOUT:
    tim = time(NULL);
    now = localtime(&tim);
    printf("INFO: Date-stamp after exiting calcRoutabilityMetrics: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
        now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

    // Because the start- and end-terminals of each path are immovable, we add additional congestion
    // at/around these points if the 'addCongestion' flat is set. This has been shown to improve
    // routing results in cases with closely spaced terminals:
    if (addCongestion)  {
      addCongestionAroundAllTerminals(&user_inputs, &mapInfo, cellInfo, contigPathCoords, contiguousPathLengths);
    }

    // Determine the iterations with the best routing metrics. The lowest-cost iteration is the one with
    // the lowest number of cells with DRCs. If multiple iterations contain zero DRC cells, then
    // the best iteration is the DRC-free iteration with the lowest routing cost. This function also
    // finds the iteration with the shortest aggregate path-length which has the fewest number of DRC-cells.
    determineBestIterations(&mapInfo, &routability, cost_multipliers_used);
    printf("DEBUG: After returning from function determineBestIterations after iteration %d, the iteration with the lowest-cost routing metrics is %d.\n",
            mapInfo.current_iteration, routability.lowest_cost_iteration);
    printf("DEBUG: The iteration with the shortest aggregate path-length is %d.\n", routability.shortest_path_iteration);

    printRoutabilityMetrics(stdout, &routability, &user_inputs, &mapInfo, max_routed_nets, 30);

    if (mapInfo.current_iteration >= 3)  {
      printf("INFO: Last 3 path non-pseudo lengths are %'8.3f, %'8.3f, and %'8.3f\n",
              routability.nonPseudoPathLengths[mapInfo.current_iteration], routability.nonPseudoPathLengths[mapInfo.current_iteration-1],
              routability.nonPseudoPathLengths[mapInfo.current_iteration-2]);
      printf("INFO: Last 3 non-pseudo DRC counts are %d, %d, and %d\n",
              routability.nonPseudo_num_DRC_cells[mapInfo.current_iteration], routability.nonPseudo_num_DRC_cells[mapInfo.current_iteration-1],
              routability.nonPseudo_num_DRC_cells[mapInfo.current_iteration-2]);
    }  // End if if-block for current_iteration >= 3

    // Re-calculate the 'sequence' array, which is sorted in descending order of the
    // time required to find each path.
    int temp;
    for (int i = 0; i < max_routed_nets; i++)  {
      for (int j = i+1; j < max_routed_nets; j++)  {
        if (routability.path_elapsed_time[sequence[i]] < routability.path_elapsed_time[sequence[j]])  {
          temp = sequence[i];
          sequence[i] = sequence[j];
          sequence[j] = temp;
        }
      }  // End of loop for index 'j'
    }  // End of loop for index 'i'

    // Print out the sequence of path-finding for the next iteration:
    printf("\nINFO: Sequence of next path-finding iteration:\n");
    for (int i=0; i < max_routed_nets; i++)  {
      printf("  Sequence %d: Path %d with elapsed time of %'d seconds, length of %'d cell-units, and %'lu explored cells.\n",
               i, sequence[i], routability.path_elapsed_time[sequence[i]], pathLengths[sequence[i]], routability.path_explored_cells[sequence[i]]);
    }
    printf("INFO: *********** End of sequence list *********\n");


    // Update the HTML table-of-contents file with the results of
    // iteration # 'current_iteration', including the generation of PNG map-files
    // and a new HTML file to display these PNG files.
    updateHtmlTableOfContents(fp_TOC, &mapInfo, cellInfo, &user_inputs, &routability,
                              shapeTypeNames, cost_multipliers_used);

    // printf("DEBUG: Returned successfully from function 'updateHtmlTableOfContents'.\n");


    //
    // Check whether we can exit the path-finding algorithm:
    //
    adequateSolutionFound = determineIfSolved(mapInfo.current_iteration, DRC_free_threshold,
                                      user_inputs.num_nets - user_inputs.num_pseudo_nets,
                                      user_inputs.maxIterations, &routability);
    printf("DEBUG: determineIfSolved returned '%d'\n", adequateSolutionFound);
    // fprintf(fp_TOC, "  <UL><LI><FONT color=\"#FF3300\">adequateSolutionFound is %d.</FONT></UL>\n", adequateSolutionFound);


    // Determine which changes (if any) should be made to the routing algorithm. The
    // three possible changes are:
    //       (1) Swap start- and end-terminals of nets with DRCs
    //       (2) Change the congestion sensitivity
    //       (3) Enable the application of TRACE pseudo-congestion near pseudo-vias
    determineAlgorithmChanges(&mapInfo, DRC_free_threshold, &routability, &user_inputs);

    //
    // If we need to swap start- and end-terminals of paths that have DRCs, then do so.
    // Add comments to the log file and HTML output file reflecting this change:
    //
    if (routability.swapStartAndEndTerms[mapInfo.current_iteration])  {

      // For paths that have DRCs, swap the start- and end-terminals. The 'FALSE' in the
      // parameter list of function 'swap_start_and_end_terminals_of_DRC_paths()' tells
      // this function to actually swap the terminals (and not just count the affected nets).
      int num_nonPseudo_terminals_swapped = swap_start_and_end_terminals_of_DRC_paths(max_routed_nets, &mapInfo,
                                                                                      &routability, &user_inputs, FALSE);

      // Update the HTML file to show that terminals will be swapped for the next iteration, and how many nets are involved (excluding pseudo-paths):
      if (num_nonPseudo_terminals_swapped > 0)  {
        printf("INFO: Due to stagnant routability metrics, start- and end-terminals were swapped for %d nets to improve routing (swap #%d).\n",
                num_nonPseudo_terminals_swapped, routability.num_startEnd_terminal_swaps);
        fprintf(fp_TOC, "  <UL><LI><FONT color=\"#00CC00\">Start- and end-terminals were swapped for %d nets to improve routing (swap #%d).</FONT></UL>\n",
                num_nonPseudo_terminals_swapped, routability.num_startEnd_terminal_swaps);


        // Create a temporary string variable to contain the message that will be stored in routability->HTML_message_strings and
        // eventually printed out to the HTML file:
        char HTML_message[500];
        sprintf(HTML_message, "<FONT color=\"#00CC00\">Start- and end-terminals were swapped for %d nets to improve routing (swap #%d).</FONT>",
                num_nonPseudo_terminals_swapped, routability.num_startEnd_terminal_swaps);
        // printf("\nDEBUG: In function main, HTML_message = '%s'\n\n", HTML_message);
        add_HTML_message(HTML_message, mapInfo.current_iteration, SWAP_TERMS, &routability);
      }
    }  // End of if-block for swapStartAndEndTerms == TRUE


    //
    // If the via congestion sensitivity needs to be changed, then do so. Add comments
    // to the log file and HTML output file reflecting this change:
    //
    if (routability.changeViaCongSensitivity[mapInfo.current_iteration])  {

      // We got here, so the congestion multiplier needs to be changed.

      // Temporarily record the current via congestion sensitivity index:
      unsigned char old_via_cong_sensitivity_index = mapInfo.currentViaCongSensIndex;

      printf("DEBUG: Changing via congestion sensitivity from %d%%...\n",
             routability.viaCongSensitivityMetrics[old_via_cong_sensitivity_index].dynamicParameter);

      if (routability.changeViaCongSensitivity[mapInfo.current_iteration] == INCREASE)  {
        mapInfo.currentViaCongSensIndex++;
      }
      else if (routability.changeViaCongSensitivity[mapInfo.current_iteration] == DECREASE)  {
        mapInfo.currentViaCongSensIndex--;
      }
      else  {
        printf("\nERROR: An unexpected state occurred in which the value of routability.changeViaCongSensitivity[%d]\n",
               mapInfo.current_iteration);
        printf("       contains an illegal value (%d). Inform the software developer of this fatal error.\n\n",
               routability.changeViaCongSensitivity[mapInfo.current_iteration]);
        exit(1);
      }

      // Re-calculate the 'viaCongestionMultiplier' value that's used in function 'findPath()':
      mapInfo.viaCongestionMultiplier = (routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter / 100.0)
                                       * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;

      if (routability.changeViaCongSensitivity[mapInfo.current_iteration] == INCREASE)  {
        printf("INFO: Due to stagnant routability metrics, via congestion sensitivity increased from %d%% to %d%% (via change #%d, %d stable via metrics, %d stable trace metrics).\n",
               routability.viaCongSensitivityMetrics[old_via_cong_sensitivity_index].dynamicParameter,
               routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter,
               routability.num_viaCongSensitivity_changes, routability.num_viaCongSensitivity_stableRoutingMetrics,
               routability.num_traceCongSensitivity_stableRoutingMetrics);
        fprintf(fp_TOC, "  <UL><LI><FONT color=\"#00CC00\">Via Congestion Sensitivity increased from %d%% to %d%% due to stagnant results (via change #%d, %d stable via metrics, %d stable trace metrics).</FONT></UL>\n",
                routability.viaCongSensitivityMetrics[old_via_cong_sensitivity_index].dynamicParameter,
                routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter,
                routability.num_viaCongSensitivity_changes, routability.num_viaCongSensitivity_stableRoutingMetrics,
                routability.num_traceCongSensitivity_stableRoutingMetrics);


        // Create a temporary string variable to contain the message that will be stored in routability->HTML_message_strings and
        // eventually printed out to the HTML file:
        char HTML_message[500];
        sprintf(HTML_message, "<FONT color=\"#00CC00\">Via Congestion Sensitivity increased from %d%% to %d%% due to stagnant results <FONT size=\"1\">(via change #%d, %d stable via metrics, %d stable trace metrics)</FONT>.</FONT>",
                routability.viaCongSensitivityMetrics[old_via_cong_sensitivity_index].dynamicParameter,
                routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter,
                routability.num_viaCongSensitivity_changes, routability.num_viaCongSensitivity_stableRoutingMetrics,
                routability.num_traceCongSensitivity_stableRoutingMetrics);
        // printf("\nDEBUG: In function main, HTML_message = '%s'\n\n", HTML_message);
        add_HTML_message(HTML_message, mapInfo.current_iteration, VIA_CONG_SENS_UP, &routability);

      }  // End of if-block for changeViaCongSensitivity == INCREASE
      else if (routability.changeViaCongSensitivity[mapInfo.current_iteration] == DECREASE) {
        printf("INFO: Due to stagnant routability metrics, via congestion sensitivity reduced from %d%% to %d%% (via change #%d, via reduction #%d, %d stable via metrics, %d stable trace metrics).\n",
               routability.viaCongSensitivityMetrics[old_via_cong_sensitivity_index].dynamicParameter,
               routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter,
               routability.num_viaCongSensitivity_changes, routability.num_viaCongSensitivity_reductions,
               routability.num_viaCongSensitivity_stableRoutingMetrics, routability.num_traceCongSensitivity_stableRoutingMetrics);
        fprintf(fp_TOC, "  <UL><LI><FONT color=\"#00CC00\">Via Congestion Sensitivity reduced from %d%% to %d%% due to stagnant results (via change #%d, via reduction #%d, %d stable via metrics, %d stable trace metrics).</FONT></UL>\n",
                routability.viaCongSensitivityMetrics[old_via_cong_sensitivity_index].dynamicParameter,
                routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter,
                routability.num_viaCongSensitivity_changes, routability.num_viaCongSensitivity_reductions,
                routability.num_viaCongSensitivity_stableRoutingMetrics, routability.num_traceCongSensitivity_stableRoutingMetrics);


        // Create a temporary string variable to contain the message that will be stored in routability->HTML_message_strings and
        // eventually printed out to the HTML file:
        char HTML_message[500];
        sprintf(HTML_message, "<FONT color=\"#00CC00\">Via Congestion Sensitivity reduced from %d%% to %d%% due to stagnant results <FONT size=\"1\">(via change #%d, via reduction #%d, %d stable via metrics, %d stable trace metrics)</FONT>.</FONT>",
                routability.viaCongSensitivityMetrics[old_via_cong_sensitivity_index].dynamicParameter,
                routability.viaCongSensitivityMetrics[mapInfo.currentViaCongSensIndex].dynamicParameter,
                routability.num_viaCongSensitivity_changes, routability.num_viaCongSensitivity_reductions,
                routability.num_viaCongSensitivity_stableRoutingMetrics, routability.num_traceCongSensitivity_stableRoutingMetrics);
        // printf("\nDEBUG: In function main, HTML_message = '%s'\n\n", HTML_message);
        add_HTML_message(HTML_message, mapInfo.current_iteration, VIA_CONG_SENS_DOWN, &routability);

      }  // End of if/else-block for changeViaCongSensitivity == DECREASE
    } // End of if-statement for changeViaCongSensitivity == TRUE


    //
    // If the trace congestion sensitivity needs to be changed, then do so. Add comments
    // to the log file and HTML output file reflecting this change:
    //
    if (routability.changeTraceCongSensitivity[mapInfo.current_iteration])  {

      // We got here, so the congestion multiplier needs to be changed.

      // Temporarily record the current trace congestion sensitivity index:
      unsigned char old_trace_cong_sensitivity_index = mapInfo.currentTraceCongSensIndex;

      printf("DEBUG: Changing trace congestion sensitivity from %d%%...\n",
             routability.traceCongSensitivityMetrics[old_trace_cong_sensitivity_index].dynamicParameter);

      if (routability.changeTraceCongSensitivity[mapInfo.current_iteration] == INCREASE)  {
        mapInfo.currentTraceCongSensIndex++;
      }
      else if (routability.changeTraceCongSensitivity[mapInfo.current_iteration] == DECREASE)  {
        mapInfo.currentTraceCongSensIndex--;
      }
      else  {
        printf("\nERROR: An unexpected state occurred in which the value of routability.changeTraceCongSensitivity[%d]\n",
               mapInfo.current_iteration);
        printf("       contains an illegal value (%d). Inform the software developer of this fatal error.\n\n",
               routability.changeTraceCongSensitivity[mapInfo.current_iteration]);
        exit(1);
      }

      // Re-calculate the 'traceCongestionMultiplier' value that's used in function 'findPath()':
      mapInfo.traceCongestionMultiplier = (routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter / 100.0)
                                       * defaultCellCost * defaultEvapRate / (100.0 - defaultEvapRate) / 100.0;

      if (routability.changeTraceCongSensitivity[mapInfo.current_iteration] == INCREASE)  {
        printf("INFO: Due to stagnant routability metrics, trace congestion sensitivity increased from %d%% to %d%% (trace change #%d, %d stable trace metrics, %d stable via metrics).\n",
               routability.traceCongSensitivityMetrics[old_trace_cong_sensitivity_index].dynamicParameter,
               routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter,
               routability.num_traceCongSensitivity_changes, routability.num_traceCongSensitivity_stableRoutingMetrics,
               routability.num_viaCongSensitivity_stableRoutingMetrics);
        fprintf(fp_TOC, "  <UL><LI><FONT color=\"#00CC00\">Trace Congestion Sensitivity increased from %d%% to %d%% due to stagnant results (trace change #%d, %d stable trace metrics, %d stable via metrics).</FONT></UL>\n",
                routability.traceCongSensitivityMetrics[old_trace_cong_sensitivity_index].dynamicParameter,
                routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter,
                routability.num_traceCongSensitivity_changes, routability.num_traceCongSensitivity_stableRoutingMetrics,
                routability.num_viaCongSensitivity_stableRoutingMetrics);

        // Create a temporary string variable to contain the message that will be stored in routability->HTML_message_strings and
        // eventually printed out to the HTML file:
        char HTML_message[500];
        sprintf(HTML_message, "<FONT color=\"#00CC00\">Trace Congestion Sensitivity increased from %d%% to %d%% due to stagnant results <FONT size=\"1\">(trace change #%d, %d stable trace metrics, %d stable via metrics)</FONT>.</FONT>",
                routability.traceCongSensitivityMetrics[old_trace_cong_sensitivity_index].dynamicParameter,
                routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter,
                routability.num_traceCongSensitivity_changes, routability.num_traceCongSensitivity_stableRoutingMetrics,
                routability.num_viaCongSensitivity_stableRoutingMetrics);
        // printf("\nDEBUG: In function main, HTML_message = '%s'\n\n", HTML_message);
        add_HTML_message(HTML_message, mapInfo.current_iteration, TR_CONG_SENS_UP, &routability);

      }  // End of if-block for changeTraceCongSensitivity == INCREASE
      else if (routability.changeTraceCongSensitivity[mapInfo.current_iteration] == DECREASE) {
        printf("INFO: Due to stagnant routability metrics, trace congestion sensitivity reduced from %d%% to %d%% (trace change #%d, trace reduction #%d, %d stable trace metrics, %d stable via metrics).\n",
               routability.traceCongSensitivityMetrics[old_trace_cong_sensitivity_index].dynamicParameter,
               routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter,
               routability.num_traceCongSensitivity_changes, routability.num_traceCongSensitivity_reductions,
               routability.num_traceCongSensitivity_stableRoutingMetrics, routability.num_viaCongSensitivity_stableRoutingMetrics);
        fprintf(fp_TOC, "  <UL><LI><FONT color=\"#00CC00\">Trace Congestion Sensitivity reduced from %d%% to %d%% due to stagnant results (trace change #%d, trace reduction #%d, %d stable trace metrics, %d stable via metrics).</FONT></UL>\n",
                routability.traceCongSensitivityMetrics[old_trace_cong_sensitivity_index].dynamicParameter,
                routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter,
                routability.num_traceCongSensitivity_changes, routability.num_traceCongSensitivity_reductions,
                routability.num_traceCongSensitivity_stableRoutingMetrics, routability.num_viaCongSensitivity_stableRoutingMetrics);

        // Create a temporary string variable to contain the message that will be stored in routability->HTML_message_strings and
        // eventually printed out to the HTML file:
        char HTML_message[500];
        sprintf(HTML_message, "<FONT color=\"#00CC00\">Trace Congestion Sensitivity reduced from %d%% to %d%% due to stagnant results <FONT size=\"1\">(trace change #%d, trace reduction #%d, %d stable trace metrics, %d stable via metrics)</FONT>.</FONT>",
                routability.traceCongSensitivityMetrics[old_trace_cong_sensitivity_index].dynamicParameter,
                routability.traceCongSensitivityMetrics[mapInfo.currentTraceCongSensIndex].dynamicParameter,
                routability.num_traceCongSensitivity_changes, routability.num_traceCongSensitivity_reductions,
                routability.num_traceCongSensitivity_stableRoutingMetrics, routability.num_viaCongSensitivity_stableRoutingMetrics);
        // printf("\nDEBUG: In function main, HTML_message = '%s'\n\n", HTML_message);
        add_HTML_message(HTML_message, mapInfo.current_iteration, TR_CONG_SENS_DOWN, &routability);

      }  // End of if/else-block for changeTraceCongSensitivity == DECREASE
    } // End of if-statement for changeTraceCongSensitivity == TRUE


    //
    // If we need to enable the application of TRACE pseudo-congestion near pseudo-vias, then
    // do so. Add comments to the log file and HTML output file reflecting this change:
    //
    if (routability.enablePseudoTraceCongestion[mapInfo.current_iteration])  {

      // 'HTML_full_message' is a text string to be added to routability->HTML_message_strings and eventually
      // printed out to the HTML file for adding TRACE pseudo-congestion. 'HTML_partial_message' is a text
      // string that is sequentially concatenated to 'HTML_full_message' to create the latter.
      char HTML_full_message[1024];             // Allocate memory for HTML_full_message text string.
      strncpy(HTML_full_message, "\0", 1);      // Initialize HTML_full_message text string.
      char HTML_partial_message[512];           // Allocate memory for HTML_partial_message text string.
      strncpy(HTML_partial_message, "\0", 1);   // Initialize HTML_partial_message text string.

      // Variables 'num_pseudoPathsLayers_toggled_on' and 'num_pseudoPathsLayers_continued_on'
      // keep track of how many combinations of pseudo-paths and routing layers were toggled on
      // (or continued 'on') for the deposition of TRACE pseudo-congestion around pseudo-vias
      // for paths with DRCs (on the layers with those DRCs):
      int num_pseudoPathsLayers_toggled_on  = 0;
      int num_pseudoPathsLayers_continued_on = 0;

      // Variable 'num_layers_with_pseudoCongestion' keeps track of how many routing layers
      // will have TRACE pseudo-congestion deposited around crowded pseudo-vias.
      // Array 'pseudoCongestion_by_layer' contains Boolean elements specifying whether
      // any pseudo-paths had TRACE congestion deposited on a given layer around pseudo-vias.
      unsigned char num_layers_with_pseudoCongestion = 0;
      unsigned char pseudoCongestion_by_layer[maxRoutingLayers];
      for (int layer = 0; layer < maxRoutingLayers; layer++)  {
        pseudoCongestion_by_layer[layer] = FALSE;  // Initialize elements to FALSE
      }

      //
      // Determine whether to deposit TRACE pseudo-congestion for each combination
      // of pseudo-path and routing layer.
      for (int pseudoPath = mapInfo.numPaths; pseudoPath < max_routed_nets; pseudoPath++)  {

        // Iterate over all layers except the top and bottom layers, since we never want to
        // deposit TRACE pseudo-congestion on these two layers:
        for (int layer = 1; layer < mapInfo.numLayers - 1; layer++)  {

          unsigned char DRCs_on_path_and_layer = FALSE;
          unsigned char DRCs_on_path_and_adjacent_topBottom_layer = FALSE;

          if ((routability.recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo.numPaths][layer] & 0x000FFFFF) == 0x000FFFFF)  {
            DRCs_on_path_and_layer = TRUE;
            printf("DEBUG: DRCs on layer %d are associated with pseudo-path %d.\n", layer, pseudoPath);
          }
          else if (   (layer == 1)
                   && (routability.recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo.numPaths][0] & 0x000FFFFF) == 0x000FFFFF)  {
            DRCs_on_path_and_adjacent_topBottom_layer = TRUE;
            printf("DEBUG: DRCs on layer #0 (bottom layer) are associated with pseudo-path %d.\n", pseudoPath);
          }
          else if (   (layer == mapInfo.numLayers - 2)
                   && (routability.recent_DRC_flags_by_pseudoPath_layer[pseudoPath - mapInfo.numPaths][mapInfo.numLayers - 1] & 0x000FFFFF) == 0x000FFFFF)  {
            DRCs_on_path_and_adjacent_topBottom_layer = TRUE;
            printf("DEBUG: DRCs on layer #%d (top layer) are associated with pseudo-path %d.\n", mapInfo.numLayers - 1, pseudoPath);
          }

          if (DRCs_on_path_and_layer
             || (DRCs_on_path_and_adjacent_topBottom_layer
                          && (mapInfo.addPseudoTraceCongestionNearVias[pseudoPath][layer] == TRUE)))  {

            // We got here, so at least one of the child diff-pair paths of 'pseudoPath' contains
            // DRCs on routing layer 'layer' or an adjacent top/bottom layer. So we enable the
            // the deposition of TRACE pseudo-congestion around the pseudo-vias for this pseudo-path
            // on routing layer 'layer':
            printf("DEBUG: TRACE pseudo-congestion will be added for pseudo-path %d on layer %d\n", pseudoPath, layer);

            // So we enable the application of TRACE pseudo-congestion around
            // pseudo-vias for this pseudo-path on routing layer 'layer':
            if (mapInfo.addPseudoTraceCongestionNearVias[pseudoPath][layer] == FALSE)  {
              mapInfo.addPseudoTraceCongestionNearVias[pseudoPath][layer] = TRUE;
              num_pseudoPathsLayers_toggled_on++;
              printf("DEBUG: TRACE pseudo-congestion will START being added for pseudo-path %d on layer %d\n", pseudoPath, layer);
            }
            else  {
              num_pseudoPathsLayers_continued_on++;
              printf("DEBUG: TRACE pseudo-congestion will CONTINUE being added for pseudo-path %d on layer %d\n", pseudoPath, layer);
            }

            // Flag the current layer as one on which this feature was toggled on or off
            pseudoCongestion_by_layer[layer] = TRUE;

          }  // End of if-block for finding DRCs on 'layer' for at least 1 diff-pair child of 'pseudoPath'
        }  // End of for-loop for index 'layer'
      }  // End of for-loop for index 'pseudoPath'

      // Count how many layers will have TRACE pseudo-congestion deposited on them:
      for (int layer = 1; layer < mapInfo.numLayers - 1; layer++)  {
        if (pseudoCongestion_by_layer[layer])  {
          num_layers_with_pseudoCongestion++;
        }  // End of if-block
      }  // End of for-loop for index 'layer'


      // If this iteration is the first one in which TRACE pseudo-congestion will start being
      // added to a given pseudo-path on a given routing layer, then we reset the routing
      // metrics for all values of the via congestion sensitivity. This is done because the
      // application of TRACE pseudo-congestion makes previously calculated routing metrics
      // obsolete:
      if (num_pseudoPathsLayers_toggled_on > 0)  {
        for (int cong_sensitivity = 0; cong_sensitivity < NUM_CONG_SENSITIVITES; cong_sensitivity++)  {
          routability.viaCongSensitivityMetrics[cong_sensitivity].iterationOfMeasuredMetrics = 0;
          // Also reset the routing metrics to zero to help with debugging:
          routability.viaCongSensitivityMetrics[cong_sensitivity].fractionIterationsWithoutDRCs = 0.0;
          routability.viaCongSensitivityMetrics[cong_sensitivity].avgNonPseudoNetsWithDRCs      = 0.0;
          routability.viaCongSensitivityMetrics[cong_sensitivity].stdErrNonPseudoNetsWithDRCs   = 0.0;
          routability.viaCongSensitivityMetrics[cong_sensitivity].avgNonPseudoRoutingCost       = 0.0;
          routability.viaCongSensitivityMetrics[cong_sensitivity].stdErrNonPseudoRoutingCost    = 0.0;
        }  // End of for-loop for index 'cong_sensitivity'
        printf("INFO: Routing metrics have been reset for all values of via congestion sensitivities due to the new application of TRACE pseudo-congestion.\n");
      }  // End of if-block for newly toggled-on pseudo-path-layers


      // Notify the user that TRACE pseudo-congestion will **START** being added for selected
      // paths on selected layers that exhibited DRCs:
      if (num_pseudoPathsLayers_toggled_on > 1)  {
        // Issue notifications to user using plural nouns:
        printf("INFO: Due to stagnant routing metrics, TRACE pseudo-congestion will be deposited on %d combinations of pseudo-paths and routing-layers with DRCs to repel traces near pseudo-vias.\n",
               num_pseudoPathsLayers_toggled_on);
        fprintf(fp_TOC, "  <UL><LI><FONT color=\"#00CC00\">Due to stagnant routing metrics, TRACE pseudo-congestion will be deposited on %d combinations of pseudo-paths and routing-layers with DRCs to repel traces near pseudo-vias.</FONT></UL>\n",
               num_pseudoPathsLayers_toggled_on);

        // Concatenate additional information to the HTML message that will be printed to the HTML page:
        sprintf(HTML_partial_message, "<FONT color=\"#00CC00\">Due to stagnant routing metrics, TRACE pseudo-congestion will be deposited on %d combinations<BR>of pseudo-paths and routing-layers with DRCs to repel traces near pseudo-vias.</FONT>",
               num_pseudoPathsLayers_toggled_on);
        strcat(HTML_full_message, HTML_partial_message);

      }  // End of if-block for multiple combinations
      else if (num_pseudoPathsLayers_toggled_on == 1)  {
        // Issue notifications to user using singular nouns:
        printf("INFO: Due to stagnant routing metrics, TRACE pseudo-congestion will be deposited on %d combination of pseudo-path and routing-layer with DRCs to repel traces near a pseudo-via.\n",
               num_pseudoPathsLayers_toggled_on);
        fprintf(fp_TOC, "  <UL><LI><FONT color=\"#00CC00\">Due to stagnant routing metrics, TRACE pseudo-congestion will be deposited on %d combination of pseudo-path and routing-layer with DRCs to repel traces near a pseudo-via.</FONT></UL>\n",
               num_pseudoPathsLayers_toggled_on);

        // Concatenate additional information to the HTML message that will be printed to the HTML page:
        sprintf(HTML_partial_message, "<FONT color=\"#00CC00\">Due to stagnant routing metrics, TRACE pseudo-congestion will be deposited on %d combination<BR>of pseudo-path and routing-layer with DRCs to repel traces near a pseudo-via.</FONT>",
               num_pseudoPathsLayers_toggled_on);
        strcat(HTML_full_message, HTML_partial_message);

      }  // End of if-block for a single combination

      // Notify the user that TRACE pseudo-congestion will **CONTINUE** being added for selected
      // paths on selected layers that exhibited DRCs:
      if (num_pseudoPathsLayers_continued_on > 1)  {
        printf("INFO: Due to stagnant routing metrics, TRACE pseudo-congestion will again be deposited on %d combinations of pseudo-paths and routing-layers with DRCs to repel traces near pseudo-vias.\n",
               num_pseudoPathsLayers_continued_on);
        fprintf(fp_TOC, "  <UL><LI>Due to stagnant routing metrics, TRACE pseudo-congestion will again be deposited on %d combinations of pseudo-paths and routing-layers with DRCs to repel traces near pseudo-vias.</UL>\n",
                num_pseudoPathsLayers_continued_on);

        // Concatenate additional information to the HTML message that will be printed to the HTML page:
        sprintf(HTML_partial_message, "Due to stagnant routing metrics, TRACE pseudo-congestion will again be deposited on %d combinations of pseudo-paths and routing-layers with DRCs to repel traces near pseudo-vias.",
                num_pseudoPathsLayers_continued_on);
        strcat(HTML_full_message, HTML_partial_message);

      }  // End of if-block for multiple combinations
      else if (num_pseudoPathsLayers_continued_on == 1)  {
        printf("INFO: Due to stagnant routing metrics, TRACE pseudo-congestion will again be deposited on %d combination of pseudo-path and routing-layer with DRCs to repel traces near a pseudo-via.\n",
               num_pseudoPathsLayers_continued_on);
        fprintf(fp_TOC, "  <UL><LI>Due to stagnant routing metrics, TRACE pseudo-congestion will again be deposited on %d combination of pseudo-path and routing-layer with DRCs to repel traces near a pseudo-via.</UL>\n",
                num_pseudoPathsLayers_continued_on);

        // Concatenate additional information to the HTML message that will be printed to the HTML page:
        sprintf(HTML_partial_message, "Due to stagnant routing metrics, TRACE pseudo-congestion will again be deposited on %d combination of pseudo-path and routing-layer with DRCs to repel traces near a pseudo-via.",
                num_pseudoPathsLayers_continued_on);
        strcat(HTML_full_message, HTML_partial_message);

      }  // End of if-block for a single combination


      // Notify the user of which layers will have TRACE pseudo-congestion turned on or continued:
      if (num_layers_with_pseudoCongestion)  {
        if (num_layers_with_pseudoCongestion > 1)  {
          printf("INFO: These changes will occur on routing layers:");
          fprintf(fp_TOC, "  <UL><LI>These changes will occur on routing layers:");

          // Concatenate additional information to the HTML message that will be printed to the HTML page:
          strcat(HTML_full_message, "<UL><LI>These changes will occur on routing layers:");

        }
        else if (num_layers_with_pseudoCongestion == 1)  {
          printf("INFO: These changes will occur on routing layer");
          fprintf(fp_TOC, "  <UL><LI>These changes will occur on routing layer");

          // Concatenate additional information to the HTML message that will be printed to the HTML page:
          strcat(HTML_full_message, "<UL><LI>These changes will occur on routing layer");

        }
        for (int layer = 0; layer < mapInfo.numLayers; layer++)  {
          if (pseudoCongestion_by_layer[layer] == TRUE)  {
            printf(" %s", user_inputs.routingLayerNames[layer]);
            fprintf(fp_TOC, "&nbsp;%s", user_inputs.routingLayerNames[layer]);

            // Concatenate the layer names to the HTML message that will be printed to the HTML page:
            sprintf(HTML_partial_message, "&nbsp;%s", user_inputs.routingLayerNames[layer]);
            strcat(HTML_full_message, HTML_partial_message);

          }  // End of if-block
        }  // End of for-loop for index 'layer'
        printf(".\n");
        fprintf(fp_TOC, ".</UL>\n");
        strcat(HTML_full_message, "</LI></UL>\n");
      }  // End of if-block for (num_layers_with_pseudoCongestion > 0)


      // Send the temporary string variable 'HTML_full_message' routability->HTML_message_strings and
      // eventually printed out to the HTML file:
      // printf("\nDEBUG: In function main, HTML_full_message = '%s'\n\n", HTML_full_message);
      add_HTML_message(HTML_full_message, mapInfo.current_iteration, ADD_PSEUDO_CONG, &routability);

      //
      // Add congestion near pseudo-vias intended to repel pseudo-TRACE routing on routing layers that
      // have DRCs:
      //
      addTraceCongestionNearPseudoViasWithDRCs(&mapInfo, pathLengths, pathCoords, cellInfo, &routability, &user_inputs);

    }  // End of if-block for toggleTraceCongestionNearPseudoVias == TRUE


    // Calculate cumulative (wall-clock) time to complete this iteration:
    routability.iteration_cumulative_time[mapInfo.current_iteration] = (int) (time(NULL) - start_autorouter);
    printf("INFO: Iteration %d took %'d seconds.\n", mapInfo.current_iteration,
           routability.iteration_cumulative_time[mapInfo.current_iteration] - routability.iteration_cumulative_time[mapInfo.current_iteration - 1]);


    //
    // Replace the 'routingStatus.html' file with a new file by the same name that contains the results from the current iteration.
    // While the new file is being created, rename the old 'routingStatus.html' file to a temporary file before deleting it.
    //
    rename("routingStatus.html", "routingStatus_old.html");

    //
    // DEBUG code follows:
    //
    // printf("\nDEBUG: -----------------------------------------------------------\n");
    // if (routability.num_HTML_messages == 0)  {
    //   printf("DEBUG: At iteration %d, there are no HTML messages in function main() before calling create_routingStatus_HTML_file:\n",
    //          mapInfo.current_iteration);
    // }
    // else  {
    //   printf("DEBUG: At iteration %d, printing out %d HTML messages from function main() before calling create_routingStatus_HTML_file:\n",
    //          mapInfo.current_iteration, routability.num_HTML_messages);
    //   for (int msg_num = 0; msg_num < routability.num_HTML_messages; msg_num++)  {
    //     printf("DEBUG:    msg #%d for iteration %d has length %lu: '%s'\n", msg_num, routability.HTML_message_iter_nums[msg_num],
    //            strlen(routability.HTML_message_strings[msg_num]), routability.HTML_message_strings[msg_num]);
    //   }
    // }
    // printf("DEBUG: -----------------------------------------------------------\n");
    //
    // End of DEBUG code
    //


    if (create_routingStatus_HTML_file(input_filename, "routingStatus.html", &mapInfo, &routability, &user_inputs, shapeTypeNames, adequateSolutionFound, DRC_free_threshold, num_threads) == 0)  {
      if (remove("routingStatus_old.html") != 0)  {
        printf("\n\nERROR: The file 'routingStatus_old.html' was not successfully deleted in function main(). This behavior is not expected.\n");
        printf(    "       Please inform the software developer of this fatal error message.\n\n");
        exit(1);
      }
    }
    else  {
      printf("\n\nERROR: The file 'routingStatus.html' was not successfully created in function main(). This behavior is not expected.\n");
      printf(    "       Please inform the software developer of this fatal error message.\n\n");
      exit(1);
    }


  }  // End of main while-loop for (current_iteration <= maxIterations) && (! adequateSolutionFound)
  //
  // The above line is the end of the main loop for the iterative rip-up and re-route algorithm.
  //


  //
  // Print final status to log file and HTML file:
  //
  fprintf(fp_TOC, "</UL>\n"); 
  if (designRuleConflicts)  {
    printf("\n\nERROR: Conflicts were detected between design-rule zones on adjacent layers. Correct these and re-start the program.\n\n");
    fprintf(fp_TOC, "<FONT color=\"red\">ERROR: Conflicts were detected between <A href=\"designRules.html\">design-rule zones on adjacent layers</A>. \n");
    fprintf(fp_TOC, "Correct these errors and re-start the program.</FONT><BR>\n<BR>\n");
  }
  else if ((mapInfo.current_iteration >= user_inputs.maxIterations) && (! adequateSolutionFound))  {
    printf("INFO: %d DRC-free iterations were found (%d required).\n",
           routability.cumulative_DRCfree_iterations[mapInfo.current_iteration], DRC_free_threshold);
    printf("\n\nERROR: No solution was found after reaching the maximum number of iterations (%d) after %'d seconds, exploring %'lu cells.\n",
            user_inputs.maxIterations, routability.iteration_cumulative_time[mapInfo.current_iteration], routability.total_explored_cells);
    printf(    "       The iteration with the lowest-cost routing results is iteration %d.\n\n", routability.lowest_cost_iteration);
    fprintf(fp_TOC, "<FONT color=\"red\"><B>ERROR:</B></FONT> No solution was found after reaching the maximum number of iterations (%d) in ", user_inputs.maxIterations);
    if (routability.iteration_cumulative_time[mapInfo.current_iteration] > 1)
      fprintf(fp_TOC, "%'d seconds, exploring %'lu cells.\n", routability.iteration_cumulative_time[mapInfo.current_iteration], routability.total_explored_cells);
    else if (routability.iteration_cumulative_time[mapInfo.current_iteration] == 1)
      fprintf(fp_TOC, "~%'d second, exploring %'lu cells.\n", routability.iteration_cumulative_time[mapInfo.current_iteration], routability.total_explored_cells);
    else
      fprintf(fp_TOC, "<1 second, exploring %'lu cells.\n", routability.total_explored_cells);
    fprintf(fp_TOC, "The lowest-cost routing results are in <A href=\"iteration%04d.html\">iteration %d</A>. %d DRC-free iterations were found (%d required).<BR>\n<BR>",
            routability.lowest_cost_iteration, routability.lowest_cost_iteration, routability.cumulative_DRCfree_iterations[mapInfo.current_iteration], DRC_free_threshold);
  }
  else {
    printf("\n\nINFO: Solution was found in %'d seconds with %'lu cells explored. The lowest-cost routing results are in iteration %d.\n",
           routability.iteration_cumulative_time[mapInfo.current_iteration], routability.total_explored_cells, routability.lowest_cost_iteration);
    printf(    "INFO: %d DRC-free iterations were found (%d required).\n",
           routability.cumulative_DRCfree_iterations[mapInfo.current_iteration], DRC_free_threshold);
    fprintf(fp_TOC, "<FONT color=\"black\"><B>Program completed successfully in ");
    if (routability.iteration_cumulative_time[mapInfo.current_iteration] > 1)
      fprintf(fp_TOC, "%'d seconds after exploring %'lu cells.\n", routability.iteration_cumulative_time[mapInfo.current_iteration], routability.total_explored_cells);
    else if (routability.iteration_cumulative_time[mapInfo.current_iteration] == 1)
      fprintf(fp_TOC, "~%'d second after exploring %'lu cells.\n", routability.iteration_cumulative_time[mapInfo.current_iteration], routability.total_explored_cells);
    else
      fprintf(fp_TOC, "<1 second after exploring %'lu cells.\n", routability.total_explored_cells);
    fprintf(fp_TOC,"The lowest-cost routing results are in <A href=\"iteration%04d.html\">iteration %d</A>. %d DRC-free iterations were found (%d required).</B></FONT><BR>\n<BR>",
            routability.lowest_cost_iteration, routability.lowest_cost_iteration, routability.cumulative_DRCfree_iterations[mapInfo.current_iteration], DRC_free_threshold);
  }
  fprintf(fp_TOC, "</BODY>\n</HTML>\n");
  fclose(fp_TOC); // Close the output HTML file

  // Free memory associated with the 'routability' data structure that was used for
  // the main map:
  freeMemory_routability(&routability, &mapInfo);

  // If the user defined any diff-pair nets, then free memory associated with the
  // subMapRoutability variables that were used for  the diff-pair sub-maps:
  if (user_inputs.num_diff_pair_nets > 0)  {
    for (int wire_config = NOT_SWAPPED; wire_config <= SWAPPED; wire_config++)  {
      freeMemory_routability(&(subMapRoutability[wire_config]), &genericSubMapInfo);
    }  // End of for-loop for index 'wire_config' (0 to 1)
  }  // End of if-block for num_diff_pair_nets > 0

  // Free memory associated with the 'cellInfo' 3D array:
  freeMemory_cellInfo(&mapInfo, cellInfo);

  // Free memory used by 'pathCoords' and 'contigPathCoords' array of arrays:
  endPathfinder(max_routed_nets, pathCoords, contigPathCoords);

  // Free memory associated with user's input values:
  freeMemory_input_values(&user_inputs);
  freeMemory_mapInfo(&mapInfo);

  // Free memory associated with the pathFinding arrays, with one 'pathFinding'
  // element for each thread:
  for (int i = 0; i < num_simultaneous_pathFinding; i++)  {
    freePathFindingArrays(&pathFinding[i], &mapInfo);
  }  // End of for-loop for index 'i' (0 to num_simultaneous_pathFinding-1)
  // printf("DEBUG: pathFinding arrays were free'd for up to %d simultaneous path-finding threads.\n", num_simultaneous_pathFinding);

  // Free memory allocated within this 'main' program:
  free(input_filename);                input_filename        = NULL;
  free(sequence);                      sequence              = NULL;
  free(pathLengths);                   pathLengths           = NULL;
  free(pathCoords);                    pathCoords            = NULL;
  free(contiguousPathLengths);         contiguousPathLengths = NULL;
  free(contigPathCoords);              contigPathCoords      = NULL;

  // Print a time-stamp to STDOUT:
  tim = time(NULL);
  now = localtime(&tim);
  printf("Date-stamp: %02d-%02d-%d, %02d:%02d:%02d *************************\n",
        now->tm_mon+1, now->tm_mday, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

  return(0);
}  // End of 'main'
