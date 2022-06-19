@Entity 

{
  "Properties": [
		{
			"name" : "distance",
			"type" : "int",
			"value" : 5
		}
  ],

  "Children": [

  ]

}


@Entity:Construction_Script 


function OnConstruct()

res = GetWindowResolution()
jit.on()
UseInlineScript()
SetTranslation({x=0.0,y=0.0,z=-5.0})
SetCameraComponent(45.0, 0.1, 1000.0, res.x/res.y)

end


@Entity:Inline_Script



function OnStart() 

jit.on()

end

function OnUpdate(delta_time)




end