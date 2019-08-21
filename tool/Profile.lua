-- 调优

local Counters = {}  -- 将函数和它们的调用计算关联起来
local Names = {}  -- 关联函数和函数名

-- 用于计算调用次数的钩子
local function hook ()
  local f = debug.getinfo(2, "f").func 
  local count = Counters[f]
  if count == nil then  -- 'f'第一次被调用
    Counters[f] = 1
	Names[f] = debug.getinfo(2, "Sn")
  else   -- 递增计数器
    Counters[f] = count + 1
  end
end

function getname (func)
  local n = Names[func]
  if n.what == "C" then
    return n.name
  end
  local lc = string.format("[%s]:%d", n.short_src, n.linedefined)
  if n.what ~= "main" and n.namewhat ~= "" then
    return string.format("%s (%s)", lc, n.name)
  else
    return lc
  end
end

for func, count in ipairs(Counters) do
  print(getname(func), count)
end

