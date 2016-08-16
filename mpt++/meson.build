# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

cxx_src = files(
  'array.cpp',
  'client.cpp', 'config.cpp',
  'event.cpp',
  'logger.cpp',
  'message.cpp', 'meta.cpp', 'meta_new.cpp', 'meta_buffer.cpp',
  'node_new.cpp',
  'object.cpp',
  'proxy.cpp', 'parse.cpp',
  'queue.cpp',
  'reference_wrap.cpp',
  'std_cout.cpp',
  'value.cpp',
)

cxx_inc = [ core_inc ]
if with_shared
  cxx = [ core ]
endif

if with_plot
  cxx_src += files(
    'cycle.cpp',
    'graphic.cpp',
    'item.cpp',
    'layout.cpp',
    'mapping.cpp',
    'polyline.cpp',
    'reference_wrap.cpp',
    'transform.cpp'
  )
  if with_shared
    cxx = [ plot ] + cxx
    cxx_inc = [ plot_inc ] + cxx_inc
  endif
endif

if with_io
  cxx_src += files(
    'notify.cpp',
    'stream.cpp'
  )
  if with_shared
    cxx = [ io ] + cxx
    cxx_inc = [ io_inc ] + cxx_inc
  endif
endif

install_headers('graphic.h', subdir : 'mpt')

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
    include_directories : cxx_inc,
    install : with_cxx)
  if not with_shared
    cxx_lib = cxx_lib_static
  endif
endif

cxx = declare_dependency(
  link_with : cxx_lib,
  include_directories : [ include_directories('.') ] + cxx_inc
)
if with_cxx
  pkg.generate(
    description : 'mpt C++ library',
    libraries : cxx_lib,
    requires_private : 'mpt',
    version : '1.0.0',
    name : 'mpt++')
endif

cxx_inc = [ include_directories('.') ]
