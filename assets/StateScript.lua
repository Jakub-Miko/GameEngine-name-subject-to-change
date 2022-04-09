function OnAttach()


end

function OnUpdate(delta_time)

end

function OnKeyPressed(key) 

if(key.press_type == KeyPressType.KEY_PRESS and key.key_code == KeyCode.KEY_V) then

entity = CreateEntity("TestScript.lua",-1)


end

end