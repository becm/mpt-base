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
  'message',
  'meta',
  'misc',
  'node',
  'object',
  'output',
  'parse',
  'queue',
  'types'
]
  subdir(d)
  core_src += src
endforeach

if not meson.is_subproject()
install_headers(
  'array.h',
  'client.h', 'collection.h', 'config.h', 'convert.h', 'core.h',
  'event.h',
  'message.h', 'meta.h',
  'node.h',
  'object.h', 'output.h',
  'parse.h',
  'queue.h',
  subdir : 'mpt')
endif

core = include_directories('.')

if with_shared
  core_lib = shared_library('mptcore', core_src, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    install : with_core)

  if with_core
    pkg.generate(core_lib,
      description : 'mpt core library',
      version : '1.0.0',
      name : 'mptcore')
  endif

  core = declare_dependency(
    link_with : core_lib,
    include_directories : core)
else
  core = declare_dependency(include_directories : core)
endif
