# Makefile: create base MPT modules
MODULES = mptcore mptplot mptio mpt++
SUB = ${MODULES} lua examples
#
# creation targets
.PHONY : ${SUB} all clear clean static static_clear sub_%
all : ${MODULES} lua
examples : all
clear : static_clear sub_clear
clean : static_clear sub_clean
static : "${MPT_PREFIX_LIB}/libmpt.a"
mpt++ mptplot mptio : mptcore
mpt++ : mptplot mptio
lua : mptio
${SUB} :
	@${MAKE} -C "${@}"
#
# dispatch target to subdirectories
sub_% :
	@for m in ${SUB}; do \
		if ! ${MAKE} -C "$${m}" $(@:sub_%=%); then break; fi; \
	done
#
# combined static library
static_clear :
	${RM} "${MPT_PREFIX_LIB}/libmpt.a"

"${MPT_PREFIX_LIB}/libmpt.a" :
	@for m in ${MODULES}; do \
		if ! ${MAKE} -C "$${m}" static LIB=mpt "DIR_LIB=${MPT_PREFIX_LIB}"; then break; fi; \
	done
	${AR} s "${MPT_PREFIX_LIB}/libmpt.a"
