-- 获取变量的值
function getvarvalue (name, level, isenv)
  local value
  local found = false
  
  level = (level or 1) + 1
  
  -- 尝试局部变量
  for i = 1, math.huge do
    local n, v = debug.getlocal(level, i)
	if not n then break end
	if n == name then
	  value = v
	  found = true
	end
  end
  if found then return "local", value end
  
  -- 尝试非局部变量
  local func = debug.getinfo(level, "f").func
  for i = 1, math.huge do
    local n, v = debug.getupvalue(func, i)
	if not n then break end
	if n == name then 
	  return "upvalue", v 
	end
  end
  
  if isenv then return "noenv" end  -- 避免循环
  
  -- 没找到，从环境中获取值
  local pos, env = getvarvalue("_ENV", level, true) -- env是一个表
  if env then
    return "global", env[name]
  else
    return "noenv"
  end
end

--local a = 4; print(getvarvalue("a"))
--b = "xx"; print(getvarvalue("b"))