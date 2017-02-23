# mpt-base/examples/meson.build
# tests for MPT base modules

test_defines = [ '-DMPT_INCLUDE(x)=#x' ]

if with_core
  subdir('core')
endif

if with_cxx
  subdir('cxx')
  if with_io
    test('client instance',
      executable('client', 'client.cpp',
        cpp_args: test_defines,
        dependencies: cxx))
  endif
endif

if with_plot
  test('axis data',
    executable('axis', 'axis.c',
      c_args: test_defines,
      dependencies: plot),
    args: [ 'title=my axis', 'intervals=3', 'tpos=Up' ])
  
  test('iterate values',
    executable('iter', 'iter.c',
      c_args: test_defines,
      dependencies: plot),
    args: [ '1', 'fact 2e-3 3 2 1', 'val= 1 2 3' ])
  
  test('input generator',
    executable('gen', 'gen.c',
      c_args: test_defines,
      dependencies: plot),
    args: [ '9 poly 1 0 0 : -4' ])
endif

if with_io
  if with_shared and with_loader
    test('loader',
      executable('libassign', 'libassign.c',
        c_args: test_defines,
        dependencies: load),
      env: [ 'MPT_PREFIX_LIB=' + join_paths(meson.build_root(), 'mptio') ])
  endif
  test('I/O pipe',
    executable('pipe', join_paths('io', 'pipe.c'),
      c_args: test_defines,
      dependencies: io),
    args: [ '/bin/ls', '..' ])
endif

if with_lua
  test('lua interpreter',
    executable('interp', join_paths('lua', 'main.c'), dependencies: lua),
    env: [ 'MPT_MATHBOX=' + join_paths(meson.source_root(), 'lua', 'mathbox.lua') ],
    args: [ join_paths(meson.current_source_dir(), 'lua', 'user.lua')])
endif
