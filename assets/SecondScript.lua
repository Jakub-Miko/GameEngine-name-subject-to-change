function OnStart() 
jit.on()
EnableKeyPressedEvents()

end

function OnUpdate(delta_time)
	
	if not PropertyExists("value") then
		SetProperty_FLOAT("value",0)
	end
	prop = GetProperty_FLOAT("value")
	pos = math.sin(prop)*0.5
	pos2 = math.cos(prop)*0.5

	if IsKeyPressed(KeyCode.KEY_SPACE) then
		SetProperty_FLOAT("value", prop+0.01*delta_time)
	else
		SetProperty_FLOAT("value", prop-0.01*delta_time)
	end

	pos_sq = GetPos()

	position2 = GetMousePosition()
    position2.x = position2.x / (800 / 2);
    position2.y = position2.y / (600 / 2);
    position2.x = position2.x - 1;
    position2.y = position2.y - 1;
	position2.y = position2.y * -1;

	MoveSquare(position2.x + pos2,position2.y + pos)
	
end

function OnKeyPressed(e) 



ent = CreateEntity("TestEntity.lua",0)
SetEntityProperty_INT(ent,"prop_int", 2);

end