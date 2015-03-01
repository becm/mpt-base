-- mathbox.lua
-- create restricted environment with math operations

-- setup like lua online demo
arg=nil
debug.debug=nil
debug.getfenv=getfenv
debug.getregistry=nil
dofile=nil
io={write=io.write}
loadfile=nil
os.execute=nil
os.getenv=nil
os.remove=nil
os.rename=nil
os.tmpname=nil
package.loaded.io=io
package.loaded.package=nil
package=nil
require=nil
io = nil
os = nil

-- make math members top level elements
for i,v in pairs(math) do
  _G[i] = v
end

-- set number format
math.format = {}
-- math.format.dec=10
-- math.format.type='e'
setmetatable(math.format, {
  __call = function(self, ...)
      local ret = ''
      local dec = self.dec
      local fmt = self.type
      
      if dec == nil then dec = 10 end
      if tp  == nil then fmt = 'e' end
      local fmt = '% .' .. dec .. fmt
      for i, v in pairs({...}) do
        ret = ret .. ' ' .. string.format(fmt, v)
      end
      return ret
  end
})

-- print formated line
function push(...)
  print(math.format(...))
end
