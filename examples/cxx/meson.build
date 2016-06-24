# mpt-base/examples/cxx/meson.build
# tests for MPT C++ library

test('buffer rotation',
  executable('buffer', 'buffer.cpp',
    cpp_args : test_defines,
    dependencies: cxx))

test('config data',
  executable('config', 'config.cpp',
    cpp_args: test_defines,
    dependencies: cxx))

test('queue operations',
  executable('queue', 'queue.cpp',
    cpp_args: test_defines,
    dependencies: cxx))

# require C++ plot components
if with_plot
  test('object assignment',
    executable('object', 'object.cpp',
      cpp_args: test_defines,
      dependencies: cxx))
  
  test('cycle mapping',
    executable('map', 'map.cpp',
      cpp_args: test_defines,
      dependencies: cxx))
  
  test('layout parser',
    executable('layout', 'layout.cpp',
      cpp_args: test_defines,
      dependencies: cxx),
    args: [ meson.current_source_dir() + '/layout.lay' ])
  
  # require C++ plot and I/O components
  if with_io
    test('function output',
      executable('output', 'output.cpp',
        cpp_args: test_defines,
        dependencies: cxx))
  endif
endif