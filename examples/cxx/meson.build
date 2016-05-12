# mpt-base/examples/cxx/meson.build
# tests for MPT C++ library

cdir = meson.current_source_dir()

test('buffer rotation',
  executable('buffer', cdir + '/buffer.cpp',
    cpp_args : test_defines,
    dependencies: cxx))

test('config data',
  executable('config', cdir + '/config.cpp',
    cpp_args: test_defines,
    dependencies: cxx))

test('queue operations',
  executable('queue', cdir + '/queue.cpp',
    cpp_args: test_defines,
    dependencies: cxx))

# require C++ plot components
if with_plot
  test('object assignment',
    executable('object', cdir + '/object.cpp',
      cpp_args: test_defines,
      dependencies: cxx))
  
  test('cycle mapping',
    executable('map', cdir + '/map.cpp',
      cpp_args: test_defines,
      dependencies: cxx))
  
  test('layout parser',
    executable('layout', cdir + '/layout.cpp',
      cpp_args: test_defines,
      dependencies: cxx),
    args: [ cdir + '/layout.lay' ])
  
  # require C++ plot and I/O components
  if with_io
    test('function output',
      executable('output', cdir + '/output.cpp',
        cpp_args: test_defines,
        dependencies: cxx))
  endif
endif