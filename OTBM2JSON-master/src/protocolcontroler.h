////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __PROTOCOLCONTROLER_H__
#define __PROTOCOLCONTROLER_H__

#include "map.h"
#include "player.h"
#include <string>

//Defines the controler

class NetworkMessage;

class ProtocolControler {
public:
  ProtocolControler(SOCKET s);
  virtual ~ProtocolControler();
  bool ConnectPlayer();
  void ReceiveLoop();

private:
  void parsePacket(NetworkMessage &msg);
  void parseLogout(NetworkMessage &msg);
  void parseEditHouseOwner(NetworkMessage &msg);
  void parseEditHouseName(NetworkMessage &msg);
  void parseEditHouseCity(NetworkMessage &msg);
  void parseEditGuild(NetworkMessage &msg);
  void parseEditGuildNickname(NetworkMessage &msg);
  void parseEditGuildRank(NetworkMessage &msg);
  void parseEnableGuild(NetworkMessage &msg);
  void parseEnableGuildNickname(NetworkMessage &msg);
  void parseKickPlayer(NetworkMessage &msg);
  void parseRespawnNpc(NetworkMessage &msg);
  //void parseSetPvT(NetworkMessage &msg);
  void parseSendPvT(NetworkMessage &msg);
  void parseBlockRegister();  
  void parseSetPvP();
  void parseSetSkill(NetworkMessage &msg);
  void parseSetNoPvP();
  void parseSetMaxPlayers(NetworkMessage &msg);
  virtual void sendNetworkMessage(NetworkMessage *msg);
  virtual void sendPlayersOnline();
  virtual void sendCharsOnline();
  virtual void sendHouses(NetworkMessage &msg);
  virtual void sendPing();
  virtual void sendGodMessage(const std::string &msgg);
  virtual void sendCharInfo(const std::string &msg);
  virtual void sendHouseInfo(const std::string &msg);
  
protected:
  Map *map;
  SOCKET s;     
};

#endif // __PROTOCOLCONTROLER_H__

