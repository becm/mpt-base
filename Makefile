# Makefile: create base MPT modules
MODULES = mptcore mptplot mptio mpt++ lua
#
# creation targets
.PHONY : ${MODULES} examples
all examples : ${MODULES}
mpt++ mptplot mptio : mptcore
mpt++ : mptplot
${MODULES} examples :
	@${MAKE} -C "${@}"
#
# dispatch target to modules
.DEFAULT :
	@for m in ${MODULES} examples; do \
		if ! ${MAKE} -C "$${m}" ${@}; then break; fi; \
	done
#
