# Makefile: MPT base example programs
#
DIRS = core io cxx lua
TESTS = plot
PROGS = client readline ${TESTS}
OBJS = $(PROGS:%=%.o)

# .PHONY : all devel clear clean static test distclean
all : ${PROGS} sub_all

client : LDLIBS=-lmpt++ -lmptio -lmptcore
readline : LDLIBS=-lmptio
plot : LDLIBS=-lmptplot

include mpt.example.mk

clear : sub_clear
clean : sub_clean
test  : sub_test
