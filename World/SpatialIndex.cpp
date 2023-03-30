#include "SpatialIndex.h"
#include <World/Components/MeshComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Systems/BoxRenderer.h>
#include <variant>
#include <Application.h>

#ifdef EDITOR
#include <Editor/Editor.h>
#endif


SpatialIndex::SpatialIndex() : octree_base(new Octree), props(SpatialIndexProperties())
{

}

Octree::Octree(Octree* parent,const BoundingBox& node_box, const std::vector<Entity>& entities, World& world) : child_nodes(), entity_list(), node_box(node_box), parent(parent)
{
	

}

void Octree::FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities)
{
	for (auto entity : entity_list) {
		bool has_bounding_box = world.HasComponent<BoundingVolumeComponent>(entity);
		auto variant = BoundingVolumeComponent::GetBoundingVolume(entity);
		if (!std::holds_alternative<NullBoundingVolume>(variant)) {
			TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
			std::visit([&](auto& bounding_vol) {
				if (bounding_vol.OverlapsFrustum(frustum, transform.TransformMatrix)) {
					entities.push_back(entity);
				}
				}, variant);
		}
		else {
			auto& transform = world.GetComponent<TransformComponent>(entity);
			if (OverlapPointFrustum(transform.translation, frustum)) {
				entities.push_back(entity);
			}
		}
	}
	
	if (!active) return;

	for (int index = 0; index < 8; index++) {
		if (!(active & (1 << index))) continue;
		if (!child_nodes[index].node_box.OverlapsFrustum(frustum, glm::mat4(1.0))) continue;
		child_nodes[index].FrustumCulling(world, frustum, entities);
	}

}

void Octree::BoxCulling(World& world, const OrientedBoundingBox& box, std::vector<Entity>& entities)
{
	for (auto entity : entity_list) {
		bool has_bounding_box = world.HasComponent<BoundingVolumeComponent>(entity);
		auto variant = BoundingVolumeComponent::GetBoundingVolume(entity);
		if (!std::holds_alternative<NullBoundingVolume>(variant)) {
			TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
			std::visit([&](auto& bounding_vol) {
				if (bounding_vol.OverlapsOrientedBox(box, transform.TransformMatrix)) {
					entities.push_back(entity);
				}
				}, variant);
		}
		else {
			auto& transform = world.GetComponent<TransformComponent>(entity);
			if (OverlapPointOrientedBox(transform.translation, box)) {
				entities.push_back(entity);
			}
		}
	}

	if (!active) return;

	for (int index = 0; index < 8; index++) {
		if (!(active & (1 << index))) continue;
		if (!child_nodes[index].node_box.OverlapsOrientedBox(box, glm::mat4(1.0))) continue;
		child_nodes[index].BoxCulling(world, box, entities);
	}
}

void Octree::SphereCulling(World& world, const BoundingSphere& sphere, std::vector<Entity>& entities)
{
	for (auto entity : entity_list) {
		bool has_bounding_box = world.HasComponent<BoundingVolumeComponent>(entity);
		auto variant = BoundingVolumeComponent::GetBoundingVolume(entity);
		if (!std::holds_alternative<NullBoundingVolume>(variant)) {
			TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
			std::visit([&](auto& bounding_vol) {
				if (bounding_vol.OverlapsSphere(sphere, transform.TransformMatrix)) {
					entities.push_back(entity);
				}
				}, variant);
		}
		else {
			auto& transform = world.GetComponent<TransformComponent>(entity);
			if (OverlapPointSphere(transform.translation, sphere)) {
				entities.push_back(entity);
			}
		}
	}

	if (!active) return;

	for (int index = 0; index < 8; index++) {
		if (!(active & (1 << index))) continue;
		if (!child_nodes[index].node_box.OverlapsSphere(sphere, glm::mat4(1.0))) continue;
		child_nodes[index].SphereCulling(world, sphere, entities);
	}
}

void Octree::RayCast(World& world, const Ray& ray, std::vector<RayCastResult>& hit_results)
{
	for (auto entity : entity_list) {
		bool has_bounding_box = world.HasComponent<BoundingVolumeComponent>(entity);
		auto variant = BoundingVolumeComponent::GetBoundingVolume(entity);
		if (!std::holds_alternative<NullBoundingVolume>(variant) && !std::holds_alternative<BoundingPointLightSphere>(variant) && !std::holds_alternative<BoundingInfinity>(variant)) {
			TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
			std::visit([&](auto& bounding_vol) {
				std::vector<glm::vec3> hits;
				if (bounding_vol.IntersectRay(ray, transform.TransformMatrix, hits)) {
					for (auto& hit : hits) {
						hit_results.push_back(RayCastResult{ entity, hit });
					}
				}
				}, variant);
		}
	}

	if (!active) return;

	for (int index = 0; index < 8; index++) {
		if (!(active & (1 << index))) continue;
		std::vector<glm::vec3> null_hit;
		if (!child_nodes[index].node_box.IntersectRay(ray, glm::mat4(1.0), null_hit)) continue;
		child_nodes[index].RayCast(world, ray, hit_results);
	}
}

void Octree::AddEntity(Entity ent)
{
	//IF there are no child nodes add entity into the current node
	
	SceneNode* node = Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(ent);
	if (node->ShouldSpatialIndexIgnore()) return;
	if (node->spatial_index_node) {
		throw std::runtime_error("Entity was already in the SpatialIndex");
	}

	if (!node) {
		throw std::runtime_error("Entity can't be added to spatial index if it isn't registered in the SceneGraph first.");
	}
	auto& props = Application::GetWorld().GetSpatialIndex().GetSpatialIndexProperties();
	if (!active) {
		if (entity_list.size() + 1 <= props.max_entities_pre_node || depth >= props.max_depth) {
			entity_list.push_back(ent);
			node->spatial_index_node = this;
			node->octree_index = entity_list.size() - 1;
			return;
		}
	}

	//Get the most appropriate child index
	char index;
	if (!ProcessEntity(Application::GetWorld(), index, ent)) {
		throw std::runtime_error("Entity could not be added to the spatial index, entity must have BoundingVolumeComponent and Transform component to be added to the spatial index");
	}

	//If the entity doesn't fit any child add the entity here
	if (index == -1) {
		entity_list.push_back(ent);
		node->spatial_index_node = this;
		node->octree_index = entity_list.size() - 1;
	}
	else { 
		//If the child is active call AddEntity on the child recursively
		if (active & ( 1 << index)) {
			child_nodes[index].AddEntity(ent);
		}
		//If the child isn't active add the entity to the current node.
		else {
			BoundingBox new_box(node_box.GetBoxSize() / glm::vec3(2.0), node_box.GetBoxOffset() + ((GetPosByIndex(index) * glm::vec3(2)) - glm::vec3(1)) * (node_box.GetBoxSize() / glm::vec3(4)));
			child_nodes[index].Init(this, new_box, { ent }, Application::GetWorld(), depth + 1);
			active |= (1 << index);
		}
	}

}

void Octree::RemoveEntity(Entity ent)
{
	SceneNode* node = Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(ent);
	if (!node) {
		throw std::runtime_error("Entity is not a part of the SceneGraph");
	}
	if (!node->spatial_index_node) {
		throw std::runtime_error("This entity was not properly assigned to the Spatial Index");
	}
	Octree* tree = node->spatial_index_node;
	if (tree->entity_list[node->octree_index] != node->entity) {
		throw std::runtime_error("This entity was not properly assigned to the Spatial Index");
	}
	Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(tree->entity_list.back())->octree_index = node->octree_index;
	std::swap(tree->entity_list[node->octree_index], tree->entity_list.back());
	tree->entity_list.pop_back();
	node->spatial_index_node = nullptr;
	node->octree_index = -1;

}

void Octree::Init(Octree* parent, const BoundingBox& node_box, const std::vector<Entity>& entities, World& world, int depth)
{
	this->depth = depth;
	this->parent = parent;
	this->node_box = node_box;

	//Create Empty Cells inactive cells
	child_nodes.reserve(8);
	for (int i = 0; i < 8; i++) {
		child_nodes.emplace_back();
	}

	entt::registry& registry = world.GetRegistry();
	std::array<std::vector<Entity>, 8> entities_lists;

	auto& props = world.GetSpatialIndex().GetSpatialIndexProperties();
	//if the number of nodes is smaller than minimum than place all entities into current node and stop subdividing
	if (entities.size() <= props.max_entities_pre_node || depth >= props.max_depth) {
		for (auto entity : entities) {
			SceneNode* node = world.GetSceneGraph()->GetSceneGraphNode(entity);
			if (node) {
				if (node->ShouldSpatialIndexIgnore()) continue;
				entity_list.push_back(entity);
				node->spatial_index_node = this;
				node->octree_index = entity_list.size() - 1;
			}
		}
		return;
	}

	//Either place each entity into a child node or into current node if it doesnt fit any child.
	for (Entity entity : entities) {
		ProcessEntity(world, entities_lists, entity);
	}

	//Each child node which was assigned entites will be made active and passed its entities into this constructor resursively.
	for (int index = 0; index < 8; index++) {
		if (entities_lists[index].empty()) continue;
		active |= 1 << index;
		BoundingBox new_box(node_box.GetBoxSize() / glm::vec3(2.0), node_box.GetBoxOffset() + ((GetPosByIndex(index) * glm::vec3(2)) - glm::vec3(1)) * (node_box.GetBoxSize() / glm::vec3(4)));
		child_nodes[index].Init(this, new_box, entities_lists[index], world, depth + 1);
	}
}

void Octree::VisualizeBoxes()
{
	auto ent = Application::GetWorld().GetPrimaryEntity();
	if (ent == Entity()) return;

	auto& cam = Application::GetWorld().GetComponent<CameraComponent>(ent);
	auto& trans = Application::GetWorld().GetComponent<TransformComponent>(ent);

	for (auto ent : entity_list) {
		auto bounding_var = BoundingVolumeComponent::GetBoundingVolume(ent);
		auto& transform = Application::GetWorld().GetComponent<TransformComponent>(ent);
		if (std::holds_alternative<BoundingBox>(bounding_var)) {
			BoundingBox box = std::get<BoundingBox>(bounding_var);
			Render_Box(box.GetAdjustedBox(transform.TransformMatrix), glm::mat4(1), cam, trans.TransformMatrix, PrimitivePolygonRenderMode::WIREFRAME, glm::vec3(1, 0, 0));
		}
		
	}

#ifdef EDITOR
	if (Editor::Get()->GetSelectedEntity() != Entity() && Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(Editor::Get()->GetSelectedEntity())->spatial_index_node == this) {
		Render_Box(node_box, glm::mat4(1), cam, trans.TransformMatrix, PrimitivePolygonRenderMode::WIREFRAME, glm::vec3(0.3f,1.0f,0.3f));
	}
	else {
		Render_Box(node_box, glm::mat4(1), cam, trans.TransformMatrix, PrimitivePolygonRenderMode::WIREFRAME);
	}
#else
	Render_Box(node_box, glm::mat4(1), cam, trans.TransformMatrix, PrimitivePolygonRenderMode::WIREFRAME);
#endif
	if (active) {
		for (int index = 0; index < 8; index++) {
			if (!(active & 1 << index)) continue;
			child_nodes[index].VisualizeBoxes();
		}
	}
}

Octree::Octree(Octree* parent,const BoundingBox& node_box, World& world) : child_nodes(), entity_list(), node_box(node_box), parent(parent)
{
	child_nodes.reserve(8);
	for (int i = 0; i < 8; i++) {
		child_nodes.emplace_back();
	}
	entt::registry& registry = world.GetRegistry();
	std::array<std::vector<Entity>, 8> entities_lists;
	auto view = registry.view<TransformComponent>();
	for (Entity entity : view) {
		ProcessEntity(world, entities_lists, entity);
	}

	for (int index = 0; index < 8; index++) {
		if (entities_lists[index].empty()) continue;
		active |= 1 << index;
		BoundingBox new_box(node_box.GetBoxSize() / glm::vec3(2.0), node_box.GetBoxOffset() + ((GetPosByIndex(index) * glm::vec3(2)) - glm::vec3(1)) * (node_box.GetBoxSize() / glm::vec3(4)));
		child_nodes[index].Init(this,new_box, entities_lists[index], world, 1);
	}
};

//Change plane coordinates hierarchically.

void Octree::ProcessEntity(World& world, std::array<std::vector<Entity>, 8>& list, Entity entity)
{
	
	char index = -1;
	SceneNode* node = world.GetSceneGraph()->GetSceneGraphNode(entity);
	if (node->ShouldSpatialIndexIgnore()) return;
	if (ProcessEntity(world, index, entity)) {
		// If the node borders multiple cells it doesnt fit into any child and will be assigned into the current cell
		if (index == -1) {
			entity_list.push_back(entity);
			node->spatial_index_node = this;
			node->octree_index = entity_list.size() - 1;
		}
		//Otherwise assign the entity to the appropriate child cell
		else {
			list[index].push_back(entity);
		}
	}
	else {
		//Throw if the entity could not be processed
		throw std::runtime_error("Entity could not be Processed by the SpatialIndex");
	}
}

bool Octree::ProcessEntity(World& world, char& index_out, Entity entity)
{
	//Entity needs to have a Transform and a bounding volume component to be assigned into the spatial index.
	if (world.HasComponent<TransformComponent>(entity)) {
		bool has_bounding_box = world.HasComponent<BoundingVolumeComponent>(entity);
		auto variant = BoundingVolumeComponent::GetBoundingVolume(entity);
		if (!std::holds_alternative<NullBoundingVolume>(variant)) {
			TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
			char index = 0;
			bool bordering = false;

			//Perform intersection test against splitting planes to figure out which node it belongs to 
			std::visit([&index, &transform, &bordering, this](auto& bounding_volume) {
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

				}, variant);

			//If the node borders multiple cells it doesnt fit into any child and will be assigned into the current cell
			if (bordering) {
				index_out = -1;
				return true;

			}
			//Otherwise assign the entity to the appropriate child cell
			else {
				index_out = index;
				return true;
			}
		}
		else {
			TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
			char index = 0;

			//Perform intersection test against splitting planes to figure out which node it belongs to 
			if (OverlapPointPlane(transform.translation, GetPlane(PlaneAxis::X))) {
				index |= GetIndexByPos(1, 0, 0);
			}

			if (OverlapPointPlane(transform.translation, GetPlane(PlaneAxis::Y))) {
				index |= GetIndexByPos(0, 1, 0);
			}


			if (OverlapPointPlane(transform.translation, GetPlane(PlaneAxis::Z))) {
				index |= GetIndexByPos(0, 0, 1);
			}

			//Otherwise assign the entity to the appropriate child cell
			index_out = index;
			return true;
			
		}
	}
	return false;
}

void SpatialIndex::Init(const SpatialIndexProperties& props)
{
	this->props = props;
	if (octree_base) {
		delete octree_base;
	}
	octree_base = new Octree(nullptr, props.world_box, Application::GetWorld());
}

void SpatialIndex::Rebuild()
{
	if (octree_base) {
		delete octree_base;
	}
	octree_base = new Octree(nullptr, props.world_box, Application::GetWorld());
}
