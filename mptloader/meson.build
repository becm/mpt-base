# mpt-base/mptloader/meson.build
# build definition for mpt loader library

load_src = files(
  'library_bind.c',
  'library_meta.c',
  'library_open.c',
  'library_symbol.c',
  'readline.c'
)
if not meson.is_subproject()
  install_headers('loader.h', subdir : 'mpt')
endif

# use headers from core library
load = [ core.partial_dependency(includes : true) ]

load += declare_dependency(include_directories : include_directories('.'))

# need explicit dynamic loader library
if host_machine.system() == 'linux'
  load += meson.get_compiler('c').find_library('dl')
endif

if with_shared
  load_lib = shared_library('mptloader', load_src, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    dependencies : load,
    install : with_loader)

  if with_loader
    pkg.generate(load_lib,
      description : 'mpt loader operations',
      version : '1.0.0',
      name : 'mptloader')
  endif

  load = declare_dependency(
    link_with : load_lib,
    dependencies : load)
endif
