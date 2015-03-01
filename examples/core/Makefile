# Makefile: MPT core library example programs
#
DIR_BIN = .

interact = \
	parse \
	pipe \
	swrite \
	sread \
	mserver \
	mclient

programs = \
	cobs \
	ident \
	axis \
	environ

PROGS = $(programs:%=${DIR_BIN}/%)
INTERACT = $(interact:%=${DIR_BIN}/%)
OBJS = $(INTERACT:%=%.o) $(PROGS:%=%.o)

LDLIBS = -lmptcore

# additional libraries needed
axis mclient : LDLIBS+=-lmptplot

# general creation rules
all : ${PROGS} ${INTERACT}

clear clean : PROGS+=${INTERACT}

# general targets and definitions
include ../mpt.example.mk

# additional targets
static : ${PROGS} ${INTERACT}
static : LDFLAGS+=-static
