-- 下载Web页面的函数
-- 使用LuaSocket库
local socket = require "socket"
function download (host, file)
  local c = assert(socket.connect(host, 80))
  local count = 0  -- 计算读取的字节数
  local request = string.format(
      "GET %s HTTP/1.0\r\nhost:%s\r\n\r\n", file, host)
  c:send(request)
  while true do 
    local s, status = receive(c)
	count = count + #s 
	if status == "closed" then break end
  end
  c:close()
  print(file, count)
end

function receive (connection)
  connection:settimeout(0)
  local s, status, partial = connection:receive(2^10)
  if status == "timeout" then
    coroutine.yield(connection)
  end
  return s or partial, status
end

tasks = {}  -- 所有活跃任务的列表

function get(host, file)
  -- 为任务创建协程
  local co = coroutine.wrap(function ()
    download(host, file)
  end)
  -- 将其插入列表
  table.insert(tasks, co)
end

function dispatch ()
  local i = 1
  local timedout = {}
  while true do
    if tasks[i] == nil then  -- 没有其他的任务了？
	  if tasks[1] == nil then
	    break
	  end
	  i = 1
	  timedout = {}
	end
	local res = tasks[i]()  -- 运行一个任务
	if not res then
	  table.remove(tasks, i)
	else
	  i = i + 1
	  timedout[#timedout + 1] = res
	  if #timedout == #tasks then  -- 所有任务都阻塞了？
	    socket.select(timedout) -- 等待
      end
	end
  end
end

get("www.lua.org", "/ftp/lua-5.3.5.tar.gz")
get("www.lua.org", "/ftp/lua-5.3.4.tar.gz")
get("www.lua.org", "/ftp/lua-5.3.3.tar.gz")
get("www.lua.org", "/ftp/lua-5.3.2.tar.gz")
get("www.lua.org", "/ftp/lua-5.3.1.tar.gz")
get("www.lua.org", "/ftp/lua-5.3.0.tar.gz")
get("www.lua.org", "/ftp/lua-5.2.4.tar.gz")
get("www.lua.org", "/ftp/lua-5.2.3.tar.gz")

local t = os.clock()
dispatch()
print("execute time :", os.clock() - t)