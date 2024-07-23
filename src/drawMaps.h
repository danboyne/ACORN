//-----------------------------------------------------------------------------
// Name: start_HTML_table_of_contents
// Desc: Open an HTML output file that will contain key output data and 
//       hyperlinks to detailed information for each iteration.
//-----------------------------------------------------------------------------
FILE * start_HTML_table_of_contents(const char *input_filename, const InputValues_t *user_inputs,
                                    const MapInfo_t *mapInfo,  const int DRC_free_threshold, int num_threads);


//-----------------------------------------------------------------------------
// Name: get_RGBA_values_for_pixel
// Desc: Calculate the red, green, blue, and opacity values for a pixel
//       represented by the values (x, y, z_map) in the routing map, and
//       (equivalently) at the coordinate (x, y, z_PNG) among the PNG
//       maps, where z_map = z_PNG / 2.
//-----------------------------------------------------------------------------
///   void get_RGBA_values_for_pixel(int x, int y, int z_PNG, int z_map, int isViaLayer,
///                                  CellInfo_t ***cellInfo, const MapInfo_t *mapInfo,
///                                  unsigned char ***pathTerminals,
///                                  int *red, int *green, int *blue, int *opacity);


//-----------------------------------------------------------------------------
// Name: makePngPathThumbnail
// Desc: Create a single PNG file that overlays all the routing and via layers
//       into a single image with maximum height or width of 'maxDimension'
//       pixels. Images will retain their original aspect ratio.
//-----------------------------------------------------------------------------
int makePngPathThumbnail(int maxDimension, char *thumbnailFileName, const MapInfo_t *mapInfo,
                         const InputValues_t *user_inputs, CellInfo_t ***cellInfo, char *title);


//-----------------------------------------------------------------------------
// Name: makeHtmlIterationSummary
// Desc: Create an HTML file to display the various PNG image files showing
//       the paths, the design-rule zones, the cost-multiplier zones, and
//       statistics for the iteration. Maps will be magnified by zoom
//       factor 'mag' (mag = 1 or larger integer).
//
//       If parameter 'iteration' equals zero, then this HTML report will not
//       display or report any information related to routing. Instead, it
//       reports information only about the static map.
//-----------------------------------------------------------------------------
int makeHtmlIterationSummary(int iteration, const MapInfo_t *mapInfo,
                   CellInfo_t ***cellInfo,  const InputValues_t *user_inputs,
                   const RoutingMetrics_t *routability, char *title,
                   const DRC_details_t DRC_details[maxRecordedDRCs],
                   char *shapeTypeNames[NUM_SHAPE_TYPES]);


//-----------------------------------------------------------------------------
// Name: updateHtmlTableOfContents
// Desc: Update the HTML table-of-contents file with the results of
//       iteration # 'pathFinderRun', including the generation of PNG map-files
//       and a new HTML file to display these PNG files.
//-----------------------------------------------------------------------------
void updateHtmlTableOfContents(FILE *fp_TOC, MapInfo_t *mapInfo, CellInfo_t ***cellInfo,
                               InputValues_t *user_inputs, RoutingMetrics_t *routability,
                               DRC_details_t DRC_details[maxRecordedDRCs],
                               char *shapeTypeNames[NUM_SHAPE_TYPES], int cost_multipliers_used );


//-----------------------------------------------------------------------------
// Name: makeDesignRulePngMaps
// Desc: Create PNG map files that display the locations of the one or more 
//       design-rule (DR) zones in grey. If there are conflicts between design 
//       rules between two layers, then these regions are colored red.
//
//       Also populate the following arrays:
//
//        user_inputs->usedOnLayers[maxDesignRuleSets][maxRoutingLayers]
//            mapInfo->maxInteractionRadiusCellsOnLayer[maxRoutingLayers]
//            mapInfo->maxInteractionRadiusSquaredOnLayer[maxRoutingLayers]
//
//       This function also defines user_inputs->designRuleUsed[DR_num]
//       to specify whether design-rule set 'DR_num' is used anywhere
//       in the map. If a design-rule set is not used anywhere in the
//       map, then this function sets to FALSE all elements of the
//       array user_inputs->DR_subsetUsed[DR_num][DR_subset_num], for
//       all values of DR_subset_num.
//-----------------------------------------------------------------------------
int makeDesignRulePngMaps(CellInfo_t ***cellInfo, MapInfo_t *mapInfo, InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: makeDesignRuleReport
// Desc: Create HTML report of the various design-rule sets. Also include
//       generic PNG image depicting cross-section of design rules (not to scale).
//-----------------------------------------------------------------------------
void makeDesignRuleReport(CellInfo_t ***cellInfo, InputValues_t *user_inputs, MapInfo_t *mapInfo);


//-----------------------------------------------------------------------------
// Name: makeCostZonePngMaps
// Desc: Create PNG map files that display the cost zones for each routing
//       and via layer.
//-----------------------------------------------------------------------------
int makeCostZonePngMaps(CellInfo_t ***cellInfo, MapInfo_t *mapInfo, InputValues_t *user_inputs);


//-----------------------------------------------------------------------------
// Name: makeCostMapReport
// Desc: Create HTML report to illustrate the locations of the various cost zones.
//-----------------------------------------------------------------------------
void makeCostMapReport(CellInfo_t ***cellInfo, InputValues_t *user_inputs, MapInfo_t *mapInfo);

