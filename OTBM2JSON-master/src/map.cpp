////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <boost/config.hpp>
#include <boost/bind.hpp>

using namespace std;

#include <stdio.h>
#include <ctype.h>

#include "definitions.h"
#include "survivalsystem.h"
#include "items.h"
#include "map.h"
#include "tile.h"
#include "player.h"
#include "tools.h"
#include "networkmessage.h"
#include "npc.h"
#include "spells.h"
#include "luascript.h"
#include "quests.h"

#define EVENT_CHECKPLAYER          123
#define EVENT_CHECKPLAYERATTACKING 124

extern LuaScript g_config;
extern Spells spells;
extern std::map<long, Creature*> channel;
Map::Map()
{
  charsonline = 0;
  serverrun = false; 
  timeonline = 0; 
  SURVIVALSYS_THREAD_LOCKVARINIT(mapLock);
  SURVIVALSYS_THREAD_LOCKVARINIT(eventLock);
  SURVIVALSYS_THREAD_SIGNALVARINIT(eventSignal);
  SURVIVALSYS_CREATE_THREAD(eventThread, this);
}


Map::~Map()
{
}


bool Map::LoadMap(std::string filename)
{
  lighttick = 0; 
  lightdelta = 1;
  lightcolor = 215;
  max_players = atoi(g_config.getGlobalString("maxplayers").c_str());
  PVP_MODE = atoi(g_config.getGlobalString("pvpmode").c_str());
  int abc = atoi(g_config.getGlobalString("manutencao").c_str());        
  int allowcee = atoi(g_config.getGlobalString("allowclones").c_str());
  std::cout << ":: Player Limit: " << max_players << std::endl;
  std::cout << ":: PvP Mode: " << PVP_MODE << std::endl;
  if(abc == 1){
         std::cout << ":: Maintenance: True" << std::endl;
         manutencao = true;
  }else{
         std::cout << ":: Maintenance: False" << std::endl;
         manutencao = false;
  } 
  std::cout << ":: Allow Clones? " << allowcee << std::endl;

  FILE* f;
  std::cout << ":: Loading Map from " << filename << " ... \n";
  f=fopen(filename.c_str(),"r");
  if(f){
    fclose(f);
    loadMapXml(filename.c_str());
    std::cout << "[done]" << std::endl;
    std::string housefile = g_config.getGlobalString("housefile");
    std::cout << ":: Loading Houses from " << housefile.c_str() << "houses.xml ... ";
    FILE* h;
    std::stringstream tmp;
    tmp <<  housefile.c_str() << "houses.xml";
    h=fopen(tmp.str().c_str(),"r");
    if(h){
          fclose(h);
          loadHousesXml(housefile.c_str());
          std::cout << "[done]" << std::endl;
    }else{
          std::cout << "Falid to load the houses! But contine!" << std::endl;         
    }
    std::cout << ":: Loading Quests from data/quests/quests.xml ...";
    quests = new Quests("data/quests/quests.xml", this);
    std::cout << "[done]" << std::endl;            
    addEvent(makeTask(15000, std::bind2nd(std::mem_fun(&Map::checkLight),1)));
    return true;
  }
  else{
    std::cout << "Falid to load the map!" << std::endl;
  }
  return true;
}

void Map::onTimeThink(int timetask)
{
     timeonline += 1;
     quests->questScript->onThink();
     addEvent(makeTask(timetask, std::bind2nd(std::mem_fun(&Map::onTimeThink),timetask)));
}     

/*****************************************************************************/


SURVIVALSYS_THREAD_RETURN Map::eventThread(void *p)
{
  Map* _this = (Map*)p;

  // basically what we do is, look at the first scheduled item,
  // and then sleep until it's due (or if there is none, sleep until we get an event)
  // of course this means we need to get a notification if there are new events added
  while (true)
  {
#ifdef __DEBUG__EVENTSCHEDULER__
    std::cout << "Schedulercycle start..." << std::endl;
#endif

    SchedulerTask* task = NULL;

    // check if there are events waiting...
    SURVIVALSYS_THREAD_LOCK(_this->eventLock)

      int ret;
    if (_this->eventList.size() == 0) {
      // unlock mutex and wait for signal
      ret = SURVIVALSYS_THREAD_WAITSIGNAL(_this->eventSignal, _this->eventLock);
    } else {
      // unlock mutex and wait for signal or timeout
      ret = SURVIVALSYS_THREAD_WAITSIGNAL_TIMED(_this->eventSignal, _this->eventLock, _this->eventList.top()->getCycle());
    }
    // the mutex is locked again now...
    if (ret == SURVIVALSYS_THREAD_TIMEOUT) {
      // ok we had a timeout, so there has to be an event we have to execute...
#ifdef __DEBUG__EVENTSCHEDULER__
      std::cout << "Event found at " << SURVIVALSYS_TIME() << " which is to be scheduled at: " << _this->eventList.top()->getCycle() << std::endl;
#endif
      task = _this->eventList.top();
      _this->eventList.pop();
    }

    SURVIVALSYS_THREAD_UNLOCK(_this->eventLock);
    if (task) {
      (*task)(_this);
      delete task;
    }
  }

}

void Map::addEvent(SchedulerTask* event) {
  bool do_signal = false;
  SURVIVALSYS_THREAD_LOCK(eventLock)

    eventList.push(event);
  if (eventList.empty() || *event < *eventList.top())
    do_signal = true;

  SURVIVALSYS_THREAD_UNLOCK(eventLock)

    if (do_signal)
      SURVIVALSYS_THREAD_SIGNAL_SEND(eventSignal);

}

Tile* Map::getTile(unsigned short _x, unsigned short _y, unsigned char _z)
{
  if (_z < MAP_LAYER)
  {
    // _x & 0x3F  is like _x % 64
    TileMap *tm = &tileMaps[_x & 1][_y & 1][_z];

    // search in the stl map for the requested tile
    TileMap::iterator it = tm->find((_x << 16) | _y);

    // ... found
    if (it != tm->end())
      return it->second;
  }
	
	 // or not
  return NULL;
}


void Map::setTile(unsigned short _x, unsigned short _y, unsigned char _z, unsigned short groundId)
{
  Tile *tile = getTile(_x, _y, _z);

  if (tile != NULL)
  {
    tile->ground = groundId;
  }
  else
  {
    tile = new Tile(Position(_x,_y,_z));
    tile->ground = groundId;
    tileMaps[_x & 1][_y & 1][_z][(_x << 16) | _y] = tile;
  }  
}



int Map::loadMapXml(const char *filename){
	xmlDocPtr doc;
	xmlNodePtr root, tile, p;
	int width, height;

	xmlLineNumbersDefault(1);
	doc=xmlParseFile(filename);
	if (!doc) {
    std::cout << "FATAL: couldnt load map. exiting" << std::endl;
    exit(1);
  }
	root=xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*) "map")){
    xmlFreeDoc(doc);
    std::cout << "FATAL: couldnt load map. exiting" << std::endl;
    exit(1);
  }

	width=atoi((const char*)xmlGetProp(root, (const xmlChar *) "width"));
	height=atoi((const char*)xmlGetProp(root, (const xmlChar *) "height"));

	int xorig=((MAP_WIDTH)-width)/2;
	int yorig=((MAP_HEIGHT)-height)/2;
	tile=root->children;
	int numpz = 0;
	for(int y=0; y < height; y++){
	    for(int x=0; x < width; x++){ 
		    if (!tile) {
		    std::cout << "No tile for " << x << " / " << y << std::endl;
		    exit(1);
	    }
			const char* pz = (const char*)xmlGetProp(tile, (const xmlChar *) "pz");
			p=tile->children;
	
			while(p)
			{
				if(xmlStrcmp(p->name,(const xmlChar*) "item")==0){
					Item* myitem=new Item();
					myitem->unserialize(p);

					if (myitem->isGroundTile())
					{                      
						setTile(xorig+x, yorig+y, 7, myitem->getID());
						delete myitem;

						if (pz && (strcmp(pz, "1") == 0)) {
							numpz++;
							getTile(xorig+x, yorig+y, 7)->setPz();
						}
				    }
					else
					{
						Tile *t = getTile(xorig+x, yorig+y, 7);
						if (t)
						{
							if (myitem->isAlwaysOnTop())
								t->topItems.push_back(myitem);
							else
								t->downItems.push_back(myitem);
						}
					}

				}
				else if(xmlStrcmp(p->name,(const xmlChar*) "npc")==0){
					std::string name = (const char*)xmlGetProp(p, (const xmlChar *) "name");
					Npc* mynpc = new Npc(name.c_str(), this);
                    if(mynpc->isLoaded())
                    {
					 mynpc->pos.x=xorig+x;
					 mynpc->pos.y=yorig+y;
                    					
					 if((char*)xmlGetProp(p, (xmlChar*)"dir")){
					  std::string dir = (const char*)xmlGetProp(p, (const xmlChar *) "dir");
					  if(dir == "NORTH"){          
					    mynpc->setDirection((Direction)0);
					    mynpc->dirr = 0;
					  }else if(dir == "SOUTH"){
					   mynpc->setDirection((Direction)2);
					   mynpc->dirr = 2;
					  }else if(dir == "EAST"){
					   mynpc->setDirection((Direction)1);
					   mynpc->dirr = 1;
                      }else if(dir == "WEST"){
					   mynpc->setDirection((Direction)3);
					   mynpc->dirr = 3;
                      } 
                     }else{
                       mynpc->dirr = 0;    
                     }        
                          			
                     mynpc->masterPos = mynpc->pos;
                    
					 if((char*)xmlGetProp(p, (xmlChar*)"respaw"))
					 {
						 mynpc->shouldrespawn = true;
						 mynpc->respawntime = atoi((const char*)xmlGetProp(p, (const xmlChar *) "respaw"));
					 }else{
                         mynpc->shouldrespawn = false;
						 mynpc->respawntime = 24000;  
                     }      
					 this->placeCreature(mynpc);
                    }else delete mynpc; 
				}

				else if(xmlStrcmp(p->name,(const xmlChar*) "tele")==0){
                    Tile *tele = getTile(xorig+x, yorig+y, 7);
                    tele->setTele();
					tele->magiceffect = (bool)atoi((const char*)xmlGetProp(p, (const xmlChar *) "magic"));
					tele->teleportPos.x = atoi((const char*)xmlGetProp(p, (const xmlChar *) "local_x"));
					tele->teleportPos.y = atoi((const char*)xmlGetProp(p, (const xmlChar *) "local_y"));
					tele->teleportPos.z = 7;    
				}

				else if(xmlStrcmp(p->name,(const xmlChar*) "ropetele")==0){
                    Tile *tele = getTile(xorig+x, yorig+y, 7);
                    tele->setRopeTele();
					tele->magiceffect = (bool)atoi((const char*)xmlGetProp(p, (const xmlChar *) "magic"));
					tele->teleportPos.x = atoi((const char*)xmlGetProp(p, (const xmlChar *) "local_x"));
					tele->teleportPos.y = atoi((const char*)xmlGetProp(p, (const xmlChar *) "local_y"));
					tele->teleportPos.z = 7;
				}

				else if(xmlStrcmp(p->name,(const xmlChar*) "usetele")==0){
                    Tile *tele = getTile(xorig+x, yorig+y, 7);
                    tele->setUseTele();
					tele->magiceffect = (bool)atoi((const char*)xmlGetProp(p, (const xmlChar *) "magic"));
					tele->teleportPos.x = atoi((const char*)xmlGetProp(p, (const xmlChar *) "local_x"));
					tele->teleportPos.y = atoi((const char*)xmlGetProp(p, (const xmlChar *) "local_y"));
					tele->teleportPos.z = 7;
				}
				else if(xmlStrcmp(p->name,(const xmlChar*) "gate")==0){
                    Tile *gatetile = getTile(xorig+x, yorig+y, 7);
				    if(atoi((const char*)xmlGetProp(p, (const xmlChar *) "lvl"))){
                        gatetile->setGateLvl();
						gatetile->gatelvl = atoi((const char*)xmlGetProp(p, (const xmlChar *) "lvl"));
					}	
                }	   
				p=p->next;
			}
			tile=tile->next;
		}
	}
	
  xmlFreeDoc(doc);
	
	return 0;
}

int Map::loadHousesXml(std::string filename){
    std::stringstream tmp;
    tmp <<  filename.c_str() << "houses.xml";
    const char *filename2 = tmp.str().c_str();
    
	xmlDocPtr doc;
	xmlNodePtr root, house;

	xmlLineNumbersDefault(1);
	doc=xmlParseFile(filename2);
	if (!doc) {
       std::cout << "FATAL: couldnt load houses exiting" << std::endl;
       return 0;
    }
	root=xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*) "houses")){
       xmlFreeDoc(doc);
       std::cout << "FATAL: couldnt load houses exiting" << std::endl;
       return 0;
    }

	house=root->children;
	int housenow = 1;
	while(house)
    {
            std::string str=(char*)house->name;    
	        if(str=="house")
            {
			    char* housefile = (char*)xmlGetProp(house, (xmlChar*)"arquivo");
			    std::stringstream tmp2;
                tmp2 <<  filename.c_str() << housefile << ".xml";
			    House *house = new House(tmp2.str().c_str(), housenow);
			    housesGame[housenow] = house;
			    addEvent(makeTask(10000, std::bind2nd(std::mem_fun(&Map::saveHouse),house)));
			    housenow += 1;
            }    
            house=house->next;
	}
    	
    xmlFreeDoc(doc);	
	return 0;
}

void Map::saveHouse(House* house)
{
   house->saveHouseXml();
   addEvent(makeTask(10000, std::bind2nd(std::mem_fun(&Map::saveHouse),house)));  
}

House* Map::getHouseByID(unsigned long id)
{
  std::map<long, House*>::iterator i;
  for( i = housesGame.begin(); i != housesGame.end(); i++ )
  {
    if((i->second)->id == id)
    {
      return i->second;
    }
  }
  return NULL;
}

House* Map::getHouseByName(const char* s)
{
  std::map<long, House*>::iterator i;
	std::string txt1 = s;
	std::transform(txt1.begin(), txt1.end(), txt1.begin(), upchar);

  for( i = housesGame.begin(); i != housesGame.end(); i++ )
	{
		std::string txt2 = (i->second)->name;
		std::transform(txt2.begin(), txt2.end(), txt2.begin(), upchar);
		if(txt1 == txt2)
    {
      return i->second;
    }
  }
  return NULL;
}

Creature* Map::getCreatureByID(unsigned long id)
{
  std::map<long, Creature*>::iterator i;
  for( i = playersOnline.begin(); i != playersOnline.end(); i++ )
  {
    if((i->second)->getID() == id )
    {
      return i->second;
    }
  }
  return NULL;
}

Creature* Map::getCreatureByName(const char* s)
{
  std::map<long, Creature*>::iterator i;
	std::string txt1 = s;
	std::transform(txt1.begin(), txt1.end(), txt1.begin(), upchar);

  for( i = playersOnline.begin(); i != playersOnline.end(); i++ )
	{
		std::string txt2 = (i->second)->getName();
		std::transform(txt2.begin(), txt2.end(), txt2.begin(), upchar);
		if(txt1 == txt2)
    {
      return i->second;
    }
  }
  return NULL;
}

bool Map::placeCreature(Creature* c)
{
  Player* pCanEnter = dynamic_cast<Player*>(c);
  if ((c->access == 0 && charsonline >= max_players && pCanEnter && !pCanEnter->premmy) || (manutencao && c->access == 0))
    return false;

	SURVIVALSYS_THREAD_LOCK(mapLock)

	playersOnline[c->getID()] = c;
	Player* player = dynamic_cast<Player*>(c);
	if(player)
	   charsonline++;
	int total  = (int)playersOnline.size() - (int)charsonline;
	std::cout << ":: " << total << " npcs online. And " << charsonline << " chars online." << std::endl;
	addEvent(makeTask(1000, std::bind2nd(std::mem_fun(&Map::checkPlayer), c->id)));
	addEvent(makeTask(2000, std::bind2nd(std::mem_fun(&Map::checkPlayerAttacking), c->id)));
	if(player) {
        addEvent(makeTask(2000, std::bind2nd(std::mem_fun(&Map::checkPlayerFollowing), c->id))); 
    }
	
    if(player){
        addEvent(makeTask(10000, std::bind2nd(std::mem_fun(&Map::savePlayer), player->getID())));
    }
        
	if (!c->canMovedTo(getTile(c->pos.x, c->pos.y, c->pos.z)) || (getTile(c->pos.x, c->pos.y, c->pos.z))->isHouseDoor())
  {   
      bool found =false;
      for(int cx =c->pos.x-1; cx <= c->pos.x+1 && !found; cx++){
                    for(int cy = c->pos.y-1; cy <= c->pos.y+1 && !found; cy++){
   #if __DEBUG__                            
                        std::cout << "Search pos x:" <<cx <<" y: "<< cy << std::endl;   
   #endif                                     
						if (c->canMovedTo(getTile(cx, cy, c->pos.z)) && !(getTile(cx, cy, c->pos.z))->isHouseDoor()){
                                            c->pos.x = cx;
                                            c->pos.y = cy;
                                            found = true;
                                        }                                        
                                    }
                                }
      if(!found){
          c->pos.x = c->masterPos.x;
          c->pos.y = c->masterPos.y;
          c->pos.z = c->masterPos.z;
      }    
	}

	Tile* tile=getTile(c->pos.x, c->pos.y, c->pos.z);
	if(!tile){
		this->setTile(c->pos.x, c->pos.y, c->pos.z, 0);
		tile=getTile(c->pos.x, c->pos.y, c->pos.z);
	}
  tile->addThing(c);

	std::vector<Creature*> list;
	getSpectators(Range(c->pos, true), list);

	for(int i = 0; i < list.size(); ++i)
	{
		list[i]->onCreatureAppear(c);
	} 
    
	SURVIVALSYS_THREAD_UNLOCK(mapLock)

    return true;
}

bool Map::removeCreature(Creature* c)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)

    std::map<long, Creature*>::iterator pit = playersOnline.find(c->getID());
  if (pit != playersOnline.end()) {
    playersOnline.erase(pit);


#ifdef __DEBUG__
    std::cout << "Removing creature."<< std::endl;
#endif

    int stackpos = getTile(c->pos.x, c->pos.y, c->pos.z)->getCreatureStackPos(c);
    getTile(c->pos.x, c->pos.y, c->pos.z)->removeThing(c);
		
		std::vector<Creature*> list;
		getSpectators(Range(c->pos, true), list);

		for(int i = 0; i < list.size(); ++i)
		{
			list[i]->onCreatureDisappear(c, stackpos);
		}
  }
  int total  = (int)playersOnline.size() - (int)charsonline+1;
  std::cout << ":: " << total << " npcs online. And " << (int)(charsonline-1) << " chars online."  << std::endl;

  Player* player = dynamic_cast<Player*>(c);

  if (player){
    std::string charName = c->getName();
    player->savePlayer(charName);                    
    //player->releasePlayer();
  }/*else
    delete c;*/

   SURVIVALSYS_THREAD_UNLOCK(mapLock)

    return true;
}

void Map::thingMove(Creature *player, Thing *thing,
                    unsigned short to_x, unsigned short to_y, unsigned char to_z)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)
  Tile *fromTile = getTile(thing->pos.x, thing->pos.y, thing->pos.z);

  if (fromTile)
  {
    int oldstackpos = fromTile->getThingStackPos(thing);
    thingMoveInternal(player, thing->pos.x, thing->pos.y, thing->pos.z, oldstackpos, to_x, to_y, to_z, 1);
  }

   SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


void Map::thingMove(Creature *player,
                    unsigned short from_x, unsigned short from_y, unsigned char from_z,
                    unsigned char stackPos,
                    unsigned short to_x, unsigned short to_y, unsigned char to_z, unsigned char count)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)
  
	thingMoveInternal(player, from_x, from_y, from_z, stackPos, to_x, to_y, to_z, count);
  
	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::thingMoveInternal(Creature *player,
                    unsigned short from_x, unsigned short from_y, unsigned char from_z,
                    unsigned char stackPos,
                    unsigned short to_x, unsigned short to_y, unsigned char to_z, unsigned char count)
{

    /*#if __DEBUG__                            
         std::cout << "Move Thing Internal." << std::endl;   
    #endif*/
	Thing *thing = NULL;
    Tile *fromTile = NULL;
    Tile *toTile   = NULL;
	Item* fromContainer = NULL;
	Item* toContainer = NULL;
    //From container to container
	if(from_x == 0xFFFF && to_x == 0xFFFF) {
		Player* p = dynamic_cast<Player*>(player);
		if(!p)
			return;
      
		Item* item = NULL;
		
		if(0x40 & from_y) { 
           unsigned char from_id = from_y & 0x0F; 
           fromContainer = p->getContainer(from_id);
           if(!fromContainer || !fromContainer->isContainer()) 
              return; 

           item = fromContainer->getItem(from_z); 
           if(!item)
              return;          
        }else { 
           item = p->items[from_y]; 
           fromContainer = p->items[from_y];
           if(!fromContainer || !item)
              return; 
        }
           
		if(0x40 & to_y) { 
           unsigned char to_id = to_y & 0x0F; 
           toContainer = p->getContainer(to_id);
           if(!toContainer || !toContainer->isContainer()) 
              return; 

           Item *toSlot = toContainer->getItem(to_z); 

           if(toSlot && toSlot->isContainer()) { 
              toContainer = toSlot; 
           }
           if(!toContainer)
              return; 
        }else{ 
           toContainer = p->items[to_y] ? p->items[to_y] : p->emptySlot();
           if(!toContainer)
              return; 
        }

        if(item->isContainer()){
		     bool isItemHolding = false;
		     item->isContainerHolding(toContainer, isItemHolding);
		     if(isItemHolding || (toContainer == item && toContainer->isContainer()) || (fromContainer == item && fromContainer->isContainer())) {
                 player->sendCancel("This is impossible.");
			     return ;
             }    
		}
		
        if(toContainer->isContainer() && toContainer != fromContainer && toContainer->getContainerMaxItemCount() <= toContainer->getContainerItemCount()){
               player->sendCancel("More objects container.");
               return;
        }
               
		if(fromContainer && toContainer && fromContainer->isContainer() && toContainer->isContainer()) {
		  if(fromContainer == toContainer) {   
			   fromContainer->moveItem(from_z, to_z);  
		  }

		  else { 
			  fromContainer->removeItem(item);
			  toContainer->addItem(item); 
		  }
        }  
		thing = item;
		
	}
	//From container to tile
	else if(from_x == 0xFFFF && to_x != 0xFFFF) {
		
		Player* p = dynamic_cast<Player*>(player);
		if(!p)
			return;

		toTile = getTile(to_x, to_y, to_z);
        Item* item = NULL;
        
    	if(0x40 & from_y) {
			unsigned char from_id = from_y & 0x0F;
			fromContainer = p->getContainer(from_id);
			if(!fromContainer || !fromContainer->isContainer())
				return;
			
			item = fromContainer->getItem(from_z);
			if(!item)
                return;
		}
		else {
			item = p->items[from_y];
			fromContainer = p->items[from_y];
			if(!fromContainer || !item)
                return;
		}
           
        thing = item;   
		
	}
	//From tile to container
	else if(from_x != 0xFFFF && to_x == 0xFFFF) {
		
		Player* p = dynamic_cast<Player*>(player);
		if(!p)
			return;

		fromTile = getTile(from_x, from_y, from_z);
		thing = fromTile->getThingByStackPos(stackPos);
		
		if(!thing)
		    return;
		   
		Creature* iscreature = dynamic_cast<Creature*>(thing);
		if(iscreature){
		    player->sendCancel("You cannot take creatures.");
		    return;
        }
            
        Item* item = dynamic_cast<Item*>(thing);
        
        if(!item)
           return;
           
		if(0x40 & to_y) {
			unsigned char to_id = to_y & 0x0F;
			toContainer = p->getContainer(to_id);
			if(!toContainer || !toContainer->isContainer())
				return;

			Item *toSlot = toContainer->getItem(to_z);

			if(toSlot && toSlot->isContainer())
				toContainer = toSlot;
			if(!toContainer)
			    return;
		}
		else {
             toContainer = p->items[to_y] ? p->items[to_y] : p->emptySlot(); 
             if(!toContainer)
                 return; 
		}
           
		if(item->isContainer()){
          bool isItemHolding = false;
		  item->isContainerHolding(toContainer, isItemHolding);
		  if(isItemHolding || (toContainer == item && toContainer->isContainer())){
            player->sendCancel("This is impossible.");
		  	return ;
		  }
        }
          
        
	}
	else {
		thing = getTile(from_x, from_y, from_z)->getThingByStackPos(stackPos);
        fromTile = getTile(from_x, from_y, from_z);
        toTile   = getTile(to_x, to_y, to_z);
	}

  if (thing)
  {
	Creature* creature = dynamic_cast<Creature*>(thing);
	Item* item = dynamic_cast<Item*>(thing);
		
    if ((player->access == 0 || player->access == 1) && creature && !creature->isPushable() && creature != player) {
      player->sendCancel("Better dont touch him...");
      return;
    }
    
    Player* playerMoving = dynamic_cast<Player*>(creature);
    Npc* npcMoving = dynamic_cast<Npc*>(creature);
    Player* playere = dynamic_cast<Player*>(player);
    
    Position oldPos;
    oldPos.x = from_x;
    oldPos.y = from_y;
    oldPos.z = from_z;
        
    bool cantwalk = false;
    
    if(playerMoving && toTile && !toTile->isGateLvl() && toTile->isHouseDoor() && playerMoving->access == 0){
       cantwalk = true; 
       ItemVector::iterator iit;
       for (iit = toTile->topItems.begin(); iit != toTile->topItems.end(); iit++)
	   {
		 if ((*iit)->getID() == 1018 || (*iit)->getID() == 1019 || (*iit)->getID() == 1020 || (*iit)->getID() == 1021){
                if((*iit) && (*iit)->house)
                {
                     House *house = (*iit)->house;
                     if(house->isDoorName(toTile->pos, playerMoving->getName()))
                        cantwalk = false;       
                }
                break;         
         }       
       } 
    }else if(playerMoving && toTile && !toTile->isGateLvl() && toTile->isHouseDoor() && playerMoving->access == 0){
       ItemVector::iterator iit;
       for (iit = toTile->topItems.begin(); iit != toTile->topItems.end(); iit++)
	   {
		 if ((*iit)->getID() == 1018 || (*iit)->getID() == 1019 || (*iit)->getID() == 1020 || (*iit)->getID() == 1021){
                cantwalk = true;
         }       
       }
    }
    
    if(npcMoving/*creature*/)
    {
       int ground = getTile(creature->pos.x, creature->pos.y, creature->pos.z)->ground.getID(); 
       long long condition = (long long)creature->lastmove + (long long)creature->getWalkDuration() - 1;
       //std::cout << (long long)((long long)SURVIVALSYS_TIME() - condition) << std::endl;
       if(SURVIVALSYS_TIME() < condition)
       {                 
         /*if(playerMoving){                    
          if (player == thing)
           player->sendCancelWalk("");
         }*/
         return;                   
       }
       creature->lastmove = SURVIVALSYS_TIME();
    }
                    
    if ((fromTile != NULL || fromContainer) && (toTile != NULL || toContainer))
    {
      if (fromTile && ((abs(from_x - player->pos.x) > 1) ||
          (abs(from_y - player->pos.y) > 1)))
      {
        player->sendCancel("To far away...");
      }
			else if ((abs((fromContainer ? player->pos.x : oldPos.x) - (toContainer ? player->pos.x : to_x)) > thing->throwRange) ||
               (abs((fromContainer ? player->pos.y : oldPos.y) - (toContainer ? player->pos.y : to_y)) > thing->throwRange))
      {
        player->sendCancel("Not there...");
      }
			else if(!canThrowItemTo((fromContainer ? player->pos : Position(from_x, from_y, from_z)),
															(toContainer ? player->pos : Position(to_x, to_y, to_z)), false)) {
				player->sendCancel("You cannot throw there.");
			}
			else if (toTile && !thing->canMovedTo(toTile))
      {
        if (player == thing)
          player->sendCancelWalk("Sorry, not possible...");
        else
          player->sendCancel("Sorry, not possible...");
      }
      else if (playerMoving && toTile->isPz() && playerMoving->pzLocked) {
          if (player == thing && player->pzLocked)
            player->sendCancelWalk("You can't enter a protection zone after attacking another creature.");
          else if (playerMoving->pzLocked)
            player->sendCancel("Sorry, not possible...");
      }else if(playerMoving && toTile->isGateLvl() && !toTile->isHouseDoor() && playerMoving->level < toTile->gatelvl && player->access == 0){
            std::stringstream reqgate;
            reqgate << "Only people on level " << toTile->gatelvl << " or more can enter here.";
            player->sendCancelWalk(reqgate.str().c_str());
      }else if(playerMoving && toTile && cantwalk){
          player->sendCancelWalk("Sorry, you can't enter.");      
      }else if(playerMoving && (toTile->ground.getID() == 454 || toTile->ground.getID() == 469 || toTile->ground.getID() == 467)){ 
          if (player == thing)
            player->sendCancelWalk("Sorry, not possible.");
          else
            player->sendCancel("Sorry, not possible.");      
      }else if (playerMoving && toTile->isTele() && toTile->ground.getID() != 371) {
          teleportPlayer(toTile->teleportPos, playerMoving, toTile->magiceffect, toTile, true);
      }else if (npcMoving && toTile->isTele()) {

      }else if(npcMoving && toTile->isPz()){
            
      }      
      else if (playerMoving && fromTile->isPz() && !toTile->isPz() && player != thing && player->access != 3) {
            player->sendCancel("Sorry, not possible...");
      }else if(item && !item->isPickupable() && toContainer){
            player->sendCancel("You cannot take this object.");
      } 
			else if (fromTile && fromTile->splash == thing && fromTile->splash->isNotMoveable()) {
				player->sendCancel("You cannot move this object.");

			}
			else if (item && item->isNotMoveable()) {
				player->sendCancel("You cannot move this object.");
			}else if (item && !(item->getID() >= 100 || item->getID() <= 2750)){
                  
            }      
      else
      {   
                if(item && item->isContainer()){
                           item->closePlayersContainer();
                }             
				if(fromContainer || toContainer) {
					Player* p = dynamic_cast<Player*>(player);
						if(!p)
							return;

					thing->pos.x = to_x;
					thing->pos.y = to_y;
					thing->pos.z = to_z;

					//Throw equipment item on the floor
					if(toTile && fromContainer && !(fromTile && toContainer) && fromContainer == thing)
					{
						NetworkMessage msgo;
						p->items[from_y] = NULL;
                        toTile->addThing(thing); 
                        creatureBroadcastTileUpdated(thing->pos); 
						msgo.AddPlayerInventoryItem(p, from_y);
						p->sendNetworkMessage(&msgo);						
						return;
					}
					//Drop stackable item on floor
					else if(toTile && fromContainer && item->isStackable() && !toContainer && fromContainer->isContainer() && fromContainer != thing)
					{
                        //bool del = false; 
					    if(count == item->getItemCountOrSubtype()){
                           //del = true;
                           fromContainer->removeItem(item);
                           needContainerUpdate(fromContainer, p, item, from_z, 0xFF, true, false);
                        }else{ 
                           fromContainer->removeItem(item);     
                           item->setItemCountOrSubtype(item->getItemCountOrSubtype() - count);
                           fromContainer->addItem(item);
                           needContainerUpdate(fromContainer, p, item, from_z, 0, true, true);
                        }
                        Item *toItem = NULL;
                        if(toTile->downItems.begin() != toTile->downItems.end()){
                           ItemVector::iterator it = toTile->downItems.begin();
                           toItem = (*it);
                        }   
                        if(toItem && toItem->getID() == item->getID() && toItem->getItemCountOrSubtype() < 100){
                           int total = toItem->getItemCountOrSubtype() + count;
                           if(total > 100){  
                              Item *newitem = new Item(item->getID(), count);
                              toTile->addThing(newitem);
                              creatureBroadcastTileUpdated(thing->pos);
                           }else{
                              toItem->setItemCountOrSubtype(total);
                              creatureBroadcastTileUpdated(thing->pos);
                           }      
                        }else{       
                           Item *newitem = new Item(item->getID(), count);
						   toTile->addThing(newitem);
						   creatureBroadcastTileUpdated(thing->pos);                                              		
                        }
                        /*if(del)
                           delete item;*/                                                	
						return;
					}
					//Drop item on floor
					else if(toTile && fromContainer && !item->isStackable() && !toContainer && fromContainer->isContainer() && fromContainer != thing)
					{
						fromContainer->removeItem(item);
						needContainerUpdate(fromContainer, p, item, from_z, 0xFF, true, false);                                           
                        toTile->addThing(thing); 
						creatureBroadcastTileUpdated(thing->pos);            	
						return;
					}
					//Pickup equipment item from the floor
					else if(fromTile && !(toTile && fromContainer) && toContainer && !(0x40 & to_y) && (p->items[to_y] == toContainer || toContainer == p->emptySlot()) && !toContainer->isContainer())
					{
                         NetworkMessage msg;
                         
                         if((to_y == 1 && Item::items[item->getID()].position != "helmet") || (to_y == 2 && Item::items[item->getID()].position != "amulet") || 
                                 (to_y == 4 && Item::items[item->getID()].position != "body") || (to_y == 7 && Item::items[item->getID()].position != "legs") || 
                                 (to_y == 8 && Item::items[item->getID()].position != "boots") || (to_y == 9 && Item::items[item->getID()].position != "ring")) { 
                                msg.AddTextMessage(MSG_SMALLINFO, "You cannot put this item on this part of your body."); 
                                p->sendNetworkMessage(&msg); 
                                return; 
                        } 
                         if(to_y == 5) { 
                                 if ((p->items[6] != NULL) && (p->items[6]->getHanded() == 2)) { 
                                 msg.AddTextMessage(MSG_SMALLINFO, "You are wearing two handed weapon."); 
                                 p->sendNetworkMessage(&msg); 
                                 return; } 
                                 if ((Item::items[item->getID()].handed == 2) && ((p->items[5] != NULL && p->items[6] != NULL) || (p->items[6] != NULL))){ 
                                    std::cout << "Your item is two handed.\n"; 
                                    msg.AddTextMessage(MSG_SMALLINFO, "Your have to release one hand first."); 
                                    p->sendNetworkMessage(&msg); 
                                    return; 
                                 } 
                                                                        
                        } 
                        if(to_y == 6) { 
                                 if ((p->items[5] != NULL) && (p->items[5]->getHanded() == 2)) { 
                                 msg.AddTextMessage(MSG_SMALLINFO, "You are wearing two handed weapon."); 
                                 p->sendNetworkMessage(&msg); 
                                 return; } 
                                 if ((Item::items[item->getID()].handed == 2) && ((p->items[6] != NULL && p->items[5] != NULL) || (p->items[5] != NULL))){ 
                                    msg.AddTextMessage(MSG_SMALLINFO, "You have to release one hand first."); 
                                    p->sendNetworkMessage(&msg); 
                                    return; 
                                 } 
                                                                        
                        }
                         if(!fromTile->removeThing(thing))
                             return; 
                         if(p->items[to_y] != NULL) { 
                             Thing* fromThing = p->items[to_y]; 
                             fromThing->pos.x = from_x; 
                             fromThing->pos.y = from_y; 
                             fromThing->pos.z = from_z; 
                             fromTile->addThing(fromThing); 
                         }                   
                         p->items[to_y] = item; 
                         creatureBroadcastTileUpdated(Position(from_x, from_y, from_z)); 
                         msg.AddPlayerInventoryItem(p, to_y); 
                         p->sendNetworkMessage(&msg); 
                         return;
                         
					}
					//Pickup stackable item from the floor to container
					else if(fromTile && toContainer && item->isStackable() && !fromContainer && toContainer->isContainer())
					{
                      if(toContainer->getContainerMaxItemCount() <= toContainer->getContainerItemCount()){
                        player->sendCancel("More objects container.");
                        return;
                      }else{
                        //bool del = false;    
                        if(count == item->getItemCountOrSubtype()){
                           if(!fromTile->removeThing(item))
                              return;
                           //del = true;   
                        }else   
                           item->setItemCountOrSubtype(item->getItemCountOrSubtype() - count);
                        creatureBroadcastTileUpdated(Position(from_x, from_y, from_z));
                        Item *toSlot = toContainer->getItem(to_z);
                        if(toSlot && toSlot->getID() == item->getID() && toSlot->getItemCountOrSubtype() < 100){
                           int total = toSlot->getItemCountOrSubtype() + count;
                           if(total > 100){
                              toContainer->removeItem(toSlot);       
                              toSlot->setItemCountOrSubtype(100);
                              toContainer->addItem(toSlot);
                              needContainerUpdate(toContainer, p, toSlot, to_z, 0, false, true);
                              Item *newitem = new Item(item->getID(), (total - 100));
                              toContainer->addItem(newitem);
                              needContainerUpdate(toContainer, p, newitem, 0xFF, 0, false, false);
                           }else{
                              toContainer->removeItem(toSlot);
                              toSlot->setItemCountOrSubtype(total);
                              toContainer->addItem(toSlot);
                              needContainerUpdate(toContainer, p, toSlot, to_z, 0, false, true);
                           }      
                        }else{       
                           Item *newitem = new Item(item->getID(), count);
						   toContainer->addItem(newitem);
						   needContainerUpdate(toContainer, p, newitem, 0xFF, 0, false, false);                                              		
                        }
                        /*if(del)
                           delete item;*/							
					  }
					  return;
					}
                    //Pickup item from the floor to container
                    else if(fromTile && toContainer && !item->isStackable() && !fromContainer && toContainer->isContainer())
					{
                      if(toContainer->getContainerMaxItemCount() <= toContainer->getContainerItemCount()){
                        player->sendCancel("More objects container.");
                        return;
                      }else if(fromTile->removeThing(thing)) {
						toContainer->addItem(item);
						needContainerUpdate(toContainer, p, item, 0xFF, 0, false, false);                                              		
						creatureBroadcastTileUpdated(Position(from_x, from_y, from_z));							
					  }
					  return;
					}
					//Putting your equipment item to container 
                    else if(toContainer && fromContainer && !fromContainer->isContainer() && toContainer->isContainer() && fromContainer == thing) 
                    { 
                        NetworkMessage msg; 
                        p->items[from_y] = NULL; 
                        msg.AddPlayerInventoryItem(p, from_y); 
                        p->sendNetworkMessage(&msg); 
                        toContainer->addItem(item); 
						needContainerUpdate(toContainer, p, item, 0xFF, 0, false, false);
                        return; 
                    } 
                    //Putting items from container on your body 
                    else if(toContainer && fromContainer && !toContainer->isContainer() && fromContainer->isContainer()) 
                    { 
                        NetworkMessage msg;
                        
                        if((to_y == 1 && Item::items[item->getID()].position != "helmet") || (to_y == 2 && Item::items[item->getID()].position != "amulet") || 
                                 (to_y == 4 && Item::items[item->getID()].position != "body") || (to_y == 7 && Item::items[item->getID()].position != "legs") || 
                                 (to_y == 8 && Item::items[item->getID()].position != "boots") || (to_y == 9 && Item::items[item->getID()].position != "ring")) { 
                                msg.AddTextMessage(MSG_SMALLINFO, "You cannot put this item on this part of your body."); 
                                p->sendNetworkMessage(&msg); 
                                return; 
                        }                               
                        if(to_y == 5) { 
                                 if ((p->items[6] != NULL) && (p->items[6]->getHanded() == 2)) { 
                                 msg.AddTextMessage(MSG_SMALLINFO, "You are wearing two handed weapon."); 
                                 p->sendNetworkMessage(&msg); 
                                 return; } 
                                 if ((Item::items[item->getID()].handed == 2) && ((p->items[5] != NULL && p->items[6] != NULL) || (p->items[6] != NULL))){ 
                                    msg.AddTextMessage(MSG_SMALLINFO, "Your have to release one hand first."); 
                                    p->sendNetworkMessage(&msg); 
                                    return; 
                                 } 
                                                                        
                        } 
                        if(to_y == 6) { 
                                 if ((p->items[5] != NULL) && (p->items[5]->getHanded() == 2)) { 
                                 msg.AddTextMessage(MSG_SMALLINFO, "You are wearing two handed weapon."); 
                                 p->sendNetworkMessage(&msg); 
                                 return; } 
                                 if ((Item::items[item->getID()].handed == 2) && ((p->items[6] != NULL && p->items[5] != NULL) || (p->items[5] != NULL))){ 
                                    msg.AddTextMessage(MSG_SMALLINFO, "You have to release one hand first."); 
                                    p->sendNetworkMessage(&msg); 
                                    return; 
                                 } 
                                                                        
                        }                        
                        fromContainer->removeItem(item);                        
						needContainerUpdate(fromContainer, p, item, from_z, 0xFF, true, false);                         
                        if(p->items[to_y] != NULL) { 
                           fromContainer->addItem(p->items[to_y]);
						   needContainerUpdate(fromContainer, p, p->items[to_y], 0xFF, 0, false, false);
                        }                                  
                        p->items[to_y] = item; 
                        msg.AddPlayerInventoryItem(p, to_y); 
                        p->sendNetworkMessage(&msg);                      
                        return; 
                   }
					//Move items around containers
					else if(toContainer && fromContainer && toContainer->isContainer() && fromContainer->isContainer()) {
                        needContainerUpdate(fromContainer, player, item, from_z, to_z, true, false); 
						needContainerUpdate(toContainer, player, item, from_z, to_z, false, false);
						return;
					}
					else
						return;
				}             
        int oldstackpos = fromTile->getThingStackPos(thing);
        if(fromTile && item && (item->isStackable() || item->isMultiType())){
                    
                     thing->pos.x = to_x;
                     thing->pos.y = to_y;
                     thing->pos.z = to_z;
                     //bool del = false;
                     if(count == item->getItemCountOrSubtype()){
                        //del = true;
                        if(!fromTile->removeThing(item))
                          return; 
                     }else   
                        item->setItemCountOrSubtype(item->getItemCountOrSubtype() - count);
                        
                     creatureBroadcastTileUpdated(Position(from_x, from_y, from_z));
                     Item *toItem = NULL;
                     if(toTile->downItems.begin() != toTile->downItems.end()){
                           ItemVector::iterator it = toTile->downItems.begin();
                           toItem = (*it);
                     }   
                     if(toItem && toItem->getID() == item->getID() && toItem->getItemCountOrSubtype() < 100){
                           int total = toItem->getItemCountOrSubtype() + count;
                           if(total > 100){  
                              Item *newitem = new Item(item->getID(), count);
                              toTile->addThing(newitem);
                              creatureBroadcastTileUpdated(thing->pos);
                           }else{
                              toItem->setItemCountOrSubtype(total);
                              creatureBroadcastTileUpdated(thing->pos);
                           }      
                     }else{       
                           Item *newitem = new Item(item->getID(), count);
						   toTile->addThing(newitem);
						   creatureBroadcastTileUpdated(thing->pos);                                              		
                     }
                     /*if(del)
                           delete item;*/
                     return;                      
        }
        else if (fromTile && fromTile->removeThing(thing))
        {
		  toTile->addThing(thing);
           
          thing->pos.x = to_x;
          thing->pos.y = to_y;
          thing->pos.z = to_z;
                 			
			if (creature){    
                
            // Update direction
            if (to_y < oldPos.y) ((Player*)thing)->direction = NORTH;
            if (to_y > oldPos.y) ((Player*)thing)->direction = SOUTH;
            if (to_x > oldPos.x) ((Player*)thing)->direction = EAST;
            if (to_x < oldPos.x) ((Player*)thing)->direction = WEST;
            
            //Close containers
            if(playerMoving)
                 for(int i=0;i<16;i++)
                     if(playerMoving->containerToClose[i]){
                         playerMoving->closeContainer(i);
                         NetworkMessage msgi;
                         msgi.AddByte(0x6F);
	                     msgi.AddByte(i);
	                     playerMoving->sendNetworkMessage(&msgi);
                     }                  
            //Cancel attack if attacked creature isnt in range                 
            if(playerMoving && creature->attackedCreature != 0){
             Creature* c = getCreatureByID(creature->attackedCreature);
             if(c){      
             if((std::abs(creature->pos.x-c->pos.x) > 8) ||
				(std::abs(creature->pos.y-c->pos.y) > 5) ||
				(creature->pos.z != c->pos.z)){                      
             playerMoving->sendCancelAttacking();
             }
             }
            }
          }

					std::vector<Creature*> list;
					getSpectators(Range(min(oldPos.x, (int)to_x) - 9, max(oldPos.x, (int)to_x) + 9,
														  min(oldPos.y, (int)to_y) - 7, max(oldPos.y, (int)to_y) + 7, oldPos.z, true), list);

					for(unsigned int i = 0; i < list.size(); ++i)
					{
						list[i]->onThingMove(player, thing, &oldPos, oldstackpos);
					}

					/*if(fromTile->getThingCount() > 8) {
						cout << "Pop-up item from below..." << std::endl;

						//We need to pop up this item
						Thing *newthing = fromTile->getThingByStackPos(9);

						if(newthing != NULL) {
							creatureBroadcastTileUpdated(newthing->pos /*&oldPos*//*);
						}
					}*/
				}
			}
    }
  }
}

void Map::getSpectators(const Range& range, std::vector<Creature*>& list)
{
	CreatureVector::iterator cit;
		for (int x = range.startx; x <= range.endx; x++)
		{
			for (int y = range.starty; y <= range.endy; y++)
			{
				Tile *tile = getTile(x, y, range.endz);
				if (tile)
				{
					for (cit = tile->creatures.begin(); cit != tile->creatures.end(); cit++) {					
						list.push_back((*cit));
					}
				}
			}
		}
}

void Map::creatureBroadcastTileUpdated(const Position& pos)
{
	std::vector<Creature*> list;
	getSpectators(Range(pos, true), list);

	for(int i = 0; i < list.size(); ++i)
	{
		list[i]->onTileUpdated(&pos);
	}
}

void Map::creatureTurn(Creature *creature, Direction dir)
{
	SURVIVALSYS_THREAD_LOCK(mapLock)

    if (creature->direction != dir)
    {
      creature->direction = dir;

      int stackpos = getTile(creature->pos.x, creature->pos.y, creature->pos.z)->getThingStackPos(creature);

			std::vector<Creature*> list;
			getSpectators(Range(creature->pos, true), list);

			for(int i = 0; i < list.size(); ++i)
			{
				list[i]->onCreatureTurn(creature, stackpos);
			}
    }

   SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


void Map::creatureSay(Creature *creature, unsigned char type, const std::string &text)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)
    Player* p =dynamic_cast<Player*>(creature);                    
    if(creature->access != 1 && text == "/score")
    {
                if(p){
                   NetworkMessage msg;
                   std::stringstream score;
                   score << "You killed " << p->pled << " players and " << p->mled << " monsters. Total " << p->pled + p->mled << " creatures.";
				   msg.AddTextMessage(MSG_EVENT, score.str().c_str());
				   p->sendNetworkMessage(&msg);
                }   
    }
    else if(creature->access != 1 && text == "/cash")
    {
                Player* player = dynamic_cast<Player*>(creature);
                if(p)
                {
                   NetworkMessage msg;
                   std::stringstream score;
                   score << "You have " << p->cash << " cash coins.";
				   msg.AddTextMessage(MSG_EVENT, score.str().c_str());
				   p->sendNetworkMessage(&msg);
                }   
	}
    else if(creature->access != 1 && text[0] == '/' && text[1] == 's' && text[2] == 'e' && text[3] == 't' && text[4] == 'c' && text[5] == 'a' && text[6] == 's' && text[7] == 'h' && text[8] == ' ')
    {
                std::string pl = text;
                pl.erase(0,9);
                unsigned long trade = atoi(pl.c_str());
                if(p && trade > 0 && trade <= p->cash)
                {
                   NetworkMessage msg;  
                   std::stringstream casht;
                   casht << "Now the cash to trade is " << trade << ".";
				   msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				   p->sendNetworkMessage(&msg);
				   p->cashtrade = trade;
                }
                else if(p)
                {
                   NetworkMessage msg;  
                   std::stringstream casht;
                   casht << "The cash to trade cant be 0 or more then your total cash.";
				   msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				   p->sendNetworkMessage(&msg); 
                }             
	}
    else if(p && creature->access != 1 && text[0] == '/' && text[1] == 'a' && text[2] == 'd' && text[3] == 'd' && text[4] == 'p' && text[5] == 'l' && text[6] == 'a' && text[7] == 'y' && text[8] == 'e' && text[9] == 'r' && text[10] == ' ')
    {
                std::string pl = text;
                pl.erase(0,11);
                int xx = 0;
                int yy = 0;
                //North
                if(p->direction == NORTH)
				{
					xx = p->pos.x;
					yy = p->pos.y - 1;
				}
				// South
				if(p->direction == SOUTH)
				{
					xx = p->pos.x;
					yy = p->pos.y + 1;
				}
				// East
				if(p->direction == EAST)
				{
					xx = p->pos.x + 1;
					yy = p->pos.y;
				}
				// West
				if(p->direction == WEST)
				{
					xx = p->pos.x - 1;
					yy = p->pos.y;
				}
				bool avalibe = false;        
                  Tile *itemtile = getTile(xx,yy,7);            
				  ItemVector::iterator iit;
                  for (iit = itemtile->topItems.begin(); iit != itemtile->topItems.end(); iit++)
	              {
		            if ((*iit)->getID() == 1018 || (*iit)->getID() == 1019 || (*iit)->getID() == 1020 || (*iit)->getID() == 1021)
                    {
                      if((*iit) && (*iit)->house)
                      {        
                        House *house = (*iit)->house;
                        if(house && (house->person) == (p->getName()))
                        {
                          if(house->addDoorName(itemtile->pos, pl.c_str()))
                              avalibe = true;
                        }              
                      }   
                    }
                    break;         
                  }
                if(avalibe)
                {  
                   NetworkMessage msg;  
                   std::stringstream casht;
                   casht << "Now the player " << pl.c_str() << " can enter in this door in your house.";
				   msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				   p->sendNetworkMessage(&msg);
                }
                else if(!avalibe)
                {
                   NetworkMessage msg;  
                   std::stringstream casht;
                   casht << "Please stay front a door of your house and can have only 10 names in one door!.";
				   msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				   p->sendNetworkMessage(&msg); 
                }             
	}
	else if(p && creature->access != 1 && text[0] == '/' && text[1] == 'r' && text[2] == 'e' && text[3] == 'm' && text[4] == 'o' && text[5] == 'v' && text[6] == 'e' && text[7] == 'p' && text[8] == 'l' && text[9] == 'a' && text[10] == 'y' && text[11] == 'e' && text[12] == 'r' && text[13] == ' ')
    {
                std::string pl = text;
                pl.erase(0,14);
                int xx = 0;
                int yy = 0;
                //North
                if(p->direction == NORTH)
				{
					xx = p->pos.x;
					yy = p->pos.y - 1;
				}
				// South
				if(p->direction == SOUTH)
				{
					xx = p->pos.x;
					yy = p->pos.y + 1;
				}
				// East
				if(p->direction == EAST)
				{
					xx = p->pos.x + 1;
					yy = p->pos.y;
				}
				// West
				if(p->direction == WEST)
				{
					xx = p->pos.x - 1;
					yy = p->pos.y;
				}
				bool avalibe = false;        
                  Tile *itemtile = getTile(xx,yy,7);            
				  ItemVector::iterator iit;
                  for (iit = itemtile->topItems.begin(); iit != itemtile->topItems.end(); iit++)
	              {
		            if ((*iit)->getID() == 1018 || (*iit)->getID() == 1019 || (*iit)->getID() == 1020 || (*iit)->getID() == 1021)
                    {
                      if((*iit) && (*iit)->house)
                      {        
                        House *house = (*iit)->house;
                        if(house->removeDoorName(itemtile->pos, pl))
                          avalibe = true;            
                      }   
                    }
                    break;         
                  }
                if(avalibe)
                {  
                   NetworkMessage msg;  
                   std::stringstream casht;
                   casht << "Now the player " << pl.c_str() << " can't enter in this door in your house.";
				   msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				   p->sendNetworkMessage(&msg);
                }
                else if(!avalibe)
                {
                   NetworkMessage msg;  
                   std::stringstream casht;
                   casht << "Please stay front a door of your house and enter a valid name.";
				   msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				   p->sendNetworkMessage(&msg); 
                }             
	}
	else if(p && creature->access != 1 && text[0] == '/' && text[1] == 's' && text[2] == 'e' && text[3] == 'e' && text[4] == 'p' && text[5] == 'l' && text[6] == 'a' && text[7] == 'y' && text[8] == 'e' && text[9] == 'r' && text[10] == 's')
    {
                int xx = 0;
                int yy = 0;
                //North
                if(p->direction == NORTH)
				{
					xx = p->pos.x;
					yy = p->pos.y - 1;
				}
				// South
				if(p->direction == SOUTH)
				{
					xx = p->pos.x;
					yy = p->pos.y + 1;
				}
				// East
				if(p->direction == EAST)
				{
					xx = p->pos.x + 1;
					yy = p->pos.y;
				}
				// West
				if(p->direction == WEST)
				{
					xx = p->pos.x - 1;
					yy = p->pos.y;
				}
				bool avalibe = false;        
                  Tile *itemtile = getTile(xx,yy,7);            
				  ItemVector::iterator iit;
                  for (iit = itemtile->topItems.begin(); iit != itemtile->topItems.end(); iit++)
	              {
		            if ((*iit)->getID() == 1018 || (*iit)->getID() == 1019 || (*iit)->getID() == 1020 || (*iit)->getID() == 1021)
                    {
                      if((*iit) && (*iit)->house)
                      {        
                        House *house = (*iit)->house;
                        if(house && (house->person) == (p->getName()))
                        {
                          avalibe = true;       
                          NetworkMessage msg;
                          std::stringstream casht;
                          house->getDoorNames(itemtile->pos, casht);              
                          msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				          p->sendNetworkMessage(&msg);
                        }              
                      }   
                    }
                    break;         
                  }
                if(!avalibe)
                {
                   NetworkMessage msg;  
                   std::stringstream casht;
                   casht << "Please stay front a door of your house.";
				   msg.AddTextMessage(MSG_EVENT, casht.str().c_str());
				   p->sendNetworkMessage(&msg); 
                }             
	}
	else if(creature->access != 1 && text[0] == '/' && text[1] == 'c' && text[2] == 'a' && text[3] == 's' && text[4] == 'h' && text[5] == 't' && text[6] == 'o' && text[7] == ' ')
    {
                if(p)
                {
                   std::string pl = text;
				   pl.erase(0,8);
                   Creature* cto = getCreatureByName(pl.c_str());
                   Player* pto = dynamic_cast<Player*>(cto);
                   if(cto && p->cashtrade != 0){    
                       NetworkMessage msg;
                       NetworkMessage msg2;
                       std::stringstream casht;
                       std::stringstream casht2;
                       casht << "You give to " << pl.c_str() << " " << p->cashtrade << " cash coins.";
				       casht2 << p->getName().c_str() << " gives " << p->cashtrade << " cash coins to you.";
                       msg.AddTextMessage(MSG_ADVANCE, casht.str().c_str());
                       msg2.AddTextMessage(MSG_ADVANCE, casht2.str().c_str());
				       p->sendNetworkMessage(&msg);
				       pto->sendNetworkMessage(&msg2);
				       p->cash -= p->cashtrade;
				       pto->cash += p->cashtrade;	
                       p->cashtrade  = 0;			       
                   }else{
                       NetworkMessage msg;
                       std::stringstream erro;
                       erro << "This player is not online or you didnt set the cash to trade, set the cash with the command /setcash, exemple \"/setcash 2000\".";      
                       msg.AddTextMessage(MSG_ADVANCE, erro.str().c_str());
                       p->sendNetworkMessage(&msg);
                   }     
                       
                }   
	}// Check if this was a GM command
	else if(text[0] == '/' && creature->access > 0)
	{
		// Get the command
		switch(text[1])
		{
			default:break;
			// Summon
			case 's':
			{
                //Prepare the string                 
				std::string cmd = text;
				cmd.erase(0,3);
				
				Npc *npc = NULL;
				if(creature->access == 1)
				   npc = new Npc(cmd.c_str(), (Map *)this, false);
				else
                   npc = new Npc(cmd.c_str(), (Map *)this);
                   
                if(!npc)
                   break;      
                   
				if(!npc->isLoaded()){
					delete npc;
					break;
				}
				// Set the NPC pos
				if(creature->direction == NORTH)
				{
					npc->pos.x = creature->pos.x;
					npc->pos.y = creature->pos.y - 1;
					npc->pos.z = creature->pos.z;
				}
				// South
				if(creature->direction == SOUTH)
				{
					npc->pos.x = creature->pos.x;
					npc->pos.y = creature->pos.y + 1;
					npc->pos.z = creature->pos.z;
				}
				// East
				if(creature->direction == EAST)
				{
					npc->pos.x = creature->pos.x + 1;
					npc->pos.y = creature->pos.y;
					npc->pos.z = creature->pos.z;
				}
				// West
				if(creature->direction == WEST)
				{
					npc->pos.x = creature->pos.x - 1;
					npc->pos.y = creature->pos.y;
					npc->pos.z = creature->pos.z;
				}
				npc->masterPos = npc->pos;
				placeCreature(npc);
			} break; // case 's':
            // PvP modes                  
            case 'p':
			{
                //Prepare the string  
				std::string cmd = text;
				cmd.erase(0,3);

				if (cmd == "1")
				{
					PVP_MODE = 1;
					std::cout << ":: The server now is PvP." << std::endl;
					creatureBroadcastMessage(creature, "The server now is PvP.");
				}

				if (cmd == "2")
				{
					PVP_MODE = 2;
					std::cout << ":: The server now is No-PvP." << std::endl;					
					creatureBroadcastMessage(creature, "The server now is No-PvP.");
				}
			} break; //case 'p':
            //See text colors, used for tests to get color of hits
            case 'x':
            {
              Player* p = dynamic_cast<Player*>(creature);
              if(p){
                 std::string cmd = text;   
                 cmd.erase(0,3);
                 int color = atoi(cmd.c_str());
                 
                 NetworkMessage msg;
			     std::stringstream teste;
                 teste << "Test";
                 msg.AddAnimatedText(p->pos, color, teste.str());
                 p->sendNetworkMessage(&msg);
              }   
              break;   
            }
            //Teleport a player to the GM     
            case 'c':
			{
                //Prepare the string  
				std::string cmd = text;
				cmd.erase(0,3);
				
                Player* target = NULL;				
				Player* p = dynamic_cast<Player*>(creature);
				
			    if (getCreatureByName(cmd.c_str())) { 
                   Player* isp = dynamic_cast<Player*>(getCreatureByName(cmd.c_str()));                                 
                   if(isp){                               
                     target = dynamic_cast<Player*>(getCreatureByName(cmd.c_str()));
                   }else{
                     p->sendCancel("This player with this name cannot be found.");
                     break;    
                   }                    
                }else{
                      p->sendCancel("This creature with this name cannot be found.");
                      break;
                }
			  
			    unsigned short from_x;
                unsigned short from_y;
                unsigned char from_z;
                unsigned short to_x;
                unsigned short to_y;
                unsigned char to_z;
			  			
				from_x = target->pos.x;
				from_y = target->pos.y;
				from_z = target->pos.z;
				
                  bool lol = false;
                  for(int a=-1; a<=1;a++){
                    for(int b=-1;b<=1;b++){ 
                      Tile *tilee = getTile(p->pos.x+a, p->pos.y+b, p->pos.z);
                      if(!tilee->isBlocking() && !lol && !(a == 0 && b == 0)){
                        lol = true;
                        to_x = p->pos.x+a;
                        to_y = p->pos.y+b;
                        to_z = p->pos.z;
                      }
                    }  
                  }   
				
				Position oldPos;
   			    oldPos.x = from_x;
                oldPos.y = from_y;
                oldPos.z = from_z;
				
                Tile *fromTile = getTile(from_x, from_y, from_z);
                Tile *toTile   = getTile(to_x, to_y, to_z);
                
                if(toTile->isBlocking()){
                       p->sendCancel("He cannot go there.");
                       break;
                }
                #if __DEBUG__                            
                        std::cout << "Teleport" << std::endl;   
                #endif
                int oldstackpos = fromTile->getThingStackPos(target);

                if (fromTile->removeThing(target))
                {
					toTile->addThing(target);

                    target->pos.x = to_x;
                    target->pos.y = to_y;
                    target->pos.z = to_z;
                }

                CreatureVector::iterator cit;
                    for (int x = min(oldPos.x, (int)to_x) - 14; x <= max(oldPos.x, (int)to_x) + 14; x++)
                       for (int y = min(oldPos.y, (int)to_y) - 18; y <= max(oldPos.y, (int)to_y) + 18; y++)
                       {
                         Tile *tile = getTile(x, y, 7);
                         if (tile)
                         {
				   
                            for (cit = tile->creatures.begin(); cit != tile->creatures.end(); cit++)
                            {
                                (*cit)->onTeleport(target, target, &oldPos, oldstackpos, true);
                            }
                         }
					   }
		    } break; //case 'c':
            //Teleport to tiles and to creatures                  
            case 't':
			{
				//Prepare the string 
				std::string cmd = text;
				cmd.erase(0,3);
				
				int tel;
				bool count = false;
				Player* target = 0;				
				Player* p = dynamic_cast<Player*>(creature);				

			  if (cmd == "1"){                      
			    tel = 1;
			  }else if(cmd == "2"){
			    tel = 2;
			  }else if(cmd == "3"){
			    tel = 3;
			  }else if(cmd == "4"){
			    tel = 4;
			  }else if(cmd == "5"){
			    tel = 5;
			  }else if(cmd == "6"){
			    tel = 6;
			  }else if(cmd == "7"){
			    tel = 7;
              }else if(cmd == "8"){
			    tel = 8;
			  }else if(cmd == "9"){
			    tel = 9;
			  }else if(cmd == "10"){
			    tel = 10;
			  }else if(cmd == "15"){
			    tel = 15;
			  }else if(cmd == "20"){
			    tel = 20;
			  }else if(cmd == "25"){
			    tel = 25;
			  }else if(cmd == "30"){
			    tel = 30;
			  }else if(cmd == "40"){
			    tel = 40; 
              }else if(cmd == "50"){
			    tel = 50; 
			  }else{
			    if (getCreatureByName(cmd.c_str())) {
                   Player* isp = dynamic_cast<Player*>(getCreatureByName(cmd.c_str()));                                 
                   if(isp){                               
                     count = true;
                     target = dynamic_cast<Player*>(getCreatureByName(cmd.c_str()));
                   }else{
                     p->sendCancel("This player with this name cannot be found.");
                     break;    
                   }                           
                }else{
                      p->sendCancel("This player with this name cannot be found.");
                      break;
                }      
			  }
			  
			    unsigned short from_x;
                unsigned short from_y;
                unsigned char from_z;
                unsigned short to_x;
                unsigned short to_y;
                unsigned char to_z;
			  			
				from_x = creature->pos.x;
				from_y = creature->pos.y;
				from_z = creature->pos.z;
				
				if(creature->direction == NORTH && !count){
				    to_x = creature->pos.x;
				    to_y = creature->pos.y-tel;
				    to_z = creature->pos.z;
				}else if(creature->direction == SOUTH && !count){
				    to_x = creature->pos.x;
				    to_y = creature->pos.y+tel;
				    to_z = creature->pos.z;
				}else if(creature->direction == EAST && !count){
				    to_x = creature->pos.x+tel;
				    to_y = creature->pos.y;
				    to_z = creature->pos.z;
				}else if(creature->direction == WEST && !count){
				    to_x = creature->pos.x-tel;
				    to_y = creature->pos.y;
				    to_z = creature->pos.z;
				}else{
                  bool lol = false;
                  for(int a=-1; a<=1;a++){
                    for(int b=-1;b<=1;b++){ 
                      Tile *tilee = getTile(target->pos.x+a, target->pos.y+b, target->pos.z);
                      if(!tilee->isBlocking() && !lol && !(a == 0 && b == 0)){
                        lol = true;
                        to_x = target->pos.x+a;
                        to_y = target->pos.y+b;
                        to_z = target->pos.z;
                      }
                    }  
                  }  
                }    
				
				Position oldPos;
   			    oldPos.x = from_x;
                oldPos.y = from_y;
                oldPos.z = from_z;
				
                Tile *fromTile = getTile(from_x, from_y, from_z);
                Tile *toTile   = getTile(to_x, to_y, to_z);
                
                if(toTile->isBlocking()){
                       if(p)                  
                         p->sendCancel("You cannot go there.");
                       break;
                }
                #if __DEBUG__                            
                        std::cout << "Teleport" << std::endl;   
                #endif
                int oldstackpos = fromTile->getThingStackPos(creature);

                if (fromTile->removeThing(creature))
                {
					toTile->addThing(creature);

                    creature->pos.x = to_x;
                    creature->pos.y = to_y;
                    creature->pos.z = to_z;
                }

                CreatureVector::iterator cit;
                    for (int x = min(oldPos.x, (int)to_x) - 14; x <= max(oldPos.x, (int)to_x) + 14; x++)
                       for (int y = min(oldPos.y, (int)to_y) - 18; y <= max(oldPos.y, (int)to_y) + 18; y++)
                       {
                         Tile *tile = getTile(x, y, 7);
                         if (tile)
                         {
				   
                            for (cit = tile->creatures.begin(); cit != tile->creatures.end(); cit++)
                            {
                                (*cit)->onTeleport(creature, creature, &oldPos, oldstackpos, true);
                            }
                         }
					   }
		    } break; //case 't':
            //Close the server
            case 'q':
            {
                 creatureBroadcastMessage(creature, "The Server is going down in one minut please logout.");
                 addEvent(makeTask(60000, std::bind2nd(std::mem_fun(&Map::closeServer), creature)));
                 
            } break; //case 'q':
            //Block the server to tests         
            case 'm':
            {
                 // Create a non-const copy of the command
				 std::string cmd = text;
				 // Erase the first 2 bytes
				 cmd.erase(0,3);
				 
                 if (cmd == "1"){                      
			        manutencao = true;
			        creatureBroadcastMessage(creature, "The Server is in maintenance now.");
			     }else if(cmd == "2"){
			        manutencao = false;
			        creatureBroadcastMessage(creature, "The Server isn't in maintenance now.");
			     }
                 
            } break; //case 'm':         
            //Teleport with commands, Ex:/d 233 233              
            case 'd':
            {
                 std::string cmd = text;
                 cmd.erase(0,3);
                 
                 int lenn = strlen(cmd.c_str());
                 if(lenn < 7)
                    break;
                    
                 std::string cmdx = cmd;
                 std::string cmdy = cmd;
                 
                 cmdx.erase(3,7);
                 cmdy.erase(0,4);
                 
                 int px = atoi(cmdx.c_str());
                 int py = atoi(cmdy.c_str());
                 int pz = 7;
                 
                 Player* p = dynamic_cast<Player*>(creature);
                 
                 if(!(px>=0 && px<=512 && py>=0 && py<=512)){
                            p->sendCancel("This tile don't exist.");
                            break;
                 }
                 unsigned short from_x;
                 unsigned short from_y;
                 unsigned char from_z;
                 unsigned short to_x;
                 unsigned short to_y;
                 unsigned char to_z;
                 
                 from_x = creature->pos.x;
 				 from_y = creature->pos.y;
				 from_z = creature->pos.z;
				 
				 to_x = px;
	             to_y = py;
	             to_z = pz;
	             	             
	            Position oldPos;
   			    oldPos.x = from_x;
                oldPos.y = from_y;
                oldPos.z = from_z;
				
                Tile *fromTile = getTile(from_x, from_y, from_z);
                Tile *toTile   = getTile(to_x, to_y, to_z);
                
                if(toTile->isBlocking()){
                       p->sendCancel("You cannot go there.");
                       break;
                }
                #if __DEBUG__                            
                        std::cout << "Teleport" << std::endl;   
                #endif
                int oldstackpos = fromTile->getThingStackPos(creature);

                if (fromTile->removeThing(creature))
                {
					toTile->addThing(creature);

                    creature->pos.x = to_x;
                    creature->pos.y = to_y;
                    creature->pos.z = to_z;
                }

                CreatureVector::iterator cit;
                    for (int x = min(oldPos.x, (int)to_x) - 14; x <= max(oldPos.x, (int)to_x) + 14; x++)
                       for (int y = min(oldPos.y, (int)to_y) - 18; y <= max(oldPos.y, (int)to_y) + 18; y++)
                       {
                         Tile *tile = getTile(x, y, 7);
                         if (tile)
                         {
				   
                            for (cit = tile->creatures.begin(); cit != tile->creatures.end(); cit++)
                            {
                                (*cit)->onTeleport(creature, creature, &oldPos, oldstackpos, true);
                            }
                         }
					   }
		    } break; //case 'd':
            //Advanced Message                  
            case 'a':
            {
              //Prepare the string 
              std::string cmd = text; 
              cmd.erase(0,3);
              
              creatureAdvancedMessage(creature, cmd.c_str());
                 
            } break; //case 'q':         
            //Give the position
            case 'o': 
            { 
               // Send a message To Gamemaster.
               Player* GM = dynamic_cast<Player*>(creature);
               if(GM){ 
                NetworkMessage GMmsg;
                std::stringstream GMReply; 
                GMReply << "You are at Position: x= " << GM->pos.x << ", y= " << GM->pos.y << ", z= " << GM->pos.z << ".";  
                GMmsg.AddTextMessage(MSG_INFO, GMReply.str().c_str()); 
                GM->sendNetworkMessage(&GMmsg);
                }  
            } break; //case 'o'  
            //Nothing
            /*case 'i': 
            { 
               Item *item = new Item(1800, 10);
               Tile *tile = getTile(creature->pos.x, creature->pos.y, creature->pos.z);
               tile->addThing(item);
               creatureBroadcastTileUpdated(creature->pos);
                           
            } break; //case 'i'*/
            //Kick                               
            case 'k': 
            { 
              //Prepare the string 
              std::string cmd = text; 
              cmd.erase(0,3); 

              // If the name of the creature is not found. 
              if (!getCreatureByName(cmd.c_str())){ 
                std::stringstream GMReply; 
                GMReply << "A Player named \"" << cmd.c_str() << "\" cannot be found on this server."; 

                Player* player = dynamic_cast<Player*>(creature); 

                NetworkMessage msg; 
                msg.AddTextMessage(MSG_INFO, GMReply.str().c_str()); 
                player->sendNetworkMessage(&msg); 
              } 

              // The creature to be kicked found online, if it doesn't have access of 3 then allow to be kicked. 
              if (getCreatureByName(cmd.c_str()) && getCreatureByName(cmd.c_str())->access == 0) { 
               NetworkMessage GMmsg;
               Player* GM = dynamic_cast<Player*>(creature); 
               Player* Kicked = dynamic_cast<Player*>(getCreatureByName(cmd.c_str())); 

               std::stringstream GMReply; 
               GMReply << "You have kicked \"" << cmd.c_str() << "\" from the server."; 
               GMmsg.AddTextMessage(MSG_INFO, GMReply.str().c_str()); 
               GM->sendNetworkMessage(&GMmsg);
                
               Kicked->sendKick();
               std::cout << "Kicked player " << Kicked->getName() << "." << endl;
              } 


              // The target to be kicked is accessed at 3 and should not be kicked. 
              if (getCreatureByName(cmd.c_str()) && getCreatureByName(cmd.c_str())->access != 0) { 
                std::stringstream GMReply; 
                GMReply << "You cannot kick other Gamemasters or Monsters from the server."; 

                Player* player = dynamic_cast<Player*>(creature); 

                NetworkMessage msg; 
                msg.AddTextMessage(MSG_INFO, GMReply.str().c_str()); 
                player->sendNetworkMessage(&msg); 
              } 
          } break; // case 'k'                           
		}
	}

	// It was no command, or it was just a player
	else {
		std::vector<Creature*> list;
		getSpectators(Range(creature->pos), list);

		for(int i = 0; i < list.size(); ++i)
		{
			list[i]->onCreatureSay(creature, type, text);
		}
	}


	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


void Map::creatureChangeOutfit(Creature *creature)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)

	std::vector<Creature*> list;
	getSpectators(Range(creature->pos, true), list);

	for(int i = 0; i < list.size(); ++i)
	{
		list[i]->onCreatureChangeOutfit(creature);
	}

  SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::creatureWhisper(Creature *creature, const std::string &text)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)

	std::vector<Creature*> list;
	getSpectators(Range(creature->pos), list);

	for(int i = 0; i < list.size(); ++i)
	{
		if(abs(creature->pos.x - list[i]->pos.x) > 1 || abs(creature->pos.y - list[i]->pos.y) > 1)
			list[i]->onCreatureSay(creature, 2, std::string("blablablablabla..."));
		else
			list[i]->onCreatureSay(creature, 2, text);
	}
  SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::creatureYell(Creature *creature, std::string &text)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)

		Player* player = dynamic_cast<Player*>(creature);
  if(player && (player->access == 0 || player->access == 1) && player->exhaustedTicks >=1000) {
      player->exhaustedTicks += (long)g_config.getGlobalNumber("exhaustedadd", 0);
      NetworkMessage msg;
      msg.AddTextMessage(MSG_SMALLINFO, "You are exhausted.");
      player->sendNetworkMessage(&msg);
  }
  else {
      creature->exhaustedTicks = (long)g_config.getGlobalNumber("exhausted", 0);
      CreatureVector::iterator cit;
      std::transform(text.begin(), text.end(), text.begin(), upchar);

			for (int x = creature->pos.x - 18; x <= creature->pos.x + 18; x++)
				for (int y = creature->pos.y - 14; y <= creature->pos.y + 14; y++)
				{
					Tile *tile = getTile(x, y, 7);
					if (tile)
					{
						for (cit = tile->creatures.begin(); cit != tile->creatures.end(); cit++)
						{
                            Player* pla = dynamic_cast<Player*>(*cit);
                            if(pla)                      
							   (*cit)->onCreatureSay(creature, 3, text);
						}
					}
				}
  }    
   SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


void Map::creatureSpeakTo(Creature *creature, const std::string &receiver, const std::string &text)
{
  SURVIVALSYS_THREAD_LOCK(mapLock) 
  Creature* c = getCreatureByName(receiver.c_str());
  Player* p = dynamic_cast<Player*>(c);
  if(c && p)
  c->onCreatureSay(creature, 4, text);
  SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


void Map::creatureBroadcastMessage(Creature *creature, const std::string &text)
{
  if(creature->access == 0 || creature->access == 1) 
	  return;

  SURVIVALSYS_THREAD_LOCK(mapLock)

	std::map<long, Creature*>::iterator cit;
  for (cit = playersOnline.begin(); cit != playersOnline.end(); cit++)
  {
        Player *p = dynamic_cast<Player*>(cit->second);
        if(p)
		   cit->second->onCreatureSay(creature, 9, text);
	}

	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::creatureAdvancedMessage(Creature *creature, const std::string &text)
{
  if(creature->access == 0 || creature->access == 1) 
	  return;

  SURVIVALSYS_THREAD_LOCK(mapLock)

  std::map<long, Creature*>::iterator cit;
  for (cit = playersOnline.begin(); cit != playersOnline.end(); cit++)
  {
		Creature *c = cit->second;
        Player* player = dynamic_cast<Player*>(c);
        if(player){
         NetworkMessage msg;
         msg.Reset();
         msg.AddTextMessage(MSG_ADVANCE, text.c_str());
         player->sendNetworkMessage(&msg);
        } 
	}

	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::creatureGodMessage(const std::string &text)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)
  std::map<long, Creature*>::iterator cit;
  for (cit = playersOnline.begin(); cit != playersOnline.end(); cit++)
  {
		Creature *c = cit->second;
        Player* player = dynamic_cast<Player*>(c);
        if(player){
         NetworkMessage msg;
         msg.Reset();
         msg.AddTextMessage(MSG_INFO, text.c_str());
         player->sendNetworkMessage(&msg);
        } 
	}
	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::creatureToChannel(Creature *creature, unsigned char type, const std::string &text, unsigned short channelId)
{

  SURVIVALSYS_THREAD_LOCK(mapLock)

	std::map<long, Creature*>::iterator cit;
  for (cit = channel.begin(); cit != channel.end(); cit++)
  {
        Player* player = dynamic_cast<Player*>(cit->second);
        if(player)
		player->sendToChannel(creature, type, text, channelId);
	}

	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::getAreaTiles(Position pos, const unsigned char area[14][18], unsigned char dir, std::list<tiletargetdata>& list)
{
	tiletargetdata tt;

	Position tpos = pos;
	tpos.x -= 8;
	tpos.y -= 6;

	for(int y = 0; y < 14; y++) {
		for(int x = 0; x < 18; x++) {
			if(area[y][x] == dir) {
				Tile *t = getTile(tpos.x, tpos.y, tpos.z);
				if(t && !t->isBlockingProjectile() && canThrowItemTo(pos, tpos, false, true)) {
					tt.pos = tpos;
					tt.targetCount = t->creatures.size();
					tt.thingCount = t->getThingCount();
					list.push_back(tt);
				}
			}
			tpos.x += 1;
		}
		
		tpos.x -= 18;
		tpos.y += 1;
	}
}

bool Map::creatureMakeMagic(Creature *creature, const MagicEffectClass* me)
{
	const MagicEffectInstantSpellClass* magicInstant = dynamic_cast<const MagicEffectInstantSpellClass*>(me);
	const MagicEffectRuneClass* magicRune = dynamic_cast<const MagicEffectRuneClass*>(me);
	const MagicEffectAreaClass* magicArea = dynamic_cast<const MagicEffectAreaClass*>(me);
	const MagicEffectGroundClass* magicGround = dynamic_cast<const MagicEffectGroundClass*>(me);
    bool drinked = false;
    
	if(me->offensive && !creatureOnPrepareAttack(creature, me->centerpos))
		return false;

	if(!((std::abs(creature->pos.x-me->centerpos.x) <= 8) && (std::abs(creature->pos.y-me->centerpos.y) <= 6) &&
		(creature->pos.z == me->centerpos.z)))
		return false;

#ifdef __DEBUG__
	cout << "CreatureMakeMagic: " << creature->getName() << ", x: " << me->centerpos.x << ", y: " << me->centerpos.y << ", z: " << me->centerpos.z << std::endl;
#endif

	Player* player = dynamic_cast<Player*>(creature);
	if(player) {
		if(player->access == 0 || player->access == 1) {
			if(/*!magicCondition &&*/ player->exhaustedTicks >= 1000) {
				NetworkMessage msg;
				msg.AddMagicEffect(player->pos, NM_ME_PUFF);
				msg.AddTextMessage(MSG_SMALLINFO, "You are exhausted.");
				player->sendNetworkMessage(&msg);
				player->exhaustedTicks += (long)g_config.getGlobalNumber("exhaustedadd", 0);
				return false;
			}
			else if(magicInstant) {
				if(player->mana < magicInstant->manaCost) {
					NetworkMessage msg;
					msg.AddMagicEffect(player->pos, NM_ME_PUFF);
					msg.AddTextMessage(MSG_SMALLINFO, "You do not have enough mana.");
					player->sendNetworkMessage(&msg);
					return false;
				}
				else{
                    NetworkMessage msge;			    
					player->mana -= magicInstant->manaCost;
					player->manaspent += magicInstant->manaCost;
	                msge.AddPlayerStats(player);
	                player->sendNetworkMessage(&msge);
                 }    
			}
		}
	}
	
	if(magicGround) {
		Tile *groundtile = getTile(me->centerpos.x, me->centerpos.y, me->centerpos.z);
		if(!groundtile)
			return false;

		if(Item::items[magicGround->groundID].blocking && (groundtile->isBlocking() || !groundtile->creatures.empty())) {
			if(player) {
				if(!groundtile->creatures.empty()) {
					player->sendCancel("There is not enough room.");

					NetworkMessage msg;
					msg.AddMagicEffect(player->pos, NM_ME_PUFF);
					player->sendNetworkMessage(&msg);
				}
				else {
					player->sendCancel("You cannot throw there.");
				}
			}

			return false;
		}
	}
	
	if(player && (player->access == 0 || player->access == 1) && (magicArea || getTile(me->centerpos.x, me->centerpos.y, me->centerpos.z)->creatures.size() > 0)) {
		if(me->offensive)
			player->pzLocked = true;
		creature->exhaustedTicks = (long)g_config.getGlobalNumber("exhausted", 0);
	}

	if (me->offensive && player && (player->access == 0 || player->access == 1)) {
	    player->inFightTicks = (long)g_config.getGlobalNumber("pzlocked", 0);
	    player->sendIcons();
	}

	std::list<tiletargetdata> tilelist;
	std::list<tiletargetdata>::const_iterator tl;

	if(magicArea || magicInstant) {
		getAreaTiles(magicArea->centerpos, magicArea->area, magicArea->direction, tilelist);
	}
	else {
		Tile *t = getTile(me->centerpos.x, me->centerpos.y, me->centerpos.z);
		if(t) {
			tiletargetdata tt;
			tt.pos = me->centerpos;
			tt.targetCount = t->creatures.size();
			tt.thingCount = t->getThingCount();
			tilelist.push_back(tt);
		}
	}

	std::vector<Creature*> spectatorlist;
	getSpectators(Range(min(creature->pos.x, me->centerpos.x) - 14, max(creature->pos.x, me->centerpos.x) + 14,
											min(creature->pos.y, me->centerpos.y) - 11, max(creature->pos.y, me->centerpos.y) + 11,
											creature->pos.z), spectatorlist);

	Tile *attackertile = getTile(creature->pos.x,  creature->pos.y, creature->pos.z);
	Tile *targettile = NULL;

	CreatureVector::iterator cit;
	typedef std::pair<Creature*, struct targetdata> targetitem;
	std::vector<targetitem> targetvec;
	std::vector<targetitem>::const_iterator tv;
	targetitem ti;

	for(tl = tilelist.begin(); tl != tilelist.end(); tl++) {
		targettile = getTile(tl->pos.x,  tl->pos.y, tl->pos.z);

		if(!targettile)
			continue;
		
		if((creature->access == 0 || creature->access == 1) && me->offensive && targettile->isPz())
			continue;

		for(cit = targettile->creatures.begin(); cit != targettile->creatures.end(); cit++) {
			Creature* target = (*cit);
			Player* targetPlayer = dynamic_cast<Player*>(target);

			int damage = 0;
			int manaDamage = 0;

			if(!me->offensive || magicGround || (PVP_MODE == 1 && target != creature && !(target->access == 1 && creature->access == 1)) || (PVP_MODE == 2 && target != creature && ((target->access == 1 && creature->access != 1) || (creature->access == 1 && target->access != 1)))){                                      
				if(target->access == 0 || target->access == 1)
					damage = random_range(me->minDamage, me->maxDamage);
					
                if(me->type < 4 && me->type >= 0 && target->defenses[me->type] == true)
                    damage = 0;
                
				if(!me->offensive)
					damage = -damage;
					
				if(target->access == 0 && creature->access == 0 && me->offensive)
					damage = damage / 2;
                    
				if (damage > 0) {
					if(targetPlayer && me->offensive && (target->access == 0 || target->access == 1)){
						targetPlayer->inFightTicks = (long)g_config.getGlobalNumber("pzlocked", 0);
						targetPlayer->sendIcons();
					}
						
					if (target->manaShieldTicks >= 1000 && (damage < target->mana) ){
						manaDamage = damage;
						damage = 0;
					}
					else if (target->manaShieldTicks >= 1000 && (damage > target->mana) ){
						manaDamage = target->mana;
						damage -= manaDamage;
					}
					else if((target->manaShieldTicks < 1000) && (damage > target->health))
						damage = target->health;
					else if (target->manaShieldTicks >= 1000 && (damage > (target->health + target->mana))){
						damage = target->health;
						manaDamage = target->mana;
					}

					if(target->manaShieldTicks < 1000)
						target->drainHealth(damage);
					else if(manaDamage >0){
						target->drainHealth(damage);
						target->drainMana(manaDamage);
					}
					else
						target->drainMana(damage);
				} else {
					int newhealth = target->health - damage;
					if(newhealth > target->healthmax)
						newhealth = target->healthmax;
						
					target->health = newhealth;

					damage = target->health - newhealth;
					manaDamage = 0;
				}
			}

			ti.first = target;
			ti.second.damage = damage;
			ti.second.manaDamage = manaDamage;
			ti.second.stackpos = targettile->getCreatureStackPos(target);
			ti.second.hadSplash = targettile->splash != NULL;
			
			targetvec.push_back(ti);
		}

		if(magicGround) {
			//if(!targettile->isBlocking()) {
			Item* item = new Item(magicGround->groundID);
			item->pos = tl->pos;
			targettile->addThing(item);

			unsigned short decayTime = Item::items[magicGround->groundID].decayTime;
			addEvent(makeTask(decayTime * 1000, std::bind2nd(std::mem_fun(&Map::decayItem), item)));
			//}
		}
	}
	
	//Remove player from tile, add bodies/blood to the map
	std::map<Tile*, int> tileBodyCountMap; //optimization to save bandwidth
	for(tv = targetvec.begin(); tv != targetvec.end(); tv++) {
		Creature* target = tv->first;
		Player* targetPlayer = dynamic_cast<Player*>(target);
		Npc* targetNpc = dynamic_cast<Npc*>(target);
		Tile *targettile = getTile(target->pos.x, target->pos.y, target->pos.z);

		if(me->physical && tv->second.damage > 0) {
			if (!targettile->splash && target->damageColor != GRAY)
			{
                Item *item = NULL;
                if(target->health <= 0){
                 if(target->damageColor == GREEN)
                    item = new Item(1434, 4);
                 else
                    item = new Item(1434, 2);                  
                }else{                                                                                                        
                 if(target->damageColor == GREEN)
                    item = new Item(1437, 4);
                 else
                    item = new Item(1437, 2);
                }      
				item->pos = target->pos;
				targettile->splash = item;
                
				unsigned short decayTime;
				if(target->health <= 0)
                   decayTime = Item::items[1434].decayTime;
                else
                   decayTime = Item::items[1437].decayTime;
				targettile->decaySplashAfter = SURVIVALSYS_TIME() + decayTime*1000;
				addEvent(makeTask(decayTime*1000, std::bind2nd(std::mem_fun(&Map::decaySplash), targettile->splash)));
			}
		}
        if(player && (target->access == 1 || (targetPlayer && targetPlayer->pking == 1)) && target != creature && tv->second.damage > 0){	    
   		   target->addInflictedDamage(creature, tv->second.damage);
        }
		if (target->health <= 0) {
            if(target->access == 1 && target->shouldrespawn){
                addEvent(makeTask(target->respawntime*1000, std::bind2nd(std::mem_fun(&Map::RespawnCreature), target)));
            } 
            if(player && target->access == 1){
                      player->mled += (int)(1);
            }else if(player){
                      player->pled += (int)(1);
            }                
			if(tv->second.stackpos < 10) {
				if(tileBodyCountMap.find(targettile) == tileBodyCountMap.end())
					tileBodyCountMap[targettile] = 1;
				else {
					tileBodyCountMap[targettile]++;
				}
			}			
			
			std::cout << "Creature " << target->getName() << " died." << std::endl;
			
			playersOnline.erase(playersOnline.find(target->getID()));
			targettile->removeThing(target);
            if(player && target->access == 0 && targetPlayer && targetPlayer->pking == 0){
                  player->time += (long)g_config.getGlobalNumber("pktime", 60);
            }     
            if(player && (target->access == 1 || (targetPlayer && targetPlayer->pking == 1))){
                                 std::vector<long> creatureList = target->getInflicatedDamageCreatureList();
                                 for(std::vector<long>::const_iterator iitt = creatureList.begin(); iitt != creatureList.end(); ++iitt) 
                                 {
                                     Creature* c = getCreatureByID(*iitt);
                                     if(c)
                                     {       
                                         int addexp = (int)(target->getGainedExperience(c));
                                         int cashToGain = random_range(0,(addexp/10));
                                         c->experience += addexp;
                                         Player* p = dynamic_cast<Player*>(c);
                                         if(p){
                                             c->givenxp = (c->experience*0.05);  
                                             p->cash += cashToGain;  
                                             if(targetPlayer)
                                                targetPlayer->cash -= cashToGain;
                                             NetworkMessage msgi;
                                             msgi.AddPlayerStats(p);
                                             p->sendNetworkMessage(&msgi);
                                         }
                                         std::vector<Creature*> liste;
		                                 getSpectators(Range(c->pos), liste);

		                                 for(int i = 0; i < liste.size(); ++i)
		                                 {
                                             Player* spectatore = dynamic_cast<Player*>(liste[i]);
                                             if(spectatore && addexp != 0){
                                             NetworkMessage msgo;
			                                 std::stringstream exp;
                                             exp << addexp;
                                             msgo.AddAnimatedText(c->pos, 983, exp.str());
                                             spectatore->sendNetworkMessage(&msgo);
                                             }
		                                 }  
                                     }      
                                 }
            }                
			Item *item = new Item(target->lookcorpse);
			target->dropLoot(item);
			item->pos = target->pos;
			targettile->addThing(item);

			unsigned short decayTime = Item::items[item->getID()].decayTime;
			addEvent(makeTask(decayTime*1000, std::bind2nd(std::mem_fun(&Map::decayItem), item)));
		}
	}

	std::vector<Tile*> tileBloodVec; //keep track of the blood splashes (only 1 msg / tile / spectator 
	std::vector<Tile*> tileUpdatedVec;

	NetworkMessage msg;
	//Create a network message for each spectator
	for(int i = 0; i < spectatorlist.size(); ++i) {
		Player* spectator = dynamic_cast<Player*>(spectatorlist[i]);
		
		if(!spectator)
			continue;

		msg.Reset();
		tileBloodVec.clear();
		tileUpdatedVec.clear();

		if((!magicRune || targetvec.size() > 0) && me->animationEffect > 0 && (spectator->CanSee(creature->pos.x, creature->pos.y) || spectator->CanSee(me->centerpos.x, me->centerpos.y)))
			msg.AddDistanceShoot(creature->pos, me->centerpos, me->animationEffect);
        
		for(tl = tilelist.begin(); tl != tilelist.end(); tl++) {
			Tile *targettile = getTile(tl->pos.x,  tl->pos.y, tl->pos.z);

			if(!targettile)
				continue;

			/*if(tl->thingCount > 9 && tileBodyCountMap[targettile] > 0) {
				if(std::find(tileUpdatedVec.begin(), tileUpdatedVec.end(), targettile) == tileUpdatedVec.end()) {
					tileUpdatedVec.push_back(targettile);
					spectatorlist[i]->onTileUpdated(&tl->pos);
				}

#if __DEBUG__
				std::cout << "Pop-up item" << std::endl;
#endif
			}*/

			if(magicGround && spectator->CanSee(tl->pos.x, tl->pos.y)) {
				if(std::find(tileUpdatedVec.begin(), tileUpdatedVec.end(), targettile) == tileUpdatedVec.end()) {

					msg.AddByte(0x6a);
					msg.AddPosition(tl->pos);
					Item item = Item(magicGround->groundID);
					msg.AddItem(&item);
				}

				if(magicGround->maxDamage == 0) {
					continue;
				}
			}
			if(tl->targetCount == 0 && spectator->CanSee(tl->pos.x, tl->pos.y)) {
				if(magicArea && (creature->access > 1 || !targettile->isPz() || (targettile->isPz() && !me->offensive))) {
					if(magicArea->areaEffect != 0xFF)
						msg.AddMagicEffect(tl->pos, magicArea->areaEffect);
				}
				else if(magicRune){
				    player->sendCancel("Only on creatures.");
					msg.AddMagicEffect(creature->pos, NM_ME_PUFF);
					player->sendNetworkMessage(&msg);
					return false;
                }
            }
        }                          

		for(tv = targetvec.begin(); tv != targetvec.end(); tv++) {
			Creature *target = tv->first;
			Tile *targettile = getTile(target->pos.x, target->pos.y, target->pos.z);
			int damage = tv->second.damage;
			int manaDamage = tv->second.manaDamage;
			unsigned char targetstackpos = tv->second.stackpos;
			bool hadSplash = tv->second.hadSplash;

            if(!drinked && magicRune && me->animationEffect == NM_ANI_FIRE && me->animationcolor == 111){
                    drinked = true;
                    creatureDrinkMana(target);
            }
            
			if(spectator->CanSee(target->pos.x, target->pos.y))
			{
				if(me->physical && damage > 0)
					msg.AddMagicEffect(target->pos, NM_ME_DRAW_BLOOD);
					
				msg.AddMagicEffect(target->pos, me->damageEffect);

				if(damage != 0) {   
					std::stringstream dmg;
					dmg << std::abs(damage);
					if(me->physical)
					   msg.AddAnimatedText(target->pos, target->damageColor, dmg.str());
					else
					   msg.AddAnimatedText(target->pos, me->animationcolor, dmg.str());
				}

				if(manaDamage > 0){
					msg.AddMagicEffect(target->pos, NM_ME_LOOSE_ENERGY);
					std::stringstream manaDmg;
					manaDmg << std::abs(manaDamage);
					msg.AddAnimatedText(target->pos, 2, manaDmg.str());
				}

				if (target->health <= 0)
				{                 
					if(std::find(tileUpdatedVec.begin(), tileUpdatedVec.end(), targettile) == tileUpdatedVec.end()) {
#if __DEBUG__
						std::cout << "Remove character " << "targetstackpos: "<< (int) targetstackpos << std::endl;
#endif
						if(targetstackpos < 10) {
							//Remove character
							msg.AddByte(0x6c);
							msg.AddPosition(target->pos);
							msg.AddByte(targetstackpos);
							msg.AddByte(0x6a);
							msg.AddPosition(target->pos);
							Item item = Item(target->lookcorpse);
							msg.AddItem(&item);
						}
					}
				}
				else {
					msg.AddCreatureHealth(target);
        }

				if(me->physical && damage > 0 && target->damageColor != GRAY)
				{
					if(std::find(tileUpdatedVec.begin(), tileUpdatedVec.end(), targettile) == tileUpdatedVec.end()) {
						if(std::find(tileBloodVec.begin(), tileBloodVec.end(), targettile) == tileBloodVec.end()) {
							tileBloodVec.push_back(targettile);

							if (hadSplash)
							{
								msg.AddByte(0x6c);
								msg.AddPosition(target->pos);
								msg.AddByte(1);
								targettile->splash->setID(1437);
							}

							msg.AddByte(0x6a);
							msg.AddPosition(target->pos);
							Item item;
							if(target->health <= 0){
                               if(target->damageColor == GREEN)
                                  item = Item(1434, 4);
                               else
                                  item = Item(1434, 2);                  
                            }else{
							   if(target->damageColor == GREEN)
                                  item = Item(1437, 4);
                               else
                                  item = Item(1437, 2);
                            }          
							msg.AddItem(&item);
						}
					}
				}

				if (spectator == target){
					CreateManaDamageUpdate(target, creature, manaDamage, msg);
					CreateDamageUpdate(target, creature, damage, msg);
				}
			}
		}

		spectator->sendNetworkMessage(&msg);
	}

	return true;
}

void Map::creatureCastSpell(Creature *creature, const MagicEffectClass& me) {
  SURVIVALSYS_THREAD_LOCK(mapLock)

	creatureMakeMagic(creature, &me);

	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


bool Map::canThrowItemTo(Position from, Position to, bool creaturesBlock /* = true*/, bool isProjectile /*= false*/)
{
	if(from.x > to.x) {
		swap(from.x, to.x);
		swap(from.y, to.y);
	}

	bool steep = std::abs(to.y - from.y) > abs(to.x - from.x);

	if(steep) {
		swap(from.x, from.y);
		swap(to.x, to.y);
	}
	
	int deltax = abs(to.x - from.x);
	int deltay = abs(to.y - from.y);
	int error = 0;
	int deltaerr = deltay;
	int y = from.y;
	Tile *t = NULL;
	int xstep = ((from.x < to.x) ? 1 : -1);
	int ystep = ((from.y < to.y) ? 1 : -1);

	for(int x = from.x; x != to.x; x += xstep) {
		t = getTile((steep ? y : x), (steep ? x : y), from.z);

		if(t) {
			if(isProjectile) {
				if(t->isBlockingProjectile())
					return false;
			}
			else if(creaturesBlock && !t->creatures.empty())
				return false;
			else if((from.x != x && from.y != y) && t->isBlocking())
				return false;
		}

		error += deltaerr;

		if(2 * error >= deltax) {
			y += ystep;
			error -= deltax;
		}
	}

	return true;
}

bool Map::creatureThrowRune(Creature *creature, const MagicEffectClass& me) {
  SURVIVALSYS_THREAD_LOCK(mapLock)

	bool ret = false;
    Player* player = dynamic_cast<Player*>(creature);
    
	if(creature->pos.z != me.centerpos.z){
        if(player)               
		  creature->sendCancel("You need to be on the same floor.");
	}
	else if(!canThrowItemTo(creature->pos, me.centerpos, false, true)) {
        if(player) 
		  creature->sendCancel("You cannot throw there.");
	}
	else
		ret = creatureMakeMagic(creature, &me);

	SURVIVALSYS_THREAD_UNLOCK(mapLock)

	return ret;
}

bool Map::creatureOnPrepareAttack(Creature *creature, Position pos)
{
    if(creature){ 
	Player* player = dynamic_cast<Player*>(creature);

	Tile* tile = (Tile*)getTile(creature->pos.x, creature->pos.y, creature->pos.z);
	Tile* targettile = getTile(pos.x, pos.y, pos.z);

	if(creature->access == 0 || creature->access == 1) {
		if(tile && tile->isPz()) {
			if(player) {
				NetworkMessage msg;
				msg.AddTextMessage(MSG_SMALLINFO, "You may not attack a person while your in a protection zone.");
				player->sendNetworkMessage(&msg);
				player->sendCancelAttacking();
			}

			return false;
		}
		else if(targettile && targettile->isPz()) {
			if(player) {
				NetworkMessage msg;
				msg.AddTextMessage(MSG_SMALLINFO, "You may not attack a person in a protection zone.");
				player->sendNetworkMessage(&msg);
				player->sendCancelAttacking();
			}

			return false;
		}
	}

	return true;
    }
    return false;
}

void Map::creatureMakeDamage(Creature *creature, Creature *attackedCreature, fight_t damagetype)
{
	if(!creatureOnPrepareAttack(creature, attackedCreature->pos))
		return;
    
    if(!creature || creature->health <= 0 || !attackedCreature || attackedCreature->health <= 0)
       return;
       	
	Player* player = dynamic_cast<Player*>(creature);
	Player* attackedPlayer = dynamic_cast<Player*>(attackedCreature);

	Tile* tile = getTile(creature->pos.x, creature->pos.y, creature->pos.z);
	Tile* targettile = getTile(attackedCreature->pos.x, attackedCreature->pos.y, attackedCreature->pos.z);

	NetworkMessage msg;
	bool inReach = false;
	switch(damagetype){
		case FIGHT_MELEE:
			if((std::abs(creature->pos.x-attackedCreature->pos.x) <= 1) &&
				(std::abs(creature->pos.y-attackedCreature->pos.y) <= 1) &&
				(creature->pos.z == attackedCreature->pos.z))
					inReach = true;
		break;
		case FIGHT_DIST:
			if((std::abs(creature->pos.x-attackedCreature->pos.x) <= 8) &&
				(std::abs(creature->pos.y-attackedCreature->pos.y) <= 5) &&
				(creature->pos.z == attackedCreature->pos.z))
				  if(canThrowItemTo(creature->pos, attackedCreature->pos, false, true))
					inReach = true;
		break;
	}	
					
	if (player && (player->access == 0 || player->access == 1)) {
	    player->inFightTicks = (long)g_config.getGlobalNumber("pzlocked", 0);
	    player->sendIcons();
	    if(attackedPlayer)
 	         player->pzLocked = true;	    
	}
	if(attackedPlayer && (attackedPlayer->access == 0 || attackedPlayer->access == 1)){
	 attackedPlayer->inFightTicks = (long)g_config.getGlobalNumber("pzlocked", 0);
	 attackedPlayer->sendIcons();
  }
    if(attackedCreature->access > 1 || (PVP_MODE == 1 && creature->access == 1 && attackedCreature->access == 1) || (PVP_MODE == 2 && ((attackedCreature->access == 1 && creature->access == 1) || (creature->access == 0 && attackedCreature->access != 1)))){
        if(player){
          player->sendCancelAttacking();
          if(player->followMode == 1)
            player->followedCreature = 0;
        }
        return;
    }     
	if(!inReach){            
		return;
    }
        
    int damage = 0;
       int atk = 0, arm = 0, def = 0;
       atk    = creature->getWeaponDamage();
       arm    = attackedCreature->getArm();
       if(damagetype != FIGHT_DIST)
          def    = attackedCreature->getShieldDef();      
       damage = atk - (arm+def);
    if(attackedCreature->defenses[3] == true)
       damage = 0;
        
	int manaDamage = 0;

	if (damage < 0 || attackedCreature->access > 1)
		damage = 0;
		
	if(attackedCreature->access == 0 && creature->access == 0)	                       
          damage = damage / 2;
	
    if(damage > attackedCreature->health)
       damage = attackedCreature->health;
       	
	if (attackedCreature->manaShieldTicks <1000 && damage > 0)
		attackedCreature->drainHealth(damage);
	else if (attackedCreature->manaShieldTicks >= 1000 && damage < attackedCreature->mana){
         manaDamage = damage;
         damage = 0;
         attackedCreature->drainMana(manaDamage);
         }
    else if(attackedCreature->manaShieldTicks >= 1000 && damage > attackedCreature->mana){
         manaDamage = attackedCreature->mana;
         damage -= manaDamage;
         attackedCreature->drainHealth(damage);
         attackedCreature->drainMana(manaDamage);
         }
	else
		attackedCreature->health += min(-damage, attackedCreature->healthmax - attackedCreature->health);         
	
    std::vector<Creature*> list;
	getSpectators(Range(min(creature->pos.x, attackedCreature->pos.x) - 9,
										  max(creature->pos.x, attackedCreature->pos.x) + 9,
											min(creature->pos.y, attackedCreature->pos.y) - 7,
											max(creature->pos.y, attackedCreature->pos.y) + 7, creature->pos.z), list);

	for(int i = 0; i < list.size(); ++i)
	{
		Player* p = dynamic_cast<Player*>(list[i]);

		if (p) {
			msg.Reset();
			if(damagetype == FIGHT_DIST)
				msg.AddDistanceShoot(creature->pos, attackedCreature->pos, creature->getSubFightType());
			if (attackedCreature->manaShieldTicks < 1000 && (damage == 0) && (p->CanSee(attackedCreature->pos.x, attackedCreature->pos.y))) {
				msg.AddMagicEffect(attackedCreature->pos, NM_ME_PUFF);
			}
			else if (attackedCreature->manaShieldTicks < 1000 && (damage < 0) && (p->CanSee(attackedCreature->pos.x, attackedCreature->pos.y)))
			{
				msg.AddMagicEffect(attackedCreature->pos, NM_ME_BLOCKHIT);
			}
			else
			{ 
				if (p->CanSee(attackedCreature->pos.x, attackedCreature->pos.y))
				{
					std::stringstream dmg, manaDmg;
					dmg << std::abs(damage);
					manaDmg << std::abs(manaDamage);
					
					if(damage > 0){
					msg.AddAnimatedText(attackedCreature->pos, attackedCreature->damageColor, dmg.str());
					msg.AddMagicEffect(attackedCreature->pos, NM_ME_DRAW_BLOOD);
					}
                    
					if(manaDamage >0){
					  std::stringstream manaDmg;
					  manaDmg << std::abs(manaDamage);
					  msg.AddAnimatedText(attackedCreature->pos, 2, manaDmg.str());
					}

					if (attackedCreature->health <= 0)
					{                                        
						// remove character
						msg.AddByte(0x6c);
						msg.AddPosition(attackedCreature->pos);
						msg.AddByte(targettile->getThingStackPos(attackedCreature));
						msg.AddByte(0x6a);
						msg.AddPosition(attackedCreature->pos);
						Item item = Item(attackedCreature->lookcorpse);
						msg.AddItem(&item);
					}
					else
					{
						msg.AddCreatureHealth(attackedCreature);
					}
					if(damage > 0 && attackedCreature->damageColor != GRAY){				
					// fresh blood, first remove od
					if (targettile->splash)
					{
						msg.AddByte(0x6c);
						msg.AddPosition(attackedCreature->pos);
						msg.AddByte(1);
						targettile->splash->setID(1437);
					}
					msg.AddByte(0x6a);
					msg.AddPosition(attackedCreature->pos);
					Item item;
					if(attackedCreature->health <= 0){
                        if(attackedCreature->damageColor == GREEN)
                           item = Item(1434, 4);
                        else
                           item = Item(1434, 2);                  
                    }else{
					    if(attackedCreature->damageColor == GREEN)
                           item = Item(1437, 4);
                        else
                           item = Item(1437, 2);
                    }       
					msg.AddItem(&item);
					}
				}
			}

			if (p == attackedCreature){
				CreateManaDamageUpdate(p, creature, manaDamage, msg);
				CreateDamageUpdate(p, creature, damage, msg);
			}
	
			p->sendNetworkMessage(&msg);
		}
	}
	
	if(damage > 0 && attackedCreature->damageColor != GRAY){
		if (!targettile->splash)
		{
			Item *item = NULL;
			if(attackedCreature->health <= 0){
               if(attackedCreature->damageColor == GREEN)
                  item = new Item(1434, 4);
               else
                  item = new Item(1434, 2);                  
            }else{
			   if(attackedCreature->damageColor == GREEN)
                  item = new Item(1437, 4);
               else
                  item = new Item(1437, 2);
            }
			item->pos = attackedCreature->pos;
			targettile->splash = item;
		}
        
		unsigned short decayTime;
		if(attackedCreature->health <= 0)
           decayTime = Item::items[1434].decayTime;
        else
           decayTime = Item::items[1437].decayTime;
		targettile->decaySplashAfter = SURVIVALSYS_TIME() + decayTime*1000;
		addEvent(makeTask(decayTime*1000, std::bind2nd(std::mem_fun(&Map::decaySplash), targettile->splash)));
	}
   if(player && (damage > 0 || manaDamage >0)){
        player->addSkillTry(10);
        }
   else if(player)
   player->addSkillTry(5);
   
   if(attackedPlayer && (damage > 0 || manaDamage >0))
        attackedPlayer->addSkillShieldTry(40);
   else if(attackedPlayer)
        attackedPlayer->addSkillShieldTry(20);
        
    if(player && (attackedCreature->access == 1 ||(attackedPlayer && attackedPlayer->pking == 1)) && attackedCreature != creature && damage > 0){
                attackedCreature->addInflictedDamage(creature, damage);
    }                      
	if (attackedCreature->health <= 0) {
        if(attackedCreature->access == 1 && attackedCreature->shouldrespawn){
                addEvent(makeTask(attackedCreature->respawntime*1000, std::bind2nd(std::mem_fun(&Map::RespawnCreature), attackedCreature)));
        }                 
            if(player && attackedCreature->access == 1){
                      player->mled += (int)(1);
            }else if(player){
                      player->pled += (int)(1);
            }
            
        std::cout << "Creature " << attackedCreature->getName() << " died." << std::endl;    
               
		targettile->removeThing(attackedCreature);
		playersOnline.erase(playersOnline.find(attackedCreature->getID()));
		Player* targetPlayer = dynamic_cast<Player*>(attackedCreature);
		if(player && attackedCreature->access == 0 && targetPlayer && targetPlayer->pking == 0){
                  player->time += (long)g_config.getGlobalNumber("pktime", 60);
            }
        if(player && (attackedCreature->access == 1 ||(targetPlayer && targetPlayer->pking == 1))){          
                                 std::vector<long> creatureList = attackedCreature->getInflicatedDamageCreatureList();
                                 for(std::vector<long>::const_iterator iitt = creatureList.begin(); iitt != creatureList.end(); ++iitt) 
                                 {
                                     Creature* c = getCreatureByID(*iitt);
                                     if(c)
                                     {       
                                         int addexp = (int)(attackedCreature->getGainedExperience(c));            
                                         int cashToGain = random_range(0,(addexp/10));
                                         c->experience += addexp;
                                         Player* p = dynamic_cast<Player*>(c);
                                         if(p){
                                             c->givenxp = (c->experience*0.05);
                                             p->cash += cashToGain;
                                             if(targetPlayer)
                                                targetPlayer -= cashToGain;    
                                             NetworkMessage msgi;
                                             msgi.AddPlayerStats(p);
                                             p->sendNetworkMessage(&msgi);
                                         }
                                         std::vector<Creature*> liste;
		                                 getSpectators(Range(c->pos), liste);

		                                 for(int i = 0; i < liste.size(); ++i)
		                                 {
                                             Player* spectatore = dynamic_cast<Player*>(liste[i]);
                                             if(spectatore && addexp != 0){
                                             NetworkMessage msgo;
			                                 std::stringstream exp;
                                             exp << addexp;
                                             msgo.AddAnimatedText(c->pos, 983, exp.str());
                                             spectatore->sendNetworkMessage(&msgo);
                                             }
		                                 }    
                                     }         
                                 }          
       }  
    Item *item = new Item(attackedCreature->lookcorpse);
    attackedCreature->dropLoot(item);
    item->pos = attackedCreature->pos;
		targettile->addThing(item);

		unsigned short decayTime = Item::items[item->getID()].decayTime;
    addEvent(makeTask(decayTime*1000, std::bind2nd(std::mem_fun(&Map::decayItem), item)));
	}
}


std::list<Position> Map::getPathTo(Position start, Position to, bool creaturesBlock /* = true*/){
	std::list<Position> path;
	std::list<AStarNode*> openNodes;
	std::list<AStarNode*> closedNodes;
	int z = start.z;

	AStarNode* startNode = new AStarNode;
	startNode->parent=NULL;
	startNode->h=0;
	startNode->x=start.x;
	startNode->y=start.y;
	AStarNode* found = NULL;
	openNodes.push_back(startNode);
	while(!found && closedNodes.size() < 100){
		//get best node from open list
		openNodes.sort(lessPointer<AStarNode>());
		int size = openNodes.size();
		if(size == 0)
			return path; //no path
		AStarNode* current = openNodes.front();
		openNodes.pop_front();
		closedNodes.push_back(current);
		for(int dx=-1; dx <= 1; dx++){
			for(int dy=-1; dy <= 1; dy++){
				if(abs(dx) != abs(dy)){
					int x = current->x + dx;
					int y = current->y + dy;

					Tile *t = getTile(x, y, z);
					
                      if((!t || t->isBlocking()) && t->pos != start && t->pos != to)
						 continue;
                      if((!t || t->isBlockingWalk()) && t->pos != start && t->pos != to)
						 continue;       
                    
                    if(creaturesBlock){
                      if(t->creatures.begin() != t->creatures.end() && (*t->creatures.begin()) != NULL){                            
                         if(t->getCreature() && t->getCreature()->pos != start && t->getCreature()->pos != to){                     
                            continue;
                         }
                      }                     
                    }    
                                	
					bool isInClosed = false;
					for(std::list<AStarNode*>::iterator it = closedNodes.begin();
						it != closedNodes.end(); it++){
						AStarNode* n = *it;
						if(n->x == x && n->y == y){
							isInClosed = true;
							break;
						}
					}
					if(isInClosed)
						continue;

					bool isInOpen = false;
					AStarNode* child = NULL;
					for(std::list<AStarNode*>::iterator it = openNodes.begin();
						it != openNodes.end(); it++){
						AStarNode* n = *it;
						if(n->x == x && n->y == y){
							isInOpen = true;
							child = *it;
							break;
						}
					}

					if(!isInOpen){
						AStarNode* n = new AStarNode;
						n->x=x;
						n->y=y;
						n->h = (float)abs(n->x - to.x) + (float)abs(n->y - to.y);
						n->g = current->g + 1;
						n->parent = current;
						if(n->x == to.x && n->y == to.y){
							found = n;
						}
						openNodes.push_front(n);
					}
/*					else{
						if(current->g + 1 < child->g)
							child->parent = current;
							child->g=current->g+1;
					}*/
				}
			}
		}                   
	}
	//cleanup the mess
	while(found){
		Position p;
		p.x = found->x;
		p.y = found->y;
		p.z = z;
		path.push_front(p);
		found = found->parent;
	}

	for(std::list<AStarNode*>::iterator it = openNodes.begin();
		it != openNodes.end(); it++){
		delete *it;
	}

	for(std::list<AStarNode*>::iterator it = closedNodes.begin();
		it != closedNodes.end(); it++){
		delete *it;
	}
	
	/*for(std::list<Position>::iterator it = path.begin(); it != path.end(); it++){
		Position p = *it;
	}*/
	return path;
}



void Map::checkPlayer(unsigned long id)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)
  Creature *creature = getCreatureByID(id);

  if (creature != NULL)
  {

	 Player* player = dynamic_cast<Player*>(creature);
	 if(player){
           
		 addEvent(makeTask(1000, std::bind2nd(std::mem_fun(&Map::checkPlayer), id)));
		 
		 player->sendLight(lighttick, lightcolor); 
		 
		 if(player->time > 0 && player->pking == 0){   
                   pkTime(player);    
                   pkPlayer(creature);
                   player->pking = 1;
         }
         bool updateLife = false;
         if(player->voc == 1 || player->voc == 2){
            if(player->timeMana >= 1){
               if(player->health != player->healthmax)
                  updateLife = true;                             
		       player->mana += min(4, player->manamax - player->mana);
		       player->timeMana = 0;
            }   
		    if(player->timeLife >= 1){
               if(player->health != player->healthmax)
                  updateLife = true;                 
		       player->health += min(1, player->healthmax - player->health);
		       player->timeLife = 0;
            }   
		 }else if(player->voc == 3){
            if(player->timeMana >= 1){
               if(player->health != player->healthmax)
                  updateLife = true;                             
		       player->mana += min(2, player->manamax - player->mana);
		       player->timeMana = 0;
            }   
		    if(player->timeLife >= 1){
		       player->health += min(2, player->healthmax - player->health);
		       player->timeLife = 0;
            }
		 }else{
            if(player->timeMana >= 1){
               if(player->health != player->healthmax)
                  updateLife = true;                             
		       player->mana += min(1, player->manamax - player->mana);
		       player->timeMana = 0;
            }   
		    if(player->timeLife >= 1){
               if(player->health != player->healthmax)
                  updateLife = true;                 
		       player->health += min(4, player->healthmax - player->health);
		       player->timeLife = 0;
            }                 
         }
         
         if(updateLife){   
		    std::vector<Creature*> list;
	        getSpectators(Range(player->pos, true), list);

	        for(int i = 0; i < list.size(); ++i)
	        {
                Player *p = dynamic_cast<Player*>(list[i]);
                if(p){    
		           NetworkMessage msg;
	               msg.AddCreatureHealth(player);
	               p->sendNetworkMessage(&msg);
                }   
	        }
         }   
	    
		 NetworkMessage msg;
		 int requiredExp = player->getExpForLv(player->level+1);
		 
		 if (player->experience >= requiredExp)
         {
          int lastLv = player->level;

          player->level += 1;
          player->healthmax = player->healthmax+player->HPGain[player->voc];
          player->health = player->health+player->HPGain[player->voc];
          player->manamax = player->manamax+player->ManaGain[player->voc];
          player->mana = player->mana+player->ManaGain[player->voc];
          player->cap = player->cap+player->CapGain[player->voc];
          player->setNormalSpeed();
          changeSpeed(player->getID(), player->getSpeed());
          std::stringstream lvMsg;
          lvMsg << "You advanced from level " << lastLv << " to level " << player->level << ".";
          msg.AddTextMessage(MSG_ADVANCE, lvMsg.str().c_str());
         }
		 
		 msg.AddPlayerStats(player);
		 msg.AddCreatureHealth(creature);
		 player->sendNetworkMessage(&msg);


         //Magic Level Advance
         unsigned int reqMana = player->getReqMana(player->maglevel+1, player->voc);
         //ATTANTION: MAKE SURE THAT CHARACTERS HAVE REASONABLE MAGIC LEVELS. ESPECIALY KNIGHTS!!!!!!!!!!!

         if (reqMana % 20 < 10)                                  //CIP must have been bored when they invented this odd rounding
              reqMana = reqMana - (reqMana % 20);
         else reqMana = reqMana - (reqMana % 20) + 20;


         if (player->manaspent >= reqMana) {
            player->manaspent -= reqMana;
            player->maglevel++;
            
            std::stringstream MaglvMsg;
            MaglvMsg << "You advanced from magic level " << (player->maglevel - 1) << " to magic level " << player->maglevel << ".";
            msg.AddTextMessage(MSG_ADVANCE, MaglvMsg.str().c_str());
            
            msg.AddPlayerStats(player);
            player->sendNetworkMessage(&msg);
         }
         //End Magic Level Advance

		 if(player->inFightTicks >= 1000) {
			player->inFightTicks -= 1000;
            if(player->inFightTicks < 1000)
				player->pzLocked = false;
                player->sendIcons(); 
          }
          if(player->exhaustedTicks >=1000){
            player->exhaustedTicks -=1000;
            } 
          if(player->manaShieldTicks >=1000){
            player->manaShieldTicks -=1000;
            if(player->manaShieldTicks  < 1000)
              player->sendIcons();
          }
          if(player->hasteTicks >=1000){
            player->hasteTicks -=1000;
            if(player->hasteTicks < 1000)
              player->setNormalSpeed();
              changeSpeed(player->getID(), player->getSpeed());
              player->sendIcons();
          }
          if(player->lightTicks >= 1000){
             player->lightTicks-= 1000;
             int light = 0;
             if(player->lightTicks == 48000)
                light = 8;
             else if(player->lightTicks == 420000) 
                light = 7;
             else if(player->lightTicks == 360000) 
                light = 6;
             else if(player->lightTicks == 300000) 
                light = 5;
             else if(player->lightTicks == 240000) 
                light = 4;
             else if(player->lightTicks == 180000) 
                light = 3;
             else if(player->lightTicks == 120000) 
                light = 2;
             else if(player->lightTicks == 60000) 
                light = 1;
             if(light > 0){
               player->lightLevel = light;                        
               std::vector<Creature*> liste;
		       getSpectators(Range(player->pos), liste);
		       for(int i = 0; i < liste.size(); ++i)
		       {
                   Player* spectator = dynamic_cast<Player*>(liste[i]);
                   if(spectator){
                        NetworkMessage msg;
                        msg.AddLight(player);
                        spectator->sendNetworkMessage(&msg);
                   }
		       }
             }                     
          }                      
          player->timeMana += 1;
          player->timeLife += 1;    
	 }
	 else{
         creature->onThink(); 
		 addEvent(makeTask(300, std::bind2nd(std::mem_fun(&Map::checkPlayer), id)));
		 if(creature->manaShieldTicks >=1000){
         creature->manaShieldTicks -=300;
         }
         if(creature->hasteTicks >=1000){
            creature->hasteTicks -=300;
            }  
	 }
  }
   SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::changeOutfit(unsigned long id, int looktype){
     SURVIVALSYS_THREAD_LOCK(mapLock)
     
     Creature *creature = getCreatureByID(id);
     if(creature){
     creature->looktype = looktype;
     creatureChangeOutfit(creature);
     }
     
     SURVIVALSYS_THREAD_UNLOCK(mapLock)
     }

void Map::changeOutfitAfter(unsigned long id, int looktype, long time){

     addEvent(makeTask(time, 
     boost::bind(
     &Map::changeOutfit, this,
     id, looktype)));
     
}

void Map::changeSpeed(unsigned long id, unsigned short speed)
{
	Creature *creature = getCreatureByID(id);
	if(creature){
		if(creature->hasteTicks >= 1000 || creature->speed == speed){
			return;
		}
	
		creature->speed = speed;
		Player* player = dynamic_cast<Player*>(creature);
		if(player){
			player->sendChangeSpeed(creature);
			player->sendIcons();
		}

		std::vector<Creature*> list;
		getSpectators(Range(creature->pos), list);
		
		for(int i = 0; i < list.size(); i++)
		{
			Player* p = dynamic_cast<Player*>(list[i]);
			if(p)
				p->sendChangeSpeed(creature);
		}
	}
}


void Map::checkPlayerAttacking(unsigned long id)
{
	SURVIVALSYS_THREAD_LOCK(mapLock)

  Creature *creature = getCreatureByID(id);
  if (creature != NULL && creature->health > 0)
  {
    Npc *npc = dynamic_cast<Npc*>(creature);
	if (npc){
            npc->onAttack();
	}
    else {           
    addEvent(makeTask(2000, std::bind2nd(std::mem_fun(&Map::checkPlayerAttacking), id)));

	  if (creature->attackedCreature != 0)
    {
      Creature *attackedCreature = getCreatureByID(creature->attackedCreature);
      if (attackedCreature)
      {
          Player* player = dynamic_cast<Player*>(creature);                 
          if(player && player->followMode != 1 && player->followedCreature != 0){
              player->followedCreature = 0;
          }else if(player && player->followMode == 1 && player->followedCreature != id){
              player->followedCreature = attackedCreature->getID();
         }    
                              
	      Tile* fromtile = getTile(creature->pos.x, creature->pos.y, creature->pos.z);
        if (!attackedCreature->isAttackable() == 0 && fromtile->isPz() && (creature->access == 0 || creature->access == 1))
        {
          if (player) {
	          NetworkMessage msg;
            msg.AddTextMessage(MSG_STATUS, "You may not attack a person in a protection zone.");
            player->sendNetworkMessage(&msg);
            player->sendCancelAttacking();
          }
        }
        else
        {
          if (attackedCreature != NULL && attackedCreature->health > 0)
          {
	          this->creatureMakeDamage(creature, attackedCreature, creature->getFightType());
          }
        }
      }
	  }
   }
	}

   SURVIVALSYS_THREAD_UNLOCK(mapLock)
}

void Map::decayItem(Item* item)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)

	if(item) {
		Tile *t = getTile(item->pos.x, item->pos.y, item->pos.z);
		unsigned short decayTo   = Item::items[item->getID()].decayTo;
		unsigned short decayTime = Item::items[item->getID()].decayTime;

		if (decayTo == 0)
		{
            if(item->isContainer())
			   item->closePlayersContainer();        
			t->removeThing(item);
		}
		else
		{
            if(item->isContainer())
			   item->closePlayersContainer();
			item->setID(decayTo);
			addEvent(makeTask(decayTime*1000, std::bind2nd(std::mem_fun(&Map::decayItem), item)));
		}

		creatureBroadcastTileUpdated(item->pos);

		if (decayTo == 0)
			delete item;
	}
  
	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


void Map::decaySplash(Item* item)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)

	if (item) {
		Tile *t = getTile(item->pos.x, item->pos.y, item->pos.z);

		if ((t) && (t->decaySplashAfter <= SURVIVALSYS_TIME()))
		{
			unsigned short decayTo   = Item::items[item->getID()].decayTo;
			unsigned short decayTime = Item::items[item->getID()].decayTime;

			if (decayTo == 0)
			{
				t->splash = NULL;
			}
			else
			{
				item->setID(decayTo);
				t->decaySplashAfter = SURVIVALSYS_TIME() + decayTime*1000;
				addEvent(makeTask(decayTime*1000, std::bind2nd(std::mem_fun(&Map::decaySplash), item)));
			}
			
			creatureBroadcastTileUpdated(item->pos);

			if (decayTo == 0)
				delete item;
		}
	}
  
	SURVIVALSYS_THREAD_UNLOCK(mapLock)
}


void Map::CreateDamageUpdate(Creature* creature, Creature* attackCreature, int damage, NetworkMessage& msg)
{
			Player* player = dynamic_cast<Player*>(creature);
			if(!player)
				return;
			msg.AddPlayerStats(player);
			if (damage > 0) {
				std::stringstream dmgmesg;

				if(damage == 1)
				dmgmesg << "You lose 1 hitpoint due to an attack by ";
				else
				dmgmesg << "You lose " << damage << " hitpoints due to an attack by ";
				
				dmgmesg << attackCreature->getName();

				msg.AddTextMessage(MSG_EVENT, dmgmesg.str().c_str());
			}
			if (player->health <= 0){
			
                msg.AddTextMessage(MSG_ADVANCE, "You are dead.");             
				msg.AddTextMessage(MSG_EVENT, "Own3d!");
				
				
                //Magic Level downgrade
                /*
                unsigned int sumMana = 0;
                unsigned int lostMana = 0;
                for (int i = 0; i <= player->maglevel; i++) {              //sum up all the mana
                    sumMana += player->getReqMana(i, player->voc);
                }
                
                sumMana += player->manaspent;
                
                lostMana = (int) (sumMana * 0.1);   //player loses 10% of all spent mana when he dies
                
                if (player->manaspent >= lostMana) { //player does not lose a magic level
                   player->manaspent -= lostMana;
                } else {                             //player DOES lose a magic level
                   lostMana -= player->manaspent;
                   player->manaspent = (int) ( player->getReqMana(player->maglevel, player->voc) - lostMana );
                   player->maglevel--;
                }
                */
                //End Magic Level downgrade
                
                
                
                //Skill loss
                /*
                unsigned int lostSkilltries;
                unsigned int sumSkilltries;
                for (int i = 0; i <= 6; i++) {  //for each skill
                    lostSkilltries = 0;         //reset to 0
                    sumSkilltries = 0;
                    
                    for (int c = 11; c <= player->skills[i][SKILL_LEVEL]; c++) {    //sum up all required tries for all skill levels
                        sumSkilltries += player->getReqSkilltries(i, c, player->voc);
                    }
                    
                    sumSkilltries += player->skills[i][SKILL_TRIES];
                    
                    lostSkilltries = (int) (sumSkilltries * 0.1);           //player loses 10% of his skill tries

                    //cout << player->getName() << "died an lost " << lostSkilltries << " skill tries for skill " << i << "\n";
                    //cout << "skill tries before death: " <<  player->skills[i][SKILL_TRIES] << "\n";
                    //cout << "skill level before death: " <<  player->skills[i][SKILL_LEVEL] << "\n";

                    if (player->skills[i][SKILL_TRIES] >= lostSkilltries) { //player does not lose a skill level
                       player->skills[i][SKILL_TRIES] -= lostSkilltries;
                       
                    } else {                                                //player DOES lose a skill level
                       if (player->skills[i][SKILL_LEVEL] > 10 ) {          //skills should not be < 10
                          lostSkilltries -= player->skills[i][SKILL_TRIES];
                          player->skills[i][SKILL_TRIES] = (int) ( player->getReqSkilltries(i, player->skills[i][SKILL_LEVEL], player->voc) - lostSkilltries );
                          player->skills[i][SKILL_LEVEL]--;
                       } else {
                              player->skills[i][SKILL_LEVEL] = 10;
                              player->skills[i][SKILL_TRIES] = 0;
                       }
                    }
                    
                    //cout << "skill tries after death: " <<  player->skills[i][SKILL_TRIES] << "\n";
                    //cout << "skill level after death: " <<  player->skills[i][SKILL_LEVEL] << "\n";
                }
                */                       
                //End Skill loss
                
                
                
                //Level Downgrade                
                int expLoss = (int)(player->experience*0.05);
                int expee = player->experience - expLoss;
                int lev = player->level;
                
                for(int a=0;a<5;a++){
                  int reqExp = player->getExpForLv(lev);
                  if(expee < reqExp){
                    lev -= 1;                    
                    std::stringstream lvMsg;
                    lvMsg << "You were downgraded from level " << lev+1 << " to level " << lev << ".";
                    msg.AddTextMessage(MSG_ADVANCE, lvMsg.str().c_str());
                  }
                }
                //std::string charName = player->getName();
                //player->savePlayer(charName);    
            }
}

void Map::CreateManaDamageUpdate(Creature* creature, Creature* attackCreature, int damage, NetworkMessage& msg)
{
	Player* player = dynamic_cast<Player*>(creature);
	if(!player)
		return;

	msg.AddPlayerStats(player);
	if (damage > 0) {
		std::stringstream dmgmesg;
		dmgmesg << "You lose " << damage << " mana blocking an attack by ";	
		dmgmesg << attackCreature->getName();
		msg.AddTextMessage(MSG_EVENT, dmgmesg.str().c_str());
	}
}

bool Map::creatureSaySpell(Creature *creature, const std::string &text)
{
  SURVIVALSYS_THREAD_LOCK(mapLock)
	bool ret = false;

	Player* player = dynamic_cast<Player*>(creature);
	std::string temp, var;
  unsigned int loc = text.find( "\"", 0 );
  if( loc != string::npos && loc >= 0){
      temp = std::string(text, 0, loc-1);
      var = std::string(text, (loc+1), text.size()-loc-1);
	}
  else {
    temp = text;
    var = std::string(""); 
	}
  
	if(creature->access > 1 || !player){
		std::map<std::string, Spell*>::iterator sit = spells.getAllSpells()->find(temp);
		if(sit != spells.getAllSpells()->end() ) {
				sit->second->getSpellScript()->castSpell(creature, var);
				ret = true;
		}
	}
  else if(player){
		std::map<std::string, Spell*>* tmp = spells.getVocSpells(player->voc);
		if(tmp){
			std::map<std::string, Spell*>::iterator sit = tmp->find(temp);
			if(sit != tmp->end() ) {
				if(player->maglevel >= sit->second->getMagLv()){
					sit->second->getSpellScript()->castSpell(creature, var);
					ret = true;
				}
			}
		}
	}
  if(!ret && player){
     int mlvl;
     int lightlevel;
     int cost;
     long ticks;
     if (strcmp(temp.c_str(), "utevo lux") == 0){ 
       mlvl = 0;
       lightlevel = 5;    
       cost = 20;
       ticks = 300000; 
       ret = true; 
     }else if (strcmp(temp.c_str(), "utevo gran lux") == 0){ 
       mlvl = 3;
       lightlevel = 7;    
       cost = 50;
       ticks = 420000; 
       ret = true; 
     }else if (strcmp(temp.c_str(), "utevo gran vis lux") == 0){ 
       mlvl = 12;
       lightlevel = 8;    
       cost = 150;
       ticks = 480000; 
       ret = true; 
     }
     if(ret && player->maglevel >= mlvl){
        if(player->mana >= cost){                 
               if(player->access != 3){ 
                  player->mana -= cost;
                  player->manaspent += cost;
               }
               player->lightLevel = lightlevel;   
               player->lightTicks = ticks;
               
               NetworkMessage msge;
               msge.AddPlayerInventoryItem(player, 5); 
               player->sendNetworkMessage(&msge);
               
               std::vector<Creature*> liste;
		       getSpectators(Range(player->pos), liste);
		       for(int i = 0; i < liste.size(); ++i)
		       {
                   Player* spectator = dynamic_cast<Player*>(liste[i]);
                   if(spectator){
                        NetworkMessage msg;
                        msg.AddMagicEffect(player->pos, NM_ME_MAGIC_ENERGIE);
                        msg.AddLight(player);
                        spectator->sendNetworkMessage(&msg);
                   }
		       }
               #if __DEBUG__                            
                        std::cout << "Creature say spell" << std::endl;   
               #endif   
        }else{ 
               NetworkMessage msg; 
               msg.AddTextMessage(MSG_SMALLINFO, "You don't have enough mana."); 
               player->sendNetworkMessage(&msg);
        } 
     }else if(ret){
          NetworkMessage msg;
          msg.AddTextMessage(MSG_SMALLINFO, "You don't have enough magic level."); 
          player->sendNetworkMessage(&msg);
     }      
  }        	
  if(!ret && player){
    int cost;
    int mlvl;
    unsigned char x;
    unsigned short runeh; 
    if (strcmp(temp.c_str(), "adura vita") == 0 && player && (player->voc == 2 || player->access == 3)){ 
       mlvl = 8;      
       cost = 100; 
       runeh = 1623;
       x = 3;
       ret = true; 
    } 
    else if(strcmp(temp.c_str(), "adori gran") == 0 && player && (player->voc == 1 || player->voc == 2 || player->voc == 3 || player->access == 3)){ 
       mlvl = 3;
       cost = 70; 
       runeh = 1661;
       x = 5; 
       ret = true; 
    }   
    else if(strcmp(temp.c_str(), "adori gran flam") == 0 && player && (player->voc == 1 || player->voc == 2 || player->access == 3)){
       mlvl = 9;
       cost = 150; 
       runeh = 1654;
       x = 2; 
       ret = true; 
    }
    else if(strcmp(temp.c_str(), "adevo mas hur") == 0 && player && (player->voc == 1 || player->voc == 2 || player->access == 3)){
       mlvl = 12;
       cost = 180; 
       runeh = 1663;
       x = 3; 
       ret = true; 
    }
    else if(strcmp(temp.c_str(), "adori vita vis") == 0 && player && (player->voc == 1 || player->access == 3)){
       mlvl = 15;
       cost = 220; 
       runeh = 1618;
       x = 2; 
       ret = true; 
    }
    else if(strcmp(temp.c_str(), "adura mana") == 0 && player && (player->voc == 1 || player->voc == 2 || player->access == 3)){
       mlvl = 5;
       cost = 100; 
       runeh = 1628;
       x = 1; 
       ret = true; 
    }
    else if(strcmp(temp.c_str(), "adevo grav tera") == 0 && player && (player->voc == 1 || player->voc == 2 || player->access == 3)){
       mlvl = 14;
       cost = 250; 
       runeh = 1643;
       x = 1; 
       ret = true; 
    }
    if(ret) 
    {
       if(player->maglevel >= mlvl){            
        if(player->mana >= cost){               
          if(player->items[5] && player->items[5] != NULL && player->items[5]->getID() == 1610){
               NetworkMessage msg;      
               Item *rune = new Item(runeh, x); 
               player->items[5] = NULL;
               if(player->access != 3){  
                  player->mana -= cost;
                  player->manaspent += cost;
               }   
               player->items[5] = rune;    
               msg.AddPlayerInventoryItem(player, 5); 
               player->sendNetworkMessage(&msg);
               
               std::vector<Creature*> liste;
		       getSpectators(Range(player->pos), liste);
		       for(int i = 0; i < liste.size(); ++i)
		       {
                   Player* spectator = dynamic_cast<Player*>(liste[i]);
                   if(spectator){
                        NetworkMessage msg2;
                        msg2.AddMagicEffect(player->pos, NM_ME_MAGIC_ENERGIE);
                        spectator->sendNetworkMessage(&msg2);
                   }
		       }
               #if __DEBUG__                            
                        std::cout << "Creature say spell" << std::endl;   
               #endif  
          }
          else if(player->items[6] && player->items[6] != NULL && player->items[6]->getID() == 1610){
               NetworkMessage msg;      
               Item *rune = new Item(runeh, x); 
               player->items[6] = NULL;
               if(player->access != 3){ 
                  player->mana -= cost;
                  player->manaspent += cost;
               }   
               player->items[6] = rune;    
               msg.AddPlayerInventoryItem(player, 6); 
               player->sendNetworkMessage(&msg);
               
               std::vector<Creature*> liste;
		       getSpectators(Range(player->pos), liste);
		       for(int i = 0; i < liste.size(); ++i)
		       {
                   Player* spectator = dynamic_cast<Player*>(liste[i]);
                   if(spectator){
                        NetworkMessage msg2;
                        msg2.AddMagicEffect(player->pos, NM_ME_MAGIC_ENERGIE);
                        spectator->sendNetworkMessage(&msg2);
                   }
		       }
               #if __DEBUG__                            
                        std::cout << "Creature say spell" << std::endl;   
               #endif  
          } 
          else{ 
               NetworkMessage msg; 
               msg.AddTextMessage(MSG_SMALLINFO, "You have no rune in your hand!"); 
               player->sendNetworkMessage(&msg);
          } 
       } 
       else if(player->mana < cost){ 
          NetworkMessage msg;
          msg.AddTextMessage(MSG_SMALLINFO, "You don't have enough mana."); 
          player->sendNetworkMessage(&msg);
       }
      }else{
          NetworkMessage msg;
          msg.AddTextMessage(MSG_SMALLINFO, "You don't have enough magic level."); 
          player->sendNetworkMessage(&msg);
      }         
    }
  }  
    
	SURVIVALSYS_THREAD_UNLOCK(mapLock)
	return ret;
}


void Map::creatureDrinkMana(Creature *creature)
{
	if (creature->mana != creature->manamax)
	{
		creature->mana += random_range(50, 100);
	}
	
	if(creature->mana == creature->manamax){
        creature->mana = creature->manamax;
    }
}

void Map::CallCloseHole(Position holepos)
{
	addEvent(makeTask(10000, std::bind2nd(std::mem_fun(&Map::CloseHole), holepos)));
}

void Map::CloseHole(Position holepos)
{
	Tile *holetile = getTile(holepos.x, holepos.y, holepos.z);  
    int groundid = holetile->ground.getID();

	if (groundid == 455)
		holetile->ground.setID(454);
	else if (groundid == 470)
		holetile->ground.setID(469);
	else if (groundid == 468)
		holetile->ground.setID(467);
	else if (groundid == 468)
		holetile->ground.setID(385);
	else if (groundid == 385)
		holetile->ground.setID(371);

	creatureBroadcastTileUpdated(holepos);
}

void Map::closeServer(Creature *lol)
{
  if(lol)   
      creatureBroadcastMessage(lol, "Server down!");          
  std::map<long, Creature*>::iterator i;
  int o = playersOnline.size();
  for(int a=0;a<o; a++){
   for( i = playersOnline.begin(); i != playersOnline.end(); i++ )
   {
      Creature *c = i->second;
      Player* player = dynamic_cast<Player*>(c);
      if(player){
        player->sendKick();
      }else{
        removeCreature(c);
      }  
      break; 
   }
  }
  exit(1);
  exit(0);  
}

// Respawns a creature
void Map::RespawnCreature(Creature *oldcreature)
{
    bool canrespawn = true;     
    Creature* oldnpc = getCreatureByID(oldcreature->getID());
    
    std::vector<Creature*> liste;
    getSpectators(Range(oldcreature->masterPos), liste);
	for(int i = 0; i < liste.size(); ++i)
	{
       Player* spectatore = dynamic_cast<Player*>(liste[i]);
       if(spectatore){
          canrespawn = false;
       }
	}
    
	if (oldcreature->shouldrespawn && !oldnpc && canrespawn)
	{        
		Npc *npc = new Npc(oldcreature->getName().c_str(), (Map *)this);
		
		if(!npc->isLoaded()){
			delete npc;
			return;
		}
		
		#ifdef __DEBUG__ 
        std::cout << "Respawned creature " << npc->getName() << "." << endl;
        #endif

		npc->shouldrespawn = oldcreature->shouldrespawn;
		npc->respawntime = oldcreature->respawntime;
		npc->masterPos = oldcreature->masterPos;
		npc->pos = oldcreature->masterPos;
		npc->dirr = oldcreature->dirr;
        		
		placeCreature(npc);
		npc->setDirection((Direction)npc->dirr);
	}
    else
    {
       addEvent(makeTask(oldcreature->respawntime*1000, std::bind2nd(std::mem_fun(&Map::RespawnCreature), oldcreature)));
    }      
}

Creature* Map::findCreature(Creature *c)
{
  std::vector<Creature*> liste;
  getSpectators(Range(c->pos), liste);
  for(int dist = 0; dist<=9; dist++){
   for(int i = 0; i < liste.size(); ++i)
   {
              Player* spectatore = dynamic_cast<Player*>(liste[i]);
              if(spectatore && spectatore->access == 0 && (((spectatore->pos.x - c->pos.x <= dist && spectatore->pos.x - c->pos.x >= 0) && (spectatore->pos.y - c->pos.y <= dist && spectatore->pos.y - c->pos.y >= 0)) || ((spectatore->pos.x - c->pos.x >= -dist && spectatore->pos.x - c->pos.x <= 0) && (spectatore->pos.y - c->pos.y >= -dist && spectatore->pos.y - c->pos.y <= 0)) || ((spectatore->pos.x - c->pos.x <= dist && spectatore->pos.x - c->pos.x >= 0) && (spectatore->pos.y - c->pos.y >= -dist && spectatore->pos.y - c->pos.y <= 0)) || ((spectatore->pos.x - c->pos.x >= -dist && spectatore->pos.x - c->pos.x <= 0) && (spectatore->pos.y - c->pos.y <= dist && spectatore->pos.y - c->pos.y >= 0)))) {
               Tile *toTile = getTile(spectatore->pos.x, spectatore->pos.y, spectatore->pos.z);
               if(!toTile->isPz())
                  return spectatore;
              } 
   }
  }
  return NULL;
}

void Map::teleportPlayer(Position toPos, Creature *c, bool magic, Tile *toTile, bool ground)
{
		bool teleport = false;
		
		if(ground){
		  ItemVector::iterator iit;
	      for (iit = toTile->topItems.begin(); iit != toTile->topItems.end(); iit++)
	      {
		    if ((*iit)->getID() == 1125 || (*iit)->getID() == 1124 || (*iit)->getID() == 1123 || (*iit)->getID() == 1126 || (*iit)->getID() == 1128 || (*iit)->getID() == 1130 || (*iit)->getID() == 1132){
                teleport = true;
		    }
          }
         	
	      if (toTile->ground.getID() == 421 || toTile->ground.getID() == 1124 || toTile->ground.getID() == 455 || toTile->ground.getID() == 468 || toTile->ground.getID() == 470 || toTile->ground.getID() == 465 || toTile->ground.getID() == 466 || toTile->ground.getID() == 456 || toTile->ground.getID() == 394 || toTile->ground.getID() == 385 || toTile->ground.getID() == 387 || toTile->ground.getID() == 394 || toTile->ground.getID() == 369 || toTile->ground.getID() == 370 || toTile->ground.getID() == 294 || toTile->ground.getID() == 461 || toTile->ground.getID() == 462 || toTile->ground.getID() == 410 || toTile->ground.getID() == 411 || toTile->ground.getID() == 412 || toTile->ground.getID() == 413 || toTile->ground.getID() == 414 || toTile->ground.getID() == 418 || toTile->ground.getID() == 419 || toTile->ground.getID() == 420 || toTile->ground.getID() == 423 ||toTile->ground.getID() == 424 || toTile->ground.getID() == 386){
                teleport = true;
          }
        }else
          teleport = true;
          
	    if(teleport){
	    Player* player = dynamic_cast<Player*>(c); 
        
        #ifdef __DEBUG__       
        std::cout << "Player teleport " << player->getName() << ". x=" << player->pos.x <<  " y=" << player->pos.y << " z=" << player->pos.z << endl;    
		#endif
		
        unsigned short from_x;
		unsigned short from_y;
		unsigned char from_z;

		unsigned short to_x;
		unsigned short to_y;
		unsigned char to_z;
          			
		from_x = player->pos.x;
		from_y = player->pos.y;
		from_z = player->pos.z;
				
		to_x = toPos.x;
		to_y = toPos.y;
		to_z = toPos.z; 
		
		Position oldPos;
		oldPos.x = from_x;
		oldPos.y = from_y;
		oldPos.z = from_z;

		Tile *fromTile = getTile(from_x, from_y, from_z);
		Tile *toTile   = getTile(to_x, to_y, to_z);
		
		int oldstackpos = fromTile->getThingStackPos(player);

		if (fromTile->removeThing(player))
		{
				toTile->addThing(player);

				player->pos.x = to_x;
				player->pos.y = to_y;
				player->pos.z = to_z;
		}
	
		CreatureVector::iterator cit;
		quests->questScript->onCreatureTeleport(player->getID(), toTile->pos);		
		for (int x = min(oldPos.x, (int)to_x) - 14; x <= max(oldPos.x, (int)to_x) + 14; x++)
		{
				for (int y = min(oldPos.y, (int)to_y) - 18; y <= max(oldPos.y, (int)to_y) + 18; y++)
				{
					Tile *tile = getTile(x, y, 7);
	
					if (tile)
					{
						for (cit = tile->creatures.begin(); cit != tile->creatures.end(); cit++)
						{
							(*cit)->onTeleport(player, player, &oldPos, oldstackpos, magic);
						}
					}
				}
		}
        }
        
}

void Map::savePlayer(unsigned long id)
{
         Creature *c = getCreatureByID(id);
         Player* p = dynamic_cast<Player*>(c);  
         if(p){
           std::string charName = p->getName();
           p->savePlayer(charName);
           addEvent(makeTask(10000, std::bind2nd(std::mem_fun(&Map::savePlayer), p->getID())));
         } 
} 

void Map::needContainerUpdate(Item *container, Creature *player, Item *item, unsigned char from_slot, unsigned char to_slot, bool remove, bool updatecount)
{
    std::list<unsigned long> creatures;
    std::list<unsigned long>::iterator cit;
	for(cit = container->getPlayers(); cit != container->getPlayersEnd(); cit++)
	{
        bool continu = false;    
        for(std::list<unsigned long>::iterator it = creatures.begin(); it != creatures.end(); it++)
        {
		   if((*it) == (*cit)){
              continu = true;
			  break;
		   }
	    }
	    if(continu)
	       continue;
        creatures.push_front((*cit));
            
        Creature* creature = getCreatureByID((*cit));
        Player *p = dynamic_cast<Player*>(creature);  
        if(p && !updatecount){
             for(unsigned int i = 0; i < 16; i++) {
				if(p->getContainer(i) && p->getContainer(i) == container) {
                     if(remove)                 
			            creature->onContainerUpdated(item, i, 0xFF, from_slot, to_slot, remove);
			         else
                        creature->onContainerUpdated(item, 0xFF, i, from_slot, to_slot, remove);  
                }
            }        
        }else if(p && updatecount){
             for(unsigned int i = 0; i < 16; i++) {
				if(p->getContainer(i) && p->getContainer(i) == container) {
                     if(remove)                 
			            creature->onContainerUpdated(item, i, i, from_slot, to_slot, remove);
			         else
                        creature->onContainerUpdated(item, i, i, from_slot, to_slot, remove);  
                }
            }        
        }  
    }
}

void Map::pkTime(Player *p)
{
   Creature* c = getCreatureByID(p->getID());
   if(c){
     if(p->time > 0){
       p->time -= 1;
       addEvent(makeTask(1000, std::bind2nd(std::mem_fun(&Map::pkTime), p)));
     }else
       p->pking = 0; 
   }       
}      
      
void Map::pkPlayer(Creature *c)
{
  Player* p = dynamic_cast<Player*>(c);
  Creature* ce = getCreatureByID(p->getID());
  if(ce && p){
     bool canshow = true;     
     if(p->time <= 0)
          canshow = false;
             
     std::vector<Creature*> liste;
	 getSpectators(Range(ce->pos), liste);

     for(int i = 0; i < liste.size(); ++i)
	 {
            Player* spectatore = dynamic_cast<Player*>(liste[i]);
            if(spectatore){
                NetworkMessage msgo;
			    std::stringstream exp;
                msgo.AddAnimatedText(c->pos, 0xB4, "PK!");
                spectatore->sendNetworkMessage(&msgo);
            }
	 }
	 if(canshow)
	  addEvent(makeTask((long)g_config.getGlobalNumber("pkup", 500), std::bind2nd(std::mem_fun(&Map::pkPlayer), ce)));
  }  
}

void Map::checkLight(const long omg) {
  lighttick += lightdelta; 
  if(lighttick == 250 || lighttick == 10){ 
    if (lighttick <= 10) { 
      lightdelta = 1;
    }else if (lighttick > 10) { 
      lightdelta = -1; 
    }  
  }
  addEvent(makeTask(4000, std::bind2nd(std::mem_fun(&Map::checkLight),0))); 
}

void Map::checkPlayerFollowing(unsigned long id) 
{      
     Creature* creature = getCreatureByID(id); 
     Player* player = dynamic_cast<Player*>(creature); 
     if(player && player->health > 0) 
     {      
         addEvent(makeTask(player->getWalkDuration(), std::bind2nd(std::mem_fun(&Map::checkPlayerFollowing), id)));      
         if(player->followedCreature != 0) 
         { 
             Creature* followedCreature = getCreatureByID(player->followedCreature);
             if(followedCreature) 
             { 
                 if(abs(followedCreature->pos.x - player->pos.x) > 8 || abs(followedCreature->pos.y - player->pos.y) > 6 || followedCreature->pos.z != player->pos.z) 
                 { 
                     if(player->attackedCreature == 0)                      
                        player->sendCancelFollowing();
                     else
                        player->followedCreature = 0;
                     return; 
                 } 
                 if((abs(followedCreature->pos.x - player->pos.x) == 1 || followedCreature->pos.x - player->pos.x == 0) && 
                    (abs(followedCreature->pos.y - player->pos.y) == 1 || followedCreature->pos.y - player->pos.y == 0)) 
                 { 
                     return; 
                 } 
                        
                 std::list<Position> route = getPathTo(player->pos, followedCreature->pos); 
                 if(route.size()==0) 
                 { 
                     player->sendCancelWalk("Target Lost."); 
                     if(player->attackedCreature == 0)                      
                        player->sendCancelFollowing();
                     else
                        player->followedCreature = 0;
                     return; 
                 } 
                 else 
                     route.pop_front(); 
                 Position nextStep = route.front(); 
                 route.pop_front(); 
                 int dx = nextStep.x - player->pos.x; 
                 int dy = nextStep.y - player->pos.y; 
                 thingMove(player, player, player->pos.x + dx, player->pos.y + dy, player->pos.z); 
                 return; 
             } 
             if(player->attackedCreature == 0)                      
                        player->sendCancelFollowing();
             else
                        player->followedCreature = 0;
         } 
     } 
}

void Map::creatureMonsterYell(Npc* monster, const std::string& text) 
{
	SURVIVALSYS_THREAD_LOCK(mapLock)

	std::vector<Creature*> list;
	getSpectators(Range(monster->pos.x-14, monster->pos.x+14, monster->pos.y-14,monster->pos.x+14, 7, 7), list);

	for(unsigned int i = 0; i < list.size(); ++i) {
		list[i]->onCreatureSay(monster, 1, text);
	}

  SURVIVALSYS_THREAD_UNLOCK(mapLock)
}




