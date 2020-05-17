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

class Interpreter {
  static lua_State* __L;
  public:
  static float AUDIO_IN_CHANNEL_DATA[256];
  static float AUDIO_OUT_CHANNEL_DATA[256];
  static std::vector<MidiEvent*> MIDI_IN_DATA;
  static std::vector<MidiEvent*> MIDI_OUT_DATA;
  static bool ERROR_FLAG;
  static void Init(bool no_logo){
    if(!__L){
      __L=luaL_newstate();
      luaJIT_setmode(__L, 0, LUAJIT_MODE_ENGINE);
      luaL_openlibs(__L);
      luaL_loadbuffer(__L,CYBIN_INIT,strlen(CYBIN_INIT),"line");
      lua_pcall(__L,0,0,0);
      if(!no_logo) fprintf(stderr,"%s\n",CYBIN_LOGOTEXT);
      fprintf(stderr,"%s",CYBIN_PROMPT);
      LoadEmptyTable("midiin");
    }
  }
  static void LoadFile(char* filename){
    if(__L) {
      if(luaL_loadfile(__L,filename)){
        ERROR_FLAG=true;
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
  static void LoadBool(char* name, int boolean){
    lua_getglobal(__L,"cybin");
    lua_pushstring(__L,name);
    lua_pushboolean(__L,boolean);
    lua_settable(__L,-3);
    lua_pop(__L,1);
  }
  static void LoadEmptyTable(char* name){
    lua_getglobal(__L,"cybin");
    lua_pushstring(__L,name);
    lua_newtable(__L);
    lua_settable(__L,-3);
    lua_pop(__L,1);
  }
  static void AddNewMidiEvent(MidiEvent* event){
    lua_getglobal(__L,"cybin");
    lua_getfield(__L,-1,"midiin");
    int len = lua_objlen(__L,-1);
    lua_pushnumber(__L,len+1);
    // --- //
    lua_newtable(__L);
    lua_pushnumber(__L,event->time);
    lua_setfield(__L,-2,"time");
    lua_pushnumber(__L,event->port);
    lua_setfield(__L,-2,"port");
    for(int i=0;i<event->data.size();i++) {
      lua_pushnumber(__L,i+1);
      lua_pushnumber(__L,event->data[i]);
      lua_settable(__L,-3);
    }
    // --- //
    lua_settable(__L,-3);
    lua_pop(__L,2);
  }
  static void DoString(char* string){
    int error = luaL_loadstring(__L, string) ||
      lua_pcall(__L,0,0,0);
    if (error) {
      ERROR_FLAG=true;
      ERROR("%s", lua_tostring(__L, -1));
      lua_pop(__L, 1);
    }
  }
  static void EventLoop(char* buff){
    int error = luaL_loadbuffer(__L, buff, strlen(buff), "line") ||
      lua_pcall(__L,0,0,0);
    if (error) {
      ERROR_FLAG=true;
      ERROR("%s", lua_tostring(__L, -1));
      lua_pop(__L, 1);
    }
    fprintf(stderr,"%s",CYBIN_PROMPT);
  }
  static float* Process(double time, int numInChannels, int numOutChannels){
    for(int i=0;i<MIDI_IN_DATA.size();i++) {
      MidiEvent* event = MIDI_IN_DATA[i];
      AddNewMidiEvent(event);
      delete event;
    }
    MIDI_IN_DATA.clear();
    // --- //
    int numLuaChannels=0;
    int outChannelIndex=0;
    LoadNumber("time",time);
    int top=lua_gettop(__L);
    lua_getglobal(__L,"__process");
    if(lua_isfunction(__L, -1)){
      for(int i=0;i<numInChannels;i++) lua_pushnumber(__L,AUDIO_IN_CHANNEL_DATA[i]);
      switch(lua_pcall(__L,numInChannels,LUA_MULTRET,0)){
        case LUA_ERRRUN:
          ERROR_FLAG=true;
          fprintf(stderr,"%s\nError: __process threw an unrecoverable error and has been reset.\n", lua_tostring(__L, -1));
          lua_pop(__L, 1);
          luaL_dostring(__L,"__process=nil");
          return AUDIO_OUT_CHANNEL_DATA;
        case LUA_ERRMEM:ERROR_FLAG=true;fprintf(stderr,"MEMORY ALLOCATION ERROR\n");return 0;
        case LUA_ERRERR:ERROR_FLAG=true;fprintf(stderr,"ERROR HANDLING ERROR\n");return 0;
        default:break;
      }
      numLuaChannels=lua_gettop(__L)-top;
      for(int luaChannelIndex=0;luaChannelIndex<numLuaChannels;luaChannelIndex++){
        while(outChannelIndex>=numOutChannels) outChannelIndex-=numOutChannels;
        AUDIO_OUT_CHANNEL_DATA[outChannelIndex]=lua_tonumber(__L,-1)+(luaChannelIndex>=numOutChannels?AUDIO_OUT_CHANNEL_DATA[outChannelIndex]:0);
        outChannelIndex++;
        lua_pop(__L,1);
      }
    } else lua_pop(__L,1);
    return AUDIO_OUT_CHANNEL_DATA;
  }
  static void Shutdown(){
    if(__L) lua_close(__L);
  }
};
lua_State* Interpreter::__L;
float Interpreter::AUDIO_OUT_CHANNEL_DATA[256];
float Interpreter::AUDIO_IN_CHANNEL_DATA[256];
std::vector<MidiEvent*> Interpreter::MIDI_IN_DATA;
std::vector<MidiEvent*> Interpreter::MIDI_OUT_DATA;
bool Interpreter::ERROR_FLAG;

#endif
