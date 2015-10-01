# mpt-base/mptio/meson.build
# build definition for mptcore library

# collect source files
src_io = files(
  'connect.c',
  'init.c',
  'output_new.c',
  'readline.c'
)
foreach d : [ 'loader', 'notify', 'outdata', 'stream' ]
  subdir(d)
  src_io += src
endforeach

install_headers('notify.h', 'stream.h',
  install_dir : 'include/mpt')

# on platforms with dynamic loader library
dlib = find_library('dl', required : false)

if with_shared
  mptio = shared_library('mptio', src_io, #'libinfo.c',
    version : '1.0.0', soversion : '1',
    link_with : [ mptcore ],
    dependencies : dlib,
    include_directories : inc,
    install : true)
  
  pkgconfig_gen(
    description : 'mpt I/O library',
    libraries : mptio,
    version : '1.0.0',
    name : 'mptio')
endif
