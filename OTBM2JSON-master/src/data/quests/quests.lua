-- Additional info of all quests
function onCreaturePullLever(cid, leverPosX, leverPosY, leverPosZ, leverSide)
	if leverPosX == 117 and leverPosY == 55 and leverPosZ == 7 and leverGetSide(leverPosX, leverPosY, leverPosZ) == LEFT then
		creatureAddText(cid, MSG_ADVANCE, "Annihilator Running.")
	end
	leverMove(leverPosX, leverPosY, leverPosZ)
end


function onCreatureSinkTile(cid, tilePosX, tilePosY, tilePosZ)

end


function onCreatureTeleport(cid, teleportPosX, teleportPosY, teleportPosZ)

end


function onCreatureOpenContainerQuest(cid, questName, containerPosX, containerPosY, containerPosZ)

end


-- Every second the server make this function
function onThink()

end
