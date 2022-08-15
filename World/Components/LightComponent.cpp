#include "LightComponent.h"
#include <Application.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <stdexcept>

void ComponentInitProxy<LightComponent>::OnCreate(World& world, Entity entity) {
	if (world.HasComponentSynced<MeshComponent>(entity) != world.HasComponentSynced<BoundingVolumeComponent>(entity))
		throw std::runtime_error("Light component can't be assigned to an entity with a mesh component or a predefined bounding volume component");
	auto& light_comp = world.GetComponent<LightComponent>(entity);
	switch (light_comp.type) {
	case LightType::POINT:
		world.SetComponent<BoundingVolumeComponent>(entity, BoundingPointLightSphere(light_comp.CalcRadiusFromAttenuation()));
		break;
	case LightType::DIRECTIONAL:
		world.SetComponent<BoundingVolumeComponent>(entity, BoundingInfinity());
		break;
	default:
		throw std::runtime_error("Unsupported Light Type");
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
				Application::GetWorld().SetComponent<BoundingVolumeComponent>(ent, BoundingPointLightSphere(light_comp.CalcRadiusFromAttenuation()));
				light_comp.type = LightType::POINT;
				break;
			}
			Application::GetWorld().MarkEntityDirty(ent);
		}
	}
	else {
		throw std::runtime_error("Can't change light type of an entity without a light component");
	}
}

void LightComponent::SetAttenuation(glm::vec3 attenuation_in, Entity ent)
{
	if (Application::GetWorld().HasComponentSynced<LightComponent>(ent)) {
		LightComponent& light_comp = Application::GetWorld().GetComponentSync<LightComponent>(ent);
		if (light_comp.type != LightType::POINT) {
			throw std::runtime_error("Can't change attenuation on a non-point light");
		} 
		light_comp.attenuation = attenuation_in;
		Application::GetWorld().SetComponent<BoundingVolumeComponent>(ent, BoundingPointLightSphere(light_comp.CalcRadiusFromAttenuation()));
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
		if (light_comp.type == LightType::POINT) {
			
			if (light_comp.color.w != color_in.w) {
				light_comp.color = color_in;
				Application::GetWorld().SetComponent<BoundingVolumeComponent>(ent, BoundingPointLightSphere(light_comp.CalcRadiusFromAttenuation()));
				Application::GetWorld().MarkEntityDirty(ent);
			}
			else {
				light_comp.color = color_in;
			}
		}
		else {
			light_comp.color = color_in;
		}
	}
	else {
		throw std::runtime_error("Can't change light type of an entity without a light component");
	}
}

float LightComponent::CalcRadiusFromAttenuation()
{
	return std::sqrt(1.0 / (attenuation.b * (0.035 / color.w)));;
}
