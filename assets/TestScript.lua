@Entity

{
	"Properties": [
		{
			"name" : "prop_int",
			"type" : "vec4",
			"value" : {
				"x": 0,
				"y": 0,
				"z": 0,
				"w": 0
			}
		}
	],

	"Children" : [
		
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

SetSquareComponent({x = 0,y = 0, z = 0, w = 0})

end