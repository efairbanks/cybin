#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <soundio/soundio.h>
#include <unistd.h>
#include "libs/audio.h"
#include "libs/audiofile.h"
#include "libs/interpreter.h"
#include "libs/util.h"
char COMMAND_BUFFER[1048576];
int INTERPRETER_LOCK=0;
float* __process(float sr,int numOutChannels){
  return Interpreter::Process(sr,numOutChannels);
}
struct{
  bool offline;
  int duration;
  int samplerate;
  int channels;
  char* outfile;
} Config;
void parse_args(int argc, char** argv){
  Config.offline=false;
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
    lua_pushnumber(L,i);
    lua_settable(L,-3);
  }
  DEBUG("stack top: %d",lua_gettop(L));
  lua_pushstring(L,"frames");lua_pushnumber(L,file.frames);lua_settable(L,-3);
  lua_pushstring(L,"channels");lua_pushnumber(L,file.channels);lua_settable(L,-3);
  lua_pushstring(L,"samplerate");lua_pushnumber(L,file.sampleRate);lua_settable(L,-3);
  return 1;
}
int main(int argc, char** argv){
  // --- Start Lua --- //
  Interpreter::Init();
  // --- Register cybin.loadaudiofile --- //
  Interpreter::LoadFunction("loadaudiofile",cybin_loadaudiofile);
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
    Audio::Init(__process,&INTERPRETER_LOCK);
    // --- Handle REPL event loop --- //
    while (fgets(COMMAND_BUFFER,sizeof(COMMAND_BUFFER),stdin)!=NULL){
      Interpreter::EventLoop(COMMAND_BUFFER,&INTERPRETER_LOCK);
    }
    
    // --- Shutdown  --- //
    Audio::Shutdown();
  }
  Interpreter::Shutdown();
  return 0;
}
