-- sort库的测试
sort = require "sort"

-- 普通的数组比较，整型数据和字符串的比较
function f (a, b)
  if a > b then return 1
  elseif a == b then return 0
  else return -1
  end
end

local t = {1, 300, 400, 10, 2, 5, 0, -200, -900, -2000}
local c = {"aaa", "kkk", "ccc", "zzz", "bbb", "eee", "qqq", "aac"}


local function Tablelen(t)
  local count = 0
  for i, v in ipairs(t) do
    count = count + 1
  end
  return count
end

local function printTable(t)
  local length = Tablelen(t)
  for i = 1, length do
    io.write(t[i], ",")
  end
end

printTable(t)
print("\n")
--sort.insertsort(t, f)  -- 插入排序
--sort.qksort(t, f)        -- 快速排序
sort.mgsort(t, f)
printTable(t)
print("\n")


printTable(c)
print("\n")
--sort.insertsort(c, f)
--sort.qksort(c, f)
sort.mgsort(c, f)
printTable(c)
print("\n")