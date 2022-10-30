# mpt-base/mptplot/meson.build
# build definition for mpt plot library

# collect source files
plot_src = files(
  'mapping.c',
  'rawdata_type_traits.c',
  'rawdata_create.c'
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
    install_rpath : '$ORIGIN',
    dependencies : core,
    install : with_plot)
  
  if with_plot
    pkg.generate(plot_lib,
      description : 'mpt plotting library',
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
