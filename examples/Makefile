# Makefile: MPT base example programs
#
DIRS = core io cxx
PROGS = client readline
TESTS = plot
OBJS = $(PROGS:%=%.o) $(TESTS:%=%.o)

# .PHONY : all devel clear clean static test distclean
all : ${PROGS} sub_all

client : LDLIBS=-lmptcore
readline : LDLIBS=-lmptio
plot : LDLIBS=-lmptplot

include mpt.example.mk

clear : sub_clear
clean : sub_clean
test  : sub_test
