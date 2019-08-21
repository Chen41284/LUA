-- 设置变量的值
require "getvarvalue"

function setvarvalue (name, value, level, isenv)
  local found = false
  
  level = (level or 1) + 1
  
  local index = 0
  -- 尝试局部变量
  for i = 1, math.huge do
    local n, v = debug.getlocal(level, i)
	if not n then break end
	if n == name then
	  index = i
	  found = true
	end
  end
  
  if found then
    local n = debug.setlocal(level, index, value)   
    return "set local ", name , value 
  end
  
  -- 尝试非局部的变量
  local func = debug.getinfo(level, "f").func
  for i = 1, math.huge do
    local n, v = debug.getupvalue(func, i)
	if not n then break end
	if n == name then	
	  return "upvalue", v 
	end
  end
  
  -- 设置全局环境中的值
  if isenv then return "noenv" end  --避免循环
  
  local _, env = setvarvalue("_ENV", value, level, true)
  env[name] = value  --修改全局变量的值
  if env then
    return "set global", name, value
  else
    return "noenv"
  end
end

local a = 4; print(setvarvalue("a", 5)); print(getvarvalue("a"))
b = "AA"; print(setvarvalue("b", "BB")); print(getvarvalue("b"))
