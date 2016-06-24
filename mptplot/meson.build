# mpt-base/mptcore/meson.build
# build definition for mptcore library

# collect source files
plot_src = []
foreach d : [ 'layout', 'output', 'values' ]
  subdir(d)
  plot_src += src
endforeach

install_headers('layout.h', 'values.h',
  subdir : 'mpt')

plot_inc = include_directories('.')

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
    include_directories : [ plot_inc, core_inc ]
  )
endif
