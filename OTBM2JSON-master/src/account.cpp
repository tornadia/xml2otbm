////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <iostream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "definitions.h"
#include "item.h"
#include "account.h"
#include "player.h"

Account::Account()
{
}


Account::~Account()
{
}


bool Account::openAccount(const std::string &account, const std::string &givenpassword)
{
  std::string filename = "data/accounts/" + account + ".xml";
  std::transform(filename.begin(), filename.end(), filename.begin(), tolower);

  xmlDocPtr doc = xmlParseFile(filename.c_str());

  if (doc)
  {
    xmlNodePtr root, p, tmp;
    root = xmlDocGetRootElement(doc);

    if (xmlStrcmp(root->name,(const xmlChar*) "account"))
    {
      xmlFreeDoc(doc);
      return false;
    }

    p = root->children;

    const char* pwd = (const char*)xmlGetProp(root, (const xmlChar *)"pass");

    if ((pwd == NULL) || (givenpassword != pwd)) {
      xmlFreeDoc(doc);
      return false;
    }

    password  = pwd;

    accType   = atoi((const char*)xmlGetProp(root, (xmlChar*)"type"));
    premDays  = atoi((const char*)xmlGetProp(root, (xmlChar*)"premDays"));

    while (p)
    {
      const char* str = (char*)p->name;

      if (strcmp(str, "characters") == 0)
      {
        tmp = p->children;
        while(tmp)
        {
          const char* temp_a = (const char*)xmlGetProp(tmp, (xmlChar*)"name");

          if ((strcmp((const char*)tmp->name, "character") == 0) && (temp_a != NULL))
            charList.push_back(temp_a);

          tmp = tmp->next;
        }
      }
      p = p->next;
    }
    xmlFreeDoc(doc);
    //charList.sort();

    return true;
  }

  return false;
}


bool Account::openPlayer(const std::string &name, const std::string &givenpassword, Player &player)
{
  std::string filename="data/players/"+name+".xml";
  std::transform (filename.begin(),filename.end(), filename.begin(), tolower);

  xmlDocPtr doc;
  doc = xmlParseFile(filename.c_str());

  if (doc)
  {
    xmlNodePtr root, tmp, p, slot, container, subcontainer;
    root=xmlDocGetRootElement(doc);

    if (xmlStrcmp(root->name,(const xmlChar*) "player"))
    {
      std::cout << "Strange. Player-Savefile was no savefile for " << name << std::endl;
    }

    p = root->children;

    const char *account = (const char*)xmlGetProp(root, (const xmlChar *) "account");
    if (!openAccount(account, givenpassword)) {
      xmlFreeDoc(doc);
      return false;
    }
    if(premDays > 0)
      player.premmy = true;
                  
    player.accountNumber = atoi((const char*)xmlGetProp(root, (const xmlChar *) "account"));
    player.accountPassword = password;
    player.sex=atoi((const char*)xmlGetProp(root, (const xmlChar *) "sex"));
    if((const char*)xmlGetProp(root, (const xmlChar *) "lookdir"))
       player.setDirection((Direction)atoi((const char*)xmlGetProp(root, (const xmlChar *) "lookdir")));
    player.givenxp = (player.experience*0.05);
    player.experience=atoi((const char*)xmlGetProp(root, (const xmlChar *) "exp"));
    if((const char*)xmlGetProp(root, (const xmlChar *) "cash"))
        player.cash=atoi((const char*)xmlGetProp(root, (const xmlChar *) "cash"));
    player.level=atoi((const char*)xmlGetProp(root, (const xmlChar *) "level"));
    player.maglevel=atoi((const char*)xmlGetProp(root, (const xmlChar *) "maglevel"));
    player.voc=atoi((const char*)xmlGetProp(root, (const xmlChar *) "voc"));
    player.access=atoi((const char*)xmlGetProp(root, (const xmlChar *) "access"));
    if((const char*)xmlGetProp(root, (const xmlChar *) "cap"))
       player.cap=atoi((const char*)xmlGetProp(root, (const xmlChar *) "cap"));
    if((const char*)xmlGetProp(root, (const xmlChar *) "pkills"))
       player.pled=atoi((const char*)xmlGetProp(root, (const xmlChar *) "pkills"));
    if((const char*)xmlGetProp(root, (const xmlChar *) "mkills"))
       player.mled=atoi((const char*)xmlGetProp(root, (const xmlChar *) "mkills"));
    if((const char*)xmlGetProp(root, (const xmlChar *) "pktime"))
       player.time=atoi((const char*)xmlGetProp(root, (const xmlChar *) "pktime"));
    player.setNormalSpeed();
    while (p)
    {
      std::string str=(char*)p->name;
      if(str=="mana")
      {
        player.mana=atoi((const char*)xmlGetProp(p, (const xmlChar *) "now"));
        player.manamax=atoi((const char*)xmlGetProp(p, (const xmlChar *) "max"));
        player.manaspent=atoi((const char*)xmlGetProp(p, (const xmlChar *) "spent"));
      }
      else if(str=="health")
      {
        player.health=atoi((const char*)xmlGetProp(p, (const xmlChar *) "now"));
        player.healthmax=atoi((const char*)xmlGetProp(p, (const xmlChar *) "max"));
        player.food=atoi((const char*)xmlGetProp(p, (const xmlChar *) "food"));
      }
      else if(str=="look")
      {
        player.looktype=atoi((const char*)xmlGetProp(p, (const xmlChar *) "type"));
        player.lookmaster = player.looktype;
        player.lookhead=atoi((const char*)xmlGetProp(p, (const xmlChar *) "head"));
        player.lookbody=atoi((const char*)xmlGetProp(p, (const xmlChar *) "body"));
        player.looklegs=atoi((const char*)xmlGetProp(p, (const xmlChar *) "legs"));
        player.lookfeet=atoi((const char*)xmlGetProp(p, (const xmlChar *) "feet"));
      }
      else if(str=="spawn")
      {
        player.pos.x=atoi((const char*)xmlGetProp(p, (const xmlChar *) "x"));
        player.pos.y=atoi((const char*)xmlGetProp(p, (const xmlChar *) "y"));
        player.pos.z=atoi((const char*)xmlGetProp(p, (const xmlChar *) "z"));
      }
      else if(str=="temple")
      {
        player.masterPos.x=atoi((const char*)xmlGetProp(p, (const xmlChar *) "x"));
        player.masterPos.y=atoi((const char*)xmlGetProp(p, (const xmlChar *) "y"));
        player.masterPos.z=atoi((const char*)xmlGetProp(p, (const xmlChar *) "z"));
      }
      else if(str=="guild") 
      { 
        player.guildstatus=atoi((const char*)xmlGetProp(p, (const xmlChar *) "status")); 
        player.guildnicks=atoi((const char*)xmlGetProp(p, (const xmlChar *) "nicks")); 
        player.guildname=(const char*)xmlGetProp(p, (const xmlChar *) "name"); 
        player.guildrank=(const char*)xmlGetProp(p, (const xmlChar *) "rank"); 
        player.guildnick=(const char*)xmlGetProp(p, (const xmlChar *) "nick");
      }
      else if(str=="skills")
      {
        tmp=p->children;
        while(tmp)
        {
          int s_id, s_lvl, s_tries;
          if (strcmp((const char*)tmp->name, "skill") == 0)
          {
            s_id=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "skillid"));
            s_lvl=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "level"));
            s_tries=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "tries"));
            player.skills[s_id][SKILL_LEVEL]=s_lvl;
            player.skills[s_id][SKILL_TRIES]=s_tries;
          }
          tmp=tmp->next;
        }
      }
      else if(str=="inventory")
      {
        slot=p->children;
        while (slot)
        {
          if (strcmp((const char*)slot->name, "slot") == 0)
          {
            int sl_id = atoi((const char*)xmlGetProp(slot, (const xmlChar *)"slotid"));
            Item* myitem = new Item();
            myitem->unserialize(slot->children);
            player.items[sl_id]=myitem;
            
            if(player.items[sl_id] && player.items[sl_id]->isContainer()) {                   
              container=slot->children;                       
              while(container)
              {
                if (strcmp((const char*)container->name, "item2") == 0)
                {
                   int item_id = atoi((const char*)xmlGetProp(container, (const xmlChar *)"id"));                  
                   Item* iditem = new Item(item_id);                                                                            
                   player.items[sl_id]->addItem(iditem);
                }
                if (strcmp((const char*)container->name, "rune") == 0)
                {
                   int item_id = atoi((const char*)xmlGetProp(container, (const xmlChar *)"id"));
                   int x = atoi((const char*)xmlGetProp(container, (const xmlChar *)"x"));                  
                   Item* iditem = new Item(item_id, x);
                   player.items[sl_id]->addItem(iditem);
                }
                else if (strcmp((const char*)container->name, "stackable") == 0)
                {
                   int subitem_id = atoi((const char*)xmlGetProp(container, (const xmlChar *)"id"));
                   int x = atoi((const char*)xmlGetProp(container, (const xmlChar *)"x"));                  
                   Item* iditem = new Item(subitem_id, x);
                   player.items[sl_id]->addItem(iditem);  
                }   
                else if (strcmp((const char*)container->name, "container") == 0)
                {
                   int subitem_id = atoi((const char*)xmlGetProp(container, (const xmlChar *)"id"));                  
                   Item* subiditem = new Item(subitem_id);
                   player.items[sl_id]->addItem(subiditem);
                   AddContainer(container, subiditem);  
                }
                container=container->next;                
              } 
            }                                               
          }
          slot=slot->next;
        }
      }
      p=p->next;
    }
    std::cout << ":: Loaded " << filename << std::endl;
    xmlFreeDoc(doc);

    return true;
  }

  return false;
}

bool Account::AddContainer(xmlNodePtr container, Item* item){
      xmlNodePtr subcontainer;
      if(item->isContainer()){                    
           subcontainer=container->children;                
           while(subcontainer){
                Item* subiditem = NULL;
                Item* iditem = NULL;
                if (strcmp((const char*)subcontainer->name, "item2") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));                  
                           iditem = new Item(subitem_id);
                           item->addItem(iditem);
                }else if (strcmp((const char*)subcontainer->name, "rune") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));
                           int x = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"x"));                  
                           iditem = new Item(subitem_id, x);
                           item->addItem(iditem);           
                }else if(strcmp((const char*)subcontainer->name, "stackable") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));
                           int x = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"x"));                  
                           iditem = new Item(subitem_id, x);
                           item->addItem(iditem);  
                }else if(strcmp((const char*)subcontainer->name, "container") == 0){
                           int subitem_id = atoi((const char*)xmlGetProp(subcontainer, (const xmlChar *)"id"));                  
                           iditem = new Item(subitem_id);
                           item->addItem(iditem);
                           AddContainer(subcontainer, iditem);
                }
                subcontainer=subcontainer->next;    
          }       
      }
}

