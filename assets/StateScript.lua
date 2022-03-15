function OnAttach()


end

function OnUpdate(delta_time)

end

function OnKeyPressed(key) 

if(key.press_type == KeyPressType.KEY_PRESS) then

CreateEntity("TestEntityGroup.lua",0)

end

end