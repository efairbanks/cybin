#ifndef NEWAUDIO_H
#define NEWAUDIO_H

class AudioInterface {
public:
  virtual void SetCallback(float* (*callback)(int,int)) = 0;
  virtual void Shutdown() = 0;
  virtual void EventLoop() = 0;
  virtual int GetSampleRate() = 0;
};

class JackAudio : public AudioInterface {
private:
  jack_client_t *_client;
  jack_status_t _status;
  int _ringbuffer_frames = 4000;
  jack_ringbuffer_t *_output_ringbuffer;
  jack_ringbuffer_t *_input_ringbuffer;
  std::vector<jack_port_t*> _audio_input_ports;
  std::vector<jack_port_t*> _audio_output_ports;
  std::vector<jack_port_t*> _midi_input_ports;
  std::vector<jack_port_t*> _midi_output_ports;
  float* _extern_input_frame;
  float* _extern_output_frame;
  jack_default_audio_sample_t* _output_scratch; // scratch buffer for dumping sets of multichannel frames
  jack_default_audio_sample_t* _input_scratch; // scratch buffer for dumping sets of multichannel frames
  float* (*_callback)(int,int);
  static JackAudio* _instance;
  JackAudio() {}
  static int _process(jack_nframes_t nframes, void *arg) {
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
    return 0;
  }
public:
  static JackAudio* getInstance();
  void SetCallback(float* (*callback)(int,int)) { _callback = callback; }
  void Initialize(char* name, int audioIns, int audioOuts, float *inFrame, float *outFrame) {
    char temp[256];
    JackAudio::getInstance()->_client = jack_client_open ("cybin", JackNullOption, &JackAudio::getInstance()->_status, NULL);
    jack_set_process_callback (JackAudio::getInstance()->_client, _process, 0);
    jack_activate(JackAudio::getInstance()->_client);
    JackAudio::getInstance()->_extern_input_frame = inFrame;
    JackAudio::getInstance()->_extern_output_frame = outFrame;
    for(int i=0;i<audioIns;i++) {
      sprintf(temp,"audio-in_%d",i+1);
      JackAudio::getInstance()->CreatePort(true, false, temp);
    }
    for(int i=0;i<audioOuts;i++) {
      sprintf(temp,"audio-out_%d",i+1);
      JackAudio::getInstance()->CreatePort(true, true, temp);
    }
    JackAudio::getInstance()->_input_ringbuffer = jack_ringbuffer_create (JackAudio::getInstance()->_audio_input_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
    JackAudio::getInstance()->_output_ringbuffer = jack_ringbuffer_create (JackAudio::getInstance()->_audio_output_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
    _output_scratch = (jack_default_audio_sample_t*)malloc(JackAudio::getInstance()->_audio_output_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
    _input_scratch = (jack_default_audio_sample_t*)malloc(JackAudio::getInstance()->_audio_output_ports.size() * sizeof(jack_default_audio_sample_t) * _ringbuffer_frames);
  }
  void Shutdown() {}
  void EventLoop() {
    int frameSize = sizeof(jack_default_audio_sample_t)*_audio_output_ports.size();
    int frames = jack_ringbuffer_write_space(_output_ringbuffer)/frameSize;
    for(int i=0;i<frames;i++) {
      if(jack_ringbuffer_read_space(_input_ringbuffer)>=frameSize) {
        jack_ringbuffer_read(_input_ringbuffer,(char*)_extern_input_frame,frameSize);
      }
      _callback(_audio_input_ports.size(),_audio_output_ports.size());
      jack_ringbuffer_write(_output_ringbuffer,(const char*)_extern_output_frame,frameSize);
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
