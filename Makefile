# Makefile: create base MPT modules
MODULES = mptcore mptplot mptio mpt++ lua
#
# creation targets
.PHONY : ${MODULES} examples
examples : ${MODULES}
mpt++ mptplot mptio : mptcore
mpt++ : mptplot
${MODULES} examples : release.h
	@${MAKE} -C "${@}"
#
# release information
include mpt.release.mk
#
# dispatch target to modules
.DEFAULT :
	@for m in ${MODULES} examples; do \
		if ! ${MAKE} -C "$${m}" ${@}; then break; fi; \
	done
#
