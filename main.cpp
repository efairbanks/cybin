#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <soundio/soundio.h>
#include <unistd.h>
#include "libs/audio.h"
#include "libs/audiofile.h"
#include "libs/interpreter.h"
#define DEBUG_MESSAGES_OFF
#if defined(DEBUG_MESSAGES_ON)
#define DEBUG(MSG) printf(MSG); printf("\n")
#else
#define DEBUG(MSG)
#endif
char BUFFER[512];
int INTERPRETER_LOCK=0;
float* process(float sr,int numOutChannels){
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
    printf("\ncybin> [");
    for(int i=0;i<width;i++) printf(i>index?" ":"#");
    printf("]");
  }
  return index;
}
int main(int argc, char** argv){
  // --- Start Lua --- //
  Interpreter::Init();
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
    printf("\ncybin> Wrote audio to %s\n",Config.outfile);
  } else {        // --- REALTIME RENDERING --- //
    // --- Continue startup --- //
    Audio::Init(process,&INTERPRETER_LOCK);
    // --- Handle REPL event loop --- //
    while (fgets(BUFFER,sizeof(BUFFER),stdin)!=NULL){
      Interpreter::EventLoop(BUFFER,&INTERPRETER_LOCK);
    }
    // --- Shutdown  --- //
    Audio::Shutdown();
  }
  Interpreter::Shutdown();
  return 0;
}
