#include "LocalEntityModule.h"
#include <ThreadManager.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <ThreadManager.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/World.h>
#include "GlobalEntityModule.h"
#include "MathModule.h"

extern "C" {

	entity GetCurrentEntity_L() {
		return entity{ ThreadManager::Get()->GetThreadLocalData<ScriptSystemVM>()->GetCurrentEntity().id };
	}

	void SetTranslation_L(vec3 translation) {
		glm::vec3* trans = reinterpret_cast<glm::vec3*>(&translation);
		Application::GetWorld().SetEntityTranslation(Entity{ GetCurrentEntity_L().id }, *trans);
	}

	void SetScale_L(vec3 scale) {
		glm::vec3* sc = reinterpret_cast<glm::vec3*>(&scale);
		Application::GetWorld().SetEntityScale(Entity{ GetCurrentEntity_L().id }, *sc);
	}

	void SetRotation_L(quat rot) {
		glm::quat* rt = reinterpret_cast<glm::quat*>(&rot);
		Application::GetWorld().SetEntityRotation(Entity{ GetCurrentEntity_L().id }, *rt);
	}

}


void LocalEntityModule::OnRegisterModule(ModuleBindingProperties& props)
{
	GlobalEntityModule().RegisterModule(props);
	MathModule().RegisterModule(props);

	props.Add_FFI_declarations(R"(
	entity GetCurrentEntity_L();
	void SetTranslation_L(vec3 translation);
	void SetScale_L(vec3 scale);
	void SetRotation_L(quat rot);

	)");

	props.Add_FFI_aliases({
		{"GetCurrentEntity_L", "GetCurrentEntity"},
		{"SetTranslation_L", "SetTranslation"},
		{"SetScale_L", "SetScale"},
		{"SetRotation_L", "SetRotation"}
		});

}
