--函数生成器

local set = {}
function combin2(a, n, m, com, cur_n)
   cur_n = cur_n or 1
   if m == 0 then
     local s = {}; table.move(set, 1, #set, 1, s)  --必须添加新表（申请新的内存空间）
	 table.insert(com, #com + 1, s)
   elseif (cur_n > n) or (n - cur_n + 1 < m) then return
   else
     local temp = a[cur_n]
     table.insert(set, #set + 1, temp)
	 combin2(a, n, m - 1, com, cur_n + 1)
	 table.remove(set, #set, temp)
	 combin2(a, n, m, com, cur_n + 1)
  end
end

function combinations(ary, num)
  local combif 
  local com = {}
  combin2(ary, #ary, num, com, 1)
  
  combif = function (com)
    for i, v in ipairs(com) do
	  coroutine.yield(v)
	end
  end
  
  local c = coroutine.create(combif)
  return function ()
    status, v = coroutine.resume(c, com)
	if status then
	  return v
	end
  end
end

function printResult (a)
  for i = 1, #a do io.write(a[i], " ") end
  io.write("\n")
end

for c in combinations({'a', 'b', 'c', 'd'}, 3) do
    printResult(c)
end

