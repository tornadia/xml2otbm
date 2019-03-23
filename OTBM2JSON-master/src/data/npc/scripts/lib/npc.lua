ox = 0
oy = 0
oz = 0
dalay = 0


function getDistanceToCreature(id)
	if id == 0 or id == nil then
		selfGotoIdle()
	end
	cx, cy, cz = creatureGetPosition(id)
	if cx == nil then
		return nil
	end
	qsx, qsy, qsz = selfGetPosition()
	return math.max(math.abs(qsx-cx), math.abs(qsy-cy))	
end


function moveToPosition(x,y,z)
	selfMoveTo(x,y,z)
end


function moveToCreature(id)
	if id == 0 or id == nil then
		selfGotoIdle()
	end
	tx,ty,tz=creatureGetPosition(id)
	if tx == nil then
		selfGotoIdle()
	else
	   moveToPosition(tx, ty, tz)
   	end
end


function selfGotoIdle()
	following = false
	attacking = false
	changetarget = false
	selfAttackCreature(0)
	target = 0
end


function moveToPlayer(id)
	if id == 0 or id == nil then
		selfGotoIdle()
	end
	ix,iy,iz=creatureGetPosition(id)
	if ix == nil then
		selfGotoIdle()
	else
		moveOnPlayer(ix, iy, iz)
   	end
end


function moveToCreature2(od)
	if od == 0 or od == nil then
		selfGotoIdle()
	end
	ux,uy,uz=creatureGetPosition(od)
	if ux == nil then
		selfGotoIdle()
	else
	   moveToPosition2(ux, uy, uz)
   	end
end


function moveToPosition2(lx,ly,lz)
	qx, qy, qz = selfGetPosition()
	px=qx-lx
	py=qy-ly
	pz=qz-lz
	if px > py then
		if px == 0 then
			selfMove(2)
			selfTurn(1)
		elseif py == 0  then
			selfMove(1)
			selfTurn(3)
		elseif py == 1 and px == 2 then
			selfMove(2)
		elseif py == -1 and px == 2 then
			selfMove(0)
		elseif py == -2 and px == 1 then
			selfMove(3)
		else
			selfMove(1)
		end			
	elseif px < py then
		if px == 0 then
			selfMove(0)
			selfTurn(0)
		elseif py == 0 then 
			selfMove(3)
			selfTurn(2)
		elseif py == 1 and px == -2 then
			selfMove(2)
		elseif py == -1 and px == -2 then
			selfMove(0)
		elseif py == 2 and px == 1 then
			selfMove(3)
		else
			selfMove(1)
		end
	else
		if (px == 1 and py == -1) or (px == 2 and py == -2) then
			selfMove(0)
		elseif (px == -1 and py == -1) or (px == -2 and py == -2) then
			selfMove(1)
		elseif (px == -1 and py == 1) or (px == -2 and py == 2) then
			selfMove(2)
		else
			selfMove(3)
		end
	end
end


function moveToPlayer2(id)
	if id == 0 or id == nil then
		selfGotoIdle()
	end
	mnx,mny,mnz=creatureGetPosition(id)
	if mnx == nil then
		selfGotoIdle()
	else
		moveOnPlayer2(mnx, mny, mnz)
   	end
end


function moveOnPlayer(gx,gy,gz)
	ax, ay, az = selfGetPosition()
	bx=ax-gx
	by=ay-gy
	bz=az-gz
	rind = math.random(10)
	if bx == 1 and by == 1 then
		if rind >= 5 and rind <= 8 then
			selfMove(2)
		elseif rind == 2 then

		else
			selfMove(3)
		end
	end
	if bx == -1 and by == -1 then
		if rind >= 3 and rind <= 8 then
			selfMove(0)
		elseif rind == 1 then

		else
			selfMove(1)
		end
	end
	if bx == 1 and by == -1 then
		if rind >= 6 and rind <= 10 then
			selfMove(3)
		elseif rind == 0 then

		else
			selfMove(0)
		end
	end
	if bx == -1 and by == 1 then
		if rind >= 2 and rind <= 6 then
			selfMove(1)
		elseif rind == 10 then

		else
			selfMove(2)
		end
	end
end


function moveOnPlayer2(kx, ky, kz)
	jx, jy, jz = selfGetPosition()
	hx=jx-kx
	hy=jy-ky
	hz=jz-kz
	rend = math.random(10)
	if hx == -1 and hy == 0 then
		if rend >= 1 and rend <= 4 then
			selfMove(2)
		elseif rind == 7 then

		else
			selfMove(0)
		end
	end
	if hx == 1 and hy == 0 then
		if rend >= 4 and rend <= 9 then
			selfMove(0)
		elseif rind == 3 then

		else
			selfMove(2)
		end
	end
	if hx == 0 and hy == -1 then
		if rend >= 6 and rend <= 10 then
			selfMove(3)
		elseif rind == 4 then

		else
			selfMove(1)
		end
	end
	if hx == 0 and hy == 1 then
		if rend >= 1 and rend <= 5 then
			selfMove(1)
		elseif rind == 6 then

		else
			selfMove(3)
		end
	end
end


function creatureTurn(targat)
	do
		dalay = dalay + 1
	end
	if dalay == 8 then
		dalay = 0
		nx,ny,nz = creatureGetPosition(targat)
		if nx ~= nil then
			mx, my, mz = selfGetPosition()
			rund = math.random(2)
			if my > ny then
				if mx > nx then
					if rund == 1 then
						selfTurn(3)
					elseif rund == 2 then
						selfTurn(0)
					end					
				elseif mx < nx then
					if rund == 1 then
						selfTurn(2)
					elseif rund == 2 then
						selfTurn(0)
					end
				else
					selfTurn(0)
				end
			elseif my < ny then
				if mx > nx then
					if rund == 1 then
						selfTurn(3)
					elseif rund == 2 then
						selfTurn(1)
					end
				elseif mx < nx then
					if rund == 1 then
						selfTurn(2)
					elseif rund == 2 then
						selfTurn(1)
					end
				else
					selfTurn(1)
				end
			else
				if mx > nx then
					selfTurn(3)
				else
					selfTurn(2)				
				end
			end
		end
	end
end


function checkAttackable()
	return (n == 'Demon' or n == 'Warlock' or n == 'Dragon Lord' or n == 'Giant Spider' or n == 'Necromancer' or n == 'Behemoth' or n == 'Fire Elemental' or n == 'Hero' or n == 'Dragon' or n == 'GM Guibil' or n == 'GM Dark-bart' or n == 'GM Henriquemineiro' or n == 'GM Gifah' or n == 'Sven' or n == 'Stone Golem' or n == 'Black Knight' or n == 'GM Felipe' or n == 'Djinn')
end


function findTarget(var, bestdist)
	var = 0
	newtarget = 0
	var = findPlayer()
	newtarget = var

	if newtarget == 0 then
		return false
	else
		if target == 0 then
			attacking = true
			target = newtarget
			return true
		end
	end
end

function findNewTarget(vare, bestdista, oldcre)
	vare = 0
	newtargot = 0
	vare = findPlayer()
	if vare == oldcre then
		vare = 0
	end
	newtargot = vare

	if newtargot == 0 then
		return false
	else
		if target == oldcre then
			attacking = true
			target = newtargot
			return true
		end
	end
end	