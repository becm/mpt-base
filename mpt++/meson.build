# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

src_cxx = files(
  'array.cpp',
  'config.cpp',
  'message.cpp',
  'logger.cpp',
  'meta.cpp',
  'object.cpp',
  'parse.cpp',
  'queue.cpp',
)
cxx = [ core ]

if with_plot
  src_cxx += files(
    'cycle.cpp',
    'graphic.cpp',
    'item.cpp',
    'mapping.cpp',
    'plot.cpp',
    'polyline.cpp',
    'transform.cpp'
  )
  cxx = [ plot ] + cxx
endif

if with_io
  src_cxx += files(
    'notify.cpp',
    'stream.cpp'
  )
  cxx = [ io ] + cxx
endif

install_headers('object.h', 'graphic.h',
  subdir : 'mpt')

if with_shared
  mptcxx = shared_library('mpt++', src_cxx, #'libinfo.c',
    install_rpath : '$ORIGIN',
    dependencies : cxx,
    version : '1.0.0', soversion : '1',
    install : true)

  pkgconfig_gen(
    description : 'mpt C++ library',
    libraries : mptcxx,
    version : '1.0.0',
    name : 'mpt++')

  cxx = [ declare_dependency(
      link_with : mptcxx,
      include_directories : include_directories('.'))
    ] + cxx
endif
