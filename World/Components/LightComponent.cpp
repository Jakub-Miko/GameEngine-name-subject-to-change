#include "LightComponent.h"
#include <Application.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <stdexcept>

void ComponentInitProxy<LightComponent>::OnCreate(World& world, Entity entity) {
	if (world.HasComponentSynced<MeshComponent>(entity))
		world.RemoveComponent<MeshComponent>(entity);

	if (world.HasComponentSynced<BoundingVolumeComponent>(entity)) 
		world.RemoveComponent<BoundingVolumeComponent>(entity);

	auto& light_comp = world.GetComponent<LightComponent>(entity);
	switch (light_comp.type) {
	case LightType::POINT:
		world.SetComponent<BoundingVolumeComponent>(entity, BoundingPointLightSphere(light_comp.GetLightRange()));
		break;
	case LightType::DIRECTIONAL:
		world.SetComponent<BoundingVolumeComponent>(entity, BoundingInfinity());
		break;
	default:
		throw std::runtime_error("Unsupported Light Type");
	}

}

void ComponentInitProxy<LightComponent>::OnDestroy(World& world, Entity entity) {
	if (world.HasComponentSynced<BoundingVolumeComponent>(entity)) {
		world.RemoveComponent<BoundingVolumeComponent>(entity);
	}
	if (world.HasComponentSynced<ShadowCasterComponent>(entity)) {
		world.RemoveComponent<ShadowCasterComponent>(entity);
	}
}

void LightComponent::ChangeType(LightType type, Entity ent)
{
	if (Application::GetWorld().HasComponentSynced<LightComponent>(ent)) {
		LightComponent& light_comp = Application::GetWorld().GetComponentSync<LightComponent>(ent);
		if (type != light_comp.type) {
			switch (type) {
			case LightType::DIRECTIONAL:
				Application::GetWorld().SetComponent<BoundingVolumeComponent>(ent, BoundingInfinity());
				light_comp.type = LightType::DIRECTIONAL;
				break;
			case LightType::POINT:
				Application::GetWorld().SetComponent<BoundingVolumeComponent>(ent, BoundingPointLightSphere(light_comp.range));
				light_comp.type = LightType::POINT;
				break;
			}
			Application::GetWorld().MarkEntityDirty(ent);

			//if (Application::GetWorld().HasComponentSynced<ShadowCasterComponent>(ent)) {
			//	if (type != LightType::DIRECTIONAL) {
			//		throw std::runtime_error("Shadows are currently only supported for Directional Lights");
			//	}
			//}
		}
	}
	else {
		throw std::runtime_error("Can't change light type of an entity without a light component");
	}
}

void LightComponent::SetRange(float new_range, Entity ent)
{
	if (Application::GetWorld().HasComponentSynced<LightComponent>(ent)) {
		LightComponent& light_comp = Application::GetWorld().GetComponentSync<LightComponent>(ent);
		if (light_comp.type != LightType::POINT) {
			throw std::runtime_error("Can't change attenuation on a non-point light");
		} 
		light_comp.range = new_range;
		Application::GetWorld().SetComponent<BoundingVolumeComponent>(ent, BoundingPointLightSphere(new_range));
		Application::GetWorld().MarkEntityDirty(ent);
	}
	else {
		throw std::runtime_error("Can't change light type of an entity without a light component");
	}
}

void LightComponent::SetLightColor(glm::vec4 color_in, Entity ent)
{
	if (Application::GetWorld().HasComponentSynced<LightComponent>(ent)) {
		LightComponent& light_comp = Application::GetWorld().GetComponentSync<LightComponent>(ent);
		light_comp.color = color_in;
	}
	else {
		throw std::runtime_error("Can't change light type of an entity without a light component");
	}
}
