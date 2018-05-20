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
