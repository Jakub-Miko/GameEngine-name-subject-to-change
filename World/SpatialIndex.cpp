#include "SpatialIndex.h"
#include <World/Components/TransformComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <variant>

SpatialIndex::SpatialIndex(World& world) : octree_base(world)
{

}

Octree::Octree(const std::vector<Entity>& entities, World& world, int max_node_count) : child_nodes(), entity_list(), max_node_count(max_node_count)
{
	entt::registry& registry = world.GetRegistry();
	std::array<std::vector<Entity>, 8> entities_lists;
	if (entities.size() <= max_node_count) {
		entity_list = entities;
		return;
	}
	active = true;
	for (Entity entity : entities) {
		ProcessEntity(world, entities_lists, entity);
	}

	for (int index = 0; index < 8; index++) {
		child_nodes.emplace_back(entities_lists[index],world, max_node_count);
	}

}

Octree::Octree(World& world, int max_node_count) : child_nodes(), entity_list(), max_node_count(max_node_count)
{
	entt::registry& registry = world.GetRegistry();
	std::array<std::vector<Entity>, 8> entities_lists;
	auto view = registry.view<TransformComponent, BoundingVolumeComponent>();
	active = true;
	for (Entity entity : view) {
		ProcessEntity(world, entities_lists, entity);
	}

	for (int index = 0; index < 8; index++) {
		child_nodes.emplace_back(entities_lists[index], world, max_node_count);
	}
}

void Octree::ProcessEntity(World& world, std::array<std::vector<Entity>, 8>& list, Entity entity)
{
	if (world.HasComponent<TransformComponent>(entity) && world.HasComponent<BoundingVolumeComponent>(entity)) {
		TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
		BoundingVolumeComponent& bounding_volume = world.GetComponent<BoundingVolumeComponent>(entity);
		char index = 0;
		bool bordering = false;
		std::visit([&index, &transform, &bordering](auto& bounding_volume) {
			if (OverlapResult result = bounding_volume.OverlapsPlane(GetPlane(PlaneAxis::X), transform.TransformMatrix); result == OverlapResult::FULL_OVERLAP) {
				index &= GetIndexByPos(1, 0, 0);
			}
			else if (result == OverlapResult::PARTIAL_OVERLAP) {
				bordering = true; 
			}

			if (OverlapResult result = bounding_volume.OverlapsPlane(GetPlane(PlaneAxis::Y), transform.TransformMatrix); result == OverlapResult::FULL_OVERLAP) {
				index &= GetIndexByPos(0, 1, 0);
			}
			else if (result == OverlapResult::PARTIAL_OVERLAP) {
				bordering = true;
			}

			if (OverlapResult result = bounding_volume.OverlapsPlane(GetPlane(PlaneAxis::Z), transform.TransformMatrix); result == OverlapResult::FULL_OVERLAP) {
				index &= GetIndexByPos(0, 0, 1);
			}
			else if (result == OverlapResult::PARTIAL_OVERLAP) {
				bordering = true;
			}

			}, bounding_volume.bounding_volume_variant);

		if (bordering) {
			entity_list.push_back(entity);

		}
		else {
			list[index].push_back(entity);
		}
	}
}
