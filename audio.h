#ifndef AUDIO_H
#define AUDIO_H
class Audio{
static SoundIo* __audio_soundio;
static SoundIoDevice* __audio_device;
static SoundIoOutStream* __audio_oustream;
static float (*__audio_callback)(float);
  public:
    static void write_callback(struct SoundIoOutStream *__audio_oustream, int frame_count_min, int frame_count_max){
      const struct SoundIoChannelLayout *layout = &__audio_oustream->layout;
      float float_sample_rate = __audio_oustream->sample_rate;
      struct SoundIoChannelArea *areas;
      int frames_left = frame_count_max;
      int err;
      while (frames_left > 0) {
        int frame_count = frames_left;
        if ((err = soundio_outstream_begin_write(__audio_oustream, &areas, &frame_count))) exit(1);
        if (!frame_count) break;
        for (int frame = 0; frame < frame_count; frame += 1) {
          for (int channel = 0; channel < layout->channel_count; channel += 1) {
            float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
            *ptr = __audio_callback?__audio_callback(float_sample_rate):0;
          }
        }
        if ((err = soundio_outstream_end_write(__audio_oustream))) exit(1);
        frames_left -= frame_count;
      }
    }
    static void Init(float (*callback)(float)){
      // --- LibSndIo Setup --- //  
      __audio_callback=callback;
      if(!(__audio_soundio=soundio_create())) exit(1);
      if(soundio_connect(__audio_soundio)) exit(1);
      soundio_flush_events(__audio_soundio);
      int default_out_device_index; if((default_out_device_index=soundio_default_output_device_index(__audio_soundio)<0)) exit(1);
      if(!(__audio_device=soundio_get_output_device(__audio_soundio, default_out_device_index))) exit(1);
      __audio_oustream = soundio_outstream_create(__audio_device);
      __audio_oustream->format = SoundIoFormatFloat32NE; __audio_oustream->write_callback = write_callback;
      if(soundio_outstream_open(__audio_oustream)||__audio_oustream->layout_error) exit(1);
      if(soundio_outstream_start(__audio_oustream)) exit(1);
    }
    static void Shutdown(){
      // --- LibSndIo Teardown --- //
      soundio_outstream_destroy(__audio_oustream);
      soundio_device_unref(__audio_device);
      soundio_destroy(__audio_soundio);  
    }
    static void EventLoop(){
      soundio_wait_events(__audio_soundio);
    }
};
SoundIo* Audio::__audio_soundio;
SoundIoDevice* Audio::__audio_device;
SoundIoOutStream* Audio::__audio_oustream;
float (*Audio::__audio_callback)(float);
#endif
