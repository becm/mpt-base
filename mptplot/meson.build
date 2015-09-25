# mpt-base/mptcore/meson.build
# build definition for mptcore library

# collect source files
mptplot_src = []
foreach d : [ 'output', 'plot', 'values' ]
  subdir(d)
  mptplot_src += src
endforeach

static_sources = static_sources + mptplot_src

install_headers('output.h', 'plot.h', 'values.h',
  install_dir : 'include/mpt')

mptplot = shared_library('mptplot', mptplot_src, #'libinfo.c',
  version : '1.0.0', soversion : '1',
  include_directories : mpt_includes,
  link_with : mptcore,
  install : true)

pkgconfig_gen(
  description : 'mpt plotting library',
  libraries : mptplot,
  version : '1.0.0',
  name : 'mptplot')
