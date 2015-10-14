# mpt-base/meson.build
# build definitions for mpt

project('mpt', 'c')

if not meson.is_subproject()
#  add_global_arguments('-std=c11', language : 'c')
  add_global_arguments('-std=c++11', language : 'cpp')
endif

cpp_args = []

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

with_plot = get_option('with_plot')
if not with_plot
  cpp_args += [ '-DMPT_NO_PLOT' ]
endif

subdir('mptcore')
inc = [ include_directories('mptcore') ]
src_all = [ src_core ]

if with_plot
  subdir('mptplot')
  inc += include_directories('mptplot')
  src_all += src_plot
endif

with_io = get_option('with_io')
if with_io
  with_loader = get_option('with_loader')
  with_io = true
  subdir('mptio')
  inc += include_directories('mptio')
  src_all += src_io
endif

if dependency('lua' + get_option('lua'), required : false).found()
  subdir('lua')
endif

with_cxx = get_option('with_cxx')
if with_cxx
  add_languages('cpp')
  subdir('mpt++')
  inc += include_directories('mpt++')
  src_all += src_cxx
endif

if with_static
  mpt = static_library('mpt', src_all,
    include_directories : inc,
    install : true)
  
  pkgconfig_gen(
    description : 'static mpt library',
    libraries : [ mpt ],
    version : '1.0.0',
    name : 'mpt')
endif