////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __LUASCRIPT_H__
#define __LUASCRIPT_H__

#include <string>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

extern "C" struct lua_State;


class LuaScript
{
public:
	LuaScript();
	~LuaScript();

  int OpenFile(const char* file);
  int getField (const char *key);
  void LuaScript::setField (const char *index, int value);
  // get a global string
  std::string getGlobalString(std::string var, const std::string &defString = "");
  int getGlobalNumber(std::string var, const int defNum = 0);
  std::string getGlobalStringField (std::string var, const int key, const std::string &defString = "");
  // set a var to a val
  int setGlobalString(std::string var, std::string val);
  int setGlobalNumber(std::string var, int val);

protected:
	std::string luaFile;   // the file we represent
	lua_State*  luaState;  // our lua state
};


#endif  // #ifndef __LUASCRIPT_H__
