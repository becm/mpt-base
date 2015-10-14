# mpt-base/mptloader/meson.build.gen
# create loader library

src = files(
  'library_assign.c',
  'library_open.c',
  'meta_open.c',
  'library_bind.c'
)

mptload = static_library('mptloader', src,
  include_directories : inc,
  install : with_loader)

load = declare_dependency(link_with : mptload)

if with_loader
  pkgconfig_gen(
    description : 'mpt loader operations',
    libraries : mptload,
    version : '1.0.0',
    name : 'mptloader')
endif
