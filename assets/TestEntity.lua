@Section:Root

@Entity 

{
  "Properties": [
    {
      "name": "prop_int",
      "type": "int",
      "value": 1
    }
  ],

  "Children": [

  ],

  "Components": {
		"LabelComponent" : {
			"label": "Test 2"
		}
  }

}


@Entity:Construction_Script 


function OnConstruct()

pos = GetMousePosition()

pos = GetMousePosition()

jit.on()
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
	
else 
	
end

end

@EndSection