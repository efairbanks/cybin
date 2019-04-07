#ifndef NEWAUDIO_H
#define NEWAUDIO_H

class AudioInterface {
public:
  virtual void SetCallback(float* (*callback)(double,int,int)) = 0;
  virtual void Shutdown() = 0;
  virtual void EventLoop() = 0;
  virtual int GetSampleRate() = 0;
};

class JackAudio : public AudioInterface {
private:
  jack_client_t *_client;
  jack_status_t _status;
  long int _total_processed_frames;
  long int _total_output_frames;
  unsigned int _samplerate;
  int _ringbuffer_frames = 2048;
  jack_ringbuffer_t *_output_ringbuffer;
  jack_ringbuffer_t *_input_ringbuffer;
  std::vector<jack_port_t*> _audio_input_ports;
  std::vector<jack_port_t*> _audio_output_ports;
  std::vector<jack_port_t*> _midi_input_ports;
  std::vector<jack_port_t*> _midi_output_ports;
  float* _extern_input_frame;
  float* _extern_output_frame;
  std::vector<MidiEvent*> _midi_in_events;
  std::vector<MidiEvent*> _midi_out_events;
  std::vector<MidiEvent*> *_extern_midi_in_events;
  std::vector<MidiEvent*> *_extern_midi_out_events;
  pthread_mutex_t _midi_in_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t _midi_out_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t _total_processed_frames_mutex = PTHREAD_MUTEX_INITIALIZER;
  jack_default_audio_sample_t* _output_scratch; // scratch buffer for dumping sets of multichannel frames
  jack_default_audio_sample_t* _input_scratch; // scratch buffer for dumping sets of multichannel frames
  float* (*_callback)(double,int,int);
  static JackAudio* _instance;
  JackAudio() {}
  static double _frames_to_time(long int frames, unsigned int samplerate) {
    return ((double)(frames))/((double)(samplerate));
  }
  static long int _time_to_frames(double time, unsigned int samplerate) {
    return (long int)(time*samplerate);
  }
  static int _process(jack_nframes_t nframes, void *arg) {
    if(pthread_mutex_trylock(&(JackAudio::getInstance()->_midi_in_mutex))==0) {
      for(int midi_port=0;midi_port<JackAudio::getInstance()->_midi_input_ports.size();midi_port++) {
        jack_midi_event_t in_event;
        void* port_buf = jack_port_get_buffer(JackAudio::getInstance()->_midi_input_ports[midi_port], nframes);
        jack_nframes_t event_count = jack_midi_get_event_count(port_buf);
        for(int event_index=0;event_index<event_count;event_index++) {
          jack_midi_event_get(&in_event, port_buf, event_index);
          MidiEvent *e = new MidiEvent();
          e->time=_frames_to_time(JackAudio::getInstance()->_total_output_frames+in_event.time,JackAudio::getInstance()->_samplerate);
          e->port=midi_port;
          for(int i=0;i<in_event.size;i++) e->data.push_back(in_event.buffer[i]);
          JackAudio::getInstance()->_midi_in_events.push_back(e);
        }
      }
      pthread_mutex_unlock(&(JackAudio::getInstance()->_midi_in_mutex));
    }
    // --- //
    std::vector<void*> port_bufs;
    for(int i=0;i<JackAudio::getInstance()->_midi_output_ports.size();i++) {
      void* buf = jack_port_get_buffer(JackAudio::getInstance()->_midi_output_ports[i], nframes);
      jack_midi_clear_buffer(buf);
      port_bufs.push_back(buf);
    }
    if(pthread_mutex_trylock(&(JackAudio::getInstance()->_midi_out_mutex))==0) {
      for(int i=0;i<JackAudio::getInstance()->_midi_out_events.size();i++) {
        MidiEvent* event = JackAudio::getInstance()->_midi_out_events[i];
        long int eventTimeInFrames = _time_to_frames(event->time,JackAudio::getInstance()->_samplerate);
        long int totalOutputFrames = JackAudio::getInstance()->_total_output_frames;
        int midiFrame = eventTimeInFrames-totalOutputFrames;
        if(midiFrame<nframes) {
          unsigned char* buffer =
            jack_midi_event_reserve(
              port_bufs[event->port%JackAudio::getInstance()->_midi_output_ports.size()],
              midiFrame>=0?midiFrame:0,
              event->data.size()
            );
          for(int j=0;j<event->data.size();j++) buffer[j] = event->data[j];
        }
      }
      pthread_mutex_unlock(&(JackAudio::getInstance()->_midi_out_mutex));
    }
    // --- //
    int inputs = JackAudio::getInstance()->_audio_input_ports.size();
    int outputs = JackAudio::getInstance()->_audio_output_ports.size();
    int bytesPerFrame = sizeof(jack_default_audio_sample_t)*outputs;
    int bytesPerFrameset = nframes*bytesPerFrame;
    int inputRingbufferSpace = jack_ringbuffer_write_space(JackAudio::getInstance()->_input_ringbuffer);
    if(jack_ringbuffer_read_space(JackAudio::getInstance()->_output_ringbuffer)<bytesPerFrameset)
      return 0;
    jack_default_audio_sample_t* inScratch = JackAudio::getInstance()->_input_scratch;
    jack_default_audio_sample_t* outScratch = JackAudio::getInstance()->_output_scratch;
    jack_ringbuffer_read(JackAudio::getInstance()->_output_ringbuffer, (char*)outScratch, bytesPerFrameset);
    for(int port=0;port<outputs;port++) {
      jack_default_audio_sample_t *in, *out;
      in = (jack_default_audio_sample_t *)jack_port_get_buffer(JackAudio::getInstance()->_audio_input_ports[port], nframes);
      out = (jack_default_audio_sample_t *)jack_port_get_buffer(JackAudio::getInstance()->_audio_output_ports[port], nframes);
      for(int i=0;i<nframes;i++) {
        inScratch[i*outputs+port]=in[i];
        out[i]=outScratch[i*outputs+port];
      }
    }
    jack_ringbuffer_write(JackAudio::getInstance()->_input_ringbuffer,(char*)inScratch,inputRingbufferSpace<bytesPerFrameset?inputRingbufferSpace:bytesPerFrameset);
    JackAudio::getInstance()->_total_output_frames+=nframes;
    return 0;
  }
public:
  static JackAudio* getInstance();
  void SetCallback(float* (*callback)(double,int,int)) { _callback = callback; }
  void Initialize(char* name, int audioIns, int audioOuts, int midiIns, int midiOuts, float *inFrame, float *outFrame, std::vector<MidiEvent*> *midiIn, std::vector<MidiEvent*> *midiOut) {
    char temp[256];
    JackAudio::getInstance()->_client = jack_client_open ("cybin", JackNullOption, &JackAudio::getInstance()->_status, NULL);
    jack_set_process_callback (JackAudio::getInstance()->_client, _process, 0);
    jack_activate(JackAudio::getInstance()->_client);
    JackAudio::getInstance()->_extern_input_frame = inFrame;
    JackAudio::getInstance()->_extern_output_frame = outFrame;
    JackAudio::getInstance()->_extern_midi_in_events = midiIn;
    JackAudio::getInstance()->_extern_midi_out_events = midiOut;
    for(int i=0;i<audioIns;i++) {
      sprintf(temp,"audio-in_%d",i+1);
      JackAudio::getInstance()->CreatePort(true, false, temp);
    }
    for(int i=0;i<audioOuts;i++) {
      sprintf(temp,"audio-out_%d",i+1);
      JackAudio::getInstance()->CreatePort(true, true, temp);
    }
    for(int i=0;i<midiIns;i++) {
      sprintf(temp,"midi-in_%d",i+1);
      JackAudio::getInstance()->CreatePort(false, false, temp);
    }
    for(int i=0;i<midiOuts;i++) {
      sprintf(temp,"midi-out_%d",i+1);
      JackAudio::getInstance()->CreatePort(false, true, temp);
    }
    JackAudio::getInstance()->_input_ringbuffer = jack_ringbuffer_create (JackAudio::getInstance()->_audio_input_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
    JackAudio::getInstance()->_output_ringbuffer = jack_ringbuffer_create (JackAudio::getInstance()->_audio_output_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
    _output_scratch = (jack_default_audio_sample_t*)malloc(JackAudio::getInstance()->_audio_output_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
    _input_scratch = (jack_default_audio_sample_t*)malloc(JackAudio::getInstance()->_audio_output_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
    _samplerate = GetSampleRate();
  }
  void Shutdown() {}
  void EventLoop() {
    if(pthread_mutex_trylock(&(JackAudio::getInstance()->_midi_in_mutex))==0) {
      for(int i=0;i<JackAudio::getInstance()->_midi_in_events.size();i++)
        JackAudio::getInstance()->_extern_midi_in_events->push_back(JackAudio::getInstance()->_midi_in_events[i]);
      JackAudio::getInstance()->_midi_in_events.clear();
      pthread_mutex_unlock(&(JackAudio::getInstance()->_midi_in_mutex));
    }
    // --- //
    int frameSize = sizeof(jack_default_audio_sample_t)*_audio_output_ports.size();
    int frames = jack_ringbuffer_write_space(_output_ringbuffer)/frameSize;
    for(int i=0;i<frames;i++) {
      if(jack_ringbuffer_read_space(_input_ringbuffer)>=frameSize) {
        jack_ringbuffer_read(_input_ringbuffer,(char*)_extern_input_frame,frameSize);
      }
      _callback(_frames_to_time(i+_total_processed_frames,_samplerate),_audio_input_ports.size(),_audio_output_ports.size());
      jack_ringbuffer_write(_output_ringbuffer,(const char*)_extern_output_frame,frameSize);
    }
    _total_processed_frames+=frames;
    // --- //
    if(pthread_mutex_trylock(&(JackAudio::getInstance()->_midi_out_mutex))==0) {
      for(int i=0;i<JackAudio::getInstance()->_extern_midi_out_events->size();i++)
        JackAudio::getInstance()->_midi_out_events.push_back(JackAudio::getInstance()->_extern_midi_out_events->at(i));
      pthread_mutex_unlock(&(JackAudio::getInstance()->_midi_out_mutex));
      JackAudio::getInstance()->_extern_midi_out_events->clear();
    }
  }
  int GetSampleRate() { return jack_get_sample_rate(JackAudio::getInstance()->_client); }
  void CreatePort(bool isAudio, bool isOutput, char* name) {
    jack_port_t* port = jack_port_register(
                        JackAudio::getInstance()->_client, name,
                        isAudio ? JACK_DEFAULT_AUDIO_TYPE : JACK_DEFAULT_MIDI_TYPE,
                        isOutput ? JackPortIsOutput : JackPortIsInput, 0);
    (isAudio ? (isOutput ? _audio_output_ports : _audio_input_ports) : (isOutput ? _midi_output_ports : _midi_input_ports)).push_back(port);
  }
};
JackAudio* JackAudio::_instance;
JackAudio* JackAudio::getInstance() {
  if(_instance == 0) {
    _instance = new JackAudio;
  }
  return _instance;
}

#endif
