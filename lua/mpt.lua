
-- get lua version from string
local version = string.sub(_VERSION, 5)

if not _ENV_PREFIX then
  _ENV_PREFIX = os.getenv("LUA_ENV_PREFIX")
end

if not _ENV_PREFIX then
  _ENV_PREFIX = "MPT_"
end

-- environment setup
local platform = os.getenv(_ENV_PREFIX..'ARCH')
local prefix = os.getenv(_ENV_PREFIX..'PREFIX')
local prefix_lib = os.getenv(_ENV_PREFIX..'PREFIX_LIB')

if not prefix then prefix = '/usr/local/mpt' end

if not prefix_lib then
  if platform then prefix_lib = prefix..'/'..platform else
    prefix_lib = prefix.."/lib"
  end
end


-- generic load for lua library
local function load(name)
  local file = 'liblua'..version..'-'..name..'.so'
  name = 'luaopen_'..name
  if prefix_lib then
    local lib, err = package.loadlib(prefix_lib..'/'.. file, name)
    if lib then return lib end
  end
  print(file, name)
  return package.loadlib(file, name)
end


-- load mpt platform package
local mpt, err = load('mpt')
if not mpt then
  error(err)
end
mpt = mpt() -- import symbols from library

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
  
  function e.init(...)
    client.command('init', ...)
    client.wait()
  end
  function e.prep(...)
    client.command('prep', ...)
    client.wait()
  end
  function e.start(...)
    client.command('start',...)
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
  
  -- try to read remaining data
  function e.flush(t)
    if not t then t = 0 end
    client.wait(t)
  end
  
  return client
end

return mpt
