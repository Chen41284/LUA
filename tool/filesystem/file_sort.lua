--对文件进行排序的程序
function file_sort(infile, outfile)
    --判断outfile是否已经存在
	local f = assert(io.open(outfile, "r"))
	if nil ~= f then
	   io.write("write to " , outfile, ": yes or not\n")
	   local s = io.read()
	   if s == "not" then f:close(); return; end
	end
	f:close()
	    
    --打开文件的输入输出流
    if nil ~= infile then 
	   stdIn = io.input()         --保存当前输入流
	   io.input(infile)           --打开一个新的输入流
	end
	
	if nil ~= outfile then
	  stdOut = io.output()
	  io.output(outfile)
	end
	
	--对文件中的内容进行排序
	local lines = {}
	
	for line in io.lines() do
	  lines[#lines + 1] = line
	end
	
	table.sort(lines)
	
	for i, line in ipairs(lines) do
	  io.write(line, "\n")
	end
	
	
	--关闭文件的输入输出流，并恢复标准输入输出流
	if nil ~= infile then
		io.input():close()         --关闭当前的输入流
		io.input(stdIn)             --回复当前的输入流
	end
	
	if nil ~= outfile then
	   io.output():close()
	   io.output(stdOut)
	end
end