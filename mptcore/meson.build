# mpt-base/mptcore/meson.build
# build definition for mptcore library

# top level source files
src_core = files(
  'abort.c',
  'identifier.c',
  'log.c',
  'mode_parse.c',
  'reference.c'
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
  'array.h', 'client.h', 'config.h',
  'convert.h', 'core.h',
  'event.h', 'message.h',
  'node.h', 'parse.h', 'queue.h',
  subdir : 'mpt')

if with_shared
  libcore = shared_library('mptcore', src_core, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : inc,
    install : with_core)

  if with_core
    pkg.generate(
      description : 'mpt core library',
      libraries : libcore,
      version : '1.0.0',
      name : 'mptcore')
  endif

  core = declare_dependency(link_with : libcore,
    include_directories : include_directories('.'))
endif
