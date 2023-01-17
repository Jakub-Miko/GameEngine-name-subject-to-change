#include "GlobalEntityModule.h"
#include <ThreadManager.h>
#include <World/Systems/ScriptSystemManagement.h>

extern "C" {

	bool IsEntityValid_L(entity ent) {
		return Entity(ent.id) != Entity() && Application::Get()->GetWorld().EntityIsValid(Entity(ent.id));
	}

}

void GlobalEntityModule::OnRegisterModule(ModuleBindingProperties& props)
{
	props.Add_FFI_declarations(R"(
	typedef struct entity { uint32_t id; } entity;
	bool IsEntityValid_L(entity ent);
	)");

	props.Add_FFI_aliases({
		{"struct entity", "entity"},
		{"IsEntityValid_L", "IsEntityValid"}
		});

}
