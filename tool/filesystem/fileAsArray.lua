--文件的代理
local index = {}

local mt = {
    __index = function (t, k)
	  return t[index][k]
	end,
	
	__newindex = function(t, k, v)
	  t[index][k] = v
	end, 
	
   __len = function (t) return #t[index] end,
   
   __pairs = function(t)
     local tt = t[index]
     return function (tt, k)  --迭代函数
	   local nextkey, nextvalue = next(t[index], k)
	   return nextkey, nextvalue
	 end
   end
  }

function fileAsArray(filename)
  local proxy = {}
  local f = assert(io.open(filename, "r"))
  local filetable = {}
  local i = 0
  
  local v = f:read(1)
  while v do
    i = i + 1
    filetable[i] = v
	v = f:read(1)
  end	
  --为代理创建元表
  proxy[index] = filetable
  setmetatable(proxy, mt)
  f:close()
  return proxy
end