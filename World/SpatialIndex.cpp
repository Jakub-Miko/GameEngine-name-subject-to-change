#include "SpatialIndex.h"
#include <World/Components/TransformComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Systems/BoxRenderer.h>
#include <variant>
#include <Application.h>

SpatialIndex::SpatialIndex(World& world, const BoundingBox& world_box ) : octree_base(nullptr,world_box,world), world(world), world_box(world_box)
{

}

Octree::Octree(Octree* parent,const BoundingBox& node_box, const std::vector<Entity>& entities, World& world, int max_node_count) : child_nodes(), entity_list(), max_node_count(max_node_count), node_box(node_box), parent(parent)
{
	child_nodes.reserve(8);
	for (int i = 0; i < 8; i++) {
		child_nodes.emplace_back();
	}
	Compute_Planes();
	entt::registry& registry = world.GetRegistry();
	std::array<std::vector<Entity>, 8> entities_lists;
	if (entities.size() <= max_node_count) {
		entity_list = entities;
		return;
	}
	for (Entity entity : entities) {
		ProcessEntity(world, entities_lists, entity);
	}

	for (int index = 0; index < 8; index++) {
		if (entities_lists[index].empty()) continue;
		active |= 1 << index;
		BoundingBox new_box(node_box.GetBoxSize() / glm::vec3(2.0), node_box.GetBoxOffset() + ((GetPosByIndex(index) * glm::vec3(2)) - glm::vec3(1)) * (node_box.GetBoxSize() / glm::vec3(4)));
		child_nodes[index] = Octree(this,new_box,entities_lists[index],world, max_node_count);
	}

}

void Octree::FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities)
{
	for (auto entity : entity_list) {
		auto& bounding_volume = world.GetComponent<BoundingVolumeComponent>(entity);
		auto& transform = world.GetComponent<TransformComponent>(entity);
		std::visit([&](auto& bounding_vol) {
			if (bounding_vol.OverlapsFrustum(frustum, transform.TransformMatrix) != OverlapResult::NO_OVERLAP) {
				entities.push_back(entity);
			}
			}, bounding_volume.bounding_volume_variant);
	}
	
	if (!active) return;

	for (int index = 0; index < 8; index++) {
		if (!(active & (1 << index))) continue;
		if (child_nodes[index].node_box.OverlapsFrustum(frustum, glm::mat4(1.0)) == OverlapResult::NO_OVERLAP) continue;
		child_nodes[index].FrustumCulling(world, frustum, entities);
	}

}

void Octree::Compute_Planes()
{
	planes[(int)PlaneAxis::X] = Plane(glm::vec3(1.0f, 0.0f, 0.0f), node_box.GetBoxOffset().x);
	planes[(int)PlaneAxis::Y] = Plane(glm::vec3(0.0f, 1.0f, 0.0f), node_box.GetBoxOffset().y);
	planes[(int)PlaneAxis::Z] = Plane(glm::vec3(0.0f, 0.0f, 1.0f), node_box.GetBoxOffset().z);
}

void Octree::VisualizeBoxes()
{
	auto ent = Application::GetWorld().GetPrimaryEntity();
	if (ent == Entity()) return;

	auto& cam = Application::GetWorld().GetComponent<CameraComponent>(ent);
	auto& trans = Application::GetWorld().GetComponent<TransformComponent>(ent);

	Render_Box(node_box, glm::mat4(1), cam, trans.TransformMatrix, PrimitivePolygonRenderMode::WIREFRAME);
	if (active) {
		for (int index = 0; index < 8; index++) {
			if (!(active & 1 << index)) continue;
			child_nodes[index].VisualizeBoxes();
		}
	}
}

Octree::Octree(Octree* parent,const BoundingBox& node_box, World& world, int max_node_count) : child_nodes(), entity_list(), max_node_count(max_node_count), node_box(node_box), parent(parent)
{
	child_nodes.reserve(8);
	for (int i = 0; i < 8; i++) {
		child_nodes.emplace_back();
	}
	Compute_Planes();
	entt::registry& registry = world.GetRegistry();
	std::array<std::vector<Entity>, 8> entities_lists;
	auto view = registry.view<TransformComponent, BoundingVolumeComponent>();
	for (Entity entity : view) {
		ProcessEntity(world, entities_lists, entity);
	}

	for (int index = 0; index < 8; index++) {
		if (entities_lists[index].empty()) continue;
		active |= 1 << index;
		BoundingBox new_box(node_box.GetBoxSize() / glm::vec3(2.0), node_box.GetBoxOffset() + ((GetPosByIndex(index) * glm::vec3(2)) - glm::vec3(1)) * (node_box.GetBoxSize() / glm::vec3(4)));
		child_nodes[index] = Octree(this,new_box, entities_lists[index], world, max_node_count);
	}
};

//Change plane coordinates hierarchically.

void Octree::ProcessEntity(World& world, std::array<std::vector<Entity>, 8>& list, Entity entity)
{
	if (world.HasComponent<TransformComponent>(entity) && world.HasComponent<BoundingVolumeComponent>(entity)) {
		TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
		BoundingVolumeComponent& bounding_volume = world.GetComponent<BoundingVolumeComponent>(entity);
		char index = 0;
		bool bordering = false;
		std::visit([&index, &transform, &bordering,this](auto& bounding_volume) {
			if (OverlapResult result = bounding_volume.OverlapsPlane(GetPlane(PlaneAxis::X), transform.TransformMatrix); result == OverlapResult::FULL_OVERLAP) {
				index |= GetIndexByPos(1, 0, 0);
			}
			else if (result == OverlapResult::PARTIAL_OVERLAP) {
				bordering = true; 
			}

			if (OverlapResult result = bounding_volume.OverlapsPlane(GetPlane(PlaneAxis::Y), transform.TransformMatrix); result == OverlapResult::FULL_OVERLAP) {
				index |= GetIndexByPos(0, 1, 0);
			}
			else if (result == OverlapResult::PARTIAL_OVERLAP) {
				bordering = true;
			}

			if (OverlapResult result = bounding_volume.OverlapsPlane(GetPlane(PlaneAxis::Z), transform.TransformMatrix); result == OverlapResult::FULL_OVERLAP) {
				index |= GetIndexByPos(0, 0, 1);
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
