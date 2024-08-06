#pragma once
#include <stdint.h>
#include <entt/entt.hpp>
#include <Core/UnitConverter.h>

/**
 * @brief Specifies if the object will change in state during the game runtime (Currently not used)
 * @note May be used in the future for optimization purposes
*/
enum class EntityMode : unsigned char {
	STATIC = 0 << 0,
	DYNAMIC = 1 << 0,
	DEFAULT = STATIC
};

/**
 * @brief OR operator for EntityMode
*/
inline EntityMode operator|(const EntityMode& first, const EntityMode& second) {
	return static_cast<EntityMode>(static_cast<unsigned char>(first) | static_cast<unsigned char>(second));
}

/**
 * @brief AND operator for EntityMode
*/
inline EntityMode operator&(const EntityMode& first, const EntityMode& second) {
	return static_cast<EntityMode>(static_cast<unsigned char>(first) & static_cast<unsigned char>(second));
}

/**
 * @brief Additional entity properties stored in a TransformComponent
*/
struct EntityProperties {
	EntityProperties(EntityMode mode = EntityMode::DEFAULT) : mode(mode) {}
	EntityMode mode; ///< EntityMode assosiacted with the entity, currently not used.
};

JSON_SERIALIZABLE(EntityProperties, mode)

/**
 * @brief Identifies a unique object withing the World, is a wrapper around an entity id in the Worlds ECS(Entity Component System)
*/
class Entity {
public:
	friend class World;

	/**
	 * @brief Constructs a default invalid null entity
	*/
	Entity() : id(entt::null) {};

	/**
	 * @brief Copy constructor for an Entity
	*/
	Entity(const Entity& ref) : id(ref.id) {};

	/**
	 * @brief Assign operator for an Entity
	*/
	Entity& operator=(const Entity& ref) {
		id = ref.id;
		return *this;
	};

	/**
	 * @brief Create an Entity from an ECS entity id
	 * @param id id of an entity in the Worlds ECS
	*/
	Entity(uint32_t id) : id(id) {};

	/**
	 * @brief Create an Entity from an ECS entity(same as id just a typedef)
	 * @param id id of an entity in the Worlds ECS cast as entt::entity
	*/
	Entity(entt::entity id) : id((uint32_t)id) {};

	/**
	 * @brief EQUALS operator for an Entity
	 * @param other Entity to compare against
	*/
	bool operator==(const Entity& other) {
		return id == other.id;
	}
	
	/**
	 * @brief NOT EQUALS operator for an Entity
	 * @param other Entity to compare against
	*/
	bool operator!=(const Entity& other) {
		return id != other.id;
	}

public:
	uint32_t id; ///< Entity id representation in the Worlds ECS
};

JSON_SERIALIZABLE(Entity, id)