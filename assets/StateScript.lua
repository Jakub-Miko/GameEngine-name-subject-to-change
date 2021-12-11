function OnAttach()

	CreateEntity("ComplexEntity.lua",0)

end

function OnUpdate(delta_time)

end

function OnKeyPressed(key) 

if(key.press_type == KeyPressType.KEY_PRESS and key.key_code == KeyCode.KEY_V) then

CreateEntity("TestEntityGroup.lua",0)

end

end