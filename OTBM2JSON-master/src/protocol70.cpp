////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

#include "definitions.h"
#include "protocol70.h"
#include "items.h"
#include "tile.h"
#include "creature.h"
#include "player.h"
#include "house.h"
#include "quests.h"
#include "networkmessage.h"
#include "luascript.h"
#include "tasks.h"

extern LuaScript g_config;
std::map<long, Creature*> channel;

Protocol70::Protocol70(SOCKET s)
{
  this->s = s;
}


Protocol70::~Protocol70()
{
                                                  
}


bool Protocol70::ConnectPlayer()
{
  return map->placeCreature(player);
}


void Protocol70::ReceiveLoop()
{
  NetworkMessage msg;

  while (msg.ReadFromSocket(s))
  {
    parsePacket(msg);
  }

  if (s)
    closesocket(s);

  // Disconnect? -> Kick
  if (player)
  {
			 map->removeCreature(player);
  }
}


void Protocol70::parsePacket(NetworkMessage &msg)
{
  uint8_t recvbyte = msg.GetByte();

  if (s && player->health <= 0) {
	 if (recvbyte == 0x14)
		parseLogout(msg);
	     return;
  }
  //printf("Recived: %x \n", recvbyte);
  switch(recvbyte)
  {
    case 0x14: // logout
      parseLogout(msg);
      break;

    case 0x64: // client moving with steps
  	  parseMoveByMouse(msg);
      break;

    case 0x65: // move north
      parseMoveNorth(msg);
      break;

    case 0x66: // move east
      parseMoveEast(msg);
      break;

    case 0x67: // move south
      parseMoveSouth(msg);
      break;

    case 0x68: // move west
      parseMoveWest(msg);
      break;

    case 0x6F: // turn north
      parseTurnNorth(msg);
      break;

    case 0x70: // turn east
      parseTurnEast(msg);
      break;

    case 0x71: // turn south
      parseTurnSouth(msg);
      break;

    case 0x72: // turn west
      parseTurnWest(msg);
      break;

    case 0x78: // throw item
      parseThrow(msg);
      break;
      
    case 0x82: // use item
      parseUseItem(msg);
      break;
      
    case 0x83: // use item
      parseUseItemEx(msg);
      break;
      
    case 0x87: // close container
      parseCloseContainer(msg);
      break;
      
    case 0x8C: // throw item
      parseLookAt(msg);
      break;
      
    case 0x96:  // say something
      parseSay(msg);
      break;
      
    case 0xA1: // attack
      parseAttack(msg);
      break;
      
    case 0xA2:
      parseFollow(msg); 
      break;
        
    case 0xD2: // request Outfit
      parseRequestOutfit(msg);
      break;
      
    case 0xD3: // set outfit
      parseSetOutfit(msg);
      break;
      
    case 0x97: // request Channels
      parseGetChannels(msg);
      break;
      
    case 0x98: // open Channel
      parseOpenChannel(msg);
      break;
       
    case 0x99: // close Channel
      //parseCloseChannel(msg);
      break;
      
    case 0x9A: // open priv
      parseOpenPriv(msg);
      break;
             
    case 0xBE: // cancel move
      parseCancelMove(msg);
      break;
      
    case 0xA0: // set attack and follow mode
      parseModes(msg);
      break;
      
    case 0x69: // client quit without logout 
      break;
      
    /*case 0x7d: // trade
      msg.GetByte() //<- u16 where is bp x
      msg.GetByte() //<- u16 where is bp x
      msg.GetByte() //<- u16 where is bp y
      msg.GetByte() //<- u16 where is bp y
      msg.GetByte() //bp pos z
      msg.GetByte() //item id
      msg.GetByte() //item id
      msg.GetByte() //bp pos z
      msg.GetByte() //creature id <- u16
      msg.GetByte() //71 <- u16
      msg.GetByte() //0 <- u16
      msg.GetByte() //0 <- u16
      break;  
    case 0x7d: // trade
      unsigned short get1 = msg.GetU16();
      unsigned short get2 = msg.GetU16();
      unsigned char get3 = msg.GetByte();
      unsigned short get4 = msg.GetU16();
      unsigned char get6 = msg.GetByte();
      unsigned short get7 = msg.GetU16();
      unsigned short get8 = msg.GetU16();
      printf("%hu  %hu  %u  %hu  %u  %hu  %hu", get1, get2, get3, get4, get6, get7, get8);
      NetworkMessage newmsg;
      newmsg.AddByte(0xd7);
      player->sendNetworkMessage(&newmsg);
      break;*/
           
    default:
         printf("unknown packet header: %x \n", recvbyte);
         parseDebug(msg);
         break;  
  }
}

void Protocol70::parseFollow(NetworkMessage &msg) 
{ 
     player->followedCreature = msg.GetU32(); 
     if(player->attackedCreature != 0){
       player->attackedCreature = 0;
     }
}

void Protocol70::GetMapDescription(unsigned short x, unsigned short y, unsigned char z,
                                   unsigned short width, unsigned short height,
                                   NetworkMessage &msg)
{
	Tile* tile;

  for (int nx = 0; nx < width; nx++)
    for (int ny = 0; ny < height; ny++)
    {
      tile = map->getTile(x + nx, y + ny, (unsigned char)z);

      if (tile)
      {
				msg.AddItem(&tile->ground);

        int count = 1;

        if (tile->splash)
        {
          msg.AddItem(tile->splash);
          count++;
        }

        ItemVector::iterator it;
		    for (it = tile->topItems.begin(); ((it !=tile->topItems.end()) && (count < 10)); it++)
        {
  			  msg.AddItem(*it);
          count++;
        }

        CreatureVector::iterator itc;
		    for (itc = tile->creatures.begin(); ((itc !=tile->creatures.end()) && (count < 10)); itc++)
        {
          bool known;
          unsigned long removedKnown;
          checkCreatureAsKnown((*itc)->getID(), known, removedKnown);
  			  msg.AddCreature(*itc, known, removedKnown);
          count++;
        }


		    for (it = tile->downItems.begin(); ((it !=tile->downItems.end()) && (count < 10)); it++)
        {
  			  msg.AddItem(*it);
          count++;
        }
      }
       // tile end
      if ((nx != width-1) || (ny != height-1))
      {
        msg.AddByte(0);
        msg.AddByte(0xFF);
      }
    }
}

void Protocol70::GetMapLight(unsigned short x, unsigned short y, unsigned char z,
                                   unsigned short width, unsigned short height,
                                   NetworkMessage &msg)
{
	Tile* tile;
    for (int nx = 0; nx < width; nx++)
    for (int ny = 0; ny < height; ny++)
    {
      tile = map->getTile(x + nx, y + ny, (unsigned char)z);

      if (tile)
      {
        CreatureVector::iterator itc;
		for (itc = tile->creatures.begin(); (itc !=tile->creatures.end()); itc++)
        {
            msg.AddLight(*itc);
        }
      }
    }
}

void Protocol70::checkCreatureAsKnown(unsigned long id, bool &known, unsigned long &removedKnown)
{
	// loop through the known player and check if the given player is in
  std::list<unsigned long>::iterator i;
  for(i = knownPlayers.begin(); i != knownPlayers.end(); ++i)
  {
	  if((*i) == id)
	  {
      // know... make the creature even more known...
		  knownPlayers.erase(i);
		  knownPlayers.push_back(id);

		  known = true;
      return;
	  }
  }

  // ok, he is unknown...
  known = false;
	
  // ... but not in future
  knownPlayers.push_back(id);

	// to many known players?
  if(knownPlayers.size() > 64)
  {
    // lets try to remove one from the end of the list
    for (int n = 0; n < 64; n++)
    {
      removedKnown = knownPlayers.front();

      Creature *c = map->getCreatureByID(removedKnown);
      if ((!c) || (!CanSee(c->pos.x, c->pos.y)))
        break;

      // this creature we can't remove, still in sight, so back to the end
      knownPlayers.pop_front();
      knownPlayers.push_back(removedKnown);
    }

    // hopefully we found someone to remove :S, we got only 64 tries
    // if not... lets kick some players with debug errors :)
	  knownPlayers.pop_front();
  }
  else
  {
    // we can cache without problems :)
    removedKnown = 0;
  }
}


// Parse methods
void Protocol70::parseLogout(NetworkMessage &msg)
{
    if(player->inFightTicks >=1000 && player-> health >0) {
         sendCancel("You may not logout during or immediately after a fight!");
         return;
     }    
	// we ask the map to remove us
	if (map->removeCreature(player))
	{
		player = NULL;
		closesocket(s);
		s = 0;
	}
}

void Protocol70::parseGetChannels(NetworkMessage &msg){
     sendChannels();
}

void Protocol70::parseOpenChannel(NetworkMessage &msg){
     unsigned short channelId = msg.GetU16();
     sendChannel(channelId);
     std::map<long, Creature*>::iterator sit = channel.find(player->getID());
     if( sit == channel.end() ) {
          channel[player->getID()] = player;
         }
}

void Protocol70::parseCloseChannel(NetworkMessage &msg){
     unsigned short channelId = msg.GetU16();
     std::map<long, Creature*>::iterator sit = channel.find(player->getID());
     if( sit != channel.end() ) {
          channel.erase(sit);
         }
}

void Protocol70::parseOpenPriv(NetworkMessage &msg){
     std::string receiver; 
     receiver = msg.GetString();
     Creature* c = map->getCreatureByName(receiver.c_str());
     Player* player = dynamic_cast<Player*>(c);
     if(player) 
     sendOpenPriv(receiver);
     }

void Protocol70::sendOpenPriv(std::string &receiver){
     NetworkMessage newmsg; 
     newmsg.AddByte(0xAD); 
     newmsg.AddString(receiver); 
     newmsg.WriteToSocket(s);
}     

void Protocol70::parseCancelMove(NetworkMessage &msg)
{
     player->cancelMove = true;
}

void Protocol70::parseModes(NetworkMessage &msg)
{
     player->fightMode = msg.GetByte();
     player->followMode = msg.GetByte();
}

void Protocol70::parseDebug(NetworkMessage &msg)
{
  int dataLength = msg.getMessageLength()-3;
  if(dataLength!=0){
    printf("data: ");
    size_t data = msg.GetByte();
    while( dataLength > 0){
      printf("%d ", data);
      if(--dataLength >0)
        data = msg.GetByte();
    }
    printf("\n");
  }
}

void Protocol70::parseMoveByMouse(NetworkMessage &msg)
{
  // first we get all directions...
  std::list<Direction> path;
  size_t numdirs = msg.GetByte();
  for (size_t i = 0; i < numdirs; ++i) {
          path.push_back((Direction)msg.GetByte());
  }
  
  // then we schedule the movement...
  // the interval seems to depend on the speed of the char?
  if(player->cancelMove) {
                              player->cancelMove = false;
                              sendCancelWalk("");                 
                         }
  else{     
  map->addEvent(makeTask(0, MovePlayer(player->getID()), path, player->getWalkDuration(), StopMovePlayer(player->getID())));
 }
}


void Protocol70::parseMoveNorth(NetworkMessage &msg)
{
      if(player->followedCreature != 0){
       player->followedCreature = 0;
      } 
	  this->sleepTillMove();
      map->thingMove(player, player, player->pos.x, player->pos.y-1, player->pos.z);           
}


void Protocol70::parseMoveEast(NetworkMessage &msg)
{
    if(player->followedCreature != 0){
       player->followedCreature = 0;
    }  
	this->sleepTillMove();
  map->thingMove(player, player,
                 player->pos.x+1, player->pos.y, player->pos.z);
}


void Protocol70::parseMoveSouth(NetworkMessage &msg)
{
    if(player->followedCreature != 0){
       player->followedCreature = 0;
    }  
	this->sleepTillMove();
  map->thingMove(player, player,
                 player->pos.x, player->pos.y+1, player->pos.z);
}


void Protocol70::parseMoveWest(NetworkMessage &msg)
{
    if(player->followedCreature != 0){
       player->followedCreature = 0;
    }  
	this->sleepTillMove();
  map->thingMove(player, player,
                 player->pos.x-1, player->pos.y, player->pos.z);
}


void Protocol70::parseTurnNorth(NetworkMessage &msg)
{
	map->creatureTurn(player, NORTH);
}


void Protocol70::parseTurnEast(NetworkMessage &msg)
{
  map->creatureTurn(player, EAST);
  NetworkMessage newmsg;
  newmsg.WriteToSocket(s);
}


void Protocol70::parseTurnSouth(NetworkMessage &msg)
{
  map->creatureTurn(player, SOUTH);
}


void Protocol70::parseTurnWest(NetworkMessage &msg)
{
  map->creatureTurn(player, WEST);
  
}


void Protocol70::parseRequestOutfit(NetworkMessage &msg)
{
  msg.Reset();

  msg.AddByte(0xC8);
  msg.AddByte(player->looktype);
  msg.AddByte(player->lookhead);
  msg.AddByte(player->lookbody);
  msg.AddByte(player->looklegs);
  msg.AddByte(player->lookfeet);
  switch (player->sex) {
	  case 0:
		  msg.AddByte(PLAYER_FEMALE_1);
		  msg.AddByte(PLAYER_FEMALE_4);
		  break;
	  case 1:
		  msg.AddByte(PLAYER_MALE_1);
		  msg.AddByte(PLAYER_MALE_4);
		  break;
	  case 2:
		  msg.AddByte(160);
		  msg.AddByte(160);
		  break;
	  default:
		  msg.AddByte(PLAYER_MALE_1);
		  msg.AddByte(PLAYER_MALE_4);
  }


  msg.WriteToSocket(s);
}


void Protocol70::sendSetOutfit(const Creature* creature) {
if (CanSee(creature->pos.x, creature->pos.y)) {
	NetworkMessage newmsg;
	newmsg.AddByte(0x8E);
	newmsg.AddU32(creature->getID());
	newmsg.AddByte(creature->looktype);
	newmsg.AddByte(creature->lookhead);
	newmsg.AddByte(creature->lookbody);
	newmsg.AddByte(creature->looklegs);
	newmsg.AddByte(creature->lookfeet);
	newmsg.WriteToSocket(s);
}
}

void Protocol70::sendChannels(){
     NetworkMessage newmsg;
	 newmsg.AddByte(0xAB);
	 
	 newmsg.AddByte(3); //how many
	 
	 newmsg.AddByte(0xFF); //priv chan
	 newmsg.AddByte(0xFF); //priv chan
	 newmsg.AddString("Private Chat Channel");
	 
	 newmsg.AddByte(0x00); //clan chan
	 newmsg.AddByte(0x00); //clan chan
	 newmsg.AddString("Trade");
	 
	 newmsg.AddByte(0x04);
	 newmsg.AddByte(0x00);
	 newmsg.AddString("Game-Chat");
	 newmsg.WriteToSocket(s);
	 
}

void Protocol70::sendChannel(unsigned short channelId){
     NetworkMessage newmsg;
     if(channelId == 4){
	 newmsg.AddByte(0xAC);
	 newmsg.AddU16(channelId);	 
	 newmsg.AddString("Game-Chat");	 
	 }if(channelId == 0){
	 newmsg.AddByte(0xAC);
	 newmsg.AddU16(channelId);	 
	 newmsg.AddString("Trade");	 
	 }
     newmsg.WriteToSocket(s);
	 
}

void Protocol70::sendIcons(int icons){
     NetworkMessage newmsg;
	 newmsg.AddByte(0xA2);
	 newmsg.AddByte(icons);
	 newmsg.WriteToSocket(s);
     }

void Protocol70::parseSetOutfit(NetworkMessage &msg)
{
    int temp = msg.GetByte();
    if ( (player->sex == 0 && temp >= 136 && temp <= 139) || (player->sex == 1 && temp >= 128 && temp <= 131)){ 
	player->looktype= temp;
	player->lookmaster = player->looktype;
	player->lookhead=msg.GetByte();
	player->lookbody=msg.GetByte();
	player->looklegs=msg.GetByte();
	player->lookfeet=msg.GetByte();

	if (player->sex > 1) {
		std::cout << "set outfit to: " << (int)(player->lookhead) << " / " << (int)player->lookbody << " / " << (int)player->looklegs << " / " <<  (int)player->lookfeet << std::endl;
	}
	map->creatureChangeOutfit(player);
    }
}


void Protocol70::parseUseItemEx(NetworkMessage &msg)
{
	unsigned short from_x = msg.GetU16();
	unsigned short from_y = msg.GetU16();
	unsigned char from_z = msg.GetByte();
	unsigned short item = msg.GetU16();
	unsigned char newpos = msg.GetByte();
	unsigned short to_x = msg.GetU16();
	unsigned short to_y = msg.GetU16();
	unsigned char to_z = msg.GetByte();
	unsigned short tile_id = msg.GetU16();
	unsigned char count = msg.GetByte();

	Position pos;
	pos.x = to_x;
	pos.y = to_y;
	pos.z = to_z;
    		
    int base = (player->level*2)+(player->maglevel*3);
    
    //pick
	if(item == 1810 && (player->pos.x == to_x-1 || player->pos.x == to_x+1 || player->pos.x == to_x) && (player->pos.y == to_y-1 || player->pos.y == to_y+1 || player->pos.x == to_y) && !(player->pos.x == to_x && player->pos.x == to_y)) {
				Tile *tile = map->getTile(to_x, to_y, to_z);
				if(tile){
				Position TilePos;

				TilePos.x = to_x;
				TilePos.y = to_y;
				TilePos.z = to_z;

				int groundid = tile->ground.getID();

				if (groundid == 371)
				{
						tile->ground.setID(385);
						map->creatureBroadcastTileUpdated(TilePos);
						map->CallCloseHole(TilePos);
				}
                }
	}

	//shovel
	if(item == 1811 && (player->pos.x == to_x-1 || player->pos.x == to_x+1 || player->pos.x == to_x) && (player->pos.y == to_y-1 || player->pos.y == to_y+1 || player->pos.y == to_y) && !(player->pos.x == to_x && player->pos.y == to_y)) {
				Tile *tile = map->getTile(to_x, to_y, to_z);
				if(tile){
                Position TilePos;

				TilePos.x = to_x;
				TilePos.y = to_y;
				TilePos.z = to_z;

				int groundid = tile->ground.getID();

				if (groundid == 454)
				{
						tile->ground.setID(455);
						map->creatureBroadcastTileUpdated(TilePos);
						map->CallCloseHole(TilePos);
				}else if (groundid == 469){
						tile->ground.setID(470);
						map->creatureBroadcastTileUpdated(TilePos);
						map->CallCloseHole(TilePos);
				}else if (groundid == 467){
						tile->ground.setID(468);
						map->creatureBroadcastTileUpdated(TilePos);
						map->CallCloseHole(TilePos);
				}else if (groundid == 385){
						tile->ground.setID(468);
						map->creatureBroadcastTileUpdated(TilePos);
						map->CallCloseHole(TilePos);
				}
            }
	}
	//rope
	if(item == 1497 && (player->pos.x == to_x-1 || player->pos.x == to_x+1 || player->pos.x == to_x) && (player->pos.y == to_y-1 || player->pos.y == to_y+1 || player->pos.y == to_y) && !(player->pos.x == to_x && player->pos.y == to_y)) {
				Tile *tile = map->getTile(to_x, to_y, to_z);
				
				if(tile && tile->isRopeTele()){
                   map->teleportPlayer(tile->teleportPos, player, tile->magiceffect, tile, true);
                } 
	}
	
	if(from_x == 0xFFFF) {
        bool decreaseCharge = false;      
		if(from_y & 0x40) {
			 unsigned char containerid = from_y & 0x0F;
  	                         Item* container = player->getContainer(containerid);
  	                         if(!container)
  	                                 return;
  	 
  	                         Item* runeitem = container->getItem(from_z);
  	                         if(!runeitem)
  	                                 return;
                                       
                             if(runeitem->getItemCharge() < 1)
                                     return;
                                                
			if(item == 1623) { //Uh
				MagicEffectRuneClass runeSpell;
				runeSpell.animationEffect = 0;
				runeSpell.damageEffect = NM_ME_MAGIC_ENERGIE;
				runeSpell.animationcolor = 71; 
				runeSpell.offensive = false;
				runeSpell.physical = false;
				runeSpell.centerpos = pos;
				runeSpell.minDamage = (int)(base * 2.5);
				runeSpell.maxDamage = (int)(base * 2.8);
				runeSpell.type = 4;
				if(player->maglevel >= 4)
				  decreaseCharge = map->creatureThrowRune(player, runeSpell);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");  
			}
		else
			if(item == 1654) { //Gfb
				static unsigned char area[14][18] = {
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
				};

				MagicEffectAreaClass runeAreaSpell;
				runeAreaSpell.animationEffect = NM_ANI_FIRE;
				runeAreaSpell.damageEffect = NM_ME_FIRE_AREA;
				runeAreaSpell.areaEffect = NM_ME_FIRE_AREA;
				runeAreaSpell.animationcolor = 193;
				runeAreaSpell.offensive = true;
				runeAreaSpell.physical = false;
				runeAreaSpell.centerpos = pos;
				memcpy(&runeAreaSpell.area, area, sizeof(area));
				runeAreaSpell.direction = 1;
				runeAreaSpell.minDamage = (int)(base * 0.55);
				runeAreaSpell.maxDamage = (int)(base * 0.65);
				runeAreaSpell.type = 1;
				if(player->maglevel >= 4)
				  decreaseCharge = map->creatureThrowRune(player, runeAreaSpell);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");
			}
		else
			if(item == 1663) { //Explosion
				static unsigned char area[14][18] = {
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
				};

				MagicEffectAreaClass runeAreaSpell;
				runeAreaSpell.animationEffect = NM_ANI_FIRE;
				runeAreaSpell.damageEffect = NM_ME_EXPLOSION_DAMAGE;
				runeAreaSpell.areaEffect = NM_ME_EXPLOSION_AREA;
				runeAreaSpell.animationcolor = 0xB4;
				runeAreaSpell.offensive = true;
				runeAreaSpell.physical = true;
				runeAreaSpell.centerpos = pos;
				memcpy(&runeAreaSpell.area, area, sizeof(area));
				runeAreaSpell.direction = 1;
				runeAreaSpell.minDamage = 0;
				runeAreaSpell.maxDamage = (int)(base * 1);
				runeAreaSpell.type = 4;
				if(player->maglevel >= 5)
				  decreaseCharge = map->creatureThrowRune(player, runeAreaSpell);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");
			}
		else
			if(item == 0x0652) { //Sd
				MagicEffectRuneClass runeSpell;
				runeSpell.animationEffect = NM_ANI_SUDDENDEATH;
				runeSpell.damageEffect = NM_ME_MORT_AREA;
				runeSpell.animationcolor = 0xB4; 
				runeSpell.offensive = true;
				runeSpell.physical = true;
				runeSpell.centerpos = pos;
				runeSpell.minDamage = (int)(base * 1.35 - 20);
				runeSpell.maxDamage = (int)(base * 1.6);
				runeSpell.type = 4;
				if(player->maglevel >= 15)
				  decreaseCharge = map->creatureThrowRune(player, runeSpell);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");
			}
			else if(item == 1661) { //Hmm
				MagicEffectRuneClass runeSpell;
				runeSpell.animationEffect = NM_ANI_FIRE;
				runeSpell.damageEffect = NM_ME_ENERGY_DAMAGE;
				runeSpell.animationcolor = 71; 
				runeSpell.offensive = true;
				runeSpell.physical = false;
				runeSpell.centerpos = pos;
				runeSpell.minDamage = (int)(base * 0.3);
				runeSpell.maxDamage = (int)(base * 0.5);
				runeSpell.type = 0;
				if(player->maglevel >= 1)
				  decreaseCharge = map->creatureThrowRune(player, runeSpell);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");
			}
			else if(item == 1643) { //M Wall
				static unsigned char area[14][18] = {
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

				MagicEffectGroundClass spellGround;
				spellGround.animationEffect = NM_ANI_ENERGY;
				spellGround.offensive = true;
				spellGround.physical = false;
				spellGround.centerpos = pos;
				memcpy(&spellGround.area, area, sizeof(area));
				spellGround.direction = 1;
				spellGround.groundID = 1190;
				spellGround.type = 4;
				if(player->maglevel >= 4)
				  decreaseCharge = map->creatureThrowRune(player, spellGround);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");
                 
			}else if(item == 1628) { //Mf
				
				MagicEffectRuneClass runeSpell;
				runeSpell.animationEffect = NM_ANI_FIRE;
				runeSpell.damageEffect = NM_ME_MAGIC_ENERGIE;
				runeSpell.animationcolor = 71; 
				runeSpell.offensive = false;
				runeSpell.physical = false;
				runeSpell.centerpos = pos;
				runeSpell.type = 4;
				if(player->maglevel >= 4)
				  decreaseCharge = map->creatureThrowRune(player, runeSpell);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");
				
				
			}else if(item == 1655) { //Fire Bomb
				static unsigned char area[14][18] = {
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

				MagicEffectGroundClass spellGround;
				spellGround.animationEffect = NM_ANI_FIRE;
				spellGround.animationcolor = 193;
				spellGround.damageEffect = NM_ME_HITBY_FIRE;
				spellGround.areaEffect = NM_ME_FIRE_AREA;
				spellGround.offensive = true;
				spellGround.physical = false;
				spellGround.centerpos = pos;
				memcpy(&spellGround.area, area, sizeof(area));
				spellGround.direction = 1;
				spellGround.groundID = 1185;
				spellGround.minDamage = 20;
				spellGround.maxDamage = 20;
                spellGround.type = 1;
                if(player->maglevel >= 4)
				  decreaseCharge = map->creatureThrowRune(player, spellGround);
                else
                  player->sendCancel("You dont have enought magic level to use this spell.");
			}

			if(decreaseCharge) {
				 runeitem->setItemCharge(((int)runeitem->getItemCharge()) - 1 < 0 ? 0 : runeitem->getItemCharge() - 1);

				if(runeitem->getItemCharge() == 0) {
					container->removeItem(runeitem);
					map->needContainerUpdate(container, player, runeitem, from_z, 0xFF, true, false);
				}	
            }	
		}
	}
}

void Protocol70::parseUseItem(NetworkMessage &msg)
{
	unsigned short x = msg.GetU16();
	unsigned short y = msg.GetU16();
	unsigned char z = msg.GetByte();
	unsigned short item = msg.GetU16();
	unsigned char un = msg.GetByte();
	unsigned char stack = msg.GetByte();
#ifdef __DEBUG__
	std::cout << "parseUseItem: " << "x: " << x << ", y: " << (int)y <<  ", z: " << (int)z << ", item: " << (int)item << ", un: " << (int)un << ", stack: " << (int)stack << std::endl;
#endif
//open door
if (item && (item == 1018 || item == 1020 || item == 1019 || item == 1021 || item == 1008 || item == 1010 || item == 1011 || item == 1013) && (player->pos.x == x-1 || player->pos.x == x+1 || player->pos.x == x) && (player->pos.y == y-1 || player->pos.y == y+1 || player->pos.y == y))
	{
		Tile *tile = map->getTile(x, y, z);
		ItemVector::iterator iit;
		Item *doorfound;
		Position oldpos;

		for (iit = tile->topItems.begin(); iit != tile->topItems.end(); iit++)
		{ 
			if ((*iit)->getID() == 1018 || (*iit)->getID() == 1020 || (*iit)->getID() == 1021 || (*iit)->getID() == 1019 || (*iit)->getID() == 1008 || (*iit)->getID() == 1010 || (*iit)->getID() == 1011 || (*iit)->getID() == 1013)
			{
				doorfound = (*iit);
			}
		}

		oldpos.x = x;
		oldpos.y = y;
		oldpos.z = 7;
		
		if (doorfound && doorfound->house)
        {
            House *house = doorfound->house;        	
              if(house->isDoorName(tile->pos, player->getName())){
			     if (item == 1018)
			     {
				    doorfound->setID(1019);
			     } 
	
			     if (item == 1019)
			     {
				    doorfound->setID(1018);
			     } 
	
			     if (item == 1020)
			     {
				    doorfound->setID(1021);
			     } 
			
			     if (item == 1021)
			     {
				    doorfound->setID(1020);
			     }
		      }
        }
		else if (doorfound && (item == 1018 || item == 1019 || item == 1020 || item == 1021) && (player->access == 3 || (tile->isGateLvl() && player->level >= tile->gatelvl)))
		{	
			if (item == 1018)
			{
				doorfound->setID(1019);
			} 
	
			if (item == 1019)
			{
				doorfound->setID(1018);
			} 
	
			if (item == 1020)
			{
				doorfound->setID(1021);
			} 
			
			if (item == 1021)
			{
				doorfound->setID(1020);
			} 
		}
        else if((item == 1018 || item == 1019 || item == 1020 || item == 1021) && tile->isGateLvl() && player->level < tile->gatelvl)
        {
		    std::stringstream reqgatee;
            reqgatee << "Only people on level " << tile->gatelvl << " or more can open and close the door.";
            player->sendCancel(reqgatee.str().c_str());
		}
        else if(item == 1018 || item == 1019 || item == 1020 || item == 1021)
        {
		    player->sendCancel("Only gamemasters can open and close the door."); 
        } 
		else if (item == 1008 || item == 1010 || item == 1011 || item == 1013)
		{
			if (item == 1008)
			{
				doorfound->setID(1010);
			} 
		
			if (item == 1010)
			{
				doorfound->setID(1008);
			} 
		
			if (item == 1011)
			{
				doorfound->setID(1013);
			} 
		
			if (item == 1013)
			{
				doorfound->setID(1011);
			} 
        }
        			
		map->creatureBroadcastTileUpdated(oldpos);
    }
//Use Switch    
if (item && (item == 1369 || item == 1370) && !(abs(player->pos.x - x) > 1 || abs(player->pos.y - y) > 1)){
		Tile *tile = map->getTile(x, y, z);
		Item *switchfound;

		for (ItemVector::iterator iit = tile->downItems.begin(); iit != tile->downItems.end(); iit++){ 
			if ((*iit)->getID() == 1369 || (*iit)->getID() == 1370){
				switchfound = (*iit);
			}
		}
		int leverSide = -1;
		if(switchfound->getID() == 1370) leverSide = 1;
        map->quests->questScript->onCreaturePullLever(player->getID(), Position(x,y,z), leverSide);
        /*Position oldpos;
		oldpos.x = x;
		oldpos.y = y;
		oldpos.z = 7;
	
		if (item == 1369 || item == 1370){
			tile->removeThing(switchfound);

			if (item == 1369){
				tile->addThing(new Item(1370));
			} 
		
			if (item == 1370){
				tile->addThing(new Item(1369));
			}
        }			
		map->creatureBroadcastTileUpdated(oldpos);*/
    }
//Use Stair
if (item && item == 1124 && !(abs(player->pos.x - x) > 1 || abs(player->pos.y - y) > 1)){
				Tile *tile = map->getTile(x, y, z);
                if(tile->isUseTele()){
                   map->teleportPlayer(tile->teleportPos, player, tile->magiceffect, tile, true);
                }
}

if (item && item == 421 && !(abs(player->pos.x - x) > 1 || abs(player->pos.y - y) > 1)){
				Tile *tile = map->getTile(x, y, z);
                if(tile->isUseTele()){
                   map->teleportPlayer(tile->teleportPos, player, tile->magiceffect, tile, true);
                }
}                              
           
	if(Item(item).isContainer())
  	{
		msg.Reset();
		Item *newcontainer = NULL;
                               
  	                 if(x != 0xFFFF) {
  	                         if(abs(player->pos.x - x) > 1 || abs(player->pos.y - y) > 1)
  	                                 return;
  	                                 
  	                         Tile *t = map->getTile(x, y, z);
  	                         
  	                         //Check if container is of a quest
                             if(t){
                                Item* tmpcontainer = NULL;
                                tmpcontainer = (Item*)t->getThingByStackPos(un);
		                        if(tmpcontainer && tmpcontainer->isContainerQuest()){
		                           Quest* quest = map->quests->getQuestByName(tmpcontainer->getQuestName());
		                           quest->playerDoQuest(player, Position(x,y,z));
                                }else
  	                               newcontainer = tmpcontainer;
                             }
  	                 }            
  	 
  	                 if(x == 0xFFFF) {
  	                         Item *parentcontainer = NULL;
  	                                          
		if(0x40 & y) {
			unsigned char containerid = y & 0x0F;
			 parentcontainer = player->getContainer(containerid);

			if(parentcontainer == NULL || !parentcontainer->isContainer()) {
				return;
			}

			int n = 0;
			for (std::list<Item *>::const_iterator cit = parentcontainer->getItems(); cit != parentcontainer->getEnd(); cit++) {
				if(n == z) {
					newcontainer = (*cit);
					break;
				}
				else
					n++;
			}
		}
		else
			newcontainer = (Item*)player->items[y];
      }
		if(newcontainer && newcontainer->isContainer() && stack < 16) {  
			player->addContainer(stack, newcontainer);

			msg.AddByte(0x6E);
			msg.AddByte(stack);

			msg.AddU16(newcontainer->getID());
			msg.AddString(newcontainer->getName());
			msg.AddU16(newcontainer->getContainerMaxItemCount());
			msg.AddByte(newcontainer->getContainerItemCount());

            std::list<Item *>::const_iterator cit;
			for (cit = newcontainer->getItems(); cit != newcontainer->getEnd(); cit++) {
				msg.AddU16((*cit)->getID());
				if((*cit)->isStackable() || (*cit)->isMultiType())
					msg.AddByte((*cit)->getItemCountOrSubtype());
			}
	
			msg.WriteToSocket(s);
		}
	}
}

void Protocol70::parseCloseContainer(NetworkMessage &msg)
{
	unsigned char containerid = msg.GetByte();
	player->closeContainer(containerid);

	msg.Reset();

	msg.AddByte(0x6F);
	msg.AddByte(containerid);

	msg.WriteToSocket(s);
}

void Protocol70::parseThrow(NetworkMessage &msg)
{
  unsigned short from_x     = msg.GetU16();
  unsigned short from_y     = msg.GetU16(); 
  unsigned char  from_z     = msg.GetByte();
  unsigned char  duno1 = msg.GetByte();
  unsigned char  duno2 = msg.GetByte();
  unsigned char  from_stack = msg.GetByte();
  unsigned short to_x       = msg.GetU16();
  unsigned short to_y       = msg.GetU16(); 
  unsigned char  to_z       = msg.GetByte();
  unsigned char  count      = msg.GetByte();
  
  map->thingMove(player, from_x, from_y, from_z, from_stack, to_x, to_y, to_z, count);
}

void Protocol70::parseLookAt(NetworkMessage &msg){
  Position LookPos = msg.GetPosition();
  unsigned short ItemNum = msg.GetU16();

#ifdef __DEBUG__
  std::cout << "Look at: " << LookPos << std::endl;
  std::cout << "Itemnum: " << ItemNum << std::endl;
#endif

  NetworkMessage newmsg;
  std::stringstream ss;

/*#ifdef __DEBUG__
	ss << "You look at " << LookPos << " and see Item # " << ItemNum << ".";
  Position middle;
  newmsg.AddTextMessage(MSG_INFO, ss.str().c_str());
#else*/
  if(ItemNum == 99) //Creature
  {
         Tile* tile = map->getTile(LookPos.x, LookPos.y, LookPos.z);
         Creature* creature = NULL;
         creature = (*tile->creatures.begin());
         if(creature && creature->access >= 0 && creature->access <= 5){
           if(player == creature)
              newmsg.AddTextMessage(MSG_INFO, creature->getDescription(true).c_str());
           else
              newmsg.AddTextMessage(MSG_INFO, creature->getDescription().c_str());
         }
             
  }
  else{
    if(LookPos.x != 0xFFFF){
      Tile* tile = map->getTile(LookPos.x, LookPos.y, LookPos.z);
      Thing *thing = tile->getThingByStackPos(1);
      Item* item = dynamic_cast<Item*>(thing);
      if(item)
          newmsg.AddTextMessage(MSG_INFO, item->getDescription().c_str());
      else       
          newmsg.AddTextMessage(MSG_INFO, Item(ItemNum).getDescription().c_str());
    }else if(LookPos.x == 0xFFFF){
      Item* fromContainer = NULL;    
      if(0x40 & LookPos.y){    
          unsigned char from_id = LookPos.y & 0x0F;
          fromContainer = player->getContainer(from_id);
	      if(!fromContainer || !fromContainer->isContainer())
		      newmsg.AddTextMessage(MSG_INFO, Item(ItemNum).getDescription().c_str());
	      else{		
	          Thing* thing = fromContainer->getItem(LookPos.z);
              Item* item = dynamic_cast<Item*>(thing);
              if(item)
                 newmsg.AddTextMessage(MSG_INFO, item->getDescription().c_str());
              else
                 newmsg.AddTextMessage(MSG_INFO, Item(ItemNum).getDescription().c_str());   
          }       
      }else{
		  Thing* thing = player->items[LookPos.y];
		  fromContainer = player->items[LookPos.y];
		  if(!fromContainer || !thing)
              newmsg.AddTextMessage(MSG_INFO, Item(ItemNum).getDescription().c_str());
          else{
              Item* item = dynamic_cast<Item*>(thing);
              if(item)
                 newmsg.AddTextMessage(MSG_INFO, item->getDescription().c_str());
              else
                 newmsg.AddTextMessage(MSG_INFO, Item(ItemNum).getDescription().c_str());   
          }    
      }
    }          	                   
  }  
//#endif
  
  sendNetworkMessage(&newmsg);
}

void Protocol70::parseSay(NetworkMessage &msg)
{
  unsigned char type = msg.GetByte();
  
  std::string receiver;
  unsigned short channelId;
  if (type == 4)
    receiver = msg.GetString();
  if (type == 5)
    channelId = msg.GetU16();
  std::string text = msg.GetString();
  map->creatureSaySpell(player, text);

  switch (type)
  {
    case 0x01:
      map->creatureSay(player, type, text);
      break;
    case 0x02:
      map->creatureWhisper(player, text);
      break;
    case 0x03:
      map->creatureYell(player, text);
      break;
    case 0x04:
      map->creatureSpeakTo(player, receiver, text);
      break;
    case 0x05:
      map->creatureToChannel(player, type, text, channelId);
      break;
    case 0x09:
      map->creatureBroadcastMessage(player, text);
      break;
  }
}

void Protocol70::parseAttack(NetworkMessage &msg)
{
  unsigned long playerid = msg.GetU32();
  player->setAttackedCreature(playerid);
  if(player->attackedCreature == 0)
     player->followedCreature = 0;
}

bool Protocol70::CanSee(int x, int y)
{
  if ((x >= player->pos.x - 8) && (x <= player->pos.x + 9) &&
      (y >= player->pos.y - 6) && (y <= player->pos.y + 7))
    return true;

  return false;
}


void Protocol70::sendNetworkMessage(NetworkMessage *msg)
{
  msg->WriteToSocket(s);
}


void Protocol70::sendTileUpdated(const Position *Pos)
{
  if (CanSee(Pos->x, Pos->y))
  {
	  NetworkMessage msg;
	  msg.AddByte(0x69);
	  msg.AddPosition(*Pos);
	  GetMapDescription(Pos->x, Pos->y, Pos->z, 1, 1, msg);
      msg.AddByte(0);
      msg.AddByte(0xFF);
      GetMapLight(Pos->x, Pos->y, Pos->z, 1, 1, msg);
	  msg.WriteToSocket(s);
  }
}

void Protocol70::sendContainerUpdated(Item *item, unsigned char from_id, unsigned char to_id,
																			unsigned char from_slot, unsigned char to_slot, bool remove)
{
	NetworkMessage msg;
	if(from_id == to_id) {
		//Remove item
		msg.AddByte(0x72);
		msg.AddByte(from_id);
		msg.AddByte(from_slot);

		//Add item
		msg.AddByte(0x70);
		msg.AddByte(to_id);
		msg.AddU16(item->getID());
		if(item->isStackable() || item->isMultiType())
		  msg.AddByte(item->getItemCountOrSubtype());
	}
	else if(remove) {
		//Remove item
		msg.AddByte(0x72);
		msg.AddByte(from_id);
		msg.AddByte(from_slot);
	}
	else
	{
		//Add item
		msg.AddByte(0x70);
		msg.AddByte(to_id);
		msg.AddU16(item->getID());
		if(item->isStackable() || item->isMultiType())
		  msg.AddByte(item->getItemCountOrSubtype());
	}

	msg.WriteToSocket(s);
}

void Protocol70::sendThingMove(const Creature *creature, const Thing *thing, const Position *oldPos, unsigned char oldStackPos)
{
  NetworkMessage msg;
  
	const Creature* c = dynamic_cast<const Creature*>(thing);
  if (c && (CanSee(oldPos->x, oldPos->y)) && (CanSee(thing->pos.x, thing->pos.y)))
  {
    msg.AddByte(0x6D);
    msg.AddPosition(*oldPos);
    msg.AddByte(oldStackPos);
    msg.AddPosition(thing->pos);
  }
  else
  {
    if (CanSee(oldPos->x, oldPos->y))
    {
      msg.AddByte(0x6C);
      msg.AddPosition(*oldPos);
      msg.AddByte(oldStackPos);
    }

    if (CanSee(thing->pos.x, thing->pos.y))
    {
      msg.AddByte(0x6A);
      msg.AddPosition(thing->pos);
      if (c) {
        bool known;
        unsigned long removedKnown;
        checkCreatureAsKnown(((Creature*)thing)->getID(), known, removedKnown);
  			msg.AddCreature((Creature*)thing, known, removedKnown);
  			msg.AddLight(creature);
      }
      else
        msg.AddItem((Item*)thing);
    }
  }
	
  if (thing == this->player)
  {
		if (oldPos->y > thing->pos.y)        // north, for old x
    {
      msg.AddByte(0x65);
      GetMapDescription(oldPos->x - 8, thing->pos.y - 6, thing->pos.z, 18, 1, msg);
      msg.AddByte(0x7E);
      msg.AddByte(0xFF);
      GetMapLight(oldPos->x - 8, thing->pos.y - 6, thing->pos.z, 18, 1, msg);
    }
    else if (oldPos->y < thing->pos.y)   // south, for old x
    {
      msg.AddByte(0x67);
      GetMapDescription(oldPos->x - 8, thing->pos.y + 7, thing->pos.z, 18, 1, msg);
      msg.AddByte(0x7E);
      msg.AddByte(0xFF);
      GetMapLight(oldPos->x - 8, thing->pos.y + 7, thing->pos.z, 18, 1, msg);
    }

    if (oldPos->x < thing->pos.x)        // east, [with new y]
    {
      msg.AddByte(0x66);
      GetMapDescription(thing->pos.x + 9, thing->pos.y - 6, thing->pos.z, 1, 14, msg);
      msg.AddByte(0x62);
      msg.AddByte(0xFF);
      GetMapLight(thing->pos.x + 9, thing->pos.y - 6, thing->pos.z, 1, 14, msg);
    }
    else if (oldPos->x > thing->pos.x)   // west, [with new y]
    {
      msg.AddByte(0x68);
      GetMapDescription(thing->pos.x - 8, thing->pos.y - 6, thing->pos.z, 1, 14, msg);
      msg.AddByte(0x62);
      msg.AddByte(0xFF);
      GetMapLight(thing->pos.x - 8, thing->pos.y - 6, thing->pos.z, 1, 14, msg);
    }
  }

  msg.WriteToSocket(s);
}

void Protocol70::sendKick()
{
  closesocket(s);
  map->removeCreature(player);
}

void Protocol70::sendTeleport(const Creature *creature, const Thing *thing, const Position *oldPos, unsigned char oldStackPos, bool magiceffect) 
{ 
  NetworkMessage msg;
  //Message for players that can see oldPos and the pos 
  if (!(creature == player) && creature && (CanSee(oldPos->x, oldPos->y)) && (CanSee(creature->pos.x, creature->pos.y))) 
  { 
      msg.AddByte(0x6D); 
      msg.AddPosition(*oldPos); 
      msg.AddByte(oldStackPos); 
      msg.AddPosition(creature->pos);
      if(magiceffect)
         msg.AddMagicEffect(creature->pos, 0x0A); 
      msg.WriteToSocket(s); 
  } 
  else 
  { 
      //Message for players that can see oldPos
     if ((creature != player) && CanSee(oldPos->x, oldPos->y)) 
     { 
          msg.AddByte(0x6C); 
          msg.AddPosition(*oldPos); 
          msg.AddByte(oldStackPos); 
          msg.WriteToSocket(s);
     } 
     //Message for players that can see the pos  
     if (CanSee(creature->pos.x, creature->pos.y) && !(creature == player)) 
     {
         bool known;
         unsigned long removedKnown;
         checkCreatureAsKnown(creature->getID(), known, removedKnown);

         msg.AddByte(0x6A);
         msg.AddPosition(creature->pos);
  	     msg.AddCreature(creature, known, removedKnown);
         if(magiceffect)
            msg.AddMagicEffect(creature->pos, 0x0A);
         msg.AddLight(creature);
         msg.WriteToSocket(s);
     }
  } 
  //Mesage for the player
  if (creature == player) 
  {
             
      msg.AddByte(0x64); 
      msg.AddPosition(player->pos); 
      GetMapDescription(player->pos.x-8, player->pos.y-6, player->pos.z, 18, 14, msg); 

      msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); 
      msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); 
      msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); 

      msg.AddByte(0xE4); // TODO Light level 
      msg.AddByte(0xFF); 

      if(magiceffect)
         msg.AddMagicEffect(creature->pos, 0x0A); 

      msg.AddByte(0x82); 
      msg.AddByte(map->lighttick); //LIGHT LEVEL 
      msg.AddByte(map->lightcolor);//ight? 
      GetMapLight(player->pos.x-8, player->pos.y-6, player->pos.z, 18, 14, msg);
      msg.WriteToSocket(s); 
  }
} 

void Protocol70::sendLightLevel(const int level, int color) { 
    NetworkMessage msg; 
    msg.AddByte(0x82); // 82 - Light command 
    msg.AddByte(level); // 6F - Light level!    
    msg.AddByte(color); // D7 - Light color? 
    msg.WriteToSocket(s);    
}

void Protocol70::sendCreatureAppear(const Creature *creature)
{
  NetworkMessage msg;

  if ((creature != player) && CanSee(creature->pos.x, creature->pos.y))
  {
   // msg.AddByte(0xD3);
   // msg.AddU32(creature->getID());

    bool known;
    unsigned long removedKnown;
    checkCreatureAsKnown(creature->getID(), known, removedKnown);

    msg.AddByte(0x6A);
    msg.AddPosition(creature->pos);
  	msg.AddCreature(creature, known, removedKnown);

    msg.AddMagicEffect(creature->pos, 0x0A);//Add magic on the player
    msg.AddLight(creature);

    msg.WriteToSocket(s);
  }
  else if (creature == player)
  {
	msg.AddByte(0x0A);
    msg.AddU32(player->getID());

    msg.AddByte(0x32);
    msg.AddByte(0x00);


    msg.AddByte(0x64);
    msg.AddPosition(player->pos); //Player Pos
    GetMapDescription(player->pos.x-8, player->pos.y-6, player->pos.z, 18, 14, msg); //Player get map Description

    msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF);
    msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF);
    msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF); msg.AddByte(0xFF);

	msg.AddByte(0xE4); //Todo light level
	msg.AddByte(0xFF);

	msg.AddMagicEffect(player->pos, 0x0A);//Add magic effect on Player

    msg.AddPlayerStats(player);//Add stats of the player

    msg.AddByte(0x82);//Code of Light
    msg.AddByte(map->lighttick); //Light Level
    msg.AddByte(map->lightcolor);//Light color

    msg.AddPlayerSkills(player);//Put The Skills of The player

    msg.AddPlayerInventoryItem(player, 1);//Helmet
    msg.AddPlayerInventoryItem(player, 2);//Necklace
    msg.AddPlayerInventoryItem(player, 3);//Backpack
    msg.AddPlayerInventoryItem(player, 4);//Armor
    msg.AddPlayerInventoryItem(player, 5);//Rith Slot
    msg.AddPlayerInventoryItem(player, 6);//Left Slot
    msg.AddPlayerInventoryItem(player, 7);//Legs
    msg.AddPlayerInventoryItem(player, 8);//Boots
    msg.AddPlayerInventoryItem(player, 9);//Ring
    msg.AddPlayerInventoryItem(player, 10);//Amunation


    msg.AddTextMessage(MSG_EVENT, g_config.getGlobalString("loginmsg", "Welcome.").c_str());
    std::stringstream version;
    version << "Survival Server " << Survival_Version << " by Dark-bart.";
    msg.AddTextMessage(MSG_EVENT, version.str().c_str());
    GetMapLight(player->pos.x-8, player->pos.y-6, player->pos.z, 18, 14, msg);
	msg.WriteToSocket(s);
  }
}


void Protocol70::sendCreatureDisappear(const Creature *creature, unsigned char stackPos)
{
  if ((creature != player) && CanSee(creature->pos.x, creature->pos.y))
  {
    NetworkMessage msg;

    msg.AddMagicEffect(creature->pos, NM_ME_PUFF);

    msg.AddByte(0x6c);
    msg.AddPosition(creature->pos);
    msg.AddByte(stackPos);

    msg.WriteToSocket(s);
  }
}


void Protocol70::sendCreatureTurn(const Creature *creature, unsigned char stackPos)
{
  if (CanSee(creature->pos.x, creature->pos.y))
  {
    NetworkMessage msg;

    msg.AddByte(0x6B);
    msg.AddPosition(creature->pos);
    msg.AddByte(stackPos); 

    msg.AddByte(0x63);
    msg.AddByte(0x00);
    msg.AddU32(creature->getID());
    msg.AddByte(creature->getDirection());

    msg.WriteToSocket(s);
  }
}


void Protocol70::sendCreatureSay(const Creature *creature, unsigned char type, const std::string &text)
{
  NetworkMessage msg;

  msg.AddCreatureSpeak(creature, type, text, 0);
  
  msg.WriteToSocket(s);
}

void Protocol70::sendToChannel(const Creature * creature, unsigned char type, const std::string &text, unsigned short channelId){
    NetworkMessage msg;

    msg.AddCreatureSpeak(creature, type, text, channelId);
    
    msg.WriteToSocket(s);
}

void Protocol70::sendCancel(const char *msg)
{
  NetworkMessage netmsg;
  netmsg.AddTextMessage(MSG_SMALLINFO, msg);
  netmsg.WriteToSocket(s);
}

void Protocol70::sendCancelAttacking()
{
  NetworkMessage netmsg;
  netmsg.AddByte(0xa3);
  netmsg.WriteToSocket(s);
}

void Protocol70::sendCancelFollowing()
{
  NetworkMessage netmsg;
  netmsg.AddByte(0xa3);
  netmsg.WriteToSocket(s);
}

void Protocol70::sendChangeSpeed(const Creature *creature)
{
  NetworkMessage netmsg;
  
  netmsg.AddByte(0x8F);
  netmsg.AddU32(creature->getID());
  netmsg.AddU16(creature->getSpeed());
  
  netmsg.WriteToSocket(s);
}

void Protocol70::sendCancelWalk(const char *msg)
{
  NetworkMessage netmsg;
  netmsg.AddTextMessage(MSG_SMALLINFO, msg);
  netmsg.AddByte(0xB5);
  netmsg.WriteToSocket(s);
}


