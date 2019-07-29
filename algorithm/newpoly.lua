--返回指定系数的多项式函数
function newpoly (t)
  return  function (x)
            local sum = 0
	        for i = #t, 2, -1 do
	          sum = (sum + t[i]) * x
	        end
	        sum = sum + t[1]
	        return sum
          end
end