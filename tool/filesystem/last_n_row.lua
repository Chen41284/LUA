#!/usr/bin/lua

function last_n_row(tail_lines, filepath)
	-- 一次读取512字节数据
	read_byte_once = 512
	offset = 0
	fp = io.open(filepath,"r")
	if fp == nil then
		print("open file "..filepath.." failed.")
		--os.exit(0)
		return
	end
	line_num = 0
	while true do
		-- 每次偏移read_byte_once字节
		offset = offset - read_byte_once
		-- 以文件尾为基准偏移offset
		if fp:seek("end",offset) == nil then
			-- 偏移超出文件头后将出错,这时如果是第一次读取的话,直接将文件指针偏移到头部,否则(第二或是之后）跳出循环输出所有内容
			if offset + read_byte_once == 0 then
				fp:seek("set")
			else
				break    --offset第二或是第三次之后不是-512
			end
		end
		data = fp:read(read_byte_once)
		-- 倒转数据,方便使用find方法来从尾读取换行符
		data = data:reverse()
		index = 1
		while true do
			-- 查找换行符
			start = data:find("\n",index, true)  --从上一次的index开始寻找，"true"使用简单搜索
			if start == nil then
				break
			end
			-- 找到换行符累加
			line_num = line_num + 1
			-- 找到足够换行符
			if tail_lines + 1 == line_num then
				-- 偏移文件符指针到第line_num个换行符处
				fp:seek("end",offset+read_byte_once-start+1)
				io.write(fp:read("*all"))
				fp:close()
				--os.exit(0)
				return
			end
			index = start + 1
		end
	end

	-- 找不到足够的行,就输出全部
	fp:seek("set")
	io.write(fp:read("*all"))
	fp:close()
end	