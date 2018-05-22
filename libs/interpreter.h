#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.hpp>

#define CYBIN_PROMPT "cybin> "

class Interpreter{
  static lua_State* __L;
  public:
  static float CHANNEL_DATA[256];
  static void Init(){
    if(!__L){
      __L=luaL_newstate();
      luaL_openlibs(__L);
      fprintf(stderr,"%s",CYBIN_PROMPT);
    }
  }
  static void LoadFile(char* filename){
    if(__L) {
      if(luaL_loadfile(__L,filename)){
        fprintf(stderr,"Error: %s\n", lua_tostring(__L,-1));
        lua_pop(__L,1);
        fprintf(stderr,"%s",CYBIN_PROMPT);
      } else {
        if(lua_pcall(__L,0,0,0)) {
          fprintf(stderr,"Error: %s\n", lua_tostring(__L,-1));
          lua_pop(__L,1);
          fprintf(stderr,"%s",CYBIN_PROMPT);
        }
      }
    }
  }
  static void EventLoop(char* buff,int* lock){
    (*lock)++;
    while((*lock)>1);
    int error = luaL_loadbuffer(__L, buff, strlen(buff), "line") ||
      lua_pcall(__L,0,0,0);
    if (error) {
      fprintf(stderr,"Error: %s\n", lua_tostring(__L, -1));
      lua_pop(__L, 1);
    }
    (*lock)--;
    fprintf(stderr,"%s",CYBIN_PROMPT);
  }
  static float* Process(float samplerate, int numOutChannels){
    int numLuaChannels=0;
    int outChannelIndex=0;
    lua_getglobal(__L,"__process");
    if(lua_isfunction(__L, -1)){
      lua_pushnumber(__L,samplerate);
      if(lua_pcall(__L,1,numOutChannels,0)) return 0;
      numLuaChannels=lua_gettop(__L);
      for(int luaChannelIndex=0;luaChannelIndex<numLuaChannels;luaChannelIndex++){
        while(outChannelIndex>numOutChannels) outChannelIndex-=numOutChannels;
        CHANNEL_DATA[(numOutChannels-1)-outChannelIndex]=lua_tonumber(__L,-1)+(luaChannelIndex>=numOutChannels?CHANNEL_DATA[outChannelIndex]:0);
        outChannelIndex++;
        lua_pop(__L,1);
      }
    } else lua_pop(__L,1);
    return CHANNEL_DATA;
  }
  static void Shutdown(){
    if(__L) lua_close(__L);
  }
};
lua_State* Interpreter::__L;
float Interpreter::CHANNEL_DATA[256];

#endif
