# mpt-base/meson.build
# build definitions for mpt

project('mpt', 'c', default_options : [
  'cpp_std=c++11',
  'c_std=c11'
])

pkg = import('pkgconfig')

# conditionals from configuration
with_plot = get_option('with_plot')
with_io = get_option('with_io')
with_cxx = get_option('with_cxx')
with_loader = get_option('with_loader')
libs = get_option('libraries')


if not (is_variable('with_static') and is_variable('with_shared'))
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
endif

if not meson.is_subproject()
  gitcommand = find_program('git', required : false)
  if gitcommand.found()
    git = run_command(gitcommand, 'status', '--porcelain')
  endif
  if is_variable('git') and git.returncode() == 0 and git.stdout().strip() == ''
    git = run_command(gitcommand, 'show', '-s', '--pretty=format:%h')
    if git.returncode() == 0
      add_global_arguments('-D__VCS_TAG__="git:' + git.stdout().strip() + '"', language : 'c')
    endif
  else
    iso = run_command('date', '+%F')
    if iso.returncode() == 0
      add_global_arguments('-D__ISO_DATE__="' + iso.stdout().strip() + '"', language : 'c')
    endif
  endif
endif

with_core = true
subdir('mptcore')
src_all = [ src_core ]

if with_loader
  subdir('mptloader')
  src_all += src_loader
endif

if with_plot
  subdir('mptplot')
  src_all += src_plot
endif

if with_io
  subdir('mptio')
  src_all += src_io
endif

if dependency('lua' + get_option('lua'), required : false).found()
  subdir('lua')
endif

if with_cxx
  add_languages('cpp')
  subdir('mpt++')
  src_all += src_cxx
endif

if with_static
  mpt = static_library('mpt', src_all,
    include_directories : include_directories(
      '.',
      'mptcore',
      'mptplot',
      'mptio',
      'mpt++'
    ),
    install : true)

  pkg.generate(
    description : 'static mpt library',
    libraries : [ mpt ],
    version : '1.0.0',
    name : 'mpt')
endif
