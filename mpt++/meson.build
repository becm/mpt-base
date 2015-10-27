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
if with_shared
  cxx = [ core ]
endif

if with_plot
  src_cxx += files(
    'cycle.cpp',
    'graphic.cpp',
    'item.cpp',
    'mapping.cpp',
    'layout.cpp',
    'polyline.cpp',
    'transform.cpp'
  )
  if with_shared
    cxx = [ plot ] + cxx
  endif
endif

if with_io
  src_cxx += files(
    'notify.cpp',
    'stream.cpp'
  )
  if with_shared
    cxx = [ io ] + cxx
  endif
endif

install_headers('object.h', 'graphic.h',
  subdir : 'mpt')

if with_shared
  libcxx = shared_library('mpt++', src_cxx, 'libinfo.c',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : cxx,
    version : '1.0.0', soversion : '1',
    install : with_cxx)

  if with_cxx
    pkgconfig_gen(
      description : 'mpt C++ library',
      libraries : libcxx,
      version : '1.0.0',
      name : 'mpt++')
  endif

  cxx = [ declare_dependency(
      link_with : libcxx,
      include_directories : include_directories('.'))
    ] + cxx
endif
