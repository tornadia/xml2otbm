////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __TILE_H__
#define __TILE_H__

#include "definitions.h"
#include "position.h"
#include "item.h"

class Creature;

typedef std::vector<Item*> ItemVector;
typedef std::vector<Creature*> CreatureVector;

class Tile
{
public:
  Creature* getCreature(){
    return creatures[0];
  }

  Tile(Position position)
  {
    housedoor        = false;
    gatelevel        = false;     
    pz               = false;
    tele             = false;
    usetele          = false;
    ropetele         = false;
    splash           = NULL;
    decaySplashAfter = 0;
    gatelvl          = 0;
    pos              = position;
  }

  Item           ground;
  Item*          splash;
  ItemVector     topItems;
  CreatureVector creatures;
  ItemVector     downItems;

  __int64        decaySplashAfter;

  bool removeThing(Thing *thing);
  void addThing(Thing *thing);

  int getCreatureStackPos(Creature *c);
  int getThingStackPos(Thing *thing);
  int getThingCount();
  
  //Teleport information	
  Position teleportPos;
  bool magiceffect;
  
  //Level of the gate
  unsigned int short gatelvl;
  
  //Position of the tile
  Position pos;
  
  Thing* getThingByStackPos(int pos);
  
  bool isHouseDoor();
  bool isGateLvl();
  bool isBlocking();
  bool isBlockingWalk();
  bool isBlockingProjectile();
  bool isPz();
  bool isTele();
  bool isRopeTele();
  bool isUseTele();
  void setGateLvl();
  void setHouseDoor();
  void setPz();
  void setTele();
  void setUseTele();
  void setRopeTele();
  
  std::string getDescription();
protected:
  bool gatelevel;        
  bool housedoor;        
  bool pz;
  bool tele;
  bool ropetele;
  bool usetele;
};


#endif

