--一个双端队列
function listNew()
  return {first = 0, last = 0, count = 0}
end

--在队列的开头插入元素
function pushfirst(list, value)
  local first = list.first
  list[first] = value
  list.first = first - 1
  if count == 0 then list.last = list.first end
  list.count = list.count + 1
end

--在队列的末尾插入元素
function pushlast(list, value)
  local last = list.last
  list[last] = value
  list.last = last + 1
  if count == 0 then list.first = list.last end
  list.count = list.count + 1
end

--弹出队列开头的元素
function popfirst(list)
  local first = list.first
  if list.count == 0 then error("empty queue") return end
  local value = list[first]
  list[first] = nil         --回收内存空间
  list.first = first + 1
  list.count = list.count - 1
  return value
end

--弹出队列末尾的元素
function poplast(list)
  local last = list.last
  if list.count == 0 then error("empty queue") return end
  local value = list[last]         
  list[last] = nil          --回收内存空间
  list.last = last - 1
  list.count = list.count - 1
  return value
end