-- 沙盒 Sandbox
-- 一个使用钩子的简单沙盒
local debug = require "debug"

-- 设置授权的函数
local validfunc = {
  [string.upper] = true,
  [string.lower] = true,
  -- ... 其他的授权的函数
}

-- 最大能够执行的"step"
local steplimit = 1000

-- 控制使用的内存
-- 最大能够使用的内存（单位KB）
local memlimit = 1000

local function checkmem ()
  if collectgarbage("count") > memlimit then
    error("script uses too much memory")
  end
end

local count = 0 -- 计数器

local function step()
  checkmem()
  count = count + 1
  if count > steplimit then
    error("script uses too much CPU")
  end
end

local function hook (event)
  if event == "call" then
    local info = debug.getinfo(2, "fn")
	if not validfunc[info.func] then
	  error("calling bad function: " .. (info.name or "?"))
	end
  end
  count = count + 1
  if count > steplimit then
    error("script uses too much CPU")
  end
end

-- 加载
local f = assert(loadfile(arg[1], "t", {}))

debug.sethook(step, "", 100) -- 设置钩子

