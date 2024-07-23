#ifndef PARSE_LIBRARY_H

#define PARSE_LIBRARY_H


//-----------------------------------------------------------------------------
// Name: compile_regex
// Desc: Compile regular expression given by regex_string, and place result
//       into compiled_regex.
//-----------------------------------------------------------------------------
void compile_regex(char *regex_string, regex_t *compiled_regex);


//-----------------------------------------------------------------------------
// Name: calc_2D_Pythagorean_distance_floats
// Desc: Calculate the distance between (x1, y1) and (x2, y2) using the
//       Pythagorean formula. This function does not account for the separation
//       in the z-dimension, and is used for calculating the distance between
//       floating-point-based coordinates.
//-----------------------------------------------------------------------------
float calc_2D_Pythagorean_distance_floats(const float x1, const float y1, const float x2, const float y2);


//-----------------------------------------------------------------------------
// Name: copyDesignRuleSubset
// Desc: Copy user-supplied design-rule parameters from a source design-rule
//       subset to a destination design-rule subset. This function does not
//       copy derived/calculated parameters that are not supplied in the
//       user's input file.
//-----------------------------------------------------------------------------
void copyDesignRuleSubset(InputValues_t *user_inputs, int source_set, int source_subset,
                          int destination_set, int destination_subset);


//-----------------------------------------------------------------------------
// Name: verifyDiffPairPitch
// Desc: Verify that the diff-pair pitch for a net is equal to the diff-pair
//       pitch for that net's partner net in each design-rule set and subset.
//-----------------------------------------------------------------------------
void verifyDiffPairPitch(InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: mapPseudoNets
// Desc: Map the user-defined diff-pair nets to pseudo nets, storing the results
//       in array:
//
//         user_inputs->diffPairToPseudoNetMap[net_number] = pseudo_net_number
//
//       Also, map the pseudo nets back to the user-defined diff-pair nets,
//       storing the results in the following two arrays:
//
//         user_inputs->pseudoNetToDiffPair_1[pseudo_net_number] = diff_pair_net_1
//         user_inputs->pseudoNetToDiffPair_2[pseudo_net_number] = diff_pair_net_2
//
//-----------------------------------------------------------------------------
void mapPseudoNets(InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: checkTerminalLocations
// Desc: Confirm that start- and end-locations are within the map. Also,
//       calculate the coordinates of pseudo nets' terminals, which are the
//       midpoints of the corresponding differential-pair nets.
//-----------------------------------------------------------------------------
void checkTerminalLocations(InputValues_t *user_inputs, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: mapDesignRuleSubsets
// Desc: Create 2-dimensional mapping structure 'user_inputs->designRuleSubsetMap'
//       that maps net numbers and design-rule sets to the correct design-rule
//       subset:
//
//        user_inputs->designRuleSubsetMap[net_num][DR_set_num] = DR_subset_num
//
//       Also populate the Boolean flags of the following 2-dimensional array to
//       reflect whether a design-rule subset is used by any nets:
//
//        user_inputs->DR_subsetUsed[DR_set_num][DR_subset_num] = TRUE or FALSE
//
//-----------------------------------------------------------------------------
void mapDesignRuleSubsets(InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: calc_XYZ_cell_coordinates
// Desc: Convert the starting and ending (x,y) coordinates from microns to
//       cell units, and calculate the Z-coordinates based on the names of
//       the starting- and ending layer names.
//-----------------------------------------------------------------------------
void calc_XYZ_cell_coordinates(InputValues_t *user_inputs, const MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: getDiffPairPartnerAndPitch
// Desc:  For each net #i that is part of a differential pair, determine the
//        number j of the net's partner and save this in variable
//        'user_inputs->diffPairPartner[i] = j'.  Also, for each diff-pair net,
//        assign the pitch (in microns and cell units) for each design-rule set.
//-----------------------------------------------------------------------------
void getDiffPairPartnerAndPitch(InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: calc_congestion_adder
// Desc: Calculate a floating-point 'adder' by which a congestion radius is
//       augmented to ensure that a foreign path-center passing through a
//       discrete cell just beyond a congestion radius of a path-center would
//       not cause a design-rule violation between the two path-centers. The
//       square of the DRC radius between the path-centers is DRC_radius_squared.
//       All units are in units of cells or cells^2.
//-----------------------------------------------------------------------------
float calc_congestion_adder(float shape_radius, float baseline_cong_radius, float DRC_radius_squared);


//-----------------------------------------------------------------------------
// Name: calc_diffPair_design_rules
// Desc: Calculate optimized values of the diff-pair half-pitches of two shapes,
//       A and B, accounting for rounding errors due to discretization of continuous,
//       user-defined values into approximated, discrete, grid-based values.
//
//       Inputs to the function are:
//         o  Reference to the nominal diff-pair half-pitch of shape A, in cell units
//            (* diffPair_halfPitch_A)
//         o  Reference to the nominal diff-pair half-pitch of shape B, in cell units
//            (* diffPair_halfPitch_B)
//         o  Nominal diff-pair shape half-width (radius) for shape A, in cell units
//            (diffPair_shapeRadius_A)
//         o  Nominal diff-pair shape half-width (radius) for shape B, in cell units
//            (diffPair_shapeRadius_B)
//         o  DRC_radius[A][B] between diff-pair partner-shapes A and B, which is equal
//            to radius[B] + spacing[A][B]. The variable name is DRC_radius_AB.
//         o  DRC_radius[B][A] between diff-pair partner-shapes A and B, which is equal
//            to radius[A] + spacing[A][B]. The variable name is DRC_radius_BA.
//
//       Outputs from the function are:
//         o  Reference to the adjusted diff-pair half-pitch for shape A, including any
//            necessary adder to avoid design-rule violations, in cell units
//            (* diffPair_halfPitch_A)
//         o  Reference to the adjusted diff-pair half-pitch for shape B, including any
//            necessary adder to avoid design-rule violations, in cell units
//            (* diffPair_halfPitch_B)
//         o  Reference to the half-width (radius) of the pseudo-structure using shape A,
//            i.e., the largest distance from the pseudo-path centerline that is occupied
//            by shape A (* pseudo_halfWidth_A)
//         o  Reference to the half-width (radius) of the pseudo-structure using shape B,
//            i.e., the largest distance from the pseudo-path centerline that is occupied
//            by shape B (* pseudo_halfWidth_B)
//
//       If the function attempts to increase the diff-pair half-pitch values
//       or the pseudo-half-width values by more than 'max_adder_value_cells',
//       the program issues a fatal error message and exits. This prevents the
//       function from executing an infinite loop.
//-----------------------------------------------------------------------------
void calc_diffPair_design_rules(float diffPair_shapeRadius_A, float diffPair_shapeRadius_B,
                                float DRC_radius_AB, float DRC_radius_BA,
                                float * diffPair_halfPitch_A, float * diffPair_halfPitch_B,
                                float * pseudo_halfWidth_A, float * pseudo_halfWidth_B,
                                float max_adder_value_cells);


//-----------------------------------------------------------------------------
// Name: createUsefulDesignRuleInfo
// Desc: For each design-rule subset, convert the design-rule parameters
//       to 'cell' dimensions from microns. Also, compute useful parameters
//       for each design-rule set and subset that are derived from
//       user-supplied values.
//-----------------------------------------------------------------------------
void createUsefulDesignRuleInfo(const MapInfo_t *mapInfo, InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: defineDefaultDesignRuleSet
// Desc: If the input file contains no user-defined design-rule sets, then
//       define a default design-rule set. In this set, all spaces, trace
//       widths, and via diameters are set to the equivalent of 1 cell.
//-----------------------------------------------------------------------------
void defineDefaultDesignRuleSet(InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: verify_net_designRule_consistency
// Desc: Verify that design-rule exceptions that contain the ‘diff_pair_pitch’
//       keyword are not used for nets that don’t contain a diff-pair partner net.
//-----------------------------------------------------------------------------
void verify_net_designRule_consistency(InputValues_t *user_inputs);



#endif
