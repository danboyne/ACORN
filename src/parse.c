#include "global_defs.h"
#include "parseLibrary.h"


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
void pre_process_input_file(char *input_filename, InputValues_t *user_inputs)  {

  char line[1024], temp_line[1024];  // For reading lines from input file
  regex_t regex, regex_diff_pair, regex_single_ended, regex_special_net, regex_diff_pair_swappable_terms; // For processing regular expression (see regex.h)
  const int n_matches = 12; // Maximum number of regex matches allowed
  regmatch_t regex_match[n_matches];  // For processing regular expressions
  char netlist_flag = FALSE;      // TRUE when parsing netlist lines from input file
  char design_rule_flag = FALSE;  // TRUE when parsing design rules from input file
  char exception_flag = FALSE;   // TRUE when parsing an exception within a design-rule
  int num_nets = 0;                    // Count of number of nets.
  int num_nets_with_special_rules = 0; // Count of number of nets that have net-specific design rules
  int num_diff_pair_nets = 0;          // Count of number of diff-pair nets
  int num_block_instructions = 0; // Number of BLOCK/UNBLOCK instructions in input file
  int num_DR_zone_instructions = 0; // Number of DR_zone instructions in input file
  int num_trace_cost_zone_instructions = 0; // Number of trace_cost_zone instructions in input file
  int num_via_cost_zone_instructions = 0; // Number of via_cost_zone instructions in input file
  int num_swap_instructions = 0; // Number of PIN_SWAP/NO_PIN_SWAP instructions in input file
  int design_rule_set   = 0; // Number of design-rule sets
  int num_subsets = 0;  // Number of net-specific subsets within a design-rule set

  FILE *fp;
  fp = fopen(input_filename, "r");
  if (! fp) {
    printf ("\nERROR: Input file \"%s\" is not available for reading.\n\n",input_filename);
    exit(1);
  }
  // printf("DEBUG: Finished opening input file for reading.\n");

  //
  // Read each line in the input file:
  //
  while ((fgets(line, 1024, fp)) != NULL)  {
    // Change CR, LF, CR-LF, or LF-CR to '\0':
    line[strcspn(line, "\r\n")] = 0;

    // Filter out any lines that begin with a '#' character:
    compile_regex("^#.*$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0) {
      regfree(&regex);
      // printf("Skipping: <<%s>>\n", line);
      continue;  // Skip lines that begin with '#'
    }
    else  {
      regfree(&regex);
    }

    // Filter out any lines that begin with '//' characters:
    compile_regex("^[[:blank:]]*//", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      // printf("Skipping: <<%s>>\n", line);
      continue;  // Skip lines that begin with '//'
    }
    else  {
      regfree(&regex);
    }

    // Filter out blank lines:
    compile_regex("^[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      // printf("Skipping: <<%s>>\n", line);
      continue;  // Skip blank lines
    }
    else  {
      regfree(&regex);
    }

    // Following debug print-statement prints out the line that was read from the input file:
    // printf("\n|%s|\n", line);

    // Discard comments denoted by '//' out to the end of the line:
    compile_regex("^(.*)(//.*)$", &regex);
    if (regexec(&regex, line, 3, regex_match, 0) == 0)  {
      regfree(&regex);

      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_string
      strncpy(temp_line, line + regex_match[1].rm_so, (int)(regex_match[1].rm_eo - regex_match[1].rm_so));

      memset(line, '\0', sizeof(line));  // Reset 'line' string
      strcpy(line, temp_line);  // Copy resulting text to 'line' string
    }
    else  {
      regfree(&regex);
    }

    // Discard leading and trailing white-space:
    compile_regex("^[[:blank:]]*([^[:blank:]].*[^[:blank:]])[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 2, regex_match, 0) == 0)  {
      regfree(&regex);
      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
      strncpy(temp_line, line + regex_match[1].rm_so, (int)(regex_match[1].rm_eo - regex_match[1].rm_so));

      memset(line, '\0', sizeof(line)); // Reset 'line' string
      strcpy(line, temp_line);  // Copy resulting text to 'line' string
    }
    else  {
      regfree(&regex);
    }

    // printf("line is <<%s>> after discarding leading/trailing white-space.\n", line);

    //
    // Check for key words 'start_nets' and 'end_nets'
    //
    compile_regex("^start_nets$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      netlist_flag = TRUE;
      // printf("DEBUG: netlist_flag is TRUE\n");
      continue;
    }
    else  {
      regfree(&regex);
    }

    compile_regex("^end_nets$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      netlist_flag = FALSE;
      // printf("DEBUG: netlist_flag is FALSE\n");
      continue;
    }
    else  {
      regfree(&regex);
    }


    //
    // Count number of nets, which can consist of either 7, 8, 9, or 10 whitespace-delimited
    // tokens, depending on whether they single-ended signals (7 tokens), net-specific
    // design rules (8 tokens), differential pairs with non-swappable P/N terminals(9 tokens).
    // or differential pairs with P/N-swappable terminals (10 tokens):
    if (netlist_flag)  {

      //
      // Compile regular expressions for three types of netlist lines:
      //
      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)$",
          &regex_single_ended);
      // printf("DEBUG: Successfully compiled regex for 7 space-delimited tokens.\n");

      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)$",
          &regex_special_net);
      // printf("DEBUG: Successfully compiled regex for 8 space-delimited tokens.\n");

      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)$",
          &regex_diff_pair);
      // printf("DEBUG: Successfully compiled regex for 9 space-delimited tokens.\n");

      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+pn_swappable$",
          &regex_diff_pair_swappable_terms);
      // printf("DEBUG: Successfully compiled regex for 10 space-delimited tokens.\n");

      //
      // Check for netlist line with 7 tokens, denoting a standard net:
      //
      if (regexec(&regex_single_ended, line, 8, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 7-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // Increment the counter for the number of nets.
        num_nets++;

        continue; // Skip to next line in input file

      }  // End of if-block for matching a singled-ended netlist line

      //
      // Check for netlist line with 8 tokens, denoting a net with net-specific
      // design rules:
      //
      else if (regexec(&regex_special_net, line, 9, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 8-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // Increment the counter for the number of nets.
        num_nets++;

        // Increment the counter for the number of nets with special design rules.
        num_nets_with_special_rules++;

        continue; // Skip to next line in input file

      }  // End of else-block for matching a special net with 8 tokens

      //
      // Check for netlist line with 9 tokens, denoting a net that's part of
      // a differential pair (but without P/N-swappable terminals):
      //
      else if (regexec(&regex_diff_pair, line, 10, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 9-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // Increment the counter for the number of nets.
        num_nets++;

        // Increment the counter for the number of nets that are part of a diff pair:
        num_diff_pair_nets++;

        continue; // Skip to next line in input file

      }  // End of else-block for matching a special net with 9 tokens


      //
      // Check for netlist line with 10 tokens, denoting a net that's part of
      // a differential pair with P/N-swappable terminals:
      //
      else if (regexec(&regex_diff_pair_swappable_terms, line, 11, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 10-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // Increment the counter for the number of nets.
        num_nets++;

        // Increment the counter for the number of nets that are part of a diff pair:
        num_diff_pair_nets++;

        continue; // Skip to next line in input file

      }  // End of if/else/if-block for matching a diff-pair netlist line

      else {
        printf("\nERROR: While preprocessing the input file, I expected details about a net, but found the following line instead:\n%s\n\n", line);
        printf("       Please fix the input file and restart the program. Program is terminating.\n\n");

        // Free memory for regular expressions of nets:
        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        exit(1);
      }

    }  // End of if-block for (netlist_flag)

    //
    // Check for lines of the form "BLOCK TYPE LAYER..."
    //                          or "UNBLOCK TYPE LAYER...:
    //
    compile_regex("^([UN]*BLOCK)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //               (UN)BLOCK               <---TYPE>-->               <---Layer--->  <params>
    //
    if (regexec(&regex, line, 5, regex_match, 0) == 0)  {
      regfree(&regex);

      num_block_instructions++; // Increment the number of BLOCK/UNBLOCK instructions
      // printf("DEBUG: num_block_instructions is now %d\n", num_block_instructions);
      if (num_block_instructions >= maxBlockInstructions)  {
        printf("\nERROR: The number of BLOCK/UNBLOCK instructions in the input file has exceeded the\n");
        printf("       allowed number (%d). Pleased edit input file and re-start program.\n\n", maxBlockInstructions);
        exit(1);
      }  // End of if-block for num_block_instructions exceeding allowed value

      continue;  // Skip to next line of input file
    }  // End of if-block for matching "BLOCK TYPE LAYER..." line
    else  {
      regfree(&regex);
    }

    //
    // Check for key word 'design_rule_set':
    //
    compile_regex("^design_rule_set[[:blank:]]+([^[:blank:]]+)[[:blank:]]+(.*)$", &regex);
    //              design_rule_set            <---DR_name--->            DR_description
    if (regexec(&regex, line, 4, regex_match, 0) == 0)  {
      regfree(&regex);
      design_rule_flag = TRUE;
      // printf("DEBUG: design_rule_flag is TRUE\n");

      // Reset number of design-rule subsets, which will be counted for each new design-rule set:
      num_subsets = 0;
      continue;  // Skip to next line of input file.
    }
    else  {
      regfree(&regex);
    }

    //
    // Check for key word 'end_design_rule_set'
    //
    compile_regex("^end_design_rule_set$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);

      if (! design_rule_flag)  {
        printf("\nERROR: The 'end_design_rule_set' keyword was found outside of a design-rule set. This keyword\n");
        printf("       is only allowed between after a 'design_rule_set' statement.\n");
        printf("       The offending line is:\n");
        printf("\n%s\n\n", line);
        printf("       Please modify the input file and re-start the program. The program is terminating.\n\n");
        exit(1);
      }  // End of if-block for (!design_rule_flag)

      if (exception_flag)  {
        printf("\nERROR: The 'end_design_rule_set' keyword was found within an exception block for\n");
        printf("       a design-rule set. Each 'exception' statement must be followed by an\n");
        printf("       'end_exception' statement, prior to the 'end_design_rule_set' keyword.\n");
        printf("       The offending line is:\n");
        printf("\n%s\n\n", line);
        printf("       Please modify the input file and re-start the program. The program is terminating.\n\n");
        exit(1);
      }  // End of if-block for (exception_flag)

      design_rule_flag = FALSE;  // Reset 'design_rule_flag' since we're exiting a design-rule block
      num_subsets++;  // Increment the number of subsets, since the design-rule block counts as a subset

      // Record the number of subsets found in this design-rule set:
      user_inputs->numDesignRuleSubsets[design_rule_set] = num_subsets;

      // Check whether the number of design-rule subsets has exceeded the maximum allowed:
      if (user_inputs->numDesignRuleSubsets[design_rule_set] > maxDesignRuleSubsets)  {
        printf("\nERROR: The number of design-rule subsets (%d) has exceeded the maximimum allowed number of subsets (%d)\n",
               user_inputs->numDesignRuleSubsets[design_rule_set], maxDesignRuleSubsets);
        printf("       in one of the design-rule sets. Please fix this problem in the input file and restart the program.\n\n");
        exit(1);
      }

      // We've gotten to end of a design-rule set, so increment the number of sets:
      design_rule_set++;

      continue;  // Skip to next line of input file.
    }
    else  {
      regfree(&regex);
    }

    //
    // Check for key word 'exception = xxxxxxx'
    //
    compile_regex("^exception[[:blank:]]*=[[:blank:]]*(.*)[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 2, regex_match, 0) == 0)  {
      regfree(&regex);

      if (! design_rule_flag)  {
        printf("\nERROR: The 'exception =' keyword was found outside of a design-rule set. This keyword\n");
        printf("       is only allowed between a 'design_rule_set' and 'end_design_rule_set' statement.\n");
        printf("       The offending line is:\n");
        printf("\n%s\n\n", line);
        printf("       Please modify the input file and re-start the program. The program is terminating.\n\n");
        exit(1);
      }  // End of if-block for (!design_rule_flag)

      if (exception_flag)  {
        printf("\nERROR: The 'exception =' keyword was found nested within another exception.\n");
        printf("       Exception design-rules cannot be nested within each other.\n");
        printf("       The offending line is:\n");
        printf("\n%s\n\n", line);
        printf("       Please modify the input file and re-start the program. The program is terminating.\n\n");
        exit(1);
      }  // End of if-block for (exception_flag)

      exception_flag = TRUE;
      continue;  // Skip to next line of input file.
    }  // End of if-block for finding 'exception = ...'
    else  {
      regfree(&regex);
    }

    //
    // Check for key word 'diff_pair_pitch = xxxxxxx'
    //
    compile_regex("^diff_pair_pitch[[:blank:]]*=[[:blank:]]*(.*)[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 2, regex_match, 0) == 0)  {
      regfree(&regex);

      if (! design_rule_flag)  {
        printf("\nERROR: The 'diff_pair_pitch =' keyword was found outside of a design-rule set. This keyword\n");
        printf("       is only allowed between a 'design_rule_set' and 'end_design_rule_set' statement.\n");
        printf("       The offending line is:\n");
        printf("\n%s\n\n", line);
        printf("       Please modify the input file and re-start the program. The program is terminating.\n\n");
        exit(1);
      }  // End of if-block for (!design_rule_flag)

      if (! exception_flag)  {
        printf("\nERROR: The 'diff_pair_pitch =' keyword was found outside of a design-rule exception.\n");
        printf("       This keyword is only allowed between an 'exception =' and 'end_exception' statement.\n");
        printf("       The offending line is:\n");
        printf("\n%s\n\n", line);
        printf("       Please modify the input file and re-start the program. The program is terminating.\n\n");
        exit(1);
      }  // End of if-block for (! exception_flag)

      // We encountered a 'diff_pair_pitch =' statement, which means we need another design-rule subset to
      // accommodate the pseudo-net for diff-pairs. Increment the number of exceptions:
      num_subsets++;

      // Check whether the number of design-rule subsets has exceeded the maximum allowed:
      if (num_subsets > maxDesignRuleSubsets)  {
        printf("\nERROR: The number of design-rule subsets (%d) has exceeded the maximum allowed number of subsets (%d)\n",
               num_subsets, maxDesignRuleSubsets);
        printf("       in one of the design-rule sets. Please fix this problem in the input file and restart the program.\n\n");
        exit(1);
      }

      continue;  // Skip to next line of input file.
    }  // End of if-block for finding 'exception = ...'
    else  {
      regfree(&regex);
    }


    //
    // Check for key word 'end_exception'
    //
    compile_regex("^end_exception$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);

      if (! exception_flag)  {
        printf("\nERROR: The 'end_exception' keyword was found outside of an exception block. This keyword\n");
        printf("       is only allowed between after a 'exception =' statement.\n");
        printf("       The offending line is:\n");
        printf("\n%s\n\n", line);
        printf("       Please modify the input file and re-start the program. The program is terminating.\n\n");
        exit(1);
      }  // End of if-block for (! exception_flag)

      exception_flag = FALSE;
      // We've gotten to end of an exception, so increment the number of exceptions:
      num_subsets++;

      // Check whether the number of design-rule subsets has exceeded the maximum allowed:
      if (num_subsets > maxDesignRuleSubsets)  {
        printf("\nERROR: The number of design-rule subsets (%d) has exceeded the maximum allowed number of subsets (%d)\n",
               num_subsets, maxDesignRuleSubsets);
        printf("       in one of the design-rule sets. Please fix this problem in the input file and restart the program.\n\n");
        exit(1);
      }

      continue;  // Skip to next line of input file.
    }
    else  {
      regfree(&regex);
    }

    //
    // Check for lines of the form "DR_zone <DR name> <layer name> <shape type> ...."
    //
    compile_regex("^DR_zone[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //              DR_zone             <--DR name-->              <---Layer--->             <-Shape type->  <params>
    //
    if (regexec(&regex, line, 6, regex_match, 0) == 0)  {
      regfree(&regex);

      num_DR_zone_instructions++; // Increment the number of DR_zone instructions
      // printf("DEBUG:   num_DR_zone_instructions has been incremented to %d\n", num_DR_zone_instructions);

      continue;  // Skip to next line of input file.
    }  // End of if-block for matching "DR_zone <DR name> <layer name> <shape type> ...." line
    else  {
      regfree(&regex);
    }


    //
    // Check for lines of the following 2 forms:
    //      "trace_cost_zone <zone index> <layer name> <shape type> ...."
    //         or
    //      "via_cost_zone <zone index> <layer name> <shape type> ...."
    //
    compile_regex("^(trace|via)_cost_zone[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //                trace/via_cost_zone             <-zone index>              <---Layer--->             <-Shape type->  <params>
    //
    if (regexec(&regex, line, 7, regex_match, 0) == 0)  {
      regfree(&regex);

      size_t len;  // Temporary variable to hold length of strings.

      // Extract the command, type, and layer name and save them in the
      // 'user_inputs-> structure:
      char trace_or_via[8];
      len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(trace_or_via, line + regex_match[1].rm_so, (int)len);
      trace_or_via[len] = '\0';  // Terminate string with NULL character
      // printf("DEBUG: Type of cost-zone is '%s'\n", trace_or_via);

      // Issue error and exit if we cannot determine whether statement is
      // 'trace_cost_zone' or 'via_cost_zone':
      if ((strcasecmp(trace_or_via, "trace")) && (strcasecmp(trace_or_via, "via")))  {
        printf("\nERROR: Could not determine whether instruction was 'trace_cost_zone' or 'via_cost_zone'.\n\n");
        exit(1);
      }

      // Capture the number of parameters for this cost-zone command and increment the number
      // of cost-zone instructions:
      if (0 == strcasecmp(trace_or_via, "trace"))  {
        num_trace_cost_zone_instructions++; // Increment the number of trace_cost_zone instructions
      }
      else  {
        num_via_cost_zone_instructions++; // Increment the number of via_cost_zone instructions
      }

      // If the user exceeded the allowed number of cost-zone instructions, then issue an
      // error and exit:
      if (  (num_trace_cost_zone_instructions >= maxCostZones)
         || (  num_via_cost_zone_instructions >= maxCostZones))  {
        printf("\nERROR: More than the allowed number of '%s_cost_zone' statements were found in the\n", trace_or_via);
        printf("       input file. The allowed maximum is %d. Fix the input file and re-start the program.\n\n", maxCostZones);
        exit(1);
      }  // End of if-block for exceeding the allowed number of cost-zones

      continue;  // Skip to next line of input file

    }  // End of if-block for matching "trace_cost_zone <cost index> <layer name> <shape type> ...." line
       //                           or   "via_cost_zone <cost index> <layer name> <shape type> ...." line
    else  {
      regfree(&regex);
    }


    //
    // Check for lines of the form "PIN_SWAP    LAYER SHAPE ..."
    //                          or "NO_PIN_SWAP LAYER SHAPE ...:
    //
    compile_regex("^([NO_]*PIN_SWAP)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //               (NO_)PIN_SWAP               <---Layer--->              <---Shape--->  <params>
    //
    if (regexec(&regex, line, 5, regex_match, 0) == 0)  {
      regfree(&regex);

      num_swap_instructions++; // Increment the number of PIN_SWAP/NO_PIN_SWAP instructions
      // printf("DEBUG: num_swap_instructions incremented to %d\n", num_swap_instructions);

      continue;  // Skip to next line of input file
    }  // End of if-block for matching "(UN_)PIN_SWAP LAYER SHAPE..." line
    else  {
      regfree(&regex);
    }

    //
    // Check for line of the form "number_layers = ..."
    //
    compile_regex("^number_layers[[:blank:]]*=[[:blank:]]*([^[:blank:]]+)[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 2, regex_match, 0) == 0)  {
      regfree(&regex);
      char number_string[10] = "";
      int length = (int)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(number_string, line + regex_match[1].rm_so, length);
      number_string[length] = '\0';  // Terminate string with NULL character

      user_inputs->num_routing_layers = strtof(number_string, NULL);
      // printf("DEBUG: In pre_process_input_file, num_routing_layers = %d\n", user_inputs->num_routing_layers);
    }
    else  {
      regfree(&regex);
    }

  }  // End of while-loop for parsing lines from input file

  // Close intput file:
  fclose(fp);

  // Verify that there's at least one net to route:
  if (num_nets == 0)  {
    printf("\nERROR: The number of nets in the input file is zero. This is not allowed.\n");
    printf(  "       Please fix the netlist in the input file and re-start the program.\n\n");
    exit(1);
  }

  // Verify that num_diff_pair_nets is an even number:
  if (num_diff_pair_nets % 2)  {
    printf("\nERROR: The number of differential-pair nets in the input is an odd number. This is\n");
    printf(  "       not allowed. Please fix the netlist in the input file and re-start the program.\n\n");
    exit(1);
  }

  //
  // Save the parameters from this subroutine into the 'user_inputs-> data structure, so
  // they'll be available to the calling program:
  //
  user_inputs->num_nets           = num_nets;
  user_inputs->num_special_nets   = num_nets_with_special_rules;
  user_inputs->num_diff_pair_nets = num_diff_pair_nets;
  user_inputs->num_pseudo_nets    = num_diff_pair_nets / 2;
  user_inputs->num_block_instructions = num_block_instructions;
  user_inputs->numDesignRuleSets = design_rule_set;
  user_inputs->num_DR_zones = num_DR_zone_instructions;
  user_inputs->num_trace_cost_zone_instructions = num_trace_cost_zone_instructions;
  user_inputs->num_via_cost_zone_instructions   = num_via_cost_zone_instructions;
  user_inputs->num_swap_instructions = num_swap_instructions;

  // Verify that the number of user-defined nets plus pseudo-nets does not exceed
  // the maximum allowed by this software:
  if (user_inputs->num_nets + user_inputs->num_pseudo_nets > maxNets)  {
    printf("\nERROR: The number of user-defined nets nets in the input file (%d), plus the number of diff-pairs (%d) exceeds the\n",
            user_inputs->num_nets, user_inputs->num_pseudo_nets);
    printf(  "       maximum allowed number of nets (%d). Please reduce the netlist in the input file and re-start the program.\n\n", maxNets);
    exit(1);
  }

}  // End of function 'pre_process_input_file'


//-----------------------------------------------------------------------------
// Name: initialize_input_values
// Desc: Allocate memory for variables in the input_values structure.
//-----------------------------------------------------------------------------
void initialize_input_values(InputValues_t *input_values)  {

  // printf("DEBUG: Entered function 'initialize_input_values' with num_nets = %d, num_pseudo_nets = %d, num_routing_layers = %d...\n",
  //         input_values->num_nets, input_values->num_pseudo_nets, input_values->num_routing_layers);

  //
  // Allocate memory for data structures required for each user-defined net, each pseudo-net, and the
  // Acorn-defined 'global repellent' net:
  //
  int max_routed_nets = input_values->num_nets + input_values->num_pseudo_nets + 1;

  input_values->diffPairPartner               = malloc(max_routed_nets * sizeof(short  ));  // 1D array
  input_values->diffPairPartnerName           = malloc(max_routed_nets * sizeof(char * ));  // 1D array of strings
  input_values->diffPairPitchCells            = malloc(max_routed_nets * sizeof(float *));  // 2D array
  input_values->diffPairPitchMicrons          = malloc(max_routed_nets * sizeof(float *));  // 2D array
  input_values->netSpecificRuleName           = malloc(max_routed_nets * sizeof(char * ));  // 1D array of strings
  input_values->diffPairToPseudoNetMap        = malloc(max_routed_nets * sizeof(int    ));  // 1D array
  input_values->rats_nest_length_um           = malloc(max_routed_nets * sizeof(float  ));  // 1D array
  input_values->net_name                      = malloc(max_routed_nets * sizeof(char * ));
  input_values->isDiffPair                    = malloc(max_routed_nets * sizeof(char * ));
  input_values->isPNswappable                 = malloc(max_routed_nets * sizeof(char * ));
  input_values->isPseudoNet                   = malloc(max_routed_nets * sizeof(char * ));
  input_values->start_layer                   = malloc(max_routed_nets * sizeof(char * ));
  input_values->end_layer                     = malloc(max_routed_nets * sizeof(char * ));
  input_values->start_X_um                    = malloc(max_routed_nets * sizeof(float *));
  input_values->start_Y_um                    = malloc(max_routed_nets * sizeof(float *));
  input_values->end_X_um                      = malloc(max_routed_nets * sizeof(float *));
  input_values->end_Y_um                      = malloc(max_routed_nets * sizeof(float *));
  input_values->usesSpecialRule               = malloc(max_routed_nets * sizeof(char * ));
  input_values->designRuleSubsetMap           = malloc(max_routed_nets * sizeof(char * ));
  input_values->pseudoNetToDiffPair_1         = malloc(max_routed_nets * sizeof(int *  ));
  input_values->pseudoNetToDiffPair_2         = malloc(max_routed_nets * sizeof(int *  ));
  input_values->diffPairStartTermPitchMicrons = malloc(max_routed_nets * sizeof(float * ));
  input_values->diffPairEndTermPitchMicrons   = malloc(max_routed_nets * sizeof(float * ));
  input_values->diffPairStartTermPitch        = malloc(max_routed_nets * sizeof(short * ));
  input_values->diffPairEndTermPitch          = malloc(max_routed_nets * sizeof(short * ));

  // Allocate memory for 2-dimensional arrays, where the first dimension is the net number:
  for (int i = 0; i < max_routed_nets; i++)  {

    input_values->diffPairPartnerName[i] = malloc(maxNetNameLength * sizeof(char));
    if (input_values->diffPairPartnerName[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->diffPairPartnerName[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->netSpecificRuleName[i] = malloc(maxDesRuleSetNameLength * sizeof(char));
    if (input_values->netSpecificRuleName[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->netSpecificRuleName[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->diffPairPitchCells[i] = malloc(input_values->numDesignRuleSets * sizeof(float));
    if (input_values->diffPairPitchCells[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->diffPairPitchCells[%d]' 2-dimensional array.\n\n", i);
      exit (1);
    }

    input_values->diffPairPitchMicrons[i] = malloc(input_values->numDesignRuleSets * sizeof(float));
    if (input_values->diffPairPitchMicrons[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->diffPairPitchMicrons[%d]' 2-dimensional array.\n\n", i);
      exit (1);
    }

    // Initialize elements of two-dimensional arrays diffPairPitchCells and diffPairPitchMicrons to non-sensical values:
    for (int DR_set = 0; DR_set < input_values->numDesignRuleSets; DR_set++)  {
      input_values->diffPairPitchCells[i][DR_set]   = -999.0;
      input_values->diffPairPitchMicrons[i][DR_set] = -999.0;
      // printf("DEBUG: Initialized input_values->diffPairPitchCells[path=%d][DR_set=%d] to %6.3f\n", i, DR_set, input_values->diffPairPitchCells[i][DR_set]);
      // printf("DEBUG: Initialized input_values->diffPairPitchMicrons[path=%d][DR_set=%d] to %6.3f\n", i, DR_set, input_values->diffPairPitchMicrons[i][DR_set]);
    }  // End of for-loop for index 'DR_set'

    // Initialize 'diffPairPartner' and 'usesSpecialRule' array elements to nonsensical values:
    input_values->diffPairPartner[i]        = -1;
    input_values->diffPairToPseudoNetMap[i] = -1;

    // Initialize 'usesSpecialRule' Boolean elements to FALSE:
    input_values->usesSpecialRule[i] = FALSE;

  }  // End of for-loop for index i (0 to num_nets)

  // Allocate memory for 2-dimensional arrays whose first dimension ranges from
  // zero to (num_nets + num_pseudo_nets + 1), which is 'max_routed_nets'. Also,
  // initialize array elements for 1-dimensional arrays :
  for (int i = 0; i < max_routed_nets; i++)  {

    // Allocate memory for 'net_name' string:
    input_values->net_name[i] = malloc(maxNetNameLength * sizeof(char));
    if (input_values->net_name[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->net_name[%d]' string.\n\n", i);
      exit (1);
    }

    // Allocate memory for 'start_layer' string:
    input_values->start_layer[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->start_layer[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->start_layer[%d]' string.\n\n", i);
      exit (1);
    }

    // Allocate memory for 'end_layer' string:
    input_values->end_layer[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->end_layer[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->end_layer[%d]' string.\n\n", i);
      exit (1);
    }

    // Allocate memory for 2nd dimension of array 'designRuleSubsetMap':
    input_values->designRuleSubsetMap[i] = malloc(input_values->numDesignRuleSets * sizeof(char));
    if (input_values->designRuleSubsetMap[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->designRuleSubsetMap[%d]' 2-dimensional array.\n\n", i);
      exit (1);
    }

    // Initialize each element of designRuleSubsetMap[net_number][DR_set] to zero, i.e., the
    // default design-rule subset number:
    for (int DR_set = 0; DR_set < input_values->numDesignRuleSets; DR_set++)  {
      input_values->designRuleSubsetMap[i][DR_set] = 0;
      // printf("DEBUG:   Done initializing 'designRuleSubsetMap[i=%d][DR_set=%d]' to zero\n", i, DR_set);
    }  // End of for-loop for variable 'DR_num' (0 to numDesignRuleSets)

    // Initialize 1-dimensional array elements to non-sensical values:
    input_values->pseudoNetToDiffPair_1[i] = -1;
    input_values->pseudoNetToDiffPair_2[i] = -1;

    // Initialize 'isPseudoNet' array elements to FALSE (zero)
    input_values->isPseudoNet[i] = FALSE;

    // Initialize 'isDiffPair' Boolean elements to FALSE:
    input_values->isDiffPair[i]      = FALSE;

    // Initialize 'isPNswappable' Boolean elements to FALSE:
    input_values->isPNswappable[i]   = FALSE;

    // Initialize 'diffPairStartTermPitchMicrons' and 'diffPairEndTermPitchMicrons' to 0.0:
    input_values->diffPairStartTermPitchMicrons[i] = 0.0;
    input_values->diffPairEndTermPitchMicrons[i]   = 0.0;

    // Initialize 'diffPairStartTermPitch' and 'diffPairEndTermPitch' to zero:
    input_values->diffPairStartTermPitch[i] = 0;
    input_values->diffPairEndTermPitch[i]   = 0;

  }  // End of for-loop for index i (0 to num_nets + num_pseudo_nets + 1)

  // Define the net with the highest index as a pseudo-net so that other pseudo-paths
  // are repelled by it:
  input_values->isPseudoNet[max_routed_nets - 1] = TRUE;

  // Allocate memory for data structures required for layer (routing layer and via layer)
  for (int i = 0; i < 2*maxRoutingLayers - 1; i++ )  {
    input_values->layer_names[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->layer_names[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->layer_names[%d]' string.\n\n", i);
      exit (1);
    }
  }  // End of for-loop for index i (0 to 2*maxRoutingLayers-1)

  // Allocate memory for data structures required for (only) the routing layers. Also initialize arrays
  // that are dimensioned to 'maxRoutingLayers':
  for (int i = 0; i < maxRoutingLayers; i++ )  {
    input_values->routingLayerNames[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->routingLayerNames[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->routingLayerNames[%d]' string.\n\n", i);
      exit (1);
    }
  }  // End of for-loop for index i (0 to maxRoutingLayers)

  input_values->origin = malloc(maxNetNameLength * sizeof(char));
  if (input_values->origin == 0)  {
    printf("\nERROR: Failed to allocate memory for 'input_values->origin' string.\n\n");
    exit (1);
  }

  //
  // Allocate memory for string variables associated with 'BLOCK' commands in the user-
  // defined input text file:
  //
  input_values->block_command    = malloc(input_values->num_block_instructions * sizeof(char * ));
  input_values->block_type       = malloc(input_values->num_block_instructions * sizeof(char * ));
  input_values->block_layer      = malloc(input_values->num_block_instructions * sizeof(char * ));
  input_values->block_num_params = malloc(input_values->num_block_instructions * sizeof(char   ));
  input_values->block_parameters = malloc(input_values->num_block_instructions * sizeof(float *));
  for (int i = 0; i < input_values->num_block_instructions; i++ )  {

    input_values->block_command[i] = malloc(maxBlockInstructionLength * sizeof(char));
    if (input_values->block_command[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->block_command[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->block_type[i] = malloc(maxBlockInstructionLength * sizeof(char));
    if (input_values->block_type[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->block_type[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->block_layer[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->block_layer[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->block_layer[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->block_parameters[i] = malloc(maxBlockParameters * sizeof(float));
    if (input_values->block_parameters[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->block_parameters[%d]' array.\n\n", i);
      exit (1);
    }

  }  // End of for-loop for index i (0 to num_block_instructions)

  // Allocate memory for 1-dimensional array 'designRuleUsed':
  input_values->designRuleUsed = malloc(input_values->numDesignRuleSets * sizeof(unsigned char));

  // Allocate memory for 1 dimension of the 2-D array 'DR_subsetUsed':
  input_values->DR_subsetUsed = malloc(input_values->numDesignRuleSets * sizeof(unsigned char *));

  // Allocate memory for 1 dimension of the 2-D array 'designRules':
  input_values->designRules = malloc(input_values->numDesignRuleSets * sizeof(DesignRuleSubset_t *));

  // Allocate memory for 1 dimension of the following five 4-D arrays:
  input_values->cong_radius         = malloc(input_values->numDesignRuleSets * sizeof(float ***));
  input_values->cong_radius_squared = malloc(input_values->numDesignRuleSets * sizeof(float ***));
  input_values->DRC_radius          = malloc(input_values->numDesignRuleSets * sizeof(float ***));
  input_values->DRC_radius_squared  = malloc(input_values->numDesignRuleSets * sizeof(float ***));
  input_values->detour_distance     = malloc(input_values->numDesignRuleSets * sizeof(float ***));

  // printf("DEBUG: About to allocate memory for %d design-rule sets and their names, each with length %d characters.\n",
  //        input_values->numDesignRuleSets, maxDesRuleSetNameLength);
  for (int DR_set_1 = 0; DR_set_1 < input_values->numDesignRuleSets; DR_set_1++ )  {
    // printf("DEBUG: About to allocate memory for design-rule set %d.\n", DR_set_1);

    // Allocate memory for 2nd dimension of 2-D array 'DR_subsetUsed':
    input_values->DR_subsetUsed[DR_set_1] = malloc(input_values->numDesignRuleSubsets[DR_set_1] * sizeof(unsigned char));
    if (input_values->DR_subsetUsed[DR_set_1] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->DR_subsetUsed[%d]'.\n\n", DR_set_1);
      exit (1);
    }

    // Allocate memory for 2nd dimension of the 2-D array 'designRules':
    input_values->designRules[DR_set_1] = malloc(input_values->numDesignRuleSubsets[DR_set_1] * sizeof(DesignRuleSubset_t));
    if (input_values->designRules[DR_set_1] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->designRules[%d]'.\n\n", DR_set_1);
      exit (1);
    }
    // printf("DEBUG: Successfully allocated memory for %d elements of input_values->designRules[%d].\n",
    //        input_values->numDesignRuleSubsets[DR_set_1], DR_set_1);

    // printf("DEBUG: About to allocate memory for name of design-rule set %d.\n", DR_set_1);
    input_values->designRuleSetName[DR_set_1] = malloc(maxDesRuleSetNameLength * sizeof(char));
    if (input_values->designRuleSetName[DR_set_1] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->designRuleSetName[%d]' string.\n\n", DR_set_1);
      exit (1);
    }

    // printf("DEBUG: About to allocate memory for description of design-rule set %d.\n", DR_set_1);
    input_values->designRuleSetDescription[DR_set_1] = malloc(maxDesRuleSetDescriptionLength * sizeof(char));
    if (input_values->designRuleSetDescription[DR_set_1] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->designRuleSetDescription[%d]' string.\n\n", DR_set_1);
      exit (1);
    }

    // Initialize the 'usedOnLayer variables for each design-rule set:
    for (int layer = 0; layer < maxRoutingLayers; layer++)  {
      input_values->usedOnLayers[DR_set_1][layer] = 0;
    }  // End of for-loop for index 'layer'

    // Initialize each element of the 'designRuleUsed[]' array to FALSE:
    input_values->designRuleUsed[DR_set_1] = FALSE;

    // Allocate memory for strings of each design-rule subset name:
    for (int DR_subset_1 = 0; DR_subset_1 < input_values->numDesignRuleSubsets[DR_set_1]; DR_subset_1++)  {
      // printf("DEBUG: About to allocate memory for design-rule set #%d, subset #%d.\n", DR_set_1, DR_subset_1);

      // printf("DEBUG: About to allocate memory for name of design-rule subset #%d in design-rule set %d.\n", DR_subset_1, DR_set_1);
      input_values->designRules[DR_set_1][DR_subset_1].subsetName = malloc(maxDesRuleSetNameLength * sizeof(char));
      if (input_values->designRules[DR_set_1][DR_subset_1].subsetName == 0)  {
        printf("\nERROR: Failed to allocate memory for 'input_values->designRules[%d][%d].subsetName' string.\n\n", DR_set_1, DR_subset_1);
        exit (1);
      }

      // Initialize to FALSE each element of the 2-D array 'DR_subsetUsed[][]':
      input_values->DR_subsetUsed[DR_set_1][DR_subset_1] = FALSE;

    }  // End of for-loop for index DR_subset_1 (0 to numDesignRuleSubsets[DR_set_1])

    //
    // Allocate memory for 2nd dimension of 4-dimensional arrays 'cong_radius', 'cong_radius_squared',
    // 'DRC_radius', 'DRC_radius_squared', and 'detour_distance':
    //
    // 2nd dimension of 4-dimensional array 'cong_radius':
    const short num_subset_shapeTypes_1 = NUM_SHAPE_TYPES * input_values->numDesignRuleSubsets[DR_set_1];
    input_values->cong_radius[DR_set_1] = malloc(num_subset_shapeTypes_1 * sizeof(float **));
    if (input_values->cong_radius[DR_set_1] == NULL)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->cong_radius[%d]' matrix.\n\n", DR_set_1);
      exit (1);
    }
    // 2nd dimension of 4-dimensional array'cong_radius_squared':
    input_values->cong_radius_squared[DR_set_1] = malloc(num_subset_shapeTypes_1 * sizeof(float **));
    if (input_values->cong_radius_squared[DR_set_1] == NULL)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->cong_radius_squared[%d]' matrix.\n\n", DR_set_1);
      exit (1);
    }

    // 2nd dimension of 4-dimensional array 'DRC_radius':
    input_values->DRC_radius[DR_set_1] = malloc(num_subset_shapeTypes_1 * sizeof(float **));
    if (input_values->DRC_radius[DR_set_1] == NULL)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->DRC_radius[%d]' matrix.\n\n", DR_set_1);
      exit (1);
    }

    // 2nd dimension of 4-dimensional array 'DRC_radius_squared':
    input_values->DRC_radius_squared[DR_set_1] = malloc(num_subset_shapeTypes_1 * sizeof(float **));
    if (input_values->DRC_radius_squared[DR_set_1] == NULL)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->DRC_radius_squared[%d]' matrix.\n\n", DR_set_1);
      exit (1);
    }

    // 2nd dimension of 4-dimensional array 'detour_distance':
    input_values->detour_distance[DR_set_1] = malloc(num_subset_shapeTypes_1 * sizeof(float **));
    if (input_values->detour_distance[DR_set_1] == NULL)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->detour_distance[%d]' matrix.\n\n", DR_set_1);
      exit (1);
    }

    //
    // Allocate memory for 3rd dimension of 4-dimensional arrays 'cong_radius', 'cong_radius_squared',
    // 'DRC_radius', 'DRC_radius_squared', and 'detour_distance':
    //
    for (int subset_shapeType_1 = 0; subset_shapeType_1 < num_subset_shapeTypes_1; subset_shapeType_1++ )  {

      // 3rd dimension of 4-dimensional array 'cong_radius':
      input_values->cong_radius[DR_set_1][subset_shapeType_1] = malloc(input_values->numDesignRuleSets * sizeof(float *));
      // printf("DEBUG: Allocated space for %d pointers for input_values->cong_radius[%d][%d].\n", input_values->numDesignRuleSets, DR_set_1, subset_shapeType_1);
      if (input_values->cong_radius[DR_set_1][subset_shapeType_1] == NULL)  {
        printf("\nERROR: Failed to allocate memory for 'input_values->cong_radius[%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1);
        exit (1);
      }

      // 3rd dimension of 4-dimensional array 'cong_radius_squared':
      input_values->cong_radius_squared[DR_set_1][subset_shapeType_1] = malloc(input_values->numDesignRuleSets * sizeof(float *));
      // printf("DEBUG: Allocated space for %d pointers for input_values->cong_radius_squared[%d][%d].\n", input_values->numDesignRuleSets, DR_set_1, subset_shapeType_1);
      if (input_values->cong_radius_squared[DR_set_1][subset_shapeType_1] == NULL)  {
        printf("\nERROR: Failed to allocate memory for 'input_values->cong_radius_squared[%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1);
        exit (1);
      }

      // 3rd dimension of 4-dimensional array 'DRC_radius':
      input_values->DRC_radius[DR_set_1][subset_shapeType_1] = malloc(input_values->numDesignRuleSets * sizeof(float *));
      // printf("DEBUG: Allocated space for %d pointers for input_values->DRC_radius[%d][%d].\n", input_values->numDesignRuleSets, DR_set_1, subset_shapeType_1);
      if (input_values->DRC_radius[DR_set_1][subset_shapeType_1] == NULL)  {
        printf("\nERROR: Failed to allocate memory for 'input_values->DRC_radius[%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1);
        exit (1);
      }

      // 3rd dimension of 4-dimensional array 'DRC_radius_squared':
      input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1] = malloc(input_values->numDesignRuleSets * sizeof(float *));
      // printf("DEBUG: Allocated space for %d pointers for input_values->DRC_radius_squared[%d][%d].\n", input_values->numDesignRuleSets, DR_set_1, subset_shapeType_1);
      if (input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1] == NULL)  {
        printf("\nERROR: Failed to allocate memory for 'input_values->DRC_radius_squared[%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1);
        exit (1);
      }

      // 3rd dimension of 4-dimensional array 'detour_distance':
      input_values->detour_distance[DR_set_1][subset_shapeType_1] = malloc(input_values->numDesignRuleSets * sizeof(float *));
      // printf("DEBUG: Allocated space for %d pointers for input_values->detour_distance[%d][%d].\n", input_values->numDesignRuleSets, DR_set_1, subset_shapeType_1);
      if (input_values->detour_distance[DR_set_1][subset_shapeType_1] == NULL)  {
        printf("\nERROR: Failed to allocate memory for 'input_values->detour_distance[%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1);
        exit (1);
      }

      //
      // Allocate memory for 4th dimension of 4-dimensional arrays 'cong_radius', 'cong_radius_squared',
      // 'DRC_radius', 'DRC_radius_squared', and 'detour_distance':
      //
      for (int DR_set_2 = 0; DR_set_2 < input_values->numDesignRuleSets; DR_set_2++)  {

        const short num_subset_shapeTypes_2 = NUM_SHAPE_TYPES * input_values->numDesignRuleSubsets[DR_set_2];

        // 4th dimension of 4-dimensional array 'cong_radius':
        input_values->cong_radius[DR_set_1][subset_shapeType_1][DR_set_2] = malloc(num_subset_shapeTypes_2 * sizeof(float));
        // printf("DEBUG: Allocated space for %d elements for input_values->cong_radius[%d][%d][%d].\n", num_subset_shapeTypes_2, DR_set_1, subset_shapeType_1, DR_set_2);
        if (input_values->cong_radius[DR_set_1][subset_shapeType_1][DR_set_2] == NULL)  {
          printf("\nERROR: Failed to allocate memory for 'input_values->cong_radius[%d][%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1, DR_set_2);
          exit (1);
        }

        // 4th dimension of 4-dimensional array 'cong_radius_squared':
        input_values->cong_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2] = malloc(num_subset_shapeTypes_2 * sizeof(float));
        // printf("DEBUG: Allocated space for %d elements for input_values->cong_radius_squared[%d][%d][%d].\n", num_subset_shapeTypes_2, DR_set_1, subset_shapeType_1, DR_set_2);
        if (input_values->cong_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2] == NULL)  {
          printf("\nERROR: Failed to allocate memory for 'input_values->cong_radius_squared[%d][%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1, DR_set_2);
          exit (1);
        }

        // 4th dimension of 4-dimensional array 'DRC_radius':
        input_values->DRC_radius[DR_set_1][subset_shapeType_1][DR_set_2] = malloc(num_subset_shapeTypes_2 * sizeof(float));
        // printf("DEBUG: Allocated space for %d elements for input_values->DRC_radius[%d][%d][%d].\n", num_subset_shapeTypes_2, DR_set_1, subset_shapeType_1, DR_set_2);
        if (input_values->DRC_radius[DR_set_1][subset_shapeType_1][DR_set_2] == NULL)  {
          printf("\nERROR: Failed to allocate memory for 'input_values->DRC_radius[%d][%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1, DR_set_2);
          exit (1);
        }

        // 4th dimension of 4-dimensional array 'DRC_radius_squared':
        input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2] = malloc(num_subset_shapeTypes_2 * sizeof(float));
        // printf("DEBUG: Allocated space for %d elements for input_values->DRC_radius_squared[%d][%d][%d].\n", num_subset_shapeTypes_2, DR_set_1, subset_shapeType_1, DR_set_2);
        if (input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2] == NULL)  {
          printf("\nERROR: Failed to allocate memory for 'input_values->DRC_radius_squared[%d][%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1, DR_set_2);
          exit (1);
        }

        // 4th dimension of 4-dimensional array 'detour_distance':
        input_values->detour_distance[DR_set_1][subset_shapeType_1][DR_set_2] = malloc(num_subset_shapeTypes_2 * sizeof(float));
        // printf("DEBUG: Allocated space for %d elements for input_values->detour_distance[%d][%d][%d].\n", num_subset_shapeTypes_2, DR_set_1, subset_shapeType_1, DR_set_2);
        if (input_values->detour_distance[DR_set_1][subset_shapeType_1][DR_set_2] == NULL)  {
          printf("\nERROR: Failed to allocate memory for 'input_values->detour_distance[%d][%d][%d]' array.\n\n", DR_set_1, subset_shapeType_1, DR_set_2);
          exit (1);
        }

      }  // End of for-loop for index 'DR_set_2'
    }  // End of for-loop for index 'subset_shapeType_1'
  }  // End of for-loop for index 'DR_set_1' (0 to numDesignRuleSets)

  //
  // Allocate memory for the 3-dimensional array 'foreign_DR_subset[DR_sets][DR_subsets][DR_sets]':
  //
  input_values->foreign_DR_subset = malloc(input_values->numDesignRuleSets * sizeof(int **));
  if (input_values->foreign_DR_subset == 0)  {
    printf("\nERROR: Failed to allocate memory for 'input_values->foreign_DR_subset'.\n\n");
    exit (1);
  }
  for (int i = 0; i < input_values->numDesignRuleSets; i++ )  {
    input_values->foreign_DR_subset[i] = malloc(input_values->numDesignRuleSubsets[i] * sizeof(int *));
    if (input_values->foreign_DR_subset[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->foreign_DR_subset[%d]'.\n\n", i);
      exit (1);
    }
    for (int j = 0; j < input_values->numDesignRuleSubsets[i]; j++)  {
      input_values->foreign_DR_subset[i][j] = malloc(input_values->numDesignRuleSets * sizeof(int));
      if (input_values->foreign_DR_subset[i][j] == 0)  {
        printf("\nERROR: Failed to allocate memory for 'input_values->foreign_DR_subset[%d][%d]'.\n\n", i, j);
        exit (1);
      }
    }  // End of for-loop for index 'j'
  }  // End of for-loop for index 'i'

  //
  // Allocate memory for string variables associated with 'DR_zone' commands in the user-
  // defined input text file:
  //
  input_values->DR_zone_name       = malloc(input_values->num_DR_zones * sizeof(char * ));
  input_values->DR_zone_layer      = malloc(input_values->num_DR_zones * sizeof(char * ));
  input_values->DR_zone_shape      = malloc(input_values->num_DR_zones * sizeof(char * ));
  input_values->DR_zone_num_params = malloc(input_values->num_DR_zones * sizeof(char * ));
  input_values->DR_zone_parameters = malloc(input_values->num_DR_zones * sizeof(float *));
  for (int i = 0; i < input_values->num_DR_zones; i++ )  {

    input_values->DR_zone_name[i] = malloc(maxDesRuleSetNameLength * sizeof(char));
    if (input_values->DR_zone_name[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->DR_zone_name[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->DR_zone_layer[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->DR_zone_layer[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->DR_zone_layer[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->DR_zone_shape[i] = malloc(maxDRzoneShapeLength * sizeof(char));
    if (input_values->DR_zone_shape[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->DR_zone_shape[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->DR_zone_parameters[i] = malloc(maxBlockParameters * sizeof(float));
    if (input_values->DR_zone_parameters[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->DR_zone_parameters[%d]' array.\n\n", i);
      exit (1);
    }

  }  // End of for-loop for index i (0 to num_DR_zones)

  //
  // Allocate memory for string variables associated with 'trace_cost_zone'
  // commands in the user-defined input text file:
  //
  input_values->trace_cost_zone_index      = malloc(input_values->num_trace_cost_zone_instructions * sizeof(char   ));
  input_values->trace_cost_zone_layer      = malloc(input_values->num_trace_cost_zone_instructions * sizeof(char * ));
  input_values->trace_cost_zone_shape      = malloc(input_values->num_trace_cost_zone_instructions * sizeof(char * ));
  input_values->trace_cost_num_params      = malloc(input_values->num_trace_cost_zone_instructions * sizeof(char   ));
  input_values->trace_cost_zone_parameters = malloc(input_values->num_trace_cost_zone_instructions * sizeof(float *));
  for (int i = 0; i < input_values->num_trace_cost_zone_instructions; i++ )  {

    input_values->trace_cost_zone_layer[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->trace_cost_zone_layer[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->trace_cost_zone_layer[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->trace_cost_zone_shape[i] = malloc(maxCostShapeLength * sizeof(char));
    if (input_values->trace_cost_zone_shape[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->trace_cost_zone_shape[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->trace_cost_zone_parameters[i] = malloc(maxCostParameters * sizeof(float));
    if (input_values->trace_cost_zone_parameters[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->trace_cost_zone_parameters[%d]' array.\n\n", i);
      exit (1);
    }

  }  // End of for-loop for index i (0 to num_trace_cost_zone_instructions)

  //
  // Allocate memory for string variables associated with 'via_cost_zone'
  // commands in the user-defined input text file:
  //
  input_values->via_cost_zone_index      = malloc(input_values->num_via_cost_zone_instructions * sizeof(char   ));
  input_values->via_cost_zone_layer      = malloc(input_values->num_via_cost_zone_instructions * sizeof(char * ));
  input_values->via_cost_zone_shape      = malloc(input_values->num_via_cost_zone_instructions * sizeof(char * ));
  input_values->via_cost_num_params      = malloc(input_values->num_via_cost_zone_instructions * sizeof(char   ));
  input_values->via_cost_zone_parameters = malloc(input_values->num_via_cost_zone_instructions * sizeof(float *));
  for (int i = 0; i < input_values->num_via_cost_zone_instructions; i++ )  {

    input_values->via_cost_zone_layer[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->via_cost_zone_layer[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->via_cost_zone_layer[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->via_cost_zone_shape[i] = malloc(maxCostShapeLength * sizeof(char));
    if (input_values->via_cost_zone_shape[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->via_cost_zone_shape[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->via_cost_zone_parameters[i] = malloc(maxCostParameters * sizeof(float));
    if (input_values->via_cost_zone_parameters[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->via_cost_zone_parameters[%d]' array.\n\n", i);
      exit (1);
    }

  }  // End of for-loop for index i (0 to num_via_cost_zone_instructions)

  //
  // Allocate memory for string variables associated with 'pin_swap' and
  // 'no_pin_swap' commands in the user-defined input text file:
  //
  input_values->swap_command    = malloc(input_values->num_swap_instructions * sizeof(char * ));
  input_values->swap_shape      = malloc(input_values->num_swap_instructions * sizeof(char * ));
  input_values->swap_layer      = malloc(input_values->num_swap_instructions * sizeof(char * ));
  input_values->swap_num_params = malloc(input_values->num_swap_instructions * sizeof(char   ));
  input_values->swap_parameters = malloc(input_values->num_swap_instructions * sizeof(float *));
  for (int i = 0; i < input_values->num_swap_instructions; i++ )  {

    input_values->swap_command[i] = malloc(maxPinSwapInstructionLength * sizeof(char));
    if (input_values->swap_command[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->swap_command[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->swap_layer[i] = malloc(maxLayerNameLength * sizeof(char));
    if (input_values->swap_layer[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->swap_layer[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->swap_shape[i] = malloc(maxPinSwapShapeLength * sizeof(char));
    if (input_values->swap_shape[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->shape_shape[%d]' string.\n\n", i);
      exit (1);
    }

    input_values->swap_parameters[i] = malloc(maxPinSwapParameters * sizeof(float));
    if (input_values->swap_parameters[i] == 0)  {
      printf("\nERROR: Failed to allocate memory for 'input_values->swap_parameters[%d]' array.\n\n", i);
      exit (1);
    }

  }  // End of for-loop for index i (0 to num_swap_instructions)

  //
  // Initialize elements of arrays with 'maxTraceCostMultipliers' elements:
  //
  for (int i = 0; i < maxTraceCostMultipliers; i++)  {
    input_values->traceCostMultiplierInvoked[i] = FALSE;
  }  // End of for-loop for index 'i' (0 to maxTraceCostMultipliers)

  //
  // Initialize elements of arrays with 'maxViaCostMultipliers' elements:
  //
  for (int i = 0; i < maxViaCostMultipliers; i++)  {
    input_values->viaCostMultiplierInvoked[i] = FALSE;
  }  // End of for-loop for index 'i' (0 to maxViaCostMultipliers)


}  // End of function 'initialize_input_values'


//-----------------------------------------------------------------------------
// Name: freeMemory_input_values
// Desc: Free the memory that was allocated in function 'initialize_input_values'.
//-----------------------------------------------------------------------------
void freeMemory_input_values(InputValues_t *input_values)  {

  //
  // Free memory for data structures used for each net, including pseudo nets and the Acorn-defined
  // 'global repellent' net:
  //
  int max_routed_nets = input_values->num_nets + input_values->num_pseudo_nets + 1;

  for (int i = 0; i < max_routed_nets; i++)  {
    free(input_values->diffPairPartnerName[i]);          input_values->diffPairPartnerName[i]         = NULL;
    free(input_values->netSpecificRuleName[i]);          input_values->netSpecificRuleName[i]         = NULL;
    free(input_values->diffPairPitchCells[i]);           input_values->diffPairPitchCells[i]          = NULL;
    free(input_values->diffPairPitchMicrons[i]);         input_values->diffPairPitchMicrons[i]        = NULL;
    free(input_values->net_name[i]);                     input_values->net_name[i]                    = NULL;
    free(input_values->start_layer[i]);                  input_values->start_layer[i]                 = NULL;
    free(input_values->end_layer[i]);                    input_values->end_layer[i]                   = NULL;
    free(input_values->designRuleSubsetMap[i]);          input_values->designRuleSubsetMap[i]         = NULL;
  }

  //
  // Free memory for data associated with each routing layer:
  for (int i = 0; i < maxRoutingLayers; i++ )  {
    free(input_values->routingLayerNames[i]);     input_values->routingLayerNames[i] = NULL;
  }

  // Free 1-dimensional arrays:
  free(input_values->net_name);                 input_values->net_name = NULL;
  free(input_values->start_layer);              input_values->start_layer = NULL;
  free(input_values->end_layer);                input_values->end_layer = NULL;
  free(input_values->start_X_um);               input_values->start_X_um = NULL;
  free(input_values->start_Y_um);               input_values->start_Y_um = NULL;
  free(input_values->end_X_um);                 input_values->end_X_um = NULL;
  free(input_values->end_Y_um);                 input_values->end_Y_um = NULL;
  free(input_values->isDiffPair);               input_values->isDiffPair = NULL;
  free(input_values->isPNswappable);            input_values->isPNswappable = NULL;
  free(input_values->isPseudoNet);              input_values->isPseudoNet = NULL;
  free(input_values->diffPairPartner);          input_values->diffPairPartner = NULL;
  free(input_values->diffPairPartnerName);      input_values->diffPairPartnerName = NULL;
  free(input_values->diffPairPitchCells);       input_values->diffPairPitchCells = NULL;
  free(input_values->diffPairPitchMicrons);     input_values->diffPairPitchMicrons = NULL;
  free(input_values->diffPairToPseudoNetMap);   input_values->diffPairToPseudoNetMap = NULL;
  free(input_values->rats_nest_length_um);      input_values->rats_nest_length_um = NULL;
  free(input_values->usesSpecialRule);          input_values->usesSpecialRule = NULL;
  free(input_values->netSpecificRuleName);      input_values->netSpecificRuleName = NULL;
  free(input_values->designRuleSubsetMap);      input_values->designRuleSubsetMap = NULL;
  free(input_values->pseudoNetToDiffPair_1);    input_values->pseudoNetToDiffPair_1 = NULL;
  free(input_values->pseudoNetToDiffPair_2);    input_values->pseudoNetToDiffPair_2 = NULL;
  free(input_values->diffPairStartTermPitchMicrons);   input_values->diffPairStartTermPitchMicrons = NULL;
  free(input_values->diffPairEndTermPitchMicrons);     input_values->diffPairEndTermPitchMicrons = NULL;
  free(input_values->diffPairStartTermPitch);   input_values->diffPairStartTermPitch = NULL;
  free(input_values->diffPairEndTermPitch);     input_values->diffPairEndTermPitch = NULL;

  // Free memory for data structures required for layer (routing layer and via layer)
  for (int i = 0; i < 2*maxRoutingLayers - 1; i++ )  {
    free(input_values->layer_names[i]); input_values->layer_names[i] = NULL;
  }  // End of for-loop for index i (0 to 2*maxRoutingLayers-1)

  free(input_values->origin); input_values->origin = NULL;

  //
  // Free memory for string variables associated with 'BLOCK' commands in the user-
  // defined input text file:
  //
  for (int i = 0; i < input_values->num_block_instructions; i++ )  {
    free(input_values->block_command[i]); input_values->block_command[i] = NULL;
    free(input_values->block_type[i]); input_values->block_type[i] = NULL;
    free(input_values->block_layer[i]); input_values->block_layer[i] = NULL;
    free(input_values->block_parameters[i]); input_values->block_parameters[i] = NULL;
  }  // End of for-loop for index i (0 to num_block_instructions)
  free(input_values->block_command);    input_values->block_command = NULL;
  free(input_values->block_type);       input_values->block_type = NULL;
  free(input_values->block_layer);      input_values->block_layer = NULL;
  free(input_values->block_num_params); input_values->block_num_params = NULL;
  free(input_values->block_parameters); input_values->block_parameters = NULL;


  //
  // Free memory associated with design rules:
  //
  for (int DR_set = 0; DR_set < input_values->numDesignRuleSets; DR_set++ )  {
    for (int DR_subset = 0; DR_subset < input_values->numDesignRuleSubsets[DR_set]; DR_subset++)  {
      free(input_values->designRules[DR_set][DR_subset].subsetName);
      input_values->designRules[DR_set][DR_subset].subsetName = NULL;
    }  // End of for-loop for index DR_subset (0 to numDesignRuleSubsets[DR_set])

    free(input_values->DR_subsetUsed[DR_set]);             input_values->DR_subsetUsed[DR_set] = NULL;
    free(input_values->designRules[DR_set]);               input_values->designRules[DR_set] = NULL;
    free(input_values->designRuleSetName[DR_set]);         input_values->designRuleSetName[DR_set] = NULL;
    free(input_values->designRuleSetDescription[DR_set]);  input_values->designRuleSetDescription[DR_set] = NULL;

  }  // End of for-loop for index DR_set (0 to numDesignRuleSets)
  free(input_values->designRuleUsed);      input_values->designRuleUsed = NULL;
  free(input_values->DR_subsetUsed);       input_values->DR_subsetUsed = NULL;


  // Free 4-dimensional arrays that contain design-rule values:
  for (int DR_set_1 = 0; DR_set_1 < input_values->numDesignRuleSets; DR_set_1++ )  {
    const short num_subset_shapeTypes_1 = NUM_SHAPE_TYPES * input_values->numDesignRuleSubsets[DR_set_1];
    for (int subset_shapeType_1 = 0; subset_shapeType_1 < num_subset_shapeTypes_1; subset_shapeType_1++)  {
      for (int DR_set_2 = 0; DR_set_2 < input_values->numDesignRuleSets; DR_set_2++ )  {
        free(input_values->cong_radius[DR_set_1][subset_shapeType_1][DR_set_2]);
        input_values->cong_radius[DR_set_1][subset_shapeType_1][DR_set_2] = NULL;

        free(input_values->cong_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2]);
        input_values->cong_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2] = NULL;

        free(input_values->DRC_radius[DR_set_1][subset_shapeType_1][DR_set_2]);
        input_values->DRC_radius[DR_set_1][subset_shapeType_1][DR_set_2] = NULL;

        free(input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2]);
        input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1][DR_set_2] = NULL;

        free(input_values->detour_distance[DR_set_1][subset_shapeType_1][DR_set_2]);
        input_values->detour_distance[DR_set_1][subset_shapeType_1][DR_set_2] = NULL;

      }  // End of for-loop for index 'DR_set_2'

      free(input_values->cong_radius[DR_set_1][subset_shapeType_1]);
      input_values->cong_radius[DR_set_1][subset_shapeType_1] = NULL;

      free(input_values->cong_radius_squared[DR_set_1][subset_shapeType_1]);
      input_values->cong_radius_squared[DR_set_1][subset_shapeType_1] = NULL;

      free(input_values->DRC_radius[DR_set_1][subset_shapeType_1]);
      input_values->DRC_radius[DR_set_1][subset_shapeType_1] = NULL;

      free(input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1]);
      input_values->DRC_radius_squared[DR_set_1][subset_shapeType_1] = NULL;

      free(input_values->detour_distance[DR_set_1][subset_shapeType_1]);
      input_values->detour_distance[DR_set_1][subset_shapeType_1] = NULL;

    }  // End of for-loop for index 'subset_shapeType_1'

    free(input_values->cong_radius[DR_set_1]);
    input_values->cong_radius[DR_set_1] = NULL;

    free(input_values->cong_radius_squared[DR_set_1]);
    input_values->cong_radius_squared[DR_set_1] = NULL;

    free(input_values->DRC_radius[DR_set_1]);
    input_values->DRC_radius[DR_set_1] = NULL;

    free(input_values->DRC_radius_squared[DR_set_1]);
    input_values->DRC_radius_squared[DR_set_1] = NULL;

    free(input_values->detour_distance[DR_set_1]);
    input_values->detour_distance[DR_set_1] = NULL;

  }  // End of for-loop for  index 'DR_set_1'

  free(input_values->designRules);         input_values->designRules = NULL;
  free(input_values->cong_radius);         input_values->cong_radius = NULL;
  free(input_values->cong_radius_squared); input_values->cong_radius_squared = NULL;
  free(input_values->DRC_radius);          input_values->DRC_radius = NULL;
  free(input_values->DRC_radius_squared);  input_values->DRC_radius_squared = NULL;
  free(input_values->detour_distance);     input_values->detour_distance = NULL;

  //
  // Free memory for 3-dimensional array 'foreign_DR_subset[DR_sets][DR_subsets][DR_sets]':
  //
  for (int i = 0; i < input_values->numDesignRuleSets; i++ )  {
    for (int j = 0; j < input_values->numDesignRuleSubsets[i]; j++)  {
      free(input_values->foreign_DR_subset[i][j]);
      input_values->foreign_DR_subset[i][j] = NULL;
    }  // End of for-loop for index 'j'
    free(input_values->foreign_DR_subset[i]);
    input_values->foreign_DR_subset[i] = NULL;
  }  // End of for-loop for index 'i'
  free(input_values->foreign_DR_subset);
  input_values->foreign_DR_subset = NULL;


  //
  // Free memory for string variables associated with 'DR_zone' commands in the user-
  // defined input text file:
  //
  for (int i = 0; i < input_values->num_DR_zones; i++ )  {
    free(input_values->DR_zone_name[i]); input_values->DR_zone_name[i] = NULL;
    free(input_values->DR_zone_layer[i]); input_values->DR_zone_layer[i] = NULL;
    free(input_values->DR_zone_shape[i]); input_values->DR_zone_shape[i] = NULL;
    free(input_values->DR_zone_parameters[i]); input_values->DR_zone_parameters[i] = NULL;
  }  // End of for-loop for index i (0 to num_DR_zones)
  free(input_values->DR_zone_name);       input_values->DR_zone_name = NULL;
  free(input_values->DR_zone_layer);      input_values->DR_zone_layer = NULL;
  free(input_values->DR_zone_shape);      input_values->DR_zone_shape = NULL;
  free(input_values->DR_zone_num_params); input_values->DR_zone_num_params = NULL;
  free(input_values->DR_zone_parameters); input_values->DR_zone_parameters = NULL;

  //
  // Free memory for string variables associated with 'trace_cost_zone'
  // commands in the user-defined input text file:
  //
  for (int i = 0; i < input_values->num_trace_cost_zone_instructions; i++ )  {
    free(input_values->trace_cost_zone_layer[i]);      input_values->trace_cost_zone_layer[i] = NULL;
    free(input_values->trace_cost_zone_shape[i]);      input_values->trace_cost_zone_shape[i] = NULL;
    free(input_values->trace_cost_zone_parameters[i]); input_values->trace_cost_zone_parameters[i] = NULL;
  }  // End of for-loop for index i (0 to num_trace_cost_zone_instructions)
  free(input_values->trace_cost_zone_index);      input_values->trace_cost_zone_index = NULL;
  free(input_values->trace_cost_zone_layer);      input_values->trace_cost_zone_layer = NULL;
  free(input_values->trace_cost_zone_shape);      input_values->trace_cost_zone_shape = NULL;
  free(input_values->trace_cost_num_params);      input_values->trace_cost_num_params = NULL;
  free(input_values->trace_cost_zone_parameters); input_values->trace_cost_zone_parameters = NULL;

  //
  // Free memory for string variables associated with 'via_cost_zone'
  // commands in the user-defined input text file:
  //
  for (int i = 0; i < input_values->num_via_cost_zone_instructions; i++ )  {
    free(input_values->via_cost_zone_layer[i]);      input_values->via_cost_zone_layer[i]      = NULL;
    free(input_values->via_cost_zone_shape[i]);      input_values->via_cost_zone_shape[i]      = NULL;
    free(input_values->via_cost_zone_parameters[i]); input_values->via_cost_zone_parameters[i] = NULL;
  }  // End of for-loop for index i (0 to num_via_cost_zone_instructions)
  free(input_values->via_cost_zone_index);      input_values->via_cost_zone_index      = NULL;
  free(input_values->via_cost_zone_layer);      input_values->via_cost_zone_layer      = NULL;
  free(input_values->via_cost_zone_shape);      input_values->via_cost_zone_shape      = NULL;
  free(input_values->via_cost_num_params);      input_values->via_cost_num_params      = NULL;
  free(input_values->via_cost_zone_parameters); input_values->via_cost_zone_parameters = NULL;

  //
  // Free memory for string variables associated with 'pin_swap' and
  // 'no_pin_swap' commands in the user-defined input text file:
  //
  for (int i = 0; i < input_values->num_swap_instructions; i++ )  {
    free(input_values->swap_command[i]);    input_values->swap_command[i]    = NULL;
    free(input_values->swap_layer[i]);      input_values->swap_layer[i]      = NULL;
    free(input_values->swap_shape[i]);      input_values->swap_shape[i]      = NULL;
    free(input_values->swap_parameters[i]); input_values->swap_parameters[i] = NULL;
  }  // End of for-loop for index i (0 to num_swap_instructions)
  free(input_values->swap_command);    input_values->swap_command    = NULL;
  free(input_values->swap_shape);      input_values->swap_shape      = NULL;
  free(input_values->swap_layer);      input_values->swap_layer      = NULL;
  free(input_values->swap_num_params); input_values->swap_num_params = NULL;
  free(input_values->swap_parameters); input_values->swap_parameters = NULL;

}  // End of function 'freeMemory_input_values'


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
void set_costs_to_base_values(InputValues_t *user_inputs)  {

  // Set all trace costs to their baseline values:
  for (int cost_index = 0; cost_index < maxTraceCostMultipliers; cost_index++)  {
    user_inputs->cellCost[cost_index]   = user_inputs->baseCellCost;
    user_inputs->diagCost[cost_index]   = user_inputs->baseDiagCost;
    user_inputs->knightCost[cost_index] = user_inputs->baseKnightCost;
    // printf("DEBUG:   Set cellCost[%d] to %'lu\n", cost_index, user_inputs->cellCost[cost_index]);
    // printf("DEBUG:   Set diagCost[%d] to %'lu\n", cost_index, user_inputs->diagCost[cost_index]);
    // printf("DEBUG: Set knightCost[%d] to %'lu\n\n", cost_index, user_inputs->knightCost[cost_index]);
  }

  // Set all via (vertical) costs to their baseline values:
  for (int cost_index = 0; cost_index < maxViaCostMultipliers; cost_index++)  {
    user_inputs->vertCost[cost_index] = user_inputs->baseVertCost;
  }

}  // End of function 'set_costs_to_base_valuesf'


//-----------------------------------------------------------------------------
// Name: set_costs_to_userDefined_values
// Desc: Sets the following costs to the values defined by the user, accounting 
//       for the user-defined cost-multipliers: 
//         1. cell_cost
//         2. diag_cost
//         3. knight_cost
//         4. vert_cost
//-----------------------------------------------------------------------------
void set_costs_to_userDefined_values(InputValues_t *user_inputs)  {

  // Set all trace costs to their baseline values multiplied by the user-defined
  // cost-multipliers:
  for (int cost_index = 0; cost_index < maxTraceCostMultipliers; cost_index++)  {

    // Ensure that multiplier = 1 for index #0
    if (cost_index == 0)  {
      user_inputs->traceCostMultiplier[cost_index] = 1;
    }  // End of if-bock for cost_index == 0

    user_inputs->cellCost[cost_index]   = user_inputs->baseCellCost   * user_inputs->traceCostMultiplier[cost_index];
    user_inputs->diagCost[cost_index]   = user_inputs->baseDiagCost   * user_inputs->traceCostMultiplier[cost_index];
    user_inputs->knightCost[cost_index] = user_inputs->baseKnightCost * user_inputs->traceCostMultiplier[cost_index];
    // printf("DEBUG:   Set cellCost[%d] to %'lu\n", cost_index, user_inputs->cellCost[cost_index]);
    // printf("DEBUG:   Set diagCost[%d] to %'lu\n", cost_index, user_inputs->diagCost[cost_index]);
    // printf("DEBUG: Set knightCost[%d] to %'lu\n\n", cost_index, user_inputs->knightCost[cost_index]);
  }

  // Set all via (vertical) costs to their baseline values multiplied by the user-defined
  // cost-multipliers:
  for (int cost_index = 0; cost_index < maxViaCostMultipliers; cost_index++)  {

    // Ensure that multiplier = 1 for index #0
    if (cost_index == 0)  {
      user_inputs->traceCostMultiplier[cost_index] = 1;
    }  // End of if-bock for cost_index == 0

    user_inputs->vertCost[cost_index] = user_inputs->baseVertCost * user_inputs->viaCostMultiplier[cost_index];
  }

}  // End of function 'set_costs_to_userDefined_values'


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
void verifyDiffPairTerminals(const InputValues_t *user_inputs, CellInfo_t ***cellInfo, MapInfo_t *mapInfo)  {

  // printf("DEBUG: Entered function 'verifyDiffPairTerminals'...\n");

  int fatal_error = FALSE; // Flag is set to TRUE if error is found.

  // Iterate through each user-defined net (excluding pseudo-nets):
  for (int path = 0; path < user_inputs->num_nets; path++)  {

    // Check whether net is part of a differential pair:
    if (user_inputs->isDiffPair[path])  {

      // We got here, so 'path' is part of a differential pair of nets.
      // Get the path number of the partner net:
      const int partner = user_inputs->diffPairPartner[path];

      // Get the path number of the pseudo-net associated with this net:
      const int pseudoNetNum = user_inputs->diffPairToPseudoNetMap[path];

      // printf("DEBUG: =========== Analyzing path %d and partner %d (pseudo-parent: %d) ===========\n",
      //        path, partner, pseudoNetNum);

      // Verify that the 'isPNswappable' flag is set for both diff-pair nets
      // if the user set this flag for either net. Also set this flag
      // for the 'parent' pseudo-net:
      if (user_inputs->isPNswappable[path])  {
        user_inputs->isPNswappable[partner] = TRUE;
        user_inputs->isPNswappable[pseudoNetNum] = TRUE;
      }

      // Get the start- and end-coordinates in cell-units of both diff-pair nets and the
      // associated pseudo-path. Account for the possibility that the start- and end-
      // terminals may have previously been swapped. In other words, the variables below
      // refer to the USER-DEFINED start- and end-terminal coordinates.
      int path_startX, path_startY, path_startZ, partner_startX, partner_startY, partner_startZ;
      int path_endX, path_endY, path_endZ, partner_endX, partner_endY, partner_endZ;
      int pseudo_startX, pseudo_startY, pseudo_startZ, pseudo_endX, pseudo_endY, pseudo_endZ;
      if (! mapInfo->start_end_terms_swapped[path])  {
        path_startX    = mapInfo->start_cells[path].X;
        path_startY    = mapInfo->start_cells[path].Y;
        path_startZ    = mapInfo->start_cells[path].Z;
        partner_startX = mapInfo->start_cells[partner].X;
        partner_startY = mapInfo->start_cells[partner].Y;
        partner_startZ = mapInfo->start_cells[partner].Z;
        path_endX      = mapInfo->end_cells[path].X;
        path_endY      = mapInfo->end_cells[path].Y;
        path_endZ      = mapInfo->end_cells[path].Z;
        partner_endX   = mapInfo->end_cells[partner].X;
        partner_endY   = mapInfo->end_cells[partner].Y;
        partner_endZ   = mapInfo->end_cells[partner].Z;
        pseudo_startX  = mapInfo->start_cells[pseudoNetNum].X;
        pseudo_startY  = mapInfo->start_cells[pseudoNetNum].Y;
        pseudo_startZ  = mapInfo->start_cells[pseudoNetNum].Z;
        pseudo_endX    = mapInfo->end_cells[pseudoNetNum].X;
        pseudo_endY    = mapInfo->end_cells[pseudoNetNum].Y;
        pseudo_endZ    = mapInfo->end_cells[pseudoNetNum].Z;
      }
      else  {
        path_startX    = mapInfo->end_cells[path].X;
        path_startY    = mapInfo->end_cells[path].Y;
        path_startZ    = mapInfo->end_cells[path].Z;
        partner_startX = mapInfo->end_cells[partner].X;
        partner_startY = mapInfo->end_cells[partner].Y;
        partner_startZ = mapInfo->end_cells[partner].Z;
        path_endX      = mapInfo->start_cells[path].X;
        path_endY      = mapInfo->start_cells[path].Y;
        path_endZ      = mapInfo->start_cells[path].Z;
        partner_endX   = mapInfo->start_cells[partner].X;
        partner_endY   = mapInfo->start_cells[partner].Y;
        partner_endZ   = mapInfo->start_cells[partner].Z;
        pseudo_startX  = mapInfo->end_cells[pseudoNetNum].X;
        pseudo_startY  = mapInfo->end_cells[pseudoNetNum].Y;
        pseudo_startZ  = mapInfo->end_cells[pseudoNetNum].Z;
        pseudo_endX    = mapInfo->start_cells[pseudoNetNum].X;
        pseudo_endY    = mapInfo->start_cells[pseudoNetNum].Y;
        pseudo_endZ    = mapInfo->start_cells[pseudoNetNum].Z;
      }

      // Get design-rule number for static (user-defined) start- and end-terminals of nets 'path' and 'partner'.
      int const static_start_design_rule         = cellInfo[path_startX][path_startY][path_startZ].designRuleSet;
      int const static_end_design_rule           = cellInfo[path_endX][path_endY][path_endZ].designRuleSet;
      int const static_start_design_rule_partner = cellInfo[partner_startX][partner_startY][partner_startZ].designRuleSet;
      int const static_end_design_rule_partner   = cellInfo[partner_endX][partner_endY][partner_endZ].designRuleSet;

      // Get the design-rule subsets for the static (user-defined) start- and end-terminals of nets 'path' and 'partners':
      int const static_start_DR_subset           = user_inputs->designRuleSubsetMap[path][static_start_design_rule];
      int const static_end_DR_subset             = user_inputs->designRuleSubsetMap[path][static_end_design_rule];


      // Get the starting- and ending-coordinates of both diff-pair nets
      // in units of microns, as defined by the user. Note that Acorn does
      // not swap the variables that hold the start- and end-coordinates in
      // micron units:
      float const start_X_um         = user_inputs->start_X_um[path];
      float const start_Y_um         = user_inputs->start_Y_um[path];
      float const end_X_um           = user_inputs->end_X_um[path];
      float const end_Y_um           = user_inputs->end_Y_um[path];
      float const start_X_um_partner = user_inputs->start_X_um[partner];
      float const start_Y_um_partner = user_inputs->start_Y_um[partner];
      float const end_X_um_partner   = user_inputs->end_X_um[partner];
      float const end_Y_um_partner   = user_inputs->end_Y_um[partner];

      // Calculate the separation (in cells) between the start-terminals. Repeat for the end-terminals:
      int const static_start_separation_squared =   (path_startX - partner_startX) *  (path_startX - partner_startX)
                                                  + (path_startY - partner_startY) *  (path_startY - partner_startY);
      int const static_end_separation_squared   =   (path_endX - partner_endX) *  (path_endX - partner_endX)
                                                  + (path_endY - partner_endY) *  (path_endY - partner_endY);

      // Calculate the diff-pair pitch for the start-terminals:
      user_inputs->diffPairStartTermPitchMicrons[path] = calc_2D_Pythagorean_distance_floats(start_X_um, start_Y_um, start_X_um_partner, start_Y_um_partner);
      user_inputs->diffPairStartTermPitch[path]        = (int)round(user_inputs->diffPairStartTermPitchMicrons[path] / user_inputs->cell_size_um);

      // Calculate the diff-pair pitch for the end-terminals:
      user_inputs->diffPairEndTermPitchMicrons[path]   = calc_2D_Pythagorean_distance_floats(end_X_um, end_Y_um, end_X_um_partner, end_Y_um_partner);
      user_inputs->diffPairEndTermPitch[path]          = (int)round(user_inputs->diffPairEndTermPitchMicrons[path] / user_inputs->cell_size_um);

      // Copy the diff-pair pitch values from the 'user_inputs' structure to the 'mapInfo' structure. Note that
      // we reverse the start/end-pitches in the mapInfo structure if the start/end-terminals were previously swapped:
      if (! mapInfo->start_end_terms_swapped[path])  {
        // We got here, so the start- and end-terminals have not previously been swapped:
        mapInfo->diffPairStartTermPitchMicrons[path] = user_inputs->diffPairStartTermPitchMicrons[path];
        mapInfo->diffPairEndTermPitchMicrons[path]   = user_inputs->diffPairEndTermPitchMicrons[path];
      }
      else  {
        // We got here, so the start- and end-terminals were previously swapped:
        mapInfo->diffPairStartTermPitchMicrons[path] = user_inputs->diffPairEndTermPitchMicrons[path];
        mapInfo->diffPairEndTermPitchMicrons[path]   = user_inputs->diffPairStartTermPitchMicrons[path];
      }

      // printf("DEBUG: In verifyDiffPairTerminals:\n");
      // printf("DEBUG:      Static path start coordinates: (%d,%d,%d) cells\n", path_startX, path_startY, path_startZ);
      // printf("DEBUG:                                     (%.3f, %.3f, %d) microns\n", start_X_um, start_Y_um, path_startZ);
      // printf("DEBUG:        Static path end coordinates: (%d,%d,%d)\n", path_endX, path_endY, path_endZ);
      // printf("DEBUG:                                     (%.3f, %.3f, %d) microns\n", end_X_um, end_Y_um, path_endZ);
      // printf("DEBUG:   Static partner start coordinates: (%d,%d,%d) cells\n", partner_startX, partner_startY, partner_startZ);
      // printf("DEBUG:                                     (%.3f, %.3f, %d) microns\n", start_X_um_partner, start_Y_um_partner, partner_startZ);
      // printf("DEBUG:     Static partner end coordinates: (%d,%d,%d)\n", partner_endX, partner_endY, partner_endZ);
      // printf("DEBUG:                                     (%.3f, %.3f, %d) microns\n", end_X_um_partner, end_Y_um_partner, partner_endZ);
      // printf("DEBUG:           Static user_inputs->diffPairStartTermPitch = %d cells\n", user_inputs->diffPairStartTermPitch[path]);
      // printf("DEBUG:    Static user_inputs->diffPairStartTermPitchMicrons = %.3f microns\n", user_inputs->diffPairStartTermPitchMicrons[path]);
      // printf("DEBUG:             Static user_inputs->diffPairEndTermPitch = %d cells\n", user_inputs->diffPairEndTermPitch[path]);
      // printf("DEBUG:      Static user_inputs->diffPairEndTermPitchMicrons = %.3f microns\n", user_inputs->diffPairEndTermPitchMicrons[path]);
      // printf("DEBUG:       Dynamic mapInfo->diffPairStartTermPitchMicrons = %.3f microns\n", mapInfo->diffPairStartTermPitchMicrons[path]);
      // printf("DEBUG:         Dynamic mapInfo->diffPairEndTermPitchMicrons = %.3f microns\n", mapInfo->diffPairEndTermPitchMicrons[path]);


      // Get the swap-zone numbers for the start- and end-terminals of the two
      // diff-pair nets and the associated pseudo-path:
      const int static_start_swap_zone             = cellInfo[path_startX][path_startY][path_startZ].swap_zone;
      const int static_start_swap_zone_partner_net = cellInfo[partner_startX][partner_startY][partner_startZ].swap_zone;
      const int static_start_swap_zone_pseudo_net  = cellInfo[pseudo_startX][pseudo_startY][pseudo_startZ].swap_zone;
      const int static_end_swap_zone               = cellInfo[path_endX][path_endY][path_endZ].swap_zone;
      const int static_end_swap_zone_partner_net   = cellInfo[partner_endX][partner_endY][partner_endZ].swap_zone;
      const int static_end_swap_zone_pseudo_net    = cellInfo[pseudo_endX][pseudo_endY][pseudo_endZ].swap_zone;


      // Calculate the pitch (in microns and cells) of diff-pairs' starting- and ending terminals. Also copy the
      // diff-pair terminal pitches to the mapInfo data structure. After certain iterations, the start- and
      // end-terminals are swapped, requiring the terminals pitches to also be swapped in the mapInfo structure
      // (but not in the user_inputs structure):


      // printf("DEBUG: In verifyDiffPairTerminals, the following terminal pitch values were calculated for net #%i:\n", path);
      // printf("DEBUG:      Static diffPairStartTermPitchMicrons = %6.3f microns\n", user_inputs->diffPairStartTermPitchMicrons[path]);
      // printf("DEBUG:        Static diffPairEndTermPitchMicrons = %6.3f microns\n", user_inputs->diffPairEndTermPitchMicrons[path]);
      // printf("DEBUG:             Static diffPairStartTermPitch = %d cells\n", user_inputs->diffPairStartTermPitch[path]);
      // printf("DEBUG:               Static diffPairEndTermPitch = %d cells\n", user_inputs->diffPairEndTermPitch[path]);

      int staticStartTermInSwapZone = FALSE;
      // Check if any of the start-terminals associated with this diff-pair net
      // are in a swap-zone:
      if (static_start_swap_zone || static_start_swap_zone_partner_net || static_start_swap_zone_pseudo_net)  {

        // Set the 'startTermInSwapZone' to TRUE, which will prevent certain other tests
        // from being applied to the start-terminals later on in this function:
        staticStartTermInSwapZone = TRUE;

        // At least one of the starting-terminals is in a swap-zone. Now
        // check whether all three are in the same swap-zone. If not, then
        // issue a fatal error message and die.
        if (   (static_start_swap_zone             != static_start_swap_zone_partner_net)
            || (static_start_swap_zone             != static_start_swap_zone_pseudo_net)
            || (static_start_swap_zone_partner_net != static_start_swap_zone_pseudo_net))  {

          printf("\n\nERROR: The following terminals are not all located in the same swap-zone, which is required for differential pairs:\n");
          printf(    "            Terminal of net '%s' on layer '%s' at (%6.3f, %6.3f) microns is in swap-zone #%d.\n", user_inputs->net_name[path],
                     user_inputs->start_layer[path], user_inputs->start_X_um[path], user_inputs->start_Y_um[path], static_start_swap_zone);
          printf(    "            Terminal of net '%s' on layer '%s' at (%6.3f, %6.3f) microns is in swap-zone #%d.\n", user_inputs->net_name[partner],
                  user_inputs->start_layer[partner], user_inputs->start_X_um[partner], user_inputs->start_Y_um[partner], static_start_swap_zone_partner_net);
          printf(    "            Pseudo-terminal of pseudo-net '%s' on layer '%s' at (%6.3f, %6.3f) microns is in swap-zone #%d.\n", user_inputs->net_name[pseudoNetNum],
                  user_inputs->start_layer[pseudoNetNum], user_inputs->start_X_um[pseudoNetNum], user_inputs->start_Y_um[pseudoNetNum], static_start_swap_zone_pseudo_net);
          printf(    "       (The pseudo-terminal is the mid-point between the diff-pair terminals.)\n");
          printf(    "       Please correct this issue in the input file and restart the program.\n\n");
          exit(1);
        }  // End of if-block for start-terminals being in different swap-zones

      }  // End of if-block for start-terminal(s) being in a swap-zone


      // If the starting-terminals are not in a swap-zone, then check that
      // their Z-coordinates are identical (on same layer). If not, issue an
      // error message and exit the program.
      if ((! staticStartTermInSwapZone) && (path_startZ != partner_startZ))  {
        printf("\nERROR: The starting terminals of diff-pair nets '%s' and '%s'\n",
                user_inputs->net_name[path], user_inputs->net_name[partner]);
        printf("       are not on the same layer. Net '%s' starts on layer\n",
                user_inputs->net_name[path]);
        printf("       '%s', but net '%s' starts on layer '%s'.\n",
                user_inputs->routingLayerNames[path_startZ],
                user_inputs->net_name[partner],
                user_inputs->routingLayerNames[partner_startZ]);
        printf("       Please modify the input file so that starting- and ending-terminals\n");
        printf("       for each diff-pair are on the same routing layer.\n\n");
        fatal_error = TRUE;
      }  // End of if-block for comparing the Z-coordinates of two starting terminals

      // Check that the Z-coordinate of the ending-terminals are
      // identical (on same layer). If not, issue an error message and
      // exit the program.
      if (path_endZ != partner_endZ)  {
        printf("\nERROR: The ending terminals of diff-pair nets '%s' and '%s'\n",
               user_inputs->net_name[path], user_inputs->net_name[partner]);
        printf("       are not on the same layer. Net '%s' ends on layer\n",
               user_inputs->net_name[path]);
        printf("       '%s', but net '%s' ends on layer '%s'.\n",
               user_inputs->routingLayerNames[path_endZ],
               user_inputs->net_name[partner],
               user_inputs->routingLayerNames[partner_endZ]);
        printf("       Please modify the input file so that starting- and ending-terminals\n");
        printf("       for each diff-pair are on the same routing layer.\n\n");
        fatal_error = TRUE;
      }  // End of if-block for comparing the Z-coordinates of two starting terminals


      //
      // If the starting-terminals are not in a swap-zone, then verify that they
      // are within the same design-rule zone:
      //


      // Confirm that design-rule set of net 'path' is same as that of net 'partner', at
      // these nets' starting terminals:
      if ((! staticStartTermInSwapZone) && (static_start_design_rule != static_start_design_rule_partner))  {
        printf("\nERROR: The starting terminals of diff-pair nets '%s' and '%s'\n",
               user_inputs->net_name[path], user_inputs->net_name[partner]);
        printf("       are located in different design-rule zones. The starting terminal of net '%s'\n",
               user_inputs->net_name[path]);
        printf("       is located in design-rule set '%s', whereas the starting terminal\n",
               user_inputs->designRuleSetName[static_start_design_rule]);
        printf("       for net '%s' is located in set '%s'.\n", user_inputs->net_name[partner],
               user_inputs->designRuleSetName[static_start_design_rule_partner]);
        printf("       Please modify the input to ensure that boundaries of design-rule zones do not\n");
        printf("       separate terminals of diff-pair nets.\n\n");
        fatal_error = TRUE;
      }

      //
      // Verify that the two ending terminals are within the same design-rule zone:
      //
      int staticEndTermInSwapZone = FALSE;
      // Check if any of the end-terminals associated with this diff-pair net
      // are in a swap-zone:
      if (static_end_swap_zone || static_end_swap_zone_partner_net || static_end_swap_zone_pseudo_net)  {
        // Set the 'staticEndTermInSwapZone' to TRUE, which will prevent certain other tests
        // from being applied to the end-terminals later on in this function:
        staticEndTermInSwapZone = TRUE;
      }


      // Confirm that design-rule set of net 'path' is same as that of net 'partner', at
      // these nets' ending terminals:
      if ((! staticEndTermInSwapZone) && (static_end_design_rule != static_end_design_rule_partner))  {
        printf("\nERROR: The ending terminals of diff-pair nets '%s' and '%s'\n",
               user_inputs->net_name[path], user_inputs->net_name[partner]);
        printf("       are located in different design-rule zones. The ending terminal of net '%s'\n",
               user_inputs->net_name[path]);
        printf("       is located in design-rule set '%s', whereas the ending terminal\n",
               user_inputs->designRuleSetName[static_end_design_rule]);
        printf("       for net '%s' is located in set '%s'.\n", user_inputs->net_name[partner],
               user_inputs->designRuleSetName[static_end_design_rule_partner]);
        printf("       Please modify the input to ensure that boundaries of design-rule zones do not\n");
        printf("       separate terminals of diff-pair nets.\n\n");
        fatal_error = TRUE;
      }


      //
      // If the user-defined start-terminals are not in a swap-zone, then verify that they are within
      // a reasonable distance of each other: 6 times the intra-pair pitch provided by the user.
      //
      // Determine the diff-pair pitch at the starting terminals, and calculate the square of this length:
      const float static_start_diffPair_pitch =  user_inputs->designRules[static_start_design_rule][static_start_DR_subset].diffPairPitchCells[TRACE];
      const float static_start_diffPair_pitch_squared = static_start_diffPair_pitch * static_start_diffPair_pitch;

      // printf("DEBUG: static_start_diffPair_pitch=%.3f cells, static_start_diffPair_pitch_squared=%.3f cells^2\n",
      //         static_start_diffPair_pitch, static_start_diffPair_pitch_squared);
      // printf("       static_start_separation_squared=%d cells^2, 36 * static_start_diffPair_pitch_squared = %.3f\n",
      //         static_start_separation_squared, 36 * static_start_diffPair_pitch_squared);

      // Compare separation of terminals to 6 times the pitch. (Note that 6^2 is 36.)
      // If greater, then issue fatal error message and exit:
      if ((! staticStartTermInSwapZone) && (static_start_separation_squared > 36 * static_start_diffPair_pitch_squared))  {
        printf("\nERROR: The starting terminals for diff-pair nets '%s' and '%s' are separated by\n",
               user_inputs->net_name[path], user_inputs->net_name[partner]);
        printf("       more than 6 times the pitch for these diff-pair nets near these terminals. For\n");
        printf("       reference, the starting terminals are located at (%6.3f, %6.3f) and (%6.3f, %6.3f) microns.\n",
               user_inputs->start_X_um[path],    user_inputs->start_Y_um[path],
               user_inputs->start_X_um[partner], user_inputs->start_Y_um[partner]);
        printf("       The intra-diff-pair pitch is %6.3f microns. Please modify the input\n",
               user_inputs->designRules[static_start_design_rule][static_start_DR_subset].traceDiffPairPitchMicrons);
        printf("       file and restart the program.\n\n");
        fatal_error = TRUE;
      }

      //
      // Verify that the two ending terminals are within a reasonable distance of each
      // other: 6 times the intra-pair pitch provided by the user.
      //
      // Determine the diff-pair pitch at the ending terminals, and calculate the square of this length:
      const int static_end_diffPair_pitch =  user_inputs->designRules[static_end_design_rule][static_end_DR_subset].diffPairPitchCells[TRACE];
      const int static_end_diffPair_pitch_squared = static_end_diffPair_pitch * static_end_diffPair_pitch;

      // printf("DEBUG: end_diffPair_pitch=%d cells, end_diffPair_pitch_squared=%d cells^2\n",
      //         static_end_diffPair_pitch, static_end_diffPair_pitch_squared);
      // printf("       end_separation_squared=%d cells^2, 36 * end_diffPair_pitch_squared = %d\n",
      //         static_end_separation_squared, 36 * static_end_diffPair_pitch_squared);

      // Compare separation of terminals to 6 times the pitch. (Note that 6^2 is 36.)
      // If greater, then issue fatal error message and exit:
      if ((! staticEndTermInSwapZone) && (static_end_separation_squared > 36 * static_end_diffPair_pitch_squared))  {
        printf("\nERROR: The ending terminals for diff-pair nets '%s' and '%s' are separated by\n",
               user_inputs->net_name[path], user_inputs->net_name[partner]);
        printf("       more than 6 times the pitch for these diff-pair nets near these terminals. For\n");
        printf("       reference, the ending terminals are located at (%6.3f, %6.3f) and (%6.3f, %6.3f) microns.\n",
               user_inputs->end_X_um[path],    user_inputs->end_Y_um[path],
               user_inputs->end_X_um[partner], user_inputs->end_Y_um[partner]);
        printf("       The intra-diff-pair pitch is %6.3f microns. Please modify the input\n",
               user_inputs->designRules[static_end_design_rule][static_end_DR_subset].traceDiffPairPitchMicrons);
        printf("       file and restart the program.\n\n");
        fatal_error = TRUE;
      }

      //
      // Verify that no other nets have terminals near the two starting- and two ending-
      // terminals. Specifically, no terminals should exist within a radius R of the
      // midpoint of the diff-pair's terminals. R is equal to half the distance between
      // these terminals.
      //
      // Calculate the midpoints of the diff pairs' starting and ending terminals:
      int X_midpoint_start = (path_startX + partner_startX) / 2;
      int Y_midpoint_start = (path_startY + partner_startY) / 2;
      int Z_midpoint_start = path_startZ;
      int X_midpoint_end   = (path_endX   + partner_endX  ) / 2;
      int Y_midpoint_end   = (path_endY   + partner_endY  ) / 2;
      int Z_midpoint_end   = path_endZ;

      // Calculate square of distance between the midpoint and the terminals. This
      // distance is equivalent to half distance between the two terminals:
      float const start_radius_squared = round(((path_startX - partner_startX) * (path_startX - partner_startX)
                                              + (path_startY - partner_startY) * (path_startY - partner_startY) ) / 4.0);

      float const end_radius_squared   = round(((path_endX - partner_endX)     * (path_endX - partner_endX)
                                              + (path_endY - partner_endY)     * (path_endY - partner_endY) )     / 4.0);

      //
      // Iterate through all other nets to confirm that their terminals are not located
      // near midpoint of diff pairs' starting- or ending-terminals. This requires 4
      // comparisons: (1) foreign net's starting-terminal to diff-pair's starting-terminals
      //              (2) foreign net's ending-terminal to diff-pair's starting-terminals
      //              (3) foreign net's starting-terminal to diff-pair's ending-terminals
      //              (4) foreign net's ending-terminal to diff-pair's ending-terminals
      //
      for (int other_path = 0; other_path < user_inputs->num_nets; other_path++)  {

        // Skip the path if it's either of the diff-pair nets that we're analyzing.
        if ((other_path == path)  || (other_path == partner))  {
          continue;
        }

        // Get coordinates of other path's user-defined starting and ending terminals:
        int other_path_startX, other_path_startY, other_path_startZ;
        int other_path_endX,   other_path_endY  , other_path_endZ;
        if (! mapInfo->start_end_terms_swapped[other_path])  {
          other_path_startX = mapInfo->start_cells[other_path].X;
          other_path_startY = mapInfo->start_cells[other_path].Y;
          other_path_startZ = mapInfo->start_cells[other_path].Z;
          other_path_endX   = mapInfo->end_cells[other_path].X;
          other_path_endY   = mapInfo->end_cells[other_path].Y;
          other_path_endZ   = mapInfo->end_cells[other_path].Z;
        }
        else {
          other_path_startX = mapInfo->end_cells[other_path].X;
          other_path_startY = mapInfo->end_cells[other_path].Y;
          other_path_startZ = mapInfo->end_cells[other_path].Z;
          other_path_endX   = mapInfo->start_cells[other_path].X;
          other_path_endY   = mapInfo->start_cells[other_path].Y;
          other_path_endZ   = mapInfo->start_cells[other_path].Z;
        }

        // printf("\nDEBUG: Other path #%d has user-defined start-terminal at (%d,%d,%d) and end-terminal at (%d,%d,%d).\n", other_path,
        //        other_path_startX, other_path_startY, other_path_startZ, other_path_endX, other_path_endY, other_path_endZ);

        // Comparison #1: Check if other path's starting terminal is on same
        // layer as diff pairs' starting terminals. Skip start-terminal if
        // it's in a swap-zone:
        if ((! staticStartTermInSwapZone) && (Z_midpoint_start == other_path_startZ))  {
          // Calculate distance between diff pairs' midpoint of starting terminals
          // and the starting terminal of other net:
          const int separation_squared =
                    (X_midpoint_start - other_path_startX) * (X_midpoint_start - other_path_startX)
                  + (Y_midpoint_start - other_path_startY) * (Y_midpoint_start - other_path_startY);

          // Compare separation to radius and issue fatal error message if separation <= radius:
          if (separation_squared <= start_radius_squared)  {
            // Issue fatal error message and exit:
            printf("\nERROR: The starting terminal for net '%s', located at (%6.3f, %6.3f) microns on layer '%s',\n",
                    user_inputs->net_name[other_path], user_inputs->start_X_um[other_path],
                    user_inputs->start_Y_um[other_path], user_inputs->routingLayerNames[other_path_startZ]);
            printf("       is too close to the starting terminals of diff-pair net '%s' on the same layer,\n",
                    user_inputs->net_name[path]);
            printf("       with terminal coordinates of (%6.3f, %6.3f) and (%6.3f, %6.3f).\n",
                    user_inputs->start_X_um[path],    user_inputs->start_Y_um[path],
                    user_inputs->start_X_um[partner], user_inputs->start_Y_um[partner]);
            		printf("       Please modify the coordinates of the nets' terminals such that no terminals are too\n");
            printf("       close to the starting- or ending-terminals of differential-pair nets.\n\n");
            fatal_error = TRUE;
          }  // End of if-block for foreign terminal being close to diff-pair terminals' midpoint
        }  // End of if-block for other start-terminal being on same layer as diff-pair start-terminals


        // Comparison #2: Check if other path's ending terminal is on same layer
        // as diff pairs' starting terminals:
        if ((! staticStartTermInSwapZone) && (Z_midpoint_start == other_path_endZ))  {
          // Calculate distance between diff pairs' midpoint of starting terminals
          // and the ending terminal of other net:
          const int separation_squared =
                    (X_midpoint_start - other_path_endX) * (X_midpoint_start - other_path_endX)
                  + (Y_midpoint_start - other_path_endY) * (Y_midpoint_start - other_path_endY);

          // Compare separation to radius and issue fatal error message if separation <= radius:
          if (separation_squared <= start_radius_squared)  {
            // Issue fatal error message and exit:
            printf("\nERROR: The ending terminal for net '%s', located at (%6.3f, %6.3f) microns on layer '%s',\n",
                    user_inputs->net_name[other_path], user_inputs->end_X_um[other_path],
                    user_inputs->end_Y_um[other_path], user_inputs->routingLayerNames[other_path_endZ]);
            printf("       is too close to the starting terminals of diff-pair net '%s' on the same layer,\n",
                    user_inputs->net_name[path]);
            printf("       with terminal coordinates of (%6.3f, %6.3f) and (%6.3f, %6.3f).\n",
                    user_inputs->start_X_um[path],    user_inputs->start_Y_um[path],
                    user_inputs->start_X_um[partner], user_inputs->start_Y_um[partner]);
            printf("       Please modify the coordinates of the nets' terminals such that no terminals are too\n");
            printf("       close to the starting- or ending-terminals of differential-pair nets.\n\n");

            fatal_error = TRUE;
          }  // End of if-block for foreign terminal being close to diff-pair terminals' midpoint
        }  // End of if-block for other end-terminal being on same layer as diff-pair start-terminals


        // Comparison #3: Check if other path's starting terminal is on same layer
        // as diff pairs' ending terminals:
        if ((! staticEndTermInSwapZone) && (Z_midpoint_end == other_path_startZ))  {
          // Calculate distance between diff pairs' midpoint of ending terminals
          // and the starting terminal of other net:
          const int separation_squared =
                    (X_midpoint_end - other_path_startX) * (X_midpoint_end - other_path_startX)
                  + (Y_midpoint_end - other_path_startY) * (Y_midpoint_end - other_path_startY);

          // Compare separation to radius and issue fatal error message if separation <= radius:
          if (separation_squared <= end_radius_squared)  {
            // Issue fatal error message and exit:
            printf("\nERROR: The starting terminal for net '%s', located at (%6.3f, %6.3f) microns on layer '%s',\n",
                    user_inputs->net_name[other_path], user_inputs->start_X_um[other_path],
                    user_inputs->start_Y_um[other_path], user_inputs->routingLayerNames[other_path_startZ]);
            printf("       is too close to the ending terminals of diff-pair net '%s' on the same layer,\n",
                    user_inputs->net_name[path]);
            printf("       with terminal coordinates of (%6.3f, %6.3f) and (%6.3f, %6.3f).\n",
                    user_inputs->end_X_um[path],    user_inputs->end_Y_um[path],
                    user_inputs->end_X_um[partner], user_inputs->end_Y_um[partner]);

            printf("       Please modify the coordinates of the nets' terminals such that no terminals are too\n");
            printf("       close to the starting- or ending-terminals of differential-pair nets.\n\n");

            fatal_error = TRUE;
          }  // End of if-block for foreign terminal being close to diff-pair terminals' midpoint
        }  // End of if-block for other start-terminal being on same layer as diff-pair end-terminals


        // Comparison #4: Check if other path's ending terminal is on same layer
        // as diff pairs' ending terminals:
        if ((! staticEndTermInSwapZone) && (Z_midpoint_end == other_path_endZ))  {
          // Calculate distance between diff pairs' midpoint of ending terminals
          // and the ending terminal of other net:
          const int separation_squared =
                    (X_midpoint_end - other_path_endX) * (X_midpoint_end - other_path_endX)
                  + (Y_midpoint_end - other_path_endY) * (Y_midpoint_end - other_path_endY);

          // Compare separation to radius and issue fatal error message if separation <= radius:
          if (separation_squared <= end_radius_squared)  {
            // Issue fatal error message and exit:
            printf("\nERROR: The ending terminal for net '%s', located at (%6.3f, %6.3f) microns on layer '%s',\n",
                    user_inputs->net_name[other_path], user_inputs->end_X_um[other_path],
                    user_inputs->end_Y_um[other_path], user_inputs->routingLayerNames[other_path_endZ]);
            printf("       is too close to the ending terminals of diff-pair net '%s' on the same layer,\n",
                    user_inputs->net_name[path]);
            printf("       with terminal coordinates of (%6.3f, %6.3f) and (%6.3f, %6.3f).\n",
                   user_inputs->end_X_um[path],    user_inputs->end_Y_um[path],
                   user_inputs->end_X_um[partner], user_inputs->end_Y_um[partner]);

            printf("       Please modify the coordinates of the nets' terminals such that no terminals are too\n");
            printf("       close to the starting- or ending-terminals of differential-pair nets.\n\n");

            fatal_error = TRUE;
          }  // End of if-block for foreign terminal being close to diff-pair terminals' midpoint
        }  // End of if-block for other end-terminal being on same layer as diff-pair end-terminals

      }  // End of for-loop for index 'other_path'

    }  // End of if-block for 'isDiffPair == TRUE'

  }  // End of for-loop for index 'path'

  // printf("DEBUG: Finished processing user-defined nets. Will next process pseudo-nets...\n");

  //
  // Iterate through each pseudo-path to confirm that its terminals are not in, or close to,
  // a user-defined barrier (unless the terminal is in a swap-zone):
  //
  for (int pseudoPath = user_inputs->num_nets; pseudoPath < user_inputs->num_nets + user_inputs->num_pseudo_nets; pseudoPath++)  {

    // Get terminals of pseudo-net:
    int pseudo_start_X = mapInfo->start_cells[pseudoPath].X;
    int pseudo_start_Y = mapInfo->start_cells[pseudoPath].Y;
    int pseudo_start_Z = mapInfo->start_cells[pseudoPath].Z;
    int pseudo_end_X   = mapInfo->end_cells[pseudoPath].X;
    int pseudo_end_Y   = mapInfo->end_cells[pseudoPath].Y;
    int pseudo_end_Z   = mapInfo->end_cells[pseudoPath].Z;

    // printf("DEBUG: Pseudo-net %d has start-terminal (%d,%d,%d) and end-terminal (%d,%d,%d).\n", pseudoPath,
    //        pseudo_start_X, pseudo_start_Y, pseudo_start_Z, pseudo_end_X, pseudo_end_Y, pseudo_end_Z);

    //
    // Check wither pseudo-net's start-terminal is not in a swap-zone. If so, then check the terminal's
    // proximity to barriers:
    //
    if (! cellInfo[pseudo_start_X][pseudo_start_Y][pseudo_start_Z].swap_zone)  {

      // Check that the pseudo-net's starting terminal is not located in a user-defined barrier:
      if (cellInfo[pseudo_start_X][pseudo_start_Y][pseudo_start_Z].forbiddenTraceBarrier)  {
        int diff_pair_net_num_1 = user_inputs->pseudoNetToDiffPair_1[pseudoPath];
        int diff_pair_net_num_2 = user_inputs->pseudoNetToDiffPair_2[pseudoPath];
        printf("\n\nERROR: The mid-point of the start-terminals for the following differential pair are located within a user-defined barrier:\n");
        printf(    "          1) Net '%s' (net #%d) with start-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_1], diff_pair_net_num_1,
                    mapInfo->start_cells[diff_pair_net_num_1].X * user_inputs->cell_size_um,
                    mapInfo->start_cells[diff_pair_net_num_1].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->start_cells[diff_pair_net_num_1].Z]);
        printf(    "          2) Net '%s' (net #%d) with start-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_2], diff_pair_net_num_2,
                    mapInfo->start_cells[diff_pair_net_num_2].X * user_inputs->cell_size_um,
                    mapInfo->start_cells[diff_pair_net_num_2].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->start_cells[diff_pair_net_num_2].Z]);
        printf(    "       Modify the input file such that these terminals are outside of the barrier.\n\n");
        fatal_error = TRUE;
      }  // End of if-block for starting terminal being within a barrier

      // Check that the pseudo-net's starting terminal is not located in close
      // proximity to a user-defined barrier:
      if (get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, pseudo_start_X, pseudo_start_Y, pseudo_start_Z, pseudoPath, TRACE))  {
        int diff_pair_net_num_1 = user_inputs->pseudoNetToDiffPair_1[pseudoPath];
        int diff_pair_net_num_2 = user_inputs->pseudoNetToDiffPair_2[pseudoPath];
        printf("\n\nERROR: The mid-point of the start-terminals for the following differential pair are located too close to the map boundary or user-defined barrier:\n");
        printf(    "          1) Net '%s' (net #%d) with start-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_1], diff_pair_net_num_1,
                    mapInfo->start_cells[diff_pair_net_num_1].X * user_inputs->cell_size_um,
                    mapInfo->start_cells[diff_pair_net_num_1].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->start_cells[diff_pair_net_num_1].Z]);
        printf(    "          2) Net '%s' (net #%d) with start-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_2], diff_pair_net_num_2,
                    mapInfo->start_cells[diff_pair_net_num_2].X * user_inputs->cell_size_um,
                    mapInfo->start_cells[diff_pair_net_num_2].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->start_cells[diff_pair_net_num_2].Z]);
        printf(    "       Modify the input file such that these terminals are farther away from the barrier.\n\n");
        fatal_error = TRUE;
      }  // End of if-block for starting terminal being close to a barrier
    }  // End of if-block for (! swap_zone) for start-terminal


    //
    // Check wither pseudo-net's end-terminal is not in a swap-zone. If so, then check the terminal's
    // proximity to barriers:
    //
    if (! cellInfo[pseudo_end_X][pseudo_end_Y][pseudo_end_Z].swap_zone)  {

      // Check that the pseudo-net's ending terminal is not located in a user-defined barrier:
      if (cellInfo[pseudo_end_X][pseudo_end_Y][pseudo_end_Z].forbiddenTraceBarrier)  {
        int diff_pair_net_num_1 = user_inputs->pseudoNetToDiffPair_1[pseudoPath];
        int diff_pair_net_num_2 = user_inputs->pseudoNetToDiffPair_2[pseudoPath];
        printf("\n\nERROR: The mid-point of the end-terminals for the following differential pair are located within a user-defined barrier:\n");
        printf(    "          1) Net '%s' (net #%d) with end-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_1], diff_pair_net_num_1,
                    mapInfo->end_cells[diff_pair_net_num_1].X * user_inputs->cell_size_um,
                    mapInfo->end_cells[diff_pair_net_num_1].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->end_cells[diff_pair_net_num_1].Z]);
        printf(    "          2) Net '%s' (net #%d) with end-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_2], diff_pair_net_num_2,
                    mapInfo->end_cells[diff_pair_net_num_2].X * user_inputs->cell_size_um,
                    mapInfo->end_cells[diff_pair_net_num_2].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->end_cells[diff_pair_net_num_2].Z]);
        printf(    "       Modify the input file such that these terminals are outside of the barrier.\n\n");
        fatal_error = TRUE;
      }  // End of if-block for ending terminal being within a barrier

      // Check that the pseudo-net's ending terminal is not located in close
      // proximity to a user-defined barrier:
      if (get_unwalkable_barrier_proximity_by_path(cellInfo, user_inputs, pseudo_end_X, pseudo_end_Y, pseudo_end_Z, pseudoPath, TRACE))  {
        int diff_pair_net_num_1 = user_inputs->pseudoNetToDiffPair_1[pseudoPath];
        int diff_pair_net_num_2 = user_inputs->pseudoNetToDiffPair_2[pseudoPath];
        printf("\n\nERROR: The mid-point of the end-terminals for the following differential pair are located too close to a map boundary or user-defined barrier:\n");
        printf(    "          1) Net '%s' (net #%d) with end-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_1], diff_pair_net_num_1,
                    mapInfo->end_cells[diff_pair_net_num_1].X * user_inputs->cell_size_um,
                    mapInfo->end_cells[diff_pair_net_num_1].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->end_cells[diff_pair_net_num_1].Z]);
        printf(    "          2) Net '%s' (net #%d) with end-terminal at (%6.3f, %6.3f) microns on layer %s\n",
                    user_inputs->net_name[diff_pair_net_num_2], diff_pair_net_num_2,
                    mapInfo->end_cells[diff_pair_net_num_2].X * user_inputs->cell_size_um,
                    mapInfo->end_cells[diff_pair_net_num_2].Y * user_inputs->cell_size_um,
                    user_inputs->routingLayerNames[mapInfo->end_cells[diff_pair_net_num_2].Z]);
        printf(    "       Modify the input file such that these terminals are farther away from the barrier.\n\n");
        fatal_error = TRUE;
      }  // End of if-block for starting terminal being close to a barrier
    }  // End of if-block for (! swap_zone) for end-terminal

  }  // End of for-loop for index 'pseudoPath'

  // If the 'fatal_error' Boolean flag is set, then exit the program:
  if (fatal_error)  {
    printf("\nERROR: Program is exiting due to the above fatal errors.\n\n");
    exit(1);
  }


  // printf("DEBUG: Exiting function 'verifyDiffPairTerminals'...\n");

}  // End of function 'verifyDiffPairTerminals'


//-----------------------------------------------------------------------------
// Name: verifyAllTerminals
// Desc: For each net that is not in a pin-swap zone, verify that there
//       are no other terminals within a distance of a trace-width
//       plus a trace-to-trace spacing (a 'trace pitch').
//-----------------------------------------------------------------------------
void verifyAllTerminals(const InputValues_t *user_inputs, CellInfo_t ***cellInfo, MapInfo_t *mapInfo)  {

  // printf("DEBUG: Entered function 'verifyAllTerminals'...\n");

  int fatal_error = FALSE; // Flag is set to TRUE if error is found.

  // Iterate through each user-defined net (excluding pseudo-nets):
  for (int path = 0; path < user_inputs->num_nets; path++)  {



    // Get the (x,y) coordinates in micron units of the path's
    // start- and end-terminals, as originally defined by the
    // user, disregarding any terminal-swapping done by Acorn:
    const float path_start_X_um  = user_inputs->start_X_um[path];
    const float path_start_Y_um  = user_inputs->start_Y_um[path];
    const float path_end_X_um    = user_inputs->end_X_um[path];
    const float path_end_Y_um    = user_inputs->end_Y_um[path];

    // Get the (x,y,z) coordinates in cell units and layer number
    // of the path's start- and end-terminals, as originally defined
    // by the user, disregarding any terminal-swapping done by Acorn:
    int path_start_X_cells, path_start_Y_cells, path_start_layer;
    int path_end_X_cells,   path_end_Y_cells,   path_end_layer;
    if (! mapInfo->start_end_terms_swapped[path])  {
      path_start_X_cells = mapInfo->start_cells[path].X;
      path_start_Y_cells = mapInfo->start_cells[path].Y;
      path_start_layer   = mapInfo->start_cells[path].Z;

      path_end_X_cells   = mapInfo->end_cells[path].X;
      path_end_Y_cells   = mapInfo->end_cells[path].Y;
      path_end_layer     = mapInfo->end_cells[path].Z;
    }
    else  {
      path_start_X_cells = mapInfo->end_cells[path].X;
      path_start_Y_cells = mapInfo->end_cells[path].Y;
      path_start_layer = mapInfo->end_cells[path].Z;

      path_end_X_cells   = mapInfo->start_cells[path].X;
      path_end_Y_cells   = mapInfo->start_cells[path].Y;
      path_end_layer     = mapInfo->start_cells[path].Z;
    }  // End of if/else-block

    // Get the design-rule set numbers at the locations of the
    // user-defined start- and end-coordinates:
    const int path_start_DR_set = cellInfo[path_start_X_cells][path_start_Y_cells][path_start_layer].designRuleSet;
    const int path_end_DR_set   = cellInfo[path_end_X_cells][path_end_Y_cells][path_end_layer].designRuleSet;

    // Get the design-rule subsets associated with the current path number:
    const int path_start_DR_subset = user_inputs->designRuleSubsetMap[path][path_start_DR_set];
    const int path_end_DR_subset   = user_inputs->designRuleSubsetMap[path][path_end_DR_set];

    // Get the trace width and trace-to-trace spacing distances at the start- and
    // end-terminals for the current path number:
    const float path_start_trace_width_um   = user_inputs->designRules[path_start_DR_set][path_start_DR_subset].lineWidthMicrons;
    const float path_start_trace_spacing_um = user_inputs->designRules[path_start_DR_set][path_start_DR_subset].lineSpacingMicrons;
    const float path_end_trace_width_um     = user_inputs->designRules[path_end_DR_set][path_end_DR_subset].lineWidthMicrons;
    const float path_end_trace_spacing_um   = user_inputs->designRules[path_end_DR_set][path_end_DR_subset].lineSpacingMicrons;

    // Calculate trace-pitch values at the start- and end-terminals of the current path:
    const float path_start_trace_pitch_um = path_start_trace_width_um + path_start_trace_spacing_um;
    const float path_end_trace_pitch_um   = path_end_trace_width_um   + path_end_trace_spacing_um;

    // Get the swap-zone status/number for the current path's user-defined
    // start- and end-terminals:
    const int path_start_swapZone = cellInfo[path_start_X_cells][path_start_Y_cells][path_start_layer].swap_zone;
    const int path_end_swapZone   = cellInfo[path_end_X_cells][path_end_Y_cells][path_end_layer].swap_zone;

    // Iterate over all other non-diff-pair paths, from the current path number
    // up to the maximum path number:
    for (int other_path = path + 1; other_path < user_inputs->num_nets; other_path++)  {

      // Get the (x,y) coordinates in micron units of the other
      // path's start- and end-terminals, as originally defined by
      // the user, disregarding any terminal-swapping done by Acorn:
      const float other_path_start_X_um  = user_inputs->start_X_um[other_path];
      const float other_path_start_Y_um  = user_inputs->start_Y_um[other_path];
      const float other_path_end_X_um    = user_inputs->end_X_um[other_path];
      const float other_path_end_Y_um    = user_inputs->end_Y_um[other_path];

      // Get the (x,y,z) coordinates in cell units and layer number
      // of the other path's start- and end-terminals, as originally defined
      // by the user, disregarding any terminal-swapping done by Acorn:
      int other_path_start_X_cells, other_path_start_Y_cells, other_path_start_layer;
      int other_path_end_X_cells,   other_path_end_Y_cells,   other_path_end_layer;
      if (! mapInfo->start_end_terms_swapped[other_path])  {
        other_path_start_X_cells = mapInfo->start_cells[other_path].X;
        other_path_start_Y_cells = mapInfo->start_cells[other_path].Y;
        other_path_start_layer   = mapInfo->start_cells[other_path].Z;

        other_path_end_X_cells   = mapInfo->end_cells[other_path].X;
        other_path_end_Y_cells   = mapInfo->end_cells[other_path].Y;
        other_path_end_layer     = mapInfo->end_cells[other_path].Z;
      }
      else  {
        other_path_start_X_cells = mapInfo->end_cells[other_path].X;
        other_path_start_Y_cells = mapInfo->end_cells[other_path].Y;
        other_path_start_layer = mapInfo->end_cells[other_path].Z;

        other_path_end_X_cells   = mapInfo->start_cells[other_path].X;
        other_path_end_Y_cells   = mapInfo->start_cells[other_path].Y;
        other_path_end_layer     = mapInfo->start_cells[other_path].Z;
      }  // End of if/else-block

      // Get the design-rule set numbers at the locations of the
      // user-defined start- and end-coordinates:
      const int other_path_start_DR_set = cellInfo[other_path_start_X_cells][other_path_start_Y_cells][other_path_start_layer].designRuleSet;
      const int other_path_end_DR_set   = cellInfo[other_path_end_X_cells][other_path_end_Y_cells][other_path_end_layer].designRuleSet;

      // Get the design-rule subsets associated with the other path number:
      const int other_path_start_DR_subset = user_inputs->designRuleSubsetMap[other_path][other_path_start_DR_set];
      const int other_path_end_DR_subset   = user_inputs->designRuleSubsetMap[other_path][other_path_end_DR_set];

      // Get the trace width and trace-to-trace spacing distances at the start- and
      // end-terminals for the current path number:
      const float other_path_start_trace_width_um   = user_inputs->designRules[other_path_start_DR_set][other_path_start_DR_subset].lineWidthMicrons;
      const float other_path_start_trace_spacing_um = user_inputs->designRules[other_path_start_DR_set][other_path_start_DR_subset].lineSpacingMicrons;
      const float other_path_end_trace_width_um     = user_inputs->designRules[other_path_end_DR_set][other_path_end_DR_subset].lineWidthMicrons;
      const float other_path_end_trace_spacing_um   = user_inputs->designRules[other_path_end_DR_set][other_path_end_DR_subset].lineSpacingMicrons;

      // Calculate trace-pitch values at the start- and end-terminals of the current path:
      const float other_path_start_trace_pitch_um = other_path_start_trace_width_um + other_path_start_trace_spacing_um;
      const float other_path_end_trace_pitch_um   = other_path_end_trace_width_um   + other_path_end_trace_spacing_um;

      // Get the swap-zone status/number for the other path's user-defined
      // start- and end-terminals:
      const int other_path_start_swapZone = cellInfo[other_path_start_X_cells][other_path_start_Y_cells][other_path_start_layer].swap_zone;
      const int other_path_end_swapZone   = cellInfo[other_path_end_X_cells][other_path_end_Y_cells][other_path_end_layer].swap_zone;

      //
      // Ensure that current net's terminals are not within a trace-pitch of the 'other' net's
      // terminals. This requires four checks:
      //   (1) current net's start-terminal compared to other net's start-terminal,
      //   (2) current net's start-terminal compared to other net's end-terminal,
      //   (3) current net's end-terminal compared to other net's start-terminal, and
      //   (4) current net's end-terminal compared to other net's end-terminal.
      //
      // Comparison #1: current net's start-terminal compared to other net's start-terminal
      //
      // First, confirm that neither terminal is in a swap-zone and that both
      // terminals are on the same routing layer:
      if ((! path_start_swapZone) && (! other_path_start_swapZone) && (path_start_layer == other_path_start_layer))  {

        // Calculate the actual separation of the terminals:
        const float actual_separation_um = calc_2D_Pythagorean_distance_floats(path_start_X_um, path_start_Y_um, other_path_start_X_um, other_path_start_Y_um);

        // Next, calculate the minimum allowed distance between terminals:
        const float min_allowed_separation_um = max(path_start_trace_pitch_um, other_path_start_trace_pitch_um);

        // Compare the actual separation to the allowed separation. If it's illegally small, then
        // issue a fatal error message and set the 'fatal_error' Boolean flag:
        if (actual_separation_um < min_allowed_separation_um)  {

          // We got here, so the terminals violate the minimum trace-pitch design rules. Issue a fatal
          // error message:
          printf("\nERROR: The following two terminal are located too close together, based on design rules:\n");
          printf(  "        (1) Start-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[path], user_inputs->layer_names[path_start_layer],
                 path_start_X_um, path_start_Y_um);
          printf(  "        (2) Start-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[other_path], user_inputs->layer_names[other_path_start_layer],
                 other_path_start_X_um, other_path_start_Y_um);
          printf(  "       These terminals are separated by %.2f microns, but the minimum trace-pitch for these\n",
                 actual_separation_um);
          printf(  "       nets is larger: %.2f microns, based on user-supplied design rules. Please modify\n",
                 min_allowed_separation_um);
          printf(  "       the input file such that these terminals are farther away from each other.\n\n");

          // Set the 'fatal_error' flag so the function will cause the program to die:
          fatal_error = TRUE;
        }  // End of if-block for illegally small separation between terminals

      }  // End of if-block to confirm that neither terminal is in a swap-zone

      //
      // Comparison #2: current net's start-terminal compared to other net's end-terminal
      //
      // First, confirm that neither terminal is in a swap-zone and that both
      // terminals are on the same routing layer:
      if ((! path_start_swapZone) && (! other_path_end_swapZone) && (path_start_layer == other_path_end_layer))  {

        // Calculate the actual separation of the terminals:
        const float actual_separation_um = calc_2D_Pythagorean_distance_floats(path_start_X_um, path_start_Y_um, other_path_end_X_um, other_path_end_Y_um);

        // Next, calculate the minimum allowed distance between terminals:
        const float min_allowed_separation_um = max(path_start_trace_pitch_um, other_path_end_trace_pitch_um);

        // Compare the actual separation to the allowed separation. If it's illegally small, then
        // issue a fatal error message and set the 'fatal_error' Boolean flag:
        if (actual_separation_um < min_allowed_separation_um)  {

          // We got here, so the terminals violate the minimum trace-pitch design rules. Issue a fatal
          // error message:
          printf("\nERROR: The following two terminal are located too close together, based on design rules:\n");
          printf(  "        (1) Start-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[path], user_inputs->layer_names[path_start_layer],
                 path_start_X_um, path_start_Y_um);
          printf(  "        (2) End-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[other_path], user_inputs->layer_names[other_path_end_layer],
                 other_path_end_X_um, other_path_end_Y_um);
          printf(  "       These terminals are separated by %.2f microns, but the minimum trace-pitch for these\n",
                 actual_separation_um);
          printf(  "       nets is larger: %.2f microns, based on user-supplied design rules. Please modify\n",
                 min_allowed_separation_um);
          printf(  "       the input file such that these terminals are farther away from each other.\n\n");

          // Set the 'fatal_error' flag so the function will cause the program to die:
          fatal_error = TRUE;
        }  // End of if-block for illegally small separation between terminals

      }  // End of if-block to confirm that neither terminal is in a swap-zone

      //
      // Comparison #3: current net's end-terminal compared to other net's start-terminal
      //
      // First, confirm that neither terminal is in a swap-zone and that both
      // terminals are on the same routing layer:
      if ((! path_end_swapZone) && (! other_path_start_swapZone) && (path_end_layer == other_path_start_layer))  {

        // Calculate the actual separation of the terminals:
        const float actual_separation_um = calc_2D_Pythagorean_distance_floats(path_end_X_um, path_end_Y_um, other_path_start_X_um, other_path_start_Y_um);

        // Next, calculate the minimum allowed distance between terminals:
        const float min_allowed_separation_um = max(path_end_trace_pitch_um, other_path_start_trace_pitch_um);

        // Compare the actual separation to the allowed separation. If it's illegally small, then
        // issue a fatal error message and set the 'fatal_error' Boolean flag:
        if (actual_separation_um < min_allowed_separation_um)  {

          // We got here, so the terminals violate the minimum trace-pitch design rules. Issue a fatal
          // error message:
          printf("\nERROR: The following two terminal are located too close together, based on design rules:\n");
          printf(  "        (1) End-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[path], user_inputs->layer_names[path_end_layer],
                 path_end_X_um, path_end_Y_um);
          printf(  "        (2) Start-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[other_path], user_inputs->layer_names[other_path_start_layer],
                 other_path_start_X_um, other_path_start_Y_um);
          printf(  "       These terminals are separated by %.2f microns, but the minimum trace-pitch for these\n",
                 actual_separation_um);
          printf(  "       nets is larger: %.2f microns, based on user-supplied design rules. Please modify\n",
                 min_allowed_separation_um);
          printf(  "       the input file such that these terminals are farther away from each other.\n\n");

          // Set the 'fatal_error' flag so the function will cause the program to die:
          fatal_error = TRUE;
        }  // End of if-block for illegally small separation between terminals

      }  // End of if-block to confirm that neither terminal is in a swap-zone

      //
      // Comparison #4: current net's end-terminal compared to other net's end-terminal
      //
      // First, confirm that neither terminal is in a swap-zone and that both
      // terminals are on the same routing layer:
      if ((! path_end_swapZone) && (! other_path_end_swapZone) && (path_end_layer == other_path_end_layer))  {

        // Calculate the actual separation of the terminals:
        const float actual_separation_um = calc_2D_Pythagorean_distance_floats(path_end_X_um, path_end_Y_um, other_path_end_X_um, other_path_end_Y_um);

        // Next, calculate the minimum allowed distance between terminals:
        const float min_allowed_separation_um = max(path_end_trace_pitch_um, other_path_end_trace_pitch_um);

        // Compare the actual separation to the allowed separation. If it's illegally small, then
        // issue a fatal error message and set the 'fatal_error' Boolean flag:
        if (actual_separation_um < min_allowed_separation_um)  {

          // We got here, so the terminals violate the minimum trace-pitch design rules. Issue a fatal
          // error message:
          printf("\nERROR: The following two terminal are located too close together, based on design rules:\n");
          printf(  "        (1) End-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[path], user_inputs->layer_names[path_end_layer],
                 path_end_X_um, path_end_Y_um);
          printf(  "        (2) End-terminal of path '%s' on layer %s at (%.2f, %.2f) microns\n",
                 user_inputs->net_name[other_path], user_inputs->layer_names[other_path_end_layer],
                 other_path_end_X_um, other_path_end_Y_um);
          printf(  "       These terminals are separated by %.2f microns, but the minimum trace-pitch for these\n",
                 actual_separation_um);
          printf(  "       nets is larger: %.2f microns, based on user-supplied design rules. Please modify\n",
                 min_allowed_separation_um);
          printf(  "       the input file such that these terminals are farther away from each other.\n\n");

          // Set the 'fatal_error' flag so the function will cause the program to die:
          fatal_error = TRUE;
        }  // End of if-block for illegally small separation between terminals
      }  // End of if-block to confirm that neither terminal is in a swap-zone

      // If the 'fatal_error' Boolean flag is set, then exit the program:
      if (fatal_error)  {
        printf("\nERROR: Program is exiting due to the above fatal errors.\n\n");
        exit(1);
      }

    }  // End of for-loop for index 'other_path'
  }  // End of for-loop for index 'path'
}  // End of function 'verifyAllTerminals'


//-----------------------------------------------------------------------------
// Name: parse_input_file
// Desc: Reads input file 'filename' and parses data from file. Data is
//       written into a structure of type 'InputValues_t', with some data
//       also written to a structure of type 'MapInfo_t'.
//-----------------------------------------------------------------------------
void parse_input_file(char *input_filename, InputValues_t *user_inputs, MapInfo_t *mapInfo)  {

  char line[1024], temp_line[1024];  // For reading lines from input file
  regex_t regex, regex_diff_pair, regex_single_ended, regex_special_net, regex_diff_pair_swappable_terms; // For processing regular expression (see regex.h)
  const int n_matches = 12; // Maximum number of regex matches allowed
  regmatch_t regex_match[n_matches];  // For processing regular expressions
  char netlist_flag = FALSE;      // TRUE when parsing netlist lines from input file
  char design_rule_flag = FALSE;  // TRUE when parsing design rules from input file
  char exception_flag = FALSE;   // TRUE when parsing an exception within a design-rule
  int net_number = 0;    // Index for nets, starting at zero.
  int num_named_layers = 0; // Number of routing and via layer names in input file
  int num_block_instructions = 0; // Number of BLOCK/UNBLOCK instructions in input file 
  int num_DR_zone_instructions = 0; // Number of DR_zone instructions in input file 
  int num_trace_cost_zone_instructions = 0; // Number of trace_cost_zone instructions in input file 
  int num_via_cost_zone_instructions = 0; // Number of via_cost_zone instructions in input file 
  int num_swap_instructions = 0; // Number of PIN_SWAP/NO_PIN_SWAP instructions in input file 
  int design_rule_set   = 0; // Number of design-rule sets
  int design_rule_subset = 0; // Number of design-rule subsets within each design-rule set
  int multiplier_index; // Temporary variable for storing cost-multiplier index
  const char whitespace[] = " \t";  // Set of whitespace characters used for parsing
  size_t len;  // Temporary variable to hold length of strings.

  // Copy the number of pseudo-nets from the user_inputs variable to the mapInfo variable. Having
  // this information in both variables is redundant, but simplifies coding for other functions:
  mapInfo->numPseudoPaths = user_inputs->num_pseudo_nets;

  // Set default values for selected parameters, just in case the user does not provide them.
  // The default values are defined in the global_defs.h file.
  user_inputs->maxIterations            = defaultMaxIterations;
  mapInfo->max_iterations               = defaultMaxIterations;
  user_inputs->userDRCfreeThreshold     = defaultDRCfreeThreshold;
  user_inputs->baseVertCostCells        = defaultVertCost;
  user_inputs->baseVertCostMicrons      = defaultVertCost;
  user_inputs->baseVertCost             = defaultVertCost;
  user_inputs->runsPerPngMap            = defaultRunsPerPngMap;
  user_inputs->pinSwapCellCost          = defaultCellCost;
  user_inputs->baseCellCost             = (long)(defaultCellCost * pow(2.0, NON_PIN_SWAP_EXPONENT));

  user_inputs->pinSwapDiagCost          = defaultDiagCost;
  user_inputs->baseDiagCost             = (long)(sqrt(2) * defaultCellCost * pow(2.0, NON_PIN_SWAP_EXPONENT));

  user_inputs->pinSwapKnightCost        = defaultKnightCost;
  user_inputs->baseKnightCost           = (long)(sqrt(5) * defaultCellCost * pow(2.0, NON_PIN_SWAP_EXPONENT));

  user_inputs->pinSwapVertCost          = defaultCellCost;

  user_inputs->preEvaporationIterations = defaultPreEvaporationIterations;


  // Initialize the number of trace/via cost-multipliers to zero.
  // Initialize cost-multiplier values to 1 for traces and vias, and
  // initialize flag to zero for the use of each multiplier:
  user_inputs->numTraceMultipliersUsed = 0;
  user_inputs->numViaMultipliersUsed   = 0;
  for (int i = 0; i < maxTraceCostMultipliers; i++)  {
    user_inputs->traceCostMultiplier[i] = 1;
    user_inputs->traceCostMultiplierUsed[i] = FALSE;
  }
  for (int i = 0; i < maxViaCostMultipliers; i++)  {
    user_inputs->viaCostMultiplier[i] = 1;
    user_inputs->viaCostMultiplierUsed[i] = FALSE;
  }

  FILE *fp;
  fp = fopen(input_filename, "r");
  if (! fp) {
    printf ("\nERROR: Input file \"%s\" is not available for reading.\n\n",input_filename);
    exit(1);
  }
  // printf("DEBUG: Finished opening input file for reading.\n");

  // Initialize Boolean flag 'base_vert_cost_defined' to FALSE. It will be toggled to TRUE
  // if the user defines the vertical cost of vias:
  char base_vert_cost_defined = FALSE;

  //
  // Read each line in the input file:
  //
  while ((fgets(line, 1024, fp)) != NULL)  {
    // Change CR, LF, CR-LF, or LF-CR to '\0':
    line[strcspn(line, "\r\n")] = 0;  

    // Filter out any lines that begin with a '#' character:
    compile_regex("^#.*$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0) {
      regfree(&regex);
      // printf("Skipping: <<%s>>\n", line);
      continue;  // Skip lines that begin with '#'
    }
    else  {
      regfree(&regex);
    }

    // Filter out any lines that begin with '//' characters:
    compile_regex("^[[:blank:]]*//", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      // printf("Skipping: <<%s>>\n", line);
      continue;  // Skip lines that begin with '//'
    }
    else  {
      regfree(&regex);
    }

    // Filter out blank lines:
    compile_regex("^[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      // printf("Skipping: <<%s>>\n", line);
      continue;  // Skip blank lines
    }
    else  {
      regfree(&regex);
    }

    // Following debug print-statement prints out the line that was read from the input file:
    // printf("\n|%s|\n", line);

    // Discard comments denoted by '//' out to the end of the line:
    compile_regex("^(.*)(//.*)$", &regex);
    if (regexec(&regex, line, 3, regex_match, 0) == 0)  {
      regfree(&regex);

      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_string
      strncpy(temp_line, line + regex_match[1].rm_so, (int)(regex_match[1].rm_eo - regex_match[1].rm_so));

      memset(line, '\0', sizeof(line));  // Reset 'line' string
      strcpy(line, temp_line);  // Copy resulting text to 'line' string
    }
    else  {
      regfree(&regex);
    }

    // printf("line is <<%s>> before discarding leading/trailing white-space.\n", line);

    // Discard leading and trailing white-space:
    compile_regex("^[[:blank:]]*([^[:blank:]].*[^[:blank:]])[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 2, regex_match, 0) == 0)  {
      regfree(&regex);
      // printf("DEBUG: regex_match[0].rm_so=%d, regex_match[0].rm_eo=%d\n", regex_match[0].rm_so, regex_match[0].rm_eo);
      // printf("DEBUG: regex_match[1].rm_so=%d, regex_match[1].rm_eo=%d\n", regex_match[1].rm_so, regex_match[1].rm_eo);
      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
      strncpy(temp_line, line + regex_match[1].rm_so, (int)(regex_match[1].rm_eo - regex_match[1].rm_so));

      memset(line, '\0', sizeof(line)); // Reset 'line' string
      strcpy(line, temp_line);  // Copy resulting text to 'line' string
    }
    else  {
      regfree(&regex);
    }

    // printf("DEBUG: line is <<%s>> after discarding leading/trailing white-space.\n", line);

    // Check for key words 'start_nets' and 'end_nets'
    compile_regex("^start_nets$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      netlist_flag = TRUE;
      // printf("DEBUG: netlist_flag is TRUE\n");
      net_number = 0;
      continue;
    }
    else  {
      regfree(&regex);
    }

    compile_regex("^end_nets$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      netlist_flag = FALSE;
      // printf("DEBUG: netlist_flag is FALSE\n");

      // We've gotten to the end of the list of nets, so capture the number of net:
      user_inputs->num_nets = net_number;
      mapInfo->numPaths     = net_number;
      continue;
    }
    else  {
      regfree(&regex);
    }

    //
    // Check for list of nets, which consists of either 7, 8, 9, or 10 whitespace-delimited tokens,
    // depending on whether they single-ended signals (7 tokens), net-specific design rules
    // (8 tokens), differential pairs with non-swappable P/N terminals (9 tokens), or
    // differential pairs with P/N-swappable terminals (10 tokens):
    //
    if (netlist_flag)  {

      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)$",
          &regex_single_ended);
      // printf("DEBUG: Successfully compiled regex for 7 space-delimited tokens.\n");

      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)$",
          &regex_special_net);
      // printf("DEBUG: Successfully compiled regex for 8 space-delimited tokens.\n");

      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)$",
          &regex_diff_pair);
      // printf("DEBUG: Successfully compiled regex for 9 space-delimited tokens.\n");

      compile_regex("^([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+pn_swappable$",
          &regex_diff_pair_swappable_terms);
      // printf("DEBUG: Successfully compiled regex for 10 space-delimited tokens.\n");

      //
      // Check for netlist line with 7 tokens, denoting a standard net:
      //
      if (regexec(&regex_single_ended, line, 8, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 7-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // 1st token is name of net:
        len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
        strncpy(user_inputs->net_name[net_number], line + regex_match[1].rm_so, (int)len);
        user_inputs->net_name[net_number][len] = '\0'; // Terminate string with NULL character

        // 2nd token is start-layer:
        len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
        strncpy(user_inputs->start_layer[net_number], line + regex_match[2].rm_so, (int)len);
        user_inputs->start_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 3rd token is starting X-position:
        len = (size_t)(regex_match[3].rm_eo - regex_match[3].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[3].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_X_um[net_number] = strtof(temp_line, NULL);

        // 4th token is starting Y-position:
        len = (size_t)(regex_match[4].rm_eo - regex_match[4].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[4].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_Y_um[net_number] = strtof(temp_line, NULL);

        // 5th token is end-layer:
        len = (size_t)(regex_match[5].rm_eo - regex_match[5].rm_so);
        strncpy(user_inputs->end_layer[net_number], line + regex_match[5].rm_so, (int)len);
        user_inputs->end_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 6th token is ending X-position:
        len = (size_t)(regex_match[6].rm_eo - regex_match[6].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[6].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_X_um[net_number] = strtof(temp_line, NULL);

        // 7th token is ending Y-position:
        len = (size_t)(regex_match[7].rm_eo - regex_match[7].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[7].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_Y_um[net_number] = strtof(temp_line, NULL);

        // printf("DEBUG: Found net '%s' (# %d): %s (%6.2f,%6.2f) to %s (%6.2f,%6.2f)\n",
        //        user_inputs->net_name[net_number], net_number,
        //        user_inputs->start_layer[net_number], user_inputs->start_X_um[net_number],
        //        user_inputs->start_Y_um[net_number], user_inputs->end_layer[net_number],
        //        user_inputs->end_X_um[net_number], user_inputs->end_Y_um[net_number]);

        // Specify that this net is NOT part of a differential pair of nets:
        user_inputs->isDiffPair[net_number] = FALSE;

        // Specify that this net does NOT follow a special design rule:
        user_inputs->usesSpecialRule[net_number] = FALSE;

        // Increment the counter for the number of nets. Note that the first net
        // will be index #0.
        net_number++;  

        continue; // Skip to next line in input file

      }  // End of if-block for matching a singled-ended netlist line

      //
      // Check for netlist line with 8 tokens, denoting a net with net-specific
      // design rules:
      //
      else if (regexec(&regex_special_net, line, 9, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 8-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // 1st token is name of net:
        len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
        strncpy(user_inputs->net_name[net_number], line + regex_match[1].rm_so, (int)len);
        user_inputs->net_name[net_number][len] = '\0'; // Terminate string with NULL character

        // 2nd token is start-layer:
        len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
        strncpy(user_inputs->start_layer[net_number], line + regex_match[2].rm_so, (int)len);
        user_inputs->start_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 3rd token is starting X-position:
        len = (size_t)(regex_match[3].rm_eo - regex_match[3].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[3].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_X_um[net_number] = strtof(temp_line, NULL);

        // 4th token is starting Y-position:
        len = (size_t)(regex_match[4].rm_eo - regex_match[4].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[4].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_Y_um[net_number] = strtof(temp_line, NULL);

        // 5th token is end-layer:
        len = (size_t)(regex_match[5].rm_eo - regex_match[5].rm_so);
        strncpy(user_inputs->end_layer[net_number], line + regex_match[5].rm_so, (int)len);
        user_inputs->end_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 6th token is ending X-position:
        len = (size_t)(regex_match[6].rm_eo - regex_match[6].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[6].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_X_um[net_number] = strtof(temp_line, NULL);

        // 7th token is ending Y-position:
        len = (size_t)(regex_match[7].rm_eo - regex_match[7].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[7].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_Y_um[net_number] = strtof(temp_line, NULL);

        // 8th token is name of net-specific design rule:
        len = (size_t)(regex_match[8].rm_eo - regex_match[8].rm_so);
        strncpy(user_inputs->netSpecificRuleName[net_number], line + regex_match[8].rm_so, (int)len);
        user_inputs->netSpecificRuleName[net_number][len] = '\0'; // Terminate string with NULL character

        // printf("DEBUG: Found net '%s' (# %d) with net-specific design rules: %s (%6.2f,%6.2f) to %s (%6.2f,%6.2f) with design rule '%s'\n",
        //         user_inputs->net_name[net_number], net_number,
        //         user_inputs->start_layer[net_number], user_inputs->start_X_um[net_number],
        //         user_inputs->start_Y_um[net_number], user_inputs->end_layer[net_number],
        //         user_inputs->end_X_um[net_number], user_inputs->end_Y_um[net_number],
        //         user_inputs->netSpecificRuleName[net_number]);

        // Specify that this net follows a special design rule:
        user_inputs->usesSpecialRule[net_number] = TRUE;

        // Specify that this net is NOT part of a differential pair of nets:
        user_inputs->isDiffPair[net_number] = FALSE;

        // Increment the counter for the number of nets. Note that the first net
        // will be index #0.
        net_number++;

        continue; // Skip to next line in input file

      }  // End of else-block for matching a special net with 8 tokens

      //
      // Check for netlist line with 9 tokens, denoting a net that's part of
      // a differential pair (but does not have P/N-swappable terminals):
      //
      else if (regexec(&regex_diff_pair, line, 10, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 9-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // 1st token is name of net:
        len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
        strncpy(user_inputs->net_name[net_number], line + regex_match[1].rm_so, (int)len);
        user_inputs->net_name[net_number][len] = '\0'; // Terminate string with NULL character

        // 2nd token is start-layer:
        len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
        strncpy(user_inputs->start_layer[net_number], line + regex_match[2].rm_so, (int)len);
        user_inputs->start_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 3rd token is starting X-position:
        len = (size_t)(regex_match[3].rm_eo - regex_match[3].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[3].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_X_um[net_number] = strtof(temp_line, NULL);

        // 4th token is starting Y-position:
        len = (size_t)(regex_match[4].rm_eo - regex_match[4].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[4].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_Y_um[net_number] = strtof(temp_line, NULL);

        // 5th token is end-layer:
        len = (size_t)(regex_match[5].rm_eo - regex_match[5].rm_so);
        strncpy(user_inputs->end_layer[net_number], line + regex_match[5].rm_so, (int)len);
        user_inputs->end_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 6th token is ending X-position:
        len = (size_t)(regex_match[6].rm_eo - regex_match[6].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[6].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_X_um[net_number] = strtof(temp_line, NULL);

        // 7th token is ending Y-position:
        len = (size_t)(regex_match[7].rm_eo - regex_match[7].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[7].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_Y_um[net_number] = strtof(temp_line, NULL);

        // 8th token is name of diff-pair design rule:
        len = (size_t)(regex_match[8].rm_eo - regex_match[8].rm_so);
        strncpy(user_inputs->netSpecificRuleName[net_number], line + regex_match[8].rm_so, (int)len);
        user_inputs->netSpecificRuleName[net_number][len] = '\0'; // Terminate string with NULL character

        // 9th token is name of diff-pair partner net:
        len = (size_t)(regex_match[9].rm_eo - regex_match[9].rm_so);
        strncpy(user_inputs->diffPairPartnerName[net_number], line + regex_match[9].rm_so, (int)len);
        user_inputs->diffPairPartnerName[net_number][len] = '\0'; // Terminate string with NULL character

        // printf("DEBUG: Found diff pair net '%s' (# %d): %s (%6.2f,%6.2f) to %s (%6.2f,%6.2f) with partner '%s' and diff-pair rule '%s'\n",
        //         user_inputs->net_name[net_number], net_number,
        //         user_inputs->start_layer[net_number], user_inputs->start_X_um[net_number],
        //         user_inputs->start_Y_um[net_number], user_inputs->end_layer[net_number],
        //         user_inputs->end_X_um[net_number], user_inputs->end_Y_um[net_number],
        //         user_inputs->diffPairPartnerName[net_number], user_inputs->netSpecificRuleName[net_number]);

        // Specify that this net is part of a differential pair of nets:
        user_inputs->isDiffPair[net_number] = TRUE;

        // Specify that this net follows a special design rule:
        user_inputs->usesSpecialRule[net_number] = TRUE;

        // Increment the counter for the number of nets. Note that the first net
        // will be index #0.
        net_number++;

        continue; // Skip to next line in input file

      }  // End of else-block for matching a special net with 9 tokens

      //
      // Check for netlist line with 10 tokens, denoting a net that's part of
      // a differential pair with P/N-swappable terminals:
      //
      else if (regexec(&regex_diff_pair_swappable_terms, line, 11, regex_match, 0) == 0)  {
        // printf("DEBUG: Found 10-token line in netlist part of file.\n");

        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        // 1st token is name of net:
        len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
        strncpy(user_inputs->net_name[net_number], line + regex_match[1].rm_so, (int)len);
        user_inputs->net_name[net_number][len] = '\0'; // Terminate string with NULL character

        // 2nd token is start-layer:
        len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
        strncpy(user_inputs->start_layer[net_number], line + regex_match[2].rm_so, (int)len);
        user_inputs->start_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 3rd token is starting X-position:
        len = (size_t)(regex_match[3].rm_eo - regex_match[3].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[3].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_X_um[net_number] = strtof(temp_line, NULL);

        // 4th token is starting Y-position:
        len = (size_t)(regex_match[4].rm_eo - regex_match[4].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[4].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->start_Y_um[net_number] = strtof(temp_line, NULL);

        // 5th token is end-layer:
        len = (size_t)(regex_match[5].rm_eo - regex_match[5].rm_so);
        strncpy(user_inputs->end_layer[net_number], line + regex_match[5].rm_so, (int)len);
        user_inputs->end_layer[net_number][len] = '\0'; // Terminate string with NULL character

        // 6th token is ending X-position:
        len = (size_t)(regex_match[6].rm_eo - regex_match[6].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[6].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_X_um[net_number] = strtof(temp_line, NULL);

        // 7th token is ending Y-position:
        len = (size_t)(regex_match[7].rm_eo - regex_match[7].rm_so);
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, line + regex_match[7].rm_so, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->end_Y_um[net_number] = strtof(temp_line, NULL);

        // 8th token is name of diff-pair design rule:
        len = (size_t)(regex_match[8].rm_eo - regex_match[8].rm_so);
        strncpy(user_inputs->netSpecificRuleName[net_number], line + regex_match[8].rm_so, (int)len);
        user_inputs->netSpecificRuleName[net_number][len] = '\0'; // Terminate string with NULL character

        // 9th token is name of diff-pair partner net:
        len = (size_t)(regex_match[9].rm_eo - regex_match[9].rm_so);
        strncpy(user_inputs->diffPairPartnerName[net_number], line + regex_match[9].rm_so, (int)len);
        user_inputs->diffPairPartnerName[net_number][len] = '\0'; // Terminate string with NULL character

        // 10th token is 'pn_swappable', per the definition of the regular expression.

        // Specify that this net is part of a differential pair of nets:
        user_inputs->isDiffPair[net_number] = TRUE;

        // Specify that this net follows a special design rule:
        user_inputs->usesSpecialRule[net_number] = TRUE;

        // Specify that this net is part of a diff-pair with terminals whose polarity may be inverted:
        user_inputs->isPNswappable[net_number] = TRUE;

        // Increment the counter for the number of nets. Note that the first net
        // will be index #0.
        net_number++;

        continue; // Skip to next line in input file

      }  // End of if/else/if-block for matching a diff-pair netlist line

      else {
        printf("\nERROR: I expected details about a net, but found the following line instead:\n%s\n\n", line);
        printf("       Please fix the input file and restart the program. Program is terminating.\n\n");

        // Free memory for regular expressions of nets:
        regfree(&regex_single_ended);
        regfree(&regex_special_net);
        regfree(&regex_diff_pair);
        regfree(&regex_diff_pair_swappable_terms);

        exit(1);
      }

    }  // End of if-block for (netlist_flag)


    //
    // Check for line of the form 'layer_names = A B C D E'
    //
    compile_regex("layer_names[[:blank:]]*=[[:blank:]]*(.*)[[:blank:]]*$", &regex);
    if (regexec(&regex, line, 2, regex_match, 0) == 0)  {
      regfree(&regex);
      char layer_list[200] = "";
      len = (int)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(layer_list, line + regex_match[1].rm_so, len);
      layer_list[len] = '\0';  // Terminate string with NULL character


      // printf("DEBUG: List of layers is %s\n", layer_list);

      char *src = line + regex_match[1].rm_so;
      char *end = line + regex_match[1].rm_eo;
      int num_layers = 0;
      while (src < end)  {
        num_layers++;

        // Confirm that number of layers doesn't exceed maximum allowed:
        if (num_layers > ((2*maxRoutingLayers) - 1))  {
          printf("\nERROR: Input file specifies more layers (%d) than allowed (%d).\n\n",
             num_layers, (2*maxRoutingLayers) - 1);
          exit(1);
        }
        size_t len = strcspn(src, whitespace);
        if (src + len > end)
          len = end - src;
        // printf("DEBUG: Layer name: <<%.*s>>\n", (int)len, src);

        // Capture the layer name in the user_inputs structure:
        // printf("DEBUG: Copying %d characters from '%s' to layer_names[%d].\n", (int)len, src, num_layers);
        strncpy(user_inputs->layer_names[num_layers-1], src, (int)len);
        user_inputs->layer_names[num_layers-1][(int)len] = '\0'; // Add a NULL char to terminate string

        src += len;
        src += strspn(src, whitespace);

        // Confirm that name of layer has not been used for any previous layers:
        for (int i = 0; i < num_layers-1; i++)  {
          if (0 == strcasecmp(user_inputs->layer_names[num_layers-1], user_inputs->layer_names[i]))  {
            printf("\n\nERROR: The name of layer #%d is '%s', which is also the name of layer #%d.\n",
                    num_layers, user_inputs->layer_names[num_layers-1], i+1);
            printf("       Each layer must have a unique name. Modify the input file\n");
            printf("       '%s'\n", input_filename);
            printf("       and re-start the program.\n\n");
            exit(1);
          }
        }  // End of for-loop for index 'i'

        // printf("DEBUG: Layer %d has name %s\n", num_layers-1, user_inputs->layer_names[num_layers-1]);
      }  // End of while-loop for (src < end>

      // Calculate number of routing layers from list of layer names:
      if (num_layers % 2 == 1)  {
        // Number of named layers is an odd number (good!)
        num_named_layers = num_layers;
        printf("INFO: Number of named layers in 'layer_names' line is %d\n", num_named_layers);
      }
      else {
        // Exit with error message if even number of layer names is read:
        printf("\nERROR: The number of layer names from the input file is even:\n");
        printf("%s\n", line);
        printf("Only odd numbers of layer names are allowed.\n\n");
        exit(1);
      }

      continue;  // Skip to next line in input file
      
    }  // End of if-block for matching 'layer_names = A B C D E' line
    else  {
      regfree(&regex);
    }


    //
    // Check for line of the form "A = B":
    //
    compile_regex("([^[:blank:]]+)[[:blank:]]*=[[:blank:]]*([^[:blank:]]+)$", &regex);
    if (regexec(&regex, line, 3, regex_match, 0) == 0)  {
      regfree(&regex);
      char key[40] = "", value[40] = "";
      len = (int)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(key,   line + regex_match[1].rm_so, len);
      key[len] = '\0';  // Terminate string with NULL character

      len = (int)(regex_match[2].rm_eo - regex_match[2].rm_so);
      strncpy(value, line + regex_match[2].rm_so, len);
      value[len] = '\0';  // Terminate string with NULL character

      // printf("DEBUG: key/value is %s/%s\n", key, value);

      //
      // Based on the 'key', assign the value to the appropriate variable in the
      // user_inputs structure:
      //

      // "origin = ..."
      if (strcasecmp(key, "origin") == 0)  
        strcpy(user_inputs->origin, value);

      // "number_layers = ..."
      else if (strcasecmp(key, "number_layers") == 0) {
        user_inputs->num_routing_layers = strtof(value, NULL);
        mapInfo->numLayers = user_inputs->num_routing_layers;
      }

      // "width = ..."
      else if (strcasecmp(key, "width") == 0)  
        user_inputs->map_width_mm = strtof(value, NULL);

      // "height = ..."
      else if (strcasecmp(key, "height") == 0)
        user_inputs->map_height_mm = strtof(value, NULL);

      // "grid_resolution = ..."
      else if (strcasecmp(key, "grid_resolution") == 0)
        user_inputs->cell_size_um = strtof(value, NULL);
 
      // "maxIterations = ..."
      else if (strcasecmp(key, "maxIterations") == 0)  {
        user_inputs->maxIterations = strtof(value, NULL);
        mapInfo->max_iterations = user_inputs->maxIterations;
      }

      // "violationFreeThreshold = ..."
      else if (strcasecmp(key, "violationFreeThreshold") == 0)
        user_inputs->userDRCfreeThreshold = strtof(value, NULL);

      // "vertCost = ..." (in microns)
      else if (strcasecmp(key, "vertCost") == 0)   {
        base_vert_cost_defined = TRUE;
        user_inputs->baseVertCostMicrons = strtof(value, NULL);
      }

      // "runsPerPngMap = ..."
      else if (strcasecmp(key, "runsPerPngMap") == 0) 
        user_inputs->runsPerPngMap = strtof(value, NULL);
 
      // "preEvaporationIterations = ..."
      else if (strcasecmp(key, "preEvaporationIterations") == 0)  {
        user_inputs->preEvaporationIterations = strtof(value, NULL);
        // If user specified a value less than 2 for preEvaporationIterations,
        // then re-define the parameter as '2' the minimum allowed value.
        if (user_inputs->preEvaporationIterations < 2)  {
          printf("\nWarning: Input file specified a value less than 2 for preEvaporationIterations.\n");
          printf("         Converting this value to 2, which is the minimum allowed.\n\n");
          user_inputs->preEvaporationIterations = 2;
        }
      }

      // "allowed_directions = ..."
      else if (strcasecmp(key, "allowed_directions") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: An 'allowed_directions' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          // Set the numeric value of 'routeDirections' based on the user's inputs. See 'global_defs.h' file for
          // numeric (hexadecimal) values of each 'direction':
          if (strcasecmp(value, "ANY") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = ANY;
          }
          else if (strcasecmp(value, "NONE") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = NONE;
          }
          else if (strcasecmp(value, "MANHATTAN") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = MANHATTAN;
          }
          else if (strcasecmp(value, "X_ROUTING") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = X_ROUTING;
          }
          else if (strcasecmp(value, "NORTH_SOUTH") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = NORTH_SOUTH;
          }
          else if (strcasecmp(value, "EAST_WEST") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = EAST_WEST;
          }
          else if (strcasecmp(value, "MANHATTAN_X") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = MANHATTAN_X;
          }
          else if (strcasecmp(value, "UP_DOWN") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = UP_DOWN;
          }
          else if (strcasecmp(value, "ANY_LATERAL") == 0)  {
            user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = ANY_LATERAL;
          }
         else  {
            printf("\n\nERROR: An illegal value was specified for an 'ALLOWED_DIRECTIONS' statement:\n\n");
            printf(    "          %s = %s\n\n", key, value);
            printf(    "       Allowed values are ANY, NONE, MANHATTAN, X_ROUTING, NORTH_SOUTH, EAST_WEST, MANHATTAN_X, UP_DOWN, and ANY_LATERAL\n");
            printf(    "       Only one value may be used. Please correct the input file and re-start the program.\n\n");
            exit(1);
          }  // End of if/else block for handing various values in 'ALLOWED_DIRECTIONS = ' statement

          // printf("DEBUG:   routeDirections = %d for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'allowed_directions' token

      // "line_width = ..."
      else if (strcasecmp(key, "line_width") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'line_width' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].lineWidthMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].copy_lineWidthMicrons
             = user_inputs->designRules[design_rule_set][design_rule_subset].width_um[TRACE]
              = strtof(value, NULL);
          // printf("DEBUG:   lineWidthMicrons = %6.3f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].lineWidthMicrons, design_rule_set, design_rule_subset);
          // printf("DEBUG:   copy_lineWidthMicrons = %6.3f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].copy_lineWidthMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'line_width' token

      // "line_spacing = ..."
      else if (strcasecmp(key, "line_spacing") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'line_spacing' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].lineSpacingMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[TRACE][TRACE]
              = strtof(value, NULL);
          // printf("DEBUG:   lineSpacingMicrons = %5f for design-rule set %d, subset %d.\n",
          //        user_inputs->designRules[design_rule_set][design_rule_subset].lineSpacingMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'line_spacing' token

      // "via_up_diameter = ..."
      else if (strcasecmp(key, "via_up_diameter") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'via_up_diameter' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].viaUpDiameterMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].copy_viaUpDiameterMicrons
             = user_inputs->designRules[design_rule_set][design_rule_subset].width_um[VIA_UP]
              = strtof(value, NULL);
          // printf("DEBUG:   viaUpDiameterMicrons = %5f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].viaUpDiameterMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'via_up_diameter' token

      // "via_down_diameter = ..."
      else if (strcasecmp(key, "via_down_diameter") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'via_down_diameter' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].viaDownDiameterMicrons
           = user_inputs->designRules[design_rule_set][design_rule_subset].copy_viaDownDiameterMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].width_um[VIA_DOWN]
              = strtof(value, NULL);
          // printf("DEBUG:   viaDownDiameterMicrons = %5f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].viaDownDiameterMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'via_down_diameter' token

      // "via_up_to_trace_spacing = ..."
      else if (strcasecmp(key, "via_up_to_trace_spacing") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'via_up_to_trace_spacing' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].viaUpToTraceSpacingMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[VIA_UP][TRACE]
              = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[TRACE][VIA_UP]
                = strtof(value, NULL);
          // printf("DEBUG:   viaUpToTraceSpacingMicrons = %5f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].viaUpToTraceSpacingMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'via_up_to_trace_spacing' token

      // "via_down_to_trace_spacing = ..."
      else if (strcasecmp(key, "via_down_to_trace_spacing") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'via_down_to_trace_spacing' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].viaDownToTraceSpacingMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[VIA_DOWN][TRACE]
              = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[TRACE][VIA_DOWN]
                = strtof(value, NULL);

          // printf("DEBUG:   viaDownToTraceSpacingMicrons = %5f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].viaDownToTraceSpacingMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'via_down_to_trace_spacing' token

      // "via_up_to_via_up_spacing = ..."
      else if (strcasecmp(key, "via_up_to_via_up_spacing") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'via_up_to_via_up_spacing' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].viaUpToViaUpSpacingMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[VIA_UP][VIA_UP]
              = strtof(value, NULL);
          // printf("DEBUG:   viaUpToViaUpSpacingMicrons = %5f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].viaUpToViaUpSpacingMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'via_up_to_via_up_spacing' token

      // "via_down_to_via_down_spacing = ..."
      else if (strcasecmp(key, "via_down_to_via_down_spacing") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'via_down_to_via_down_spacing' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].viaDownToViaDownSpacingMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[VIA_DOWN][VIA_DOWN]
              = strtof(value, NULL);
          // printf("DEBUG:   viaDownToViaDownSpacingMicrons = %5f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].viaDownToViaDownSpacingMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'via_down_to_via_down_spacing' token

      // "via_up_to_via_down_spacing = ..."
      else if (strcasecmp(key, "via_up_to_via_down_spacing") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: A 'via_up_to_via_down_spacing' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          user_inputs->designRules[design_rule_set][design_rule_subset].viaUpToViaDownSpacingMicrons
            = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[VIA_UP][VIA_DOWN]
              = user_inputs->designRules[design_rule_set][design_rule_subset].space_um[VIA_DOWN][VIA_UP]
                = strtof(value, NULL);
          // printf("DEBUG:   viaUpToViaDownSpacingMicrons = %5f for design-rule set %d, subset %d.\n",
          //         user_inputs->designRules[design_rule_set][design_rule_subset].viaUpToViaDownSpacingMicrons, design_rule_set, design_rule_subset);
        }
      }  // End of if/else block for 'via_up_to_via_down_spacing' token

      // "exception = ..."
      else if (strcasecmp(key, "exception") == 0)  {
        if (design_rule_flag == FALSE)  {
          printf("\nERROR: An 'exception =' statement was found outside of a 'design_rule_set' block: %s = %s\n",
                  key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else if (exception_flag == TRUE)  {
          printf("\nERROR: An 'exception =' statement was found nested inside another 'exception =' statement.\n");
          printf("       The offending statement is: %s = %s\n",
                   key, value);
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);

        }
        else {
          // We've gotten to beginning of a design-rule exception, so set the
          // 'exception_flag' flag and increment the number of exceptions:
          exception_flag = TRUE;
          design_rule_subset++;

          // Confirm that the number of design-rule subsets has not exceeded the maximum allowed
          // value of "maxDesignRuleSubsets":
          if (design_rule_subset >= maxDesignRuleSubsets - 1)  {
            printf("\n\nERROR: For design-rule set '%s', there are more than %d exceptions, which is the\n",
                   user_inputs->designRuleSetName[design_rule_set], maxDesignRuleSubsets - 1);
            printf(    "       maximum allowed number of exceptions per design-rule set. Please modify the input file\n");
            printf(    "       and restart the program.\n\n");
            exit(1);
          }  // End of if-block for exceeding the allowed number of design-rule subsets

          // Initialize the parameter 'routeDirections' to the default value of 'ANY' in
          // case the user did not specify an 'allowed_directions' statement for this subset:
          user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = ANY;

          // Initialize the 'isDiffPairSubset' to FALSE. This variable will be changed to TRUE if
          // the parser finds a 'diff_pair_pitch' keyword:
          user_inputs->designRules[design_rule_set][design_rule_subset].isDiffPairSubset = FALSE;

          // Initialize the 'isPseudoNetSubset' to FALSE. This variable will only be TRUE for
          // design-rule subsets that are copied from an exception with a 'diff_pair_pitch' keyword:
          user_inputs->designRules[design_rule_set][design_rule_subset].isPseudoNetSubset = FALSE;
          // printf("DEBUG: Initialized isPseudoNetSubset to FALSE for design rule set %d, subset %d.\n", design_rule_set, design_rule_subset);

          // Copy name of design-rule exception into structure:
          strcpy(user_inputs->designRules[design_rule_set][design_rule_subset].subsetName, value);
          // printf("DEBUG:   ** Design rule subset name = '%s' for subset %d in design-rule set %d.\n",
          //        user_inputs->designRules[design_rule_set][design_rule_subset].subsetName, design_rule_subset, design_rule_set);

          // Copy the design-rule parameters from the default design-rule subset (#0) to the
          // 'exception' design-rule subset. These will be overwritten with any user-supplied
          // exception values:
          copyDesignRuleSubset(user_inputs, design_rule_set, 0, design_rule_set, design_rule_subset);
          // printf("DEBUG:      Copied design-rule parameters from set %d, subset %d, to set %d, subset %d.\n",
          //         design_rule_set, 0, design_rule_set, design_rule_subset);

        }
      }  // End of if/else block for 'exception' token

      // "diff_pair_pitch = ..."
      else if (strcasecmp(key, "diff_pair_pitch") == 0)  {
        if (! exception_flag)  {
          printf("\nERROR: A 'diff_pair_pitch =' statement was found outside of a design-rule exception: %s = %s\n",
                  key, value);
          printf("       This keyword is only allowed between an 'exception =' and 'end_exception' statement.\n");
          printf("       Please correct input file and re-start the program.\n\n");
          exit(1);
        }
        else {
          // Copy the diff-pair pitch into data structure:
          user_inputs->designRules[design_rule_set][design_rule_subset].traceDiffPairPitchMicrons = strtof(value, NULL);

          // Flag this design-rule subset as being dedicated to differential pairs:
          user_inputs->designRules[design_rule_set][design_rule_subset].isDiffPairSubset = TRUE;

          // printf("DEBUG:   Design rule #%d, subset #%d, is flagged as a diff-pair design-rule subset.\n", design_rule_set, design_rule_subset);
          // printf("DEBUG:   traceDiffPairPitchMicrons = %6.3f for design rule #%d, subset #%d.\n",
          //        user_inputs->designRules[design_rule_set][design_rule_subset].traceDiffPairPitchMicrons, design_rule_set, design_rule_subset);

        }  // End of else block (design_rule_flag is TRUE)
      }  // End of if/else block for 'diff_pair_pitch = ...' token


      else {
        printf("\nERROR: The following unexpected 'key = value' statement was found in the input file:\n");
        printf("       %s = %s\n\n", key, value);
        exit(1);
      }  // End of if/else-block

    }  // End of if-block for matching "A = B" line
    else  {
      regfree(&regex);
    }


    //
    // Check for lines of the form "BLOCK TYPE LAYER..." 
    //                          or "UNBLOCK TYPE LAYER...:
    //
    compile_regex("^([UN]*BLOCK)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //               (UN)BLOCK               <---TYPE>-->               <---Layer--->  <params>
    //
    if (regexec(&regex, line, 5, regex_match, 0) == 0)  {
      regfree(&regex);

      int num_parameters = 0; // number of parameters that follow the layer name in BLOCK command

      // printf("DEBUG: Detected BLOCK/UNBLOCK command in input file. num_block_instructions=%d\n",
      //        num_block_instructions);


      // Extract the command, type, and layer name and save them in the
      // 'user_inputs' structure:
      len = (int)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(user_inputs->block_command[num_block_instructions], line + regex_match[1].rm_so, len);
      user_inputs->block_command[num_block_instructions][len] = '\0';  // Terminate string with NULL character

      len = (int)(regex_match[2].rm_eo - regex_match[2].rm_so);
      strncpy(user_inputs->block_type[num_block_instructions], line + regex_match[2].rm_so, len);
      user_inputs->block_type[num_block_instructions][len] = '\0';  // Terminate string with NULL character

      len = (int)(regex_match[3].rm_eo - regex_match[3].rm_so);
      strncpy(user_inputs->block_layer[num_block_instructions], line + regex_match[3].rm_so, len);
      user_inputs->block_layer[num_block_instructions][len] = '\0';  // Terminate string with NULL character

      // printf("DEBUG: command/type/layer is '%s/%s/%s'\n", user_inputs->block_command[num_block_instructions],
      //                                                     user_inputs->block_type[num_block_instructions],
      //                                                     user_inputs->block_layer[num_block_instructions] );

      char *src = line + regex_match[4].rm_so; // Get beginning pointer of string containing parameters
      char *end = line + regex_match[4].rm_eo; // Get end pointer of string containing parameters
      size_t leading_space = strspn(src, whitespace); // Check for leading spaces
      // printf("DEBUG: leading_space is %d.\n", (int)leading_space);
      src = src + leading_space;  // Remove leading space from series of floating-point parameters 

      while (src < end)  {

        size_t len = strcspn(src, whitespace); // Find token up to next space character
        if (src + len > end)
          len = end - src;
        // printf("DEBUG: Parameter: <<%.*s>>\n", (int)len, src);

        // Capture the floating point parameter in the user_inputs structure:
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, src, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->block_parameters[num_block_instructions][num_parameters] = strtof(temp_line, NULL);

        // printf("DEBUG:   Added parameter %8.2f to user_inputs->block_parameters[%d][%d] structure.\n",
        //         user_inputs->block_parameters[num_block_instructions][num_parameters],
        //         num_block_instructions, num_parameters );

        // Increment the number of parameters and confirm that it has not exceeded the max allowable:
        num_parameters++; // Increment the number of parameters
        if (num_parameters > maxBlockParameters)  {
          printf("\nERROR: Input file specifies more numeric parameters than allowed (%d) for command '%s %s %s'.\n\n",
             maxBlockParameters,  user_inputs->block_command[num_block_instructions],
                                  user_inputs->block_type[num_block_instructions],
                                  user_inputs->block_layer[num_block_instructions] );
          exit(1);
        }

        src += len;
        src += strspn(src, whitespace);
      }  // End of while-loop for (src < end)

      // Capture the number of parameters for this BLOCK command:
      user_inputs->block_num_params[num_block_instructions] = num_parameters;
      // printf("DEBUG:   Number of BLOCK/UNBLOCK parameters is %d\n", 
      //                         user_inputs->block_num_params[num_block_instructions]);

      num_block_instructions++; // Increment the number of BLOCK/UNBLOCK instructions
      // printf("DEBUG: num_block_instructions is now %d\n", num_block_instructions);
      if (num_block_instructions >= maxBlockInstructions)  {
        printf("\nERROR: The number of BLOCK/UNBLOCK instructions in the input file has exceeded the\n");
        printf("       allowed number (%d). Pleased edit input file and re-start program.\n\n", maxBlockInstructions);
        exit(1);
      }  // End of if-block for num_block_instructions exceeding allowed value

      continue;  // Skip to next line of input file
    }  // End of if-block for matching "BLOCK TYPE LAYER..." line
    else  {
      regfree(&regex);
    }


    // Check for key word 'design_rule_set':
    compile_regex("^design_rule_set[[:blank:]]+([^[:blank:]]+)[[:blank:]]+(.*)$", &regex);
    //              design_rule_set            <---DR_name--->            DR_description
    if (regexec(&regex, line, 4, regex_match, 0) == 0)  {
      regfree(&regex);

      design_rule_flag = TRUE;

      // Check that the number of design-rule sets has not exceeded the maximum allowed value
      // of "maxDesignRuleSets".
      if (design_rule_set >= maxDesignRuleSets)  {
        printf("\n\nERROR: The number of design-rule sets specified in the input file exceeds the allowed value of %d.\n", maxDesignRuleSets);
        printf(    "       Please reduce the number of design-rule sets in the input file and restart the program.\n\n");
        exit(1);
      }  // End of if-block for the number of design-rule sets exceeding the limit

      design_rule_subset = 0; // Initialize the subset number to zero for the new design-rule set
      // printf("DEBUG: Found 'design_rule_set', so setting design_rule_flag to TRUE, and design_rule_subset to zero.\n");

      // Initialize the parameter 'routeDirections' to the default value of 'ANY' in
      // case the user did not specify an 'allowed_directions' statement for this subset:
      user_inputs->designRules[design_rule_set][design_rule_subset].routeDirections = ANY;

      // Initialize the 'isDiffPairSubset' to FALSE. This variable will be changed to TRUE if
      // the parser finds a 'diff_pair_pitch' keyword:
      user_inputs->designRules[design_rule_set][design_rule_subset].isDiffPairSubset = FALSE;

      // Initialize the 'isPseudoNetSubset' to FALSE. This variable will only be TRUE for
      // design-rule subsets that are copied from an exception with a 'diff_pair_pitch' keyword:
      user_inputs->designRules[design_rule_set][design_rule_subset].isPseudoNetSubset = FALSE;
      // printf("DEBUG: Initialized isPseudoNetSubset to FALSE for design rule set %d, subset %d.\n", design_rule_set, design_rule_subset);

      // Token after 'design_rule_set' is the unique name of the set:
      size_t len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(user_inputs->designRuleSetName[design_rule_set], line + regex_match[1].rm_so, (int)len);
      user_inputs->designRuleSetName[design_rule_set][len] = '\0'; // Terminate string with NULL character
      // printf("DEBUG:   Name of design-rule set %d is '%s'.\n", design_rule_set,
      //         user_inputs->designRuleSetName[design_rule_set]);

      // Text after name of design-rule set is the description of the set:
      len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
      strncpy(user_inputs->designRuleSetDescription[design_rule_set], line + regex_match[2].rm_so, (int)len);
      user_inputs->designRuleSetDescription[design_rule_set][len] = '\0'; // Terminate string with NULL character
      // printf("DEBUG:   Description of design-rule set %d is '%s'.\n", design_rule_set,
      //        user_inputs->designRuleSetDescription[design_rule_set]);

      // Confirm that name of design-rule set has not been used for previous design-rule sets:
      for (int i = 0; i < design_rule_set; i++)  {
        if (0 == strcasecmp(user_inputs->designRuleSetName[design_rule_set], user_inputs->designRuleSetName[i]))  {
          printf("\n\nERROR: The name of design-rule set #%d is '%s', which is also the name of design-rule set #%d.\n",
                  design_rule_set+1, user_inputs->designRuleSetName[design_rule_set], i+1);
          printf("       Each design-rule set must have a unique name. Modify the input file\n");
          printf("       '%s'\n", input_filename);
          printf("       and re-start the program.\n\n");
          exit(1);
        }
      }  // End of for-loop for index 'i'

      // Assign the name '__DEFAULT__' to the 0th subset associated with the design rules
      // in this 'design_rule_set' block:
      strcpy(user_inputs->designRules[design_rule_set][0].subsetName, "__DEFAULT__");

      continue;  // Skip to next line of input file.
    }
    else  {
      regfree(&regex);
    }

    // Check for key word 'end_design_rule_set'
    compile_regex("^end_design_rule_set$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);

      design_rule_flag = FALSE;
      // printf("DEBUG:   **** 'end_design_rule_set' statement found, so changing design_rule_flag to FALSE\n");

      // We've gotten to end of a design-rule set, so increment the number of sets:
      design_rule_set++;

      continue;  // Skip to next line of input file
    }
    else  {
      regfree(&regex);
    }

    // Check for key word 'end_exception'
    compile_regex("^end_exception$", &regex);
    if (regexec(&regex, line, 0, regex_match, 0) == 0)  {
      regfree(&regex);
      if (exception_flag == FALSE)  {
        printf("\nERROR: An 'end_exception' statement was found without a corresponding 'exception = ' statement beforehand.\n");
        printf("       Please fix the input text file and restart the program.\n\n");
        exit(1);
      }
      else  {
        exception_flag = FALSE;
        // printf("DEBUG:   ** 'end_exception' statement found, so changing exception_flag to FALSE\n");

        // If the design-rule exception that we just exited was a diff-pair exception, then make a copy of
        // the design-rule exception and increment the number of exceptions. The copy will be used for
        // routing and design-rule checking the wide pseudo-net; the original version will be used for
        // routing and design-rule checking the individual diff-pair nets:
        if (user_inputs->designRules[design_rule_set][design_rule_subset].isDiffPairSubset)  {
          // printf("DEBUG: 'end_exception' statement found with 'isDiffPairSubset' = TRUE, so copying rules from set %d, subset %d, to set %d, subset %d\n",
          //         design_rule_set, design_rule_subset, design_rule_set, design_rule_subset + 1);
          copyDesignRuleSubset(user_inputs, design_rule_set, design_rule_subset, design_rule_set, design_rule_subset + 1);

          // Also copy the name of the exception, so the same name applies to two (related) exceptions:
          // printf("DEBUG: Copying name '%s' to new subset\n", user_inputs->designRules[design_rule_set][design_rule_subset].subsetName);
          strcpy(user_inputs->designRules[design_rule_set][design_rule_subset + 1].subsetName,
                 user_inputs->designRules[design_rule_set][design_rule_subset].subsetName);
          // printf("DEBUG: After copying, the destination string is '%s'\n",
          //        user_inputs->designRules[design_rule_set][design_rule_subset + 1].subsetName);

          // Increment the number of design-rule subsets for the current design-rule set:
          design_rule_subset++;
          // printf("DEBUG: 'end_exception' statement found with 'isDiffPairSubset' = TRUE, so design_rule_subset has been increased to %d\n", design_rule_subset);

          // In the copied design-rule subset, set the 'isPseudoNetSubset' flag to tell Acorn
          // that this subset is to be used for routing and design-rule checking pseudo-nets:
          user_inputs->designRules[design_rule_set][design_rule_subset].isPseudoNetSubset = TRUE;
          // printf("DEBUG: isPseudoNetSubset flag set to TRUE for design rule set %d, subset %d\n", design_rule_set, design_rule_subset);

        }

        continue;
      }  // End of if/else block for (exception_flag == FALSE)
    }
    else  {
      regfree(&regex);
    }

    //
    // Check for lines of the form "DR_zone <DR name> <layer name> <shape type> ...."
    //
    compile_regex("^DR_zone[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //              DR_zone             <--DR name-->              <---Layer--->             <-Shape type->  <params>
    //
    if (regexec(&regex, line, 6, regex_match, 0) == 0)  {
      regfree(&regex);

      int num_parameters = 0; // number of parameters that follow the shape type in DR_zone command

      // printf("\nDEBUG: Detected DR_zone command in input file. num_DR_zone_instructions = %d\n",
      //         num_DR_zone_instructions);

      // Extract the command, type, and layer name and save them in the
      // 'user_inputs' structure:
      len = (int)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(user_inputs->DR_zone_name[num_DR_zone_instructions], line + regex_match[1].rm_so, len);
      user_inputs->DR_zone_name[num_DR_zone_instructions][len] = '\0';  // Terminate string with NULL character

      len = (int)(regex_match[2].rm_eo - regex_match[2].rm_so);
      strncpy(user_inputs->DR_zone_layer[num_DR_zone_instructions], line + regex_match[2].rm_so, len);
      user_inputs->DR_zone_layer[num_DR_zone_instructions][len] = '\0';  // Terminate string with NULL character

      len = (int)(regex_match[3].rm_eo - regex_match[3].rm_so);
      strncpy(user_inputs->DR_zone_shape[num_DR_zone_instructions], line + regex_match[3].rm_so, len);
      user_inputs->DR_zone_shape[num_DR_zone_instructions][len] = '\0';  // Terminate string with NULL character


      // printf("DEBUG:   DR_zone name/layer/shape is '%s/%s/%s'\n", user_inputs->DR_zone_name[num_DR_zone_instructions],
      //                                                     user_inputs->DR_zone_layer[num_DR_zone_instructions],
      //                                                     user_inputs->DR_zone_shape[num_DR_zone_instructions] );

      char *src = line + regex_match[4].rm_so; // Get beginning pointer of string containing parameters
      char *end = line + regex_match[4].rm_eo; // Get end pointer of string containing parameters
      size_t leading_space = strspn(src, whitespace); // Check for leading spaces
      // printf("DEBUG: leading_space is %d.\n", (int)leading_space);
      src = src + leading_space;  // Remove leading space from series of floating-point parameters 

      while (src < end)  {

        size_t len = strcspn(src, whitespace); // Find token up to next space character
        if (src + len > end)
          len = end - src;
        // printf("DEBUG: Parameter: <<%.*s>>\n", (int)len, src);

        // Capture the floating point parameter in the user_inputs structure:
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, src, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->DR_zone_parameters[num_DR_zone_instructions][num_parameters] = strtof(temp_line, NULL);

        // printf("DEBUG:   Added parameter %8.2f to user_inputs->DR_zone_parameters[%d][%d] structure.\n",
        //         user_inputs->DR_zone_parameters[num_DR_zone_instructions][num_parameters],
        //         num_DR_zone_instructions, num_parameters );

        // Increment the number of parameters and confirm that it has not exceeded the max allowable:
        //   DR_zone <DR name> <layer name> <shape type> ....
        num_parameters++; // Increment the number of parameters
        if (num_parameters > maxDRzoneParameters)  {
          printf("\nERROR: Input file specifies more numeric parameters than allowed (%d) for command 'DR_ZONE %s %s %s'.\n\n",
             maxDRzoneParameters,  user_inputs->DR_zone_name[num_DR_zone_instructions],
                                   user_inputs->DR_zone_layer[num_DR_zone_instructions],
                                   user_inputs->DR_zone_shape[num_DR_zone_instructions] );
          exit(1);
        }

        src += len;
        src += strspn(src, whitespace);
      }  // End of while-loop for (src < end)

      // Capture the number of parameters for this DR_zone command:
      user_inputs->DR_zone_num_params[num_DR_zone_instructions] = num_parameters;
      // printf("DEBUG:   Number of DR_zone parameters is %d\n",
      //                         user_inputs->DR_zone_num_params[num_DR_zone_instructions]);

      num_DR_zone_instructions++; // Increment the number of DR_zone instructions
      // printf("DEBUG:   num_DR_zone_instructions has been incremented to %d\n", num_DR_zone_instructions);

      continue;  // Skip to next line of input file.
    }  // End of if-block for matching "DR_zone <DR name> <layer name> <shape type> ...." line
    else  {
      regfree(&regex);
    }


    //
    // Check for lines of the form "trace_cost_multiplier <1 to 15> <multiplier integer>"
    //
    compile_regex("^trace_cost_multiplier[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)$", &regex);
    //              trace_cost_multiplier              <-1 to 15->              <-Multiplier-> 
    //
    if (regexec(&regex, line, 3, regex_match, 0) == 0)  {
      regfree(&regex);
      size_t len;  // Temporary variable to hold length of strings.

      // printf("\nDEBUG: Detected trace_cost_multiplier statement in input file.\n");

      // Extract the index for the multiplier, which is the 2nd token:
      len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
      strncpy(temp_line, line + regex_match[1].rm_so, (int)len);
      temp_line[len] = '\0';  // Terminate string with NULL character
      multiplier_index = strtof(temp_line, NULL);
      // printf("DEBUG: trace_cost_multiplier index is %d\n", multiplier_index);
 
      // If user supplied index value that's too large, then issue error
      // message and terminate program:
      if (multiplier_index > maxTraceCostMultipliers - 1)  {
        printf("\nERROR: A 'trace_cost_multiplier' line in the input file specifies an\n");
        printf("       index of %d, which is larger than the maximum allowed index (%d).\n",
                multiplier_index, maxTraceCostMultipliers - 1);
        printf("       Please correct input file and re-start the program.\n\n");
        exit(1);
      }  // End of if-block

      // 3rd token is cost multiplier:
      len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
      strncpy(temp_line, line + regex_match[2].rm_so, (int)len);
      temp_line[len] = '\0';  // Terminate string with NULL character
      user_inputs->traceCostMultiplier[multiplier_index] = strtof(temp_line, NULL);
      // printf("DEBUG: trace_cost_multiplier value is %d.\n", user_inputs->traceCostMultiplier[multiplier_index]);

      continue;  // Skip to next line of input file

    }  // End of if-block for matching "trace_cost_multiplier <index number>  <multiplier>"
    else  {
      regfree(&regex);
    }


    //
    // Check for lines of the form "via_cost_multiplier <1 to 7> <multiplier integer>"
    //
    compile_regex("^via_cost_multiplier[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([.[:digit:]]+)$", &regex);
    //              via_cost_multiplier              <-1 to 15->              <-Multiplier-> 
    //
    if (regexec(&regex, line, 3, regex_match, 0) == 0)  {
      regfree(&regex);
      size_t len;  // Temporary variable to hold length of strings.

      // printf("\nDEBUG: Detected via_cost_multiplier statement in input file.\n");

      // Extract the index for the multiplier, which is the 2nd token:
      len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
      strncpy(temp_line, line + regex_match[1].rm_so, (int)len);
      temp_line[len] = '\0';  // Terminate string with NULL character
      multiplier_index = strtof(temp_line, NULL);
      // printf("DEBUG: via_cost_multiplier index is %d\n", multiplier_index);

      // If user supplied index value that's too large, then issue error
      // message and terminate program:
      if (multiplier_index > maxViaCostMultipliers - 1)  {
        printf("\nERROR: A 'via_cost_multiplier' line in the input file specifies an\n");
        printf("       index of %d, which is larger than the maximum allowed index (%d).\n",
                multiplier_index, maxViaCostMultipliers - 1);
        printf("       Please correct input file and re-start the program.\n\n");
        exit(1);
      }  // End of if-block
 
      // 3rd token is cost multiplier:
      len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
      strncpy(temp_line, line + regex_match[2].rm_so, (int)len);
      temp_line[len] = '\0';  // Terminate string with NULL character
      user_inputs->viaCostMultiplier[multiplier_index] = strtof(temp_line, NULL);
      // printf("DEBUG: via_cost_multiplier value is %d.\n", user_inputs->viaCostMultiplier[multiplier_index]);

      continue;  // Skip to next line of input file.

    }  // End of if-block for matching "via_cost_multiplier <index number>  <multiplier>"
    else  {
      regfree(&regex);
    }


    //
    // Check for lines of the following 2 forms:
    //      "trace_cost_zone <zone index> <layer name> <shape type> ...."
    //         or
    //      "via_cost_zone <zone index> <layer name> <shape type> ...."
    //
    compile_regex("^(trace|via)_cost_zone[[:blank:]]+([.[:digit:]]+)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //                trace/via_cost_zone             <-zone index>              <---Layer--->             <-Shape type->  <params>
    //
    if (regexec(&regex, line, 7, regex_match, 0) == 0)  {
      regfree(&regex);
      size_t len;  // Temporary variable to hold length of strings.
      int num_parameters = 0; // number of parameters that follow the shape type in this cost-zone command

      // printf("\nDEBUG: Detected trace_cost_zone or via_cost_zone command in input file.\n");
      // printf("          num_trace_cost_zone_instructions = %d\n", num_trace_cost_zone_instructions);
      // printf("            num_via_cost_zone_instructions = %d\n", num_via_cost_zone_instructions);
      // printf("DEBUG: regex_match[1].rm_eo is %d.  regex_match[1].rm_so is %d.\n",
      //         regex_match[1].rm_eo, regex_match[1].rm_so);

 
      // Extract the command, type, and layer name and save them in the
      // 'user_inputs' structure:
      char trace_or_via[8];
      len = (size_t)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(trace_or_via, line + regex_match[1].rm_so, (int)len);
      trace_or_via[len] = '\0';  // Terminate string with NULL character
      // printf("DEBUG: Type of cost-zone is '%s'\n", trace_or_via);
  
      // Issue error and exit if we cannot determine whether statement is 
      // 'trace_cost_zone' or 'via_cost_zone':
      if ((strcasecmp(trace_or_via, "trace")) && (strcasecmp(trace_or_via, "via")))  {
        printf("\nERROR: Could not determine whether instruction was 'trace_cost_zone' or 'via_cost_zone'.\n\n");
        exit(1);
      }

      // Extract the index of the cost-multiplier, which is the 2nd token:
      len = (size_t)(regex_match[2].rm_eo - regex_match[2].rm_so);
      memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
      strncpy(temp_line, line + regex_match[2].rm_so, (int)len);
      temp_line[len] = '\0';  // Terminate string with NULL character
      multiplier_index = strtof(temp_line, NULL);
      // printf("DEBUG: Cost-multiplier index is %d\n", multiplier_index);

      // Confirm that the multiplier index does not exceed the allowed value:
      if (((0 == strcasecmp(trace_or_via, "trace")) && (multiplier_index >= maxTraceCostMultipliers-1))
         || ((0 == strcasecmp(trace_or_via, "via")) && (multiplier_index >= maxViaCostMultipliers-1))) {
        printf("\nERROR: The command '%s_cost_zone %d...' references an illegal cost-zone index (%d).\n",
               trace_or_via, multiplier_index, multiplier_index);
        printf("       The maximum index is %d for trace_cost_zone commands, and %d for via_cost_zone commands.\n",
                maxTraceCostMultipliers-1, maxViaCostMultipliers-1);
        printf("       Fix input file and re-start the program.\n\n");
        exit(1);
      }  

      // Extract the name of the routing or via layer, which is the 3rd token:
      len = (size_t)(regex_match[3].rm_eo - regex_match[3].rm_so);
      strncpy(temp_line, line + regex_match[3].rm_so, (int)len);
      temp_line[len] = '\0';  // Terminate string with NULL character
      // printf("DEBUG: Layer name is '%s'\n", temp_line);

      // Place the multiplier index and the layer name into the appropriate user_inputs elements.
      // Also flag this 'multiplier_index' as being used.
      if (0 == strcasecmp(trace_or_via, "trace"))  {
        user_inputs->trace_cost_zone_index[num_trace_cost_zone_instructions] = multiplier_index;
        user_inputs->traceCostMultiplierInvoked[multiplier_index] = TRUE;
        strcpy(user_inputs->trace_cost_zone_layer[num_trace_cost_zone_instructions], temp_line);
        // printf("DEBUG: trace_cost_zone_layer[%d] = '%s'\n", num_trace_cost_zone_instructions, 
        //         user_inputs->trace_cost_zone_layer[num_trace_cost_zone_instructions]);
      }
      else {
        user_inputs->via_cost_zone_index[num_via_cost_zone_instructions] = multiplier_index;
        user_inputs->viaCostMultiplierInvoked[multiplier_index] = TRUE;
        strcpy(user_inputs->via_cost_zone_layer[num_via_cost_zone_instructions], temp_line);
        // printf("DEBUG: via_cost_zone_layer[%d] = '%s'\n", num_via_cost_zone_instructions, 
        //         user_inputs->via_cost_zone_layer[num_via_cost_zone_instructions]);
      }  // End of if/else-block

      // Extract the shape of the cost zone, which is the 4th token (ALL, RECT, CIR or TRI):
      len = (size_t)(regex_match[4].rm_eo - regex_match[4].rm_so);
      strncpy(temp_line, line + regex_match[4].rm_so, (int)len);
      temp_line[len] = '\0';  // Terminate string with NULL character
      // printf("DEBUG: Cost-zone shape is '%s'\n", temp_line);

      // Place the shape name into the appropriate user_inputs element:
      if (0 == strcasecmp(trace_or_via, "trace"))  {
        strcpy(user_inputs->trace_cost_zone_shape[num_trace_cost_zone_instructions], temp_line);
        // printf("DEBUG: trace_cost_zone_shape[%d] = '%s'\n", num_trace_cost_zone_instructions, 
        //         user_inputs->trace_cost_zone_shape[num_trace_cost_zone_instructions]);
      }
      else if (0 == strcasecmp(trace_or_via, "via"))  {
        strcpy(user_inputs->via_cost_zone_shape[num_via_cost_zone_instructions], temp_line);
        // printf("DEBUG: via_cost_zone_shape[%d] = '%s'\n", num_via_cost_zone_instructions, 
        //         user_inputs->via_cost_zone_shape[num_via_cost_zone_instructions]);
      }

      char *src = line + regex_match[5].rm_so; // Get beginning pointer of string containing parameters
      char *end = line + regex_match[5].rm_eo; // Get end pointer of string containing parameters
      size_t leading_space = strspn(src, whitespace); // Check for leading spaces
      // printf("DEBUG: leading_space is %d.\n", (int)leading_space);
      src = src + leading_space;  // Remove leading space from series of floating-point parameters 

      while (src < end)  {

        size_t len = strcspn(src, whitespace); // Find token up to next space character
        if (src + len > end)
          len = end - src;
        // printf("DEBUG: Parameter: <<%.*s>>\n", (int)len, src);

        // Capture the floating point parameter in the user_inputs structure:
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, src, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        float parameter = strtof(temp_line, NULL);   // Convert string to floating point number
        
        if (0 == strcasecmp(trace_or_via, "trace"))  {
          user_inputs->trace_cost_zone_parameters[num_trace_cost_zone_instructions][num_parameters] = parameter;
        }
        else if (0 == strcasecmp(trace_or_via, "via"))  {
          user_inputs->via_cost_zone_parameters[num_via_cost_zone_instructions][num_parameters] = parameter;
        }



        // printf("DEBUG:   Added parameter %8.2f to user_inputs->DR_zone_parameters[%d][%d] structure.\n",
        //         user_inputs->DR_zone_parameters[num_DR_zone_instructions][num_parameters],
        //         num_DR_zone_instructions, num_parameters );

        // Increment the number of parameters and confirm that it has not exceeded the max allowable:
        //   DR_zone <DR name> <layer name> <shape type> ....
        num_parameters++; // Increment the number of parameters
        if (num_parameters > maxCostParameters)  {
          if (0 == strcasecmp(trace_or_via, "trace"))  {
            printf("\nERROR: Input file specifies more numeric parameters than allowed (%d) for command 'TRACE_COST_ZONE %d %s %s'.\n\n",
             maxCostParameters, user_inputs->trace_cost_zone_index[num_trace_cost_zone_instructions],
                                user_inputs->trace_cost_zone_layer[num_trace_cost_zone_instructions],
                                user_inputs->trace_cost_zone_shape[num_trace_cost_zone_instructions] );
          }
          else {
            printf("ERROR: Input file specifies more numeric parameters than allowed (%d) for command 'VIA_COST_ZONE %d %s %s'.\n\n",
             maxCostParameters, user_inputs->via_cost_zone_index[num_via_cost_zone_instructions],
                                user_inputs->via_cost_zone_layer[num_via_cost_zone_instructions],
                                user_inputs->via_cost_zone_shape[num_via_cost_zone_instructions] );
          }
          exit(1);
        }

        src += len;
        src += strspn(src, whitespace);
      }  // End of while-loop for (src < end)

      // Capture the number of parameters for this cost-zone command and increment the number
      // of cost-zone instructions:
      if (0 == strcasecmp(trace_or_via, "trace"))  {
        user_inputs->trace_cost_num_params[num_trace_cost_zone_instructions] = num_parameters;
        // printf("DEBUG:   Number of trace cost-zone parameters is %d\n", 
        //                       user_inputs->trace_cost_num_params[num_trace_cost_zone_instructions]);
        num_trace_cost_zone_instructions++; // Increment the number of trace_cost_zone instructions
      }
      else  {
        user_inputs->via_cost_num_params[num_via_cost_zone_instructions] = num_parameters;
        // printf("DEBUG:   Number of via cost-zone parameters is %d\n", 
        //                       user_inputs->via_cost_num_params[num_via_cost_zone_instructions]);
        num_via_cost_zone_instructions++; // Increment the number of via_cost_zone instructions
      }

      // If the user exceeded the allowed number of cost-zone instructions, then issue an
      // error and exit:
      if (  (num_trace_cost_zone_instructions >= maxCostZones) 
         || (  num_via_cost_zone_instructions >= maxCostZones))  {
        printf("\nERROR: More than the allowed number of '%s_cost_zone' statements were found in the\n", trace_or_via);
        printf("       input file. The allowed maximum is %d. Fix the input file and re-start the program.\n\n", maxCostZones);
        exit(1);
      }  // End of if-block for exceeding the allowed number of cost-zones

      continue;  // Skip to next line of input file.

    }  // End of if-block for matching "trace_cost_zone <cost index> <layer name> <shape type> ...." line
       //                           or   "via_cost_zone <cost index> <layer name> <shape type> ...." line
    else  {
      regfree(&regex);
    }


    //
    // Check for lines of the form "PIN_SWAP    LAYER SHAPE ..." 
    //                          or "NO_PIN_SWAP LAYER SHAPE ...:
    //
    compile_regex("^([NO_]*PIN_SWAP)[[:blank:]]+([^[:blank:]]+)[[:blank:]]+([^[:blank:]]+)(.*)$", &regex);
    //               (NO_)PIN_SWAP               <---Layer--->              <---Shape--->  <params>
    //
    if (regexec(&regex, line, 5, regex_match, 0) == 0)  {
      regfree(&regex);
      int num_parameters = 0; // number of parameters that follow the shape type in SWAP command

      // printf("DEBUG: Detected PIN_SWAP/NO_PIN_SWAP command in input file. num_swap_instructions=%d\n",
      //         num_swap_instructions);

      // Extract the command, routing layer name, and shape and save them in the
      // 'user_inputs' structure:
      len = (int)(regex_match[1].rm_eo - regex_match[1].rm_so);
      strncpy(user_inputs->swap_command[num_swap_instructions], line + regex_match[1].rm_so, len);
      user_inputs->swap_command[num_swap_instructions][len] = '\0';  // Terminate string with NULL character

      len = (int)(regex_match[2].rm_eo - regex_match[2].rm_so);
      strncpy(user_inputs->swap_layer[num_swap_instructions], line + regex_match[2].rm_so, len);
      user_inputs->swap_layer[num_swap_instructions][len] = '\0';  // Terminate string with NULL character

      len = (int)(regex_match[3].rm_eo - regex_match[3].rm_so);
      strncpy(user_inputs->swap_shape[num_swap_instructions], line + regex_match[3].rm_so, len);
      user_inputs->swap_shape[num_swap_instructions][len] = '\0';  // Terminate string with NULL character


      // printf("DEBUG: SWAP command/layer/shape is '%s/%s/%s'\n", user_inputs->swap_command[num_swap_instructions],
      //                                                     user_inputs->swap_layer[num_swap_instructions],
      //                                                     user_inputs->swap_shape[num_swap_instructions] );

      char *src = line + regex_match[4].rm_so; // Get beginning pointer of string containing parameters
      char *end = line + regex_match[4].rm_eo; // Get end pointer of string containing parameters
      size_t leading_space = strspn(src, whitespace); // Check for leading spaces
      // printf("DEBUG: leading_space is %d.\n", (int)leading_space);
      src = src + leading_space;  // Remove leading space from series of floating-point parameters 

      while (src < end)  {

        size_t len = strcspn(src, whitespace); // Find token up to next space character
        if (src + len > end)
          len = end - src;
        // printf("DEBUG: SWAP parameter: <<%.*s>>\n", (int)len, src);

        // Capture the floating point parameter in the user_inputs structure:
        memset(temp_line, '\0', sizeof(temp_line)); // Reset temp_line string
        strncpy(temp_line, src, (int)len);
        temp_line[len] = '\0';  // Terminate string with NULL character
        user_inputs->swap_parameters[num_swap_instructions][num_parameters] = strtof(temp_line, NULL);

        // printf("DEBUG:   Added parameter %8.2f to user_inputs->swap_parameters[%d][%d] structure.\n",
        //         user_inputs->swap_parameters[num_swap_instructions][num_parameters],
        //         num_swap_instructions, num_parameters );

        // Increment the number of parameters and confirm that it has not exceeded the max allowable:
        num_parameters++; // Increment the number of parameters
        if (num_parameters > maxPinSwapParameters)  {
          printf("\nERROR: Input file specifies more numeric parameters than allowed (%d) for command '%s %s %s'.\n\n",
             maxPinSwapParameters, user_inputs->swap_command[num_swap_instructions],
                                   user_inputs->swap_layer[num_swap_instructions],
                                   user_inputs->swap_shape[num_swap_instructions] );
          exit(1);
        }

        src += len;
        src += strspn(src, whitespace);
      }  // End of while-loop for (src < end)

      // Capture the number of parameters for this PIN_SWAP/NO_PIN_SWAPcommand:
      user_inputs->swap_num_params[num_swap_instructions] = num_parameters;
      // printf("DEBUG:   Number of PIN_SWAP/NO_PIN_SWAP parameters is %d\n", 
      //                         user_inputs->swap_num_params[num_swap_instructions]);

      num_swap_instructions++; // Increment the number of PIN_SWAP/NO_PIN_SWAP instructions
      // printf("DEBUG: num_swap_instructions incremented to %d\n", num_swap_instructions);

      continue;  // Skip to next line of input file

    }  // End of if-block for matching "(UN_)PIN_SWAP LAYER SHAPE..." line
    else  {
      regfree(&regex);
    }

  }  // End of while-loop for reading lines from input file

  // Capture the number of design-rule sets specified in the input file:
  if (design_rule_set > 0)  {
    user_inputs->numDesignRuleSets = design_rule_set;
    printf("\nINFO: Number of user-defined design-rule sets is %d\n", user_inputs->numDesignRuleSets);
  }  // End of if-block for design_rule_set > 0
  else {
    // If the input file contains no user-defined design-rule sets, then define a default 
    // design-rule set. In this set, all spaces, trace widths, and via diameters set to 
    // the equivalent of 1 cell:
    defineDefaultDesignRuleSet(user_inputs);
  }  // End of else-block (design_rule_set == 0)
  

  // Capture the number of BLOCK/UNBLOCK commands in the user_inputs structure:
  user_inputs->num_block_instructions = num_block_instructions;
  printf("\nINFO: Number of BLOCK/UNBLOCK instructions is %d\n", user_inputs->num_block_instructions);

  // Capture the number of DR_zone instructions in the user_inputs structure:
  user_inputs->num_DR_zones = num_DR_zone_instructions;
  printf("\nINFO: Number of DR_zone instructions is %d\n", user_inputs->num_DR_zones);
 
  // Capture the number of trace_cost_zone instructions and via_cost_zone instructions
  // in the user_inputs structure:
  user_inputs->num_trace_cost_zones = num_trace_cost_zone_instructions;
  user_inputs->num_via_cost_zones   = num_via_cost_zone_instructions;
  printf("\nINFO: Number of trace_cost_zone instructions is %d\n", user_inputs->num_trace_cost_zones);
  printf("\nINFO: Number of via_cost_zone instructions is %d\n", user_inputs->num_via_cost_zones);

  // Capture the number of trace_cost_multiplier and via_cost_multiplier statements
  // that were invoked in any trace_cost_zone and via_cost_zone statements:
  for (int i= 0; i < maxTraceCostMultipliers; i++)  {
    if (user_inputs->traceCostMultiplierInvoked[i])  {
      user_inputs->numTraceMultipliersInvoked++;
    }
  }  // End of for-loop for index i
  for (int i= 0; i < maxViaCostMultipliers; i++)  {
    if (user_inputs->viaCostMultiplierInvoked[i])  {
      user_inputs->numViaMultipliersInvoked++;
    }
  }  // End of for-loop for index i
  // printf("DEBUG: %d trace-cost multipliers are invoked in trace_cost_zone statements.\n", user_inputs->numTraceMultipliersInvoked);
  // printf("DEBUG: %d via-cost multipliers are invoked in via_cost_zone_statements.\n", user_inputs->numViaMultipliersInvoked);

  // Capture the number of PIN_SWAP/NO_PIN_SWAP commands in the user_inputs structure:
  user_inputs->num_swap_instructions = num_swap_instructions;
  printf("\nINFO: Number of PIN_SWAP/NO_PIN_SWAP instructions is %d\n", user_inputs->num_swap_instructions);

  // Close input file:
  fclose(fp);



  // If the vertCost parameter was not defined in the input file, then define it
  // as 25% of the square root of [map length (in microns)  X  map width (in microns)].
  // This implies that the autorouter will add a pair of vias to a trace to avoid
  // increasing the trace length by half the linear dimension of the map.
  if (! base_vert_cost_defined)  {   // vertCost was not defined by the user
    user_inputs->baseVertCostMicrons = sqrt(user_inputs->map_width_mm * 1000
                                    * user_inputs->map_height_mm * 1000)/4;
    printf("INFO: vertCost was not defined in input file, so we defined it as %'.1f microns.\n", user_inputs->baseVertCostMicrons);
  }  // End of if-block

  // Calculate vertCostCells parameter from vertCostMicrons variable. The conversion
  // factor is the number of cells per micron:
  user_inputs->baseVertCostCells = user_inputs->baseVertCostMicrons / user_inputs->cell_size_um;
  // printf("DEBUG: user_inputs->baseVertCostCells = %'lu in function 'parse_input_file'.\n", user_inputs->baseVertCostCells);

  // Calculate vertCost parameter from vertCostCells variable. The conversion
  // factor is the cost per cell:
  user_inputs->baseVertCost = (unsigned long) (user_inputs->baseVertCostCells * user_inputs->baseCellCost);
  // printf("DEBUG: user_inputs->baseVertCost = %'lu in function 'parse_input_file'.\n", user_inputs->baseVertCost);

  // Define the width and height of the map in units of cells:
  mapInfo->mapWidth  = 1000 * user_inputs->map_width_mm  / user_inputs->cell_size_um;
  mapInfo->mapHeight = 1000 * user_inputs->map_height_mm / user_inputs->cell_size_um;
  // printf("INFO: Map is %d cells wide by %d cells high.\n", mapInfo->mapWidth, mapInfo->mapHeight);

  // Define the diagonal distance of the map, which is used as an upper limit for any
  // routing restrictions:
  mapInfo->mapDiagonal = sqrt(mapInfo->mapWidth * mapInfo->mapWidth   +   mapInfo->mapHeight * mapInfo->mapHeight);

  // Issue a fatal error message if the map width or map height is larger than allowed
  // by the program:
  {
    int map_too_big_error = FALSE;

    // Check the X-direction:
    if (mapInfo->mapWidth > maxWidthCells)  {
      printf("\n\nERROR: The combination of map width (%6.3f mm) and resolution (%6.3f microns) results in too many\n",
             user_inputs->map_width_mm, user_inputs->cell_size_um);
      printf("       cells in the X-direction: %d cells. The maximum allowed is %d cells.\n", mapInfo->mapWidth, maxWidthCells);
      printf("       Modify the input file to reduce the size or increase the 'grid_resolution' value.\n\n");
      map_too_big_error = TRUE;
    }  // End of if-block for mapWidth> maxWidthCells

    // Check the Y-direction:
    if (mapInfo->mapHeight > maxHeightCells)  {
      printf("\n\nERROR: The combination of map height (%6.3f mm) and resolution (%6.3f microns) results in too many\n",
             user_inputs->map_height_mm, user_inputs->cell_size_um);
      printf("       cells in the Y-direction: %d cells. The maximum allowed is %d cells.\n", mapInfo->mapHeight, maxHeightCells);
      printf("       Modify the input file to reduce the size or increase the 'grid_resolution' value.\n\n");
      map_too_big_error = TRUE;
    }  // End of if-block for mapWidth> maxWidthCells


    // Exit the program if an error was found:
    if (map_too_big_error)  {
      exit(1);
    }
  }  // End of block for error-checking the size of the map


  // Calculate the "rat's nest" distance between the start- and end-terminal for each path,
  // and the average length of these rat's nest paths:
  //
  // Iterate through all user-defined nets to calculate the the "rat's nest" distance
  // between the start- and end-terminals, and the average of these distances:
  //
  float sum_length = 0.0;
  for (int path = 0; path < user_inputs->num_nets; path++)  {

    // Use the Pythagorean theorem to calculate the straight-line lateral distance between
    // the start- and end-terminals for each path: sqrt[ deltaX^2 + deltaY^2 ]
    user_inputs->rats_nest_length_um[path] = sqrt(    (user_inputs->end_X_um[path] - user_inputs->start_X_um[path])
                                                    * (user_inputs->end_X_um[path] - user_inputs->start_X_um[path])
                                                  +   (user_inputs->end_Y_um[path] - user_inputs->start_Y_um[path])
                                                    * (user_inputs->end_Y_um[path] - user_inputs->start_Y_um[path]) );

    // printf("DEBUG: Rat's nest length for net %d is %6.3f microns\n", path, user_inputs->rats_nest_length_um[path]);

    sum_length += user_inputs->rats_nest_length_um[path];
  }  // End of for-loop for index 'path'
  // Calculate the average rat's nest length:
  user_inputs->avg_rats_nest_length_um = sum_length / user_inputs->num_nets;
  // printf("\nDEBUG: Average length of rat's nest lines = %6.3f microns\n\n", user_inputs->avg_rats_nest_length_um);

  //
  // For each design-rule subset, convert the design-rule parameters to 'cell'
  // dimensions from microns. Also, compute useful parameters for each
  // design-rule set and subset that are derived from user-supplied values.
  //
  createUsefulDesignRuleInfo(mapInfo, user_inputs);

  //
  // For each net #i that is part of a differential pair, determine the number j of the net's
  // partner and save this in variable 'user_inputs->diffPairPartner[i] = j'.
  //
  // For each diff-pair net, also assign the pitch (in microns and cell units) for
  // each design-rule set.
  //
  getDiffPairPartnerAndPitch(user_inputs);

  //
  // Verify that the diff-pair pitch for a net is equal to the diff-pair pitch for
  // that net's partner net on each layer:
  //
  verifyDiffPairPitch(user_inputs);

  // Check that the number of layer names listed on the 'layer_names' line is consistent
  // with the number of routing layers specified on the 'number_layers' line:
  if ((user_inputs->num_routing_layers * 2 - 1) != num_named_layers)  {
    printf("\nERROR: The number of layer names (%d) in the intput file is inconsistent with,\n",
                   num_named_layers);
    printf("       the number of routing layers specified in the 'number_layers' statement (%d).\n\n",
                   user_inputs->num_routing_layers);
    exit(1);
  }


  // Assign a layer number (starting with zero) for each routing layer:
  // 'numRoutingLayers' is the number of routing layers, excluding via layers
  for (int i = 0; i < user_inputs->num_routing_layers; i++)  {
    strcpy(user_inputs->routingLayerNames[i], user_inputs->layer_names[2*i]);
    // printf("DEBUG: Routing layer '%s' is mapped to layer number %d.\n",
    //             user_inputs->layer_names[2*i], i);
  }  // End of for-loop

  //
  // Convert the starting and ending (x,y) coordinates from microns to cell units, and calculate
  // the Z-coordinates based on the names of the starting- and ending layer names.
  //
  calc_XYZ_cell_coordinates(user_inputs, mapInfo);


  //
  // Map the user-defined diff-pair nets to pseudo nets, storing the results in array:
  //
  //         user_inputs->diffPairToPseudoNetMap[net_number] = pseudo_net_number
  //
  // Also, map the pseudo nets back to the user-defined diff-pair nets, storing the
  // results in the following two arrays:
  //
  //         user_inputs->pseudoNetToDiffPair_1[pseudo_net_number] = diff_pair_net_1
  //         user_inputs->pseudoNetToDiffPair_2[pseudo_net_number] = diff_pair_net_2
  //
  mapPseudoNets(user_inputs);


  // Confirm that start- and end-locations are within the map. Also calculate the
  // pitch (in cell units) of the start- and end-terminals of differential
  // pairs.
  checkTerminalLocations(user_inputs, mapInfo);


  //
  // Create 2-dimensional mapping structure 'user_inputs->designRuleSubsetMap' that maps
  // net numbers and design-rule sets to the correct design-rule subset:
  //
  //   user_inputs->designRuleSubsetMap[net_num][DR_set_num] = DR_subset_num
  mapDesignRuleSubsets(user_inputs);


  //
  // Verify that design-rule exceptions that contain the 'diff_pair_pitch' keyword are not
  // used for nets that don't contain a diff-pair partner net.
  //
  verify_net_designRule_consistency(user_inputs);

}  // End of function 'parse_input_file'

