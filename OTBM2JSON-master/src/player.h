////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __player_h_
#define __player_h_

#include "creature.h"


class Protocol;

enum slots_t {
	SLOT_WHEREEVER=0,
	SLOT_HEAD=1,
	SLOT_NECKLACE=2,
	SLOT_BACKPACK=3,
	SLOT_ARMOR=4,
	SLOT_RIGHT=5,
	SLOT_LEFT=6,
	SLOT_LEGS=7,
	SLOT_FEET=8,
	SLOT_RING=9,
	SLOT_AMMO=10
};

enum skills_t {
    SKILL_FIST,
    SKILL_CLUB,
    SKILL_SWORD,
    SKILL_AXE,
    SKILL_DIST,
    SKILL_SHIELD,
    SKILL_FISH
};

enum skillsid_t {
    SKILL_LEVEL,
    SKILL_TRIES
};

class NetworkMessage;

// Defines a player...

class Player : public Creature
{
public:
	Player(const char *name, Protocol* p);
	virtual ~Player();

    //Item Functions
    Item* emptySlot();
    Item* empty;
    Item* getItem(int pos);
	int addItem(Item* item, int pos);
	unsigned int getContainerCount() {return vcontainers.size();}; //Returns the current number of containers open
	Item* getContainer(unsigned char containerid);
	unsigned char getContainerID(Item* container);
	void addContainer(unsigned char containerid, Item *container);
	void closeContainer(unsigned char containerid);
	void closeContainerWithoutRemove(unsigned char containerid);
    
	std::string getName(){return name;};
	void speak(const std::string &text);
	
  unsigned short int sex, voc;
  int cap;
  int pking;
  //Cash
  unsigned long cash;
  unsigned long cashtrade;
  
  bool premmy;
  unsigned int time;
  
  //Info of chest of quests
  double openchest[10];
  
  unsigned int pled, mled;
  //bool poisoned, burning, energy, drunk, paralised;
  int food;
  
  //Guild infos
  unsigned short int guildstatus; 
  unsigned short int guildnicks; 
  std::string guildname; 
  std::string guildrank; 
  std::string guildnick; 
  
  bool cancelMove;
  unsigned long followedCreature;
  
  virtual int getWeaponDamage() const;
  virtual int getShieldDef() const;
  virtual int getArm() const;
  
  int fightMode, followMode;
  int accountNumber;
  std::string accountPassword;  
  int skills[7][2];
  
  int timeMana;
  int timeLife;
  
  //reminder: 0 = None, 1 = Sorcerer, 2 = Druid, 3 = Paladin, 4 = Knight
  unsigned short CapGain[5];          //for level advances
  unsigned short ManaGain[5];
  unsigned short HPGain[5];
  
  //for skill advances
  unsigned int getReqSkilltries (int skill, int level, int voc);
  
  //for magic level advances
  unsigned int getReqMana(int maglevel, int voc); 
  

  //items
  Item* items[11]; //equipement of the player
	typedef std::pair<unsigned char, Item*> containerItem;
	typedef std::vector<containerItem> containerLayout;
	containerLayout vcontainers;
	bool containerToClose[16];

  void    usePlayer() { useCount++; };
  //void    releasePlayer() { useCount--; if (useCount == 0) delete this; };
  
  void releasePlayer();

  fight_t getFightType();
  subfight_t getSubFightType();
  
  void sendIcons();
  void sendLight(const int llevel, int lcolor); 
  bool CanSee(int x, int y);
  void addSkillTry(int skilltry);
  void addSkillShieldTry(int skilltry);
  void sendNetworkMessage(NetworkMessage *msg);
  void sendCancelAttacking();
  void sendCancelFollowing();
  void sendChangeSpeed(Creature* creature);
  void savePlayer(std::string &name);
  void sendToChannel(Creature *creature, unsigned char type, const std::string &text, unsigned short channelId);
  virtual void sendCancel(const char *msg);
  virtual void sendKick();
  virtual unsigned long getIP();
  virtual void sendCancelWalk(const char *msg);
  virtual void setAttackedCreature(unsigned long id);
  virtual bool isAttackable() { return (access == 0); };
  virtual bool isPushable() const { return (access == 0); };
  virtual void dropLoot(Item *item) {};

protected:
  virtual void onThingMove(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos);
  virtual void onTeleport(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos, bool magiceffect);
  virtual void onCreatureAppear(const Creature *creature);
  virtual void onCreatureDisappear(const Creature *creature, unsigned char stackPos);
  virtual void onCreatureTurn(const Creature *creature, unsigned char stackpos);
  virtual void onCreatureSay(const Creature *creature, unsigned char type, const std::string &text);
  virtual void onCreatureChangeOutfit(const Creature* creature);
  virtual void onThink();
  virtual void onTileUpdated(const Position *Pos);
  virtual void onContainerUpdated(Item *item, unsigned char from_id, unsigned char to_id, unsigned char from_slot, unsigned char to_slot, bool remove);	
  virtual std::string getDescription(bool self = false);
  void saveContainer(Item* item, xmlNodePtr pn, std::stringstream &sb);
    
    int useCount;
    Protocol *client;
	std::string name, password;
};


#endif // __player_h_
