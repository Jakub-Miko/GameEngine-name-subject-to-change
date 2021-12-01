#pragma once
#include <string>
#include <vector>
#include <World/Components/DynamicPropertiesComponent.h>
#include <unordered_map>

struct EntityParseResult {
	
	std::string construction_script;		
	DynamicPropertiesComponent properties;
	std::vector<std::string> children;

};



class EntityParser {
public:

	static EntityParseResult ParseConstructionScript(const std::string& script);

};