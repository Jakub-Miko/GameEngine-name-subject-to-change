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
#include <Core/Defines.h>

/**
 * @brief Get the entity from the its name in the @ref Prefabs "prefab" (in the @ref LabelComponent )
 * @param name The LabelComponent name of the entity in the currently processed @ref Prefabs "prefab"
 * @return the Entity with the provided name
 * @warning This function throws if an invalid name was provided
 * @lua
 */
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
	
	/**
	 * @brief Get the entity from the its name in the @ref Prefabs "prefab" (in the @ref LabelComponent )
	 * @param name The LabelComponent name of the entity in the currently processed @ref Prefabs "prefab"
	 * @return the Entity with the provided name
	 * @warning This function throws if an invalid name was provided
	 * @lua
	 */
	LIBEXP entity GetEntityByName_L(const char* name) {
		Entity ent = GetEntityByName(name);
		return entity{ ent.id };
	}

	/**
	 * @brief Applies a physical force in a direction to a simulated object
	 * @param name Name of the object to apply the force to 
	 * @param direction the direction of the applied force
	 * @lua
	 */
	LIBEXP void ApplyForce_L(const char* name, vec3 direction) {
		Entity ent = GetEntityByName(name);
		glm::vec3 dir = *reinterpret_cast<glm::vec3*>(&direction);
		Application::GetWorld().GetPhysicsEngine().ApplyForce(ent, dir);
	}

	/**
	 * @brief Sets the linear velocity of a simulated object
	 * @param name Name of the object to set the linear velocity of 
	 * @param velocity the direction and magnitude of the velocity to set
	 * @lua
	 */
	LIBEXP void SetLinearVelocity_L(const char* name, vec3 velocity) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = *reinterpret_cast<glm::vec3*>(&velocity);
		Application::GetWorld().GetPhysicsEngine().SetLinearVelocity(ent, vel);
	}

	/**
	 * @brief Sets the angular velocity of a simulated object
	 * @param name Name of the object to set the angular velocity of
	 * @param velocity the direction specifies the axis of rotation and the magnitude specifies angular speed
	 * @lua
	 */
	LIBEXP void SetAngularVelocity_L(const char* name, vec3 velocity) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = *reinterpret_cast<glm::vec3*>(&velocity);
		Application::GetWorld().GetPhysicsEngine().SetAngularVelocity(ent, vel);
	}

	/**
	 * @brief Gets the linear velocity of a simulated object
	 * @param name Name of the object to get the linear velocity of
	 * @return The linear velocity vector
	 * @lua
	 */
	LIBEXP vec3 GetLinearVelocity_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = Application::GetWorld().GetPhysicsEngine().GetLinearVelocity(ent);
		return *reinterpret_cast<vec3*>(&vel);
	}

	/**
	 * @brief Gets the angular velocity of a simulated object
	 * @param name Name of the object to get the angular velocity of
	 * @return The angular velocity vector
	 * @lua
	 */
	LIBEXP vec3 GetAngularVelocity_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 vel = Application::GetWorld().GetPhysicsEngine().GetAngularVelocity(ent);
		return *reinterpret_cast<vec3*>(&vel);
	}

	/**
	 * @brief Sets The linear factor of a simulated object
	 * 
	 * The linear factor is basically the multiplier of linear movement, if it is 0 the object will fully resist any linear translation.
	 * Since it is a vector it can be used to limit the objects translation in certain directions.
	 * 
	 * @param name Name of the object to set the linear factor of 
	 * @param factor The linear factor vector to set
	 * @lua 
	 */
	LIBEXP void SetLinearFactor_L(const char* name, vec3 factor) {
		Entity ent = GetEntityByName(name);
		glm::vec3 fact = *reinterpret_cast<glm::vec3*>(&factor);
		Application::GetWorld().GetPhysicsEngine().SetLinearFactor(ent, fact);
	}

	/**
	 * @brief Sets The angular factor of a simulated object
	 *
	 * The angular factor is basically the multiplier of angular movement, if it is 0 the object will fully resist any rotation.
	 * Since it is a vector it can be used to limit the objects rotation around certain axis.
	 *
	 * @param name Name of the object to set the angular factor of
	 * @param factor The angular factor vector to set
	 * @lua
	 */
	LIBEXP void SetAngularFactor_L(const char* name, vec3 factor) {
		Entity ent = GetEntityByName(name);
		glm::vec3 fact = *reinterpret_cast<glm::vec3*>(&factor);
		Application::GetWorld().GetPhysicsEngine().SetAngularFactor(ent, fact);
	}

	/**
	 * @brief Sets the mass of a simulated object
	 * @param name Name of the object to set the mass of
	 * @param mass The mass to set
	 * @lua 
	 */
	LIBEXP void SetMass_L(const char* name, float mass) {
		Entity ent = GetEntityByName(name);
		Application::GetWorld().GetPhysicsEngine().SetMass(ent, mass);
	}

	/**
	 * @brief Sets the friction coefficient of a simulated object
	 * @param name Name of the object to set the friction coefficient of
	 * @param friction The friction coefficient to set
	 * @lua 
	 */
	LIBEXP void SetFriction_L(const char* name, float friction) {
		Entity ent = GetEntityByName(name);
		Application::GetWorld().GetPhysicsEngine().SetFriction(ent, friction);
	}

	/**
	 * @brief Sets the local translation of an object 
	 * @param name Name of the object to set the translation of
	 * @param translation the translation to set
	 * @lua 
	 */
	LIBEXP void SetChildTranslation_L(const char * name,vec3 translation) {
		Entity ent = GetEntityByName(name);
		glm::vec3* trans = reinterpret_cast<glm::vec3*>(&translation);
		Application::GetWorld().SetEntityTranslation(ent, *trans);
	}

	/**
	 * @brief Sets the local scale of an object
	 * @param name Name of the object to set the scale of
	 * @param scale the translation to set
	 * @lua
	 */
	LIBEXP void SetChildScale_L(const char* name, vec3 scale) {
		Entity ent = GetEntityByName(name);
		glm::vec3* sc = reinterpret_cast<glm::vec3*>(&scale);
		Application::GetWorld().SetEntityScale(ent, *sc);
	}

	/**
	 * @brief Sets the local rotation of an object
	 * @param name Name of the object to set the rotation of
	 * @param rot the rotation to set
	 * @lua
	 */
	LIBEXP void SetChildRotation_L(const char* name, quat rot) {
		Entity ent = GetEntityByName(name);
		glm::quat* rt = reinterpret_cast<glm::quat*>(&rot);
		Application::GetWorld().SetEntityRotation(ent, *rt);
	}

	/**
	 * @brief Gets the local translation of the object 
	 * @param name The object to get the translation of 
	 * @return The objects local translation 
	 * @lua
	 */
	LIBEXP vec3 GetChildTranslation_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 trans = Application::GetWorld().GetComponent<TransformComponent>(ent).translation;
		return *reinterpret_cast<vec3*>(&trans);
	}

	/**
	 * @brief Gets the world translation of the object
	 * @param name The object to get the translation of
	 * @return The objects world translation
	 * @lua
	 */
	LIBEXP vec3 GetChildWorldTranslation_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 trans = Application::GetWorld().GetComponent<TransformComponent>(ent).TransformMatrix[3];
		return *reinterpret_cast<vec3*>(&trans);
	}

	/**
	 * @brief Gets the local scale of the object
	 * @param name The object to get the scale of
	 * @return The objects local scale
	 * @lua
	 */
	LIBEXP vec3 GetChildScale_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::vec3 scale = Application::GetWorld().GetComponent<TransformComponent>(ent).size;
		return *reinterpret_cast<vec3*>(&scale);
	}

	/**
	 * @brief Gets the local rotation of the object
	 * @param name The object to get the rotation of
	 * @return The objects local rotation
	 * @lua
	 */
	LIBEXP quat GetChildRotation_L(const char* name) {
		Entity ent = GetEntityByName(name);
		glm::quat rot = Application::GetWorld().GetComponent<TransformComponent>(ent).rotation;
		return *reinterpret_cast<quat*>(&rot);
	}

	/**
	 * @brief Plays a skeletal animation on a skeletal mesh.
	 * @param name Name of the skeletal mesh to play the animation on 
	 * @param path Path to the animation to play
	 * @lua
	 */
	LIBEXP void PlayAnimation_L(const char* name, const char* path) {
		Entity ent = GetEntityByName(name);
		if(!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id) 
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).SetAnimation(SkeletalAnimationPlayback(SkeletalAnimationManager::Get()->LoadAnimationAsync(FileManager::Get()->GetPath(path))));
	}

	/**
	 * @brief Adds an animation layer to a skeletal mesh by pushing it onto the stack 
	 * @param name Name of the skeletal mesh to add the layer to  
	 * @param path The path of the animation file from which to create the new layer
	 * @param weight The interpolation weight of the new layer used for blending 
	 * @see @ref animation_page "More on animations and animation layers"
	 * @lua
	 */
	LIBEXP void AddAnimationLayer_L(const char* name, const char* path, float weight) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		SkeletalAnimationPlayback::AnimationPlaybackLayer layer;
		layer.anim = SkeletalAnimationManager::Get()->LoadAnimationAsync(FileManager::Get()->GetPath(path));
		layer.playback_state = AnimationPlaybackState();
		layer.time = 0.0f;
		layer.weight = weight;
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().AddLayer(layer);
	}

	/**
	 * @brief Removes an animation layer from a skeletal mesh by popping it off the stack
	 * @param name Name of the skeletal mesh to remove the layer of
	 * @see @ref animation_page "More on animations and animation layers"
	 * @lua
	 */
	LIBEXP void RemoveAnimationLayer_L(const char* name) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().RemoveLayer();
	}

	/**
	 * @brief Promotes the animation layer to the bottom of the layer stack, making it primary 
	 * 
	 * This if all animation layers have a weight of 0, this animation will be playing without regards of its own layer weight. The layer weight of the primary layer is thus irrelevant.
	 * 
	 * @param name Name of the skeletal mesh to promote the layer of 
	 * @param index The index on the stack of the animation layer to promote 
	 * 
	 * @see @ref animation_page "More on animations and animation layers"
	 * 
	 * @lua 
	 */
	LIBEXP void PromoteAnimationLayer_L(const char* name, int index) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().PromoteLayerToPrimary(index);
	}

	/**
	 * @brief Changes the layer weight of an animation layer
	 * 
	 * The layer weight determines the blending ratio of the current layer and the combined animations of the layers below it. This means that weight of 1 means it will override all previous layers.
	 * 
	 * @param name Name of the skeletal mesh which contains the layer to change the weight of
	 * @param index The index on the stack of the animation layer to change the weight of  
	 * @param weight The weight to set
	 *
	 * @see @ref animation_page "More on animations and animation layers"
	 *
	 * @lua  
	 */
	LIBEXP void ChangeLayerWeight_L(const char* name, int index, float weight) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().GetLayer(index).weight = weight;
	}

	/**
	 * @brief Enables or disables the animation speed matching on an animation layer 
	 * 
	 * Speed match matches the length of all animation layers which have this feature enabled. It does this by computing an average length based on the layer weight of speed match enabled layers.
	 * It then stretches all animations in the enabled layers to match this length. This is useful when blending between animations which need to be synched, but have different speeds, such as 
	 * a running and walking animations.
	 * 
	 * @param name Name of the skeletal mesh which contains the layer to enable or disable the speed match of 
	 * @param index The index on the stack of the animation layer to enable or disable the speed match of  
	 * @param enable Whether to enable or disable speed match
	 * 
	 * @see @ref animation_page "More on animations and animation layers"
	 * 
	 * @lua 
	 */
	LIBEXP void SetAnimationSpeedMatch_L(const char* name, int index, bool enable) {
		Entity ent = GetEntityByName(name);
		if (!Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error(std::string("Child with name: ") + name + " of entity: " + std::to_string(GetCurrentEntity_L().id)
			+ " does not have Skeletal Mesh Component.");
		Application::GetWorld().GetComponent<SkeletalMeshComponent>(ent).GetAnimation().GetLayer(index).speed_match = enable;
	}

	/**
	 * @brief Plays a sound file on an object with an AudioComponent
	 * @param name Name of the object with an AudioComponent to play the sound through
	 * @param path The path to the audio file to play
	 * @lua 
	 */
	LIBEXP void PlaySound_L(const char* name, const char* path) {
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
