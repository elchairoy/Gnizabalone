ROOT    := .
BIN     := bin
SRC     := src
INCLUDE := include
TESTS   := tests

CC             := gcc
CC_FLAGS       := -Wall -ansi -pedantic -ggdb -I${INCLUDE} -I${SRC} -lm
CC_TESTS_FLAGS := -I${INCLUDE} -lm -DTEST -pg -g # I am not adding here the c90 standard, because it interferes with libtap.

VALGRIND       := valgrind
VALGRIND_FLAGS := --leak-check=yes --error-exitcode=2  --track-origins=yes 
CALLGRIND_FLAGS := --tool=callgrind

SOURCES       := $(shell find ${SRC} -type f -name '*.c')
HEADERS       := $(shell find ${ROOT} -type f -name '*.h')
TESTS_SOURCES := $(shell find ${TESTS} -type f -name '*.c')

EXECUTABLE      := Gnizabalon
TEST_EXECUTABLE := GnizabalonTests
TEST_EXECUTABLE2 := GnizabalonTests2

all: ${BIN}/${EXECUTABLE}

compile-test: ${BIN}/${TEST_EXECUTABLE}

test: ${BIN}/${TEST_EXECUTABLE}

test2: ${BIN}/${TEST_EXECUTABLE2}

run: clean ${BIN}/${EXECUTABLE}
	@./${BIN}/${EXECUTABLE}

valgrind: ${BIN}/${EXECUTABLE}
	${VALGRIND} ${VALGRIND_FLAGS} $<

valgrind-test: ${BIN}/${TEST_EXECUTABLE}
	${VALGRIND} ${VALGRIND_FLAGS} $<

callgrind: ${BIN}/${TEST_EXECUTABLE}
	${VALGRIND} ${CALLGRIND_FLAGS} $<

test_against_stockfish: clean ${BIN}/${TEST_EXECUTABLE}
	python3 tests/play_against_stockfish.py

test_against_itself:
	python3 tests/play_against_stockfish.py

# We are compiling here the sources as well, because the tester code uses the c code.
${BIN}/${TEST_EXECUTABLE}: ${TESTS_SOURCES} ${SOURCES} ${HEADERS}
	mkdir bin -p
	${CC} ${TESTS_SOURCES} ${SOURCES} ${CC_TESTS_FLAGS} -o $@

${BIN}/${TEST_EXECUTABLE2}: ${TESTS_SOURCES} ${SOURCES} ${HEADERS}
	mkdir bin -p
	${CC} ${TESTS_SOURCES} ${SOURCES} ${CC_TESTS_FLAGS} -o $@

${BIN}/${EXECUTABLE}: ${SOURCES} ${HEADERS}
	mkdir bin -p
	${CC} ${SOURCES} ${CC_TESTS_FLAGS} -o $@ 

clean:
	rm -f ${BIN}/*