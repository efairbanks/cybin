require "main"
--[[
time=0
function __process(sr)
  time=time+440/sr
  return math.random()
  --return math.sin(math.pi*time)
end
--]]
