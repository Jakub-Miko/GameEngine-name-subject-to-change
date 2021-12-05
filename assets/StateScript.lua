function OnAttach()

	CreateEntity("TestEntity.lua")

end

function OnUpdate(delta_time)

end

function OnKeyPressed(key) 

if(key.press_type == KeyPressType.KEY_PRESS) then

print(CreateEntity("TestEntity.lua"))

end

end