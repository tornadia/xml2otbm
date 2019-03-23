////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __spells_h_
#define __spells_h_

#include "map.h"
#include "luascript.h"
#include "player.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}


class Spell;
class SpellScript;

class Spells
{
public:
  Spells(Map* map);
  bool loadFromXml();
  virtual ~Spells();

  Map* map;

  bool isLoaded(){return loaded;}
  std::map<std::string, Spell*>* getVocSpells(int voc){
                        if(voc>maxVoc || voc<0){
                                      return 0;
                                      }
                        return &(vocationSpells.at(voc));
                        } 
  std::map<std::string, Spell*>* getAllSpells(){
                        return &allSpells;
                        }
protected:

  std::map<std::string, Spell*> allSpells;
  std::vector<std::map<std::string, Spell*> > vocationSpells;
  bool loaded;
  int maxVoc;
};


class Spell
{
public:
  Spell(std::string name, std::string words, int magLv, int mana, Map* map);
  virtual ~Spell();

  Map* map;

  bool isLoaded(){return loaded;}
  SpellScript* getSpellScript(){return script;};
  std::string getWords(){return words;};
  int getMana(){return mana;};
  int getMagLv(){ return magLv;};
  
protected:
  std::string name, words;
  int magLv, mana;
  bool loaded;
  SpellScript* script;
};

class SpellScript : protected LuaScript{
public:
  SpellScript(std::string scriptname, Spell* spell);
  virtual ~SpellScript(){}
  
  void castSpell(Creature* creature, std::string var);
  bool isLoaded(){return loaded;}
  
  static Spell* SpellScript::getSpell(lua_State *L);
  static int SpellScript::luaActionDoSpell(lua_State *L);
  static int luaActionGetPos(lua_State *L);
  static int luaActionChangeOutfit(lua_State *L);
  static int luaActionManaShield(lua_State *L);
  static int luaActionChangeSpeed(lua_State *L);
  static int luaActionGetSpeed(lua_State *L);
protected:
    int registerFunctions();
	Spell* spell;
	bool loaded;      
};

#endif
