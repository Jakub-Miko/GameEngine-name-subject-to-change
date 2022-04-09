@Entity 

{
  "Properties": [
    {
        "name": "hello_world",
        "type": "vec4",
        "value": {
            "x": 5.0,
            "y": 5.0,
            "z": 5.0,
            "w": 5.0
        }
    }
  ],

  "Children": [
    "ComplexEntityChild.lua"
  ]

}


@Entity:Construction_Script 


function OnConstruct()

pos = GetMousePosition()

pos = GetMousePosition()

SetTranslation({x=pos.x,y=pos.y,z=0.0})

UseInlineScript()

end


@Entity:Inline_Script

function OnStart()



end


function OnUpdate(delta_time)

pos = GetMousePosition()
MoveSquare(pos.x,pos.y)

end