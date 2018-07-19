# mpt-base/examples/cxx/meson.build
# tests for MPT C++ library

test('array operations',
  executable('array', 'array.cpp',
    cpp_args: test_defines,
    dependencies: cxx)
)
test('config data',
  executable('config', 'config.cpp',
    cpp_args: test_defines,
    dependencies: cxx)
)
test('queue operations',
  executable('queue', 'queue.cpp',
    cpp_args: test_defines,
    dependencies: cxx)
)
# require C++ I/O components
if with_io
  test('buffer rotation',
    executable('buffer', 'buffer.cpp',
      cpp_args : test_defines,
      dependencies: cxx)
  )
endif

# require C++ plot components
if with_plot
  test('object assignment',
    executable('object', 'object.cpp',
      cpp_args: test_defines,
      dependencies: cxx)
  )
  test('iterator processing',
    executable('iter', 'iter.cpp',
      cpp_args: test_defines,
      dependencies: cxx)
  )
  test('cycle mapping',
    executable('map', 'map.cpp',
      cpp_args: test_defines,
      dependencies: cxx)
  )
  test('layout parser',
    executable('layout', 'layout.cpp',
      cpp_args: test_defines,
      dependencies: cxx),
    args: [ join_paths(meson.current_source_dir(), 'layout.lay') ]
  )
  # require C++ plot and I/O components
  if with_io
    test('function output',
      executable('output', 'output.cpp',
        cpp_args: test_defines,
        dependencies: cxx)
    )
  endif
endif