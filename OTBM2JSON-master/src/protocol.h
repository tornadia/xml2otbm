////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <string>

#include "definitions.h"
#include "map.h"
#include "player.h"

// Base class to represent different protocols...
class Protocol
{
public:
  Protocol();
  virtual ~Protocol();

  void setPlayer(Player* p);
  virtual bool CanSee(int x, int y) = 0;
  virtual void sleepTillMove();
  virtual unsigned long GetProtocolIp();
  virtual void sendNetworkMessage(NetworkMessage *msg) = 0;
  virtual void sendThingMove(const Creature *creature, const Thing *thing, const Position *oldPos, unsigned char oldStackPos) = 0;
  virtual void sendTeleport(const Creature *creature, const Thing *thing, const Position *oldPos, unsigned char oldStackPos, bool magiceffect) = 0;
  virtual void sendKick() = 0;
  virtual void sendLightLevel(const int level, int color) = 0; 
  virtual void sendCreatureAppear(const Creature *creature) = 0;
  virtual void sendCreatureDisappear(const Creature *creature, unsigned char stackPos) = 0;
  virtual void sendCreatureTurn(const Creature *creature, unsigned char stackPos) = 0;
  virtual void sendCreatureSay(const Creature *creature, unsigned char type, const std::string &text) = 0;
  virtual void sendSetOutfit(const Creature* creature) = 0;
  virtual void sendTileUpdated(const Position *Pos) = 0;
  virtual void sendContainerUpdated(Item *item, unsigned char from_id, unsigned char to_id, unsigned char from_slot, unsigned char to_slot, bool remove) = 0;
  virtual void sendIcons(int icons) = 0;
  virtual void sendCancel(const char *msg) = 0;
  virtual void sendCancelWalk(const char *msg) = 0;
  virtual void sendChangeSpeed(const Creature* creature) = 0;
  virtual void sendCancelAttacking() = 0;
  virtual void sendCancelFollowing() = 0;
  virtual void sendChannels() = 0;
  virtual void sendChannel(unsigned short channelId) = 0;
  virtual void sendToChannel(const Creature * creature, unsigned char type, const std::string &text, unsigned short channelId) = 0;
  virtual void sendOpenPriv(std::string &receiver) =0;
  
protected:
  Map    *map;
  Player *player;
  SOCKET s;
};


#endif  // #ifndef __PROTOCOL_H__
