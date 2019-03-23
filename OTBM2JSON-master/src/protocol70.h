////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef tprot70_h
#define tprot70_h

#include "protocol.h"
#include "creature.h"
#include "item.h"
#include <string>


class NetworkMessage;

class Protocol70 : public Protocol
{
public:
  // Message the client sent
  Protocol70(SOCKET s);
  virtual ~Protocol70();

  bool ConnectPlayer();
  void ReceiveLoop();

private:
  // The socket the player is on...
  std::list<unsigned long> knownPlayers;
  void checkCreatureAsKnown(unsigned long id, bool &known, unsigned long &removedKnown);

  // Parse Methods
  void parsePacket(NetworkMessage &msg);
  void parseLogout(NetworkMessage &msg);
  void parseCancelMove(NetworkMessage &msg);
  void parseModes(NetworkMessage &msg);
  void parseDebug(NetworkMessage &msg);
  void parseMoveByMouse(NetworkMessage &msg);
  void parseMoveNorth(NetworkMessage &msg);
  void parseMoveEast(NetworkMessage &msg);
  void parseMoveSouth(NetworkMessage &msg);
  void parseMoveWest(NetworkMessage &msg);  
  void parseTurnNorth(NetworkMessage &msg);
  void parseTurnEast(NetworkMessage &msg);
  void parseTurnSouth(NetworkMessage &msg);
  void parseTurnWest(NetworkMessage &msg);
  void parseRequestOutfit(NetworkMessage &msg);
  void parseSetOutfit(NetworkMessage &msg);
  void parseSay(NetworkMessage &msg);
  void parseLookAt(NetworkMessage &msg);
  void parseAttack(NetworkMessage &msg);
  void parseFollow(NetworkMessage &msg);
  void parseThrow(NetworkMessage &msg);
  void parseUseItemEx(NetworkMessage &msg);
  void parseUseItem(NetworkMessage &msg);
  void parseCloseContainer(NetworkMessage &msg);
  void parseGetChannels(NetworkMessage &msg);
  void parseOpenChannel(NetworkMessage &msg);
  void parseOpenPriv(NetworkMessage &msg);
  void parseCloseChannel(NetworkMessage &msg);

  //Send Methods
  //void sendPlayerLookAt(std::string);
  void sendSetOutfit(const Creature* creature);
  virtual void sendChannels();
  virtual void sendChannel(unsigned short channelId);
  virtual void sendOpenPriv(std::string &receiver);
  virtual void sendToChannel(const Creature * creature, unsigned char type, const std::string &text, unsigned short channelId);
  virtual void sendNetworkMessage(NetworkMessage *msg);
  virtual void sendIcons(int icons);
  virtual void sendThingMove(const Creature *creature, const Thing *thing, const Position *oldPos, unsigned char oldstackpos);
  virtual void sendTeleport(const Creature *creature, const Thing *thing, const Position *oldPos, unsigned char oldstackpos, bool magiceffect);
  virtual void sendKick();
  virtual void sendLightLevel(const int level, int color);
  virtual void sendCreatureAppear(const Creature *creature);
  virtual void sendCreatureDisappear(const Creature *creature, unsigned char stackPos);
  virtual void sendCreatureTurn(const Creature *creature, unsigned char stackpos);
  virtual void sendCreatureSay(const Creature *creature, unsigned char type, const std::string &text);
  virtual void sendCancel(const char *msg);
  virtual void sendCancelWalk(const char *msg);
  virtual void sendChangeSpeed(const Creature* creature);
  virtual void sendCancelAttacking();
  virtual void sendCancelFollowing();
  virtual void sendTileUpdated(const Position *Pos);
  virtual void sendContainerUpdated(Item *item, unsigned char from_id, unsigned char to_id, unsigned char from_slot, unsigned char to_slot, bool remove);
  virtual bool CanSee(int x, int y);

  // Translate a map area to client readable format
  void GetMapDescription(unsigned short x, unsigned short y, unsigned char z,
                         unsigned short width, unsigned short height,
                         NetworkMessage &msg);
  void GetMapLight(unsigned short x, unsigned short y, unsigned char z,
                         unsigned short width, unsigned short height,
                         NetworkMessage &msg);                       

};

#endif // tprot70_h



