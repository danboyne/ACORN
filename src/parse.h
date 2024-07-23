//-----------------------------------------------------------------------------
// Name: pre_process_input_file
// Desc: Perform a coarse parsing of the user's input file in order to determine
//       the following:
//          -  Number of nets
//          -  Number of nets that are diff-pairs
//          -  Number of pseudo nets, defined as half the number of diff-pair nets
//          -  Number of nets with net-specific design rules
//          -  Number of block/unblock instructions
//          -  Number of design-rule sets (design_rule_set statements)
//          -  Number of subsets in each design-rule set
//          -  Number of design-rule zones (DR_zone statements)
//          -  Number of trace_cost_zone statements in the file
//          -  Number of via_cost_zone statements in the file
//          -  Number of pin-swap instructions
//          -  Number of routing layers
//-----------------------------------------------------------------------------
void pre_process_input_file(char *input_filename, InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: initialize_input_values
// Desc: Allocate memory for variables in the input_values structure.
//-----------------------------------------------------------------------------
void initialize_input_values(InputValues_t *input_values);


//-----------------------------------------------------------------------------
// Name: freeMemory_input_values
// Desc: Free the memory that was allocated in function 'initialize_input_values'.
//-----------------------------------------------------------------------------
void freeMemory_input_values(InputValues_t *input_values);


//-----------------------------------------------------------------------------
// Name: set_costs_to_base_values
// Desc: Sets the following costs to their base costs in the user_inputs structure,
//       for all trace and via cost-multiplier indices:
//         1. cell_cost
//         2. diag_cost
//         3. knight_cost
//         4. vert_cost
//
//       This function does *not* change the user-defined multipliers. That is,
//       the above four cost values can be re-calculated using the user-defined
//       multipliers, if desired.
//-----------------------------------------------------------------------------
void set_costs_to_base_values(InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: set_costs_to_userDefined_values
// Desc: Sets the following costs to the values defined by the user, accounting 
//       for the user-defined cost-multipliers: 
//         1. cell_cost
//         2. diag_cost
//         3. knight_cost
//         4. vert_cost
//-----------------------------------------------------------------------------
void set_costs_to_userDefined_values(InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: verifyDiffPairTerminals
// Desc: For each diff-pair net that is not in a pin-swap zone, verify that the
//       two starting terminals and two ending terminals are on the same layer
//       and within the same design-rule zone. Verify that the two starting
//       terminals and two ending terminals are within a reasonable distance
//       of each other. Verify that there are no other non-diff-pair terminals
//       between the two starting and two ending terminals. Calculate the pitch
//       of diff-pairs' start- and end terminals. (If the start-terminals are
//       located in a pin-swap zone, then this pitch is meaningless, and is
//       assigned a value of zero.)
//
//       For each diff-pair net that is located in a pin-swap zone, verify that
//       the two start-terminals are located in the same swap-zone as the
//       associated pseudo-net's start-terminal.
//
//       For all diff-pair nets, verify that the pseudo-nets' terminals are
//       not located within a user-defined barrier, or in close proximity to
//       such barriers. Verify that the 'isPNswappable' flag is set for both
//       nets if the user set this flag for either net.
//-----------------------------------------------------------------------------
void verifyDiffPairTerminals(const InputValues_t *user_inputs, CellInfo_t ***cellInfo, MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: verifyAllTerminals
// Desc: For each net that is not in a pin-swap zone, verify that there
//       are no other terminals within a distance of a trace-width
//       plus a trace-to-trace spacing (a 'trace pitch').
//-----------------------------------------------------------------------------
void verifyAllTerminals(const InputValues_t *user_inputs, CellInfo_t ***cellInfo, MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: parse_input_file
// Desc: Reads input file 'filename' and parses data from file. Data is
//       written into a structure of type 'InputValues_t', with some data
//       also written to a structure of type 'MapInfo_t'.
//-----------------------------------------------------------------------------
void parse_input_file(char *input_filename, InputValues_t *user_inputs, MapInfo_t *mapInfo);


