# mpt-base/mptcore/meson.build
# build definition for mptcore library

# collect source files
src_plot = []
foreach d : [ 'output', 'plot', 'values' ]
  subdir(d)
  src_plot += src
endforeach

install_headers('output.h', 'plot.h', 'values.h',
  install_dir : 'include/mpt')

if with_shared
  mptplot = shared_library('mptplot', src_plot, #'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : inc,
    link_with : mptcore,
    install : true)
  
  pkgconfig_gen(
    description : 'mpt plotting library',
    libraries : mptplot,
    version : '1.0.0',
    name : 'mptplot')
endif
