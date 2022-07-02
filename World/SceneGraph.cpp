#include "SceneGraph.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <World/World.h>
#include <json.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <World/EntityManager.h>
#include <World/Components/SerializableComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Components/LoadedComponent.h>

static void SerializeNode(const SceneNode& node, nlohmann::json& base_object, World* world);
static void DeserializeNode(const nlohmann::json& json, SceneNode* parent, World* world);

void SceneGraph::clear()
{
	m_Nodes.clear();
	m_dirty_nodes.clear();
	root_calclulation_nodes.clear();

	SceneNode node;
	node.dirty = false;
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
	node.dirty = false;
	node.entity = ent;

	if (m_world->HasComponent<LoadedComponent>(ent)) {
		node.prefab = true;
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

void SceneGraph::MarkEntityDirty(SceneNode* ent)
{
	if (ent) {
		if (ent->dirty) {
			return;
		}
		else {
			std::lock_guard<std::mutex> lock(dirty_mutex);
			ent->dirty = true;
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


	//Prefab hierarchy is defined in its file not the scene file
	if (node.prefab) {
		return;
	}

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
			if (current->dirty) break;
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
	node->dirty = false;
	TransformComponent& transform = m_world->GetRegistry().get<TransformComponent>((entt::entity)node->entity.id);
	glm::mat4 upstream_transform = glm::mat4(1.0f);
	if (upstream != &root_node) {
		upstream_transform = m_world->GetRegistry().get<TransformComponent>((entt::entity)upstream->entity.id).TransformMatrix;
	}

	transform.TransformMatrix = glm::translate(glm::mat4(1.0f), transform.translation) * glm::toMat4(transform.rotation) * glm::scale(glm::mat4(1.0f), transform.size);

	transform.TransformMatrix = upstream_transform * transform.TransformMatrix;

	SceneNode* children = node->first_child;
	while (children != nullptr) {
		RecalculateDownstream(children, node);
		children = children->next;
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
	if (world->HasComponent<LoadedComponent>(entity)) {
		world->SetComponent<ConstructionComponent>(entity, world->GetComponent<LoadedComponent>(entity).file_path, parent->entity);
	}

	if (json.find("children") != json.end()) {
		for (const nlohmann::json& json_current : json["children"]) {
			DeserializeNode(json_current, node, world);
		}
	}


}

















