////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include "definitions.h"
#include "item.h"
#include "map.h"
#include "house.h"
#include <iostream>
#include <sstream>

extern Map gmap;

// Returns the ID of this item's ItemType
unsigned short Item::getID() const {
    return id;
}

// Sets the ID of this item's ItemType
void Item::setID(unsigned short newid) {
    id = newid;
}

// Return how many items are stacked or 0 if non stackable
unsigned char Item::getItemCountOrSubtype() const {
    return count;
}


Item::Item(const unsigned short _type) {
    id = _type;
    count = 0;
    questName = "none";
    
    //House info
    house = NULL;
    
		chargecount = 0;
		maxitems = items[id].maxItems;
		actualitems = 0;
    throwRange = 6;
}

Item::Item(const unsigned short _type, unsigned char _count) {
    id = _type;
    questName = "none";
    //House info
    house = NULL;
    
		count = 0;
		chargecount = 0;

		if(isStackable() || isMultiType())
			count = _count;
		else
			chargecount = _count;

    throwRange = 6;
}

Item::Item() {
    id = 0;
    questName = "none";
    //House info
    house = NULL;
    
		chargecount = 0;
		maxitems = 20;
		actualitems = 0;

    throwRange = 6;
}

Item::~Item() {
	  for(std::list<Item *>::const_iterator cit = lcontained.begin(); cit != lcontained.end(); cit++)
	  {
         delete (*cit);
      }
	  lcontained.clear();
}

int Item::unserialize(xmlNodePtr p){
	id =atoi((const char*)xmlGetProp(p, (const xmlChar *) "id"));
	const char* x =(const char*)xmlGetProp(p, (const xmlChar *) "x");
	
    if(x)    
	     if(isStackable() || isMultiType())
			count = atoi(x);
		 else if(isRune())
			chargecount = atoi(x);
	else{ 
         if(isStackable() || isMultiType())
             setItemCountOrSubtype(1);
         else if(isRune())
             setItemCharge(1);
    }
	return 0;
}

/*xmlNodePtr Item::serialize(){
	std::stringstream s;
	xmlNodePtr ret;
	ret=xmlNewNode(NULL,(const xmlChar*)"item");
	s.str("");
	s << getID();
	xmlSetProp(ret, (const xmlChar*)"id", (const xmlChar*)s.str().c_str());
	if(isStackable()){
		s.str("");
		s <<count;
		xmlSetProp(ret, (const xmlChar*)"count", (const xmlChar*)s.str().c_str());
	}
	return ret;
}*/

// Add an item to (this) container if possible
void Item::addItem(Item *newitem) {
    //first check if we are a container, there is an item to be added and if we can add more items...
    //if (!iscontainer) throw TE_NoContainer();
    if (newitem == NULL)
      return;
    //if (maxitems <=actualitems) throw TE_ContainerFull();

    // seems we should add the item...
    // new items just get placed in front of the items we already have...
		if(actualitems < maxitems) {
			lcontained.push_front(newitem);

			// increase the itemcount
			actualitems++;
		}
}

// Add a creature to (this) container if possible
void Item::addPlayer(Creature *newcreature) {
    if (newcreature == NULL)
      return;
		if(lcreatures.size() < 15) {
			lcreatures.push_front(newcreature->getID());
			actualicreatures++;
		}
}

// Remove a creature to (this) container if possible
void Item::removePlayer(Creature* creature)
{
	for (std::list<unsigned long>::iterator cit = lcreatures.begin(); cit != lcreatures.end(); cit++) {
		if((*cit) == creature->getID()) {
			lcreatures.erase(cit);
			actualicreatures--;
			break;
		}
	}
}

void Item::closePlayersContainer() {
    for(std::list<unsigned long>::iterator cit = lcreatures.begin(); cit != lcreatures.end(); cit++) {                 
        Creature* creature = gmap.getCreatureByID((*cit));
        Player *ciplayer = dynamic_cast<Player*>(creature);
        if(ciplayer){
           unsigned char containerid = ciplayer->getContainerID(this);
           if(containerid != 0xFF){
               ciplayer->closeContainerWithoutRemove(containerid);
               NetworkMessage msgi;
               msgi.AddByte(0x6F);
	           msgi.AddByte(containerid);
	           ciplayer->sendNetworkMessage(&msgi);
           }
        }
    }
    lcreatures.clear();
    for (std::list<Item*>::iterator cit = lcontained.begin(); cit != lcontained.end(); cit++)
		if((*cit)->isContainer())
			(*cit)->closePlayersContainer();
}

void Item::removeItem(Item* item)
{
	for (std::list<Item*>::iterator cit = lcontained.begin(); cit != lcontained.end(); cit++) {
		if((*cit) == item) {
			lcontained.erase(cit);
			actualitems--;
			break;
		}
	}
}

int Item::getHanded() const { 
      return items[id].handed; 
}

void Item::isContainerHolding(Item* item, bool& found)
{
	if(found || item == NULL)
		return;

	for (std::list<Item*>::iterator cit = lcontained.begin(); cit != lcontained.end(); cit++) {
		if((*cit)->isContainer()) {

			if((*cit) == item) {
				found = true;
				break;
			}
			else
	            (*cit)->isContainerHolding(item, found);
		}
	}
}

void Item::findItem(Item* item, bool& found)
{
	if(found || item == NULL)
		return;

	for (std::list<Item*>::iterator cit = lcontained.begin(); cit != lcontained.end(); cit++) {
		if((*cit)->isContainer()) {
			if((*cit) == item) {
				found = true;
				break;
			}
			else
	            (*cit)->findItem(item, found);
		}else if((*cit) == item){
            found = true;
            break;
        }  
	}
}

void Item::moveItem(unsigned char from_slot, unsigned char to_slot)
{
	int n = 0;
	for (std::list<Item*>::iterator cit = lcontained.begin(); cit != lcontained.end(); cit++) {
		if(n == from_slot) {
			Item *item = (*cit);
			lcontained.erase(cit);
			lcontained.push_front(item);
			break;
		}
		n++;
	}
}

Item* Item::getItem(unsigned long slot_num)
{
	int n = 0;      
	for (std::list<Item *>::const_iterator cit = getItems(); cit != getEnd(); cit++) {
		if(n == slot_num){
		    if(*cit)
			   return *cit;
		}else
			n++;
	}

	return NULL;
}

unsigned char Item::getSlotNumberByItem(Item* item)
{
	unsigned char n = 0;			
	for (std::list<Item *>::const_iterator cit = getItems(); cit != getEnd(); cit++) {
		if(*cit == item)
			return n;
		else
			n++;
	}

	return 0xFF;
}

std::list<Item *>::const_iterator Item::getItems() {
    return lcontained.begin();
}

std::list<Item *>::const_iterator Item::getEnd() {
    return lcontained.end();
}

std::list<unsigned long>::iterator Item::getPlayers() {
    return lcreatures.begin();
}

std::list<unsigned long>::iterator Item::getPlayersEnd() {
    return lcreatures.end();
}

bool Item::isBlockingProjectile() const {
	return items[id].blockingProjectile;
}

bool Item::isBlockingWalk() const {
	return items[id].blockingWalk;
}

bool Item::isBlocking() const {
	return items[id].blocking;
}

bool Item::isStackable() const {
	return items[id].stackable;
}

bool Item::isRune() const {
	if(id > 1610 && id < 1667)
	   return true;
	else
       return false;   
}

bool Item::isMultiType() const {
	return items[id].multitype;
}

bool Item::isAlwaysOnTop() const {
	return items[id].alwaysOnTop;
}

bool Item::isPickupable() const {
	return items[id].pickupable;
}

bool Item::isNotMoveable() const {
    if(isContainerQuest()) return true;
	return items[id].notMoveable;
}

bool Item::isGroundTile() const {
	return items[id].groundtile;
}

bool Item::isContainer() const {
	return items[id].iscontainer;
}

bool Item::isContainerQuest() const {
    if(!(strcmp(questName.c_str(), "none") == 0)) 
	   return true;
	return false;
}

bool Item::isWeapon() const
{
  return (items[id].weaponType != NONE && items[id].weaponType != SHIELD && items[id].weaponType != AMO && items[id].weaponType != ARMOR);
}

bool Item::isShield() const
{
  return (items[id].weaponType == SHIELD);
}

bool Item::isArmor() const
{
  return (items[id].weaponType == ARMOR);
}

bool Item::isAmmo() const
{
  return (items[id].weaponType == AMO);
}

WeaponType Item::getWeaponType() const {
		  return items[id].weaponType;
}

std::string Item::getDescription() 
{
  std::stringstream s;
  std::string str;
  if (items[id].name.length())
  {
    s << "You see a " << items[id].name;
    if(isArmor())
       s << " (Arm:" << items[id].arm <<")";
    else if(isWeapon() || isShield())
       s << " (Atk:" << items[id].attack <<" def:" << items[id].defence <<")";
    
  }else if(id == 1018 || id == 1019 || id == 1020 || id == 1021){
           if(!house)
              s<<"You see an experience gate. Only gamemasters and the and a person with a qualified level can enter there.";
           else{
              s << house->getDescription().c_str(); 
           }   
  }else
    s<<"You see an item of type " << id <<".\n";
    if(id > 1610 && id < 1667)
        s << " (" << (int)chargecount << "x).";
    if(isStackable()) 
        s<<"These are "<< (int)count << " pieces.";
    if(items[id].weight > 0 )
        s << "\nIt weights " << items[id].weight << " oz.\n";
    s << items[id].description;
	str = s.str();
	return str;
}

std::string Item::getName()
{
	return items[id].name;
}

// Add item into the container
Item& Item::operator<<(Item* toAdd) {
    addItem(toAdd);
    return *this;
}
