function OnAttach()

camera = CreateEntity("CameraEntity.lua",-1)
SetPrimaryEntity(camera)
print(GetPrimaryEntity().id)

end

function OnUpdate(delta_time)

end

function OnKeyPressed(key) 

if(key.press_type == KeyPressType.KEY_PRESS and key.key_code == KeyCode.KEY_V) then

pos = GetMousePosition()

entity = CreateSerializableEntity("TestScript.lua",-1)
SetEntityProperty_VEC3(entity, "position", {x = pos.x, y = pos.y, z = 1.0})

end

if(key.press_type == KeyPressType.KEY_PRESS and key.key_code == KeyCode.KEY_P) then

print(GetPrimaryEntity().id)

end

end