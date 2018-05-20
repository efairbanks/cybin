#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.hpp>

#define CYBIN_PROMPT "io.write(\"cybin> \")\n"

class Interpreter{
  static lua_State* __L;
  public:
  static void Init(){
    if(!__L){
      __L=luaL_newstate();
      luaL_openlibs(__L);
    }
  }
  static void LoadFile(char* filename){
    if(__L)
      if(!luaL_loadfile(__L,filename))
        lua_pcall(__L,0,0,0);
  }
  static void EventLoop(char* buff,int* lock){
    (*lock)++;
    while((*lock)>1);
    int error = luaL_loadbuffer(__L, buff, strlen(buff), "line") ||
      lua_pcall(__L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "%s\n", lua_tostring(__L, -1));
      lua_pop(__L, 1);  /* pop error message from the stack */
    }
    // --- print prompt --- //
    
    // ------ // 
    (*lock)--;
  }
  static double Process(double sr){
    lua_getglobal(__L,"__process");
    lua_pushnumber(__L,sr);
    if(lua_pcall(__L,1,1,0)) return 0; // error!!
    double result=lua_tonumber(__L,-1);
    lua_pop(__L,1);
    return result;
  }
  static void Shutdown(){
    if(__L) lua_close(__L);
  }
};
lua_State* Interpreter::__L;

#endif
