# release.mk: create release information
RELEASE_HEADER ?= release.h
${RELEASE_HEADER} :
	@if [ -n "${RELEASE}" ]; then \
		echo "#define __RELEASE__ \"${RELEASE}\"" > "${@}"; \
	else \
		echo "#define __ISO_DATE__ \"`date +%F`\"" > "${@}"; \
	fi
#
# add to auto-remove
GEN_FILES += ${RELEASE_HEADER}
