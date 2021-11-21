#pragma once
#include <string>
#include <variant>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

using Script_Variant_type = std::variant<int, float, double, glm::vec2, glm::vec3, std::string>;


class ScriptComponent {
public:
	ScriptComponent(const std::string& ref) : script_path(ref) {}
	std::string script_path;

	//TODO: Implement custom allocator
	std::unordered_map<std::string, Script_Variant_type> m_Properties;
};
