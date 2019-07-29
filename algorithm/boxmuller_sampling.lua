--Box-Muller法生成正太分布（高斯分布）的随机数

function boxmuller_sampling(mu, sigma)
  local u = math.random()
  local v = math.random()
  local z = math.sqrt(-2.0 * math.log(u)) * math.cos(2 * math.pi * v)
  if nil == mu then mu = 0 end
  if nil == sigma then sigma = 1 end
  
  return z * sigma + mu
end