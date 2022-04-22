@Entity 

{
  "Properties": [
    
  ],

  "Children": [

  ]

}


@Entity:Construction_Script 


function OnConstruct()

res = GetWindowResolution()

UseInlineScript()
SetTranslation({x=0.0,y=0.0,z=-5.0})
SetCameraComponent(45.0, 0.1, 1000.0, res.x/res.y)

end


@Entity:Inline_Script



function OnUpdate(delta_time)

norm_scree_pos = GetMousePosition()
offset = { x = -1.0, y =0.0 }

norm_scree_pos.x = norm_scree_pos.x + offset.x;
norm_scree_pos.y = norm_scree_pos.y + offset.y;
norm_scree_pos.y = norm_scree_pos.y * -1;
distance = 5.0;

pos = {
x = math.cos(norm_scree_pos.x * 3.1415926) * math.cos(norm_scree_pos.y * 3.1415926/2) ,
y = math.sin(norm_scree_pos.y * 3.1415926 / 2),
z = math.sin(norm_scree_pos.x * 3.1415926) * math.cos(norm_scree_pos.y * 3.1415926/2)
}

print(pos.x .. pos.y .. pos.z)

end