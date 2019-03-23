////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __SURVIVALSERV_ITEM_H
#define __SURVIVALSERV_ITEM_H

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <iostream>
#include <list>
#include <vector>

#include "thing.h"
#include "items.h"

#include "player.h"
#include "networkmessage.h"

class House;
class Creature;
class Player;
class NetworkMessage;

class Item : public Thing
{
    private:
        std::string questName;    
        unsigned id;  // The same id as in ItemType
	    unsigned char count; // Number of stacked items
		unsigned char chargecount; // Number of charges on the item
		unsigned short maxitems; // Number of max items in container  
		unsigned short actualitems; // Number of items in container
		unsigned short actualicreatures; // Number of creatures with this container open
        std::list<Item *> lcontained; // List of items if this is a container
        std::list<unsigned long> lcreatures; // List of creatures with this container open

    public:
		static Items items;
        unsigned short getID() const; // ID as in ItemType
        void setID(unsigned short newid);
	
        House *house;
    	
    	// Equipament type
        bool isWeapon() const;
        bool isShield() const;    
        bool isAmmo() const;
        bool isArmor() const;
    
	   WeaponType getWeaponType() const;
       int getHanded() const;
        
        //Configs of the Item
        bool isBlockingProjectile() const;
        bool isBlockingWalk() const;
        bool isRune() const;
        bool isBlocking() const;
		bool isStackable() const;
        bool isMultiType() const;
		bool isAlwaysOnTop() const;
		bool isGroundTile() const;
		bool isNotMoveable() const;
		bool isPickupable() const;
		bool isContainer() const;
        bool isContainerQuest() const;
        
		int use(){std::cout << "Use " << id << std::endl; return 0;};
		int use(Item*){std::cout << "Use with item ptr " << id << std::endl; return 0;};
		int use(Creature*){std::cout << "Use with creature ptr " << id << std::endl; return 0;};
		
        int unserialize(xmlNodePtr p);
		//xmlNodePtr serialize();
		
        std::string getDescription();
		std::string getName();
		std::string getQuestName() {return questName;};
		void setQuestName(std::string name) {questName = name;};
		
        // Get the number of items
           unsigned char getItemCountOrSubtype() const;
		   void setItemCountOrSubtype(unsigned char n) {count = n;};
		   unsigned char getItemCharge() const {return chargecount;};
		   void setItemCharge(unsigned char n) {chargecount = n;};

        // Constructor for items
          Item(const unsigned short _type);
          Item(const unsigned short _type, unsigned char _count);
		  Item();
         ~Item();

        // Definition for iterator over backpack itemsfclose(f);
				int getContainerItemCount() {return actualitems;};
				int getContainerMaxItemCount() {return maxitems;};
        std::list<Item *>::const_iterator getItems();     // Begin();
        std::list<Item *>::const_iterator getEnd();       // Tterator beyond the last element
                void addItem(Item* newitem);     // Add an item to the container
				void removeItem(Item* item); // Remove an item from the container
				void moveItem(unsigned char from_slot, unsigned char to_slot); //Move a item to a slot in the container
				unsigned char getSlotNumberByItem(Item* item); //Get the slot by an item
				void isContainerHolding(Item* item, bool& found); //Search all containers for the item recursively
				void findItem(Item* item, bool& found); //Search all containers for the item recursively
                Item* getItem(unsigned long slot_num); // Get Item of a Slot
                Item& operator<<(Item*); // Put items into the container
        //Creatures with container open
                void addPlayer(Creature* newcreature);     // Add an item to the container
				void removePlayer(Creature* creature); // Remove an item from the container 
                void closePlayersContainer();
                std::list<unsigned long>::iterator getPlayers();     // Begin();
                std::list<unsigned long>::iterator getPlayersEnd();       // End();      
};


#endif
