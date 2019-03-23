time = 0
hiid = nil
ho = false
thing = 0
cash = 0
things = 1
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

	if ((string.find(msg, '(%a*)hi(%a*)')) and (ho == false)) and getDistanceToCreature(cid) < 5 then
		selfSay('Hello, ' .. creatureGetName(cid) ..' What do you want? I sell runes, shovels, ropes, picks and backpacks.')
		ho = true
		hiid = cid
		time = 1	
	end
	if (((string.find(msg, '(%a*)hi(%a*)')) and (ho == true)) and (hiid ~= cid)) and getDistanceToCreature(cid) < 5 then
		selfSay('Wait, ' .. creatureGetName(cid) ..' I will talk to you in a seconds.')	
	end
	if string.find(msg, '(%a*)bye(%a*)') and ho == true and cid == hiid then
		selfSay('Good bye, ' .. creatureGetName(cid) ..'.')
		ho = false
		time = 0
		hiid = nil	
	end
	if ((ho == true) and (cid == hiid)) then
		if count <= 0 then
			count = 1
		end
		if string.find(msg, '(%a*)fuck(%a*)') or string.find(msg, '(%a*)ass(%a*)') or string.find(msg, '(%a*)asshole(%a*)') or string.find(msg, '(%a*)suck(%a*)') or string.find(msg, '(%a*)bitch(%a*)') then
			selfAttackCreature(cid)
			selfSay('TAKE THIS!')
			selfSay('fuck kill')
			health = creatureSeeHealth(cid)-1
			creatureGetHealth(cid, health)
			selfAttackCreature(0)
			time = 1	
		end
		if string.find(msg, '(%a*)rune(%a*)') then
			selfSay('Do you want to buy ' .. count ..'  rune(s) for ' .. (count*10) .. ' cash coins?')
			thing = 1610
			cash = count*10
			time = 1
			things = count
		end
		if string.find(msg, '(%a*)shovel(%a*)') then
			selfSay('Do you want to buy ' .. count ..'  shovel(s) for ' .. (count*50) .. ' cash coins?')
			thing  = 1811
			cash = count*50
			time = 1
			things = count
		end
		if string.find(msg, '(%a*)pick(%a*)') then
			selfSay('Do you want to buy ' .. count ..'  pick(s) for ' .. (count*100) .. ' cash coins?')
			thing  = 1810
			cash = count*100
			time = 1
			things = count
		end
		if string.find(msg, '(%a*)rope(%a*)') then
			selfSay('Do you want to buy ' .. count ..' rope(s) for ' .. (count*50) .. ' cash coins?')
			thing  = 1497
			cash = count*50
			time = 1
			things = count
		end
		if string.find(msg, '(%a*)backpack(%a*)') then
			selfSay('Do you want to buy ' .. count ..' backpack(s) for ' .. (count*20) .. ' cash coins?')
			thing  = 1411
			cash = count*20
			time = 1
			things = count
		end
		if string.find(msg, '(%a*)yes(%a*)') and thing >= 100 and thing <= 2750 then
			if creatureSeeCash(cid) >= cash and thing ~= 0 then
				total = 0
				while total < things do
					creatureAddItem(thing, 0, cid, 0)
					total = total + 1
				end
				selfSay('Here it is.')
				creatureGetCash(cid, cash)
			else
				selfSay('Please you need more cash, get more cash and come back.')
			end
			thing = 0
			cash = 0
			things = 1
			time = 1
		end
		if string.find(msg, '(%a*)no(%a*)') then
			selfSay('Okay.')
			thing = 0
			cash = 0
			things = 1
			time = 1
		end
		if string.find(msg, '(%a*)help(%a*)') then
			selfSay('Yes I am here to help you what do you need ? I can help you with travels, spells and equipaments.')
			time = 1
		end
		if string.find(msg, '(%a*)spells(%a*)') then
			selfSay('I know only the runes spells. They are adura vita, adori gran, adori gran flam, adevo mas hur, adori vita vis, adura mana and adevo grav tera.')
			time = 1
		end
		if string.find(msg, '(%a*)equipaments(%a*)') then
			selfSay('I sell shovels, runes, ropes, picks and backpacks.')
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
