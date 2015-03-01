
-- load mpt package

local function loadmpt()
  mpt = require('mpt')
end

if not pcall(loadmpt) then
  print('unable to load mpt')
end

-- test mpt loading
function testmpt()
  local n = os.tmpname()
  local s = mpt.open(n, 'w')
  
  s.encoding = 'cobs'
  s:push('hallo')
  s:push()
  s:flush()
  
  s = mpt.open(n, 'r')
  s = s:read()
  
  os.remove(n)
  
  return s == mpt.cobs('\x04\x20hallo')
end

-- test math loading
function testmath(...)
  local m = io.open('mathbox.lua')
  m = m:read('*a')
  
  for i,v in pairs({...}) do
    v = io.open(v)
    c = m .. v:read('*a')
    c, e = loadstring(c)
    if c then
      pcall(c)
    else
      print(e)
    end
  end
end

-- show table recursively
function rprint(t, ind)
  local seen = {}
  if ind == nil then
    ind = ' '
  end
  
  function rec(t, pre, name)
    for i,v in pairs(t) do
      if type(v)=="table" then
        if seen[v] then
          print(pre .. i .. " > <" .. seen[v] .. ">")
        else
          print(pre .. i, v)
          curr = name .. i
          seen[v] = curr
          rec(v, pre .. ind, curr .. '.')
        end
      else
        print(pre .. tostring(i), v)
      end
    end
  end
  
  print(t)
  if type(t)=="table" then
    seen[_G] = '%global'
    seen[t] = ''
    rec(t, ind, '')
  end
end
