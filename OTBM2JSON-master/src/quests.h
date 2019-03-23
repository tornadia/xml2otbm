////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUESTS_H
#define __QUESTS_H

#include "map.h"
#include "luascript.h"
#include "position.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class Item;
class QuestLoot{
  public:
    QuestLoot(Item *item, Position itemPos, Map* map);
    ~QuestLoot();
    
    void addItemList(Item item) {bpItems.push_front(item);};
    int getItemListSize() {return bpItems.size();};
    Item* getLoot();
    
    bool bploot;
    Position pos;
    Item* chest;
    Item itemloot;    
    std::string description;
  protected:
    Map *map;        
    std::list<Item > bpItems;             
};
class QuestScript;
class Quest{
  public:
    Quest(const char *questname, Map* map);
    ~Quest();
    Map* map;
    QuestScript *questScript;
    void playerDoQuest(Player *player, Position pos);
    void addName(std::string name);
    bool removeName(std::string name);
    bool isNameinList(std::string name);
    void addQuestLoot(QuestLoot* questloot) {listQuestloots.push_front(questloot);};
    std::string getName() {return questname;};
  protected:
    void saveNamesList();
    QuestLoot* getQuestLootByPos(Position pos);        
    std::list<std::string> listNames;
    std::list<QuestLoot *> listQuestloots;
    std::string questname;             
};

class Quests{
  public:
    Quests(const char *arfq, Map* map);
    ~Quests();
    void addQuest(Quest* quest) {listQuests.push_front(quest);};
    Quest* getQuestByName(std::string name);
    //Quest* getQuestByChestPos(Position pos);
    int getQuestListSize() {return listQuests.size();};
    Map* map;
    QuestScript *questScript;
  protected:
    std::list<Quest *> listQuests;                      
};

class QuestScript : protected LuaScript{
public:
	QuestScript(Quests* quests);
	virtual ~QuestScript(){}
	virtual void onCreaturePullLever(int cid, Position leverPos, int leverSide);
	virtual void onCreatureSinkTile(int cid, Position tilePos);
	virtual void onCreatureTeleport(int cid, Position teleportPos);
	virtual void onCreatureOpenContainerQuest(int cid, std::string questName, Position containerPos);
    virtual void onThink();
    static Quests* getQuests(lua_State *L);
	//static int luaAddMagicEffect(lua_State *L);
	//static int luaRespawnCreature(lua_State *L);
	static int luaLeverMove(lua_State *L);
	static int luaLeverGetSide(lua_State *L);
	//static int luaItemGet(lua_State *L);
	//static int luaItemAdd(lua_State *L);
	//static int luaItemSee(lua_State *L);
	//static int luaTileUp(lua_State *L);
	//static int luaTileDown(lua_State *L);
	//static int luaTeleportAdd(lua_State *L);
	//static int luaTeleportRemove(lua_State *L);
    //static int luaGetCreatureByPos(lua_State *L);
	//static int luaGetItemIdByPos(lua_State *L);
	//static int luaGetQuestNameByPos(lua_State *L);
	//static int luaDoorOpen(lua_State *L);
	//static int luaDoorClose(lua_State *L);
	//static int luaXmlAddName(lua_State *L);
	//static int luaXmlGetName(lua_State *L);
	//static int luaXmlSeeName(lua_State *L);
	static int luaCreatureAddText(lua_State *L);
    static int luaCreatureGetName(lua_State *L);
    static int luaCreatureGetLevel(lua_State *L);
	static int luaCreatureGetID(lua_State *L);
	static int luaCreatureGetPos(lua_State *L);
	static int luaCreatureTeleport(lua_State *L);
	static int luaCreatureSeeCash(lua_State *L);
	static int luaCreatureGetCash(lua_State *L);
	static int luaCreatureAddCash(lua_State *L);
	static int luaCreatureSeeHealth(lua_State *L);
	static int luaCreatureAddHealth(lua_State *L);
    static int luaCreatureGetHealth(lua_State *L);
	
    bool isLoaded(){return loaded;}
protected:
	int registerFunctions();
	bool loaded;
	Quests* quests;
};

#endif




