# mpt.lib.mk: library creation template
#
# require library name
ifeq (${LIB},)
  $(error no library name defined)
endif
#
# include global configuration
include $(dir $(lastword $(MAKEFILE_LIST)))mpt.config.mk
#
# preprocessor flags
CPPWARN ?= all error pedantic
CPPFLAGS ?= -W $(CPPWARN:%=-W%) $(INC:%=-I%) $(DEF:%=-D%)
# compiler flags
CFLAGS ?= -fPIE -fPIC -g -pg -fstack-protector
CXXFLAGS ?= ${CFLAGS}
# FFLAGS ?= -fpic -O5 -Wall
#
# default object creation/removal
OBJS ?= $(SRCS:%.c=%.o)
CLEAN_FILES ?= ${OBJS}
#
# shared lib defaults
SHLIB_MAJOR ?= 1
SHLIB_MINOR ?= 0
SHLIB_TEENY ?= 0
SHLIB_OBJS  ?= ${OBJS}
#
# static library defaults
STATIC_OBJS ?= ${OBJS}
#
# linker and link options
LDDIRS ?= '${DIR_LIB}'
LDFLAGS ?= '-hlib${LIB}.so.${SHLIB_MAJOR}' -zorigin -rpath=\$$ORIGIN $(LDDIRS:%=-L%)
LINK_FLAGS ?= -shared $(LDFLAGS:%=-Wl,%) ${LDLIBS} -o
LINK ?= ${CC} ${CFLAGS}
#
# path to library without type suffix
LIB_FULLNAME ?= ${DIR_LIB}/lib${LIB}
#
# general library rules
.PHONY: shared devel static install
shared : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}
devel : ${LIB_FULLNAME}.so header
static : ${LIB_FULLNAME}.a
install : devel

$(dir ${LIB_FULLNAME}) :
	install -d '${@}'

${LIB_FULLNAME}.a : ${STATIC_OBJS} ${LIB_FULLNAME}.a(${STATIC_OBJS})
	${AR} s '${@}'

${LIB_FULLNAME}.a(%.o) : %.o
	${AR} S${ARFLAGS} '${@}' $?

${LIB_FULLNAME}.so : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}
	cd '${@D}'; ln -fs '${@F}.${SHLIB_MAJOR}' '${@F}'

${LIB_FULLNAME}.so.${SHLIB_MAJOR} : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY}
	cd '${@D}'; ln -fs '${@F}.${SHLIB_MINOR}.${SHLIB_TEENY}' '${@F}'

${LIB_FULLNAME}.so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY} : ${SHLIB_OBJS}
	${LINK} ${LINK_FLAGS} '${@}' ${SHLIB_OBJS}

extensions = a so so.${SHLIB_MAJOR} so.${SHLIB_MAJOR}.${SHLIB_MINOR} so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY}
CLEAR_FILES += $(extensions:%=${LIB_FULLNAME}.%)
#
# object dependancies
${SHLIB_OBJS} ${STATIC_OBJS} : ${HEADER}
#
# header export
.PHONY: header
header : ${HEADER}; $(call install_files,${DIR_INC},${HEADER})
clear_header = $(notdir ${HEADER})
CLEAR_FILES += $(clear_header:%.h=${DIR_INC}/%.h)
#
# maintenance targets
.PHONY: clear clean distclean uninstall
clear : ; ${RM} ${CLEAR_FILES}
clean : ; ${RM} ${CLEAN_FILES}
distclean : ; ${RM} ${CLEAR_FILES} ${CLEAN_FILES}
uninstall : clear
#
