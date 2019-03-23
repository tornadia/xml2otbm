
area = {
				{0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 2, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 3, 0, 0},
				{0, 0, 2, 2, 2, 2, 0, 0, 1, 0, 0, 3, 3, 3, 3, 0, 0},
				{0, 0, 2, 2, 2, 2, 2, 2, 0, 3, 3, 3, 3, 3, 3, 0, 0},
				{0, 0, 2, 2, 2, 2, 0, 0, 4, 0, 0, 3, 3, 3, 3, 0, 0},
				{0, 0, 2, 2, 0, 0, 0, 0, 4, 0, 0, 0, 0, 3, 3, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0} 
}

damageEffect = NM_ME_FIRE_AREA
areaEffect = NM_ME_FIRE_AREA
animationEffect = NM_ME_FIRE_AREA
animationColor = ORANGE
offensive = true
needDirection = true
npcRune = false
physical = false
type = TYPE_FIRE

function onCast(cid, creaturePos, level, maglv, var)
centerpos = {x=creaturePos.x, y=creaturePos.y, z=creaturePos.z}
base = (level*2)+(maglv*3)
 minDmg = base*1
 maxDmg = base*1.52
doMagic(cid, area, centerpos, damageEffect, areaEffect, animationEffect, animationColor, offensive, physical, minDmg, maxDmg, needDirection, npcRune, type)
end



