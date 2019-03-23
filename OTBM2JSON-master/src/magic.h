////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __MAGIC_H__
#define __MAGIC_H__

#include "position.h"

class BaseMagicEffect
{
public:
	BaseMagicEffect();
    virtual ~BaseMagicEffect() {};
    
	Position centerpos;
	unsigned char damageEffect;
	unsigned char animationcolor;
	unsigned char animationEffect;
};

class MagicEffectClass : public BaseMagicEffect
{
public:
	MagicEffectClass();
	int minDamage;
	int maxDamage;
	int type;
	bool offensive;
	bool physical;
};

class MagicEffectRuneClass : public MagicEffectClass
{
public:
	MagicEffectRuneClass();
};

class MagicEffectAreaClass : public MagicEffectClass
{
public:
	MagicEffectAreaClass();
	unsigned char direction;
	unsigned char areaEffect;
	unsigned char area[14][18];
};

class MagicEffectInstantSpellClass : public MagicEffectAreaClass
{
public:
	MagicEffectInstantSpellClass();
	int manaCost;
};

class MagicEffectGroundClass : public MagicEffectAreaClass
{
public:
	MagicEffectGroundClass();
	unsigned short groundID;
};

#endif
