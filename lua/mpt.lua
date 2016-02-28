
-- get lua version from string
local version = _VERSION:sub(5)
local env_prefix = _ENV_PREFIX

-- select environemnt prefix
if not env_prefix then
  env_prefix = os.getenv("LUA_ENV_PREFIX")
end

if not env_prefix then
  env_prefix = "MPT_"
end

-- library loader path
local prefix_lib = os.getenv(env_prefix..'PREFIX_LIB')
if not prefix_lib then
  local platform = os.getenv(env_prefix..'ARCH')
  
  -- prefix or default root
  local prefix = os.getenv(env_prefix..'PREFIX')
  if not prefix then prefix = '/usr/local/mpt' end
  
  if platform then
    -- explicit platform selected
    prefix_lib = prefix..'/lib/'..platform
  else
    -- determine platform string from default shared object location
    local p, a = package.cpath:match('(/usr/lib/([%w_-]+)/lua/'..version..'/%?.so)')
    if p and a then
      prefix_lib = prefix.."/lib/"..a
    else
      -- fallback to generic shared object location
      prefix_lib = prefix.."/lib"
    end
  end
end

-- generic load for lua library
local function load(name, ver, path)
  local file = 'liblua'..version..'-'..name..'.so'
  if ver then file = file..'.'..ver end
  name = 'luaopen_'..name
  if path then
    local lib, err = package.loadlib(path..'/'.. file, name)
    if lib then return lib end
  end
  return package.loadlib(file, name)
end

-- load mpt platform package
local mpt, err = load('mpt', 1, prefix_lib)
if not mpt then
  error(err)
end
mpt = mpt() -- import symbols from library

-- add loading wrapper
function mpt.loadmodule(name, ver, path)
  if not path then path = prefix_lib end
  local pkg, err = load(name, ver, path)
  if not pkg then
    error(err)
  end
  return pkg()
end

-- shortcut to stream create/connect
function mpt.connect(...)
  return mpt.stream():connect(...)
end

-- create client process
function mpt.client(c, w, o)
  local con = c
  local twait = w
  local out = o
  
  if not w then twait = 1000 end
  if not o then out = io.output() end
  
  -- single command submission
  local function command(...)
    con:push('\x04\x00')
    con:push(...)
    con:push()
    con:flush()
  end
  
  -- print output from passed command
  local function wait(t)
    if not t then t = twait end
    local ret = con:read(t)
    while ret and #ret > 0 do
      out:write(ret)
      if t == nil then return end
      ret = con:read(t)
    end
  end
  
  return {
    command = command,
    wait = wait
  }
end

-- register operations in environment
function mpt.setup(c, e)
  local client
  
  if not e then e = _G end
  
  -- auto-generate client
  if type(c) == "string" then
    c = mpt.pipe(c, '-c-')
    if not c then
      error('failed to start client process')
    end
    c.encoding = "command"
  end
  client = mpt.client(c)
  
  -- direct client operations
  function e.init(...)
    client.command('init', ...)
    client.wait()
  end
  function e.prep(...)
    client.command('prep', ...)
    client.wait()
  end
  function e.step(...)
    client.command('step',...)
    client.wait()
  end
  -- no regular return
  function e.read(...)
    client.command('read', ...)
    client.wait(0)
  end
  function e.cont(...)
    client.command('cont', ...)
    client.wait(0)
  end
  function e.stop(...)
    client.command('stop', ...)
    client.wait(0)
  end
  -- combined read,init,prep,cont
  function e.start(...)
    client.command('start', ...)
    client.wait()
  end
  
  -- try to read remaining data
  function e.flush(t)
    if not t then t = 0 end
    client.wait(t)
  end
  
  return client
end

return mpt
