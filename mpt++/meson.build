# mpt-base/mpt++/meson.build
# build definition for mpt C++ library

src_xx = files(
  'array.cpp',
  'config.cpp',
  'cycle.cpp',
  'message.cpp',
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
  'transform.cpp'
)
mptxx_link = [ mptplot, mptcore ]

if get_option('with_io')
  src_xx += files(
    'notify.cpp',
    'stream.cpp'
  )
  mptxx_link = [ mptio ] + mptxx_link
endif

install_headers('object.h', 'graphic.h',
  install_dir : 'include/mpt')

if with_shared
  mptxx = shared_library('mpt++', src_xx, #'libinfo.c',
    include_directories : inc,
    install_rpath : '$ORIGIN',
    link_with : mptxx_link,
    version : '1.0.0', soversion : '1',
    install : true)
  
  pkgconfig_gen(
    description : 'mpt C++ library',
    libraries : mptxx,
    version : '1.0.0',
    name : 'mpt++')
endif
