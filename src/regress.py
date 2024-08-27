#!/usr/bin/python3

import subprocess
import os
import shutil     # Used for shell utilities like 'copy2' for copying files
import re         # Regular Expressions
import argparse   # Module that enables parsing of command-line arguments
import multiprocessing  # Module to allow this script to determine the number of available CPUs

#
# Parse optional command-line arguments, consisting of:
#   -t  N           <<== Specify the maximum number of threads N for the Acorn executable to use. Default is number of available CPUs.
#   -f  <fileName>  <<== Specify full path and file name containing list of test-cases to execute. Default is 'regression_test_list.txt'
#   -d  <dirPath>   <<== Specify directory that contains test-case files.
#   -s  <sourcPath> <<== Specify directory that contains Acorn source code. Default is current working directory.
#   -o  <ouputPath> <<== Specify directory for writing output files. Default is current working directory.


#
parser = argparse.ArgumentParser(description='Acorn regression testing')
parser.add_argument("-t", default=multiprocessing.cpu_count(),  type=int, help="Maximum number of threads for acorn executable to use. Default is number of CPUs in system.")
parser.add_argument("-f", default='', type=str, help="Full path and name of file containing list of test-cases to run. Default is regression_test_list.txt")
parser.add_argument("-d", default='', type=str, help="Directory path containing test-cases to run.")
parser.add_argument("-s", default=os.getcwd(), type=str, help="Directory path containing Acorn source code. Default is current directory.")
parser.add_argument("-o", default=os.getcwd(), type=str, help="Directory path for writing output files. Default is current directory.")
args = parser.parse_args()

# Define the maximum number of threads passed from the command-line. This value will be used
# below when launching 'acorn.exe':
max_threads = args.t

# Define directory where source-code resides:
source_code_dir = args.s

# Define directory to write output files:
output_dir = args.o

# If user did not specify a location of regression tests, then check common locations:
regression_dir = args.d
if (regression_dir == ''):
  if (os.path.isdir("/home/danbo/autorouter/acorn/regression_tests")):
    regression_dir = "/home/danbo/autorouter/acorn/regression_tests"
  elif (os.path.isdir("/home/Geek/AutoRouter/regression_tests")):
    regression_dir = "/home/Geek/AutoRouter/regression_tests"
  elif (os.path.isdir("/home/ec2-user/acorn/regression_tests")):
    regression_dir = "/home/ec2-user/acorn/regression_tests"
  elif (os.path.isdir("/home/ec2-user/acorn/ACORN_TESTS")):
    regression_dir = "/home/ec2-user/acorn/ACORN_TESTS"
  else:
    print("Error: Location of regression tests was not found.")
    exit(1)

# Define the name of the list of test-cases to run -- either extracted from the command line (with -f switch)
# or the default value:
if (args.f == ''):
  test_list_file_name = regression_dir + "/regression_test_list.txt"
else:
  test_list_file_name = args.f 


# Capture current directory:
start_directory = os.getcwd()
print("INFO: Started in directory " + start_directory + ".\n\n")

# Compile source code by running 'make' in directory that contains source code:
print("Running 'make' in directory '" + source_code_dir + "'...")
os.chdir(source_code_dir)
make_output = subprocess.call('make')

if (make_output):
  print("\nERROR: 'make' exited with non-zero return-code of " + str(make_output) + ".")
  exit(1)

# Check that executable file 'acorn.exe' exists. If not, issue a message and exit:
if (not(os.path.isfile("acorn.exe"))):
  print("\nERROR: Executable file 'acorn.exe' does not exist after running 'make' in directory")
  print("       '" + start_directory + "'.")
  exit(1)

print("INFO: 'make' exited with return-code of zero...\n\n\n")

print("INFO: Accessing regression suite in directory " + regression_dir + "...")

test_list_file = open(test_list_file_name, mode="r")

test_names = []  # Array 'test_names' is array of strings -- one for each regression test

for line in test_list_file:
  # print "DEBUG: " + line
  line = line.strip()  # Strip leading and trailing whitespace
  line = line.strip('\r\n')  # Strip trailing carriage return and line-feed
  if (line != "" and line[0] != "#"): 
    test_names.append(line)
    print("Test '" + test_names[len(test_names)-1] + "' will be performed.")

  else:
    # print "INFO: Found comment line!"
    pass

test_list_file.close()

test_count = len(test_names)

# Before launching the regression tests, confirm that each of the required test-case
# files exists:
for test_name in (test_names):
  test_input_filename = regression_dir + "/" + test_name + ".txt"
  
  # Check if input file exists for this test. If not, then issue error message and exit:
  if (not(os.path.isfile(test_input_filename))):
    print("\nERROR: The file '" + test_input_filename + "'")
    print("       does not exist, even though it was specified as the input file for test")
    print("       '" + test_name + "' in the list of tests in file")
    print("       '" + test_list_file_name + "'.\n")
    exit(1)

  # For the current test, check whether a sub-directory already exists in the
  # output directory. If so, then issue error message and exit:
  if (os.path.exists(output_dir + "/" + test_name)):
    print("\nERROR: The directory '" + test_name + "'")
    print("       already exists in directory")
    print("       '" + output_dir + "'.")
    print("       Check the following file for duplicate test names:")
    print("       '" + test_list_file_name + "'.")
    exit(1)

#
# Open an HTML file for writing that will contain a tabulated summary of the 
# regression test results:
#
try:
  summary_file = open(output_dir + "/regression.html", "w")
except:
  print("\nERROR: Cannot open HTML file '" + output_dir + "/regression.html'")
  print("       for writing.")
  exit(1)

#
# Write HTML header information and first row of table:
#
summary_file.write("<HTML>\n<HEAD>\n  <TITLE>Regression Results</TITLE></HEAD>\n")
# Boyne commented out the following line 21 June 2018 to eliminate auto-refresh:
# summary_file.write("  <meta http-equiv=\"refresh\" content=\"15\"></HEAD>\n")
summary_file.write("<BODY><H1>Autorouter Regression Test Results</H1>\n\n")
summary_file.write("<BODY><H3>" + str(test_count) + " tests using " + str(max_threads) + " thread(s):</H2>\n\n")
summary_file.write("<TABLE border=\"1\" cellpadding=\"1\">\n")
summary_file.write("  <TR>\n")
summary_file.write("    <TD><B><U>Test Name</U></B><FONT size=\"1\"><UL><LI>Run time<LI>Explored Cells</UL></FONT></TD>\n")
summary_file.write("    <TH><U>Width</U><BR><FONT size=\"1\">(mm)<BR>(cells)</FONT></TH>\n")
summary_file.write("    <TH><U>Height</U><BR><FONT size=\"1\">(mm)<BR>(cells)</FONT></TH>\n")
summary_file.write("    <TH><FONT size=\"2\">Routing<BR>Layers</FONT></TH>\n")
summary_file.write("    <TH><FONT size=\"2\">Solved<BR>?</FONT><BR><FONT size=\"1\">(DRC-<BR>free<BR>iter)</FONT></TH>\n")
summary_file.write("    <TH><U>Initial Metrics</U><BR><FONT size=\"1\">Length (mm)<BR>Via count<BR>DRC cells</FONT></TH>\n")
summary_file.write("    <TH><U>Final Metrics</U><BR><FONT size=\"1\">Length (mm)<BR>Via count<BR>DRC cells</FONT></TH>\n")
summary_file.write("    <TH><U>Best Metrics</U><BR><FONT size=\"1\">Length (mm)<BR>Via count<BR>DRC cells</FONT></TH>\n")
summary_file.write("    <TH>Last<BR>Iter.</TH>\n")
summary_file.write("    <TH>Best<BR>Iter.</TH>\n")
summary_file.write("    <TH>Initial Map</TH>\n")
summary_file.write("    <TH>Final Map</TH>\n")
summary_file.write("    <TH>Best Map</TH>\n")
summary_file.write("    <TH>Parameters<BR><FONT size=\"1\">(User-specified<BR>&nbsp;&nbsp;&nbsp;&nbsp;in parentheses)</FONT></TH>\n")
summary_file.write("  </TR>\n")


print("\n\nINFO: Starting suite of " + str(test_count) + " tests...")

#
# Cycle through each test in the list of tests and run them serially:
#
test_number = 0
for test_name in (test_names):
  test_number = test_number + 1
  print("\nTest " + str(test_number) + "/" + str(test_count) + ": " + test_name + "...")
  working_dir_name = output_dir + "/" + test_name

  test_input_filename = regression_dir + "/" + test_name + ".txt"

  # Check if input file exists for this test. If not, then issue error message and exit:
  if (not(os.path.isfile(test_input_filename))):
    print("\nERROR: The file '" + test_input_filename + "'")
    print("       does not exist, even though it was specified as the input file for test")
    print("       '" + test_name + "' in the list of tests in file")
    print("       '" + test_list_file_name + "'.\n")
    exit(1)
  

  # For the current test, check whether a sub-directory already exists in the
  # output directory. If so, then issue error message and exit:
  if (os.path.exists(output_dir + "/" + test_name)):
    print("\nERROR: The directory '" + test_name + "'")
    print("       already exists in directory")
    print("       '" + output_dir + "'.")
    print("       Check the following file for duplicate test names:")
    print("       '" + test_list_file_name + "'.")
    exit(1)

  # Make a sub-directory in the output directory:
  try:
    os.mkdir(working_dir_name)
  except:
    print("\nERROR: Unable to create sub-directory")
    print("       '" + test_name + "'")
    print("       in directory '" + output_dir + "'.")
    exit(1)

  # Copy the input file for this test to the working directory:
  try:
    shutil.copy2(test_input_filename, working_dir_name)
  except:
    print("\nERROR: Unable to copy the input file:")
    print("       '" + test_input_filename + "'")
    print("       to the newly created sub-directory:")
    print("       '" + working_dir_name + "'.")
    exit(1)

  # Re-define the test_input_filename variable to point to the copy in the
  # current working directory:
  test_input_filename = working_dir_name + "/" + test_name + ".txt"

  # Change to the newly created directory for the current test:
  try:
    os.chdir(working_dir_name)
  except:
    print("\nERROR: Unable to change to the newly created sub-directory")
    print("       '" + test_name + "'")
    print("       in directory '" + output_dir + "'.")
    exit(1)

  # In the newly created directory for the current test, create a symbolic
  # link to the 'acorn.exe' executable in the source-code directory:
  try:
    os.symlink(source_code_dir + "/acorn.exe", "acorn.exe")
  except:
    print("\nERROR: Unable to create a symbolic link to 'acorn.exe' in the newly created sub-directory")
    print("       '" + test_name + "', with the source 'acorn.exe' located in")
    print("       directory '" + source_code_dir + "'.")
    exit(1)

  # Create a log file to which the auto-router will write STDOUT and STDERR:
  try:
    log_file = open(working_dir_name + "/" + "logfile.txt", mode="w")
  except:
    print("\nERROR: Unable to create a write-able 'logfile.txt' file in the newly created sub-directory")
    print("       '" + test_name + "'.")
    exit(1)

  #
  # Execute the auto-router ('acorn.exe') using the input file for the current test. If user specified on the
  # command-line a maximum number of threads to use, then invoke acorn.exe with the -t switch to specify the
  # maximum number of threads to use:
  #
  if (max_threads):
    print("INFO: Launching autorouter using 'acorn.exe -t " + str(max_threads) + " " + test_input_filename + "'...")
    router_output = subprocess.call(["./acorn.exe", "-t", str(max_threads), test_input_filename], stdout=log_file, stderr=subprocess.STDOUT, shell=False)
  else:
    print("INFO: Launching autorouter using 'acorn.exe " + test_input_filename + "'...")
    router_output = subprocess.call(["./acorn.exe", test_input_filename], stdout=log_file, stderr=subprocess.STDOUT, shell=False)

  # Close the log file:
  log_file.close()

  # Re-open the log file, read its contents, and close the file:
  log_file = open(working_dir_name + "/" + "logfile.txt", mode="r")
  log_file_contents = log_file.read()
  log_file.close()
  
  # Open the input file, read its contents, and close the file:
  input_file = open(test_input_filename, mode="r")
  input_file_contents = input_file.read()
  input_file.close()

  #
  # From the input file, extract key pieces of information. For some parameters,
  # extract them from the log file of the auto-router:
  #

  # Extract the number of routing layers from input file (e.g., 'number_layers = 4')
  num_routing_layers = (re.search(r'^\s*number_layers\s*=\s*(\d+)', input_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
  # print "DEBUG: Found number of layers: " + str(num_routing_layers)


  # Extract the grid width and height in millimeters from input file. Examples:
  #        width  = 0.8  // in millimeters
  #        height = 0.5  // in millimeters
  width_in_mm = (re.search(r'^\s*width\s*=\s*(\d*\.*\d*)', input_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
  height_in_mm = (re.search(r'^\s*height\s*=\s*(\d*\.*\d*)', input_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
  # print "DEBUG: Found width in mm: " + str(width_in_mm)
  # print "DEBUG: Found height in mm: " + str(height_in_mm)

  # Extract the grid resolutions (in microns) from the input file:
  resolution_in_microns = (re.search(r'^\s*grid_resolution\s*=\s*(\d*\.*\d*)', input_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
  # print "DEBUG: Found grid resolutions in microns: " + str(resolution_in_microns)

  # Calculate the grid width and height in cells
  width_in_cells  = int(1000 * float(width_in_mm)  / float(resolution_in_microns))
  height_in_cells = int(1000 * float(height_in_mm) / float(resolution_in_microns))
  # print "DEBUG: Map is " + str(width_in_cells) + " cells wide by " + str(height_in_cells) + " cells high."

  # Extract the optional 'vertCost' parameter value, if the user specified it in the
  # input file (e.g., 'vertCost  =   37'). If it's not found, then extract the default
  # value from the autorouter's log file:
  try:
    vertCostMicrons = (re.search(r'^\s*vertCost\s*=\s*(\d+)', input_file_contents,
                             re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
    # Surround the value by parentheses to denote that it was user-specified:
    vertCostMicrons = "(" + str(vertCostMicrons) + ")"
    # print "DEBUG: Found vertCost: " + str(vertCostMicrons)
  except:
    try: # Parameter was not found in input file, so search the log file:
      vertCostMicrons = (re.search(r'^INFO: vertCost was not defined in input file, so we defined it as (\S+) microns', log_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
      # print "DEBUG: Found vertCostMicrons: " + str(vertCostMicrons)
    except:
      vertCostMicrons = 'Not Found'
      # print "DEBUG: 'vertCost' parameter not found in input file, and 'vertCostMicrons not found in the log file."


  # Extract the optional 'violationFreeThreshold' parameter value, if the user specified it
  # in the input file (e.g., 'violationFreeThreshold =  10'). If it's not found, then extract 
  # the default value from the autorouter's log file:
  try:
    userDRCfreeThreshold = (re.search(r'^\s*violationFreeThreshold\s*=\s*(\d+)', input_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
    # Surround the value by parentheses to denote that it was user-specified:
    userDRCfreeThreshold = "(" + str(userDRCfreeThreshold) + ")"
    # print "DEBUG: Found violationFreeThreshold: " + str(userDRCfreeThreshold)
  except:
    try: # Parameter was not found in input file, so search the log file:
      userDRCfreeThreshold = (re.search(r'^\s*userDRCfreeThreshold\s*=\s*(\d+)', log_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
      # print "DEBUG: Found violationFreeThreshold: " + str(userDRCfreeThreshold)
    except:
      userDRCfreeThreshold = 'Not Found'
      # print "DEBUG: 'violationFreeThreshold' parameter not found in input file or log file."


  # Extract the optional 'maxIterations' parameter value, if the user specified it in the
  # input file (e.g., 'maxIterations = 1000'). If it's not found, then extract the default
  # value from the autorouter's log file:
  try:
    maxIterations = (re.search(r'^\s*maxIterations\s*=\s*(\d+)', input_file_contents,
                             re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
    # Surround the value by parentheses to denote that it was user-specified:
    maxIterations = "(" + str(maxIterations) + ")"
    # print "DEBUG: Found maxIterations: " + str(maxIterations)
  except:
    try: # Parameter was not found in input file, so search the log file:
      maxIterations = (re.search(r'^\s*maxIterations\s*=\s*(\d+)', log_file_contents,
                               re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
      # print "DEBUG: Found maxIterations: " + str(maxIterations)
    except:
      maxIterations = 'Not Found'
      # print "DEBUG: 'maxIterations' parameter not found in input file or log file."


  #
  # From the output log file's contents, extract key attributes:
  #

  # Extract whether a solution was found (e.g., 'INFO: Solution was found.')
  try:
    solution_found = (re.search(r'^INFO: Solution was found in ', log_file_contents,
                                 re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(0)
    solution_found = 1
    solution_found_text = 'Yes'
  except:
    solution_found = 0
    solution_found_text = '&nbsp;No&nbsp;'
  # print "DEBUG: Was solution found?..." + str(solution_found_text)


  # Extract whether the user defined cost-zone multipliers in the map:
  try:
    cost_multipliers_used = (re.search(r'there are cost multipliers used in the map', log_file_contents,
                                 re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(0)
    cost_multipliers_used = True
  except:
    cost_multipliers_used = False
  # print "DEBUG: Cost-multipliers used?..." + str(cost_multipliers_used)


  # Extract the number of DRC-free iterations that Acorn requires for a
  # successful exit:
  try:
    acornDRCfreeThreshold = (re.search(r'^INFO: Program requires at least (\d+) DRC-free solutions before it terminates', log_file_contents,
                                 re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
    # print "DEBUG: acornDRCfreeThreshold = '" + str(acornDRCfreeThreshold) + "'"
  except:
    acornDRCfreeThreshold = '??'
    # print "DEBUG: acornDRCfreeThreshold = '" + str(acornDRCfreeThreshold) + "'"


  # Extract the number of DRC-free iterations that Acorn achieved:
  try:
    DRCfreeIterations = (re.search(r'^INFO: (\d+) DRC-free iterations were found ', log_file_contents,
                                 re.IGNORECASE | re.DOTALL | re.MULTILINE)).group(1)
    # print "DEBUG: DRCfreeIterations = '" + str(DRCfreeIterations) + "'"
  except:
    DRCfreeIterations = '??'
    # print "DEBUG: DRCfreeIterations = '" + str(DRCfreeIterations) + "'"


  #
  # For the initial iteration, extract the aggregate path length and via count from
  # the log file:
  #
  try: 
    initial_metrics = re.search(r'^\s+User-defined nets:\s+([\d\.,]+) mm,\s*([\d\.,]+) vias,\s*[\d\.,]+ cells with DRCs \([\d\.,]+ / 2\)\s*$',
                                 log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
    total_initial_length   = initial_metrics.group(1)
    initial_via_count      = initial_metrics.group(2)
  except:
    try:
      initial_metrics = re.search(r'^\s+All nets:\s+([\d\.,]+) mm,\s*([\d\.,]+) vias,\s*[\d\.,]+ cells with DRCs \([\d\.,]+ / 2\)\s*$',
                                   log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
      total_initial_length   = initial_metrics.group(1)
      initial_via_count      = initial_metrics.group(2)
    except:
      total_initial_length   = "??"
      initial_via_count      = "??"      

  # Extract the number of cells with DRC violations for the initial iteration from the log file:
  try:
    initial_DRC_count = re.search(r'^\s+Number of cells with non-pseudo-DRC violations: ([\d\.,]+)\s*$',
                               log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE).group(1)
  except:
    try:
      initial_DRC_count = re.search(r'^\s+Number of cells with DRC violations: ([\d\.,]+)\s*$',
                                 log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE).group(1)
    except:
      initial_DRC_count = "??"

  # print "DEBUG: total initial length = " + str(total_initial_length)
  # print "DEBUG: initial via count = " + str(initial_via_count)
  # print "DEBUG: initial DRC count = " + str(initial_DRC_count)

  #
  # For the final iteration, extract the aggregate path length, via count, and DRC count:
  #

  # To determine the number of iterations, extract the iterations from the log 
  # file, e.g. 'INFO: Starting iteration number 12...'
  try:
    iterations = re.findall(r'^INFO: Starting iteration number (\d+)\.\.\.\s*$',
                            log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
    num_iterations = len(iterations)
  except:
    num_iterations = "??"
  # print "DEBUG: " + str(num_iterations) + " iterations were found."
  # print "DEBUG: " + iterations[num_iterations - 1]
    
  #
  # For the final iteration, extract the aggregate path length and via count from the
  # log file:
  #
  try:
    final_metrics = re.search(r"^INFO: Starting iteration number " + str(num_iterations) 
                              + "\.\.\..*?\s+User-defined nets:\s+([\d\.,]+) mm,\s*([\d\.,]+) vias,\s*[\d\.,]+ cells with DRCs \([\d\.,]+ / 2\)\s*$",
                                 log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
    total_final_length   = final_metrics.group(1)
    final_via_count      = final_metrics.group(2)
  except:
    try:
      final_metrics = re.search(r"^INFO: Starting iteration number " + str(num_iterations) 
                                + "\.\.\..*?\s+All nets:\s+([\d\.,]+) mm,\s*([\d\.,]+) vias,\s*[\d\.,]+ cells with DRCs \([\d\.,]+ / 2\)\s*$",
                                   log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
      total_final_length   = final_metrics.group(1)
      final_via_count      = final_metrics.group(2)
    except:      
      total_final_length   = "??"
      final_via_count      = "??"

  # Extract the number of DRCs for the final iteration from the log file:
  try:
    final_DRC_count = re.search(r"^INFO: Starting iteration number " + str(num_iterations)
                                     + "\.\.\..*?\s+Number of cells with non-pseudo-DRC violations: ([\d\.,]+)\s*$",
                                     log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE).group(1)
  except:
    try:
      final_DRC_count = re.search(r"^INFO: Starting iteration number " + str(num_iterations)
                                       + "\.\.\..*?\s+Number of cells with DRC violations: ([\d\.,]+)\s*$",
                                       log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE).group(1)
    except:    
      final_DRC_count = "??"


  # Extract the total elapsed time from the log file, as well as the number of explored cells and best iteration:
  try:
    time_and_cell_metrics = re.search(r"^INFO: Solution was found in ([\d\.,]+) seconds with ([\d\.,]+) cells explored\. The lowest-cost routing results are in iteration (\d+)\.$",
                                      log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
    elapsed_time   = time_and_cell_metrics.group(1)
    explored_cells = time_and_cell_metrics.group(2)
    best_iteration = int(time_and_cell_metrics.group(3))
    # print "\nDEBUG: The best_iteration is " + str(best_iteration) + "\n"
  except:
    try:
      time_and_cell_metrics = re.search(r"^ERROR: No solution was found after reaching the maximum number of iterations .* after ([\d\.,]+) seconds, exploring ([\d\.,]+) cells\..*The iteration with the lowest-cost routing results is iteration (\d+)\.$",
                                        log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
      elapsed_time   = time_and_cell_metrics.group(1)
      explored_cells = time_and_cell_metrics.group(2)
      best_iteration = int(time_and_cell_metrics.group(3))
      # print "\nDEBUG: The best_iteration is " + str(best_iteration) + "\n"
    except:
      elapsed_time   = "??"
      explored_cells = "??"
      best_iteration = "??"        
  
  
  
  #
  # For the best iteration, extract the aggregate path length and via count from the
  # log file:
  #
  try:
    best_metrics = re.search(r"^INFO: Starting iteration number " + str(best_iteration) 
                              + "\.\.\..*?\s+User-defined nets:\s+([\d\.,]+) mm,\s*([\d\.,]+) vias,\s*[\d\.,]+ cells with DRCs \([\d\.,]+ / 2\)\s*$",
                                 log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
    total_best_length   = best_metrics.group(1)
    best_via_count      = best_metrics.group(2)
    # print "DEBUG: total_best_length = '" + str(total_best_length) + "'"
    # print "DEBUG: best_via_count = '" + str(best_via_count) + "'"
  except:
    try:
      best_metrics = re.search(r"^INFO: Starting iteration number " + str(best_iteration) 
                                + "\.\.\..*?\s+All nets:\s+([\d\.,]+) mm,\s*([\d\.,]+) vias,\s*[\d\.,]+ cells with DRCs \([\d\.,]+ / 2\)\s*$",
                                   log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE)
      total_best_length   = best_metrics.group(1)
      best_via_count      = best_metrics.group(2)
      # print "DEBUG: total_best_length = '" + str(total_best_length) + "'"
      # print "DEBUG: best_via_count = '" + str(best_via_count) + "'"
    except:      
      total_best_length   = "??"
      best_via_count      = "??"
      # print "DEBUG: total_best_length = '" + str(total_best_length) + "'"
      # print "DEBUG: best_via_count = '" + str(best_via_count) + "'"

  # Extract the number of DRCs for the best iteration from the log file:
  try:
    best_DRC_count = re.search(r"^INFO: Starting iteration number " + str(best_iteration)
                                     + "\.\.\..*?\s+Number of cells with non-pseudo-DRC violations: ([\d\.,]+)\s*$",
                                     log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE).group(1)
    # print "DEBUG: best_DRC_count = '" + str(best_DRC_count) + "'"
  except:
    try:
      best_DRC_count = re.search(r"^INFO: Starting iteration number " + str(best_iteration)
                                       + "\.\.\..*?\s+Number of cells with DRC violations: ([\d\.,]+)\s*$",
                                       log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE).group(1)
      # print "DEBUG: best_DRC_count = '" + str(best_DRC_count) + "'"
    except:    
      best_DRC_count = "??"
      # print "DEBUG: best_DRC_count = '" + str(best_DRC_count) + "'"
  
  
  # Extract the number of threads from the log file:
  try:
    num_threads = re.search(r"^INFO: Number of threads is ([\d\.,]+)\.$",
                                      log_file_contents, re.IGNORECASE | re.DOTALL | re.MULTILINE).group(1)
  except:
    num_threads = "??"
  
  # print "DEBUG: total final length = " + str(total_final_length)
  # print "DEBUG: final via count = " + str(final_via_count)
  # print "DEBUG: final DRC count = " + str(final_DRC_count)

  # 
  # From the output directory for this test, extract the names of the composite .png
  # graphics files for (a) the initial iteration, (b) the final iteration, and
  # (c) the best iteration:
  #
  # File name of composite PNG file for initial iteration:
  initial_PNG_file = "map_composite_iter" + str('{:04d}'.format(1)) + ".png"
  print("DEBUG:   initial_PNG_file PNG file: '" + initial_PNG_file + "'.")
  
  # File name of composite PNG file for final iteration:
  final_PNG_file = "map_composite_iter" + str('{:04d}'.format(num_iterations)) + ".png"
  print("DEBUG:   final_PNG_file PNG file: '" + final_PNG_file + "'.")
  
  # File name of composite PNG file for best iteration:
  best_PNG_file = "map_composite_iter" + str('{:04d}'.format(best_iteration)) + ".png"
  print("DEBUG:   best_PNG_file PNG file: '" + best_PNG_file + "'.")
  

  #
  # Write a summary of the regression test results to the HTML summary file:
  #
  summary_file.write("  <TR>\n")
  summary_file.write("    <TD><FONT size=\"1\">Test " + str(test_number) + "/" + str(test_count) + ":</FONT><BR><BR><FONT size=\"2\"><A href=\"" + str(test_name) 
                            + "/routingStatus.html\">" + test_name + "</A></FONT>\n")
  summary_file.write("        <BR><FONT size=\"1\"><UL><LI>" + str(elapsed_time) + " seconds<BR><BR>\n")
  summary_file.write("        <LI>" + str(explored_cells) + " explored cells</UL></FONT>")
  if (test_number < test_count):
    summary_file.write("<BR>\n")
    summary_file.write("        <FONT size=\"1\" color=\"grey\"><A href=\"" + str(test_names[test_number]) + "/routingStatus.html\">Next test</A></FONT></TD>\n")
  else:
    summary_file.write("</TD>\n")
  summary_file.write("    <TD align=\"center\"><FONT size=\"2\">" + str(width_in_mm) + " mm<BR><BR>" 
                                                        + str(width_in_cells) + " cells</FONT></TD>\n")
  summary_file.write("    <TD align=\"center\"><FONT size=\"2\">" + str(height_in_mm) + " mm<BR><BR>" 
                                                        + str(height_in_cells) + " cells</FONT></TD>\n")
  summary_file.write("    <TD align=\"center\">" + str(num_routing_layers) + "</TD>\n")

  if (solution_found == 0):  # If solution was not found, then highight with red background color:
    summary_file.write("    <TD bgcolor=\"#FF0000\" align=\"center\">" + str(solution_found_text) + "<BR><BR><FONT size=\"1\">(" 
                       + str(DRCfreeIterations) + "&nbsp;&nbsp;<BR>&nbsp;&nbsp;/" + str(acornDRCfreeThreshold) + ")</FONT></TD>\n")
  else:
    summary_file.write("    <TD align=\"center\">" + str(solution_found_text) + "<BR><BR><FONT size=\"1\">(" 
                       + str(DRCfreeIterations) + "&nbsp;&nbsp;<BR>&nbsp;&nbsp;/" + str(acornDRCfreeThreshold) + ")</FONT></TD>\n")

  # Print metrics from iteration #1:
  if (cost_multipliers_used):
    summary_file.write("    <TD align=\"center\"><FONT size=\"1\"><I><U>Initial</U></I><BR></FONT><FONT size=\"2\">" + str(total_initial_length) 
                               + " mm<BR><FONT size=\"1\">(Disregarding<BR>cost-<BR>zones)</FONT><BR><BR>" 
                               + str(initial_via_count) + " vias<BR><BR>" + str(initial_DRC_count) 
                               + " X's<BR></FONT></TD>\n")
  else:
    summary_file.write("    <TD align=\"center\"><FONT size=\"1\"><I><U>Initial</U></I><BR></FONT><FONT size=\"2\">" + str(total_initial_length) 
                               + " mm<BR><BR>" + str(initial_via_count) + " vias<BR><BR>" + str(initial_DRC_count) + " X's</FONT></TD>\n")            
  
  # Print metrics from the final iteration:
  summary_file.write("    <TD align=\"center\"><FONT size=\"1\"><I><U>Final</U></I><BR></FONT><FONT size=\"2\">" + str(total_final_length)  
                               + " mm<BR><BR>" + str(final_via_count) + " vias<BR><BR>" + str(final_DRC_count) + " X's</FONT></TD>\n")
  
  # Print metrics from the best iteration:
  summary_file.write("    <TD align=\"center\"><FONT size=\"1\"><I><U>Best</U></I><BR></FONT><FONT size=\"2\">" + str(total_best_length) 
                               + " mm<BR><BR>" + str(best_via_count) + " vias<BR><BR>" + str(best_DRC_count) + " X's</FONT></TD>\n")

  # If final iteration and best iteration both had DRCs, then highlight the final iteration count with red background color:
  if ((final_DRC_count != '0') and (best_DRC_count != '0')):  
    summary_file.write("    <TD bgcolor=\"#FF0000\" align=\"center\"><FONT size=\"1\"><I>Final<BR><U>Iter</U></I><BR></FONT>"
                        + str(num_iterations) + "</TD>\n")
  else:
    # If final iteration had DRCs, but best iteration had none, then highight the final iteration count with yellow background color:
    if (final_DRC_count != '0'):  
      summary_file.write("    <TD bgcolor=\"#FFFF00\" align=\"center\"><FONT size=\"1\"><I>Final><BR><U>Iter</U></I><BR></FONT>"
                          + str(num_iterations) + "</TD>\n")
    else:      
      # If final iteration had no DRCs, then add no highlighting to the final iteration count:
      summary_file.write("    <TD align=\"center\"><FONT size=\"1\"><I>Final<BR><U>Iter</U></I><BR></FONT>"
                          + str(num_iterations) + "</TD>\n")
    
  if (best_DRC_count != '0'):  # If best iteration had DRCs, then highight with red background color:
    summary_file.write("    <TD bgcolor=\"#FF0000\" align=\"center\"><FONT size=\"1\"><I>Best<BR><U>Iter</U></I><BR></FONT>"
                        + str(best_iteration) + "</TD>\n")
  else:
    summary_file.write("    <TD align=\"center\"><FONT size=\"1\"><I>Best<BR><U>Iter</U></I><BR></FONT>" + str(best_iteration) + "</TD>\n")

  # Calculate a scaling factor for the PNG image files so their maximum dimension is 200 pixels:
  max_pixels = float(200)
  # print "DEBUG: width_in_cells is", width_in_cells
  # print "DEBUG: height_in_cells is", height_in_cells
  # print "DEBUG: max_pixels is", max_pixels
  if (width_in_cells > height_in_cells):
    scale_factor = max_pixels / width_in_cells
  else:
    scale_factor = max_pixels / height_in_cells
  # print "DEBUG: scale_factor is", scale_factor
  thumbnail_X = int(scale_factor * width_in_cells)
  thumbnail_Y = int(scale_factor * height_in_cells)
  # print "DEBUG: thumbnail_X is", thumbnail_X
  # print "DEBUG: thumbnail_Y is", thumbnail_Y

  #
  # Display the composite PNG image from the initial iteration:
  #
  try:
    summary_file.write("    <TD><A href=\"" + test_name + "/iteration0001.html\">\n")
    summary_file.write("      <FONT size=\"1\"><I><U>Initial</U></I><BR></FONT>\n")
    summary_file.write("      <IMG border=\"1\" src=\"" + test_name + "/" + initial_PNG_file + "\" \n")
    summary_file.write("        width=\"" + str(thumbnail_X) + "\" height=\"" + str(thumbnail_Y) + "\"></A></TD>\n")
  except:
    summary_file.write("    <TD> </TD>\n")

  #
  # Likewise, display the composite PNG image from the final iteration: 
  #
  try:
    summary_file.write("    <TD><A href=\"" + test_name + "/iteration" + str('{:04d}'.format(num_iterations)) + ".html\">\n")
    summary_file.write("      <FONT size=\"1\"><I><U>Final</U></I><BR></FONT>\n")
    summary_file.write("      <IMG border=\"1\" src=\"" + test_name + "/" + final_PNG_file + "\" \n")
    summary_file.write("        width=\"" + str(thumbnail_X) + "\" height=\"" + str(thumbnail_Y) + "\"></A></TD>\n")
  except:
    summary_file.write("    <TD> </TD>\n")
    
  #
  # Finally, display the composite PNG image from the best iteration: 
  #
  try:
    summary_file.write("    <TD><A href=\"" + test_name + "/iteration" + str('{:04d}'.format(best_iteration)) + ".html\">\n")
    summary_file.write("      <FONT size=\"1\"><I><U>Best</U></I><BR></FONT>\n")
    summary_file.write("      <IMG border=\"1\" src=\"" + test_name + "/" + best_PNG_file + "\" \n")
    summary_file.write("        width=\"" + str(thumbnail_X) + "\" height=\"" + str(thumbnail_Y) + "\"></A></TD>\n")
  except:
    summary_file.write("    <TD> </TD>\n")

  

  summary_file.write("    <TD><FONT size=\"1\">vertCostMicrons: " + str(vertCostMicrons) 
                       +       "<BR><BR>violationFreeThreshold:"
                       +       "<BR>&nbsp;&nbsp;User: " + str(userDRCfreeThreshold) 
                       +       "<BR>&nbsp;&nbsp;Acorn: <B>" + str(acornDRCfreeThreshold) + "</B>"
                       +                "<BR><BR>maxIterations: " + str(maxIterations) 
                       +                       "</FONT></TD>\n")

  summary_file.write("  </TR>\n")
  
  # Flush the buffer after writing a row in the HTML table:
  summary_file.flush()

  print("\n")
  print("************************************************************************************")
  print("************************************************************************************")

  # Change the current directory back up to the output directory:
  try:
    os.chdir(output_dir)
  except:
    print("\nERROR: Unable to change to the output directory")
    print("       '" + output_dir + "'.")
    exit(1)


# Close the HTML summary file:
summary_file.write("</TABLE>\n</BODY>\n</HTML>\n")
summary_file.close()

