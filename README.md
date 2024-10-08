# Welcome to Acorn!

### What is Acorn?
Acorn is a basic autorouter in the context of designing microelectronic silicon chips, packages, and printed circuit boards (PCBs). Written in C, the program reads a user-supplied text file that describes the layout and constraints of the chip/package/PCB, including the start- and end-terminals of wires (nets) that should be routed. For each net, Acorn attempts to find a path from the start-terminal to the end-terminal that does not cross any other nets. 
### Why Acorn?
There are many proprietary autorouters that focus on only one part of the microelectronics system, e.g., the silicon chip, **or** the surrounding package, **or** the PCB. Acorn is intended to offer an open-source utility for routing across two or all three of these domains.
### Getting Started
Acorn was developed and tested in a Linux environment, and requires the user to download and then compile the C source code into the Acorn executable file, `acorn.exe`. In the directory that contains the `makefile`, `.c` and `.h` files, type the following command from a command-line:

>make

For the `make` command to work, your system must have the GNU Compiler Collection, `gcc`, and the GNU Make software. The compiler must also have access to the following four libraries: Math (which is included in GCC), [OpenMP](https://www.openmp.org/) (also included in GCC), [LibPng](http://www.libpng.org/), and the [GD Grapics Library](https://libgd.github.io/). The latter two libraries can be installed on Red Hat-based Linux distributions using the following commands:

>yum -y install libpng-devel

>yum -y install gd-devel


Once you've compiled Acorn into the `acorn.exe` executable, copy this file to a working directory and launch Acorn from the command-line using the following command:

>acorn.exe &nbsp; &nbsp; `<input file>` &nbsp; &nbsp; &nbsp; > &nbsp; &nbsp; &nbsp; logfile.txt

The `<input file>` is a text file that describes the silicon chip, package, and/or PCB, in addition to the locations of the start- and end-terminals of each net. There are hundreds of examples of such input files in the `tests` subdirectory. To get started, try a small example such as `4n_800x_500y_4L_obstacles_wide_vias.txt`. In other words, copy this text file to the working directory that contains `acorn.exe`, and type the following command:

>acorn.exe &nbsp; &nbsp; 4n_800x_500y_4L_obstacles_wide_vias.txt&nbsp; &nbsp; &nbsp; > &nbsp; &nbsp; &nbsp; logfile.txt

Depending on how fast your computer is, and how many cores it contains, the program should take a few minutes to complete. (30 seconds with 16 threads is typical.) As it's running, you can monitor two output files: the `logfile.txt` file (using the `less` program, for example) and the `routingStatus.html` file using your favorite web browser. The latter file will be created in the working directory from which you launched Acorn. As a reminder, you can open this file from a browser using CTL-o (or CMD-o on Macs) and navigating to the working directory to open `routingStatus.html`.  The top of this file provides you with the current status, followed by a graph and animated view of the routing. Below the graph is a table that summarizes the results from each iteration. Each row in the table contains a hyperlink to that iteration's routing. Finally, the bottom of the HTML page contains information about the Acorn run, including a link to the input file, design rules, etc.

Incidentally, if you'd like Acorn to use fewer threads than are available on your system, you can use the `-t` switch to specify the number of threads. For example, if you'd like Acorn to use only 2 threads, invoke Acorn like so:

>acorn.exe &nbsp; &nbsp; `-t  2` &nbsp; &nbsp; 4n_800x_500y_4L_obstacles_wide_vias.txt&nbsp; &nbsp; &nbsp; > &nbsp; &nbsp; &nbsp; logfile.txt

 ### Where Can I Get More Help?
 Documentation for Acorn is a work-in-progress. Please be patient.

