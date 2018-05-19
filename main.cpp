#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <soundio/soundio.h>
#include "audio.h"
#include "interpreter.h"
#define DEBUG_MESSAGES_OFF
#if defined(DEBUG_MESSAGES_ON)
#define DEBUG(MSG) printf(MSG); printf("\n")
#else
#define DEBUG(MSG)
#endif
float process(float sr){
  return Interpreter::Process(sr);
}
int main(int argc, char** argv){
  // --- Startup --- //
  Interpreter::Init();
  if(argc<2) return 0;
  Interpreter::LoadFile(argv[1]);
  Audio::Init(process);
  // --- Loop --- //
  for(;;) Audio::EventLoop();
  // --- Shutdown  --- //
  Audio::Shutdown();
  Interpreter::Shutdown();
  return 0;
}
