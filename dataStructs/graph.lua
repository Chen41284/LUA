--根据节点的名称返回对应的节点
function name2node(graph, name)
  local node = graph[name]
  if not node then
    node = {name = name, adj = {}}
	graph[name] = node
  end
  return node
end

--从文件中加载图,增加节点的权重
--[[文件的格式:
 a b 12
 c d 15
 ...
 f g 100
]]
function readgraph(filename)
  local f = assert(io.open(filename, "r"))
  local graph = {}
  for line in f:lines() do
    local namefrom, nameto, label = string.match(line, "(%S*)%s+(%S+)%s+(%d+)")
	local from = name2node(graph, namefrom)
	local to = name2node(graph, nameto)
	--将节点加入邻接集合中
	from.adj[to] = label
  end
  f:close()
  return graph
end

--寻找两个节点之间的路径，深度优先搜索
function findpath(cur, to, visited, path)
  visited = visited or {}                --访问过的结点
  path = path or {}                      --访问的路径
  if visited[cur] then
    return nil                           --已经访问过的
  end
  path[#path + 1] = cur
  visited[cur] = true
  if cur == to then
    return path                          --已经到达终点
  end
  --遍历邻接的集合
  for node in pairs(cur.adj) do
    local p = findpath(node, to, visited, path)
	if p then return p end
  end
  path[#path] = nil                      --将当前路径的节点删除
end

--两个节点之间的最短路径
function dijkstra(g, s, e)
  local unvisited = {}
  local dist = {}
  local current = g[s]
  local last = g[e]
  local prev = {}
  
  for _, j in pairs(g) do unvisited[j] = true end
  for _, j in pairs(g) do dist[j] = math.huge end
 
  dist[current] = 0
  
  while current ~= last do
    for to, label in pairs(current.adj) do
	  if unvisited[to] == true then
	    local ndist = dist[current] + label
		if dist[to] > ndist then
		   dist[to] = ndist
		end
	  end
	end
	
	unvisited[current] = false
	prev[#prev + 1] = current
	current = last
	for i, j in pairs(unvisited) do
	  if j == true and dist[i] < dist[current] then
	     current = i
	  end
	end
   end
   
   prev[#prev + 1] = last
   return prev
end

--打印路径 
function print_path(path)
  for i = 1, #path do
    print(path[i].name)
  end
end