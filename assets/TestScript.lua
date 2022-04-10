@Entity

{
	"Properties": [
		{
			"name" : "position",
			"type" : "vec3",
			"value" : {
				"x": 0,
				"y": 0,
				"z": 0
			}
		}
	],

	"Children" : [
		
	]

}

@Entity:Inline_Script


function OnStart()

prop = GetProperty_VEC3("position")

MoveSquare(prop.x, prop.y)

end


function OnUpdate(delta_time)

end


@Entity:Construction_Script

function OnConstruct()

SetScale({x=0.1,y=0.1,z=1.0})
SetSquareComponent({x = 0.5, y=0.0, z=1.0, w=1.0 })
UseInlineScript()

end