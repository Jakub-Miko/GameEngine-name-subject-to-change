#pragma once
#include <stdint.h>
#include <entt/entt.hpp>

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