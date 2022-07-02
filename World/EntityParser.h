#pragma once
#include <string>
#include <vector>
#include <World/Components/DynamicPropertiesComponent.h>
#include <unordered_map>

struct EntityParseResult {
	std::string component_json = "";
	std::string construction_script = "";
	std::string inline_script = "";
	DynamicPropertiesComponent properties;
	std::vector<std::string> children;
	bool has_inline = false;
};



class EntityParser {
public:

	static EntityParseResult ParseEntity(const std::string& script);

};