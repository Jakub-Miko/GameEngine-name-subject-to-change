#include "SceneGraph.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <World/World.h>
#include <json.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <World/EntityManager.h>
#include <World/Components/LoadedComponent.h>

static void SerializeNode(const SceneNode& node, nlohmann::json& base_object, World* world);
static void DeserializeNode(nlohmann::json& json, SceneNode* parent, World* world);

void SceneGraph::clear()
{
	m_Nodes.clear();
	m_dirty_nodes.clear();
	root_calclulation_nodes.clear();
	deserialize = false;

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
		throw std::runtime_error("File could not be opened: " + file_path);
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
		float* translation = glm::value_ptr(transform.translation);
		float* scale = glm::value_ptr(transform.size);
		float* rotation = glm::value_ptr(transform.rotation);
		for (int i = 0; i < 3; i++) {
			current_json_node["transform"]["translation"].push_back(translation[i]);
			current_json_node["transform"]["scale"].push_back(scale[i]);
		}
		for (int i = 0; i < 4; i++) {
			current_json_node["transform"]["rotation"].push_back(rotation[i]);
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

void SceneGraph::DeserializeSystem()
{
	using namespace nlohmann;
	if (!deserialize) return;
	
	std::ifstream file(deserialize_path);
	if (!file.is_open()) {
		throw std::runtime_error("File could not be opened: " + deserialize_path);
	}
	json base_json;
	base_json << file;

	for (json& current_json : base_json["SceneGraph"]) {
		DeserializeNode(current_json, &root_node, m_world);
	}

	deserialize = false;

}


void SceneGraph::Deserialize(const std::string& file_path)
{
	deserialize_path = file_path;
	deserialize = true;

}

static void DeserializeNode(nlohmann::json& json, SceneNode* parent, World* world) {
	Entity entity;
	if(json.find("transform") == json.end() || json["transform"].size() != 3) {
		// Failed
		return;
	}

	float translation[3];
	float scale[3];
	float rotation[4];

	for (int i = 0; i < 3; i++) {
		translation[i] = json["transform"]["translation"][i];
		scale[i] = json["transform"]["scale"][i];
	}
	for (int i = 0; i < 4; i++) {
		rotation[i] = json["transform"]["rotation"][i];
	}


	if (json.find("file_path") != json.end()) {
		entity = EntityManager::Get()->CreateEntityInplace(json["file_path"].get<std::string>() , parent->entity);
	}
	else {
		entity = world->CreateEntity(parent->entity); 
	}

	TransformComponent& transform = world->GetComponent<TransformComponent>(entity);
	world->SetEntityRotation(entity, glm::make_quat(rotation));
	world->SetEntityScale(entity, glm::make_vec3(scale));
	world->SetEntityTranslation(entity, glm::make_vec3(translation));

	if (json.find("children") != json.end()) {
		for (nlohmann::json& json_current : json["children"]) {
			DeserializeNode(json_current, transform.scene_node, world);
		}
	}


}

















