# mpt-base/mptcore/meson.build
# build definition for mptcore library

# top level source files
src_core = files(
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
  src_core += src
endforeach

install_headers(
  'array.h', 'client.h', 'config.h', 'convert.h',
  'core.h', 'message.h', 'node.h', 'parse.h', 'queue.h',
  install_dir : 'include/mpt')

if with_shared
  mptcore = shared_library('mptcore', src_core, #'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : inc,
    install : true)

  pkgconfig_gen(
    description : 'mpt core library',
    libraries : mptcore,
    version : '1.0.0',
    name : 'mptcore')
endif
