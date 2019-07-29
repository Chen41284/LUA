--计算导数
function derivative(f, delta)
  delta = delta or 1e-4
  return function (x)
            return (f(x + delta) - f(x)) / delta
	     end
end