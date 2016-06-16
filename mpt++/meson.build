# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

cxx_src = files(
  'array.cpp',
  'config.cpp',
  'message.cpp',
  'value.cpp',
  'logger.cpp',
  'meta.cpp', 'meta_new.cpp',
  'object.cpp',
  'parse.cpp',
  'queue.cpp',
  'reference_wrap.cpp'
)
if with_shared
  cxx = [ core ]
  cxx_inc = [ core_inc ]
  cxx_req = 'mptcore'
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
    cxx_inc = [ plot_inc ] + cxx_inc
    cxx_req = 'mptplot ' + cxx_req
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
    cxx_req = 'mptio ' + cxx_req
  endif
endif

install_headers('object.h', 'graphic.h',
  subdir : 'mpt')

cxx_inc = [ include_directories('.') ] + cxx_inc

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
      requires_private : cxx_req,
      version : '1.0.0',
      name : 'mpt++')
  endif
  
  cxx = declare_dependency(
    link_with : cxx_lib,
    include_directories : cxx_inc
  )
endif

cxx_inc = [ include_directories('.') ]
