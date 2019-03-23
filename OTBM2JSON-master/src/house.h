////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __HOUSE_H
#define __HOUSE_H

#include <string>
#include <sstream>
#include <stdio.h>

#include "map.h"
#include "position.h"

//Defines a house

class Item;
class Tile;

class Door{
  public:    
    Door(Position position) { pos = position; };
    ~Door() {};
	Position pos;
	void addName(std::string name)
    {
         listNames.push_front(name);
    };
    bool removeName(std::string name)
    {
         for (std::list<std::string>::iterator cit = listNames.begin(); cit != listNames.end(); cit++){
		   if((*cit) == name){
			 listNames.erase(cit);
			 return true;
			 break;
		   }
	     }
	     return false;
    };
    bool searchName(std::string name)
    {
         for (std::list<std::string>::iterator cit = listNames.begin(); cit != listNames.end(); cit++){
		   if((*cit) == name){
			 return true;
			 break;
		   }
	     }
	     return false;
    };
    void saveNames(xmlNodePtr pn, xmlNodePtr sn, std::stringstream &sb)
    {
         for(std::list<std::string>::iterator cit = listNames.begin(); cit != listNames.end(); cit++)
         {                               
           if((*cit) == "") 
             continue;        
	       pn = xmlNewNode(NULL,(const xmlChar*)"player");
	       sb << (*cit);     xmlSetProp(pn, (const xmlChar*) "name", (const xmlChar*)sb.str().c_str());      sb.str("");
	       xmlAddChild(sn, pn);
        }
    };
    void getNames(std::stringstream &names)
    {    
         names << "Door list: ";
         for(std::list<std::string>::iterator cit = listNames.begin(); cit != listNames.end(); cit++)
         {                               
           if((*cit) == "") 
             continue;
           if(cit == listNames.begin())
             names << (*cit).c_str();
           else   
	         names << ", " << (*cit).c_str();
        }
        names << ".";
    };
    void clearNames() { listNames.clear(); };     
  protected:
	std::list<std::string> listNames;
};

class House{
  public:
    House(const char *arfq, const unsigned int ido);
    ~House() {};
    
    unsigned int id;
    std::string person;
    std::string name;
    std::string city;
    
    std::string getDescription();
    
    int getDoors() { return listDoors.size(); };
    int getTiles() { return listTiles.size(); };
    
    bool isDoorName(Position pos, std::string name);
    bool addDoorName(Position pos, std::string name);
    bool removeDoorName(Position pos, std::string name);
    void getDoorNames(Position pos, std::stringstream &names);
    void clearAllDoorsNames();
    
    void saveHouseXml();
    
  protected:                  
    std::string arquivo; 
    
    std::list<Tile *> listTiles;
    std::list<Door *> listDoors;
    
    int loadHouseXml();
    int loadContainerXml(xmlNodePtr container, Item* item);
    
    void saveContainerXml(Item* item, xmlNodePtr pn, std::stringstream &sb);                
};

#endif




