--数组的排列组合
--target = 2
function combin(a, n, m, com, single, target)
  if m == 0 or n < m or #a == 0 then 
    if #single == target then
      table.insert(com, #com + 1, single) 
	end
  elseif n == m then 
    table.move(a, 1, #a, #single + 1, single)
    table.insert(com, #com + 1, single)
  else
    local single_non_add = {}
	table.move(single, 1, #single, 1, single_non_add)
	local temp = a[1]
	table.remove(a, 1)
	--创建新的变量空间一定要使用local, 否则会导致不同的递归使用同一个变量的地址空间
	local a_1 = {}; table.move(a, 1, #a, 1, a_1); 
    local a_2 = {}; table.move(a, 1, #a, 1, a_2);	
	combin(a_1, n - 1, m, com, single_non_add, target)
	table.insert(single, #single + 1, temp)
	combin(a_2, n - 1, m - 1, com, single, target)
  end
end


set = {}
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