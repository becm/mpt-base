-- mathbox.lua
-- restricted environment with math operations

local mbox = math

-- set number format
mbox.format = {}
-- mbox.format.dec=10
-- mbox.format.type='e'
setmetatable(mbox.format, {
  __call = function(self, ...)
      local ret = ''
      local dec = self.dec
      local fmt = self.type
      local width = self.width
      
      if width == nil then width = 0 end
      if dec == nil then dec = 6 end
      if fmt == nil then fmt = 'e' end
      local fmt = '%' .. width .. '.' .. dec .. fmt
      ret = {}
      for i, v in pairs({...}) do
        ret[i] = string.format(fmt, v)
      end
      return ret
  end
})

-- print formated line
mbox.push = function()
  local out = io.write
  local nl  = '\n'
  local fmt = mbox.format
  return function(...)
    out(table.concat(fmt(...), ' '))
    out(nl)
  end
end

mbox.push = mbox.push()

return mbox
