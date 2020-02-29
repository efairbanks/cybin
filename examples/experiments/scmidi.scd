SuperDirt.start;
MIDIClient.init;
~midiOut = MIDIOut.newByName("Midi Through","Midi Through Port-0")
~dirt.soundLibrary.addMIDI(\midi, ~midiOut);
