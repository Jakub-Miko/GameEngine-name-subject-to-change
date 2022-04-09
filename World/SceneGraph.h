#pragma once
#include <World/Entity.h>
#include <vector>
#include <json.hpp>
#include <unordered_map>
#include <mutex>

class World;

struct SceneNode {

	Entity entity;
	SceneNode* parent = nullptr;
	SceneNode* first_child = nullptr;
	SceneNode* next = nullptr;
	SceneNode* previous = nullptr;
	bool dirty = false;
	bool prefab = false;

};


class SceneGraph {
public:
	friend class World;

public:
	SceneGraph(World* world) : m_world(world), m_Nodes(), dirty_mutex(), m_dirty_nodes(), root_calclulation_nodes(), addition_mutex()
	{
		SceneNode node;
		node.dirty = false;
		node.entity = Entity();
		node.first_child = nullptr;
		node.next = nullptr;
		node.previous = nullptr;
		node.parent = nullptr;

		root_node = node;
	}

	~SceneGraph() {}

	void clear();

	SceneNode* AddEntity(Entity ent, SceneNode* parent = nullptr);

	void MarkEntityDirty(SceneNode* ent);


	void CalculateMatricies();

	void RecalculateDownstream(SceneNode* node, SceneNode* upstream);

private:
	friend class GameLayer;
	
	void Serialize(nlohmann::json& output_json);
	void Deserialize(const nlohmann::json& input_json);

	std::unordered_map<uint32_t, SceneNode> m_Nodes;
	SceneNode root_node;
	World* m_world;
	std::mutex dirty_mutex;
	std::mutex addition_mutex;
	std::vector<SceneNode*> m_dirty_nodes;
	std::vector<SceneNode*> root_calclulation_nodes;
};  
