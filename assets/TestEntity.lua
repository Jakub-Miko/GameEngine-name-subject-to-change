@Entity 

{
  "Properties": [
    {
      "Name": "prop_int",
      "Type": "int",
      "Value": 1
    }
  ],

  "Children": [

  ]

}


@Entity:Construction_Script 


function OnConstruct()

pos = GetMousePosition()

pos = GetMousePosition()


SetScale({x=0.1,y=0.1,z=1.0})
SetTranslation({x=pos.x,y=pos.y,z=0.0})
SetSquareComponent({x = 0.5, y=0.0, z=1.0, w=1.0 })
UseInlineScript()


end


@Entity:Inline_Script

function OnStart()



end


function OnUpdate(delta_time)

pos = GetPos()

if not IsMouseButtonPressed(MouseButtonCode.MOUSE_BUTTON_LEFT) then	
	MoveSquare(pos.x + GetProperty_INT("prop_int") *0.001*delta_time,pos.y)
else 
	MoveSquare(pos.x -0.001*delta_time,pos.y)
end

end