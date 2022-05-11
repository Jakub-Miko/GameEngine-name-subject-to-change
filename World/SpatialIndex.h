#pragma once 
#include "World.h"
#include "Entity.h"
#include <vector>
#include <array>
#include <Core/Geometry.h>
#include <Core/BoundingVolumes.h>

class Octree {
public:
	Octree(Octree* parent,const BoundingBox& node_box, const std::vector<Entity>& entities, World& world, int max_node_count = 1);
	Octree() = default;

	enum class PlaneAxis : unsigned char{
		X = 0, Y = 1, Z = 2
	};
	
	
	Plane GetPlane(PlaneAxis axis) {
		return planes[(char)axis];
	}

	static constexpr char GetIndexByPos(bool x, bool y, bool z) {
		return (x << 2) | (y << 1) | (z << 0);
	}

	static constexpr glm::vec3 GetPosByIndex(char index) {
		return glm::vec3((bool)(index & (1 << 2)), (bool)(index & (1 << 1)), (bool)(index & (1 << 0)));
	}

public:

	void VisualizeBoxes();

	Octree* parent = nullptr;
	BoundingBox node_box;
	Plane planes[3];
	std::vector<Entity> entity_list;
	std::vector<Octree> child_nodes;
	int max_node_count = 100;
	char active = 0;

	void FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities);


private:
	void Compute_Planes();
	friend class SpatialIndex;
	Octree(Octree* parent,const BoundingBox& node_box, World& world,int max_node_count = 1);

	void ProcessEntity(World& world, std::array<std::vector<Entity>, 8>& list, Entity entity);

};


class SpatialIndex {
public:
	SpatialIndex(World& world, const BoundingBox& world_box = BoundingBox({ 100,100,100 }));

	void Visualize() {
		octree_base.VisualizeBoxes();
	}

	void FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities) {
		octree_base.FrustumCulling(world, frustum, entities);
	}

	void Rebuild() {
		octree_base = Octree(nullptr,world_box, world);
	}

private:
	Octree octree_base;
	World& world;
	BoundingBox world_box;
};
