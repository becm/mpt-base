# Makefile: MPT base example programs
#
DIRS = core io cxx
PROGS = client readline
TESTS = plot
OBJS = $(PROGS:%=%.o)

# .PHONY : all devel clear clean static test distclean
all : ${PROGS} sub_all

client : LDLIBS=-lmptio -lmptcore
client : CC=${CXX}
client : client.o

readline : LDLIBS=-lmptio
readline : readline.o

plot : LDLIBS=-lmptplot
plot : plot.o

include mpt.example.mk

clear : sub_clear
clean : sub_clean
test  : sub_test
