# mpt-base/mptloader/meson.build
# create loader library

src_loader = files(
  'library_assign.c',
  'library_open.c',
  'meta_open.c',
  'library_bind.c',
  'readline.c'
)

if with_shared
  if host_machine.system() == 'linux'
    load_dep = [ core, find_library('dl') ]
  else
    load_dep = core
  endif
  libloader = shared_library('mptloader', src_loader, 'libinfo.c',
    include_directories : include_directories('..'),
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
