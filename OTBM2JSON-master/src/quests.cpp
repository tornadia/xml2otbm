////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include "quests.h"
#include "tile.h"
#include "item.h"

Quests::Quests(const char *arfq, Map* map)
{
     this->questScript = new QuestScript(this);                
     std::string filename = "data/quests/quests.xml";
	 std::transform(filename.begin(), filename.end(), filename.begin(), tolower);
	 xmlDocPtr doc = xmlParseFile(filename.c_str());
	 this->map = map;
	 if(doc){  
		xmlNodePtr root, p, tmp, tmp2;
		root = xmlDocGetRootElement(doc);

		if (xmlStrcmp(root->name,(const xmlChar*) "quests")){
		    std::cerr << "Malformed XML" << std::endl;
		}
		p = root->children;
        while (p)
		{
			if (strcmp((const char*)p->name, "quest") == 0){
                const char* name = (const char*)xmlGetProp(p, (const xmlChar *)"name");
                Quest* quest = new Quest(name, this->map);
                quest->questScript = this->questScript;
                tmp = p->children;
                while (tmp)
                {
                    if(strcmp((const char*)tmp->name, "questitem") == 0)
					{
                       Position pos;
                       pos.x = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"x"));
                       pos.y = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"y"));
                       pos.z = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"z"));
                       Item* item;
                       if((const char*)xmlGetProp(tmp, (const xmlChar *)"chest"))
                          item = new Item(atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"chest")));
                       else
                          item = new Item(1294);
                       item->setQuestName(quest->getName());
                       QuestLoot* questloot = new QuestLoot(item, pos, this->map);
                       if((const char*)xmlGetProp(tmp, (const xmlChar *)"description"))
                          questloot->description = (const char*)xmlGetProp(tmp, (const xmlChar *)"description");
                       tmp2 = tmp->children;
                       while (tmp2)
                       {
                           if(strcmp((const char*)tmp2->name, "item") == 0)
					       {
                              int id = 0;
                              int x = 0;
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *)"id"))        
                                 id = atoi((const char*)xmlGetProp(tmp2, (const xmlChar *)"id"));
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *)"x"))
                                 x = atoi((const char*)xmlGetProp(tmp2, (const xmlChar *)"x"));
                              if(id >= 100 && id <= 2750){
                                 Item item;
                                 if(x > 0)
                                    item = Item(id,x);
                                 else
                                    item= Item(id);
                                 questloot->itemloot = item;          
                              }         
                           }
                           tmp2=tmp2->next;     
                       }
                       quest->addQuestLoot(questloot);               
                    }
                    if(strcmp((const char*)tmp->name, "questbp") == 0)
					{
                       Position pos;
                       pos.x = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"x"));
                       pos.y = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"y"));
                       pos.z = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"z"));
                       Item* item;
                       if((const char*)xmlGetProp(tmp, (const xmlChar *)"chest"))
                          item = new Item(atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"chest")));
                       else
                          item = new Item(1294);
                       item->setQuestName(quest->getName());
                       QuestLoot* questloot = new QuestLoot(item, pos, this->map);
                       questloot->bploot = true;
                       if((const char*)xmlGetProp(tmp, (const xmlChar *)"bpid"))
                          questloot->itemloot = Item(atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"bpid")));
                       else
                          questloot->itemloot = Item(1411);
                       if((const char*)xmlGetProp(tmp, (const xmlChar *)"description"))      
                          questloot->description = (const char*)xmlGetProp(tmp, (const xmlChar *)"description");
                       tmp2 = tmp->children;
                       while (tmp2)
                       {
                           if(strcmp((const char*)tmp2->name, "item") == 0)
					       {
                              int id = 0;
                              int x = 0;
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *)"id"))        
                                 id = atoi((const char*)xmlGetProp(tmp2, (const xmlChar *)"id"));
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *)"x"))
                                 x = atoi((const char*)xmlGetProp(tmp2, (const xmlChar *)"x"));
                              if(id >= 100 && id <= 2750){
                                 Item item;
                                 if(x > 0)
                                    item = Item(id,x);
                                 else
                                    item= Item(id);
                                 questloot->addItemList(item);          
                              }         
                           }
                           tmp2=tmp2->next;     
                       }
                       quest->addQuestLoot(questloot);               
                    }
                    tmp=tmp->next;                    
                }
                addQuest(quest);                  
			}
			p=p->next;
        }         
     }             
}

Quests::~Quests()
{
     for (std::list<Quest *>::iterator cit = listQuests.begin(); cit != listQuests.end(); cit++)
     {
        Quest* quest = dynamic_cast<Quest*>(*cit);
        if(quest){
           delete quest;
        }   
     }            
     delete questScript;
}

Quest* Quests::getQuestByName(std::string name)
{
     for (std::list<Quest *>::iterator cit = listQuests.begin(); cit != listQuests.end(); cit++)
     {
         Quest* quest = dynamic_cast<Quest*>(*cit);
         if(quest && quest->getName() == name){
            return quest;
         }  
     }
     return NULL;      
}
                              
Quest::Quest(const char *questname, Map* map)
{
     this->questname = questname;
     this->map = map;
     std::string filename = "data/quests/names/"+this->questname+".xml";
	 std::transform(filename.begin(), filename.end(), filename.begin(), tolower);
	 xmlDocPtr doc = xmlParseFile(filename.c_str());
	 if(doc){     
		xmlNodePtr root, p;
		root = xmlDocGetRootElement(doc);

		if (xmlStrcmp(root->name,(const xmlChar*) "names")){
		    std::cerr << "Malformed XML" << std::endl;
		}
		
		p = root->children;
        while (p)
		{
            if (strcmp((const char*)p->name, "name") == 0){
                if((const char*)xmlGetProp(p, (const xmlChar *)"person"))              
                   listNames.push_front((const char*)xmlGetProp(p, (const xmlChar *)"person"));
            }
            p=p->next;    
        }
     }                        
}

Quest::~Quest()
{
     for (std::list<QuestLoot *>::iterator cit = listQuestloots.begin(); cit != listQuestloots.end(); cit++)
     {
        QuestLoot* questloot = dynamic_cast<QuestLoot*>(*cit);
        if(questloot){
           delete questloot;
        }   
     }            
}

QuestLoot* Quest::getQuestLootByPos(Position pos)
{
     for (std::list<QuestLoot *>::iterator cit = listQuestloots.begin(); cit != listQuestloots.end(); cit++)
     {
        QuestLoot* questloot = dynamic_cast<QuestLoot*>(*cit);
        if(questloot && questloot->pos == pos){
           return questloot;
        }   
     }
     return NULL;      
}
           
void Quest::playerDoQuest(Player* player, Position pos)
{
     QuestLoot* questloot = getQuestLootByPos(pos);
     if(!questloot)
        return;
     NetworkMessage msg;
     if(!isNameinList(player->getName())){ 
       for(int c=1;c<=10;c++){   
         if(player->items[c] && player->items[c]->isContainer()){
            if(player->items[c]->getContainerMaxItemCount() <= player->items[c]->getContainerItemCount()){
                 std::stringstream msgdescription;
                 msgdescription << questloot->description.c_str() << " More objects container.";                                             
                 msg.AddTextMessage(MSG_INFO, msgdescription.str().c_str());
            }else{                 
                 Item* item = questloot->getLoot();                                 
                 player->items[c]->addItem(item);
		         this->map->needContainerUpdate(player->items[c], player, item, 0xFF, 0, false, false);
		         msg.AddTextMessage(MSG_INFO, questloot->description.c_str());
		         addName(player->getName());
		         this->questScript->onCreatureOpenContainerQuest(player->getID(), getName(), pos);
            }    
         }
       }     
     }else{               
        msg.AddTextMessage(MSG_INFO, "The chest is empty.");
     }
     player->sendNetworkMessage(&msg);    
}
     
void Quest::saveNamesList()
{
    std::stringstream sb; 
    std::string filename = "data/quests/names/"+questname+".xml";  
    xmlDocPtr doc;
	xmlNodePtr sn, root;
	doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"names", NULL);
	root = doc->children;
	
    for (std::list<std::string>::iterator cit = listNames.begin(); cit != listNames.end(); cit++)
    {
       if((*cit) != ""){
         sn = xmlNewNode(NULL,(const xmlChar*)"name");
         sb << (*cit).c_str();                           
         xmlSetProp(sn, (const xmlChar*) "person", (const xmlChar*)sb.str().c_str());            
         sb.str("");
         xmlAddChild(root, sn);
       }   
    }
    xmlAddChild(root, sn);
    if (xmlSaveFile(filename.c_str(), doc)){
       /*#ifdef __DEBUG__
       std::cout << ":: Saved quest names succefully!\n";
       #endif*/
       xmlFreeDoc(doc);
    }else{
       std::cout << ":: Couldn't save quests name =(\n";
       xmlFreeDoc(doc);
    }
}

void Quest::addName(std::string name)
{
     listNames.push_front(name);
     saveNamesList();
}

bool Quest::removeName(std::string name)
{
     bool removed = false;
     for(std::list<std::string>::iterator cit = listNames.begin(); cit != listNames.end(); cit++)
     {
         if((*cit) == name)
         {
            listNames.erase(cit); 
            removed = true;                     
         }                          
     }
     if(removed){
        saveNamesList();         
        return true;
     }   
     return false;    
}

bool Quest::isNameinList(std::string name)
{
     for(std::list<std::string>::iterator cit = listNames.begin(); cit != listNames.end(); cit++)
     {
         if((*cit) == name)
            return true;
     }
     return false;       
}     
     
QuestLoot::QuestLoot(Item *item, Position itemPos, Map* map)
{
     bploot = false;
     this->map = map;
     this->description = "You have found something.";
     this->chest = item;
     this->pos = itemPos;
     Tile *tile = this->map->getTile(pos.x, pos.y, pos.z);
     tile->addThing(this->chest);
     this->map->creatureBroadcastTileUpdated(pos);
}

QuestLoot::~QuestLoot()
{
                       
}

Item* QuestLoot::getLoot()
{
    Item* item = new Item(itemloot.getID());
    item->setItemCharge(itemloot.getItemCharge());
    item->setItemCountOrSubtype(itemloot.getItemCountOrSubtype());
    if(item && item->isContainer() && bploot){
       for(std::list<Item >::iterator cit = bpItems.begin(); cit != bpItems.end(); cit++)
       {
           Item* subitem = new Item((*cit).getID());
           subitem->setItemCharge((*cit).getItemCharge());
           subitem->setItemCountOrSubtype((*cit).getItemCountOrSubtype());
           if(subitem)
              item->addItem(subitem);                                
       }                                          
    }
    return item;                  
}

QuestScript::QuestScript(Quests* quests){
	std::string scriptname = "data/quests/quests.lua";
    this->loaded = false;
	luaState = lua_open();
	luaopen_loadlib(luaState);
	luaopen_base(luaState);
	luaopen_math(luaState);
	luaopen_string(luaState);
	luaopen_io(luaState);
    lua_dofile(luaState, "data/quests/lib.lua");
	
	FILE* in=fopen(scriptname.c_str(), "r");
	if(!in)
		return;
	else
		fclose(in);
	lua_dofile(luaState, scriptname.c_str());
	this->loaded=true;
	this->quests=quests;
	this->setGlobalNumber("addressOfQuests", (int) quests);
	this->registerFunctions();
}

int QuestScript::registerFunctions(){
    lua_register(luaState, "leverMove", QuestScript::luaLeverMove);
    lua_register(luaState, "leverGetSide", QuestScript::luaLeverGetSide);
    lua_register(luaState, "creatureAddText", QuestScript::luaCreatureAddText);
	lua_register(luaState, "creatureGetName", QuestScript::luaCreatureGetName);
	lua_register(luaState, "creatureGetID", QuestScript::luaCreatureGetID);
	lua_register(luaState, "creatureGetLevel", QuestScript::luaCreatureGetLevel);
	lua_register(luaState, "creatureGetPos", QuestScript::luaCreatureGetPos);
	lua_register(luaState, "creatureTeleport", QuestScript::luaCreatureTeleport);
	lua_register(luaState, "creatureSeeCash", QuestScript::luaCreatureSeeCash);
	lua_register(luaState, "creatureAddCash", QuestScript::luaCreatureAddCash);
	lua_register(luaState, "creatureGetCash", QuestScript::luaCreatureGetCash);
	lua_register(luaState, "creatureSeeHealth", QuestScript::luaCreatureSeeHealth);
	lua_register(luaState, "creatureAddHealth", QuestScript::luaCreatureAddHealth);
	lua_register(luaState, "creatureGetHealth", QuestScript::luaCreatureGetHealth);
	return true;
}

void QuestScript::onThink(){
	lua_pushstring(luaState, "onThink");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	lua_call(luaState, 0,0);
}


void QuestScript::onCreaturePullLever(int cid, Position leverPos, int leverSide){
	lua_pushstring(luaState, "onCreaturePullLever");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	lua_pushnumber(luaState, cid);
	lua_pushnumber(luaState, leverPos.x);
	lua_pushnumber(luaState, leverPos.y);
	lua_pushnumber(luaState, leverPos.z);
	lua_pushnumber(luaState, leverSide);
	lua_call(luaState, 5,0);
}

void QuestScript::onCreatureSinkTile(int cid, Position tilePos){
	lua_pushstring(luaState, "onCreatureSinkTile");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	lua_pushnumber(luaState, cid);
	lua_pushnumber(luaState, tilePos.x);
	lua_pushnumber(luaState, tilePos.y);
	lua_pushnumber(luaState, tilePos.z);
	lua_call(luaState, 4,0);
}

void QuestScript::onCreatureTeleport(int cid, Position teleportPos){
	lua_pushstring(luaState, "onCreatureTeleport");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	lua_pushnumber(luaState, cid);
	lua_pushnumber(luaState, teleportPos.x);
	lua_pushnumber(luaState, teleportPos.y);
	lua_pushnumber(luaState, teleportPos.z);
	lua_call(luaState, 4,0);
}

void QuestScript::onCreatureOpenContainerQuest(int cid, std::string questName, Position containerPos){
	lua_pushstring(luaState, "onCreatureOpenContainerQuest");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	lua_pushnumber(luaState, cid);
	lua_pushstring(luaState, questName.c_str());
	lua_pushnumber(luaState, containerPos.x);
	lua_pushnumber(luaState, containerPos.y);
	lua_pushnumber(luaState, containerPos.z);
	lua_call(luaState, 5,0);
}

Quests* QuestScript::getQuests(lua_State *L){
	lua_getglobal(L, "addressOfQuests");
	int val = (int)lua_tonumber(L, -1);
	lua_pop(L,1);

	Quests* myquests = (Quests*) val;
	if(!myquests){
		return 0;
	}
	return myquests;
}

int QuestScript::luaCreatureGetID(lua_State *L){
	const char* s = lua_tostring(L, -1);
	lua_pop(L,1);
	Quests* myquests = getQuests(L);
	Creature *c = myquests->map->getCreatureByName(s);
	
	if(c) {
		lua_pushnumber(L, c->getID());
	}
	else
		lua_pushnumber(L, 0);

	return 1;
}

int QuestScript::luaCreatureGetName(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Quests* myquests = getQuests(L);
	Creature* creature = myquests->map->getCreatureByID(id);
	if(creature)
	   lua_pushstring(L, creature->getName().c_str());
	else
       lua_pushstring(L, "NULL");   
	return 1;
}

int QuestScript::luaCreatureGetLevel(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Quests* myquests = getQuests(L);
	Creature* creature = myquests->map->getCreatureByID(id);
	if(creature)
	   lua_pushnumber(L, creature->level);
	else
       lua_pushnumber(L, 0);   
	return 1;
}

int QuestScript::luaCreatureGetPos(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	
	if(!c){
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
	}
	else{
		lua_pushnumber(L, c->pos.x);
		lua_pushnumber(L, c->pos.y);
		lua_pushnumber(L, c->pos.z);
	}
	return 3;
}

int QuestScript::luaCreatureSeeHealth(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	Player *player = dynamic_cast <Player*>(c);
	if(player)
	    lua_pushnumber(L, player->health);
	else
        lua_pushnumber(L, 0);    
	return 1;
}

int QuestScript::luaCreatureGetHealth(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int get = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
	if(player && player->access != 3){
        player->health -= std::min(get, player->health-1);       
        std::vector<Creature*> list;
	    myquests->map->getSpectators(Range(player->pos, true), list);

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
	return 0;
}

int QuestScript::luaCreatureAddHealth(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int add = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
    if(player && player->access != 3){
	    player->health += std::min(add, player->healthmax - player->health);
	    std::vector<Creature*> list;
	    myquests->map->getSpectators(Range(player->pos, true), list);

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
	return 0;
}

int QuestScript::luaCreatureAddText(lua_State *L){
	int id = (int)lua_tonumber(L, -3);
	int type = (int)lua_tonumber(L, -2);
	int len = lua_strlen(L, -1);
	std::string msg(lua_tostring(L, -1), len);
	lua_pop(L,3);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	Player *player = dynamic_cast <Player*>(c);
	if(player){
	    NetworkMessage newmsg;
	    if(type == 1)
	       newmsg.AddTextMessage(MSG_SMALLINFO, msg.c_str());
	    else if(type == 2)
	       newmsg.AddTextMessage(MSG_INFO, msg.c_str());
	    else if(type == 3)
	       newmsg.AddTextMessage(MSG_STATUS, msg.c_str());
	    else if(type == 4)
	       newmsg.AddTextMessage(MSG_EVENT, msg.c_str());
        else
	       newmsg.AddTextMessage(MSG_ADVANCE, msg.c_str());    
	    player->sendNetworkMessage(&newmsg);
	    lua_pushnumber(L, 1);
	}else
        lua_pushnumber(L, 0);    
	return 1;
}

int QuestScript::luaCreatureSeeCash(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	Player *player = dynamic_cast <Player*>(c);
	if(player)
	    lua_pushnumber(L, player->cash);
	else
        lua_pushnumber(L, 0);    
	return 1;
}

int QuestScript::luaCreatureGetCash(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int cash = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
	if(player)
	    player->cash = player->cash - cash;   
	return 0;
}

int QuestScript::luaCreatureAddCash(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int cash = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Quests* myquests = getQuests(L);
	Creature* c = myquests->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
	if(player)
	    player->cash = player->cash + cash;   
	return 0;
}

int QuestScript::luaCreatureTeleport(lua_State *L){
	int id = (int)lua_tonumber(L, -3);
	int to_x = (int)lua_tonumber(L, -2);
	int to_y = (int)lua_tonumber(L, -1);
	lua_pop(L,4);
	Quests* myquests = getQuests(L);
	
	Position pos;
	pos.x = to_x;
	pos.y = to_y;
	pos.z = 7;
	
	Creature* target = myquests->map->getCreatureByID(id);
	if(target)
	  myquests->map->teleportPlayer(pos, target, true, NULL, false);

	return 0;
}

int QuestScript::luaLeverMove(lua_State *L){
    Position pos;
	pos.x = (int)lua_tonumber(L, -3);
	pos.y = (int)lua_tonumber(L, -2);
	pos.z = (int)lua_tonumber(L, -1);
	lua_pop(L,3);
	Quests* myquests = getQuests(L);
	
	Tile *tile = myquests->map->getTile(pos.x, pos.y, pos.z);
	Item *switchfound = NULL;
    if(tile){
	   for (ItemVector::iterator iit = tile->downItems.begin(); iit != tile->downItems.end(); iit++){ 
		   if ((*iit)->getID() == 1369 || (*iit)->getID() == 1370){
			  switchfound = (*iit);
		   }
	   }
       if(switchfound){
	      if(switchfound->getID() == 1369)
		     switchfound->setID(1370);
          else if(switchfound->getID() == 1370)
		     switchfound->setID(1369);
	      myquests->map->creatureBroadcastTileUpdated(pos);
	      lua_pushnumber(L, 1);
	      return 1;
       }   
    }
    lua_pushnumber(L, 0);			
	return 1;
}

int QuestScript::luaLeverGetSide(lua_State *L){
    Position pos;
	pos.x = (int)lua_tonumber(L, -3);
	pos.y = (int)lua_tonumber(L, -2);
	pos.z = (int)lua_tonumber(L, -1);
	lua_pop(L,3);
	Quests* myquests = getQuests(L);
	
	Tile *tile = myquests->map->getTile(pos.x, pos.y, pos.z);
	Item *switchfound = NULL;
    if(tile){
	   for (ItemVector::iterator iit = tile->downItems.begin(); iit != tile->downItems.end(); iit++){ 
		   if ((*iit)->getID() == 1369 || (*iit)->getID() == 1370){
			  switchfound = (*iit);
		   }
	   }
       if(switchfound){
	      if(switchfound->getID() == 1369)
		     lua_pushnumber(L, -1);
          else if(switchfound->getID() == 1370)
		     lua_pushnumber(L, 1);
	      return 1;
       }   
    }
    lua_pushnumber(L, 0);			
	return 1;
}
