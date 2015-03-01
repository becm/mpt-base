# Makefile: MPT base example programs
#
DIRS = core cxx
PROGS = client
OBJS = $(PROGS:%=%.o)

# .PHONY : all devel clear clean static test distclean
all : test subtest

client : LDLIBS=-lmpt++ -lmptclient -lmptplot -lmptcore
client : CC=${CXX}
client : client.o

include mpt.example.mk

clear : subclear
clean : subclean
test  : subtest

# cleansub : clearsub
subclean subclear subtest subtest :
	@for d in ${DIRS}; do ${MAKE} -C "$${d}" $(@:sub%=%); done;
