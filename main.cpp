#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <soundio/soundio.h>
#include <unistd.h>
#include "audio.h"
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
  if(argc<2) return 0;
  Interpreter::LoadFile(argv[1]);
  Audio::Init(process,&INTERPRETER_LOCK);
  // --- //
  int pid=fork();
  if(!pid) {
    while (fgets(BUFFER,sizeof(BUFFER),stdin)!=NULL)
      Interpreter::EventLoop(BUFFER,&INTERPRETER_LOCK);
      printf("\ncybin> ");
  }
  // --- Loop --- //
  for(;;) Audio::EventLoop();
  // --- Shutdown  --- //
  Audio::Shutdown();
  Interpreter::Shutdown();
  return 0;
}
