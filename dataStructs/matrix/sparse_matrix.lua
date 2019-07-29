--稀疏矩阵
function sparse_matrix(M, N)
  local sm = {}
  for i = 1, M do
    local row = {}
	for j = 1, N do
	  row[j] = nil
	end
	sm[i] = row
  end
  
  return sm
end


--打印稀疏矩阵(以0填充)
function print_sparse_full(mat, M, N)
  for i = 1, M do
    for j = 1, N do
	  if mat[i][j] == nil then
	    io.write("0 ")
	  else
	    io.write(mat[i][j], " ")
      end
	end
	io.write("\n")
  end
end

--稀疏矩阵的相乘
function sparse_matrix_mul(a, b)
  local c = {}
  for i = 1, #a do
    local resultline = {}          --va和vb都是非空的，即不存在nil与数值相乘
    for k, va in pairs(a[i]) do           --va == a[i][k]
	  for j, vb in pairs(b[k]) do         --vb == b[k][j]
	    local res = (resultline[j] or 0) + va * vb
		resultline[j] = (res ~= 0) and res or nil
	  end
	end
	c[i] = resultline
  end
  return c
end

--稀疏矩阵相加
function sparse_matrix_add(a, b)
  local c = {}
  for i = 1, #a do
    local resultline = {}
	--添加a的每行元素
	for k, va in pairs(a[i]) do
	  resultline[k] = va
	end
	--添加b的每行元素
	for k, vb in pairs(b[i]) do
	  resultline[k] = (resultline[k] or 0 ) + vb
	end
    c[i] = resultline
  end
  return c
end

--打印稀疏矩阵，打印非零值的行号，列好以及相应的值
function print_sparse(mat, M, N)
  for i = 1, M do
    for k, value in pairs(mat[i]) do
	  io.write("(", i, ", ", k , ")  ", value, "\n")
	end
  end
  