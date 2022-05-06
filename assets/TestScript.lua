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
jit.on()
SetTranslation(vec3({prop.x, prop.y, 0}))

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

	position2 = GetProperty_VEC3("position")

	SetTranslation(vec3({position2.x + pos2,position2.y + pos, z = 0 }))

end


@Entity:Construction_Script

function OnConstruct()
jit.on()
SetScale({x=0.1,y=0.1,z=1.0})
SetSquareComponent({x = 0.5, y=0.0, z=1.0, w=1.0 })
UseInlineScript()

end