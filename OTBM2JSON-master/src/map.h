////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __SURVIVALSERV_MAP_H
#define __SURVIVALSERV_MAP_H

#include <queue>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "position.h"
#include "item.h"
#include "creature.h"
#include "house.h"
#include "magic.h"
#include "survivalsystem.h"
#include "scheduler.h"
#include "networkmessage.h"

#define MAP_WIDTH    512
#define MAP_HEIGHT   512
#define MAP_LAYER     16

class Creature;
class Npc;
class Player;
class Tile;
class House;
class Quests;

class Range {
public:
	Range(Position centerpos, bool multilevel = false) {
		this->startx = centerpos.x - 9;
		this->endx = centerpos.x + 9;
		this->starty = centerpos.y - 7;
		this->endy = centerpos.y + 7;
		if(multilevel)
			this->startz = 0;
		else
			this->startz = centerpos.z;
		this->endz = centerpos.z;
		this->multilevel = multilevel;
	}

	Range(int startx, int endx, int starty, int endy, int z, bool multilevel = false)
	{
		this->startx = startx;
		this->endx = endx;
		this->starty = starty;
		this->endy = endy;
		if(multilevel)
			this->startz = 0;
		else
			this->startz = startz;
		this->endz = z;
		this->multilevel = multilevel;
	}

	Range(int startx, int endx, int starty, int endy, int startz, int endz)
	{
		this->startx = startx;
		this->endx = endx;
		this->starty = starty;
		this->endy = endy;
		this->startz = startz;
		this->endz = endz;
		this->multilevel = multilevel;
	}

	int startx;
	int endx;
	int starty;
	int endy;
	int startz;
	int endz;
	bool multilevel;
};

struct targetdata {
	int damage;
	int manaDamage;
	unsigned char stackpos;
	bool hadSplash;
};

struct tiletargetdata {
	Position pos;
	int targetCount;
	unsigned char thingCount;
};

struct AStarNode{
	int x,y;
	AStarNode* parent;
	float f, g, h;
	bool operator<(const AStarNode &node){return this->h < node.h;}
};

template<class T> class lessPointer : public std::binary_function<T*, T*, bool> {
		  public:
		  bool operator()(T*& t1, T*& t2){
				return *t1 < *t2;
		  }
};

class Map {
  public:
    Map();
    ~Map();
    
	Quests* quests;
	bool servbloked;
    bool serverrun;	
	bool manutencao;
	int PVP_MODE;
    int lightdelta, lighttick, lightcolor; 
    long charsonline;
	uint32_t max_players;
	unsigned long long timeonline;
    bool LoadMap(std::string filename);
    void onTimeThink(int timetask);
    Tile* getTile(unsigned short _x, unsigned short _y, unsigned char _z);

    std::map<long, Creature*> playersOnline;
    std::map<long, House*> housesGame;

    bool placeCreature(Creature* c);
    bool removeCreature(Creature* c);
    
    void thingMove(Creature *player, Thing *thing, unsigned short to_x, unsigned short to_y, unsigned char to_z);
    void thingMove(Creature *player, unsigned short from_x, unsigned short from_y, unsigned char from_z, unsigned char stackPos, unsigned short to_x, unsigned short to_y, unsigned char to_z, unsigned char count);
	void creatureBroadcastTileUpdated(const Position& pos);
    void creatureTurn(Creature *creature, Direction dir);
    void creatureSay(Creature *creature, unsigned char type, const std::string &text);
    void creatureWhisper(Creature *creature, const std::string &text);
    void creatureMonsterYell(Npc* monster, const std::string& text);
    void creatureYell(Creature *creature, std::string &text);
    void creatureSpeakTo(Creature *creature, const std::string &receiver, const std::string &text);
    void creatureBroadcastMessage(Creature *creature, const std::string &text);
    void creatureAdvancedMessage(Creature *creature, const std::string &text);
    void creatureGodMessage(const std::string &text);
    void creatureToChannel(Creature *creature, unsigned char type, const std::string &text, unsigned short channelId);
	void creatureChangeOutfit(Creature *creature);
	bool creatureThrowRune(Creature *creature, const MagicEffectClass& me);
  	void creatureCastSpell(Creature *creature, const MagicEffectClass& me);
	bool creatureSaySpell(Creature *creature, const std::string &text);
    void changeOutfitAfter(unsigned long id, int looktype, long time);
    void changeSpeed(unsigned long id, unsigned short speed);
    void creatureDrinkMana(Creature *creature);
	void addEvent(SchedulerTask*);
	void CallCloseHole(Position holepos);
	void CloseHole(Position holepos);
	void closeServer(Creature *lol);
	void RespawnCreature(Creature *oldcreature);
	void teleportPlayer(Position toPos, Creature *c, bool magic, Tile *toTile, bool ground);
	void savePlayer(unsigned long id);
	void needContainerUpdate(Item *container, Creature *player, Item *item, unsigned char from_slot, unsigned char to_slot, bool remove, bool updatecount);
    void pkTime(Player *p);
    void pkPlayer(Creature *c);
    
    void getSpectators(const Range& range, std::vector<Creature*>& list);
	
    Creature* findCreature(Creature *c);
    Creature* getCreatureByID(unsigned long id);
	Creature* getCreatureByName(const char* s);
	
	House* getHouseByID(unsigned long id);
	House* getHouseByName(const char* s);	

	std::list<Position> getPathTo(Position start, Position to, bool creaturesBlock = true);
	bool canThrowItemTo(Position from, Position to, bool creaturesBlock/* = true*/, bool isProjectile = false);
    
    SURVIVALSYS_THREAD_LOCKVAR mapLock;
  protected:
    // use this internal function to move things around to avoid the need of recursive locks
    void thingMoveInternal(Creature *player, unsigned short from_x, unsigned short from_y, unsigned char from_z, unsigned char stackPos, unsigned short to_x, unsigned short to_y, unsigned char to_z, unsigned char count);        
    void changeOutfit(unsigned long id, int looktype);
	bool creatureOnPrepareAttack(Creature *creature, Position pos);
	void creatureMakeDamage(Creature *creature, Creature *attackedCreature, fight_t damagetype);
	bool creatureMakeMagic(Creature *creature, const MagicEffectClass* me);
    void checkLight(const long omg);
	void CreateDamageUpdate(Creature* player, Creature* attackCreature, int damage, NetworkMessage& msg);
	void getAreaTiles(Position pos, const unsigned char area[14][18], unsigned char dir, std::list<tiletargetdata>& list);
	void creatureApplyMagicEffect(Creature *target, const MagicEffectClass& me, NetworkMessage& msg);
	void CreateManaDamageUpdate(Creature* player, Creature* attackCreature, int damage, NetworkMessage& msg);

    SURVIVALSYS_THREAD_LOCKVAR eventLock;
	SURVIVALSYS_THREAD_SIGNALVAR eventSignal;

    static SURVIVALSYS_THREAD_RETURN eventThread(void *p);

    struct MapEvent
    {
      __int64  tick;
      int      type;
      void*    data;
    };

    void checkPlayerAttacking(unsigned long id);
    void checkPlayerFollowing(unsigned long id);
    void checkPlayer(unsigned long id);
    void decayItem(Item* item);
    void decaySplash(Item* item);

	std::priority_queue<SchedulerTask*, std::vector<SchedulerTask*>, lessSchedTask > eventList;

    int loadMapXml(const char *filename);
    int loadHousesXml(std::string filename);
    void saveHouse(House* house);

    typedef std::map<unsigned long, Tile*> TileMap;
    TileMap tileMaps[64][64][MAP_LAYER];

    void Map::setTile(unsigned short _x, unsigned short _y, unsigned char _z, unsigned short groundId);
    friend class Npc;
	//uint32_t max_players;
};
    
#endif
