# mpt-base/mptcore/meson.build
# build definition for mptcore library

# top level source files
mptcore_src = files(
  'abort.c',
  'identifier.c',
  'log.c',
  'mode_parse.c'
)
# process subdirectory sources
foreach d : [
  'array',
  'client',
  'config',
  'convert',
  'event',
  'message',
  'meta',
  'node',
  'parse',
  'queue'
]
  subdir(d)
  mptcore_src += src
endforeach

static_sources += mptcore_src

install_headers(
  'array.h', 'client.h', 'config.h', 'convert.h',
  'core.h', 'message.h', 'node.h', 'parse.h', 'queue.h',
  install_dir : 'include/mpt')

mptcore = shared_library('mptcore', mptcore_src, #'libinfo.c',
  version : '1.0.0', soversion : '1',
  include_directories : mpt_includes,
  install : true)

pkgconfig_gen(
  description : 'mpt core library',
  libraries : mptcore,
  version : '1.0.0',
  name : 'mptcore')
