#pragma once
#include <string>
#include <vector>
#include <World/Components/DynamicPropertiesComponent.h>
#include <unordered_map>

/**
 * @brief The result of parsing a an entity from a file
*/
struct EntityParseResult {
	std::string component_json = ""; ///< Json string containing the data for all the Entities components
	std::string construction_script = ""; ///< Construction Script of an entity @warning Should only be used on Prefab roots
	std::string inline_script = ""; ///< Inline Script of an entity @warning Should only be used on Prefab roots
	DynamicPropertiesComponent properties; ///< Deserialized DynamicPropertiesComponent of the entity
	std::vector<std::string> children; ///< identifiers for the children templates of this entity
	bool has_inline = false; ///< Whether the Entity has an inline script @todo Redundant
};


/**
 * @brief Static class for parsing entities from string
*/
class EntityParser {
public:
	/**
	 * @brief Parses an entity from String
	 * @param entity_string string to parse the entity from
	 * @return EntityParseResult containing parsed data needed to create a new entity from the string
	*/
	static EntityParseResult ParseEntity(const std::string& entity_string);

};