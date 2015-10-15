# mpt-base/meson.build
# build definitions for mpt

project('mpt', 'c')

if not meson.is_subproject()
#  add_global_arguments('-std=c11', language : 'c')
  add_global_arguments('-std=c++11', language : 'cpp')
endif

c_args = []
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
  add_global_arguments('-DMPT_NO_PLOT', language : ['c', 'cpp'])
endif

git = run_command('git', 'status', '--porcelain')
if git.returncode() == 0 and git.stdout().strip() == ''
  git = run_command('git', 'show', '-s', '--pretty=format:%h')
  if git.returncode() == 0
    add_global_arguments('-D__VCS_TAG__="' + git.stdout().strip() + '"', language : 'c')
  endif
else
  iso = run_command('date', '+%F')
  if iso.returncode() == 0
    add_global_arguments('-D__ISO_DATE__="' + iso.stdout().strip() + '"', language : 'c')
  endif
endif

inc = [ include_directories('.') ]
with_core = true
subdir('mptcore')
inc += include_directories('mptcore')
src_all = [ src_core ]

with_loader = get_option('with_loader')
if with_loader
  subdir('mptloader')
  src_all += src_loader
endif

if with_plot
  subdir('mptplot')
  inc += include_directories('mptplot')
  src_all += src_plot
endif

with_io = get_option('with_io')
if with_io
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