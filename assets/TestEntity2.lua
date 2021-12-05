@Entity 

{
  "Properties": [
    {
      "Name": "prop_int",
      "Type": "int",
      "Value": 5
    }
  ],

  "Children": [
    
  ]

}


@Entity:Construction_Script 


function OnConstruct()

pos = GetMousePosition()

pos = GetMousePosition()
pos.x = pos.x / (800 / 2)
pos.y = pos.y / (600 / 2)
pos.x = pos.x - 1
pos.y = pos.y - 1
pos.y = pos.y * -1


SetScale({x=0.1,y=0.1,z=1.0})
SetTranslation({x=pos.x,y=-pos.y,z=0.0})
SetSquareComponent({x = 0.5, y=0.0, z=1.0, w=1.0 })
UseInlineScript()


end


@Entity:Inline_Script

function OnStart()

print("Spawned")

end


function OnUpdate(delta_time)

pos = GetPos()

if not IsMouseButtonPressed(MouseButtonCode.MOUSE_BUTTON_LEFT) then	
	MoveSquare(pos.x - 0.001*delta_time,pos.y)
else 
	MoveSquare(pos.x + 0.001*delta_time,pos.y)
end

end