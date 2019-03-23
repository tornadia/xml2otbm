time = 0
hiid = nil
ho = false
infiresla = false
idundert = false
isurvival = false
cash = 0
moves = 0
maxmoves = 0

function onThingMove(creature, thing, oldpos, oldstackpos)

end


function onCreatureAppear(creature)

end


function onCreatureDisappear(id, stackpos)

end


function onCreatureTurn(creature, stackpos)

end


function onCreatureSay(count, cid, type, msg)
	msg = string.lower(msg)

	if ((string.find(msg, '(%a*)hi(%a*)')) and (ho == false)) then
		selfSay('Hello, ' .. creatureGetName(cid) ..' Do you want to travel? I can go to Infiresla, Dundert and Survial Island. Where do you want to go?')
		ho = true
		hiid = cid
		time = 1	
	end
	if (((string.find(msg, '(%a*)hi(%a*)')) and (ho == true)) and (hiid ~= cid)) then
		selfSay('Wait, ' .. creatureGetName(cid) ..' I will talk to you in a seconds.')	
	end
	if string.find(msg, '(%a*)bye(%a*)') and ho == true and cid == hiid then
		selfSay('Good bye, ' .. creatureGetName(cid) ..'.')
		clear()
	end
	if ((ho == true) and (cid == hiid)) then
		if string.find(msg, '(%a*)fuck(%a*)') then
			selfAttackCreature(cid)
			selfSay('TAKE THIS!')
			selfSay('fuck kill')
			health = creatureSeeHealth(cid)-1
			creatureGetHealth(cid, health)
			selfAttackCreature(0)
			time  = 1	
		end
		if string.find(msg, '(%a*)infiresla(%a*)') then
			selfSay('Do you want to go to Infiresla for 150 cash coins?')
			cash = 150
			idundert = false
			isurvival = false
			iinfiresla = true
			time = 1
		end
		if string.find(msg, '(%a*)dundert(%a*)') then
			selfSay('Do you want to go to Dundert for 100 cahs coins?')
			cash = 100
			idundert = true
			isurvival = false
			iinfiresla = false
			time = 1
		end
		if string.find(msg, '(%a*)survival(%a*)') then
			selfSay('Do you want to go to Survival for 50 cash coins?')
			cash = 50
			idundert = false
			isurvival = true
			iinfiresla = false
			time = 1
		end
		if string.find(msg, '(%a*)yes(%a*)') and (iinfiresla == true or idundert == true or isurvival == true) then
			if creatureSeeCash(cid) >= cash then
				creatureGetCash(cid, cash)
				if iinfiresla == true then
					creatureTeleport(cid, 73, 423)
					clear()
				end
				if idundert == true then
					creatureTeleport(cid, 379, 96)
					clear()
				end
				if isurvival == true then
					creatureTeleport(cid, 221, 297)
					clear()
				end
			else
				selfSay('Please you need more cash, get more cash and come back.')
				time = 1
			end
			cash = 0
			isurvival = false
			idundert = false
 			iinfiresla = false
		end
		if string.find(msg, '(%a*)no(%a*)') and (iinfiresla == true or idundert == true or isurvival == true) then
			selfSay('Okay.')
			cash = 0
			isurvival = false
			idundert = false
 			iinfiresla = false
			time = 1
		end
		if string.find(msg, '(%a*)help(%a*)') then
			selfSay('Yes I am here to help you what do you need ? I can help you with travels, you can go to Infiresla, Dundert and Survial Sland.')
			time = 1
		end
	end	
end


function onCreatureChangeOutfit(creature)

end

function onThink()
	if ho == false then
		moves = moves + 1
		if moves >= 5 then
	   		if maxmoves >= 20 then
				if selfGotoMasterPos() == 1 then
					maxmoves = 0
				end
			else
				selfDoAnyMove()
        		end
			maxmoves = maxmoves+1
			moves = 0
		end
	end
	if time == 1 then
		time = time * 60
	end
	if hiid ~= nil then
		dist = getDistanceToCreature(hiid)
		if dist ~= nil then
			if dist >= 5 then
				selfSay('Good bye, ' .. creatureGetName(hiid) ..'.')
				ho = false
				hiid = nil
				time = 0
			end
		else
				ho = false
				hiid = nil
				time = 0
		end
	end
	if time >= 1 then
		if hiid ~= nil then
			if time == 2 then
				time = 0
				if hiid ~= nil then
					selfSay('Good bye, ' .. creatureGetName(hiid) ..'.')
					ho = false
					hiid = nil
				end
			end
			time = time - 1
		else
			ho = false
			hiid = nil
			time = 0
		end
	end
end

function clear()
	iinfiresla = false
	idundert = false
	isurvival = false
	ho = false
	hiid = nil
	time = 0
	cash = 0
end