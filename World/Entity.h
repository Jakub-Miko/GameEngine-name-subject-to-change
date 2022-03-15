#pragma once
#include <stdint.h>
#include <entt/entt.hpp>

enum class EntityMode : unsigned char {
	DEFAULT = 0, STATIC = 1, DYNAMIC = 2
};

struct EntityProperties {
	EntityProperties(EntityMode mode = EntityMode::DEFAULT) : mode(mode) {}
	EntityMode mode;
};

class Entity {
public:
	friend class World;

	Entity() : id(0) {};
	Entity(const Entity& ref) : id(ref.id) {};
	Entity& operator=(const Entity& ref) {
		id = ref.id;
		return *this;
	};

	Entity(uint32_t id) : id(id) {};
	Entity(entt::entity id) : id((uint32_t)id) {};

public:
	uint32_t id;
};