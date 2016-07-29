
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
  local m = io.open(os.getenv('MPT_MATHBOX'))
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
