////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "definitions.h"
#include "npc.h"
#include "tile.h"
#include "spells.h"
#include "player.h"
#include "networkmessage.h"
#include "map.h"

extern Spells spells;

Npc::Npc(const char *name, Map* map, bool onExp) : Creature(name)
{                    
	this->loaded  = false;
	this->defense = 0;
	this->armor   = 0;
	this->change  = 0;
	this->changeper = 100;
	this->monster = false;
	this->pushable = true;
	this->combat_dist = 1;
	this->runLife = 0;
	this->firstthink = true;
	this->containerLoot = 0;
	std::string filename = "data/npc/" + std::string(name) + ".xml";
	std::transform(filename.begin(), filename.end(), filename.begin(), tolower);
	xmlDocPtr doc = xmlParseFile(filename.c_str());
	if (doc){
		this->loaded=true;
		xmlNodePtr root, p, tmp, tmp2;
		root = xmlDocGetRootElement(doc);

		if (xmlStrcmp(root->name,(const xmlChar*) "npc")){
		    std::cerr << "Malformed XML" << std::endl;
		}

		p = root->children;
        if((const char*)xmlGetProp(root, (const xmlChar *)"monster")) {
			this->monster = (bool)atoi((const char*)xmlGetProp(root, (const xmlChar *)"monster"));
		}
        if((const char*)xmlGetProp(root, (const xmlChar *)"script")){
		    this->scriptname = (const char*)xmlGetProp(root, (const xmlChar *)"script");
        }else{
            this->scriptname = "";
            this->monster = true;
        }    
		if ((const char*)xmlGetProp(root, (const xmlChar *)"name")) {
			this->name = (const char*)xmlGetProp(root, (const xmlChar *)"name");
		}
		if (onExp && (const char*)xmlGetProp(root, (const xmlChar *)"exp")) {
			givenxp = atoi((const char*)xmlGetProp(root, (const xmlChar *)"exp"));
		}
		if ((const char*)xmlGetProp(root, (const xmlChar *)"access")) {
			access = atoi((const char*)xmlGetProp(root, (const xmlChar *)"access"));
		}
		if ((const char*)xmlGetProp(root, (const xmlChar *)"level")) {
			level = atoi((const char*)xmlGetProp(root, (const xmlChar *)"level"));
        }
        if((const char*)xmlGetProp(root, (const xmlChar *)"maglevel")) {
            maglevel = atoi((const char*)xmlGetProp(root, (const xmlChar *)"maglevel"));
        }
        if((const char*)xmlGetProp(root, (const xmlChar *)"pushable")) {
			this->pushable = (bool)atoi((const char*)xmlGetProp(root, (const xmlChar *)"pushable"));
		}    
        if((const char*)xmlGetProp(root, (const xmlChar *)"speed")) {
            const char* speedy = (const char*)xmlGetProp(root, (const xmlChar *)"speed");
            if(strcmp(speedy, "very very low") == 0)
                    speed = 120;        
            else if(strcmp(speedy, "very low") == 0)
                    speed = 220;
            else if(strcmp(speedy, "low") == 0)
                    speed = 260;
            else if(strcmp(speedy, "medium") == 0)
                    speed = 320;
            else if(strcmp(speedy, "fast") == 0)
                    speed = 380;
            else if(strcmp(speedy, "very fast") == 0)
                    speed = 420;
            else if(strcmp(speedy, "very very fast") == 0)
                    speed = 520;
            else{
                 if(atoi(speedy) >= 20 && atoi(speedy) <= 1000)
                    speed = atoi(speedy);
                 else
                    setNormalSpeed();    
            }                                 
        }else{                     
			setNormalSpeed();
        }
        if(monster)	   
		   std::cout << "Monster: " << getName() << "." << std::endl;
        else
           std::cout << "Npc: " << getName() << "." << std::endl;
           
		while (p)
		{
			const char* str = (char*)p->name;
			if (strcmp(str, "mana") == 0){
				this->mana = atoi((const char*)xmlGetProp(p, (const xmlChar *)"now"));
				this->manamax = atoi((const char*)xmlGetProp(p, (const xmlChar *)"max"));
			}
			if (strcmp(str, "health") == 0){
				this->health = atoi((const char*)xmlGetProp(p, (const xmlChar *)"now"));
				this->healthmax = atoi((const char*)xmlGetProp(p, (const xmlChar *)"max"));
			}
			if (strcmp(str, "look") == 0){
				this->looktype = atoi((const char*)xmlGetProp(p, (const xmlChar *)"type"));
				this->lookmaster = this->looktype;
				this->lookhead = atoi((const char*)xmlGetProp(p, (const xmlChar *)"head"));
				this->lookbody = atoi((const char*)xmlGetProp(p, (const xmlChar *)"body"));
				this->looklegs = atoi((const char*)xmlGetProp(p, (const xmlChar *)"legs"));
				this->lookfeet = atoi((const char*)xmlGetProp(p, (const xmlChar *)"feet"));
				this->lookcorpse = atoi((const char*)xmlGetProp(p, (const xmlChar *)"corpse"));
			}
			if (strcmp(str, "combat") == 0){
                if((const char*)xmlGetProp(p, (xmlChar*)"defense"))
				    this->defense = atoi((const char*)xmlGetProp(p, (xmlChar*)"defense")); 
                    
				if((const char*)xmlGetProp(p, (xmlChar*)"armor"))
                    this->armor = atoi((const char*)xmlGetProp(p, (xmlChar*)"armor"));
                
                if((const char*)xmlGetProp(p, (xmlChar*)"change_probability"))
                    this->change = atoi((const char*)xmlGetProp(p, (xmlChar*)"change_probability")); 
                       
			    if((const char*)xmlGetProp(p, (xmlChar*)"change_per"))
                    this->changeper = atoi((const char*)xmlGetProp(p, (xmlChar*)"change_per"));  
                    
                if((const char*)xmlGetProp(p, (const xmlChar *)"combat_dist"))
			        this->combat_dist = atoi((const char*)xmlGetProp(p, (const xmlChar *)"combat_dist"));
			    
                if((const char*)xmlGetProp(p, (const xmlChar *)"runlife"))
			        this->runLife = atoi((const char*)xmlGetProp(p, (const xmlChar *)"runlife"));
			        
                if((const char*)xmlGetProp(p, (const xmlChar *)"damage_color")){
			        std::string color = (const char*)xmlGetProp(p, (const xmlChar *)"damage_color"); 
                    if(strcmp(color.c_str(), "red") == 0)
                          damageColor = RED;                      
                    else if(strcmp(color.c_str(), "green") == 0)
                          damageColor = GREEN;
                    else if(strcmp(color.c_str(), "gray") == 0)
                          damageColor = GRAY;
                    else if(strcmp(color.c_str(), "yellow") == 0)
                          damageColor = YELLOW;
                    else if(strcmp(color.c_str(), "dark_blue") == 0)
                          damageColor = DARK_BLUE;
                    else if(strcmp(color.c_str(), "light_blue") == 0)
                          damageColor = LIGHT_BLUE;                                                       
                }
            }
			if (strcmp(str, "attacks") == 0)
			{
				tmp=p->children;
				while(tmp)
				{
					if (strcmp((const char*)tmp->name, "attack") == 0)
					{
						int cycleTicks = -1;
						int probability = -1;

						if((const char*)xmlGetProp(tmp, (const xmlChar *)"cycleticks"))
							cycleTicks = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"cycleticks"));

						if((const char*)xmlGetProp(tmp, (const xmlChar *)"probability"))
							probability = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"probability"));

						TimeProbabilityClass timeprobsystem(cycleTicks, probability);

						std::string attacktype = (const char*)xmlGetProp(tmp, (const xmlChar *)"type");

						if(strcmp(attacktype.c_str(), "melee") == 0)
						{
							PhysicalAttackClass* physicalattack = new PhysicalAttackClass();
							
							physicalattack->fighttype = FIGHT_MELEE;
							if(xmlGetProp(tmp, (const xmlChar *)"mindamage")) 
								physicalattack->minWeapondamage = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"mindamage"));
                                
							if(xmlGetProp(tmp, (const xmlChar *)"maxdamage"))
								physicalattack->maxWeapondamage = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"maxdamage"));
                                
							physicalAttacks[physicalattack] = TimeProbabilityClass(cycleTicks, probability);
						}
						else if(strcmp(attacktype.c_str(), "distance") == 0)
						{
							PhysicalAttackClass* physicalattack = new PhysicalAttackClass();

							physicalattack->fighttype = FIGHT_DIST;
							std::string subattacktype = (const char*)xmlGetProp(tmp, (const xmlChar *)"name");

							if(strcmp(subattacktype.c_str(), "bolt") == 0)
								physicalattack->disttype = DIST_BOLT;
							else if(strcmp(subattacktype.c_str(), "arrow") == 0)
								physicalattack->disttype = DIST_ARROW;
							else if(strcmp(subattacktype.c_str(), "throwingstar") == 0)
								physicalattack->disttype = DIST_THROWINGSTAR;
							else if(strcmp(subattacktype.c_str(), "throwingknife") == 0)
								physicalattack->disttype = DIST_THROWINGKNIFE;
							else if(strcmp(subattacktype.c_str(), "smallstone") == 0)
								physicalattack->disttype = DIST_SMALLSTONE;
							else if(strcmp(subattacktype.c_str(), "largerock") == 0)
								physicalattack->disttype = DIST_LARGEROCK;
							else if(strcmp(subattacktype.c_str(), "snowball") == 0)
								physicalattack->disttype = DIST_SNOWBALL;
							else if(strcmp(subattacktype.c_str(), "powerbolt") == 0)
								physicalattack->disttype = DIST_POWERBOLT;

							if(xmlGetProp(tmp, (const xmlChar *)"mindamage"))
								physicalattack->minWeapondamage = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"mindamage"));
								
							if(xmlGetProp(tmp, (const xmlChar *)"maxdamage"))
								physicalattack->maxWeapondamage = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"maxdamage"));
					
							physicalAttacks[physicalattack] = TimeProbabilityClass(cycleTicks, probability);
						}
						else if(strcmp(attacktype.c_str(), "spell") == 0) {
							SpellAttackClass* spellattack = new SpellAttackClass();
							
							if((const char*)xmlGetProp(tmp, (const xmlChar *)"name"))
                                spellattack->spellname = (const char*)xmlGetProp(tmp, (const xmlChar *)"name");
							
                            if(xmlGetProp(tmp, (const xmlChar *)"mindamage")) 
								spellattack->minDamage = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"mindamage"));
                                
							if(xmlGetProp(tmp, (const xmlChar *)"maxdamage"))
								spellattack->maxDamage = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"maxdamage"));
								
                            if(spells.getAllSpells()->find(spellattack->spellname) != spells.getAllSpells()->end())
							{
								npcSpells[spellattack].push_back(timeprobsystem);
							}
						}
					}

					tmp = tmp->next;
				}
			}
			if (xmlStrcmp(root->name,(const xmlChar*) "loots")){ 
			    tmp=p->children;
                  while(tmp)
                  {
                   if (strcmp((const char*)tmp->name, "item") == 0)
                   {
                    int id = 0;
                    int chance = 0;
                    int x = 0;                 
                    int per = 100;
                    
                    if((const char*)xmlGetProp(tmp, (const xmlChar *) "id"))
                       id=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "id"));
                    if((const char*)xmlGetProp(tmp, (const xmlChar *) "x"))
                       x=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "x"));
                    if((const char*)xmlGetProp(tmp, (const xmlChar *) "chance"))
                       chance=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "chance"));
                    if((const char*)xmlGetProp(tmp, (const xmlChar *) "per"))
                       per=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "per"));
                    
                    if(per > 0){
                       int probability = chance;       
                       chance = std::max(chance, 0);
                       
	                   if(chance >= 0)
	                      probability = std::min(per, chance);
	
                       if(random_range(1, per) <= probability && id >= 100 && id<=2570)
                          npcLoots[id] = x;
                    }                       
                   }
                   if (strcmp((const char*)tmp->name, "container") == 0)
                   {
                    int containerId = 0;
                    int containerChance = 0;             
                    int containerPer = 100;
                    
                    if((const char*)xmlGetProp(tmp, (const xmlChar *) "id"))
                       containerId=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "id"));
                    if((const char*)xmlGetProp(tmp, (const xmlChar *) "chance"))
                       containerChance=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "chance"));
                    if((const char*)xmlGetProp(tmp, (const xmlChar *) "per"))
                       containerPer=atoi((const char*)xmlGetProp(tmp, (const xmlChar *) "per"));
                    
                    if(containerPer > 0){
                       int probability = containerChance;       
                       containerChance = std::max(containerChance, 0);
                       
	                   if(containerChance >= 0)
	                      probability = std::min(containerPer, containerChance);
	
                       if(random_range(1, containerPer) <= probability && containerId >= 100 && containerId<=2570)
                          containerLoot = containerId;
                    }
                    tmp2=tmp->children;
                    while(tmp2)
                    {
                          if (strcmp((const char*)tmp2->name, "item") == 0)
                          {
                              int id = 0;
                              int chance = 0;
                              int x = 0;                 
                              int per = 100;
                    
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *) "id"))
                                 id=atoi((const char*)xmlGetProp(tmp2, (const xmlChar *) "id"));
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *) "x"))
                                 x=atoi((const char*)xmlGetProp(tmp2, (const xmlChar *) "x"));
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *) "chance"))
                                 chance=atoi((const char*)xmlGetProp(tmp2, (const xmlChar *) "chance"));
                              if((const char*)xmlGetProp(tmp2, (const xmlChar *) "per"))
                                 per=atoi((const char*)xmlGetProp(tmp2, (const xmlChar *) "per"));
                    
                              if(per > 0){
                                 int probability = chance;       
                                 chance = std::max(chance, 0);
                       
	                             if(chance >= 0)
	                                probability = std::min(per, chance);
	
                                 if(random_range(1, per) <= probability && id >= 100 && id<=2570)
                                    containerLoots[id] = x;
                              }      
                          }                           
                          tmp2=tmp2->next;     
                    }                                  
                   }
                   tmp=tmp->next;
                 }
			}
			if(strcmp(str, "voices") == 0)
			{
				tmp=p->children;
				while(tmp)
				{
					if (strcmp((const char*)tmp->name, "voice") == 0) {
						int cycleTicks, probability;

						if((const char*)xmlGetProp(tmp, (const xmlChar *)"cycleticks"))
							cycleTicks = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"cycleticks"));
						else
							cycleTicks = 30000;

						if((const char*)xmlGetProp(tmp, (const xmlChar *)"probability"))
							probability = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"probability"));
						else
							probability = 30;
						
						std::string sentence = (const char*)xmlGetProp(tmp, (const xmlChar *)"sentence");

						if(sentence.length() > 0) {
                            yellingSentences[sentence] = TimeProbabilityClass(cycleTicks, probability);               
						}
					}

					tmp = tmp->next;
				}
			}
			if(strcmp(str, "defenses") == 0)
			{
				tmp=p->children;
				while(tmp)
				{
					if (strcmp((const char*)tmp->name, "defense") == 0) {
						std::string immunity = (const char*)xmlGetProp(tmp, (const xmlChar *)"immunity");

						if(strcmp(immunity.c_str(), "energy") == 0)
							defenses[0] = true;
						else if(strcmp(immunity.c_str(), "fire") == 0)
							defenses[1] = true;
						else if(strcmp(immunity.c_str(), "poison") == 0)
							defenses[2] = true;	
						else if(strcmp(immunity.c_str(), "physical") == 0)
							defenses[3] = true;
					}

					tmp = tmp->next;
				}
			}
			if(strcmp(str, "summons") == 0)
			{
				tmp=p->children;
				while(tmp)
				{
					if (strcmp((const char*)tmp->name, "summon") == 0) {
						int cycleTicks, probability, times;

						if((const char*)xmlGetProp(tmp, (const xmlChar *)"cycleticks"))
							cycleTicks = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"cycleticks"));
						else
							cycleTicks = 30000;

						if((const char*)xmlGetProp(tmp, (const xmlChar *)"probability"))
							probability = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"probability"));
						else
							probability = 30;
						
						if((const char*)xmlGetProp(tmp, (const xmlChar *)"times"))
							times = atoi((const char*)xmlGetProp(tmp, (const xmlChar *)"times"));
						else
							times = 5;
							
						std::string name = (const char*)xmlGetProp(tmp, (const xmlChar *)"name");

						if(name.length() > 0) {
                            summonSentences[name] = TimeProbabilityClass(cycleTicks, probability, times);               
						}
					}

					tmp = tmp->next;
				}
			}
				
			p = p->next;
		}

		xmlFreeDoc(doc);
	}
	//Now try to load the script
	if(this->scriptname != "")
	   this->script = new NpcScript(this->scriptname, this);
	else
       this->script = NULL;   
	//if(!this->script->isLoaded())
	//	this->loaded=false;
	this->map=map;
}


Npc::~Npc()
{
    for(std::map<PhysicalAttackClass*, TimeProbabilityClass>::iterator it = physicalAttacks.begin(); it != physicalAttacks.end(); ++it) {
		delete it->first;
	}
	for(std::map<SpellAttackClass*, TimeProbabilityClassVec>::iterator it = npcSpells.begin(); it != npcSpells.end(); ++it) {
		delete it->first;
	}
	
	physicalAttacks.clear();
    npcSpells.clear();
    
    if(script)       
	delete this->script;
}

void Npc::dropLoot(Item *container)
{
    if(!container->isContainer())
       return;
    for(lootItems::iterator loots = npcLoots.begin(); loots != npcLoots.end(); ++loots) {
		if(loots->second > 0)
		   container->addItem(new Item(loots->first, loots->second));
		else
           container->addItem(new Item(loots->first));  
	}
    if(containerLoot != 0)
    {
       Item *newcontainer = new Item(containerLoot);
       for(lootItems::iterator loots = containerLoots.begin(); loots != containerLoots.end(); ++loots) {
		if(loots->second > 0)
		   newcontainer->addItem(new Item(loots->first, loots->second));
		else
           newcontainer->addItem(new Item(loots->first));  
	   }
       container->addItem(newcontainer);              
    }                     
}
     
std::string Npc::getDescription(bool self){
    std::stringstream s;
	std::string str;	
	s << "You see " << name << ".";
	str = s.str();
	return str;
}
  
int Npc::getDistanceToTarget(Position toPos, Position fromPos)
{
	return std::max(std::abs(fromPos.x - toPos.x), std::abs(fromPos.y - toPos.y));
}
  
bool Npc::isInRange(const Position &p)
{
    if(map->getTile(p.x, p.y, p.z)->isPz())
       return false;
	return ((std::abs(p.x - this->pos.x) <= 10) && (std::abs(p.y - this->pos.y) <= 8) &&
		(p.z == this->pos.z));
}

bool Npc::changeCreature()
{
    int _probability, probability;
    _probability = change;
    _probability = std::max(_probability, 0);

	if(_probability >= 0)
	   probability = std::min(changeper, _probability);
	   
	bool ret = (random_range(1, changeper) <= probability);
	return ret;
}
            
void Npc::onThingMove(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos){
    if(!monster)
       return;
    const Creature* creature = dynamic_cast<const Creature *>(thing);
	if(creature) {
		if(isInRange(creature->pos) && creature->getID() != attackedCreature) {
			OnCreatureEnter(creature);
		}
		else if(!isInRange(creature->pos)) {
			OnCreatureLeave(creature);
		}     
	}                    
}

void Npc::onTeleport(const Creature *player, const Thing *thing, const Position *oldPos, unsigned char oldstackpos, bool magiceffect){
    if(!monster)
       return;
    const Creature* creature = dynamic_cast<const Creature *>(thing);
    if(creature){
	   if(isInRange(creature->pos) && creature->getID() != attackedCreature) {
		  OnCreatureEnter(creature);
	   }
	   else if(!isInRange(creature->pos)) {
		  OnCreatureLeave(creature);
	   }
   }	   
}

void Npc::OnCreatureEnter(const Creature *creature)
{
    Creature *creatureAttacking = map->getCreatureByID(attackedCreature);
	if(attackedCreature == 0 || !creatureAttacking || !isInRange(creatureAttacking->pos) || changeCreature()) {
		const Player *player = dynamic_cast<const Player*>(creature);
		if(player && player->access == 0 && isInRange(player->pos)) {
			attackedCreature = player->getID();
		}
	}
}

void Npc::OnCreatureLeave(const Creature *creature)
{
	if(attackedCreature == creature->getID()) {
		attackedCreature = 0;
		Creature *creature = map->findCreature(this);
		if(creature != NULL)
		   attackedCreature = creature->getID();
	}
}

void Npc::onAttack()
{
    map->addEvent(makeTask(500, std::bind2nd(std::mem_fun(&Map::checkPlayerAttacking), getID()))); 

    if (attackedCreature != 0) {
		Player *attackedPlayer = dynamic_cast<Player*>(map->getCreatureByID(this->attackedCreature));
		if (attackedPlayer) {
			Tile* fromtile = map->getTile(this->pos.x, this->pos.y, this->pos.z);
			if (!attackedPlayer->isAttackable() == 0 && fromtile->isPz() && this->access == 0) {
				return;
			}
			else {
				if (attackedPlayer != NULL && attackedPlayer->health > 0) {
					doAttacks(attackedPlayer);
				}
			}
		}
	}
}

bool Npc::doAttacks(Player* attackedPlayer)
{
	bool ret = false;
	
	for(PhysicalAttacks::iterator paIt = physicalAttacks.begin(); paIt != physicalAttacks.end(); ++paIt) {
		TimeProbabilityClass& timeprobsystem = paIt->second;
		if(timeprobsystem.onTick(500)) {
				curPhysicalAttack = paIt->first;
				map->creatureMakeDamage(this, attackedPlayer, getFightType());
		}
	}

		for(AttackSpells::iterator iaIt = npcSpells.begin(); iaIt != npcSpells.end(); ++iaIt) {
			for(TimeProbabilityClassVec::iterator asIt = iaIt->second.begin(); asIt != iaIt->second.end(); ++asIt) {
				TimeProbabilityClass& timeprobsystem = *asIt;
				if(timeprobsystem.onTick(500)) {
					std::map<std::string, Spell*>::iterator rit = spells.getAllSpells()->find(iaIt->first->spellname);
					if( rit != spells.getAllSpells()->end() ) {
                        curSpellAttack = iaIt->first;
						rit->second->getSpellScript()->castSpell(this, "");
					}
				}
			}
		}

	return ret;
}

void Npc::onCreatureAppear(const Creature *creature){
	if(monster){
	   if(isInRange(creature->pos)) {
		  OnCreatureEnter(creature);
	   }
    }
    if(script)
       this->script->onCreatureAppear(creature->getID());
}

void Npc::onCreatureDisappear(const Creature *creature, unsigned char stackPos){
	if(monster)
	   OnCreatureLeave(creature);
    
    if(script)
       this->script->onCreatureDisappear(creature->getID());
}

void Npc::onCreatureSay(const Creature *creature, unsigned char type, const std::string &text){
	if(creature->getID() == this->getID())
		return;
	const Player* p = dynamic_cast<const Player*>(creature);	
	if(script && p)	
	   this->script->onCreatureSay(creature->getID(), type, text);
	if(monster){
	   if(text == "exeta res")
	      attackedCreature = creature->getID();
    }	   
}

void Npc::onThink(){
    if(!map->serverrun)
       return;
    
    if(script)
	   this->script->onThink();
    
    for(YellingSentences::iterator ysIt = yellingSentences.begin(); ysIt != yellingSentences.end(); ++ysIt) {
        if((ysIt->second).onTick(300)) {
            map->creatureMonsterYell(this, ysIt->first);
		}
	}
	
    if(!monster)
       return;
    if(firstthink){
       Creature *creature = map->findCreature(this);
	   if(creature != NULL)
		  attackedCreature = creature->getID();
	   firstthink = false;	  
    }
             
	if(attackedCreature != 0){
	   Creature* moveToCreature = map->getCreatureByID(attackedCreature);
	   if(moveToCreature && isInRange(moveToCreature->pos)){
          for(YellingSentences::iterator ysIt = summonSentences.begin(); ysIt != summonSentences.end(); ++ysIt) {
              if((ysIt->second).onTick(300) && (ysIt->second).times > 0) {
                 (ysIt->second).times -= 1;  
                 std::stringstream name;
                 name << "/s " << (ysIt->first).c_str();                      
                 doSay(name.str().c_str());
		      }
	      }                       
	      if(getDistanceToTarget(moveToCreature->pos, pos) > 1 && combat_dist <= 1 && health > runLife){
	         doMoveTo(moveToCreature->pos);   
	      }else if(getDistanceToTarget(moveToCreature->pos, pos) <= 1 && combat_dist <= 1 && health > runLife){
             int i = random_range(1,40);
             if(i <= 6)
	            doTurnAi();
	         else if(i == 10)
                doMoveAi();
          }else if(combat_dist > 1 && getDistanceToTarget(moveToCreature->pos, pos) < combat_dist && health > runLife){
             doMoveTo(calculeMove(moveToCreature->pos, true));
             doTurnAi();   
          }else if(combat_dist > 1 && getDistanceToTarget(moveToCreature->pos, pos) > combat_dist && health > runLife){
             doMoveTo(moveToCreature->pos);    
          }else if(health <= runLife){
             doMoveTo(calculeMove(moveToCreature->pos, true));
             doTurnAi();      
          }      
	   }else
          setAttackedCreature(0);
    }
}

void Npc::doSay(std::string msg){
	if(!map->creatureSaySpell(this, msg))
		this->map->creatureSay(this, 1, msg);
}

void Npc::doAttack(int id){
	attackedCreature = id;
}

Position Npc::calculeMove(Position toPos, bool run)
{
    int prevdist = getDistanceToTarget(toPos, pos);  
    for(int dy = -1; dy <= 1; dy++) {
			for(int dx = -1; dx <= 1; dx++) {
                int x = pos.x+dx;
                int y = pos.y+dy;    
				if((pos.x == x && pos.y == y) || !(dx == 0 || dy == 0))
					continue;	
                int dist = getDistanceToTarget(toPos, Position(x,y,pos.z));
                
                bool condition = false;
                if(run){
                   if(dist > prevdist)
                      condition = true;     
                }else{
                   if(dist < prevdist)
                      condition = true;   
                }
                   
                if(condition || (prevdist == 0)) {
					Tile *t;
					
                    if((!(t = map->getTile(x, y, pos.z))) || t->isBlocking() || t->isPz() || t->creatures.size()|| t->isBlockingWalk())
					   continue;     	  
						
					Position Pos;
                    Pos.x = x;
                    Pos.y = y;
                    Pos.z = 7;
					return Pos;
				}
			}
	}
	
	int prevDistX = toPos.x - pos.x;
	int prevDistY = toPos.y - pos.y;
                   
	for(int dy = -1; dy <= 1; dy++) {
			for(int dx = -1; dx <= 1; dx++) {
                int x = pos.x+dx;
                int y = pos.y+dy;    
				if((pos.x == x && pos.y == y) || !(dx == 0 || dy == 0))
					continue;	
                int distX = toPos.x - x;
                int distY = toPos.y - y;                
                      
                bool condition = false;
                if(run){
                   if((distX < prevDistX && distX <= 0 && prevDistX < 0) || (distX > prevDistX && distX >= 0 && prevDistX > 0))
                      condition = true;
                   else if((distY < prevDistY && distY <= 0 && prevDistY < 0) || (distY > prevDistY && distY >= 0 && prevDistY > 0))
                      condition = true;        
                }else{
                   if((distX > prevDistX && distX <= 0 && prevDistX < 0) || (distX < prevDistX && distX >= 0 && prevDistX > 0))
                      condition = true;
                   else if((distY > prevDistY && distY <= 0 && prevDistY < 0) || (distY < prevDistY && distY >= 0 && prevDistY > 0))
                      condition = true;   
                }
                   
                if(condition) {
					Tile *t;
					
                    if((!(t = map->getTile(x, y, pos.z))) || t->isBlocking() || t->isPz() || t->creatures.size()|| t->isBlockingWalk())
					   continue;     	  
						
					Position Pos;
                    Pos.x = x;
                    Pos.y = y;
                    Pos.z = 7;
					return Pos;
				}
			}
	}
	for(int y = pos.y - 1; y <= pos.y + 1; ++y) {
			for(int x = pos.x - 1; x <= pos.x + 1; ++x) {
				if((pos.x == x && pos.y == y))
					continue;
					
                int dist = getDistanceToTarget(toPos, Position(x,y,pos.z));
                
                bool condition = false;
                if(run){
                   if(dist > prevdist)
                      condition = true;     
                }else{
                   if(dist < prevdist)
                      condition = true;   
                }
                
                if(condition) {
					Tile *t;
					
                    if((!(t = map->getTile(x, y, pos.z))) || t->isBlocking() || t->isPz() || t->creatures.size() || t->isBlockingWalk())
					   continue;   	  
						
					Position toPos;
                    toPos.x = x;
                    toPos.y = y;
                    toPos.z = 7;
					return toPos;
				}
			}
	}                 
}
         
void Npc::doMove(int direction){
	switch(direction){
		case 0:
			this->map->thingMove(this, this,this->pos.x, this->pos.y+1, this->pos.z);
		break;
		case 1:
			this->map->thingMove(this, this,this->pos.x+1, this->pos.y, this->pos.z);
		break;
		case 2:
			this->map->thingMove(this, this,this->pos.x, this->pos.y-1, this->pos.z);
		break;
		case 3:
			this->map->thingMove(this, this,this->pos.x-1, this->pos.y, this->pos.z);
		break;
	}
}

void Npc::doMoveAi(){
    if(attackedCreature == 0)
	   return;
	Creature *toCreature = map->getCreatureByID(attackedCreature);
    if(!toCreature){
       OnCreatureLeave(toCreature);           
       return;
    }
    int random = random_range(1,2);   
    int x = this->pos.x - toCreature->pos.x;
    int y = this->pos.y - toCreature->pos.y;
    if(x == 1 && y == 1){
         if(random == 2)
            doMove(2);
         else
            doMove(3);
    }else if(x == -1 && y == -1){
         if(random == 2)
            doMove(0);
         else
            doMove(1); 
    }else if(x == 1 && y == -1){
         if(random == 2)
            doMove(3);
         else
            doMove(0); 
    }else if(x == -1 && y == 1){
         if(random == 2)
            doMove(1);
         else
            doMove(2); 
    }else if(x == -1 && y == 0){
         if(random == 2)
            doMove(2);
         else
            doMove(0); 
    }else if(x == 1 && y == 0){
         if(random == 2)
            doMove(0);
         else
            doMove(2); 
    }else if(x == 0 && y == -1){
         if(random == 2)
            doMove(3);
         else
            doMove(1); 
    }else if(x == 0 && y == 1){
         if(random == 2)
            doMove(1);
         else
            doMove(3); 
    }      
}  

void Npc::doAnyMove(){
    int random = random_range(1,4);   
    if(random == 1){
       doMove(0);
    }else if(random == 2){
       doMove(1); 
    }else if(random == 3){
       doMove(2); 
    }else if(random == 4){
       doMove(3);     
    }           
}
   
void Npc::doTurn(Creature *creature, int dire){
	switch(dire){
		case 0:
			this->map->creatureTurn(creature, NORTH);
		break;
		case 1:
			this->map->creatureTurn(creature, SOUTH);
		break;
		case 2:
			this->map->creatureTurn(creature, EAST);
		break;
		case 3:
			this->map->creatureTurn(creature, WEST);
		break;
	}
}

void Npc::doTurnAi(){
	if(attackedCreature == 0)
	   return;
	Creature *toCreature = map->getCreatureByID(attackedCreature);
    if(!toCreature){
       OnCreatureLeave(toCreature);           
       return;
    }
    int random = random_range(1,2);   
    Position toPos = toCreature->pos;
    Position fromPos = this->pos;
    if(fromPos.y > toPos.y){
       if(fromPos.x > toPos.x){
          if(random == 1)
             doTurn(this, 3);
          else
             doTurn(this, 0);
       } 
       else if(fromPos.x < toPos.x){
          if(random == 1)
             doTurn(this, 2);
          else
             doTurn(this, 0);
       }else{
          doTurn(this, 0); 
       }                    
    }else if(fromPos.y < toPos.y){
       if(fromPos.x > toPos.x){
          if(random == 1)
             doTurn(this, 3);
          else
             doTurn(this, 1);
       } 
       else if(fromPos.x < toPos.x){
          if(random == 1)
             doTurn(this, 2);
          else
             doTurn(this, 1);
       }else{
          doTurn(this, 1); 
       }                    
    }else{
       if(fromPos.x > toPos.x)
          doTurn(this, 3);
       else
          doTurn(this, 2);
    }                         
}

void Npc::doMoveTo(Position target){
	if(route.size() == 0 || route.back() != target || route.front() != this->pos){
		route = this->map->getPathTo(this->pos, target);
	}
	if(route.size()==0){
        doAnyMove();                
		return;
	}
	else route.pop_front();
	Position nextStep=route.front();
	route.pop_front();
	int dx = nextStep.x - this->pos.x;
	int dy = nextStep.y - this->pos.y;
	if((dx == 1 || dx == -1 || dx == 0) && (dy == 1 || dy == -1 || dy == 0) && target != this->pos)
	   this->map->thingMove(this, this,this->pos.x + dx, this->pos.y + dy, this->pos.z);
}

NpcScript::NpcScript(std::string scriptname, Npc* npc){
	this->loaded = false;
	if(scriptname == "")
		return;
	luaState = lua_open();
	luaopen_loadlib(luaState);
	luaopen_base(luaState);
	luaopen_math(luaState);
	luaopen_string(luaState);
	luaopen_io(luaState);
    lua_dofile(luaState, "data/npc/scripts/lib/npc.lua");
	
	FILE* in=fopen(scriptname.c_str(), "r");
	if(!in)
		return;
	else
		fclose(in);
	lua_dofile(luaState, scriptname.c_str());
	this->loaded=true;
	this->npc=npc;
	this->setGlobalNumber("addressOfNpc", (int) npc);
	this->registerFunctions();
}

void NpcScript::onThink(){
	lua_pushstring(luaState, "onThink");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	lua_call(luaState, 0,0);
}


void NpcScript::onCreatureAppear(int cid){
	if(npc->getID() != cid){
		lua_pushstring(luaState, "onCreatureAppear");
		lua_gettable(luaState, LUA_GLOBALSINDEX);
		lua_pushnumber(luaState, cid);
		lua_call(luaState, 1,0);
	}
}

void NpcScript::onCreatureDisappear(int cid){
	lua_pushstring(luaState, "onCreatureDisappear");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	lua_pushnumber(luaState, cid);
	lua_call(luaState, 1,0);
}

void NpcScript::onCreatureSay(int cid, unsigned char type, const std::string &text){
	lua_pushstring(luaState, "onCreatureSay");
	lua_gettable(luaState, LUA_GLOBALSINDEX);
	int count = atoi(text.c_str());
    lua_pushnumber(luaState, count);
	lua_pushnumber(luaState, cid);
	lua_pushnumber(luaState, type);
	lua_pushstring(luaState, text.c_str());
	lua_call(luaState, 4,0);
}

int NpcScript::registerFunctions(){
	lua_register(luaState, "selfSay", NpcScript::luaActionSay);
	lua_register(luaState, "selfDoAnyMove", NpcScript::luaActionDoAnyMove);
	lua_register(luaState, "selfMove", NpcScript::luaActionMove);
	lua_register(luaState, "selfTurn", NpcScript::luaActionTurn);
	lua_register(luaState, "selfMoveTo", NpcScript::luaActionMoveTo);
	lua_register(luaState, "selfGetPosition", NpcScript::luaSelfGetPos);
	lua_register(luaState, "selfAttackCreature", NpcScript::luaActionAttackCreature);
	lua_register(luaState, "selfGotoMasterPos", NpcScript::luaActionGotoMasterPos);
	lua_register(luaState, "creatureGetName", NpcScript::luaCreatureGetName);
	lua_register(luaState, "creatureGetName2", NpcScript::luaCreatureGetName2);
	lua_register(luaState, "findPlayer", NpcScript::luaFindPlayer);
	lua_register(luaState, "creatureGetPosition", NpcScript::luaCreatureGetPos);
	lua_register(luaState, "selfGetPosition", NpcScript::luaSelfGetPos);
	lua_register(luaState, "creatureTeleport", NpcScript::luaCreatureTeleport);
	lua_register(luaState, "creatureAddItem", NpcScript::luaCreatureAddItem);
	lua_register(luaState, "creatureGetItem", NpcScript::luaCreatureGetItem);
	lua_register(luaState, "creatureSeeCash", NpcScript::luaCreatureSeeCash);
	lua_register(luaState, "creatureAddCash", NpcScript::luaCreatureAddCash);
	lua_register(luaState, "creatureGetCash", NpcScript::luaCreatureGetCash);
	lua_register(luaState, "creatureSeeHealth", NpcScript::luaCreatureSeeHealth);
	lua_register(luaState, "creatureAddHealth", NpcScript::luaCreatureAddHealth);
	lua_register(luaState, "creatureGetHealth", NpcScript::luaCreatureGetHealth);
	return true;
}

Npc* NpcScript::getNpc(lua_State *L){
	lua_getglobal(L, "addressOfNpc");
	int val = (int)lua_tonumber(L, -1);
	lua_pop(L,1);

	Npc* mynpc = (Npc*) val;
	if(!mynpc){
		return 0;
	}
	return mynpc;
}

int NpcScript::luaCreatureGetName2(lua_State *L){
	const char* s = lua_tostring(L, -1);
	lua_pop(L,1);
	Npc* mynpc = getNpc(L);
	Creature *c = mynpc->map->getCreatureByName(s);
	
	if(c) {
		lua_pushnumber(L, c->getID());
	}
	else
		lua_pushnumber(L, 0);

	return 1;
}

int NpcScript::luaCreatureGetName(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc = getNpc(L);
	Creature* creature = mynpc->map->getCreatureByID(id);
	if(creature)
	   lua_pushstring(L, creature->getName().c_str());
	else
       lua_pushstring(L, "NULL");   
	return 1;
}

int NpcScript::luaCreatureSeeCash(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc = getNpc(L);
	Creature* c = mynpc->map->getCreatureByID(id);
	Player *player = dynamic_cast <Player*>(c);
	if(player)
	    lua_pushnumber(L, player->cash);
	else
        lua_pushnumber(L, 0);    
	return 1;
}

int NpcScript::luaCreatureGetCash(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int cash = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Npc* mynpc = getNpc(L);
	Creature* c = mynpc->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
	if(player)
	    player->cash = player->cash - cash;   
	return 0;
}

int NpcScript::luaCreatureAddCash(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int cash = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Npc* mynpc = getNpc(L);
	Creature* c = mynpc->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
	if(player)
	    player->cash = player->cash + cash;   
	return 0;
}

int NpcScript::luaCreatureSeeHealth(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc = getNpc(L);
	Creature* c = mynpc->map->getCreatureByID(id);
	Player *player = dynamic_cast <Player*>(c);
	if(player)
	    lua_pushnumber(L, player->health);
	else
        lua_pushnumber(L, 0);    
	return 1;
}

int NpcScript::luaCreatureGetHealth(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int get = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Npc* mynpc = getNpc(L);
	Creature* c = mynpc->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
	if(player && player->access != 3){
        player->health -= std::min(get, player->health-1);       
        std::vector<Creature*> list;
	    mynpc->map->getSpectators(Range(player->pos, true), list);

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

int NpcScript::luaCreatureAddHealth(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int add = (int)lua_tonumber(L, -1);
	lua_pop(L,2);
	Npc* mynpc = getNpc(L);
	Creature* c = mynpc->map->getCreatureByID(id);
	Player* player = dynamic_cast <Player*>(c);
    if(player && player->access != 3){
	    player->health += std::min(add, player->healthmax - player->health);
	    std::vector<Creature*> list;
	    mynpc->map->getSpectators(Range(player->pos, true), list);

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

int NpcScript::luaCreatureGetPos(lua_State *L){
	int id = (int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc = getNpc(L);
	Creature* c = mynpc->map->getCreatureByID(id);
	
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

int NpcScript::luaFindPlayer(lua_State *L){
	lua_pop(L,1);
	Npc* mynpc = getNpc(L);
	if(mynpc->map->serverrun){
        Creature* c = mynpc->map->findCreature(mynpc);
        if(c && (c->access == 0 || c->access == 1))
		   lua_pushnumber(L, c->getID());
		else
		   lua_pushnumber(L, 0);   
	}
	else
		lua_pushnumber(L, 0);
		
	return 1;
}

int NpcScript::luaSelfGetPos(lua_State *L){
	lua_pop(L,1);
	Npc* mynpc = getNpc(L);
	lua_pushnumber(L, mynpc->pos.x);
	lua_pushnumber(L, mynpc->pos.y);
	lua_pushnumber(L, mynpc->pos.z);
	return 3;
}

int NpcScript::luaActionSay(lua_State* L){
	int len = lua_strlen(L, -1);
	std::string msg(lua_tostring(L, -1), len);
	lua_pop(L,1);

	Npc* mynpc=getNpc(L);
	if(mynpc)
		mynpc->doSay(msg);
	return 0;
}

int NpcScript::luaActionMove(lua_State* L){
	int dir=(int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc=getNpc(L);
	if(mynpc)
		mynpc->doMove(dir);
	return 0;
}

int NpcScript::luaActionDoAnyMove(lua_State* L){
	Npc* mynpc=getNpc(L);
	if(mynpc)
		mynpc->doAnyMove();
	return 0;
}

int NpcScript::luaActionTurn(lua_State* L){
	int dire=(int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc=getNpc(L);
	if(mynpc)
		mynpc->doTurn(mynpc, dire);
	return 0;
}

int NpcScript::luaActionMoveTo(lua_State* L){
	Position target;
	target.z=(int)lua_tonumber(L, -1);
	lua_pop(L,1);
	target.y=(int)lua_tonumber(L, -1);
	lua_pop(L,1);
	target.x=(int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc=getNpc(L);
	if(mynpc)
		mynpc->doMoveTo(target);
	return 0;
}

int NpcScript::luaActionGotoMasterPos(lua_State* L){
	lua_pop(L,1);
	Npc* mynpc=getNpc(L);
	if(mynpc){
        if(mynpc->map->getTile(mynpc->masterPos.x, mynpc->masterPos.y, mynpc->masterPos.z))
		   mynpc->doMoveTo(mynpc->masterPos);
    }
    if(mynpc->pos == mynpc->masterPos)
       lua_pushnumber(L, 1);	
	else 
	   lua_pushnumber(L, 0);
	return 1;   
}

int NpcScript::luaActionAttackCreature(lua_State *L){
	int id=(int)lua_tonumber(L, -1);
	lua_pop(L,1);
	Npc* mynpc=getNpc(L);
	if(mynpc)
		mynpc->doAttack(id);
	return 0;
}

int NpcScript::luaCreatureTeleport(lua_State *L){
	int id = (int)lua_tonumber(L, -3);
	int to_x = (int)lua_tonumber(L, -2);
	int to_y = (int)lua_tonumber(L, -1);
	lua_pop(L,4);
	Npc* mynpc = getNpc(L);
	
	Position pos;
	pos.x = to_x;
	pos.y = to_y;
	pos.z = 7;
	
	Creature* target = mynpc->map->getCreatureByID(id);
	if(target)
	  mynpc->map->teleportPlayer(pos, target, true, NULL, false);

	return 0;
}

int NpcScript::luaCreatureAddItem(lua_State *L){
	int id = (int)lua_tonumber(L, -4);
	int count = (int)lua_tonumber(L, -3);
	int creature = (int)lua_tonumber(L, -2);
	int rune = (int)lua_tonumber(L, -1);
	lua_pop(L,5);
	Npc* mynpc = getNpc(L);	
	Creature* c = mynpc->map->getCreatureByID(creature);
	Player *target = dynamic_cast<Player*>(c);
	if(target){
        int i;       
	    for(i=1 ;i<11;i++){
           if(target->items[i] == NULL && i == 10){
              Item* item = NULL;
              if(rune == 1)
                  item = new Item(id, count);
              else
                  item = new Item(id);
              if(item != NULL){    
                 target->items[i] = item;
                 NetworkMessage msg;
                 msg.AddPlayerInventoryItem(target, i);
                 target->sendNetworkMessage(&msg);
                 lua_pushnumber(L, 1);
              }else
                 lua_pushnumber(L, 0);   
              return 1;
           }                                 
           else if(target->items[i] && target->items[i]->isContainer() && !(target->items[i]->getContainerMaxItemCount() <= target->items[i]->getContainerItemCount())){
              Item* item = NULL;
              if(rune == 1)
                  item = new Item(id, count);
              else
                  item = new Item(id);
              if(item != NULL){    
                 target->items[i]->addItem(item);
                 mynpc->map->needContainerUpdate(target->items[i], c, item, 0xFF, 0, false, false);
                 lua_pushnumber(L, 1);
              }else
                 lua_pushnumber(L, 0);   
              return 1;
           }     
        }
        Tile *t = mynpc->map->getTile(target->pos.x, target->pos.y, target->pos.z);
        Item* item = NULL;
        if(rune == 1)
           item = new Item(id, count);
        else
           item = new Item(id);
        t->addThing(item);
        mynpc->map->creatureBroadcastTileUpdated(target->pos);
        lua_pushnumber(L, 1);  
	    return 1;
        
    }
    lua_pushnumber(L, 0);  
	return 1;
}

int NpcScript::luaCreatureGetItem(lua_State *L){
	int id = (int)lua_tonumber(L, -2);
	int creature = (int)lua_tonumber(L, -1);
	lua_pop(L,3);
	Npc* mynpc = getNpc(L);	
	Creature* c = mynpc->map->getCreatureByID(creature);
	Player *target = dynamic_cast<Player*>(c);
	if(target){
        int i;      
	    for(i=1 ;i<11;i++){
           if(target->items[i] && target->items[i]->getID() == id){
              target->items[i] = NULL;
              NetworkMessage msg;
              msg.AddPlayerInventoryItem(target, i);
              target->sendNetworkMessage(&msg);
              lua_pushnumber(L, 1);
              return 1;
           }else if(target->items[i] && target->items[i]->isContainer()){
              for(int g=0;g<=target->items[i]->getContainerItemCount();g++){
                      if(target->items[i]->getItem(g) && target->items[i]->getItem(g)->getID() == id){
                        target->items[i]->removeItem(target->items[i]->getItem(g));
                        mynpc->map->needContainerUpdate(target->items[i], c, target->items[i]->getItem(g), g, 0xFF, true, false);                      
                        lua_pushnumber(L, 1);
                        //delete target->items[i]->getItem(g);
                        return 1;
                      }
              }
           } 
        }
    }
    lua_pushnumber(L, 0);  
	return 1;
}


