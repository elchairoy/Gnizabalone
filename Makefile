# --- Directories and Executable Name ---
BIN     := bin
SRC     := src
INCLUDE := include
EXECUTABLE := ${BIN}/Gnizabalone.exe

# --- Compiler and Flags ---
CC := gcc
CFLAGS := 

# --- Source and Object Files ---
SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c,$(BIN)/%.o,$(SOURCES))

# --- Rules ---

# Default rule: build the executable
all: ${EXECUTABLE}

# Rule to run the executable
run: all
	./${EXECUTABLE}

# Build executable from object files
${EXECUTABLE}: ${OBJECTS}
	@mkdir -p ${BIN}
	${CC} ${OBJECTS} -I${INCLUDE} -lm -o $@

# Compile source files to object files
$(BIN)/%.o: $(SRC)/%.c $(INCLUDE)/*.h
	@mkdir -p ${BIN}
	${CC} ${CFLAGS} -I${INCLUDE} -c $< -o $@

# Clean generated files
clean:
	rm -f ${OBJECTS} ${EXECUTABLE}

.PHONY: all run clean