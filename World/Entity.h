#pragma once
#include <stdint.h>
#include <entt/entt.hpp>
#include <Core/UnitConverter.h>

enum class EntityMode : unsigned char {
	STATIC = 0 << 0,
	DYNAMIC = 1 << 0,
	DEFAULT = STATIC
};

inline EntityMode operator|(const EntityMode& first, const EntityMode& second) {
	return static_cast<EntityMode>(static_cast<unsigned char>(first) | static_cast<unsigned char>(second));
}

inline EntityMode operator&(const EntityMode& first, const EntityMode& second) {
	return static_cast<EntityMode>(static_cast<unsigned char>(first) & static_cast<unsigned char>(second));
}


struct EntityProperties {
	EntityProperties(EntityMode mode = EntityMode::DEFAULT) : mode(mode) {}
	EntityMode mode;
};

JSON_SERIALIZABLE(EntityProperties, mode)

class Entity {
public:
	friend class World;

	Entity() : id(entt::null) {};
	Entity(const Entity& ref) : id(ref.id) {};
	Entity& operator=(const Entity& ref) {
		id = ref.id;
		return *this;
	};

	Entity(uint32_t id) : id(id) {};
	Entity(entt::entity id) : id((uint32_t)id) {};

	bool operator==(const Entity& other) {
		return id == other.id;
	}
	
	bool operator!=(const Entity& other) {
		return id != other.id;
	}

public:
	uint32_t id;
};

JSON_SERIALIZABLE(Entity, id)