# mpt-base/mptcore/meson.build
# build definition for mptcore library

# collect source files
src_plot = []
foreach d : [ 'output', 'plot', 'values' ]
  subdir(d)
  src_plot += src
endforeach

install_headers('output.h', 'plot.h', 'values.h',
  subdir : 'mpt')

if with_shared
  mptplot = shared_library('mptplot', src_plot, #'libinfo.c',
    install_rpath : '$ORIGIN',
    version : '1.0.0', soversion : '1',
    dependencies : core,
    install : with_plot)
  
  pkgconfig_gen(
    description : 'mpt plotting library',
    libraries : mptplot,
    version : '1.0.0',
    name : 'mptplot')

  plot = declare_dependency(link_with : mptplot,
    include_directories : include_directories('.'))
endif
