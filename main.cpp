#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
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
int main(int argc, char** argv){
  // --- Startup --- //
  Interpreter::Init();
  for(int i=1;i<argc;i++) Interpreter::LoadFile(argv[i]);
  Audio::Init(process,&INTERPRETER_LOCK);
  // --- //
  while (fgets(BUFFER,sizeof(BUFFER),stdin)!=NULL){
    Interpreter::EventLoop(BUFFER,&INTERPRETER_LOCK);
  }
  // --- Shutdown  --- //
  Audio::Shutdown();
  Interpreter::Shutdown();
  return 0;
}
