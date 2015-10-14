# mpt-base/mptio/meson.build
# build definition for mptcore library

# collect source files
src_io = files(
  'connect.c',
  'init.c',
  'output_new.c',
  'readline.c'
)
foreach d : [ 'notify', 'outdata', 'stream' ]
  subdir(d)
  src_io += src
endforeach

if with_loader
  subdir('loader')
  src_io += src
endif

install_headers('notify.h', 'stream.h',
  subdir : 'mpt')

# on platforms with dynamic loader library
dlib = find_library('dl', required : false)

if with_shared
  mptio = shared_library('mptio', src_io, #'libinfo.c',
    install_rpath : '$ORIGIN',
    version : '1.0.0', soversion : '1',
    dependencies : [ core, dlib ],
    include_directories : include_directories('../mptplot'),
    install : true)

  pkgconfig_gen(
    description : 'mpt I/O library',
    libraries : mptio,
    version : '1.0.0',
    name : 'mptio')

  io = declare_dependency(link_with : mptio,
    include_directories : include_directories('.'))
endif
