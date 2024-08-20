#include "LocalEntityModule.h"
#include <ThreadManager.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <ThreadManager.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/World.h>
#include "GlobalEntityModule.h"
#include "MathModule.h"

extern "C" {

	/**
	 * @brief Gets the currently processed entity
	 * @return the currently processed entity
	 * @lua
	 */
	LIBEXP entity GetCurrentEntity_L() {
		return entity{ ThreadManager::Get()->GetThreadLocalData<ScriptSystemVM>()->GetCurrentEntity().id };
	}

	/**
	 * @brief Sets to local translation of the currently processed entity
	 * @param translation the new local translation 
	 * @lua
	 */
	LIBEXP void SetTranslation_L(vec3 translation) {
		glm::vec3* trans = reinterpret_cast<glm::vec3*>(&translation);
		Application::GetWorld().SetEntityTranslation(Entity{ GetCurrentEntity_L().id }, *trans);
	}

	/**
	 * @brief Sets to local scale of the currently processed entity
	 * @param scale the new local scale
	 * @lua
	 */
	LIBEXP void SetScale_L(vec3 scale) {
		glm::vec3* sc = reinterpret_cast<glm::vec3*>(&scale);
		Application::GetWorld().SetEntityScale(Entity{ GetCurrentEntity_L().id }, *sc);
	}

	/**
	 * @brief Sets to local scale of the currently processed entity
	 * @param rot the new local rotation
	 * @lua
	 */
	LIBEXP void SetRotation_L(quat rot) {
		glm::quat* rt = reinterpret_cast<glm::quat*>(&rot);
		Application::GetWorld().SetEntityRotation(Entity{ GetCurrentEntity_L().id }, *rt);
	}

	/**
	 * @brief Gets the current local translation of the currently processed entity
	 * @return the local translation of the currently processed entity
	 * @lua 
	 */
	LIBEXP vec3 GetTranslation_L() {
		glm::vec3 trans = Application::GetWorld().GetComponent<TransformComponent>(Entity{ GetCurrentEntity_L().id }).translation;
		vec3 translation = vec3{ trans.x, trans.y, trans.z };
		return translation;
	}

	/**
	 * @brief Gets the current local scale of the currently processed entity
	 * @return the local scale of the currently processed entity
	 * @lua
	 */
	LIBEXP vec3 GetScale_L() {
		glm::vec3 scl = Application::GetWorld().GetComponent<TransformComponent>(Entity{ GetCurrentEntity_L().id }).size;
		vec3 scale = vec3{ scl.x, scl.y, scl.z };
		return scale;
	}

	/**
	 * @brief Gets the current local rotation of the currently processed entity
	 * @return a quaternion the local rotation of the currently processed entity
	 * @lua
	 */
	LIBEXP quat GetRotation_L() {
		glm::quat rot = Application::GetWorld().GetComponent<TransformComponent>(Entity{ GetCurrentEntity_L().id }).rotation;
		quat rotation = *reinterpret_cast<quat*>(&rot);
		return rotation;
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
	vec3 GetTranslation_L();
	vec3 GetScale_L();
	quat GetRotation_L();
	)");

	props.Add_FFI_aliases({
		{"GetCurrentEntity_L", "GetCurrentEntity"},
		{"SetTranslation_L", "SetTranslation"},
		{"SetScale_L", "SetScale"},
		{"SetRotation_L", "SetRotation"},
		{"GetTranslation_L", "GetTranslation"},
		{"GetScale_L", "GetScale"},
		{"GetRotation_L", "GetRotation"}
		});

}
