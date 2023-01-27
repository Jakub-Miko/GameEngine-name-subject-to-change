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
	
	entity GetEntityByName_L(const char* name) {
		Entity ent = GetEntityByName(name);
		return entity{ ent.id };
	}

	void ApplyForce_L(const char* name, vec3 direction) {
		Entity ent = GetEntityByName(name);
		glm::vec3 dir = *reinterpret_cast<glm::vec3*>(&direction);
		Application::GetWorld().GetPhysicsEngine().ApplyForce(ent, dir);
	}

	void SetLinearVelocity_L(const char* name, vec3 velocity) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = *reinterpret_cast<glm::vec3*>(&velocity);
		Application::GetWorld().GetPhysicsEngine().SetLinearVelocity(ent, vel);
	}

	void SetAngularVelocity_L(const char* name, vec3 velocity) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = *reinterpret_cast<glm::vec3*>(&velocity);
		Application::GetWorld().GetPhysicsEngine().SetAngularVelocity(ent, vel);
	}

	vec3 GetLinearVelocity_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = Application::GetWorld().GetPhysicsEngine().GetLinearVelocity(ent);
		return *reinterpret_cast<vec3*>(&vel);
	}

	vec3 GetAngularVelocity_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = Application::GetWorld().GetPhysicsEngine().GetAngularVelocity(ent);
		return *reinterpret_cast<vec3*>(&vel);
	}


	void SetLinearFactor_L(const char* name, vec3 factor) {
		Entity ent = GetEntityByName(name);
		glm::vec3 fact = *reinterpret_cast<glm::vec3*>(&factor);
		Application::GetWorld().GetPhysicsEngine().SetLinearFactor(ent, fact);
	}

	void SetAngularFactor_L(const char* name, vec3 factor) {
		Entity ent = GetEntityByName(name);
		glm::vec3 fact = *reinterpret_cast<glm::vec3*>(&factor);
		Application::GetWorld().GetPhysicsEngine().SetAngularFactor(ent, fact);
	}

	void SetMass_L(const char* name, float mass) {
		Entity ent = GetEntityByName(name);
		Application::GetWorld().GetPhysicsEngine().SetMass(ent, mass);
	}

	void SetFriction_L(const char* name, float friction) {
		Entity ent = GetEntityByName(name);
		Application::GetWorld().GetPhysicsEngine().SetFriction(ent, friction);
	}

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

	vec3 GetChildTranslation_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 trans = Application::GetWorld().GetComponent<TransformComponent>(ent).translation;
		return *reinterpret_cast<vec3*>(&trans);
	}

	vec3 GetChildWorldTranslation_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 trans = Application::GetWorld().GetComponent<TransformComponent>(ent).TransformMatrix[3];
		return *reinterpret_cast<vec3*>(&trans);
	}

	vec3 GetChildScale_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 scale = Application::GetWorld().GetComponent<TransformComponent>(ent).size;
		return *reinterpret_cast<vec3*>(&scale);
	}

	quat GetChildRotation_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::quat rot = Application::GetWorld().GetComponent<TransformComponent>(ent).rotation;
		return *reinterpret_cast<quat*>(&rot);
	}

	void PlayAnimation_L(const char* name, const char* path) {
		Entity ent = GetEntityByName(name);
		if(!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id) 
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).SetAnimation(AnimationPlayback(AnimationManager::Get()->LoadAnimationAsync(FileManager::Get()->GetPath(path))));
	}

	void AddAnimationLayer_L(const char* name, const char* path, float weight) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		AnimationPlayback::AnimationPlaybackLayer layer;
		layer.anim = AnimationManager::Get()->LoadAnimationAsync(FileManager::Get()->GetPath(path));
		layer.playback_state = AnimationPlaybackState();
		layer.time = 0.0f;
		layer.weight = weight;
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().AddLayer(layer);
	}

	void RemoveAnimationLayer_L(const char* name) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().RemoveLayer();
	}

	void PromoteAnimationLayer_L(const char* name, int index) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().PromoteLayerToPrimary(index);
	}

	void ChangeLayerWeight_L(const char* name, int index, float weight) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().GetLayer(index).weight = weight;
	}

	void SetAnimationSpeedMatch_L(const char* name, int index, bool enable) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().GetLayer(index).speed_match = enable;
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
	LocalEntityModule().RegisterModule(props);
	LocalPropertySetModule().RegisterModule(props);

	props.Add_FFI_declarations(R"(
	void SetChildTranslation_L(const char * name,vec3 translation);
	void SetChildScale_L(const char* name, vec3 scale);
	void SetChildRotation_L(const char* name, quat rot);
	void PlayAnimation_L(const char* name, const char* path);
	void PlaySound_L(const char* name, const char* path);
	entity GetEntityByName_L(const char* name);
	void ApplyForce_L(const char* name, vec3 direction);
	void SetLinearVelocity_L(const char* name, vec3 velocity);
	void SetAngularVelocity_L(const char* name, vec3 velocity);
	void SetLinearFactor_L(const char* name, vec3 factor);
	void SetAngularFactor_L(const char* name, vec3 factor);
	void SetMass_L(const char* name, float mass);
	void SetFriction_L(const char* name, float friction);
	vec3 GetLinearVelocity_L(const char* name);
	vec3 GetAngularVelocity_L(const char* name); 
	vec3 GetChildTranslation_L(const char* name);
	vec3 GetChildWorldTranslation_L(const char* name);
	vec3 GetChildScale_L(const char* name);
	quat GetChildRotation_L(const char* name); 
	void AddAnimationLayer_L(const char* name, const char* path, float weight);
	void RemoveAnimationLayer_L(const char* name);
	void PromoteAnimationLayer_L(const char* name, int index);
	void ChangeLayerWeight_L(const char* name, int index, float weight);
	void SetAnimationSpeedMatch_L(const char* name, int index, bool enable);

	)");

	props.Add_FFI_aliases({
		{"SetChildTranslation_L","SetChildTranslation"},
		{"SetChildScale_L","SetChildScale"},
		{"SetChildRotation_L","SetChildRotation"},
		{"PlayAnimation_L","PlayAnimation"},
		{"PlaySound_L","PlaySound"},
		{"GetEntityByName_L","GetEntityByName"},
		{"ApplyForce_L","ApplyForce"},
		{"SetLinearVelocity_L","SetLinearVelocity"},
		{"SetAngularVelocity_L","SetAngularVelocity"},
		{"SetLinearFactor_L","SetLinearFactor"},
		{"SetAngularFactor_L","SetAngularFactor"},
		{"SetMass_L","SetMass"},
		{"SetFriction_L","SetFriction"},
		{"GetLinearVelocity_L","GetLinearVelocity"},
		{"GetAngularVelocity_L","GetAngularVelocity"},
		{"GetChildTranslation_L","GetChildTranslation"},
		{"GetChildWorldTranslation_L","GetChildWorldTranslation"},
		{"GetChildScale_L","GetChildScale"},
		{"GetChildRotation_L","GetChildRotation"},
		{"AddAnimationLayer_L","AddAnimationLayer"},
		{"RemoveAnimationLayer_L","RemoveAnimationLayer"},
		{"PromoteAnimationLayer_L","PromoteAnimationLayer"},
		{"ChangeLayerWeight_L","ChangeLayerWeight"},
		{"SetAnimationSpeedMatch_L", "SetAnimationSpeedMatch"}

		});
}
