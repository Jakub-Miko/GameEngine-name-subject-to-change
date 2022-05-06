#include "GlobalEntityModule.h"
#include <ThreadManager.h>
#include <World/Systems/ScriptSystemManagement.h>


void GlobalEntityModule::OnRegisterModule(ModuleBindingProperties& props)
{
	props.Add_FFI_declarations(R"(
	typedef struct entity { uint32_t id; } entity;
	)");

	props.Add_FFI_aliases({
		{"struct entity", "entity"}
		});

}
