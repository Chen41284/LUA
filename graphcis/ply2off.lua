--.ply格式的文件转化为.off
--均为三角网格，具有x,y,z坐标，每个面均只有三个顶点

--判断ply是那种格式
function ply2off(file_ply, file_off)
  local f = assert(io.open(file_ply, "rb"))
  
  --读取开头的32个字节(字符)，用于判断PLY的格式
  local blocksize = 32
  local ply_header = f:read(blocksize)
  if string.match(ply_header, "ascii") then
    ply2off_ascii(file_ply, file_off)
  elseif string.match(ply_header, "binary_little_endian") then
    ply2off_binary(file_ply, file_off, "<")
  elseif string.match(ply_header, "binary_big_endian")  then
    ply2off_binary(file_ply, file_off, ">")
  else
    error("Unkown ply format")
  end
  
  f:close()
end

--ascii
function ply2off_ascii(file_ply, file_off)
  local ply = assert(io.open(file_ply, "r"))
  local off = assert(io.open(file_off, "w"))
  
  --逐行读取PLY的头文件直到 end_header
  local face = 0
  local vertex = 0
  for line in ply:lines() do
     if line == "end_header" then break end
	 if string.match(line, "element vertex (%d+)") then
	    vertex = string.match(line, "element vertex (%d+)")
	 end
	 if string.match(line, "element face (%d+)") then
	    face = string.match(line, "element face (%d+)")
	 end
  end
  
  local data = ply:read("a")   --读取剩下的所有数据元素
  off:write("OFF\n")
  off:write(vertex, " ",  face, " ", 0, "\n")
  off:write(data)
  
  ply:close()
  off:close()
end


--判断PLY头文件中数据的类型，返回相应的string.pack的解析类型
function type2stringpack(s)
    if s == "float" then return "f"
	elseif s == "double" then return "d"
	elseif s == "uchar" then return "B"
	elseif s == "char" then return "b"
	elseif s == "int" then return "i"
	elseif s == "unsigned long int" then return "L"
	elseif s == "long" then return "l"
	elseif s == "size_t" then return "T"
	else error("unknown type")
	end
end



--binary_little_endian
function ply2off_binary(file_ply, file_off, endian)
   
  local ply = assert(io.open(file_ply, "rb"))
  local off = assert(io.open(file_off, "w"))
  
  local blocksize = 512   --读取512个字符，用于判断文档的结尾以及vertex和face出现的位置
  local header = ply:read(blocksize)
  local vertex = string.match(header, "element vertex (%d+)")
  local face = string.match(header, "element face (%d+)")
  local type_xyz = string.match(header, "property (%a+) x")--假设x,y,z的类型均相同
  local type_num, type_list_vertex = string.match(header, "property list (%a+) (%a+) vertex_indices")
  
  --判断x,y,z的格式
  
  local _, position = header:find("end_header") --返回end_header的结尾的位置
  print(position, " ", vertex, " ", face)
  ply:seek("set")   --将重置到开头
  ply:seek("set", position + 1) --将ply移动到数据元素开始的位置
  
  --编写OFF文件的开头
  off:write("OFF\n")
  off:write(vertex, " ", face, " ", 0, "\n")  --边的数量为0
  
  -- endian 端位
  local vertex_bytes = 3 * 4    --每行顶点
  type_xyz = endian .. type2stringpack(type_xyz)
  for i = 1, vertex do
    local raw = ply:read(vertex_bytes)
	local v1 = string.unpack(type_xyz, raw:sub(1, 4))
	local v2 = string.unpack(type_xyz, raw:sub(5, 8))
	local v3 = string.unpack(type_xyz, raw:sub(9, 12))
	off:write(v1, " ", v2, " ", v3, "\n")
  end
  
  local face_bytes = 1 + 3 * 4      --每行面的数量（3, x, x, x),假设为三角网格
  type_num = type2stringpack(type_num)
  type_list_vertex = endian .. type2stringpack(type_list_vertex)  --端位加类型
  for j = 1, face do
    local raw = ply:read(face_bytes)
	local num = string.unpack(type_num, raw:sub(1, 1))
	local f1 = string.unpack(type_list_vertex, raw:sub(2, 5))
	local f2 = string.unpack(type_list_vertex, raw:sub(6, 9))
	local f3 = string.unpack(type_list_vertex, raw:sub(10, 13))
	off:write(num, " ", f1, " ", f2, " ", f3, "\n")
  end
  
  ply:close()
  off:close()
end


