# mpt-base/lua/meson.build
# build definition for mpt LUA library

lua_version = run_command('pkg-config',
  '--variable=major_version',
  'lua' + get_option('lua')).stdout().strip()

lua_lmod = run_command('pkg-config',
  '--define-variable=prefix=' + get_option('prefix'),
  '--variable=INSTALL_LMOD',
  'lua' + get_option('lua')).stdout().strip()

lua_inc = run_command('pkg-config',
  '--cflags',
  'lua' + get_option('lua')).stdout().strip()

install_data('mathbox.lua', 'mpt.lua',
  install_dir : lua_lmod)

if with_shared and with_io
  lua_lib = shared_library('lua' + lua_version + '-mpt', 'mpt.c',
    version : '1.0.0', soversion : '1',
    install_rpath : '$ORIGIN',
    c_args : lua_inc,
    include_directories : include_directories('../mptcore'),
    link_with : io_lib,
    link_args : ['-Wl,--unresolved-symbols=ignore-in-object-files'],
    install : with_lua)
  
  lua = [
    declare_dependency(link_with : lua_lib),
    dependency('lua' + get_option('lua'))
  ]
else
  with_lua = false
endif
