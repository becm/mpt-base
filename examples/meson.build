# mpt-base/examples/meson.build
# tests for MPT base modules

test_defines = [ '-DMPT_INCLUDE(x)=#x' ]

# location of dynamic loaded library
dir_libs_test = join_paths(meson.current_build_dir(), '..', 'mptplot')
# location of MPT Lua modules
dir_lua_modules = join_paths(meson.current_source_dir(), '..', 'lua')

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
    args: [ '1', 'fact(3:2e-2::1)', 'lin(4 : 1 2)' ])
  
  test('input generator',
    executable('gen', 'gen.c',
      c_args: test_defines,
      dependencies: plot),
    args: [ '9', 'poly 1 0 0 : -4', 'lin 1 0' ])
    
  if with_cxx
    test('graph binding',
      executable('graph', 'graph.cpp',
        cpp_args: test_defines,
        dependencies: cxx),
      args: [ 'w1 w2', 'ax' ])
    
    test('graphic updates',
      executable('graphic', 'graphic.cpp',
        cpp_args: test_defines,
        dependencies: cxx))
  endif
endif

if with_shared and with_loader
  if with_plot
    test('loader',
      executable('libassign', 'libassign.c',
        c_args: test_defines,
        dependencies: load),
      # use runtime from meson build location
      env: [ 'MPT_PREFIX_LIB=' + dir_libs_test ])
  endif
endif

if with_io
  test('I/O pipe',
    executable('pipe', join_paths('io', 'pipe.c'),
      c_args: test_defines,
      dependencies: io),
    args: [ '/bin/ls', '..' ])
endif

if with_lua
  test('lua interpreter',
    executable('interp' + lua_version, join_paths('lua', 'main.c'),
      c_args: test_defines + [ '-D__MPT_BUILD_LUA_PATH="' + dir_lua_modules + '"' ],
      dependencies: lua),
    args: [ join_paths(meson.current_source_dir(), 'lua', 'user.lua')])
endif
