#include <lua.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <soundio/soundio.h>
#include "audio.h"
#define DEBUG_MESSAGES_OFF
#if defined(DEBUG_MESSAGES_ON)
#define DEBUG(MSG) printf(MSG); printf("\n")
#else
#define DEBUG(MSG)
#endif
lua_State *L;
void bail(lua_State *L, char *msg){fprintf(stderr,"\nFATAL ERROR:\n  %s: %s\n\n",msg,lua_tostring(L,-1));exit(1);}
// ------------ //
// ------------ //
// ------------ //
float __process(float sr){
  lua_getglobal(L, "__process");;
  lua_pushnumber(L, sr);
  if (lua_pcall(L, 1, 1, 0)) bail(L, "lua_pcall() failed"); 
  double mynumber = lua_tonumber(L, -1);
  lua_pop(L,1);
  return mynumber;
}
// ------------ //
// ------------ //
// ------------ //
// ------------ //
// ------------ //
// ------------ //
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
  // --- Init audio --- //
  Audio::Init(__process);
  // --- Loop --- //
  for(;;) Audio::EventLoop();
  // --- Shutdown audio --- //
  Audio::Shutdown();
  // --- LuaJIT Teardown --- //
  DEBUG("lua_close()");
  lua_close(L);
  return 0;
}
