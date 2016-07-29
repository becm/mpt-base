# mpt.gnu.mk: use GNU extensions
DIR_BASE ?= $(dir $(lastword $(MAKEFILE_LIST)))
#
# ISO format date
ISODATE ?= $(shell date +%F)
#
# version information
define vcs_tag
  $(if $(shell git status --porcelain),,$(shell printf '%s:%s' 'git' `git show -s --pretty=format:%h`))
endef
VCS_TAG ?= $(strip $(call vcs_tag))
#
# install header files
define install_files
  $(if $(strip ${2}),@install -d '${1}' && install -C -m 644 ${2} '${1}' && printf 'install(%s): %s\n' '${1}' '${2}')
endef
