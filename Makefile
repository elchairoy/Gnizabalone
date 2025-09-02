#
# A robust Makefile for a C project on Linux
#
# This Makefile automates the build process, including
# creating necessary directories, compiling source files,
# and providing rules for debugging and memory checking.
#

# --- Directories and Executable Name ---
BIN     := bin
SRC     := src
INCLUDE := include
EXECUTABLE := ${BIN}/Gnizabalone.exe

# --- Compiler and Flags ---
CC := gcc
# Standard compiler flags for warnings and strictness
CFLAGS :=
# Debug flags: -g adds debugging symbols for use with GDB
DEBUG_CFLAGS := -g
# Valgrind flags for thorough memory leak checking
VALGRIND_FLAGS := --leak-check=full --show-leak-kinds=all --track-origins=yes

# --- Source and Object Files ---
# Find all .c files in the source directory
SOURCES := $(wildcard $(SRC)/*.c)
# Map source files to their corresponding object files in the bin directory
OBJECTS := $(patsubst $(SRC)/%.c,$(BIN)/%.o,$(SOURCES))
DEBUG_OBJECTS := $(patsubst $(SRC)/%.c,$(BIN)/%_debug.o,$(SOURCES))

# --- Rules ---

# Default rule: builds the final executable
all: ${EXECUTABLE}

# Rule to run the executable after building it
run: all
	./${EXECUTABLE}

# Rule to build the executable from object files
${EXECUTABLE}: ${OBJECTS}
	@mkdir -p ${BIN}
	${CC} ${OBJECTS} -I${INCLUDE} -lm -o $@

# Rule to build the debug version of the executable
debug: ${EXECUTABLE}_debug

# Rule to compile and link the debug executable
${EXECUTABLE}_debug: ${DEBUG_OBJECTS}
	@mkdir -p ${BIN}
	${CC} ${DEBUG_OBJECTS} -I${INCLUDE} -lm -o $@

# Rule to compile source files into object files
# The -I flag tells the compiler where to find include files
$(BIN)/%.o: $(SRC)/%.c $(INCLUDE)/*.h
	@mkdir -p ${BIN}
	${CC} ${CFLAGS} -I${INCLUDE} -c $< -o $@

# Rule to compile debug source files into object files
$(BIN)/%_debug.o: $(SRC)/%.c $(INCLUDE)/*.h
	@mkdir -p ${BIN}
	${CC} ${DEBUG_CFLAGS} -I${INCLUDE} -c $< -o $@

# Rule to run the executable with Valgrind for memory checking
valgrind: ${EXECUTABLE}_debug
	valgrind ${VALGRIND_FLAGS} ./${EXECUTABLE}_debug

# Rule to run the executable with Valgrind and the Callgrind tool for profiling
callgrind: ${EXECUTABLE}_debug
	valgrind --tool=callgrind ./${EXECUTABLE}_debug

# Rule to clean up all generated files
clean:
	rm -f ${OBJECTS} ${EXECUTABLE} ${EXECUTABLE}_debug

# Phony targets prevent conflicts with files of the same name
.PHONY: all run clean debug valgrind callgrind