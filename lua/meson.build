# mpt-base/lua/meson.build
# build definition for mpt LUA library

lua_version = run_command('pkg-config',
  '--variable=major_version',
  'lua' + get_option('lua')).stdout().strip()

lua_cmod = run_command('pkg-config',
  '--define-variable=prefix=' + get_option('prefix'),
  '--variable=INSTALL_CMOD',
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

if with_shared
  lua = shared_library('lua' + lua_version + '-mpt', 'mpt.c',
    install_rpath : get_option('prefix') + '/' + get_option('libdir'),
    c_args : lua_inc,
    include_directories : inc,
    link_with : mptcore,
    install_dir : lua_cmod,
    install : true)
endif
