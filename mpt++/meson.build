# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

cxx_src = files(
  'array.cpp',
  'buffer.cpp',
  'client.cpp', 'config.cpp',
  'event.cpp',
  'logger.cpp',
  'makeid.cpp',
  'message.cpp',
  'meta_create.cpp', 'meta_basic.cpp', 'meta_typed.cpp', 'meta_buffer.cpp',
  'node.cpp',
  'object.cpp',
  'output.cpp',
  'parse.cpp',
  'queue.cpp',
  'refcount_wrap.cpp',
  'std_cout.cpp',
  'typed_array.cpp',
  'value.cpp',
)

# creator overrides
cxx_src += files(
  'meta_new.cpp', 'node_new.cpp'
)

cxx = [ core ]

if with_plot
  cxx_src += files(
    'cycle.cpp',
    'destination.cpp',
    'graphic.cpp',
    'item.cpp',
    'layout.cpp',
    'mapping.cpp',
    'polyline.cpp',
    'transform.cpp'
  )
  cxx += plot
endif

if with_io
  cxx_src += files(
    'notify.cpp',
    'stream.cpp'
  )
  cxx += io
endif

if not meson.is_subproject()
  install_headers('graphic.h', subdir : 'mpt')
endif

if with_shared
  cxx_lib = shared_library('mpt++', cxx_src, 'libinfo.c',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : cxx,
    version : '1.0.0', soversion : '1',
    install : with_cxx)
endif

if with_static
  cxx_lib_static = static_library('mpt++', cxx_src,
    dependencies : cxx,
    install : with_cxx)
  if not with_shared
    cxx_lib = cxx_lib_static
  endif
endif

cxx = declare_dependency(
  link_with : cxx_lib,
  dependencies : cxx,
  include_directories : [ include_directories('.') ])

if with_cxx
  pkg.generate(
    description : 'mpt C++ library',
    libraries : cxx_lib,
    requires_private : 'mpt',
    version : '1.0.0',
    name : 'mpt++')
endif
