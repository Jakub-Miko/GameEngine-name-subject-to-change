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

norm_scree_pos = GetMousePosition()
offset = vec2({ x = 1.0, y =0.0 })

norm_scree_pos = offset + norm_scree_pos;

norm_scree_pos = norm_scree_pos * vec2({-1,-1});
norm_scree_pos.y = math.min(math.max(-0.8, norm_scree_pos.y),0.8)
distance = GetProperty_INT("distance");

if IsKeyPressed(KeyCode.KEY_UP) then
		SetProperty_INT("distance", distance - 1)
end

if IsKeyPressed(KeyCode.KEY_DOWN) then
		SetProperty_INT("distance", distance + 1)
end

pos = {
x = math.sin(norm_scree_pos.x * 3.1415926) * math.cos(norm_scree_pos.y * 3.1415926/2) ,
y = math.sin(norm_scree_pos.y * 3.1415926 / 2),
z = math.cos(norm_scree_pos.x * 3.1415926) * math.cos(norm_scree_pos.y * 3.1415926/2)
}

pos_mul = {x = pos.x* distance,y = pos.y* distance,z = pos.z* distance}

rot = quat_lookat(-vec3(pos), vec3({0,1,0}) )

SetTranslation(pos_mul)
SetRotation(rot)


end