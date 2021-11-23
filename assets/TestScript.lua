function OnUpdate(delta_time)
if not IsMouseButtonPressed(MouseButtonCode.MOUSE_BUTTON_LEFT) then	
	MoveSquare(0.001*delta_time,0)
else 
	MoveSquare(-0.001*delta_time,0)
end
end
