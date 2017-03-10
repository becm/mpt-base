
-- test mpt loading
function testmpt()
  if not mpt then
    mpt = require('mpt')
    print("loaded mpt module")
  end
  local n = os.tmpname()
  local s = mpt.open(n, 'w')
  local t = n..n
  
  s.encoding = 'cobs'
  s:push(t)
  s:push()
  s:flush()
  
  s = mpt.open(n, 'r')
  s = s:read()
  
  os.remove(n)
  
  return s == mpt.cobs(t)
end

-- test math loading
function testmath(...)
  if not mbox then
    mbox = require('mathbox')
  end
  
  for i,v in pairs({...}) do
    c, e = loadfile(v, "t", mbox)  -- Lua < 5.2 ignores mode and env arguments
    if c then
      if setfenv then setfenv(c, mbox) end  -- Lua < 5.2 environment assignment
      c()
    else
      print("# " .. e)
    end
  end
end
