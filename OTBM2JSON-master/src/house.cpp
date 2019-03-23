////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <stdio.h>

#include "house.h"
#include "tile.h"
#include "item.h"


extern Map gmap;

House::House(const char *arfq, const unsigned int ido)
{           
     arquivo = arfq;
     id = ido;
     person = "";
     city = "";
     name = "";
     loadHouseXml();
                   
}                  
	  
int House::loadHouseXml(){   
	xmlDocPtr doc;
	xmlNodePtr root, item, tile;

	xmlLineNumbersDefault(1);
	doc=xmlParseFile(arquivo.c_str());
	if (!doc) {
       std::cout << "FATAL: couldnt load house exiting" << std::endl;
       return 0;
    }
	root=xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*) "house")){
       xmlFreeDoc(doc);
       std::cout << "FATAL: couldnt load house exiting" << std::endl;
       return 0;
    
    }
    if((char*)xmlGetProp(root, (xmlChar*)"name"))
        name = (char*)xmlGetProp(root, (xmlChar*)"name");
    if((char*)xmlGetProp(root, (xmlChar*)"person"))
        person = (char*)xmlGetProp(root, (xmlChar*)"person");
    if((char*)xmlGetProp(root, (xmlChar*)"city"))
        city = (char*)xmlGetProp(root, (xmlChar*)"city");
         
	tile=root->children;
	while(tile)
    {  
            std::string str=(char*)tile->name;    
	        if(str=="tile")
            {                     
			    int x = atoi((const char*)xmlGetProp(tile, (const xmlChar *) "x"));
                int y = atoi((const char*)xmlGetProp(tile, (const xmlChar *) "y"));
                Tile *t = gmap.getTile(x, y, 7);
                listTiles.push_front(t); 
                item=tile->children;
                while(item){
                      std::string str=(char*)item->name;
                      if(str=="item")
                      {
                           int id = atoi((const char*)xmlGetProp(item, (const xmlChar *) "id"));                          
					       Item* myitem=new Item(id);
					       if (t)
						   {
							if (myitem->isAlwaysOnTop())
								t->topItems.push_back(myitem);
							else
								t->downItems.push_back(myitem);
						   }
						   if(myitem && myitem->isContainer()){
                               loadContainerXml(item, myitem); 
                           }          
					  }
                      else if(str=="rune")
                      {
                           int id = atoi((const char*)xmlGetProp(item, (const xmlChar *) "id"));
                           int runex = atoi((const char*)xmlGetProp(item, (const xmlChar *) "x"));                          
					       Item* myitem=new Item(id, runex);
					       if (t)
						   {    
							if (myitem->isAlwaysOnTop())
								t->topItems.push_back(myitem);
							else
								t->downItems.push_back(myitem);
						   }
					  }
					  else if(str=="stackable")
                      {
                           int id = atoi((const char*)xmlGetProp(item, (const xmlChar *) "id"));
                           int stackx = atoi((const char*)xmlGetProp(item, (const xmlChar *) "x"));                          
					       Item* myitem=new Item(id, stackx);
					       if (t)
						   {    
							if (myitem->isAlwaysOnTop())
								t->topItems.push_back(myitem);
							else
								t->downItems.push_back(myitem);
						   }
					  }
                      item=item->next;     
                }             
            }else if(str=="door"){
                  int x = atoi((const char*)xmlGetProp(tile, (const xmlChar *) "x"));    
                  int y = atoi((const char*)xmlGetProp(tile, (const xmlChar *) "y"));
                  Door* door = new Door(Position(x,y,7));
                  listDoors.push_front(door);
                  Tile *doortile = gmap.getTile(x, y, 7);
                  item=tile->children;
                  
                          ItemVector::iterator iit;
	                      for (iit = doortile->topItems.begin(); iit != doortile->topItems.end(); iit++)
	                      {
		                     if ((*iit) && ((*iit)->getID() == 1018 || (*iit)->getID() == 1019 || (*iit)->getID() == 1020 || (*iit)->getID() == 1021)){
                                 (*iit)->house = this;
                                 doortile->setHouseDoor();
                                 break;
		                     }
                          }
                          
                  while(item){
                      std::string str=(char*)item->name;
                      if(str=="player" && doortile)
                      {           
                          char* pname = (char*)xmlGetProp(item, (xmlChar*)"name");
                          door->addName(pname);      
                      }
                      item=item->next;               
                  }              
            }         
            tile=tile->next;
	}
    std::cout << "\nLoaded House \"" << name.c_str() << "\" with "<< listTiles.size() << " tiles and " << listDoors.size() << " doors." << std::endl;	
    xmlFreeDoc(doc);
	return 0;
}

int House::loadContainerXml(xmlNodePtr container, Item* item){
      xmlNodePtr subcontainer;
      if(item->isContainer()){                    
           subcontainer=container->children;                     
           while(subcontainer){
                Item* subiditem = NULL;
                Item* iditem = NULL;
                if (strcmp((const char*)subcontainer->name, "item") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));                  
                           iditem = new Item(subitem_id);
                           item->addItem(iditem);
                }else if (strcmp((const char*)subcontainer->name, "rune") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));
                           int x = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"x"));                  
                           iditem = new Item(subitem_id, x);
                           item->addItem(iditem);
                }else if (strcmp((const char*)subcontainer->name, "stackable") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));
                           int x = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"x"));                  
                           iditem = new Item(subitem_id, x);
                           item->addItem(iditem);                      
                }else if(strcmp((const char*)subcontainer->name, "container") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));                  
                           iditem = new Item(subitem_id);
                           item->addItem(iditem);
                           loadContainerXml(subcontainer, iditem);
                }
                subcontainer=subcontainer->next;    
          }       
      }
}

std::string House::getDescription(){
    std::stringstream s;
	std::string str;	
    s <<  "You see " << name << ". " << person << " owns this house.";
	str = s.str();
	return str;
}

void House::saveHouseXml()
{  
    std::stringstream sb; 
      
    xmlDocPtr doc;
	xmlNodePtr nn, sn, pn, root, bp, subbp;
	doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"house", NULL);
	root = doc->children;
       
	sb << name;  	           xmlSetProp(root, (const xmlChar*) "name", (const xmlChar*)sb.str().c_str());     sb.str("");
	sb << person;              xmlSetProp(root, (const xmlChar*) "person", (const xmlChar*)sb.str().c_str());	sb.str("");
	sb << city;                xmlSetProp(root, (const xmlChar*) "city", (const xmlChar*)sb.str().c_str());	sb.str("");
	
    for (std::list<Door *>::iterator cit = listDoors.begin(); cit != listDoors.end(); cit++)
    {
       Door* door = dynamic_cast<Door*>(*cit);
       if(door){
         sn = xmlNewNode(NULL,(const xmlChar*)"door");
         sb << door->pos.x;                           
         xmlSetProp(sn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());            
         sb.str("");
         sb << door->pos.y;                
         xmlSetProp(sn, (const xmlChar*) "y", (const xmlChar*)sb.str().c_str());            
         sb.str(""); 
         door->saveNames(pn, sn, sb);
         xmlAddChild(root, sn);
       }   
    }
	for (std::list<Tile *>::iterator cit = listTiles.begin(); cit != listTiles.end(); cit++)
	{
      Tile *tile = dynamic_cast<Tile*>(*cit);
      if(tile){ 
          sn = xmlNewNode(NULL,(const xmlChar*)"tile");
          sb << tile->pos.x;                          
          xmlSetProp(sn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());            
          sb.str("");
          sb << tile->pos.y;                            
          xmlSetProp(sn, (const xmlChar*) "y", (const xmlChar*)sb.str().c_str());            
          sb.str("");
          ItemVector::iterator iit;
          for (iit = tile->downItems.begin(); iit != tile->downItems.end(); iit++){
            Item* item = dynamic_cast <Item*>((*iit));
            if(item){        
              if(item->isRune())
              {
                pn = xmlNewNode(NULL,(const xmlChar*)"rune");
    	        sb << item->getID();                             
                xmlSetProp(pn, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
                sb.str("");
                sb << (int)item->getItemCharge();
                xmlSetProp(pn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());            
                sb.str("");
	            xmlAddChild(sn, pn);
              }
              else if(!item->isContainer() && (item->isStackable() || item->isMultiType()))
              {
                pn = xmlNewNode(NULL,(const xmlChar*)"stackable");
    	        sb << item->getID();                             
                xmlSetProp(pn, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
                sb.str("");
                sb << (int)item->getItemCountOrSubtype();
                xmlSetProp(pn, (const xmlChar*) "x", (const xmlChar*)sb.str().c_str());            
                sb.str("");
	            xmlAddChild(sn, pn);
              }
              else if(item->isContainer())
              {   
                pn = xmlNewNode(NULL,(const xmlChar*)"item");
                sb << item->getID();                             
                xmlSetProp(pn, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
                sb.str("");         
          
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
                    saveContainerXml(newitem, bp, sb);                              
                    xmlAddChild(pn, bp);
                  }else if(item->getItem(a)){                                      
                    bp = xmlNewNode(NULL,(const xmlChar*)"item");
                    sb << item->getItem(a)->getID();
                    xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
                    sb.str("");
                    xmlAddChild(pn, bp);
                  }
                }
	            xmlAddChild(sn, pn);
              }  
              else
              {
    	        pn = xmlNewNode(NULL,(const xmlChar*)"item");
    	        sb << item->getID();                             
                xmlSetProp(pn, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
                sb.str("");
	            xmlAddChild(sn, pn);
              }
            }       
          }      
       xmlAddChild(root, sn);
       }
    }
   if (xmlSaveFile(arquivo.c_str(), doc)){
       /*#ifdef __DEBUG__
       std::cout << ":: Saved house succefully!\n";
       #endif*/
       xmlFreeDoc(doc);
   }else{
       std::cout << ":: Couldn't save house =(\n";
       xmlFreeDoc(doc);
   }  
}

void House::saveContainerXml(Item* item, xmlNodePtr pn, std::stringstream &sb)
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
              saveContainerXml(newitem, bp, sb);                           
              xmlAddChild(pn, bp);
            }else if(item->getItem(a)){                                      
              bp = xmlNewNode(NULL,(const xmlChar*)"item");
              sb << item->getItem(a)->getID();
              xmlSetProp(bp, (const xmlChar*) "id", (const xmlChar*)sb.str().c_str());            
              sb.str("");
              xmlAddChild(pn, bp);
            }
          }
}

bool House::isDoorName(Position pos, std::string name)
{
     if(person == name)
       return true;
       
     Door* targetDoor = NULL;
     for (std::list<Door *>::iterator cit = listDoors.begin(); cit != listDoors.end(); cit++)
     {
       Door* door = dynamic_cast<Door*>(*cit);
       if(door && door->pos == pos){
          targetDoor = door;
          break;
       }   
     }
     if(targetDoor && targetDoor->searchName(name))
        return true;
     return false;   
}

bool House::addDoorName(Position pos, std::string name)
{
     Door* targetDoor = NULL;
     for (std::list<Door *>::iterator cit = listDoors.begin(); cit != listDoors.end(); cit++)
     {
       Door* door = dynamic_cast<Door*>(*cit);
       if(door && door->pos == pos){
          targetDoor = door;
          break;
       }   
     }
     if(targetDoor){
        targetDoor->addName(name);
        return true;
     }   
     return false;   
}

bool House::removeDoorName(Position pos, std::string name)
{
     Door* targetDoor = NULL;
     for (std::list<Door *>::iterator cit = listDoors.begin(); cit != listDoors.end(); cit++)
     {
       Door* door = dynamic_cast<Door*>(*cit);
       if(door && door->pos == pos){
          targetDoor = door;
          break;
       }   
     }
     if(targetDoor){
        if(targetDoor->removeName(name))
           return true;
     }   
     return false;   
}

void House::getDoorNames(Position pos, std::stringstream &names)
{
     Door* targetDoor = NULL;
     for (std::list<Door *>::iterator cit = listDoors.begin(); cit != listDoors.end(); cit++)
     {
       Door* door = dynamic_cast<Door*>(*cit);
       if(door && door->pos == pos){
          targetDoor = door;
          break;
       }   
     }
     if(targetDoor){
        targetDoor->getNames(names);
     }   
}

void House::clearAllDoorsNames()
{
     for (std::list<Door *>::iterator cit = listDoors.begin(); cit != listDoors.end(); cit++)
     {
       Door* door = dynamic_cast<Door*>(*cit);
       if(door){
          door->clearNames();
       }   
     }  
}


