# mpt-base/meson.build
# build definitions for mpt

project('mpt', 'c',
#  version : '0.0.1'
  default_options : [
    'cpp_std=c++11',
    'c_std=c11'
  ],
  meson_version : '>= 0.47',
)

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
    git = run_command(gitcommand, 'status', '--porcelain', check : true)
  endif
  if is_variable('git') and git.returncode() == 0 and git.stdout().strip() == ''
    git = run_command(gitcommand, 'show', '-s', '--pretty=format:%h', check : true)
    if git.returncode() == 0
      git = '__VCS_TAG__="git:' + git.stdout().strip() + '"'
      add_global_arguments('-D' + git, language : 'c')
      add_global_arguments('-D' + git, language : 'cpp')
    endif
  else
    iso = run_command('date', '+%F', check : true)
    if iso.returncode() == 0
      iso = '__ISO_DATE__="' + iso.stdout().strip() + '"'
      add_global_arguments('-D' + iso, language : 'c')
      add_global_arguments('-D' + iso, language : 'cpp')
    endif
  endif
endif

with_core = true
subdir('mptcore')
static_src = [ core_src ]
static_dep = [ core ]

if with_loader
  subdir('mptloader')
  static_src += load_src
  static_dep += load
endif

if with_plot
  subdir('mptplot')
  static_src += plot_src
  static_dep += plot
endif

if with_io
  subdir('mptio')
  static_src += io_src
  static_dep += io
endif

if with_static
  mpt = static_library('mpt', static_src,
    dependencies : static_dep,
    install : true)

  pkg.generate(
    description : 'static mpt library',
    libraries : mpt,
    version : '1.0.0',
    name : 'mpt')
  
  # bind dependencies to static library
  if not with_shared
    core = declare_dependency(link_with : mpt, dependencies : core)
    if with_loader
      load = declare_dependency(link_with : mpt, dependencies : load)
    endif
    if with_plot
      plot = declare_dependency(link_with : mpt, dependencies : plot)
    endif
    if with_io
      io = declare_dependency(link_with : mpt, dependencies : io)
    endif
  endif
endif

lua = get_option('lua')
if lua != '' and lua != 'false'
  lua = dependency('lua' + lua)
  with_lua = lua.found()
  if with_lua
    subdir('lua')
  endif
else
  with_lua = false
endif

if with_cxx
  add_languages('cpp')
  subdir('mpt++')
endif

if get_option('with_tests')
  subdir('examples')
endif
