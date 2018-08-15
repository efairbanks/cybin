#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.hpp>
#include <unistd.h>
#include "util.h"

#define CYBIN_PROMPT "cybin> "
#define CYBIN_INIT "package.path=\"./?.cybin;\"..package.path;cybin={};"
#define CYBIN_LOGOTEXT "      ___                                                 ___\n     /  /\\          ___        _____        ___          /__/\\\n    /  /:/         /__/|      /  /::\\      /  /\\         \\  \\:\\\n   /  /:/         |  |:|     /  /:/\\:\\    /  /:/          \\  \\:\\\n  /  /:/  ___     |  |:|    /  /:/~/::\\  /__/::\\      _____\\__\\:\\\n /__/:/  /  /\\  __|__|:|   /__/:/ /:/\\:| \\__\\/\\:\\__  /__/::::::::\\\n \\  \\:\\ /  /:/ /__/::::\\   \\  \\:\\/:/~/:/    \\  \\:\\/\\ \\  \\:\\~~\\~~\\/\n  \\  \\:\\  /:/     ~\\~~\\:\\   \\  \\::/ /:/      \\__\\::/  \\  \\:\\  ~~~\n   \\  \\:\\/:/        \\  \\:\\   \\  \\:\\/:/       /__/:/    \\  \\:\\\n    \\  \\::/          \\__\\/    \\  \\::/        \\__\\/      \\  \\:\\\n     \\__\\/                     \\__\\/                     \\__\\/\n"

class Interpreter{
  static lua_State* __L;
  public:
  static float CHANNEL_DATA[256];
  static void Init(){
    if(!__L){
      __L=luaL_newstate();
      luaJIT_setmode(__L, 0, LUAJIT_MODE_ENGINE);
      luaL_openlibs(__L);
      luaL_loadbuffer(__L,CYBIN_INIT,strlen(CYBIN_INIT),"line");
      lua_pcall(__L,0,0,0);
      fprintf(stderr,"%s\n",CYBIN_LOGOTEXT);
      fprintf(stderr,"%s",CYBIN_PROMPT);
    }
  }
  static void LoadFile(char* filename){
    if(__L) {
      if(luaL_loadfile(__L,filename)){
        ERROR("%s", lua_tostring(__L,-1));
        lua_pop(__L,1);
        fprintf(stderr,"%s",CYBIN_PROMPT);
      } else {
        if(lua_pcall(__L,0,0,0)) {
          ERROR("%s", lua_tostring(__L,-1));
          lua_pop(__L,1);
          fprintf(stderr,"%s",CYBIN_PROMPT);
        }
      }
    }
  }
  static void LoadFunction(char* name, int (*function)(lua_State* L)){
    lua_getglobal(__L,"cybin");
    lua_pushstring(__L,name);
    lua_pushcfunction(__L,function);
    lua_settable(__L,-3);
    lua_pop(__L,1);
  }
  static void LoadNumber(char* name, double number){
    lua_getglobal(__L,"cybin");
    lua_pushstring(__L,name);
    lua_pushnumber(__L,number);
    lua_settable(__L,-3);
    lua_pop(__L,1);
  }
  static void LoadString(char* name, char* string){
    lua_getglobal(__L,"cybin");
    lua_pushstring(__L,name);
    lua_pushstring(__L,string);
    lua_settable(__L,-3);
    lua_pop(__L,1);
  }
  static void EventLoop(char* buff){
    int error = luaL_loadbuffer(__L, buff, strlen(buff), "line") ||
      lua_pcall(__L,0,0,0);
    if (error) {
      ERROR("%s", lua_tostring(__L, -1));
      lua_pop(__L, 1);
    }
    fprintf(stderr,"%s",CYBIN_PROMPT);
  }
  static float* Process(float samplerate, int numOutChannels){
    //return CHANNEL_DATA;
    int numLuaChannels=0;
    int outChannelIndex=0;
    int top=lua_gettop(__L);
    lua_getglobal(__L,"__process");
    if(lua_isfunction(__L, -1)){
      lua_pushnumber(__L,samplerate);
      switch(lua_pcall(__L,1,LUA_MULTRET,0)){
        case LUA_ERRRUN:
          fprintf(stderr,"%s\nError: __process threw an unrecoverable error and has been reset.\n", lua_tostring(__L, -1));
          lua_pop(__L, 1);
          luaL_dostring(__L,"__process=nil");
          return CHANNEL_DATA;
        case LUA_ERRMEM:fprintf(stderr,"MEMORY ALLOCATION ERROR\n");return 0;
        case LUA_ERRERR:fprintf(stderr,"ERROR HANDLING ERROR\n");return 0;
        default:break;
      }
      numLuaChannels=lua_gettop(__L)-top;
      for(int luaChannelIndex=0;luaChannelIndex<numLuaChannels;luaChannelIndex++){
        while(outChannelIndex>=numOutChannels) outChannelIndex-=numOutChannels;
        CHANNEL_DATA[outChannelIndex]=lua_tonumber(__L,-1)+(luaChannelIndex>=numOutChannels?CHANNEL_DATA[outChannelIndex]:0);
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
