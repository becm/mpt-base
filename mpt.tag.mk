# mpt.tag.mk: determine build tag
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
# define VCS tag or ISO date
DEF += $(if ${VCS_TAG},'__VCS_TAG__="${VCS_TAG}"','__ISO_DATE__="${ISODATE}"')
