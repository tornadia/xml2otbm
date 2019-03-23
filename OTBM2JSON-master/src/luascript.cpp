////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>

#include "luascript.h"

LuaScript::LuaScript()
{
  luaState = NULL;
}


LuaScript::~LuaScript()
{
  if (luaState)
	  lua_close(luaState);
}


int LuaScript::OpenFile(const char *filename)
{
	luaState = lua_open();

	if (lua_dofile(luaState, filename))
		return false;

  return true;
}


std::string LuaScript::getGlobalString(std::string var, const std::string &defString)
{
	lua_getglobal(luaState, var.c_str());

  if(!lua_isstring(luaState, -1))
  	  return defString;

	int len = (int)lua_strlen(luaState, -1);
	std::string ret(lua_tostring(luaState, -1), len);
	lua_pop(luaState,1);

	return ret;
}

int LuaScript::getGlobalNumber(std::string var, const int defNum)
{
	lua_getglobal(luaState, var.c_str());

  if(!lua_isnumber(luaState, -1))
  	  return defNum;

	int val = (int)lua_tonumber(luaState, -1);
	lua_pop(luaState,1);

	return val;
}


int LuaScript::setGlobalString(std::string var, std::string val)
{
	return false;
}

int LuaScript::setGlobalNumber(std::string var, int val){
	lua_pushnumber(luaState, val);
	lua_setglobal(luaState, var.c_str());
	return true;
}

std::string LuaScript::getGlobalStringField (std::string var, const int key, const std::string &defString) {
      lua_getglobal(luaState, var.c_str());

      lua_pushnumber(luaState, key);
      lua_gettable(luaState, -2);  /* get table[key] */
      if(!lua_isstring(luaState, -1))
  	  return defString;
      std::string result = lua_tostring(luaState, -1);
      lua_pop(luaState, 2);  /* remove number and key*/
      return result;
}

int LuaScript::getField (const char *key) {
      int result;
      lua_pushstring(luaState, key);
      lua_gettable(luaState, -2);  /* get table[key] */
      result = (int)lua_tonumber(luaState, -1);
      lua_pop(luaState, 1);  /* remove number and key*/
      return result;
}

void LuaScript::setField (const char *index, int val) {
      lua_pushstring(luaState, index);
      lua_pushnumber(luaState, (double)val);
      lua_settable(luaState, -3);
    }
