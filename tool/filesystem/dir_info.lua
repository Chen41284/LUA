--获取当前的目录下的文件
function dir_info()
  local f = io.popen("dir /B", "r")
  local dir = {}
  for entry in f:lines() do
    local new_name = string.gsub(entry, ".dat", ".txt")
	os.rename(entry, new_name)
    dir[#dir + 1] = entry
  end
  require("function")
  print_table(dir)
end