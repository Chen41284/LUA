--函数的积分
function integral(f, low, up, step)
  local step = step or 1e-4
  local int = 0
  for i = low, up, step do
    int = int + f(i) * step
  end
  return int
end