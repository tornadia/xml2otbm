////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __npc_h_
#define __npc_h_

#include "creature.h"
#include "map.h"
#include "tools.h"
#include "luascript.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

// Defines an NPC...     
class TimeProbabilityClass {
public:
	TimeProbabilityClass()
	{
		setDefault();
	}

	TimeProbabilityClass(int _cycleTicks, int _probability, int _times = 0)
	{
		setDefault();
		init(_cycleTicks, _probability, _times);
	};

	~TimeProbabilityClass() {};

	bool onTick(int ticks)
	{
		ticksleft -= ticks;
		
		if(ticksleft <= 0) {
			ticksleft = cycleTicks;
			bool ret = (random_range(1, 100) <= probability);
			return ret;
		}

		return false;
	}
	
	void init(int _cycleTicks, int _probability, int _times)
	{
		if(_cycleTicks >= 0) {
			this->ticksleft = _cycleTicks;
			this->cycleTicks = _cycleTicks;
		}

		_probability = std::max(_probability, 0);

		if(_probability >= 0)
			probability = std::min(100, _probability);
			
		times = _times;	
	}
	
	int times;
private:
	void setDefault()
	{
        times = 0; 
		cycleTicks = 5000;
		ticksleft = cycleTicks;
		probability = 50;
	}

	int ticksleft;
	int cycleTicks;
	int probability;
};

class PhysicalAttackClass {
public:
	PhysicalAttackClass()
	{
		disttype = DIST_NONE;
		minWeapondamage = 0;
		maxWeapondamage = 1;
	};

	~PhysicalAttackClass() {};

	fight_t fighttype;
	subfight_t disttype;

	int minWeapondamage;
	int maxWeapondamage;
};

class SpellAttackClass {
public:
	SpellAttackClass()
	{
		minDamage = 0;
		maxDamage = 10;
		spellname = "";
	};
	
	~SpellAttackClass() {};

	int minDamage;
	int maxDamage;
	std::string spellname;
};

class Npc;
class NpcScript : protected LuaScript{
public:
	NpcScript(std::string name, Npc* npc);
	virtual ~NpcScript(){}
//	virtual void onThingMove(const Player *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos);
//	virtual void onTeleport(const Player *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos, bool magiceffect);
	virtual void onCreatureAppear(int cid);
	virtual void onCreatureDisappear(int cid);
	virtual void onCreatureSay(int cid, unsigned char type, const std::string &text);
	virtual void onThink();
	static Npc* getNpc(lua_State *L);
	static int luaActionSay(lua_State *L);
	static int luaActionDoAnyMove(lua_State *L);
	static int luaActionMove(lua_State *L);
	static int luaActionGotoMasterPos(lua_State *L);
	static int luaActionTurn(lua_State *L);
	static int luaActionMoveTo(lua_State *L);
	static int luaCreatureGetName(lua_State *L);
	static int luaCreatureGetName2(lua_State *L);
	static int luaActionAttackCreature(lua_State *L);
	static int luaCreatureGetPos(lua_State *L);
	static int luaFindPlayer(lua_State *L);
	static int luaSelfGetPos(lua_State *L);
	static int luaCreatureTeleport(lua_State *L);
	static int luaCreatureAddItem(lua_State *L);
	static int luaCreatureGetItem(lua_State *L);
	static int luaCreatureSeeCash(lua_State *L);
	static int luaCreatureGetCash(lua_State *L);
	static int luaCreatureAddCash(lua_State *L);
	static int luaCreatureSeeHealth(lua_State *L);
	static int luaCreatureGetHealth(lua_State *L);
	static int luaCreatureAddHealth(lua_State *L);

    bool isLoaded(){return loaded;}
protected:
	int registerFunctions();
	Npc* npc;
	bool loaded;
};

class Npc : public Creature
{
public:
  Npc(const char *name, Map* map, bool onExp = true);
  virtual ~Npc();
  
  virtual void onAttack();
  virtual bool isPlayer() const { return false; };
  virtual bool isMonster() const { if(monster) return true; return false;};
  virtual void dropLoot(Item *item);
  void speak(const std::string &text){};
  int getDistanceToTarget(Position toPos, Position fromPos);
  
  std::string getName(){return name;};
  fight_t getFightType(){if(curPhysicalAttack != NULL) return curPhysicalAttack->fighttype; else return FIGHT_MELEE;};
  subfight_t getSubFightType()  {if(curPhysicalAttack != NULL) return curPhysicalAttack->disttype; else return DIST_BOLT;}
  
  bool firstthink;
  int defense, armor;
  int change;
  int changeper;
  bool pushable;
  bool monster;
  int combat_dist;
  int runLife;
  Map* map;
  
  int getMinSpellDamage(){if(curSpellAttack != NULL) return curSpellAttack->minDamage; else return 0;};
  int getMaxSpellDamage(){if(curSpellAttack != NULL) return curSpellAttack->maxDamage; else return 0;};
  virtual int getWeaponDamage() const 
  { 
    if(curPhysicalAttack != NULL)
       return random_range(curPhysicalAttack->minWeapondamage, curPhysicalAttack->maxWeapondamage);
	else
	   return 0;
  };
  
  virtual int getShieldDef() const { return defense; };
  virtual int getArm() const {return armor; };
  virtual bool isPushable() const { return pushable; };
  
  void doSay(std::string msg);
  void doMove(int dir);
  void doMoveTo(Position pos);
  void doMoveAi();
  void doAnyMove();
  void doTurn(Creature *creature, int dire);
  void doTurnAi();
  void doAttack(int id);
  Position calculeMove(Position toPos, bool run = false);
  bool isLoaded(){return loaded;}

protected:      	
    PhysicalAttackClass	*curPhysicalAttack;
    SpellAttackClass *curSpellAttack;
    
     
	bool doAttacks(Player* attackedPlayer);
	bool changeCreature();
	bool isInRange(const Position &pos);	
      
    typedef std::vector<TimeProbabilityClass> TimeProbabilityClassVec;
    
    typedef std::map<SpellAttackClass*, TimeProbabilityClassVec> AttackSpells;
    AttackSpells npcSpells;
    
	typedef std::map<PhysicalAttackClass*, TimeProbabilityClass> PhysicalAttacks;
	PhysicalAttacks physicalAttacks;
	
	typedef std::map<int, int> lootItems;
    lootItems npcLoots;
    lootItems containerLoots;
    int containerLoot;
    
	typedef std::map<std::string, TimeProbabilityClass> YellingSentences;
	YellingSentences yellingSentences;
	YellingSentences summonSentences;
	
  	void OnCreatureEnter(const Creature *creature);
	void OnCreatureLeave(const Creature *creature);
            
  virtual void onThingMove(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos);
  virtual void onTeleport(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos, bool magiceffect);
  virtual void onCreatureAppear(const Creature *creature);
  virtual void onCreatureDisappear(const Creature *creature, unsigned char stackPos);
  virtual void onCreatureSay(const Creature *creature, unsigned char type, const std::string &text);
  virtual void onThink();
  virtual std::string getDescription(bool self = false);
  
  std::string name;
  std::string scriptname;
  NpcScript* script;
  std::list<Position> route;
  bool loaded;
};


#endif
