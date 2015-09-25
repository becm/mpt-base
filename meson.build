# mpt-base/meson.build
# build definitions for mpt

project('mpt', 'c', 'cpp')

add_global_arguments('-std=c++11', language : 'cpp')

mpt_includes = include_directories(
   meson.current_source_dir() + '/mptcore',
   meson.current_source_dir() + '/mptplot',
   meson.current_source_dir() + '/mptio',
   meson.current_source_dir() + '/mpt++',
)

static_sources = []
subdir('mptcore')
subdir('mptplot')
subdir('mptio')
subdir('mpt++')

mpt = static_library('mpt', static_sources,
  include_directories : mpt_includes,
  dependencies : find_library('dl'),
  install : true)

pkgconfig_gen(
  description : 'static mpt library',
  libraries : [ mpt ],
  version : '1.0.0',
  name : 'mpt')
