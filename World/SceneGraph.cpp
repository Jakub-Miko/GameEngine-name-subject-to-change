#include "SceneGraph.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <World/World.h>
#include <json.hpp>
#include <fstream>
#include <Application.h>
#include <glm/gtc/type_ptr.hpp>
#include <World/EntityManager.h>
#include <World/Components/SerializableComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Components/PrefabComponent.h>

static void SerializeNode(const SceneNode& node, nlohmann::json& base_object, World* world);
static void DeserializeNode(const nlohmann::json& json, SceneNode* parent, World* world);

void SceneGraph::clear()
{
	m_Nodes.clear();
	m_dirty_nodes.clear();
	root_calclulation_nodes.clear();

	SceneNode node;
	node.state = SceneNodeState(0);
	node.entity = Entity();
	node.first_child = nullptr;
	node.next = nullptr;
	node.previous = nullptr;
	node.parent = nullptr;

	root_node = node;
}

SceneNode* SceneGraph::AddEntity(Entity ent, SceneNode* parent)
{
	SceneNode* parent_node;
	if (!parent) {
		parent_node = &root_node;
	}
	else {
		parent_node = parent;
	}

	SceneNode node;
	node.first_child = nullptr;
	node.parent = parent_node;
	node.next = parent_node->first_child;
	node.previous = nullptr;
	node.state = SceneNodeState(0);
	node.entity = ent;

	if (m_world->HasComponent<PrefabComponent>(ent)) {
		node.state = node.state | SceneNodeState::PREFAB;
	}


	std::lock_guard<std::mutex> lock(addition_mutex);
	auto new_node = m_Nodes.insert(std::make_pair(ent.id, node));
	if (parent_node->first_child) {
		parent_node->first_child->previous = &((new_node.first)->second);
	}
	parent_node->first_child = &((new_node.first)->second);
	MarkEntityDirty(parent_node->first_child);
	return parent_node->first_child;
}

void SceneGraph::RemoveEntity(Entity entity)
{ 
	SceneNode* node = GetSceneGraphNode(entity);
	if (!node) {
		throw std::runtime_error("Trying to remove entity that doesn't exist in the SceneGraph");
	}
	if (node->next) {
		node->next->previous = node->previous;
	}
	if (node->previous) {
		node->previous->next = node->next;
	}
	PrefabComponent* prefab_comp;
	if (node->parent->IsPrefab() && (prefab_comp = &Application::GetWorld().GetComponentSync<PrefabComponent>(node->parent->entity))->first_child == node->entity) {
		if (node->next) {
			prefab_comp->first_child = node->next->entity;
		}
		else {
			prefab_comp->first_child = Entity();
		}
	}
	if (node->parent->first_child == node) {
		if (node->previous) {
			node->parent->first_child = node->previous;
		}
		else if (node->next) {
			node->parent->first_child = node->next;
		}
		else {
			node->parent->first_child = nullptr;
		}
	}
	 
	if (node->spatial_index_node) {
		m_world->GetSpatialIndex().RemoveEntity(entity);
	}

	std::lock_guard<std::mutex> lock(addition_mutex);
	auto fnd = m_Nodes.find(entity.id);
	if (fnd != m_Nodes.end()) {
		m_Nodes.erase(entity.id);
	}
	else {
		throw std::runtime_error("Trying to remove entity that doesn't exist in the SceneGraph");
	}

}

SceneNode* SceneGraph::AddEntityToPrefabRoot(Entity ent, SceneNode* parent_node)
{
	if (!parent_node) {
		throw std::runtime_error("In AddEntityToPrefabRoot a parent must be specified");
	}

	if (!m_world->HasComponent<PrefabComponent>(parent_node->entity)) {
		throw std::runtime_error("In AddEntityToPrefabRoot a parent must be a Prefab Root (must contain a prefab component).");
	}

	SceneNode* first_child_scenenode = nullptr;
	PrefabComponent& parent_prefab = m_world->GetComponent<PrefabComponent>(parent_node->entity);
	if (parent_prefab.first_child != Entity()) {
		first_child_scenenode = GetSceneGraphNode(parent_prefab.first_child);
		parent_prefab.first_child = ent;
	}
	else {
		parent_prefab.first_child = ent;
	}
	

	SceneNode node;
	node.first_child = nullptr;
	node.parent = parent_node;
	node.next = first_child_scenenode;
	node.previous = nullptr;
	node.state = SceneNodeState(0);
	node.entity = ent;

	if (m_world->HasComponent<PrefabComponent>(ent)) {
		node.state = node.state | SceneNodeState::PREFAB;
	}


	std::lock_guard<std::mutex> lock(addition_mutex);
	auto new_node = m_Nodes.insert(std::make_pair(ent.id, node));
	if (first_child_scenenode) {
		first_child_scenenode->previous = &((new_node.first)->second);
	}
	MarkEntityDirty(&((new_node.first)->second));
	return &((new_node.first)->second);
}

void SceneGraph::MarkEntityDirty(SceneNode* ent)
{
	std::lock_guard<std::mutex> lock(dirty_mutex);
	if (ent) {
		if (ent->IsDirty()) {
			ent->state = ent->state & ~(SceneNodeState::DIRTY_TRANSFORM);
			return;
		}
		else {
			ent->state = (ent->state | SceneNodeState::DIRTY) & ~(SceneNodeState::DIRTY_TRANSFORM);
			m_dirty_nodes.push_back(ent);
		}
	}
	else {
		throw std::runtime_error("Invalid SceneNode.");
	}
}

void SceneGraph::MarkEntityDirtyTransform(SceneNode* ent)
{
	std::lock_guard<std::mutex> lock(dirty_mutex);
	if (ent) {
		if (ent->IsDirty()) {
			ent->state = ent->state | SceneNodeState::DIRTY_TRANSFORM;
			return;
		}
		else {
			ent->state = ent->state | SceneNodeState::DIRTY | SceneNodeState::DIRTY_TRANSFORM;
			m_dirty_nodes.push_back(ent);
		}
	}
	else {
		throw std::runtime_error("Invalid SceneNode.");
	}
}

void SceneGraph::Serialize(nlohmann::json& output_json)
{
	using namespace nlohmann;
	output_json["SceneGraph"] = nlohmann::json();
	SceneNode* current_node = root_node.first_child;
	while (current_node) {
		SerializeNode(*current_node, output_json["SceneGraph"], m_world);
		current_node = current_node->next;
	}
	
}


static void SerializeNode(const SceneNode& node, nlohmann::json& base_object, World* world) {
	using namespace nlohmann;
	
	if (!world->HasComponent<SerializableComponent>(node.entity)) {
		return;
	}
	base_object.push_back(json::object());
	json& current_json_node = base_object.back();
	//Serialize here
	current_json_node["id"] = node.entity.id;
	current_json_node["state"] = (char)node.state;

	SceneNode* current_node = node.first_child;
	while (current_node) {
		SerializeNode(*current_node, current_json_node["children"], world);
		current_node = current_node->next;
	}

}


void SceneGraph::CalculateMatricies()
{
	root_calclulation_nodes.clear();
	for (auto node : m_dirty_nodes) {
		SceneNode* current = node->parent;
		while (current != &root_node) {
			if (current->IsDirty()) break;
			current = current->parent;
		}
		if (current == &root_node) {
			root_calclulation_nodes.push_back(node);
		} 
	}
	m_dirty_nodes.clear();

	for (auto root : root_calclulation_nodes) {
		RecalculateDownstream(root, root->parent);
	}

}

void SceneGraph::RecalculateDownstream(SceneNode* node, SceneNode* upstream)
{
	node->state = node->state & ~(SceneNodeState::DIRTY);
	TransformComponent& transform = m_world->GetRegistry().get<TransformComponent>((entt::entity)node->entity.id);
	glm::mat4 upstream_transform = glm::mat4(1.0f);

	if (!(bool)(node->state & SceneNodeState::DIRTY_TRANSFORM)) {
		if (upstream != &root_node) {
			upstream_transform = m_world->GetRegistry().get<TransformComponent>((entt::entity)upstream->entity.id).TransformMatrix;
		}
		transform.TransformMatrix = glm::translate(glm::mat4(1.0f), transform.translation) * glm::toMat4(transform.rotation) * glm::scale(glm::mat4(1.0f), transform.size);
	}
	else {
		node->state = node->state & ~(SceneNodeState::DIRTY_TRANSFORM);
	}


	if (!node->IsUIEntity()) {
		transform.TransformMatrix = upstream_transform * transform.TransformMatrix;

		//Update entry in spatial index
		if (node->spatial_index_node) {
			m_world->GetSpatialIndex().RemoveEntity(node->entity);
		}
		m_world->GetSpatialIndex().AddEntity(node->entity);
	}
	else {
		if (upstream->IsUIEntity()) {
			transform.TransformMatrix = upstream_transform * transform.TransformMatrix;
		}
	}

	SceneNode* children = node->first_child;
	while (children != nullptr) {
		RecalculateDownstream(children, node);
		children = children->next;
	}
	if (node->IsPrefab()) {
		Entity first_ent = m_world->GetComponent<PrefabComponent>(node->entity).first_child;
		SceneNode* prefab_child = GetSceneGraphNode(first_ent);
		while (prefab_child) {
			RecalculateDownstream(prefab_child, node);
			prefab_child = prefab_child->next;
		}
	}

}

SceneNode* SceneGraph::GetSceneGraphNode(Entity entity)
{
	std::lock_guard<std::mutex> lock(addition_mutex);
	auto fnd = m_Nodes.find(entity.id);
	if (fnd == m_Nodes.end()) {
		return nullptr;
	} 

	return &fnd->second;
}


void SceneGraph::Deserialize(const nlohmann::json& input_json)
{
	using namespace nlohmann;

	for (const json& current_json : input_json["SceneGraph"]) {
		DeserializeNode(current_json, &root_node, m_world);
	}

}

static void DeserializeNode(const nlohmann::json& json, SceneNode* parent, World* world) {
	Entity entity(json["id"].get<uint32_t>());
	
	SceneNode* node = world->GetSceneGraph()->AddEntity(entity, parent);
	world->SetComponent<SerializableComponent>(entity);
	if (world->HasComponent<PrefabComponent>(entity)) {
		world->SetComponent<ConstructionComponent>(entity, world->GetComponent<PrefabComponent>(entity).GetFilePath(), parent->entity);
	}

	if (json.contains("state")) {
		node->state = (SceneNodeState)json["state"].get<char>();
	}

	if (json.find("children") != json.end()) {
		for (const nlohmann::json& json_current : json["children"]) {
			DeserializeNode(json_current, node, world);
		}
	}


}

















