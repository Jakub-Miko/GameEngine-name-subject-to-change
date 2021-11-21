function OnUpdate(delta_time)
	
	if PropertyExists("value") ~= 1 then
		SetProperty_FLOAT("value",0)
	end
	prop = GetProperty_FLOAT("value")
	pos = math.sin(prop)*0.5
	pos2 = math.cos(prop)*0.5
	
	if IsKeyPressed(KeyCode.KEY_SPACE) == 1 then
		SetProperty_FLOAT("value", prop+0.01*delta_time)
	else
		SetProperty_FLOAT("value", prop+-0.01*delta_time)
	end


	MoveSquare(pos2,pos)
	
end