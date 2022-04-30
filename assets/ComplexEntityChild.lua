@Entity 

{
  "Properties": [
    
  ],

  "Children": [
    
  ]

}


@Entity:Construction_Script 


function OnConstruct()
jit.on()

SetScale({x=0.2,y=0.2,z=1.0})
SetTranslation({x=0.0,y=0.0,z=0.0})
SetSquareComponent({x = 0.0, y=0.5, z=1.0, w=1.0 })
UseInlineScript()


end


@Entity:Inline_Script

function OnStart()
jit.on()


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

	position2 = {x=0,y=0}

	MoveSquare(position2.x + pos2,position2.y + pos)

end