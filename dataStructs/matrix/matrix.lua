--Lua的数据结构
function matrix(N, M)
  local mt = {}
  for i = 1, N do
    local row = {}
	mt[i] = row
	for j = 1, M do
	  row[j] = 0
	end
  end
  return mt
end

--将数组存储为一维的空间，按索引来提取
function matrix_one_dimension(N, M)
  local mt = {}
  for i = 1, N do
    local aux = (i - 1) * M
	for j = 1, M do
	  mt[aux + j] = 0
	end
  end
  return mt
end

--打印矩阵
function print_matrix(mat)
  local N = #mat
  local M = #mat[1]
  
  for i = 1, N do
    for j = 1, M do
	  io.write(mat[i][j], " ")
	end
	io.write("\n")
  end
end