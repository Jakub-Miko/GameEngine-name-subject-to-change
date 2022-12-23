#include "PrefabManipulationModule.h"
#include "LocalEntityModule.h"
#include <World/Components/SkeletalMeshComponent.h>
#include <World/Components/AudioComponent.h>
#include "LocalPropertySetModule.h"
#include <glm/glm.hpp>
#include <Application.h>
#include <stdexcept>
#include <variant>
#include <World/World.h>

static Entity GetEntityByName(const char* name) {
	Entity current_ent = Entity(GetCurrentEntity_L().id);
	auto& props = Application::GetWorld().GetComponent<DynamicPropertiesComponent>(current_ent).m_Properties;
	auto find = props.find(name);
	Entity ent;
	if (find != props.end()) {
		Script_Variant_type& prop = (*find).second;
		try {
			ent = std::get<Entity>(prop);
		}
		catch (std::bad_variant_access& e) {
			//std::cout << e.what() << "\n";
			throw std::runtime_error(std::string("Child with name: " )+ name + " of entity: " + std::to_string(current_ent.id) + " could not be found.");
		}
	}
	else {
		throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(current_ent.id) + " could not be found.");
	}
	return ent;
}

extern "C" {
	

	void SetChildTranslation_L(const char * name,vec3 translation) {
		Entity ent = GetEntityByName(name);
		glm::vec3* trans = reinterpret_cast<glm::vec3*>(&translation);
		Application::GetWorld().SetEntityTranslation(ent, *trans);
	}

	void SetChildScale_L(const char* name, vec3 scale) {
		Entity ent = GetEntityByName(name);
		glm::vec3* sc = reinterpret_cast<glm::vec3*>(&scale);
		Application::GetWorld().SetEntityScale(ent, *sc);
	}

	void SetChildRotation_L(const char* name, quat rot) {
		Entity ent = GetEntityByName(name);
		glm::quat* rt = reinterpret_cast<glm::quat*>(&rot);
		Application::GetWorld().SetEntityRotation(ent, *rt);
	}

	void PlayAnimation_L(const char* name, const char* path) {
		Entity ent = GetEntityByName(name);
		if(!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id) 
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).SetAnimation(AnimationPlayback(AnimationManager::Get()->LoadAnimationAsync(FileManager::Get()->GetPath(path))));
	}

	void PlaySound_L(const char* name, const char* path) {
		Entity ent = GetEntityByName(name);
		auto& world = Application::GetWorld();
		if (!world.HasComponent<AudioComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have an Audio Component.");
		auto& comp = world.GetComponent<AudioComponent>(ent);
		comp.PlayAudio(path);
	}

}

void PrefabManipulationModule::OnRegisterModule(ModuleBindingProperties& props)
{
	LocalEntityModule().OnRegisterModule(props);
	LocalPropertySetModule().OnRegisterModule(props);

	props.Add_FFI_declarations(R"(
	void SetChildTranslation_L(const char * name,vec3 translation);
	void SetChildScale_L(const char* name, vec3 scale);
	void SetChildRotation_L(const char* name, quat rot);
	void PlayAnimation_L(const char* name, const char* path);
	void PlaySound_L(const char* name, const char* path);
	)");

	props.Add_FFI_aliases({
		{"SetChildTranslation_L","SetChildTranslation"},
		{"SetChildScale_L","SetChildScale"},
		{"SetChildRotation_L","SetChildRotation"},
		{"PlayAnimation_L","PlayAnimation"},
		{"PlaySound_L","PlaySound"}
		});
}
