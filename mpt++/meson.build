# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

cxx_src = files(
  'array.cpp',
  'config.cpp',
  'message.cpp',
  'value.cpp',
  'logger.cpp',
  'meta.cpp',
  'object.cpp',
  'parse.cpp',
  'queue.cpp',
  'reference.cpp'
)
if with_shared
  cxx = [ core ]
  cxx_requires = 'mptcore'
endif

if with_plot
  cxx_src += files(
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
    cxx_requires = 'mptplot ' + cxx_requires
  endif
endif

if with_io
  cxx_src += files(
    'notify.cpp',
    'stream.cpp'
  )
  if with_shared
    cxx = [ io ] + cxx
    cxx_requires = 'mptio ' + cxx_requires
  endif
endif

install_headers('object.h', 'graphic.h',
  subdir : 'mpt')

if with_shared
  cxx_lib = shared_library('mpt++', cxx_src, 'libinfo.c',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : cxx,
    version : '1.0.0', soversion : '1',
    install : with_cxx)

  if with_cxx
    pkg.generate(
      description : 'mpt C++ library',
      libraries : cxx_lib,
      requires : cxx_requires,
      version : '1.0.0',
      name : 'mpt++')
  endif

  cxx = [ declare_dependency(link_with : cxx_lib,
            include_directories : include_directories('.'))
        ] + cxx
endif
