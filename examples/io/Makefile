# Makefile: MPT I/O library example programs
#
DIR_BIN = .

progs = \
	swrite sread \
	mserver mclient \
	pipe

PROGS = $(progs:%=${DIR_BIN}/%)
TESTS = $(tests:%=${DIR_BIN}/%)
LDLIBS = -lmptio

# general creation rules
all : ${PROGS} ${TESTS}

# additional libraries needed
sread mclient : LDLIBS+=-lmptcore

# general targets and definitions
include ../mpt.example.mk

# additional targets
static : ${PROGS} ${TESTS}
static : LDFLAGS+=-static
