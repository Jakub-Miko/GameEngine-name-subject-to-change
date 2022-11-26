#include "BoundingVolumeComponent.h"
#include <Application.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/SkeletalMeshComponent.h>

BoundingVolumeComponent::bounding_volume_variant_type BoundingVolumeComponent::GetBoundingVolume(Entity ent) {
	auto& world = Application::GetWorld();
	if (!world.HasComponent<BoundingVolumeComponent>(ent)) {
		if (world.HasComponent<MeshComponent>(ent)) {
			return bounding_volume_variant_type(world.GetComponent<MeshComponent>(ent).GetMesh()->GetBoundingBox());
		}
		else if (world.HasComponent<SkeletalMeshComponent>(ent)) {
			return bounding_volume_variant_type(world.GetComponent<SkeletalMeshComponent>(ent).GetMesh()->GetBoundingBox());
		}
		else {
			return bounding_volume_variant_type(NullBoundingVolume());
		}

	}
	else {
		BoundingVolumeComponent& bounding_volume_comp = world.GetComponent<BoundingVolumeComponent>(ent);
		return bounding_volume_comp.bounding_volume_variant;
	}
}