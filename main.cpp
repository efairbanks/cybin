#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <soundio/soundio.h>
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <jack/midiport.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <vector>
#include "libs/audiofile.h"
#include "libs/newaudio.h"
#include "libs/interpreter.h"
#include "libs/util.h"
float* __process(double time,int numInChannels,int numOutChannels){
  return Interpreter::Process(time,numInChannels,numOutChannels);
}
struct{
  bool offline;
  bool list_devices;
  int set_device;
  float duration;
  int samplerate;
  int channels;
  char* outfile;
  char* loadfile;
  int fps;
  int render_width;
  int render_height;
} Config;
typedef struct{
  char COMMAND_BUFFER[1048576];
  char dirty=0;
} SharedInput;
void parse_args(int argc, char** argv){
  Config.offline=false;
  Config.list_devices=false;
  Config.set_device=-1;
  Config.duration=10;
  Config.samplerate=48000;
  Config.channels=2;
  Config.outfile="cybin.wav";
  Config.loadfile=NULL;
  Config.fps=25;
  Config.render_width=400;
  Config.render_height=300;
  for(int i=1;i<argc;i++){
    char* currentArg=argv[i];
    if(strcmp("--offline",currentArg)==0){
      Config.offline=true;
      i++;currentArg=argv[i];
      if(currentArg[0]=='-'&&currentArg[1]=='-'){
        i--;
      } else {
        Config.outfile=currentArg;
      }
    } else if(strcmp("--duration",currentArg)==0){
      i++;currentArg=argv[i];
      Config.duration=atof(currentArg);
    } else if(strcmp("--samplerate",currentArg)==0){
      i++;currentArg=argv[i];
      Config.samplerate=atoi(currentArg);
    } else if(strcmp("--channels",currentArg)==0){
      i++;currentArg=argv[i];
      Config.channels=atoi(currentArg);
    } else if(strcmp("--list-devices",currentArg)==0){
      Config.list_devices=true;
    } else if(strcmp("--set-device",currentArg)==0){
      i++;currentArg=argv[i];
      Config.set_device=atoi(currentArg);
    } else if(strcmp("--render-width",currentArg)==0){
      i++;currentArg=argv[i];
      Config.render_width=atoi(currentArg);
    } else if(strcmp("--render-height",currentArg)==0){
      i++;currentArg=argv[i];
      Config.render_height=atoi(currentArg);
    } else if(strcmp("--fps",currentArg)==0){
      i++;currentArg=argv[i];
      Config.fps=atoi(currentArg);
    } else {
      Config.loadfile=currentArg;
    }
  }
}
int print_progress(int n, int d, int width, int last){
  int index=n*width/d;
  if(index!=last){
    printf("\n%s [",CYBIN_PROMPT);
    for(int i=0;i<width;i++) printf(i>index?" ":"#");
    printf("]");
  }
  return index;
}
int cybin_loadaudiofile(lua_State* L){
  const char* fileName = lua_tostring(L,1);
  AudioFile file(fileName);
  if(file.buffer==NULL) return 0;
  lua_newtable(L);
  DEBUG("stack top: %d",lua_gettop(L));
  for(int i=0;i<file.frames*file.channels;i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,file.buffer[i]);
    lua_settable(L,-3);
  }
  DEBUG("stack top: %d",lua_gettop(L));
  lua_pushstring(L,"frames");lua_pushnumber(L,file.frames);lua_settable(L,-3);
  lua_pushstring(L,"channels");lua_pushnumber(L,file.channels);lua_settable(L,-3);
  lua_pushstring(L,"samplerate");lua_pushnumber(L,file.sampleRate);lua_settable(L,-3);
  return 1;
}
int cybin_midiout(lua_State* L) {
  int nargs = lua_gettop(L);
  if (lua_gettop(L) < 1) return luaL_error(L, "expecting at least 1 argument");
  MidiEvent *event = new MidiEvent();
  lua_getglobal(L,"cybin");
  lua_getfield(L,-1,"time");
  event->time = lua_tonumber(L, -1);
  lua_pop(L,2);
  event->port = lua_tointeger(L, 1);
  for(int i=2;i<=nargs;i++) event->data.push_back(lua_tointeger(L, i));
  Interpreter::MIDI_OUT_DATA.push_back(event);
  return 0;
}
void* input_handler(void* data){
  SharedInput* input=(SharedInput*)data;
  // input handlng thread
  for(;;){
    if(!input->dirty){
      fgets(input->COMMAND_BUFFER,sizeof(input->COMMAND_BUFFER),stdin);
      DEBUG("BUFFER DIRTY!");
      input->dirty=true;
    }
  }
}
int main(int argc, char** argv){
  // --- Start Lua --- //
  Interpreter::Init();
  // --- Register cybin.loadaudiofile --- //
  Interpreter::LoadFunction("loadaudiofile",cybin_loadaudiofile);
  Interpreter::LoadFunction("midiout",cybin_midiout);
  // --- Configure environment --- //
  parse_args(argc,argv);
  if(Config.offline) {   // --- OFFLINE RENDERING ---- //
    Interpreter::LoadNumber("samplerate",Config.samplerate);
    Interpreter::LoadNumber("channels",Config.channels);
    Interpreter::LoadBool("offline",Config.offline);
    if(Config.loadfile!=NULL) Interpreter::LoadFile(Config.loadfile);
    printf("Rendering %f seconds of %d-channel audio to %s at %dHz",Config.duration,Config.channels,Config.outfile,Config.samplerate);
    int frames=int(Config.duration*Config.samplerate);
    float* buffer = (float*)malloc(frames*Config.channels*sizeof(float));
    int progress=-1;
    int glFrameCounter=0;
    for(int i=0;i<frames;i++) {
      progress=print_progress(i,frames,20,progress);
      float* samples=Interpreter::Process(((double)i)/((double)Config.samplerate),Config.samplerate,Config.channels);
      for(int j=0;j<Config.channels;j++) buffer[i*Config.channels+j]=samples[j];
    }
    AudioFile file(buffer,int(Config.duration*Config.samplerate),Config.channels,Config.samplerate);
    file.Write(Config.outfile);
    printf("\n%s Wrote audio to %s\n",CYBIN_PROMPT,Config.outfile);
  } else {
    JackAudio::getInstance()->Initialize(
      "cybin",
      Config.channels, Config.channels,
      Config.channels, Config.channels,
      Interpreter::AUDIO_IN_CHANNEL_DATA,
      Interpreter::AUDIO_OUT_CHANNEL_DATA,
      &Interpreter::MIDI_IN_DATA,
      &Interpreter::MIDI_OUT_DATA);
    JackAudio::getInstance()->SetCallback(__process);
    Interpreter::LoadNumber("samplerate",JackAudio::getInstance()->GetSampleRate());
    Interpreter::LoadNumber("channels", Config.channels);
    if(Config.loadfile!=NULL) Interpreter::LoadFile(Config.loadfile);
    // --- Handle REPL event loop --- //
    SharedInput Input;
    pthread_t input_handler_thread;
    pthread_create(&input_handler_thread,NULL,input_handler,(void*)&Input);
    for(;;){
      if(Input.dirty){
        Interpreter::EventLoop(Input.COMMAND_BUFFER);
        Input.dirty=false;
        DEBUG("BUFFER CLEAN!");
      }
      JackAudio::getInstance()->EventLoop();
      usleep(10000000/10000);
    }
    // --- Shutdown  --- //
    JackAudio::getInstance()->Shutdown();

    // --- REALTIME RENDERING --- //
    /*
    // --- Continue startup --- //
    Audio::Init(__process,Config.set_device);
    Interpreter::LoadNumber("samplerate",Audio::samplerate);
    Interpreter::LoadNumber("channels",Audio::channels);
    if(Config.loadfile!=NULL) Interpreter::LoadFile(Config.loadfile);
    if(Config.list_devices) {
      Audio::ListDevices();
      printf("cybin> "); // this is inelegant, we can do better.
    }
    // --- Handle REPL event loop --- //
    SharedInput Input;
    pthread_t input_handler_thread;
    pthread_create(&input_handler_thread,NULL,input_handler,(void*)&Input);
    for(;;){
      if(Input.dirty){
        Interpreter::EventLoop(Input.COMMAND_BUFFER);
        Input.dirty=false;
        DEBUG("BUFFER CLEAN!");
      }
      Audio::EventLoop();
      Frag::EventLoop();
      usleep(10000000/10000);
    }
    // --- Shutdown  --- //
    Audio::Shutdown();
    */
  }
  Interpreter::Shutdown();
  return 0;
}
