#pragma once
#include <stdint.h>

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

public:
	uint32_t id;
};