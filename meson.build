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
static_src = [ core_src ]
static_inc = [ core_inc ]

if with_loader
  subdir('mptloader')
  static_src += load_src
endif

if with_plot
  subdir('mptplot')
  static_src += plot_src
  static_inc += plot_inc
endif

if with_io
  subdir('mptio')
  static_src += io_src
  static_inc += io_inc
endif

with_lua = dependency('lua' + get_option('lua'), required : false).found()
if with_lua
  subdir('lua')
endif

if with_static
  mpt = static_library('mpt', static_src,
    include_directories : static_inc,
    install : true)

  pkg.generate(
    description : 'static mpt library',
    libraries : [ mpt ],
    version : '1.0.0',
    name : 'mpt')
endif

if with_cxx
  add_languages('cpp')
  subdir('mpt++')
endif

if with_shared and get_option('with_tests')
  subdir('examples')
endif
