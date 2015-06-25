# Makefile: MPT base example programs
#
DIRS = core io cxx lua
TESTS = client axis
PROGS = readline ${TESTS}
OBJS = $(PROGS:%=%.o)

# .PHONY : all devel clear clean static test distclean
all : ${PROGS} sub_all

client : LDLIBS=-lmpt++ -lmptio -lmptcore
readline : LDLIBS=-lmptio
axis : LDLIBS=-lmptplot -lmptcore

include mpt.example.mk

clear : sub_clear
clean : sub_clean
test  : sub_test
