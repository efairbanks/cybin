#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <soundio/soundio.h>
#include <unistd.h>
#include "audio.h"
#include "audiofile.h"
#include "interpreter.h"
#define DEBUG_MESSAGES_OFF
#if defined(DEBUG_MESSAGES_ON)
#define DEBUG(MSG) printf(MSG); printf("\n")
#else
#define DEBUG(MSG)
#endif
char BUFFER[512];
int INTERPRETER_LOCK=0;
float process(float sr){
  return Interpreter::Process(sr);
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
  Config.duration=-1;
  Config.samplerate=44100;
  Config.channels=1;
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
    } else {
      Interpreter::LoadFile(currentArg);
    }
  }
}
int main(int argc, char** argv){
  // --- Start Lua --- //
  Interpreter::Init();
  // --- Configure environment --- //
  parse_args(argc,argv);
  if(Config.offline) {   // --- OFFLINE RENDERING ---- //
    int samples=Config.duration*Config.samplerate*Config.channels;
    float* buffer = (float*)malloc(samples*sizeof(float));
    for(int i=0;i<samples;i++) buffer[i]=Interpreter::Process(Config.samplerate);
    AudioFile file(buffer,Config.duration*Config.samplerate,Config.channels,Config.samplerate);
    file.Write(Config.outfile);
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
