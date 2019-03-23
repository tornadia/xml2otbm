////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include "definitions.h"
#include "items.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <iostream>

ItemType::ItemType()
{
	iscontainer     = false;
	stackable       = false;
    multitype       = false;
	useable	        = false;
	notMoveable     = false;
	alwaysOnTop     = false;
	groundtile      = false;
	blocking        = false;
	pickupable      = false;
	blockingWalk    = false;
	blockingProjectile = false;
	
	speed	   = 0;
	id         =  100;
	maxItems   =    8;
	weight     =   0;
    weaponType = NONE;
    attack     =    0;
    defence    =    0;
    arm        =    0;
    handed     = 1;
    decayTo    =    0;
    decayTime  =   60;
	damage     = 0;
}

ItemType::~ItemType()
{
}


Items::Items()
{
}

Items::~Items()
{
	for (ItemMap::iterator it = items.begin(); it != items.end(); it++)
		delete it->second;
}


int Items::loadFromDat(std::string file)
{
	int id = 100;  // survival.dat start with id 100
	
	FILE* f=fopen(file.c_str(), "rb");
	
	if(!f){
	#ifdef __DEBUG__
	std::cout << "FAILED!" << std::endl;
	#endif
		return -1;
	}
	
	fseek(f,0,SEEK_END);
	long size=ftell(f);

#ifdef __DEBUG__
	bool warningwrongoptordershown = false;
#endif
	
	fseek(f, 0x0C, SEEK_SET);
	// loop throw all Items until we reach the end of file
	while(ftell(f) < size)
	{
		ItemType* iType= new ItemType();
		iType->id	  = id;

#ifdef __DEBUG__
		int lastoptbyte = 0;
#endif
		// read the options until we find a 0xff
		int optbyte;
		while (((optbyte = fgetc(f)) >= 0) &&   // no error
				   (optbyte != 0xFF))			    // end of options
		{
#ifdef __DEBUG__
			if (optbyte < lastoptbyte)
			{
			 	if (!warningwrongoptordershown)
				{
					std::cout << "WARNING! Unexpected option order in file survival.dat." << std::endl;
					warningwrongoptordershown = true;
				}
			}
			lastoptbyte = optbyte;
#endif
			switch (optbyte)
			{
	   		case 0x00:
		   		//is groundtile	   				
    			iType->groundtile = true;
    			iType->speed=(int)fgetc(f);
				if(iType->speed==0) {
                  iType->blocking=true;
                  }    
                  fgetc(f);
		   		break;

        case 0x01: // all OnTop
        case 0x02: // can walk trough (open doors, arces, bug pen fence ??)
          iType->alwaysOnTop=true;
          break;

				case 0x03:
					//is a container
					iType->iscontainer=true;
					break;	   			

				case 0x04:
					//is stackable
					iType->stackable=true;
					break;

				case 0x05:
					//is useable
					iType->useable=true;
					break;

				case 0x0A:
					//is multitype !!! wrong definition (only water splash on floor)
                    iType->multitype=true;
					break;

				case 0x0B:
					//is blocking
					iType->blocking=true;
					break;
				
				case 0x0C:
					//is on moveable
                    iType->notMoveable=true;
					break;
	
				case 0x0F:
					//can be equipped
					iType->pickupable=true;
					break;

				case 0x10:
					//makes light (skip 4 bytes)
                    fgetc(f); //number of tiles around
                    fgetc(f); // always 0
                    fgetc(f); // 215 items, 208 fe non existant items other values
                    fgetc(f); // always 0
					break;

        case 0x06: // ladder up (id 1124)   why a group for just 1 item ???  
            break;
        case 0x09: //can contain fluids
            break;
        case 0x0D: // blocks missiles (walls, magic wall etc)
                    iType->blockingProjectile = true;
            break;
        case 0x0E: // blocks monster movement (flowers, parcels etc)
                    iType->blockingWalk = true;
            break;
        case 0x11: // can see what is under (ladder holes, stairs holes etc)
            break;
        case 0x12: // tiles that don't cause level change
            break;
        case 0x18: // cropses that don't decay
            break;
        case 0x19: // monster has animation even when iddle (rot, wasp, slime, fe)
            break;
        case 0x14: // player color templates
            break;

				case 0x07: // writtable objects
                    fgetc(f); //max characters that can be written in it (0 unlimited)
                    fgetc(f); //max number of  newlines ? 0, 2, 4, 7
				    break;
				case 0x08: // writtable objects that can't be edited 
                    fgetc(f); //always 0 max characters that can be written in it (0 unlimited) 
                    fgetc(f); //always 4 max number of  newlines ? 
				    break;
				case 0x13: // mostly blocking items, but also items that can pile up in level (boxes, chairs etc)
                    fgetc(f); //always 8
                    fgetc(f); //always 0
                    break;
				case 0x16: // ground, blocking items and mayby some more
                    fgetc(f); //12, 186, 210, 129 and other.. 
                    fgetc(f); //always 0
				    break;
				case 0x1A: 
                    fgetc(f); //action that can be performed (doors-> open, hole->open, book->read) not all included ex. wall torches
                    fgetc(f); //always 4
				    break;
				default:
						std::cout << "Unknown byte: " << (unsigned short)optbyte << std::endl;
			}
		}
		
		// now skip the size and sprite data		
 		int width  = fgetc(f);
 		int height = fgetc(f);
 		if ((width > 1) || (height > 1))
 		   int skip = fgetc(f);
 		   
		int blendframes = fgetc(f);
		int xdiv        = fgetc(f);
		int ydiv        = fgetc(f);
		int animcount   = fgetc(f);

	  	fseek(f, width*height*blendframes*xdiv*ydiv*animcount*2, SEEK_CUR);

	  	// store the found item	  	
		items[id] = iType;
 		id++;
   	}
   	
   	fclose(f);

	return 0;
}

int Items::loadXMLInfos(std::string file)
{
	xmlDocPtr doc;
	doc = xmlParseFile(file.c_str());

	if (doc) {
		xmlNodePtr root, p;
		root = xmlDocGetRootElement(doc);

		if (xmlStrcmp(root->name, (const xmlChar*)"items")) {
			xmlFreeDoc(doc);
			return -1;
		}

		p = root->children;
		while (p) {
			std::string elem = (char*)p->name;
			if (elem == "item" && xmlGetProp(p, (xmlChar*)"id")) {
				// get the id...
				int id = atoi((const char*)xmlGetProp(p, (xmlChar*)"id"));
				// now let's find the item and load it's definition...
				ItemMap::iterator it = items.find(id);
				if ((it != items.end()) && (it->second != NULL))
        {
					ItemType *itemtype = it->second;

					// set general properties...
					char* name = (char*)xmlGetProp(p, (xmlChar*)"name");
          if (name)
						itemtype->name = name;
          else
            std::cout << "Missing name tag for item: " << id << std::endl;

          char* weight = (char*)xmlGetProp(p, (xmlChar*)"weight");
          if (weight)
						itemtype->weight = atoi(weight);
          else
            std::cout << "Missing weight tag for item: " << id << std::endl;

					// and optional properties
          char* description = (char*)xmlGetProp(p, (xmlChar*)"descr");
          if (description)
						itemtype->description = description;
						
          char* decayTo = (char*)xmlGetProp(p, (xmlChar*)"decayto");
          if (decayTo)
						itemtype->decayTo = atoi(decayTo);

          char* decayTime = (char*)xmlGetProp(p, (xmlChar*)"decaytime");
          if (decayTime)
						itemtype->decayTime = atoi(decayTime);
		
					char* damage = (char*)xmlGetProp(p, (xmlChar*)"damage");
					if(damage)
						itemtype->damage = atoi(damage);

					// now set special properties...
					// first we check the type...
					char* type = (char*)xmlGetProp(p, (xmlChar*)"type");
				  if (type)
          {
            if (!strcmp(type, "container"))
            {
							// we have a container...
							// TODO set that we got a container
              char* maxitems = (char*)xmlGetProp(p, (xmlChar*)"maxitems");
							if (maxitems)
                itemtype->maxItems = atoi(maxitems);
              else
								std::cout << "Item " << id << " is a container but lacks a maxitems definition." << std::endl;

						}
            else if (!strcmp(type, "weapon"))
            {
							// we have a weapon...
							// find out which type of weapon we have...
              char *skill = (char*)xmlGetProp(p, (xmlChar*)"skill");
              if (skill)
              {
						    if (!strcmp(skill, "sword"))
								  itemtype->weaponType = SWORD;
							  else if (!strcmp(skill, "club"))
							    itemtype->weaponType = CLUB;
							  else if (!strcmp(skill, "axe"))
								  itemtype->weaponType = AXE;
							  else if (!strcmp(skill, "distance"))
								  itemtype->weaponType = DIST;
							  /*else if (!strcmp(skill, "magic"))
								  itemtype->weaponType = MAGIC*/
							  else if (!strcmp(skill, "shielding"))
								  itemtype->weaponType = SHIELD;
                else
                  std::cout << "Wrong skill tag for weapon" << std::endl;
              }
              else
								std::cout << "Missing skill tag for weapon" << std::endl;

							char* attack = (char*)xmlGetProp(p, (xmlChar*)"attack");
              if (attack)
								itemtype->attack = atoi(attack);
              else if(itemtype->weaponType != DIST)
								std::cout << "Missing attack tag for weapon: " << id << std::endl;

							char* defence = (char*)xmlGetProp(p, (xmlChar*)"defence");
							if (defence)
                itemtype->defence = atoi(defence);
              else if(itemtype->weaponType != DIST)
								std::cout << "Missing defence tag for weapon: " << id << std::endl;
                                itemtype->handed = 1; 
                                char* handed = (char*)xmlGetProp(p, (xmlChar*)"handed"); 

                                if (handed) 
                                itemtype->handed = atoi(handed);
						}
            else if (!strcmp(type, "amunition"))
            {
							// we got some amo
							itemtype->weaponType = AMO;
						}
            else if (!strcmp(type, "armor"))
            {
							// armor...
							char* position = (char*)xmlGetProp(p, (xmlChar*)"position"); 
                            if(position) 
                                itemtype->position = position; 
                            else 
                                 std::cout << "Missing position for armor: " << id << std::endl;
							itemtype->weaponType = ARMOR;
							char* arme = (char*)xmlGetProp(p, (xmlChar*)"arm");
							if(arme)
							   itemtype->arm = atoi(arme);
							else
                               itemtype->arm = 0;   
						}
					}
				} else {
						  std::cout << "Invalid item " << id << std::endl;
				}

			}
			p = p->next;
		}

		xmlFreeDoc(doc);

		return 0;
	}
	return -1;
}

const ItemType& Items::operator[](int id)
{
	ItemMap::iterator it = items.find(id);
	if ((it != items.end()) && (it->second != NULL))
	  return *it->second;

	#ifdef __DEBUG__
	std::cout << "WARNING! unknown itemtypeid " << id << ". using defaults." << std::endl;
	#endif
	   
	return dummyItemType;
}	

