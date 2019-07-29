function split (s, p)
    local rt= {}
	local count = 1
    string.gsub(s, '[^'..p..']+', 
	              function(w) 
				  --table.insert(rt, w) 
				  rt[count] = w 
				  count = count + 1
				  end )  --p的补集
    return rt
end