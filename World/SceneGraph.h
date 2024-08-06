#pragma once
#include <World/Entity.h>
#include <vector>
#include <json.hpp>
#include <unordered_map>
#include <mutex>


class World;
class Octree;

/**
 * @brief a Bitset containing flags signifing the state of an Entity with repect to its World
 * 
 * This state contains relevant information which allows the SceneGraph to handle its transformation properly
*/
enum class SceneNodeState : char {
	DIRTY = 1, ///< Local transformation (translation, rotation, scale) has been changed and world tranform needs to be recalculated
	PREFAB = 2, ///< The entity is a Prefab, and possibly has two seperate child lists (scene and prefab children)
	DIRTY_TRANSFORM = 4, ///< The world transform has been manipulated directly and should not be recomputed, but its children should be still updated
	PREFAB_CHILD = 8, ///< The entity is in a hierarchy root of which is a prafab not the scene root
	UI_ENTITY = 16 ///< The entity is a 2D UI element and should not be affected by parent 3D transforms
};

/**
 * @brief OR operator for SceneNodeState
*/
inline SceneNodeState operator|(const SceneNodeState& first, const SceneNodeState& second) {
	return SceneNodeState((char)first | (char)second);
}

/**
 * @brief AND operator for SceneNodeState
*/
inline SceneNodeState operator&(const SceneNodeState& first, const SceneNodeState& second) {
	return SceneNodeState((char)first & (char)second);
}

/**
 * @brief NOT operator for SceneNodeState
*/
inline SceneNodeState operator~(const SceneNodeState& first) {
	return SceneNodeState(~(char)first);
}

/**
 * @brief Entity representation within the scene hierarchy
*/
struct SceneNode {

	Entity entity; ///< Entity this Node referes to 
	SceneNode* parent = nullptr; ///< Parent of the entity in the scene hierarchy
	SceneNode* first_child = nullptr; ///< the head of the scene children list
	SceneNode* next = nullptr; ///< next neighbour node sharing the same parent
	SceneNode* previous = nullptr; ///< previous neighbour node sharing the same parent
	Octree* spatial_index_node = nullptr; ///< Octree node this Entity is in (Allows for fast removal without spatial lookup)
	uint32_t octree_index = -1; ///< index in the list of the octree node 
	SceneNodeState state = (SceneNodeState)0; ///< The state of the entity defined by SceneNodeState
	
	/**
	 * @brief Checks if if the entity world transform and its children world transforms should be recalculated
	*/
	bool IsDirty() const {
		return (char)state & (char)SceneNodeState::DIRTY;
	}

	/**
	 * @brief Checks if the Entity is a Prefab root with a separate Prefab hierarchy
	*/
	bool IsPrefab() const {
		return (char)state & (char)SceneNodeState::PREFAB;
	}

	/**
	 * @brief Checks if en entity is a 2D UI element and should not be influenced by 3D parent transforms 
	 * @todo create a seperate hierarchy for ui elements similar to prefabs
	*/
	bool IsUIEntity() const {
		return (char)state & (char)SceneNodeState::UI_ENTITY;
	}

	/**
	 * @brief Checks if the entity should not be included in a spatial index
	*/
	bool ShouldSpatialIndexIgnore() const {
		return IsUIEntity();
	}

};

/**
 * @brief Stores the Worlds scene hierarchy and handles updating world trasnforms and updating entities positions in the SpatialIndex
*/
class SceneGraph {
public:
	friend class World;

public:
	/**
	 * @brief Created an empty SceneGraph associated with the world
	 * @param world World to associate with
	*/
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

	/**
	 * @brief Clears the SceneGraphs into an initial state 
	 * 
	 * Used for example when closing a Scene
	*/
	void clear();

	/**
	 * @brief Add an entity into the Scene hierarchy and also places it into the SpatialIndex if needed
	 * @param ent Entity add
	 * @param parent Parent of the entity to place the new entity under
	 * @return new Instance of the SceneNode representing the entities placement in the SceneGraph
	*/
	SceneNode* AddEntity(Entity ent, SceneNode* parent = nullptr);

	/**
	 * @brief Removes the entity from the SceneGraph and from SpatialIndex if needed
	 * @param entity Entity to remove
	*/
	void RemoveEntity(Entity entity);

	/**
	 * @brief Used to place an Entity into the SceneGraph not under the parent as a Scene child, but as a prefab child
	 * @param ent Entity to place into a Prefav
	 * @param parent Prefab root to place the entity under
	 * @return new Instance of the SceneNode representing the entities placement in the SceneGraph
	 * @warning parent must be associated with an Entity which has a PrefabComponent
	*/
	SceneNode* AddEntityToPrefabRoot(Entity ent, SceneNode* parent = nullptr);

	/**
	 * @brief Mark an entity dirty when its local tranformations(translation, rotation, scale) have been updated
	 * @param ent Entity node to mark dirty
	*/
	void MarkEntityDirty(SceneNode* ent);

	/**
	 * @brief Mark an entity dirty when its world tranform has been updated directly
	 * @param ent Entity node to mark dirty
	*/
	void MarkEntityDirtyTransform(SceneNode* ent);

	/**
	 * @brief Gets the SceneGraph root node
	 * @return 
	*/
	const SceneNode* GetRootNode() const {
		return &root_node;
	}

	/**
	 * @brief Recualculated all world space matricies in the Scene
	*/
	void CalculateMatricies();

	/**
	 * @brief Rercursive function used to traverse the Scenegraph to update all entities world transforms
	 * @param node node to process
	 * @param upstream parent world tranformation which needs to be applied to the current node local tranform, unless the node is marked as SceneNodeState::DIRTY_TRANSFORM
	*/
	void RecalculateDownstream(SceneNode* node, SceneNode* upstream);

	/**
	 * @brief Gets the SceneNode associated with an entity
	 * @param entity entity to get the SceneNode of
	 * @return SceneNode pointer to the node associated with the entity, if none exists returns nullptr
	*/
	SceneNode* GetSceneGraphNode(Entity entity);

private:
	friend class GameLayer;
	
	/**
	 * @brief Serializes the Scenegraph into a JSON file as a part of the Scene serialization process
	 * @param output_json json object containing the serialized SceneGraph
	*/
	void Serialize(nlohmann::json& output_json);

	/**
	 * @brief Loads the SceneGraph from a json object as a part of Loading a Scene
	 * @param input_json json objects that represents the SceneGraph
	 * @warning This method assumes that the entity ids located in the json are already valid, and needs to be called after all entities have beed deserialized
	*/
	void Deserialize(const nlohmann::json& input_json);

	std::unordered_map<uint32_t, SceneNode> m_Nodes; ///<Map that stores all SceneNode associated with entities
	SceneNode root_node; ///< The root SceneNode
	World* m_world; ///< World to which Entities in this SceneGraph belong
	std::mutex dirty_mutex; ///< Mutex for synchronizing access to the dirty list
	std::mutex addition_mutex; ///< Mutex for synchronizing access to the SceneNode map
	std::vector<SceneNode*> m_dirty_nodes; ///< Vector that stores all dirty nodes which need an update
	std::vector<SceneNode*> root_calclulation_nodes; ///< Used for ensuring hierarchies dont get updated twice when a child of a dirty entity is also marked dirty
};  
