////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include "definitions.h"

#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

#include <stdlib.h>

#include "protocol.h"
#include "player.h"
#include "tools.h"
#include "luascript.h"

extern LuaScript g_config;

Player::Player(const char *name, Protocol *p) : Creature(name)
{
  client           = p;
  this->name       = name;
  looktype         = PLAYER_MALE_1;
  pzLocked         = false;
  premmy         = false;
  cancelMove       = false;
  cap              = 300;
  level            = 1;
  empty            = new Item();
  useCount         = 0; 
  inFightTicks     = 0;
  voc              = 0;
  followedCreature = 0;
  cash             = 0;
  cashtrade        = 0;
  mana             = 0;
  manamax          = 0;
  manaspent        = 0;
  pking            = 0;
  time             = 0;
  food             = 0;   
  pled             = 0;
  mled             = 0; 
  experience       = 0;
  maglevel         = 0;
  access           = 0; 
  fightMode        = 0; 
  followMode       = 0;
  guildstatus      = 0; 
  guildnicks       = 0;
  guildname        = "None"; 
  guildrank        = "None"; 
  guildnick        = "None";
  for(int i = 0; i < 7; i++)
  {
     skills[i][SKILL_LEVEL] = 1;
     skills[i][SKILL_TRIES] = 0;
  }
  for(int i=0;i<10;i++)
     openchest[i] = 0;       
  for(int i = 0; i < 11; i++)
	 items[i] = NULL;
  for(int i = 0; i < 16; i++)
	 containerToClose[i] = false;     
  
  CapGain[0]  = 30;
  CapGain[1]  = 10;
  CapGain[2]  = 10;     
  CapGain[3]  = 20;
  CapGain[4]  = 25;
  
  ManaGain[0] = 30;
  ManaGain[1] = 30;
  ManaGain[2] = 30;
  ManaGain[3] = 15;
  ManaGain[4] = 5;
  
  HPGain[0]   = 30;
  HPGain[1]   = 5;
  HPGain[2]   = 5;
  HPGain[3]   = 10;
  HPGain[4]   = 15;  
  
  timeMana    = 1;
  timeLife    = 1;
} 


Player::~Player()
{
    for(containerLayout::iterator cl = vcontainers.begin(); cl != vcontainers.end(); ++cl)                           
        (cl->second)->removePlayer(this);                  
	for (int i = 0; i < 11; i++)
		if (items[i])
            delete items[i];
    delete empty;        
    delete client;
}

std::string Player::getDescription(bool self){
    std::stringstream s;
	std::string str;
    if(self){ 
      s << "You see yourself."; 
      if(voc > 0){
        if(access != 3)
          s << " You are " << g_config.getGlobalStringField("vocations", voc) << ".";
        else
            s << "You are a gamemaster.";
      }         
      if(guildstatus != 0 && guildnicks == 0) 
        s << " You are " << guildrank << " of the " << guildname << "."; 
      else{ 
        if(guildnicks != 0) 
          s << " You are " << guildrank << " of the " << guildname << ".(""" << guildnick << """)"; 
      } 
   }else { 
      s << "You see " << name << " (Level " << level <<")."; 
        if(voc > 0){ 
          if(sex != 0) 
            s << " He"; 
          else 
            s << " She"; 
          if(access != 3)
            s << " is "<< g_config.getGlobalStringField("vocations", voc) << ".";
          else
            s << " is a gamemaster.";   
        } 
        if(guildstatus != 0 && guildnicks == 0){ 
          if(sex != 0) 
            s << " He"; 
          else 
            s << " She"; 
          s << " is " << guildrank << " of the " << guildname << "."; 
       }else{ 
         if(guildnicks != 0){ 
           if(sex != 0) 
             s << " He"; 
           else 
             s << " She"; 
           s << " is " << guildrank << " of the " << guildname << ". (""" << guildnick << """)"; 
         } 
       } 
   } 
   str = s.str(); 
   return str; 
}

Item* Player::getItem(int pos)
{
	if(pos>0 && pos <11)
		return items[pos];
	return NULL;
}

Item* Player::emptySlot() 
{                  
      return empty; 
}

int Player::getWeaponDamage() const
{
	double damagemax = skills[SKILL_FIST][SKILL_LEVEL] + 5;
	bool dist = false;
    for (int slot = SLOT_RIGHT; slot <= SLOT_LEFT; slot++)
    {
	  if (items[slot])
      {
		  if ((items[slot]->isWeapon()))
          {
			  switch (items[slot]->getWeaponType())
              {
					case SWORD:
						damagemax = (skills[SKILL_SWORD][SKILL_LEVEL]*Item::items[items[slot]->getID()].attack)/15;
                        break;
					case CLUB:
						damagemax = (skills[SKILL_CLUB][SKILL_LEVEL]*Item::items[items[slot]->getID()].attack)/15;
                        break;
					case AXE:
						damagemax = (skills[SKILL_AXE][SKILL_LEVEL]*Item::items[items[slot]->getID()].attack)/15;
                        break;
					case DIST:
						if(random_range(1,100) <= 50)
						   dist = true;
						damagemax = (skills[SKILL_DIST][SKILL_LEVEL]*35)/15;
                        break;
			  }
          }
      }
    }
    if(!dist) 
	   return 1+(int)(damagemax*rand()/(RAND_MAX+1.0));
    else
       return 1+random_range((int)(damagemax*0.25), (int)damagemax);
}   

int Player::getArm() const
{
  int armmax = 0;
  
  for (int slot = SLOT_HEAD; slot <= SLOT_FEET; slot++)
  {
  	if (items[slot])
    {
       if (items[slot]->isArmor())
          armmax = armmax + Item::items[items[slot]->getID()].arm;
    }      
  } 

  if (armmax == 0)
    return armmax;
  else
    armmax = 1+(int)( random_range( (int)(armmax*0.35),(int)armmax) );
  return armmax;
}

int Player::getShieldDef() const
{
  double defmax = 0;
  for(int slot = SLOT_RIGHT; slot <= SLOT_LEFT; slot++)
  if(items[slot])
  {
    defmax = defmax + Item::items[items[slot]->getID()].defence;
  }
  defmax = (defmax*skills[SKILL_SHIELD][SKILL_LEVEL])/30;
  return 1+(int)( random_range( (int)(defmax*0.25),(int)defmax) );
}

void Player::speak(const std::string &text)
{
     
}

void Player::sendIcons()
{
     int icons = 0;
     if(inFightTicks >= 6000 || inFightTicks ==4000 || inFightTicks == 2000)
                     icons |= ICON_SWORDS;
     if(manaShieldTicks >= 1000)
                     icons |= ICON_MANASHIELD;
     if(speed != getNormalSpeed())
                     icons |= ICON_HASTE;                 
     client->sendIcons(icons);             
}

int Player::addItem(Item* item, int pos){
#ifdef __DEBUG__
	std::cout << "Should add item at " << pos <<std::endl;
#endif

  if(pos>0 && pos <11)
  {
    if (items[pos])
      delete items[pos];
	items[pos]=item;
  }
	return true;
}

unsigned int Player::getReqSkilltries (int skill, int level, int voc) {
    unsigned short int SkillBases[7] = { 50, 50, 50, 50, 30, 50, 20 };
    float SkillMultipliers[7][5] = {
                                   {1.5, 1.5, 1.5, 1.2, 1.1},     // Fist
                                   {2, 2, 1.8, 1.2, 1.1},         // Club
                                   {2, 2, 1.8, 1.2, 1.1},         // Sword
                                   {2, 2, 1.8, 1.2, 1.1},         // Axe
                                   {2, 2, 1.8, 1.1, 1.4},         // Distance
                                   {1.5, 1.5, 1.5, 1.1, 1.1},     // Shielding
                                   {1.1, 1.1, 1.1, 1.1, 1.1}      // Fishing
                                   };
                                   
    return (int) ( SkillBases[skill] * pow((float) SkillMultipliers[skill][voc], (float) ( level - 11) ) );
}

void Player::addSkillTry(int skilltry)
{
int skill;
std::string skillname;

skill = 0; skillname = "fist fighting";
for (int slot = SLOT_RIGHT; slot <= SLOT_LEFT; slot++)
    if (items[slot])
    {
     if (items[slot]->isWeapon())
     {
      switch (items[slot]->getWeaponType())
            {
            case SWORD: skill = 2; skillname = "sword fighting"; break;
            case CLUB: skill = 1; skillname = "club fighting"; break;
            case AXE: skill = 3; skillname = "axe fighting"; break;
            case DIST: skill = 4; skillname = "distance fighting"; break;
            }
      }
    }     

skills[skill][SKILL_TRIES] += skilltry;

if (skills[skill][SKILL_TRIES] >= getReqSkilltries (skill, (skills[skill][SKILL_LEVEL] + 1), voc))
{
   skills[skill][SKILL_LEVEL]++;
   skills[skill][SKILL_TRIES] = 0;

   NetworkMessage msg;
   std::stringstream advMsg;
   advMsg << "You advanced in " << skillname << ".";
   msg.AddTextMessage(MSG_ADVANCE, advMsg.str().c_str());
   msg.AddPlayerSkills(this);
   sendNetworkMessage(&msg);
}


}

void Player::addSkillShieldTry(int skilltry)
{
 int skill = 0;
 std::string skillname;
 for (int slot = SLOT_RIGHT; slot <= SLOT_LEFT; slot++)
    if (items[slot])
    {
     if (items[slot]->isShield())
     {
      switch (items[slot]->getWeaponType())
            {
            case SHIELD: skill = 5; skillname = "shielding"; break;
            }
      }
    }     
 if(skill != 5)
   return;
 skills[skill][SKILL_TRIES] += skilltry;

 if (skills[skill][SKILL_TRIES] >= getReqSkilltries (skill, (skills[skill][SKILL_LEVEL] + 1), voc))
 {
   skills[skill][SKILL_LEVEL]++;
   skills[skill][SKILL_TRIES] = 0;

   NetworkMessage msg;
   std::stringstream advMsg;
   advMsg << "You advanced in " << skillname << ".";
   msg.AddTextMessage(MSG_ADVANCE, advMsg.str().c_str());
   msg.AddPlayerSkills(this);
   sendNetworkMessage(&msg);
 }
}

unsigned int Player::getReqMana(int maglevel, int voc) {
  float ManaMultiplier[5] = { 0, 1.1, 1.1, 1.4, 3 };
  return (unsigned int) ( 400 * pow(ManaMultiplier[voc], maglevel-1) );       //will calculate required mana for a magic level
}

Item* Player::getContainer(unsigned char containerid)
{
  for(containerLayout::iterator cl = vcontainers.begin(); cl != vcontainers.end(); ++cl)
  {
	  if(cl->first == containerid)
			return cl->second;
  }
  return NULL;
}

unsigned char Player::getContainerID(Item* container)
{
  for(containerLayout::iterator cl = vcontainers.begin(); cl != vcontainers.end(); ++cl)
  {
	  if(cl->second == container)
			return cl->first;
	}

	return 0xFF;
}

void Player::addContainer(unsigned char containerid, Item *container)
{
#ifdef __DEBUG__
	cout << Creature::getName() << ", addContainer: " << (int)containerid << std::endl;
#endif
	if(containerid > 0xF)
		return;
		
    container->addPlayer(this);
    
    bool found = false;
    for(int i=0;i<11;i++){
        if(items[i] && items[i]->isContainer()){
            if(items[i] == container)
               found = true;
            else   
               items[i]->findItem(container, found);
        }       
        if(found)      
            break;  
    }
    if(!found)
         containerToClose[containerid] = true;
           
	for(containerLayout::iterator cl = vcontainers.begin(); cl != vcontainers.end(); ++cl) {
		if(cl->first == containerid) {     
            cl->second = container;
			return;
		}
	}
    
	containerItem vItem;
	vItem.first = containerid;
	vItem.second = container;

	vcontainers.push_back(vItem);
}

void Player::closeContainer(unsigned char containerid)
{
  for(containerLayout::iterator cl = vcontainers.begin(); cl != vcontainers.end(); ++cl)
  {                            
	  if(cl->first == containerid)
	  {
            (cl->second)->removePlayer(this);
            containerToClose[containerid] = false;      
		    vcontainers.erase(cl);
			break;
		}
	}

#ifdef __DEBUG__
	cout << Creature::getName() << ", closeContainer: " << (int)containerid << std::endl;
#endif
}

void Player::closeContainerWithoutRemove(unsigned char containerid)
{
  for(containerLayout::iterator cl = vcontainers.begin(); cl != vcontainers.end(); ++cl)
  {                            
	  if(cl->first == containerid)
	  {  
		    vcontainers.erase(cl);
			break;
		}
	}

#ifdef __DEBUG__
	cout << Creature::getName() << ", closeContainer: " << (int)containerid << std::endl;
#endif
}

fight_t Player::getFightType()
{
  for (int slot = SLOT_RIGHT; slot <= SLOT_LEFT; slot++)
  {
    if (items[slot])
    {
			if ((items[slot]->isWeapon())) {
				switch (items[slot]->getWeaponType())
        {
					case DIST:
						return FIGHT_DIST;
				}
			}
    }
  }
  return FIGHT_MELEE;
}

subfight_t Player::getSubFightType()
{
	fight_t type = getFightType();
	if(type == FIGHT_DIST) {
		return DIST_BOLT;
	}
	return DIST_NONE;
}

bool Player::CanSee(int x, int y)
{
  return client->CanSee(x, y);
}


void Player::sendNetworkMessage(NetworkMessage *msg)
{
  client->sendNetworkMessage(msg);
}

void Player::sendLight(const int llevel, int lcolor) { 
  client->sendLightLevel(llevel, lcolor); 
}

void Player::sendCancel(const char *msg)
{
  client->sendCancel(msg);
}
void Player::sendChangeSpeed(Creature* creature){
     client->sendChangeSpeed(creature);
}

void Player::sendToChannel(Creature *creature, unsigned char type, const std::string &text, unsigned short channelId){
     client->sendToChannel(creature, type, text, channelId);
}

void Player::sendCancelAttacking()
{
  attackedCreature = 0;   
  client->sendCancelAttacking();
}

void Player::sendCancelFollowing()
{
  followedCreature = 0;   
  client->sendCancelFollowing();
}

void Player::sendCancelWalk(const char *msg)
{
  client->sendCancelWalk(msg);
}

void Player::onThingMove(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos)
{
  client->sendThingMove(player, thing, oldPos, oldstackpos);
}

void Player::onTeleport(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos, bool magiceffect)
{
  client->sendTeleport(player, thing, oldPos, oldstackpos, magiceffect);
}

void Player::sendKick()
{
  client->sendKick();
}

void Player::setAttackedCreature(unsigned long id){
     attackedCreature = id;
}

void Player::onCreatureAppear(const Creature *creature)
{
  client->sendCreatureAppear(creature);
}


void Player::onCreatureDisappear(const Creature *creature, unsigned char stackPos)
{
  client->sendCreatureDisappear(creature, stackPos);
}


void Player::onCreatureTurn(const Creature *creature, unsigned char stackPos)
{
  client->sendCreatureTurn(creature, stackPos);
}

unsigned long Player::getIP()
{
  return client->GetProtocolIp();
}

void Player::onCreatureSay(const Creature *creature, unsigned char type, const std::string &text)
{
   client->sendCreatureSay(creature, type, text);
}

void Player::onCreatureChangeOutfit(const Creature* creature) {
   client->sendSetOutfit(creature);
}

void Player::onThink(){}

void Player::onTileUpdated(const Position *Pos)
{
  client->sendTileUpdated(Pos);
}

void Player::onContainerUpdated(Item *item, unsigned char from_id, unsigned char to_id,
																unsigned char from_slot, unsigned char to_slot, bool remove)
{
	client->sendContainerUpdated(item, from_id, to_id, from_slot, to_slot, remove);
}

void Player::releasePlayer()
{
        useCount--;
        if (useCount == 0){
            for(containerLayout::iterator cl = vcontainers.begin(); cl != vcontainers.end(); ++cl)                           
               (cl->second)->removePlayer(this);
        }                
        
}
  
void Player::savePlayer(std::string &name)
{  
    std::string filename = "data/players/"+name+".xml";
    std::stringstream sb;
    
    xmlDocPtr doc;
	xmlNodePtr nn, sn, pn, root, bp, subbp;
	doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"player", NULL);
	root = doc->children;
	
	if (health <= 0)
	   {
       health = healthmax;
       mana = manamax;
       pos.x = masterPos.x;
       pos.y = masterPos.y;
       pos.z = masterPos.z;
       
       int expLoss = (int)(experience*0.05);
       experience -= expLoss;
       
       //Player died?
       for(int a=0;a<5;a++){
	     int reqExp =  getExpForLv(level);
         if (experience < reqExp)
		    {
            level -= 1;
            healthmax -= HPGain[voc];
            health -= HPGain[voc];
            
            if ((manamax - ManaGain[voc]) <= 0)
                 manamax = 0;
            else manamax = manamax - ManaGain[voc];
            if ((mana - ManaGain[voc]) <= 0)
                 mana = 0;
            else mana = mana - ManaGain[voc];
            
            cap -= CapGain[voc];            
            }
         }
       }
       
	sb << name;  	           xmlSetProp(root, (const xmlChar*) "name", (const xmlChar*)sb.str().c_str());     sb.str("");
	sb << accountNumber;       xmlSetProp(root, (const xmlChar*) "account", (const xmlChar*)sb.str().c_str());	sb.str("");
	sb << sex;                 xmlSetProp(root, (const xmlChar*) "sex", (const xmlChar*)sb.str().c_str());     	sb.str("");	
	sb << getDirection();
    if (sb.str() == "North"){sb.str(""); sb << "0";}
	if (sb.str() == "East") {sb.str(""); sb << "1";}
	if (sb.str() == "South"){sb.str(""); sb << "2";}
	if (sb.str() == "West") {sb.str(""); sb << "3";}
	xmlSetProp(root, (const xmlChar*) "lookdir", (const xmlChar*)sb.str().c_str());                             sb.str("");
	sb << experience;         xmlSetProp(root, (const xmlChar*) "exp", (const xmlChar*)sb.str().c_str());       sb.str("");	
	sb << cash;               xmlSetProp(root, (const xmlChar*) "cash", (const xmlChar*)sb.str().c_str());       sb.str("");	
    sb << voc;                xmlSetProp(root, (const xmlChar*) "voc", (const xmlChar*)sb.str().c_str());       sb.str("");
	sb << level;              xmlSetProp(root, (const xmlChar*) "level", (const xmlChar*)sb.str().c_str());     sb.str("");	
	sb << access;             xmlSetProp(root, (const xmlChar*) "access", (const xmlChar*)sb.str().c_str());	sb.str("");	
	sb << cap;    	          xmlSetProp(root, (const xmlChar*) "cap", (const xmlChar*)sb.str().c_str());       sb.str("");
	sb << maglevel;	          xmlSetProp(root, (const xmlChar*) "maglevel", (const xmlChar*)sb.str().c_str());  sb.str("");
	sb << pled;               xmlSetProp(root, (const xmlChar*) "pkills", (const xmlChar*)sb.str().c_str());	sb.str("");	
	sb << mled;    	          xmlSetProp(root, (const xmlChar*) "mkills", (const xmlChar*)sb.str().c_str());    sb.str("");
	sb << time;	              xmlSetProp(root, (const xmlChar*) "pktime", (const xmlChar*)sb.str().c_str());    sb.str("");
	

	pn = xmlNewNode(NULL,(const xmlChar*)"spawn");
	sb << pos.x;    xmlSetProp(pn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << pos.y;  	xmlSetProp(pn, (const xmlChar*) "y", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << pos.z; 	xmlSetProp(pn, (const xmlChar*) "z", (const xmlChar*)sb.str().c_str());	       sb.str("");
	xmlAddChild(root, pn);
	
	pn = xmlNewNode(NULL,(const xmlChar*)"temple");
	sb << masterPos.x;  xmlSetProp(pn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << masterPos.y;  xmlSetProp(pn, (const xmlChar*) "y", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << masterPos.z; 	xmlSetProp(pn, (const xmlChar*) "z", (const xmlChar*)sb.str().c_str());	       sb.str("");
	xmlAddChild(root, pn);
	
	pn = xmlNewNode(NULL,(const xmlChar*)"guild"); 
    sb << guildstatus; xmlSetProp(pn, (const xmlChar*) "status", (const xmlChar*)sb.str().c_str()); sb.str(""); 
    sb << guildnicks; xmlSetProp(pn, (const xmlChar*) "nicks", (const xmlChar*)sb.str().c_str()); sb.str(""); 
    sb << guildname; xmlSetProp(pn, (const xmlChar*) "name", (const xmlChar*)sb.str().c_str()); sb.str(""); 
    sb << guildrank; xmlSetProp(pn, (const xmlChar*) "rank", (const xmlChar*)sb.str().c_str()); sb.str(""); 
    sb << guildnick; xmlSetProp(pn, (const xmlChar*) "nick", (const xmlChar*)sb.str().c_str()); sb.str(""); 
    xmlAddChild(root, pn);

	pn = xmlNewNode(NULL,(const xmlChar*)"health");
	sb << health;     xmlSetProp(pn, (const xmlChar*) "now", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << healthmax;  xmlSetProp(pn, (const xmlChar*) "max", (const xmlChar*)sb.str().c_str());        sb.str("");
	                     xmlSetProp(pn, (const xmlChar*) "food", (const xmlChar*)"0");	   
	xmlAddChild(root, pn);
	
	pn = xmlNewNode(NULL,(const xmlChar*)"mana");
	sb << mana;      xmlSetProp(pn, (const xmlChar*) "now", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << manamax;   xmlSetProp(pn, (const xmlChar*) "max", (const xmlChar*)sb.str().c_str());        sb.str("");
    sb << manaspent; xmlSetProp(pn, (const xmlChar*) "spent", (const xmlChar*)sb.str().c_str());      sb.str("");
	xmlAddChild(root, pn);
    	               
	pn = xmlNewNode(NULL,(const xmlChar*)"look");
    sb << lookmaster;       xmlSetProp(pn, (const xmlChar*) "type", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << lookhead;         xmlSetProp(pn, (const xmlChar*) "head", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << lookbody;         xmlSetProp(pn, (const xmlChar*) "body", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << looklegs;         xmlSetProp(pn, (const xmlChar*) "legs", (const xmlChar*)sb.str().c_str());        sb.str("");
	sb << lookfeet;         xmlSetProp(pn, (const xmlChar*) "feet", (const xmlChar*)sb.str().c_str());        sb.str("");
	xmlAddChild(root, pn);
    	               
    	      
	sn = xmlNewNode(NULL,(const xmlChar*)"skills");
	for (int i = 0; i <= 6; i++)
	  {
	  pn = xmlNewNode(NULL,(const xmlChar*)"skill");
	  sb << i;                          xmlSetProp(pn, (const xmlChar*) "skillid", (const xmlChar*)sb.str().c_str());      sb.str("");
	  sb << skills[i][SKILL_LEVEL];     xmlSetProp(pn, (const xmlChar*) "level", (const xmlChar*)sb.str().c_str());        sb.str("");
	  sb << skills[i][SKILL_TRIES];     xmlSetProp(pn, (const xmlChar*) "tries", (const xmlChar*)sb.str().c_str());        sb.str("");
	  xmlAddChild(sn, pn);
      }
   xmlAddChild(root, sn);
	
	sn = xmlNewNode(NULL,(const xmlChar*)"inventory");
	for (int i = 1; i <= 10; i++)
	{
   	  if (items[i] && !items[i]->isContainer())
      {
    	  pn = xmlNewNode(NULL,(const xmlChar*)"slot");
    	  sb << i;                             
          xmlSetProp(pn, (const xmlChar*) "slotid", (const xmlChar*)sb.str().c_str());            
          sb.str("");
          
      	  nn = xmlNewNode(NULL,(const xmlChar*)"item");
          sb << items[i]->getID();
          xmlSetProp(nn, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
          sb.str("");
          
          if(items[i]->isRune()){
             sb << (int)items[i]->getItemCharge();
             xmlSetProp(nn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());            
             sb.str("");
          }else if(items[i]->isStackable() || items[i]->isMultiType()){
             sb << (int)items[i]->getItemCountOrSubtype();
             xmlSetProp(nn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());            
             sb.str("");
          }
          
	      xmlAddChild(pn, nn);
	      xmlAddChild(sn, pn);
      }
      //Save containers
      else if(items[i])
      {
          pn = xmlNewNode(NULL,(const xmlChar*)"slot");
    	  sb << i;                             
          xmlSetProp(pn, (const xmlChar*) "slotid", (const xmlChar*)sb.str().c_str());            
          sb.str("");
          
      	  nn = xmlNewNode(NULL,(const xmlChar*)"item");
          sb << items[i]->getID();
          xmlSetProp(nn, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
          sb.str(""); 
          xmlAddChild(pn, nn);         
          
          Item* item = items[i];
          for(int b=0;b<=item->getContainerItemCount();b++){
            int a = item->getContainerItemCount() - b;
            if(item->getItem(a) && item->getItem(a)->isRune()){
              bp = xmlNewNode(NULL,(const xmlChar*)"rune");
              sb << item->getItem(a)->getID();
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());
              sb.str("");
              sb << (int)item->getItem(a)->getItemCharge();
              xmlSetProp(bp, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());          
              sb.str("");
              xmlAddChild(pn, bp);                  
            }else if(item->getItem(a) && (item->getItem(a)->isStackable() || item->getItem(a)->isMultiType())){
              bp = xmlNewNode(NULL,(const xmlChar*)"stackable");
              sb << item->getItem(a)->getID();
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());
              sb.str("");
              sb << (int)item->getItem(a)->getItemCountOrSubtype();
              xmlSetProp(bp, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());          
              sb.str("");
              xmlAddChild(pn, bp);
            }else if(item->getItem(a) && item->getItem(a)->isContainer()){
              bp = xmlNewNode(NULL,(const xmlChar*)"container");
    	      sb << item->getItem(a)->getID();                          
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
              sb.str("");
              Item* newitem = item->getItem(a);
              saveContainer(newitem, bp, sb);                              
              xmlAddChild(pn, bp);
            }else if(item->getItem(a)){                                      
              bp = xmlNewNode(NULL,(const xmlChar*)"item2");
              sb << item->getItem(a)->getID();
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
              sb.str("");
              xmlAddChild(pn, bp);
            }
          }
	      xmlAddChild(sn, pn);
      }    
   }
   xmlAddChild(root, sn);
	
	//Save the character
    if (xmlSaveFile(filename.c_str(), doc))
       {
       /*#ifdef __DEBUG__
       std::cout << ":: Saved character succefully!\n";
       #endif*/
       xmlFreeDoc(doc);
       }
    else
       {
       std::cout << ":: Couldn't save character =(\n";
       xmlFreeDoc(doc);
       }
}
//Save more containers...
void Player::saveContainer(Item* item, xmlNodePtr pn, std::stringstream &sb)
{
          xmlNodePtr bp;                       
          for(int b=0;b<=item->getContainerItemCount();b++){
            int a = item->getContainerItemCount() - b;
            if(item->getItem(a) && item->getItem(a)->isRune()){
              bp = xmlNewNode(NULL,(const xmlChar*)"rune");
              sb << item->getItem(a)->getID();
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());
              sb.str("");
              sb << (int)item->getItem(a)->getItemCharge();
              xmlSetProp(bp, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());          
              sb.str("");
              xmlAddChild(pn, bp);                  
            }else if(item->getItem(a) && (item->getItem(a)->isStackable() || item->getItem(a)->isMultiType())){
              bp = xmlNewNode(NULL,(const xmlChar*)"stackable");
              sb << item->getItem(a)->getID();
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());
              sb.str("");
              sb << (int)item->getItem(a)->getItemCountOrSubtype();
              xmlSetProp(bp, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());          
              sb.str("");
              xmlAddChild(pn, bp);                  
            }else if(item->getItem(a) && item->getItem(a)->isContainer()){
              bp = xmlNewNode(NULL,(const xmlChar*)"container");
    	      sb << item->getItem(a)->getID();                          
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
              sb.str("");
              Item* newitem = item->getItem(a);
              saveContainer(newitem, bp, sb);                           
              xmlAddChild(pn, bp);
            }else if(item->getItem(a)){                                      
              bp = xmlNewNode(NULL,(const xmlChar*)"item2");
              sb << item->getItem(a)->getID();
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
              sb.str("");
              xmlAddChild(pn, bp);
            }
          }
}


