# mpt-base/mptio/meson.build
# build definition for mptcore library

# collect source files
mptio_src = files(
  'connect.c',
  'init.c',
  'output_new.c',
  'readline.c'
)
foreach d : [ 'loader', 'notify', 'outdata', 'stream' ]
  subdir(d)
  mptio_src += src
endforeach

static_sources = static_sources + mptio_src

install_headers('notify.h', 'stream.h',
  install_dir : 'include/mpt')

mptio = shared_library('mptio', mptio_src, #'libinfo.c',
  version : '1.0.0', soversion : '1',
  link_with : [ mptcore ],
  dependencies : find_library('dl'),
  include_directories : mpt_includes,
  install : true)

pkgconfig_gen(
  description : 'mpt I/O library',
  libraries : mptio,
  version : '1.0.0',
  name : 'mptio')
