# mpt-base/mptloader/meson.build
# create loader library

src_loader = files(
  'library_assign.c',
  'library_open.c',
  'meta_open.c',
  'library_bind.c'
)

mptload = shared_library('mptloader', src_loader, 'libinfo.c',
  include_directories : inc,
  dependencies : find_library('dl'),
  install : with_loader)

load = declare_dependency(link_with : mptload)

if with_loader
  pkgconfig_gen(
    description : 'mpt loader operations',
    libraries : mptload,
    version : '1.0.0',
    name : 'mptloader')
endif
