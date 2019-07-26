--图形的点的操作
--计算两个3D点之间的欧拉距离
function distance_point(a, b)
  return math.sqrt((a[1] - b[1])^2 + (a[2] - b[2])^2 + (a[3] - b[3])^2)
end

