//-----------------------------------------------------------------------------
// Name: defineBarriers
// Desc: Modifies the 'cellInfo' 3D matrix based on the BLOCK/UNBLOCK
//       statements described in the 'user_inputs' data structure.
//-----------------------------------------------------------------------------
void defineBarriers(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                           InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: defineProximityZones
// Desc: Cells are made unwalkable a half-linewidth or via radius away from
//       (1) user-defined barriers, and (2) the perimeter of the map, and
//       (3) pin-swap zones. Cells in pin-swap zones are never part of a
//       proximity zone.
//-----------------------------------------------------------------------------
void defineProximityZones(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                          InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: defineCellDesignRules
// Desc: Modifies the 'cellInfo' 3D matrix based on the DR_zone
//       statements described in the 'user_inputs' data structure. 
//
//       This function defines the design-rule set number for each cell
//       in the map. If no design-rule zones were defined by the user,
//       then all cells are assigned the default design-rule set
//       of zero.
//-----------------------------------------------------------------------------
void defineCellDesignRules(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                           InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: defineCellCosts
// Desc: Modifies the 'cellInfo' 3D matrix based on the 'trace_cost_multiplier'
//       and 'via_cost_multiplier' statements described in the 'user_inputs' 
//       data structure. 
//
//       This function defines the cost-zone number for each cell
//       in the map. If no cost-zones were defined by the user,
//       then all cells are assigned the default design-rule set
//       of zero, whose multiplier is one.
//-----------------------------------------------------------------------------
void defineCellCosts(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                           InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: definePinSwapZones
// Desc: Modifies the 'cellInfo' 3D matrix based on the PIN_SWAP/NO_PIN_SWAP
//       statements described in the 'user_inputs' data structure. Each cell
//       is flagged as either being in a pin-swappable zone, or not being in 
//       such a zone. For those cells in a pin-swappable zone, a unique number for
//       that zone is defined in this function. A pin-swap zone is defined as
//       all the cells that share a common side or face (in all 3 dimensions).
//
//       Function also maps the swap-zone numbers to the starting-coordinates of
//       each path that is within each pin-swap zone. If a path's ending-
//       coordinates are within a pin-swap zone, then the function swaps the
//       path's starting and ending coordinates, and ensures that that the
//       isPNswappable flag is set for the nets, and the start-terminal's
//       pitch values are set to zero.
//-----------------------------------------------------------------------------
void definePinSwapZones(CellInfo_t ***cellInfo, MapInfo_t *mapInfo,
                        InputValues_t *user_inputs);

