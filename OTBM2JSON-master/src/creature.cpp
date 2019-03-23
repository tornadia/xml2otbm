////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>

#include "definitions.h"
#include "creature.h"
#include "tile.h"

using namespace std;

static unsigned int idcount = 0x4711;


Creature::Creature(const char *name) : access(0)
{
  idcount++;
  id         = idcount;
  direction  = NORTH;
  this->name = name;
  damageColor = RED;
  lookhead   = 0;
  lookbody   = 0;
  looklegs   = 0;
  lookfeet   = 0;
  lookmaster = 0;
  looktype   = PLAYER_MALE_1;
  lookcorpse = 2276;
  pzLocked = false;
  inFightTicks    = 0;
  manaShieldTicks = 0;
  hasteTicks      = 0;
  lightTicks      = 0;
  exhaustedTicks  = 0;
  lightLevel      = 0;
  shouldrespawn    = false;
  respawntime      = 300;
  dirr             = 0;
  health           = 100;
  healthmax        = 100;
  experience       = 0;
  lastmove         = 0;
  movedelay        = 0;
  attackedCreature = 0;
  speed            = 220;
  givenxp          = 0;
  for(int i=0;i<4;i++)
        defenses[i] = false;
}

void Creature::addInflictedDamage(Creature* attacker, int damage)
{
	if(damage <= 0)
		return;

	unsigned long id = 0;
	if(attacker) {
		id = attacker->getID();
	}

	totaldamagelist[id].push_back(make_pair(SURVIVALSYS_TIME(), damage));
}

int Creature::getGainedExperience(Creature* attacker)
{
	int totaldamage = getTotalInflictedDamage();
	int attackerdamage = getInflicatedDamage(attacker);
	int gainexperience = 0;
	
	if(attackerdamage > 0 && totaldamage > 0) {
		gainexperience = (int)std::floor(((double)attackerdamage / totaldamage) * givenxp);
	}

	return gainexperience;
}

int Creature::getInflicatedDamage(Creature* attacker)
{
	unsigned long id = 0;
	if(attacker) {
		id = attacker->getID();
	}

	return getInflicatedDamage(id);
}

int Creature::getInflicatedDamage(unsigned long id)
{
	int ret = 0;
	std::map<long, DamageList >::const_iterator tdIt = totaldamagelist.find(id);
	if(tdIt != totaldamagelist.end()) {
		for(DamageList::const_iterator dlIt = tdIt->second.begin(); dlIt != tdIt->second.end(); ++dlIt) {
			ret += dlIt->second;
		}
	}

	return ret;
}

int Creature::getTotalInflictedDamage()
{
	int ret = 0;
	std::map<long, DamageList >::const_iterator tdIt;
	for(tdIt = totaldamagelist.begin(); tdIt != totaldamagelist.end(); ++tdIt) {
		ret += getInflicatedDamage(tdIt->first);
	}

	return ret;
}

std::vector<long> Creature::getInflicatedDamageCreatureList()
{
	std::vector<long> list;

	std::map<long, DamageList >::const_iterator tdIt;
	for(tdIt = totaldamagelist.begin(); tdIt != totaldamagelist.end(); ++tdIt) {
		list.push_back(tdIt->first);
	}

	return list;
}

void Creature::drainHealth(int damage)
{
  health -= min(health, damage);
}

void Creature::drainMana(int damage)
{
  mana -= min(mana, damage);
}

void Creature::setAttackedCreature(unsigned long id)
{
  attackedCreature = id;
}

bool Creature::canMovedTo(Tile *tile)
{
  if (tile->creatures.size())
    return false;

  return Thing::canMovedTo(tile);
}

std::string Creature::getDescription(bool self){
    std::stringstream s;
	std::string str;	
	s << "You see a " << name << ".";
	str = s.str();
	return str;
}

int Creature::getWalkDuration()
{
     int delay[17] = {540, 500, 460, 420, 400, 370, 350, 330, 310, 300, 280, 270, 260, 250, 240, 230};
     int speedy[17] = {220, 240, 260, 280, 300, 320, 340, 360, 380, 400, 420, 440, 460, 480, 500, 520};
     for(int i=0;i<17;i++)
     {
         if(getSpeed() >= (speedy[i]-10) && ((i != 16 && getSpeed() < (speedy[i+1]-10)) || getSpeed() < speedy[i]))
               return delay[i];
     }
     if(getSpeed() < 220)
        return 700;
     int StepTime = (230-((int)((getSpeed()-520)/20)*10));
     if(StepTime < 100) StepTime = 100;
     return StepTime;
}    
