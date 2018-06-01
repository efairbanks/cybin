#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <soundio/soundio.h>
#include <unistd.h>
#include <pthread.h>
#include "libs/audio.h"
#include "libs/audiofile.h"
#include "libs/interpreter.h"
#include "libs/frag.h"
#include "libs/util.h"
float* __process(float sr,int numOutChannels){
  return Interpreter::Process(sr,numOutChannels);
}
struct{
  bool offline;
  bool list_devices;
  int set_device;
  int duration;
  int samplerate;
  int channels;
  char* outfile;
} Config;
typedef struct{
  char COMMAND_BUFFER[1048576];
  int dirty=0;
} SharedInput;
void parse_args(int argc, char** argv){
  Config.offline=false;
  Config.list_devices=false;
  Config.set_device=-1;
  Config.duration=10;
  Config.samplerate=44100;
  Config.channels=2;
  Config.outfile="cybin.wav";
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
      Config.duration=atoi(currentArg);
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
    } else {
      Interpreter::LoadFile(currentArg);
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
int cybin_loadfragmentshader(lua_State* L){
  const char* shader = lua_tostring(L,1);
  Frag::LoadString((char*)shader);
  return 0;
}
int cybin_loadfragmentshaderfile(lua_State* L){
  const char* fileName = lua_tostring(L,1);
  Frag::LoadFile((char*)fileName);
  return 0;
}
int cybin_getuniformid(lua_State* L){
  const char* uniform = lua_tostring(L,1);
  lua_pushnumber(L,Frag::GetUniformID((char*)uniform));
  return 1;
}
int cybin_setuniform1f(lua_State* L){
  if(lua_isnil(L,1)) return 0;
  int uniformid = lua_tonumber(L,1);
  double uniformvalue1 = lua_tonumber(L,2);
  if(uniformid>=0) glUniform1f(uniformid,uniformvalue1);
  return 0;
}
int cybin_setuniform2f(lua_State* L){
  if(lua_isnil(L,1)) return 0;
  int uniformid = lua_tonumber(L,1);
  double uniformvalue1 = lua_tonumber(L,2);
  double uniformvalue2 = lua_tonumber(L,3);
  if(uniformid>=0) glUniform2f(uniformid,uniformvalue1,uniformvalue2);
  return 0;
}
int cybin_setuniform3f(lua_State* L){
  if(lua_isnil(L,1)) return 0;
  int uniformid = lua_tonumber(L,1);
  double uniformvalue1 = lua_tonumber(L,2);
  double uniformvalue2 = lua_tonumber(L,3);
  double uniformvalue3 = lua_tonumber(L,4);
  if(uniformid>=0) glUniform3f(uniformid,uniformvalue1,uniformvalue2,uniformvalue3);
  return 0;
}
int cybin_setuniform4f(lua_State* L){
  if(lua_isnil(L,1)) return 0;
  int uniformid = lua_tonumber(L,1);
  double uniformvalue1 = lua_tonumber(L,2);
  double uniformvalue2 = lua_tonumber(L,3);
  double uniformvalue3 = lua_tonumber(L,4);
  double uniformvalue4 = lua_tonumber(L,5);
  if(uniformid>=0) glUniform4f(uniformid,uniformvalue1,uniformvalue2,uniformvalue3,uniformvalue4);
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
  Frag::Init(argc,argv,NULL,NULL,NULL);
  // --- Register cybin.loadaudiofile --- //
  Interpreter::LoadFunction("loadaudiofile",cybin_loadaudiofile);
  Interpreter::LoadFunction("loadfragmentshader",cybin_loadfragmentshader);
  Interpreter::LoadFunction("loadfragmentshaderfile",cybin_loadfragmentshaderfile);
  Interpreter::LoadFunction("getuniformid",cybin_getuniformid);
  Interpreter::LoadFunction("setuniform1f",cybin_setuniform1f);
  Interpreter::LoadFunction("setuniform2f",cybin_setuniform2f);
  Interpreter::LoadFunction("setuniform3f",cybin_setuniform3f);
  Interpreter::LoadFunction("setuniform4f",cybin_setuniform4f);
  // --- Configure environment --- //
  parse_args(argc,argv);
  if(Config.offline) {   // --- OFFLINE RENDERING ---- //
    printf("Rendering %d seconds of %d-channel audio to %s at %dHz",Config.duration,Config.channels,Config.outfile,Config.samplerate);
    int frames=Config.duration*Config.samplerate;
    float* buffer = (float*)malloc(frames*Config.channels*sizeof(float));
    int progress=-1;
    for(int i=0;i<frames;i++) {
      progress=print_progress(i,frames,20,progress);
      float* samples=Interpreter::Process(Config.samplerate,Config.channels);
      for(int j=0;j<Config.channels;j++) buffer[i*Config.channels+j]=samples[j];
    }
    AudioFile file(buffer,Config.duration*Config.samplerate,Config.channels,Config.samplerate);
    file.Write(Config.outfile);
    printf("\n%s Wrote audio to %s\n",CYBIN_PROMPT,Config.outfile);
  } else {        // --- REALTIME RENDERING --- //
    // --- Continue startup --- //
    Audio::Init(__process,Config.set_device);
    if(Config.list_devices) {
      Audio::ListDevices();
      exit(0);
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
  }
  Interpreter::Shutdown();
  return 0;
}
