# mpt-base/mptio/meson.build
# build definition for mptcore library

# collect source files
io_src = files(
  'connect.c',
  'init.c',
  'output_local.c',
  'output_remote.c'
)
foreach d : [ 'notify', 'outdata', 'stream' ]
  subdir(d)
  io_src += src
endforeach

if not meson.is_subproject()
  install_headers('notify.h', 'stream.h', subdir : 'mpt')
endif

io = include_directories('.')

if with_shared
  io_lib = shared_library('mptio', io_src, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : core,
    install : with_io)

  if with_io
    pkg.generate(
      description : 'mpt I/O library',
      libraries : io_lib,
      requires_private : 'mptcore',
      version : '1.0.0',
      name : 'mptio')
  endif

  io = declare_dependency(
    link_with : io_lib,
    dependencies : core,
    include_directories : io)
else
  io = declare_dependency(include_directories : io, dependencies : core)
endif
