# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

cxx_src = files(
  'array.cpp',
  'config.cpp',
  'cycle.cpp',
  'event.cpp',
  'graphic.cpp',
  'item.cpp',
  'logger.cpp',
  'mapping.cpp',
  'meta.cpp',
  'object.cpp',
  'parse.cpp',
  'plot.cpp',
  'polyline.cpp',
  'queue.cpp',
  'stream.cpp',
  'transform.cpp'
)

static_sources = static_sources + cxx_src

install_headers('object.h', 'graphic.h',
  install_dir : 'include/mpt')

mptxx = shared_library('mpt++', cxx_src, #'libinfo.c',
  include_directories : mpt_includes,
  install_rpath : '$ORIGIN',
  link_with : [ mptio, mptplot, mptcore ],
  version : '1.0.0', soversion : '1',
  install : true)

pkgconfig_gen(
  description : 'mpt C++ library',
  libraries : mptxx,
  version : '1.0.0',
  name : 'mpt++')


