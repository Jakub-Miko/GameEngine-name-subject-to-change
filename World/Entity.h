#pragma once
#include <cstddef>

class Entity {
public:
	friend class World;
	uint32_t id;
	Entity() : id(0) {};
	Entity(const Entity& ref) : id(ref.id) {};
	Entity& operator=(const Entity& ref) {
		id = ref.id;
		return *this;
	};

private:
	Entity(uint32_t id) : id(id) {};
};