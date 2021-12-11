@Entity

{
	"Properties": [
		{
			"Name" : "prop_int",
			"Type" : "int",
			"Value" : 6
		}
	],

	"Children" : [
		"lol", "Hello There"
	]

}

@Entity:Inline_Script

function OnUpdate(delta_time)
pos = GetPos()

if not IsMouseButtonPressed(MouseButtonCode.MOUSE_BUTTON_LEFT) then	
	MoveSquare(pos.x + 0.001*delta_time,pos.y)
else 
	MoveSquare(pos.x -0.001*delta_time,pos.y)
end

end


@Entity:Construction_Script

function OnConstruct()

SetSquareComponent()

end