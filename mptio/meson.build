# mpt-base/mptio/meson.build
# build definition for mptcore library

# collect source files
src_io = files(
  'connect.c',
  'init.c',
  'output_new.c'
)
foreach d : [ 'notify', 'outdata', 'stream' ]
  subdir(d)
  src_io += src
endforeach

install_headers('notify.h', 'stream.h',
  subdir : 'mpt')

# on platforms with dynamic loader library
dlib = find_library('dl', required : false)

if with_shared
  libio = shared_library('mptio', src_io, 'libinfo.c',
    include_directories : include_directories('../mptplot', '..'),
    install_rpath : '$ORIGIN',
    version : '1.0.0', soversion : '1',
    dependencies : [ core, dlib ],
    install : with_io)

  if with_io
    pkg.generate(
      description : 'mpt I/O library',
      libraries : libio,
      version : '1.0.0',
      name : 'mptio')
  endif

  io = declare_dependency(link_with : libio,
    include_directories : include_directories('.'))
endif
