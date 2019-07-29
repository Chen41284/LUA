--获取文件的大小
function filesize(filename)
  local file = assert(io.open(filename, "r"))
  local current = file:seek()     --保存当前位置
  local size = file:seek("end")   --获取文件大小
  file:seek("set", current)       --回复当前的位置
  file:close()
  return size
end