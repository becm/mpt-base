# mpt-base/lua/meson.build
# build definition for mpt LUA library

# need to modify target prefix
lua_lmod = run_command('pkg-config',
  '--define-variable=prefix=' + get_option('prefix'),
  '--variable=INSTALL_LMOD',
  'lua' + get_option('lua')).stdout().strip()

# get_compile_args() not exported by dependency object
lua_cflags = run_command('pkg-config',
  '--cflags',
  'lua' + get_option('lua')).stdout().strip()

# use pkgconfig wrapper if available
if meson.version() >= '0.36'
  lua_version = lua.get_pkgconfig_variable('major_version')
else
  lua_version = run_command('pkg-config',
    '--variable=major_version',
    'lua' + get_option('lua')).stdout().strip()
endif

if not meson.is_subproject()
  install_data('mpt.lua', install_dir : lua_lmod)
endif

if with_shared and with_io
  # shared_module() does not allow versioning and links libc by default anyway
  lua_lib = shared_library('lua' + lua_version + '-mpt', 'mpt.c',
    version : '1.0.0', soversion : '1',
    install_rpath : '$ORIGIN',
    c_args : lua_cflags,
    dependencies : io,
    link_args : ['-Wl,--unresolved-symbols=ignore-in-object-files'],
    install : with_lua)
  
  lua = [
    declare_dependency(link_with : lua_lib),
    lua
  ]
endif
