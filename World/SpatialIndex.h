#pragma once 
#include "Entity.h"
#include <vector>
#include <array>
#include <Core/Geometry.h>
#include <Core/BoundingVolumes.h>

class World;

struct SpatialIndexProperties {
	BoundingBox world_box = BoundingBox(glm::vec3(100, 100, 100), glm::vec3(0, 0, 0));
	int max_entities_pre_node = 1;
	int max_depth = 20;
};

class Octree {
public:
	Octree(Octree* parent,const BoundingBox& node_box, const std::vector<Entity>& entities, World& world);
	Octree() = default;

	enum class PlaneAxis : unsigned char{
		X = 0, Y = 1, Z = 2
	};
	
	
	Plane GetPlane(PlaneAxis axis) {
		switch (axis) {
		case PlaneAxis::X:
			return Plane(glm::vec3(1.0f, 0.0f, 0.0f), node_box.GetBoxOffset().x);
		case PlaneAxis::Y:
			return Plane(glm::vec3(0.0f, 1.0f, 0.0f), node_box.GetBoxOffset().y);
		case PlaneAxis::Z:
			return Plane(glm::vec3(0.0f, 0.0f, 1.0f), node_box.GetBoxOffset().z);
		}
	}

	static constexpr char GetIndexByPos(bool x, bool y, bool z) {
		return (x << 2) | (y << 1) | (z << 0);
	}

	static constexpr glm::vec3 GetPosByIndex(char index) {
		return glm::vec3((bool)(index & (1 << 2)), (bool)(index & (1 << 1)), (bool)(index & (1 << 0)));
	}

public:

	void Init(Octree* parent, const BoundingBox& node_box, const std::vector<Entity>& entities, World& world, int depth = 0);

	void VisualizeBoxes();  

	Octree* parent = nullptr;
	BoundingBox node_box;
	std::vector<Entity> entity_list;
	std::vector<Octree> child_nodes; 
	int depth = 0;
	char active = 0;

	void FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities);

	void BoxCulling(World& world, const OrientedBoundingBox& box, std::vector<Entity>& entities);

	void AddEntity(Entity ent);

	void RemoveEntity(Entity ent);

private:
	friend class SpatialIndex;
	Octree(Octree* parent,const BoundingBox& node_box, World& world);

	void ProcessEntity(World& world, std::array<std::vector<Entity>, 8>& list, Entity entity);
	bool ProcessEntity(World& world, char& index, Entity entity);

};


class SpatialIndex {
public:
	SpatialIndex();
	~SpatialIndex() {
		if (octree_base) {
			delete octree_base;
		}
	}

	void Init(const SpatialIndexProperties& props);

	void Visualize() {
		octree_base->VisualizeBoxes();
	}

	void FrustumCulling(World& world, const Frustum& frustum, std::vector<Entity>& entities) {
		octree_base->FrustumCulling(world, frustum, entities);
	}


	void BoxCulling(World& world, const OrientedBoundingBox& box, std::vector<Entity>& entities) {
		octree_base->BoxCulling(world, box, entities);
	}

	void AddEntity(Entity ent) {
		octree_base->AddEntity(ent);
	}

	void RemoveEntity(Entity ent) {
		octree_base->RemoveEntity(ent);
	}

	void Rebuild();

	const SpatialIndexProperties& GetSpatialIndexProperties() const {
		return props;
	}

private:
	Octree* octree_base = nullptr;
	SpatialIndexProperties props;
};
