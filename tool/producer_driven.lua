--生产者驱动的设计
function receive ()
  local value = coroutine.yield()
  return value
end

function send(cons, x)
  coroutine.resume(cons, x)
end

function producer (cons)
  coroutine.resume(cons)   --启动消费者，进入循环等待
  repeat
    local x = io.read()
	send(cons, x)
  until x == nil
end

function filter (cons)
  coroutine.resume(cons)
  return coroutine.create(function ()
    for line = 1, math.huge do
	  local x = receive()
	  if x == nil then break end
	  x = string.format("%5d %s", line, x)
	  send(cons, x)
	end
  end)
end

function consumer () 
  return coroutine.create(function ()
    while true do
      local x = receive()
	  if x == nil then break end
	  io.write(x, "\n")
    end
  end)
end

--producer(consumer())
producer(filter(consumer()))