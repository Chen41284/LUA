--八王后问题
N = 8   --棋盘的大小
count = 0  --统计isplaceok的次数
solution = 0

--检查当前行是否可以放置, (n,c)位置是否会被攻击
function isplaceok(a, n, c) 
    count = count + 1
	for i = 1, n - 1 do
		if (a[i] == c) or              --同一列
		   (a[i] - i == c - n) or      --同一对角线,135度
		   (a[i] + i == c + n) then    --同一对角线,45度
		   return false
		end
	end
	return true
end

--打印八王后的棋盘
function printsolution(a, f)
  solution = solution + 1
  f:write("solution ", solution, "\n")
  for i = 1, N do   --对于每一行
	 for j = 1, N do  --对于每一列
	    --输出'X'或是'-'，外加一个空格
		f:write(a[i] == j and "X" or "-", " ")
	 end
   f:write("\n")
  end
  f:write("count ", count, "\n")
  f:write("\n")
end

--把'n'到'N'的所有王后放到棋盘上
function addqueen(a, n, filename)
  local f = assert(io.open(filename, "a+"))
  if n > N then   --判断是否所有的王后都放置好了
    printsolution(a, f)
  else  --尝试放置第n个王后
    for c = 1, N do
	  if isplaceok(a, n, c) then
	    a[n] = c    --把第n个王后放置到列c上
		addqueen(a, n + 1, filename)
	   end -- if
	end --for
  end --else
  f:close()
end -- function


--1~n的全排列
function permutation(a, n, filename)
   local f = assert(io.open(filename, "a+"))
   permu(a, 1, n,  f)
   f:close() 
   print("end")
end

function swap(a, b)
  local temp = a
  a = b
  b = temp
  return a, b
end


--1~8之间的全排列
solution2 = 0
count2 = 0
function permu(a, n, H, f)
  if n > H then
    for i = 2, H do
	  count2 = count2 + 1
	  if (isplaceok(a, i, a[i]) == false) then
	    return
	  end
	end
	for i = 1, H do
	  f:write(a[i], " ")
	end
	f:write("\n")
	solution2 = solution2 + 1
	f:write("solution ", solution2, "\n")
	for i = 1, H do
	  for j = 1, H do
	  f:write(a[i] == j and "X" or "-", " ")
	  end
	  f:write("\n")
	end
	f:write("count " , count2, "\n")
	f:write("=====================\n")
	return
  end
  for i = n, H do
    local temp = a[i]
	a[i] = a[n]
	a[n] = temp
	
	permu(a, n + 1, H,  f)
	
	temp = a[i]
	a[i] = a[n]
	a[n] = temp
  end
end