
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
function mpt.client(dest, w)
  local client = mpt.pipe(dest, '-c-')
  local wait = w
  
  client.encoding = "command"
  
  if not w then wait = 1000 end
  
  -- client process closure
  local function cmd(arg, t)
    if arg then
      client:push(arg)
      client:push()
      client:flush()
    end
    if t == nil then t = wait end
    return client:read(t)
  end
  
  return cmd
end

-- register operations in environment
function mpt.setup(c, g)
  local client = c
  local out = io.output()
  local env = g
  
  if not env then env = _G end
  
  -- auto-generate client
  if type(c) == "string" then
    client = mpt.client(c)
  end
  
  -- create command
  local function join(cmd, ...)
    local a = {...}
    return cmd..' '..table.concat(a, ' ')
  end
  
  -- print output from passed command
  local function wait(command, tcont)
    local ret = client(command, tcont)
    while ret and #ret > 0 do
      out:write(ret)
      if tcont == nil then return end
      ret = client(nil, tcont)
    end
  end
  
  -- functions with output
  function env.init (...) wait(join('init', ...)) end
  function env.prep (...) wait(join('prep', ...)) end
  function env.start(...) wait(join('start',...)) end
  function env.step (...) wait(join('step', ...)) end
  -- no regular return
  function env.read (...) wait(join('read', ...), 0) end
  function env.cont ()    wait('cont', 0) end
  function env.stop ()    wait('stop', 0) end
  
  -- try to read remaining data
  function env.flush(len)
    if not len then len = 0 end
    wait(nil, len)
  end
end

return mpt
