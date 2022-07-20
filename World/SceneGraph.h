#pragma once
#include <World/Entity.h>
#include <vector>
#include <json.hpp>
#include <unordered_map>
#include <mutex>


class World;
class Octree;

enum class SceneNodeState : char {
	DIRTY = 1, PREFAB = 2
};

inline SceneNodeState operator|(const SceneNodeState& first, const SceneNodeState& second) {
	return SceneNodeState((char)first | (char)second);
}

inline SceneNodeState operator&(const SceneNodeState& first, const SceneNodeState& second) {
	return SceneNodeState((char)first & (char)second);
}

inline SceneNodeState operator~(const SceneNodeState& first) {
	return SceneNodeState(~(char)first);
}

struct SceneNode {

	Entity entity;
	SceneNode* parent = nullptr;
	SceneNode* first_child = nullptr;
	SceneNode* next = nullptr;
	SceneNode* previous = nullptr;
	Octree* spatial_index_node = nullptr;
	uint32_t octree_index = -1;
	SceneNodeState state; 
	bool IsDirty() const {
		return (char)state & (char)SceneNodeState::DIRTY;
	}
	bool IsPrefab() const {
		return (char)state & (char)SceneNodeState::PREFAB;
	}

};


class SceneGraph {
public:
	friend class World;

public:
	SceneGraph(World* world) : m_world(world), m_Nodes(), dirty_mutex(), m_dirty_nodes(), root_calclulation_nodes(), addition_mutex()
	{
		SceneNode node;
		node.state = SceneNodeState(0);
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

	void RemoveEntity(Entity entity);

	//USed internally to diferentiate between preab children and scenegraph children
	SceneNode* AddEntityToPrefabRoot(Entity ent, SceneNode* parent = nullptr);

	void MarkEntityDirty(SceneNode* ent);

	const SceneNode* GetRootNode() const {
		return &root_node;
	}

	void CalculateMatricies();

	void RecalculateDownstream(SceneNode* node, SceneNode* upstream);

	SceneNode* GetSceneGraphNode(Entity entity);

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
