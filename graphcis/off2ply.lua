--off格式的文件转为ply文件
--为了简便，顶点的类型均使用float
--面的数据类型中，每个面的顶点数量类型均为uchar，顶点的编号均为int

PLY_header = [==[
ply
format () 1.0
comment LUA generated PLY File
obj_info vtkPolyData points and polygons: vtk4.0
element vertex %d
property float x
property float y
property float z
element face %d
property list uchar int vertex_indices
end_header
]==]


function off2ply(file_off, file_ply, ply_type, endian)
  if ply_type == nil or string.lower(ply_type) == "ascii" then
    off2ply_ascii(file_off, file_ply)
  elseif string.lower(ply_type) == "binary" then
    if endian == nil or string.lower(endian) == "little" then
	  off2ply_binary(file_off, file_ply, "binary_little_endian", "<")
    elseif string.lower(endian) == "big" then
	  off2ply_binary(file_off, file_ply, "binary_big_endian", ">")
	else
	  error("Unkown endian, \"little\", \"big\" ")
	end
  else
    error("Unkown type")
  end
end


--ascii
function off2ply_ascii(file_off, file_ply)
  local off = assert(io.open(file_off, "r"))
  local ply = assert(io.open(file_ply, "w"))
  
  local header = string.gsub(PLY_header, "%b()", "ascii")

  --读取off文件的开头
  local first = off:read("l")
  if first ~= "OFF" then
    error("none off file")
	return
  end
  local second = off:read("l")
  local vertex, face, edge = string.match(second, "(%d+)%s+(%d+)%s+(%d+)")
  header = string.format(header, vertex, face)
  local data = off:read("a")  --剩下的所有元素数据
  ply:write(header)
  ply:write(data)
  off:close()
  ply:close()

end

--binary
function off2ply_binary(file_off, file_ply, binary_type, endian)
  local off = assert(io.open(file_off, "r"))
  local ply = assert(io.open(file_ply, "wb"))
  
  local header = string.gsub(PLY_header, "%b()", binary_type)
  
  --读取off文件的开头
  local first = off:read("l")
  if first ~= "OFF" then
    error("none off file")
	return
  end
  
  local second = off:read("l")
  local vertex, face, edge = string.match(second, "(%d+)%s+(%d+)%s+(%d+)")
  header = string.format(header, vertex, face)
  ply:write(header)
  
  --顶点数据
  local vertex_pack = endian .. "f f f"
  for i = 1, vertex do
    local v1, v2, v3 = off:read("n", "n", "n")
	ply:write(string.pack(vertex_pack, v1, v2, v3))
  end
  
  --面的数据
  local face_pack = endian .. "B i4 i4 i4"
  for j = 1, face do
    local num, f1, f2, f3 = off:read("n", "n", "n", "n")
	ply:write(string.pack(face_pack, num, f1, f2, f3))
  end
  
  off:close()
  ply:close()
end

