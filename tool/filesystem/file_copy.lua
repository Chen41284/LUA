--文件的复制与拷贝
function file_copy(input, output)
  local In = io.input()
  local Out = io.output()
  io.input(input)
  io.output(output)
  
  
  --按块的内容复制与拷贝
  while true do
    local block = io.read(2^13)     --大小为8KB
	if not block then break end
	io.write(block)
  end
  
  io.input():close()
  io.input(In)
  io.output():close()
  io.output(Out)
end