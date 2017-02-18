# mpt-base/mptcore/meson.build
# build definition for mptcore library

# top level source files
core_src = []
# process subdirectory sources
foreach d : [
  'array',
  'client',
  'config',
  'convert',
  'event',
  'log',
  'message',
  'misc',
  'node',
  'output',
  'parse',
  'queue'
]
  subdir(d)
  core_src += src
endforeach

install_headers(
  'array.h', 'client.h', 'config.h',
  'convert.h', 'core.h', 'event.h',
  'message.h', 'meta.h', 'node.h',
  'object.h', 'output.h',
  'parse.h', 'queue.h',
  subdir : 'mpt')

core_inc = include_directories('.')

if with_shared
  core_lib = shared_library('mptcore', core_src, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    install : with_core)

  if with_core
    pkg.generate(
      description : 'mpt core library',
      libraries : core_lib,
      version : '1.0.0',
      name : 'mptcore')
  endif

  core = declare_dependency(
    link_with : core_lib,
    include_directories : core_inc
  )
endif
