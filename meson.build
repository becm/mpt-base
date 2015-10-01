# mpt-base/meson.build
# build definitions for mpt

project('mpt', 'c')

if not meson.is_subproject()
#  add_global_arguments('-std=c11', language : 'c')
  add_global_arguments('-std=c++11', language : 'cpp')
endif

libs = get_option('libraries')
if libs == 'shared'
  with_static = false
  with_shared = true
elif libs == 'static'
  with_static = true
  with_shared = false
else
  with_static = true
  with_shared = true
endif

inc = include_directories(
   meson.current_source_dir() + '/mptcore',
   meson.current_source_dir() + '/mptplot',
   meson.current_source_dir() + '/mptio',
   meson.current_source_dir() + '/mpt++'
)
subdir('mptcore')
subdir('mptplot')
subdir('mptio')

src = [
  src_core,
  src_plot,
  src_io
]

if get_option('with_cxx')
  add_languages('cpp')
  subdir('mpt++')
  src += src_xx
endif

if with_static
  mpt = static_library('mpt', src,
    include_directories : inc,
    dependencies : find_library('dl'),
    install : true)
  
  pkgconfig_gen(
    description : 'static mpt library',
    libraries : [ mpt ],
    version : '1.0.0',
    name : 'mpt')
endif