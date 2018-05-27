# mpt-base/mptcore/meson.build
# build definition for mptcore library

# collect source files
plot_src = files(
  'mapping.c',
  'rawdata_typeid.c', 'rawdata_create.c'
)
foreach d : [ 'history', 'layout', 'values', 'output' ]
  subdir(d)
  plot_src += src
endforeach

if not meson.is_subproject()
  install_headers('history.h', 'layout.h', 'values.h', subdir : 'mpt')
endif

plot = include_directories('.')

if with_shared
  plot_lib = shared_library('mptplot', plot_src, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : core,
    install : with_plot)
  
  if with_plot
    pkg.generate(
      description : 'mpt plotting library',
      libraries : plot_lib,
      requires_private : 'mptcore',
      version : '1.0.0',
      name : 'mptplot')
  endif
  
  plot = declare_dependency(
    link_with : plot_lib,
    dependencies : core,
    include_directories : plot)
else
  plot = declare_dependency(include_directories : plot, dependencies : core)
endif
