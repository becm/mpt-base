# mpt.lib.mk: library creation template
#
# require library name
ifeq (${LIB},)
  $(error no library name defined)
endif
#
# include global configuration
include $(dir $(lastword $(MAKEFILE_LIST)))mpt.tag.mk
include $(dir $(lastword $(MAKEFILE_LIST)))mpt.config.mk
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
LDDIRS ?= '${PREFIX_LIB}'
LDFLAGS ?= '-hlib${LIB}.so.${SHLIB_MAJOR}' -zorigin -rpath=\$$ORIGIN $(LDDIRS:%=-L%)
LINK_FLAGS ?= -shared $(LDFLAGS:%=-Wl,%) ${LDLIBS}
LINK ?= ${CC} ${CFLAGS}
#
# library targets
LIB_FULLNAME ?= ${PREFIX_LIB}/lib${LIB}
LIB_STATIC = ${LIB_FULLNAME}.a
LIB_SHARED = ${LIB_FULLNAME}.so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY}
#
# general library rules
.PHONY: shared devel static install
shared : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}
devel : ${LIB_FULLNAME}.so
static : ${LIB_FULLNAME}.a
install : header devel

${LIB_STATIC} : ${LIB_STATIC}(${STATIC_OBJS})
	${AR} sU '${@}'

${LIB_STATIC}(%.o) : %.o
	install -d '${@D}'
	${AR} SU${ARFLAGS} '${@}' $?

${LIB_FULLNAME}.so : ${LIB_FULLNAME}.so.${SHLIB_MAJOR}
	cd '${@D}'; ln -fs '${?F}' '${@F}'

${LIB_FULLNAME}.so.${SHLIB_MAJOR} : ${LIB_SHARED}
	cd '${@D}'; ln -fs '${?F}' '${@F}'

${LIB_SHARED} : ${SHLIB_OBJS}
	install -d '${@D}'
	${LINK} -o '${@}' ${SHLIB_OBJS} ${LINK_FLAGS}

extensions = a so so.${SHLIB_MAJOR} so.${SHLIB_MAJOR}.${SHLIB_MINOR} so.${SHLIB_MAJOR}.${SHLIB_MINOR}.${SHLIB_TEENY}
CLEAR_FILES += $(extensions:%=${LIB_FULLNAME}.%)
#
# header export
.PHONY: header
header : ${HEADER}; $(call install_files,${PREFIX_INC},${HEADER})
clear_header = $(notdir ${HEADER})
CLEAR_FILES += $(clear_header:%.h=${PREFIX_INC}/%.h)
#
# maintenance targets
.PHONY: clear clean distclean uninstall
clear : ; ${RM} ${CLEAR_FILES}
clean : ; ${RM} ${CLEAN_FILES}
distclean : ; ${RM} ${CLEAR_FILES} ${CLEAN_FILES}
uninstall : clear
#
