# mpt-base/mptloader/meson.build
# create loader library

src_loader = files(
  'proxy_type.c',
  'library_assign.c',
  'library_open.c',
  'meta_open.c',
  'library_bind.c',
  'readline.c'
)

if with_shared
  # need explicit dynamic loader library
  if host_machine.system() == 'linux'
    load_dep = [ core, meson.get_compiler('c').find_library('dl') ]
  else
    load_dep = core
  endif
  libloader = shared_library('mptloader', src_loader, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : load_dep,
    install : with_loader)

  load = declare_dependency(link_with : libloader)

  if with_loader
    pkg.generate(
      description : 'mpt loader operations',
      libraries : libloader,
      version : '1.0.0',
      name : 'mptloader')
  endif
endif
