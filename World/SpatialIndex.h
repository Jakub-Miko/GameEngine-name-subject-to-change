#pragma once 
#include "Entity.h"
#include <vector>
#include <array>
#include <Core/Geometry.h>
#include <Core/BoundingVolumes.h>

class World;

/**
 * @brief Specifies properties and constraints of the SpatialIndex implementation(Octree)
*/
struct SpatialIndexProperties {
	BoundingBox world_box = BoundingBox(glm::vec3(10000, 10000, 10000), glm::vec3(0, 0, 0)); ///< the size of a box which should encompass the entire world, should be set higher just to be safe
	int max_entities_per_node = 1; ///< how many entities need to be in and octree cell for it to be split, unless its already at its max_depth
	int max_depth = 20; ///< how many level deep can cell be in the octree hierarchy, after this limit cells no longer split
};

/**
 * @brief Main recursive structure implementing the Spatial index
 * 
 * It is a 3D box that can be split into 8 uniform child boxes represented as its children in a tree structure.
 * It behaves as a binary tree for lookups in 3D space.
 * This structure represents its own nodes since its recursive.
*/
class Octree {
public:
	/**
	 * @brief Create an Octree cell which splits entities amongs its children to satisfy SpatialIndexProperties
	 * @param parent a parent octree cell
	 * @param node_box bounding box of this cell
	 * @param entities entities to distribute among itself and its children
	 * @param world reference to the world from which the Entities originate
	 * @warning For now its better to use the default constructor and the Octree::Init method
	 * @todo this constructor doesn't do what it seems to do, check Octree::Init, probably should be removed
	*/
	Octree(Octree* parent,const BoundingBox& node_box, const std::vector<Entity>& entities, World& world);
	Octree() = default;

	/**
	 * @brief Represents the axis orthogonal to the specified plane
	*/
	enum class PlaneAxis : unsigned char{
		X = 0, Y = 1, Z = 2
	};
	
	/**
	 * @brief Return a Plane which represents a wall which splits this octree cell along a specified axis
	 * @param axis axis for the plane to split
	 * @return Plane splitting the bounding box of this cell
	*/
	Plane GetPlane(PlaneAxis axis) {
		switch (axis) {
		case PlaneAxis::X:
			return Plane(glm::vec3(1.0f, 0.0f, 0.0f), node_box.GetBoxOffset().x);
		case PlaneAxis::Y:
			return Plane(glm::vec3(0.0f, 1.0f, 0.0f), node_box.GetBoxOffset().y);
		case PlaneAxis::Z:
			return Plane(glm::vec3(0.0f, 0.0f, 1.0f), node_box.GetBoxOffset().z);
		}
		return {glm::vec3(1.0f, 0.0f, 0.0f), 0};
	}

	/**
	 * @brief Transform the relative child cell position to an index
	 * @param x child cell position in the x direction
	 * @param y child cell position in the y direction
	 * @param z child cell position in the z direction
	 * @return index representing the child cell position
	 * @see Octree::GetPosByIndex
	*/
	static constexpr char GetIndexByPos(bool x, bool y, bool z) {
		return (x << 2) | (y << 1) | (z << 0);
	}

	/**
	 * @brief Gets a vector representing the child cell position from a index, reverse of Octree::GetIndexByPos
	 * @param index index to get the child cell position from
	 * @return a relative vector specifying the offset of a child cell from the current cell
	 * @see Octree::GetIndexByPos
	*/
	static constexpr glm::vec3 GetPosByIndex(char index) {
		return glm::vec3((bool)(index & (1 << 2)), (bool)(index & (1 << 1)), (bool)(index & (1 << 0)));
	}

public:

	/**
	 * @brief Initializes an Octree cell which splits entities amongs its children to satisfy SpatialIndexProperties
	 * @param parent a parent octree cell
	 * @param node_box bounding box of this cell
	 * @param entities entities to distribute among itself and its children
	 * @param world reference to the world from which the Entities originate
	 * @param depth the depth of this cell from the root of the octree
	*/
	void Init(Octree* parent, const BoundingBox& node_box, const std::vector<Entity>& entities, World& world, int depth = 0);

	/**
	 * @brief Renders the wireframe of the SpatialIndex boxes for visualization
	*/
	void VisualizeBoxes();  

	Octree* parent = nullptr; ///< the parent Octree cell
	BoundingBox node_box; ///< the box representing this cell
	std::vector<Entity> entity_list; ///< entities within this cell
	std::vector<Octree> child_nodes; ///< child cells
	int depth = 0; ///< depth from root
	char active = 0; ///< bitmask representing which of the 8 children is utilized

	/**
	 * @brief Execute a frustum culling algorithm, which searches for entities whose mesh is inside the specified frustum
	 * @param world World in which entities originate 
	 * @param frustum Frustum which should containg all the returned objects
	 * @param entities output vector which will be filled with entities withing the frustum 
	*/
	void FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities);

	/**
	 * @brief Execute a box culling algorithm, which searches for entities whose mesh is inside the specified box
	 * @param world World in which entities originate
	 * @param box Box which should containg all the returned objects
	 * @param entities output vector which will be filled with entities withing the box
	*/
	void BoxCulling(World& world, const OrientedBoundingBox& box, std::vector<Entity>& entities);

	/**
	 * @brief Execute a sphere culling algorithm, which searches for entities whose mesh is inside the specified Sphere
	 * @param world World in which entities originate
	 * @param sphere Sphere which should containg all the returned objects
	 * @param entities output vector which will be filled with entities withing the sphere
	*/
	void SphereCulling(World& world, const BoundingSphere& sphere, std::vector<Entity>& entities);

	/**
	 * @brief Executes a ray-cast algorithm which return all objects whose mesh intersects the specified Ray
	 * @param world World in which entities originate
	 * @param ray Ray with which objects are intersected
	 * @param hit_results output vector which will be filled with entities intersecting the ray
	*/
	void RayCast(World& world, const Ray& ray, std::vector<RayCastResult>& hit_results);

	/**
	 * @brief Add a new entity to this Octree cell or its children
	 * @param ent Entity that will be fitted to the smallest unoccupied child cell or this cell itself
	*/
	void AddEntity(Entity ent);

	/**
	 * @brief Removes the entity from the cell
	 * @param ent Entity to remove
	*/
	void RemoveEntity(Entity ent);

private:
	friend class SpatialIndex;
	
	/**
	 * @brief Internal constructor used by cells to create its children
	 * @param parent parent cell
	 * @param node_box bounding box of this cell
	 * @param world reference to the world from which the Entities originate
	*/
	Octree(Octree* parent,const BoundingBox& node_box, World& world);

	/**
	 * @brief Internal method used by the cell to split entities among it self and its children
	 * @param world reference to the world from which the Entities originate
	 * @param list 8 vectors containing entities belonging to 8 respective child cells ,this function will either add the entity to its own list or to one of there vectors.
	 * @param entity entity to place in either itself or one of the eight child cells
	*/
	void ProcessEntity(World& world, std::array<std::vector<Entity>, 8>& list, Entity entity);

	/**
	 * @brief Internal method used by the cell to split entities among it self and its children
	 * @param world reference to the world from which the Entities originate
	 * @param index represents the index of the cell the entity belongs to, or -1(255) when it can't fit in any of them(as such it belongs to their parent)
	 * @param entity entity to place in either itself or one of the eight child cells
	*/
	bool ProcessEntity(World& world, char& index, Entity entity);

};

/**
 * @brief The to level Spatial index class, responsible for accelerated spatial lookup algorithms such as Frustum culling and RayCasting
 * 
 * @note This is just a wrapper over the recursive class Octree
*/
class SpatialIndex {
public:
	SpatialIndex();
	~SpatialIndex() {
		if (octree_base) {
			delete octree_base;
		}
	}

	void Init(const SpatialIndexProperties& props);

	/**
	 * @brief calls Octree::VisualizeBoxes
	*/
	void Visualize() {
		octree_base->VisualizeBoxes();
	}

	/**
	 * @brief calls Octree::FrustumCulling
	*/
	void FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities) {
		octree_base->FrustumCulling(world, frustum, entities);
	}

	/**
	 * @brief calls Octree::RayCast
	*/
	void RayCast(World& world, const Ray& ray, std::vector<RayCastResult>& hit_results) {
		octree_base->RayCast(world, ray, hit_results);
	}

	/**
	 * @brief calls Octree::BoxCulling
	*/
	void BoxCulling(World& world, const OrientedBoundingBox& box, std::vector<Entity>& entities) {
		octree_base->BoxCulling(world, box, entities);
	}

	/**
	 * @brief calls Octree::SphereCulling
	*/
	void SphereCulling(World& world, const BoundingSphere& sphere, std::vector<Entity>& entities) {
		octree_base->SphereCulling(world, sphere, entities);
	}

	/**
	 * @brief calls Octree::AddEntity
	*/
	void AddEntity(Entity ent) {
		octree_base->AddEntity(ent);
	}

	/**
	 * @brief calls Octree::RemoveEntity
	*/
	void RemoveEntity(Entity ent) {
		octree_base->RemoveEntity(ent);
	}

	/**
	 * @brief Rebuild the entire octree structure
	*/
	void Rebuild();

	/**
	 * @brief Get SpatialIndexProperties of the SpatialIndex
	*/
	const SpatialIndexProperties& GetSpatialIndexProperties() const {
		return props;
	}

private:
	Octree* octree_base = nullptr; ///< the root Octree cell
	SpatialIndexProperties props; ///< the SpatialIndexProperties of the SpatialIndex
};
