# mpt-base/mptcore/meson.build
# build definition for mptcore library

# collect source files
src_plot = []
foreach d : [ 'output', 'layout', 'values' ]
  subdir(d)
  src_plot += src
endforeach

install_headers('output.h', 'layout.h', 'values.h',
  subdir : 'mpt')

if with_shared
  libplot = shared_library('mptplot', src_plot, 'libinfo.c',
    version : '1.0.0', soversion : '1',
    include_directories : include_directories('..'),
    install_rpath : '$ORIGIN',
    dependencies : core,
    install : with_plot)
  
  if with_plot
    pkg.generate(
      description : 'mpt plotting library',
      libraries : libplot,
      version : '1.0.0',
      name : 'mptplot')
  endif
  
  plot = declare_dependency(link_with : libplot,
    include_directories : include_directories('.'))
endif
