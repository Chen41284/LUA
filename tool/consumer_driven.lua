--消费者驱动的设计
function receive (prod)
  local status, value = coroutine.resume(prod)
  return value
end

function send (x)
  coroutine.yield(x)
end

function producer ()
  return coroutine.create(function ()
    repeat
	  local x = io.read()
	  send(x)
	until x == nil
  end)
end

function filter (prod)
  return coroutine.create(function ()
    for line = 1, math.huge do
	  local x = receive(prod)
	  if x == nil then break end
	  x = string.format("%5d %s", line, x)
	  send(x)
	end
  end)
end

function consumer (prod)
  while true do
    local x = receive(prod)
	if x == nil then break end
	io.write(x, "\n")
  end
end

--consumer(producer())
consumer(filter(producer()))
