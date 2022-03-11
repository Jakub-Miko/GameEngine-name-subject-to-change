#pragma once 
#include "World.h"
#include "Entity.h"
#include <vector>
#include <array>
#include <Core/Geometry.h>

class Octree {
public:
	Octree(const std::vector<Entity>& entities, World& world, int max_node_count = 100);

	enum class PlaneAxis : unsigned char{
		X = 0, Y = 1, Z = 2
	};
	
	
	static Plane& GetPlane(PlaneAxis axis) {
		switch (axis) {
		case PlaneAxis::X:
			return Plane(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f);
		case PlaneAxis::Y:
			return Plane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
		case PlaneAxis::Z:
			return Plane(glm::vec3(0.0f, 0.0f, 1.0f), 0.0f);
		default:
			return Plane(glm::vec3(0.0f, 0.0f, 0.0f), 0.0f);
		};
	}

	static constexpr char GetIndexByPos(bool x, bool y, bool z) {
		return (x << 2) & (y << 1) & (z << 0);
	}

public:
	std::vector<Entity> entity_list;
	std::vector<Octree> child_nodes;

	int max_node_count = 100;
	bool active = false;

private:
	friend class SpatialIndex;
	Octree(World& world,int max_node_count = 100);

	void ProcessEntity(World& world, std::array<std::vector<Entity>, 8>& list, Entity entity);

};


class SpatialIndex {
public:
	SpatialIndex(World& world);

private:
	Octree octree_base;

};