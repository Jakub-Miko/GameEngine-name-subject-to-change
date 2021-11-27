
function OnConstruct()

print("Im alive")

end

function OnUpdate(delta_time)
pos = GetPos()

if not IsMouseButtonPressed(MouseButtonCode.MOUSE_BUTTON_LEFT) then	
	MoveSquare(pos.x + 0.001*delta_time,pos.y)
else 
	MoveSquare(pos.x -0.001*delta_time,pos.y)
end

end
