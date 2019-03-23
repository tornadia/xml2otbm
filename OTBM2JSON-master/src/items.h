////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __SURVIVALSERV_ITEMS_H
#define __SURVIVALSERV_ITEMS_H

#include <map>
#include <string>

enum WeaponType 
{
  NONE, SWORD, CLUB, AXE, DIST/*, MAGIC*/, AMO, SHIELD, ARMOR
};

class ItemType {
public:
	ItemType();
	~ItemType();
	
    std::string position; 
    short int      handed;
	unsigned short id;
    unsigned short maxItems;
	unsigned short weight;
	
    std::string    name;		   // the name of the item
	std::string description;	// additional description... as in "The blade is a magic flame." for fireswords
  
  WeaponType     weaponType;
  unsigned short attack;
  unsigned short defence;
  unsigned short arm;
  unsigned short decayTo;
  unsigned short decayTime;
  unsigned short damage;
  uint8_t speed;

	// Other bools
	bool iscontainer;
	bool stackable;
	bool multitype;
	bool useable;
	bool notMoveable;
	bool alwaysOnTop;
	bool groundtile; 
	bool blocking;
	bool blockingWalk;
	bool pickupable;
	bool blockingProjectile;
};


class Items {
public:
	Items();
	~Items();
	
	int loadFromDat(std::string);
  int loadXMLInfos(std::string);
	
	const ItemType& operator[](int id);
		 
protected:
	typedef std::map<unsigned short, ItemType*> ItemMap;
	ItemMap items;

	ItemType dummyItemType; // use this for invalid ids
};

#endif









