# mpt-base/mptloader/meson.build
# create loader library

load_src = files(
  'library_assign.c',
  'library_bind.c',
  'library_meta.c',
  'library_open.c',
  'readline.c'
)

if with_shared
  # need explicit dynamic loader library
  if host_machine.system() == 'linux'
    load_dep = [ core, meson.get_compiler('c').find_library('dl') ]
  else
    load_dep = core
  endif
  load_lib = shared_library('mptloader', load_src, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : load_dep,
    install : with_loader)

  if with_loader
    pkg.generate(
      description : 'mpt loader operations',
      libraries : load_lib,
      requires_private : 'mptcore',
      version : '1.0.0',
      name : 'mptloader')
  endif

  load = declare_dependency(
    link_with : load_lib,
    include_directories : core_inc
  )
endif
