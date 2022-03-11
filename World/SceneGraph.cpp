#include "SceneGraph.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <World/World.h>
#include <json.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <World/Components/LoadedComponent.h>

static void SerializeNode(const SceneNode& node, nlohmann::json& base_object, World* world);

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

void SceneGraph::Serialize(const std::string& file_path)
{
	using namespace nlohmann;

	json base_object;
	SceneNode* current_node = root_node.first_child;
	while (current_node) {
		SerializeNode(*current_node, base_object["SceneGraph"], m_world);
		current_node = current_node->next;
	}
	
	std::ofstream file_stream(file_path);
	if (file_stream.is_open()) {
		file_stream << base_object;
		file_stream.close();
	}
	else {
		throw std::runtime_error("Couldn't open file: " + file_path);
	}

}

static void SerializeNode(const SceneNode& node, nlohmann::json& base_object, World* world) {
	using namespace nlohmann;
	base_object.push_back(json::object());
	json& current_json_node = base_object.back();
	//Serialize here
	current_json_node["id"] = node.entity.id;
	if (world->HasComponent<TransformComponent>(node.entity)) {
		TransformComponent& transform = world->GetComponent<TransformComponent>(node.entity);
		float* mat = glm::value_ptr(transform.TransformMatrix);
		for (int i = 0; i < 16; i++) {
			current_json_node["transform"].push_back(mat[i]);
		}

	}

	if (world->HasComponent<LoadedComponent>(node.entity)) {
		LoadedComponent& loaded = world->GetComponent<LoadedComponent>(node.entity);
		current_json_node["file_path"] = loaded.file_path;

	}

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
