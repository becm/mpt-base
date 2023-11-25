# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

cxx_src = files(
  'array.cpp',
  'client.cpp',
  'config.cpp',
  'collection.cpp',
  'convertable.cpp',
  'event.cpp',
  'identifier.cpp',
  'io.cpp',
  'io_buffer.cpp', 'io_buffer_metatype.cpp',
  'io_queue.cpp',
  'iterator.cpp',
  'logger.cpp',
  'message.cpp',
  'meta_buffer.cpp',
  'metatype.cpp',
  'metatype_create.cpp',
  'metatype_basic.cpp',
  'metatype_generic.cpp',
  'node.cpp',
  'object.cpp',
  'output.cpp',
  'parse.cpp',
  'property.cpp',
  'queue.cpp',
  'refcount_wrap.cpp',
  'std_cout.cpp',
  'type_traits_wrap.cpp',
  'value.cpp',
)

# creator overrides
cxx_src += files(
  'meta_new.cpp', 'node_new.cpp'
)

cxx = [ core ]

if with_plot
  cxx_src += files(
    'color.cpp',
    'cycle.cpp',
    'destination.cpp',
    'graph.cpp',
    'graphic.cpp',
    'item_group.cpp',
    'layout.cpp',
    'linepart.cpp',
    'mapping.cpp',
    'point.cpp',
    'polyline.cpp',
    'transform.cpp',
    'value_store.cpp'
  )
  cxx += plot
endif

if with_io
  cxx_src += files(
    'io_stream.cpp', 'io_stream_input.cpp',
    'notify.cpp',
    'socket.cpp',
    'stream.cpp'
  )
  cxx += io
endif

if not meson.is_subproject()
  install_headers(['graphic.h', 'io.h'], subdir : 'mpt')
endif

if with_shared
  cxx_lib = shared_library('mpt++', cxx_src, 'libinfo.c',
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
