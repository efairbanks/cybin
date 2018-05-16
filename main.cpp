#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <soundio/soundio.h>
#define DEBUG_MESSAGES_ON
#if defined(DEBUG_MESSAGES_ON)
#define DEBUG(MSG) printf(MSG); printf("\n")
#else
#define DEBUG(MSG)
#endif
lua_State *L;
void bail(lua_State *L, char *msg){fprintf(stderr,"\nFATAL ERROR:\n  %s: %s\n\n",msg,lua_tostring(L,-1));exit(1);}
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
        lua_getglobal(L, "__process");;
        lua_pushnumber(L, float_sample_rate);
        if (lua_pcall(L, 1, 1, 0)) bail(L, "lua_pcall() failed"); 
        double mynumber = lua_tonumber(L, -1);
        lua_pop(L,1);
        *ptr = mynumber;
      }
    }
    if ((err = soundio_outstream_end_write(outstream))) exit(1);
    frames_left -= frame_count;
  }
}
int main(int argc, char** argv){
  // --- LuaJIT Setup --- //
  DEBUG("L = luaL_newstate();");
  L = luaL_newstate();
  DEBUG("luaL_openlibs();");
  luaL_openlibs(L);
  if(argc<2) return 0;
  DEBUG("lluaL_loadfile();");
  if (luaL_loadfile(L, argv[1])) bail(L, "luaL_loadfile() failed");
  DEBUG("lua_pcall();");
  if (lua_pcall(L, 0, 0, 0)) bail(L, "lua_pcall() failed");
  // --- LibSndIo Setup --- //  
  DEBUG("soundio_create()");
  SoundIo* soundio; if(!(soundio=soundio_create())) return 1;
  DEBUG("soundio_connect()");
  if(soundio_connect(soundio)) return 1;
  DEBUG("soundio_flush_events()");
  soundio_flush_events(soundio);
  DEBUG("soundio_default_output_device_index()");
  int default_out_device_index; if((default_out_device_index=soundio_default_output_device_index(soundio)<0)) return 1;
  DEBUG("soundio_get_output_device()");
  struct SoundIoDevice* device; if(!(device=soundio_get_output_device(soundio, default_out_device_index))) return 1;
  DEBUG("soundio_outstream_create()");
  struct SoundIoOutStream* outstream = soundio_outstream_create(device);
  outstream->format = SoundIoFormatFloat32NE; outstream->write_callback = write_callback;
  DEBUG("soundio_outstream_open()");
  if(soundio_outstream_open(outstream)||outstream->layout_error) return 1;
  DEBUG("soundio_outstream_start()");
  if(soundio_outstream_start(outstream)) return 1;
  // --- Loop --- //
  DEBUG("soundio_wait_events()");
  for(;;) soundio_wait_events(soundio);
  // --- LuaJIT Teardown --- //
  DEBUG("lua_close()");
  lua_close(L);
  // --- LibSndIo Teardown --- //
  DEBUG("soundio_outstream_destroy()");
  soundio_outstream_destroy(outstream);
  DEBUG("soundio_device_unref()");
  soundio_device_unref(device);
  DEBUG("soundio_destroy()");
  soundio_destroy(soundio);
  return 0;
}
