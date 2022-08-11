#include "LightComponent.h"
#include <World/Components/MeshComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <stdexcept>

void ComponentInitProxy<LightComponent>::OnCreate(World& world, Entity entity) {
	if (world.HasComponentSynced<MeshComponent>(entity) != world.HasComponentSynced<BoundingVolumeComponent>(entity))
		throw std::runtime_error("Light component can't be assigned to an entity with a mesh component or a predefined bounding volume component");
	auto& light_comp = world.GetComponent<LightComponent>(entity);
	switch (light_comp.type) {
	case LightType::POINT:
		world.SetComponent<BoundingVolumeComponent>(entity, BoundingSphere());
		break;
	case LightType::DIRECTIONAL:
		world.SetComponent<BoundingVolumeComponent>(entity, BoundingInfinity());
		break;
	default:
		throw std::runtime_error("Unsupported Light Type");
	}

}