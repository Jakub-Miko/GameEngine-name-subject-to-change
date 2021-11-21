function OnUpdate(delta_time)
if IsMouseButtonPressed(MouseButtonCode.MOUSE_BUTTON_LEFT) ~= 1 then	
	MoveSquare(0.001*delta_time,0)
else 
	MoveSquare(-0.001*delta_time,0)
end
end
