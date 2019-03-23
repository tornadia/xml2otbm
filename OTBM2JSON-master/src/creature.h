////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __creature_h
#define __creature_h

#include "thing.h"
#include "position.h"
#include "survivalsystem.h"
#include "networkmessage.h"
#include <vector>
#include <map>

enum subfight_t {
  DIST_NONE = 0,
  DIST_BOLT = NM_ANI_BOLT,
  DIST_ARROW = NM_ANI_ARROW, 
  DIST_FIRE = NM_ANI_FIRE,
  DIST_ENERGY = NM_ANI_ENERGY,
  DIST_POISONARROW = NM_ANI_POISONARROW,
  DIST_BURSTARROW = NM_ANI_BURSTARROW,
  DIST_THROWINGSTAR = NM_ANI_THROWINGSTAR,
  DIST_THROWINGKNIFE = NM_ANI_THROWINGKNIFE,
  DIST_SMALLSTONE = NM_ANI_SMALLSTONE,
  DIST_SUDDENDEATH = NM_ANI_SUDDENDEATH,
  DIST_LARGEROCK = NM_ANI_LARGEROCK,
  DIST_SNOWBALL = NM_ANI_SNOWBALL,
  DIST_POWERBOLT = NM_ANI_POWERBOLT
};

enum damagecolors_t
{
     GREEN	= -189,
     ORANGE = 193,
     GRAY = 86,
     YELLOW = 210,
     RED = 180,
     DARK_BLUE = 2,
     LIGHT_BLUE = 71
};

enum fight_t {
	FIGHT_MELEE,
	FIGHT_DIST
};

enum playerLooks
{
	PLAYER_MALE_1=0x80,
	PLAYER_MALE_2=0x81,
	PLAYER_MALE_3=0x82,
	PLAYER_MALE_4=0x83,
	PLAYER_FEMALE_1=0x88,
	PLAYER_FEMALE_2=0x89,
	PLAYER_FEMALE_3=0x8A,
	PLAYER_FEMALE_4=0x8B,
};

enum defenses_t {
    DEFENSE_FIRE,
    DEFENSE_ENERGY,
    DEFENSE_POSION,
    DEFENSE_PHYSICAL,
};

class Map;
class Item;
class Thing;
class Player;

class Creature : public Thing
{
public:
  Creature(const char *name);
  virtual ~Creature() {};
  
  virtual const std::string& getName() const {return name; };
  unsigned long getID() const { return id; }
  void setDirection(Direction dir) { direction = dir; }
  virtual fight_t getFightType(){return FIGHT_MELEE;};
  virtual subfight_t getSubFightType() {return DIST_NONE;}
  virtual void drainHealth(int);
  virtual void drainMana(int);
  virtual std::string getDescription(bool self = false);
  virtual void setAttackedCreature(unsigned long id);
  virtual int getShieldDef() const{};
  virtual int getArm() const{};
  virtual void dropLoot(Item *item) {};
  virtual bool isAttackable() { return true; };
  virtual bool isPushable() const {return true;}
  virtual int sendInventory(){return 0;};
  virtual int addItem(Item* item, int pos){return 0;};
  virtual Item* getItem(int pos){return NULL;}
  virtual Direction getDirection(){return direction;}
  Direction getDirection() const { return direction; }
  
  int access;
  int maglevel;
  int level;
  int speed;
  
  unsigned long experience;
  double givenxp;
  
  unsigned short int lookhead, lookbody, looklegs, lookfeet, looktype, lookcorpse, lookmaster;
  long inFightTicks, exhaustedTicks, manaShieldTicks, hasteTicks, lightTicks;
  unsigned short lightLevel;
  int mana, manamax, manaspent;
  int health, healthmax;
  bool pzLocked;
  bool defenses[4];
  Position masterPos;
  int damageColor;
  bool shouldrespawn;
  unsigned int respawntime;
  unsigned short int dirr;
  unsigned long attackedCreature;
  
  uint64_t lastmove;
  uint64_t movedelay;
  
  unsigned short getSpeed() const { return speed; };
  void setNormalSpeed(){ if(access == 3){ speed = 1000; return; } speed = 220 + (2* (level - 1)); };
  int getNormalSpeed(){ if(access == 3){ return 1000; } return 220 + (2* (level - 1)); };
  int getExpForLv(int lv) { return (int)(((50 * (lv-1) * (lv-1) * (lv-1)) - ((150 * (lv-1)) * (lv-1)) + (400 * (lv-1))) / 3); };
  virtual int getWeaponDamage() const { return 1+(int)(10.0*rand()/(RAND_MAX+1.0)); };
  virtual int getStepDuration(int underground) { return (1000*120*100)/(getSpeed()*underground); };
  virtual int getWalkDuration();
  
  virtual bool canMovedTo(Tile *tile);
  virtual void sendCancel(const char *msg) { };
  virtual void sendCancelWalk(const char *msg) { };
  
    virtual void addInflictedDamage(Creature* attacker, int damage);
	virtual int getGainedExperience(Creature* attacker);
	virtual std::vector<long> getInflicatedDamageCreatureList();
	
protected:
	virtual int getInflicatedDamage(Creature* attacker);
	virtual int getInflicatedDamage(unsigned long id);
	virtual int getTotalInflictedDamage();
	
	typedef std::vector< std::pair<uint64_t, long> > DamageList;
	typedef std::map<long, DamageList > TotalDamageList;
	TotalDamageList totaldamagelist;
       	
private:
  virtual void onThink(){};
  virtual void onThingMove(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos) { };
  virtual void onTeleport(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos, bool magiceffect) { };
  virtual void onCreatureAppear(const Creature *creature) { };
  virtual void onCreatureDisappear(const Creature *creature, unsigned char stackPos) { };
  virtual void onCreatureTurn(const Creature *creature, unsigned char stackPos) { };
  virtual void onCreatureSay(const Creature *creature, unsigned char type, const std::string &text) { };
  virtual void onCreatureChangeOutfit(const Creature* creature) { };
  virtual void onTileUpdated(const Position *Pos) { };
  virtual void onContainerUpdated(Item *item, unsigned char from_id, unsigned char to_id, unsigned char from_slot, unsigned char to_slot, bool remove) {};

  friend class Map;

  Direction          direction;
  unsigned long      id;
  std::string        name;
};

#endif
