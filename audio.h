#ifndef AUDIO_H
#define AUDIO_H

class Audio{
static SoundIo* soundio;
static SoundIoDevice* device;
static SoundIoOutStream* outstream;
static float (*audio_callback)(float);
  public:
    static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max){
      const struct SoundIoChannelLayout *layout = &outstream->layout;
      float float_sample_rate = outstream->sample_rate;
      struct SoundIoChannelArea *areas;
      int frames_left = frame_count_max;
      int err;
      while (frames_left > 0) {
        int frame_count = frames_left;
        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) exit(1);
        if (!frame_count) break;
        for (int frame = 0; frame < frame_count; frame += 1) {
          for (int channel = 0; channel < layout->channel_count; channel += 1) {
            float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
            *ptr = audio_callback?audio_callback(float_sample_rate):0;
          }
        }
        if ((err = soundio_outstream_end_write(outstream))) exit(1);
        frames_left -= frame_count;
      }
    }
    static void Init(float (*callback)(float)){
      // --- LibSndIo Setup --- //  
      audio_callback=callback;
      if(!(soundio=soundio_create())) exit(1);
      if(soundio_connect(soundio)) exit(1);
      soundio_flush_events(soundio);
      int default_out_device_index; if((default_out_device_index=soundio_default_output_device_index(soundio)<0)) exit(1);
      if(!(device=soundio_get_output_device(soundio, default_out_device_index))) exit(1);
      outstream = soundio_outstream_create(device);
      outstream->format = SoundIoFormatFloat32NE; outstream->write_callback = write_callback;
      if(soundio_outstream_open(outstream)||outstream->layout_error) exit(1);
      if(soundio_outstream_start(outstream)) exit(1);
    }
    static void Shutdown(){
      // --- LibSndIo Teardown --- //
      soundio_outstream_destroy(outstream);
      soundio_device_unref(device);
      soundio_destroy(soundio);  
    }
    static void EventLoop(){
      soundio_wait_events(soundio);
    }
};
SoundIo* Audio::soundio;
SoundIoDevice* Audio::device;
SoundIoOutStream* Audio::outstream;
float (*Audio::audio_callback)(float);

#endif
