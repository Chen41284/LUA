--数组打乱顺序
function shuffle(t)
 if type(t) ~= "table" then return end
 tab = {}
 index = 1
 repeat
   local n = math.random(#t)
   if n ~= nil then
     tab[index] = t[n]
     table.remove(t, n)
	 index = index + 1
   end
 until #t == 0
 table.move(tab, 1, #tab, 1, t)
end