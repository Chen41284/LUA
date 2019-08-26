-- XML解析器
local lxp = require "lxp"

local count = 0

local function tablelen (t)
  local len = 0
  for k, v in pairs(t) do
    len = len + 1
  end
  return len
end



callbacks = {
  StartElement = function (parser, tagname, atts)
    io.write("+ ", string.rep(" ", count), tagname)
	if tablelen(atts) > 0 then
	  io.write("  ", string.rep(" ", count + 1))
	  for k, v in pairs(atts) do
	    io.write(k, " = ", v, ",")
	  end
	  io.write("\n")
	else io.write("\n") end
	count = count + 1
  end,
  
  CharacterData = function (parser, chars)
     count = count + 1
	 io.write("  ", string.rep(" ", count), chars, "\n")
	 count = count - 1
  end,
  
  EndElement = function (parser, tagname)
    count = count - 1
	io.write("- ", string.rep(" ", count), tagname, "\n")
  end,
}

p = lxp.new(callbacks)  -- 创建新的解析器

local f = assert(io.open("test.xml", "r"))


for l in f:lines() do  -- 迭代输入器
  assert(p:parse(l))    -- 解析一行
  assert(p:parse("\n")) -- 增加换行符
end

assert(p:parse(l))      -- 解析文档
p:close()               -- 关闭解析器

f:close()

p:close()

