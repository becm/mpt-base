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
