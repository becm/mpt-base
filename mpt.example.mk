# mpt.example.mk: template for examples
#
# include global configuration
include $(dir $(lastword $(MAKEFILE_LIST)))mpt.config.mk
#
# preprocessor/compiler flags
INC ?= ${DIR_INC}
CPPFLAGS ?= -Wall -W -Werror $(INC:%=-I%) $(DEF:%=-D'%')
CXXFLAGS ?= ${CFLAGS}
CFLAGS ?= -g -fstack-protector
#
# flags for linker
LDDIRS  ?= ${DIR_LIB}
DLDIRS  ?= ${LDDIRS}
LDFLAGS ?= $(LDDIRS:%=-L%) $(DLDIRS:%=-Wl,-R%)
#
# static version of all programs
STATIC ?= $(PROGS:%=%_static)
#
# auto-generated content
CLEAR_FILES ?= ${PROGS} ${STATIC}
CLEAN_FILES ?= ${OBJS}
#
# general rules
.PHONY: clear clean all static sub_% test_%
all : sub_all ${PROGS}
test : sub_test $(TESTS:%=test_%)

test_% : %
	@prog=$(@:test_%=%) ; \
	printf "\033[01;34m%s\033[0m " "./$${prog}" 1>&2; echo "${ARGS}" 1>&2; \
	"./$${prog}" ${ARGS} < /dev/null && printf "\n"

clear : sub_clear
	${RM} ${CLEAR_FILES}
clean : sub_clean
	${RM} ${CLEAN_FILES}
distclean : sub_distclean
	${RM} ${CLEAR_FILES} ${CLEAN_FILES}

# subdirectory template
sub_% :
	@for d in ${DIRS}; do if ! ${MAKE} -C "$${d}" $(@:sub_%=%); then exit 1; fi; done;

# static template
static : ${STATIC} sub_static
%_static : %.cpp
	${CXX} -static ${CXXFLAGS} ${LDFLAGS} -o ${@} $^ ${LDLIBS}
%_static : %.c
	${CC} -static ${CPPFLAGS} ${CFLAGS} ${LDFLAGS} -o ${@} $^ ${LDLIBS}
