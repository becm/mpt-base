# mpt-base/examples/meson.build
# tests for MPT base modules

test_defines = [ '-DMPT_INCLUDE(x)=#x' ]

if with_core
  subdir('core')
endif

if with_cxx
  subdir('cxx')
endif

cdir = meson.current_source_dir()

if with_plot
  test('axis data',
    executable('axis', cdir + '/axis.c',
      c_args: test_defines,
      dependencies: plot),
    args: [ 'title=my axis', 'intervals=3', 'tpos=Up' ])
  
  test('iterate values',
    executable('iter', cdir + '/iter.c',
      c_args: test_defines,
      dependencies: plot),
    args: [ '1', 'fact 2e-3 3 2 1', 'val= 1 2 3' ])
  
  test('input generator',
    executable('gen', cdir + '/gen.c',
      c_args: test_defines,
      dependencies: plot),
    args: [ '9 poly 1 0 0 : -4' ])
endif

if with_io
  test('I/O pipe',
    executable('pipe', cdir + '/io/pipe.c',
      c_args: test_defines,
      dependencies: io),
    args: [ '/bin/ls', '..' ])
endif

if with_lua
  test('lua interpreter',
    executable('interp', cdir + '/lua/main.c', dependencies: lua),
    env: [ 'MPT_MATHBOX=' + meson.source_root() + '/lua/mathbox.lua' ],
    args: [ cdir + '/lua/user.lua'])
endif
