#include "definitions.h"
#include "magic.h"

BaseMagicEffect::BaseMagicEffect()
{
	centerpos.x = 0;
	centerpos.y = 0;
	centerpos.z = 0;
	damageEffect = 0;
	animationcolor = 0;
	animationEffect = 0;
}

MagicEffectClass::MagicEffectClass()
{
	minDamage = 0;
	maxDamage = 0;
	type = 4;
	offensive = false;
	physical = false;
}

MagicEffectRuneClass::MagicEffectRuneClass()
{

}

MagicEffectAreaClass::MagicEffectAreaClass()
{
	direction = 0;
	areaEffect = 0xFF;
	memset(area, 0, sizeof(area));
}

MagicEffectInstantSpellClass::MagicEffectInstantSpellClass()
{
	manaCost = 0;
}

MagicEffectGroundClass::MagicEffectGroundClass()
{
	groundID = 0;
}
