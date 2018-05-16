SinOsc = {phase=0,freq=440,amp=1}
SinOsc.__index = SinOsc
function SinOsc.new()
  local o={}
  setmetatable(o,SinOsc)
  return o
end
function SinOsc:Process(samplerate)
  self.phase=self.phase+(1.0*math.pi*self.freq/samplerate)
  return math.sin(self.phase)*self.amp;
end
----------------------
----------------------
Line={from=1,to=0,duration=1,delta=-1}
Line.__index = Line
function Line.new()
  local o={}
  setmetatable(o,Line)
  return o
end
function Line:Process(samplerate)
  output=0
  if self.delta>-1 then
    output=self.to*self.delta/self.duration + self.from*(self.duration-self.delta)/self.duration
    self.delta=self.delta+1/samplerate
    if self.delta>self.duration+1/samplerate then self.delta = -1 end
  else
    output=self.to
  end
  return output
end
function Line:Reset()
  self.delta=0
end
----------------------
----------------------
Voice={}
Voice.__index=Voice
function Voice.new()
  local o={}
  setmetatable(o,Voice)
  o.osc=SinOsc.new()
  o.release=Line.new()
  o.attack=Line.new()
  o.attack.from=0
  o.attack.to=1
  o.attack.duration=0.005
  return o
end
function Voice:PlayNote(note)
  self.osc.freq=440*math.pow(2,(note-69)/12);
  self.attack:Reset()
  self.release:Reset()
end
function Voice:Process(sr)
  return self.osc:Process(sr)*self.attack:Process(sr)*self.release:Process(sr)
end
----------------------
----------------------
Synth={}
Synth.__index=Synth
function Synth.new()
  local o={}
  setmetatable(o,Synth)
  o.numVoices=50
  o.voices={}
  o.voiceIndex=1
  for i=1,o.numVoices do
    table.insert(o.voices,Voice.new())
  end
  return o
end
function Synth:PlayNote(note)
  self.voices[self.voiceIndex]:PlayNote(note)
  self.voiceIndex=(self.voiceIndex+1)%self.numVoices
  self.voiceIndex=self.voiceIndex+1
end
function Synth:Process(sr)
  output=0
  for i=1,#(self.voices) do
    output=output+self.voices[i]:Process(sr)
  end
  return output/self.numVoices
end
----------------------
----------------------
env=Line.new()
env.duration=0.07
synth=Synth.new()
time=0
beat=0
notes={7,0,3,0}
function __process(sr)
  cps=4
  delta=1/sr
  beat=math.floor(time)
  if math.floor(time)~=math.floor(time+cps*delta) then
    -- do on-beat stuff
    note=notes[beat%#notes+1]
    offset=(math.floor(beat/8)*5)%12
    synth:PlayNote(note+60+offset)
    if beat%4==2 then synth:PlayNote(note+53+offset) end
    if beat%8==6 or beat%8==0 then synth:PlayNote(offset+36); synth:PlayNote(offset+48) end
    if beat%8==4 then env:Reset() end
    beat=beat+1
  end
  time=time+cps*delta
  return synth:Process(sr)+env:Process(sr)*math.random()*0.05
end
