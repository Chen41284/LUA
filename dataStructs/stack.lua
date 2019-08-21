--Stack类
Stack = {pointer = 0}

function Stack:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function Stack:push(v)
  self.pointer = self.pointer + 1
  self[self.pointer] = v
end

function Stack:pop()
  if self:isempty() then return nil end
  local pointer = self.pointer
  local v = self[pointer]
  self[pointer] = nil
  self.pointer = pointer - 1
  return v
end

function Stack:top()
  return self[self.pointer]
end

function Stack:size()
  return self.pointer
end

function Stack:isempty()
  return self.pointer == 0
end

StackQueue = Stack:new({bottomPointer = 0 })

function StackQueue:isempty()
  return self.bottomPointer == self.pointer
end

function StackQueue:insertbottom (v)
  self[self.bottomPointer] = v
  self.bottomPointer = self.bottomPointer - 1
end

function StackQueue:size()
  return self.pointer - self.bottomPointer
end

function StackQueue:bottom()
  return self[self.bottomPointer + 1]
end
