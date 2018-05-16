SinOsc = {phase=0,freq=440,amp=1}
SinOsc.__index = SinOsc
function SinOsc.new()
  local o = {}
  setmetatable(o,SinOsc)
  return o
end
function SinOsc:Process(samplerate)
  self.phase=self.phase+(1.0*math.pi*self.freq/samplerate)
  return math.sin(self.phase)*self.amp;
end

oscs={}
freqs={}
for i=1,80 do
  local osc=SinOsc.new()
  osc.freq=math.pow(math.random(),2)*18000
  table.insert(freqs,osc.freq)
  table.insert(oscs,osc)
end


modmodulator=SinOsc.new()
modmodulator.freq=1/4
modulator=SinOsc.new()
modulator.freq=0.1
function __process(sr)
  local sum=0
  modmodval=modmodulator:Process(48000)*0.5+0.5;
  modulator.freq=math.pow(modmodval,2.5)*300
  modval=modulator:Process(48000)*0.5+0.55
  for i,osc in ipairs(oscs) do
    osc.freq=freqs[i]*modval;
    sum=sum+osc:Process(48000);
  end
  return sum/#oscs;
end
