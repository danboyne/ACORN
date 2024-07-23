# Specify the build target executable:
TARGET = acorn

#
# Specify the source files:
#
SRCS = acorn.c global_defs.c aStarLibrary.c findShortPathHeuristically.c optimizeDiffPairs.c \
       createDiffPairs.c pruneDiffPairs.c routability.c prepareMap.c parse.c drawMaps.c parseLibrary.c \
       processDiffPairs.c

#
# Specify the object files:
#
OBJS = $(SRCS:.c=.o)

#
# Specify the linker flags
#
LDFLAGS =

#
# Create variable 'GIT_VERSION' that will be used in the C code
# to print out the version number during execution. See the man
# page for 'git describe' for more details, or
# http://www.kernel.org/pub/software/scm/git/docs/git-describe.html
GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"

#
# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
# Specify the compiler:
### CC = gcc -std=c99
### CC = gcc -std=c18
### CC = gcc -std=gnu18
CC = gcc -std=gnu11

#
# Specify the command for deleting files:
RM = rm -f

#
# Specify compiler options:
#
#  -g adds debugging information to the executable file, such as line-numbers
#          for GDB and Valgrind
#  -Wall turns on most, but not all, compiler warnings
#  -fopenmp  used for parallel processing ('Open MP')
# '-fstack-usage' generates a *.su file that reports stack usage
#
# CFLAGS  = -O2 -g -Wall -fopenmp -DVERSION=\"$(GIT_VERSION)\" -fstack-usage
CFLAGS  = -O2 -g -Wall -fopenmp -DVERSION=\"$(GIT_VERSION)\"
# Replace above line with line below when debugging with GDB and Valgrind:
# CFLAGS  = -O0 -g -Wall -fopenmp -DVERSION=\"$(GIT_VERSION)\" -fstack-usage
# Replace above line with line below when profiling with GPROF and Valgrind:
# CFLAGS  = -O2 -g -Wall -pg -fopenmp -DVERSION=\"$(GIT_VERSION)\" -fstack-usage 
# Replace above line with line below when profiling with Intel Vtune:
# CFLAGS  = -O2 -g -dynamic -Wall -fopenmp -DVERSION=\"$(GIT_VERSION)\" -fstack-usage -shared-libgcc -D TBB_USE_THREADING_TOOLS

# define output file:
OUTPUT = acorn.exe

# define any directories containing header files other than /usr/include
#
INCLUDES = -I/usr/include

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -L/usr/lib64

# define any libraries to link into executable:
#   lpng is the PNG library
#   lgomp is the GNU Open MP library
#   lm is the math library
#   lgsl is the GNU Scientific Library
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -lpng -lgomp -lm

# The main rule
all: $(TARGET)

# Compile the source files
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@  

# Link the object files into the executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(OUTPUT) $(LIBS)

# Clean up the object files
clean:
	$(RM) $(OBJS) $(OUTPUT)
