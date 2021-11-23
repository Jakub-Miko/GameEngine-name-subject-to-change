#pragma once
#include <string>
#include <variant>
#include <LuaEngine.h>
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

template<>
class LuaEngineObjectDelegate<glm::vec2> {
public:
	static void SetObject(LuaEngineProxy proxy, const glm::vec2& value) {
		proxy.SetTableItem(value.x, "x");
		proxy.SetTableItem(value.y, "y");
	}

	static glm::vec2 GetObject(LuaEngineProxy proxy, int index = -1) {
		return glm::vec2(proxy.GetTableField<double>("x", index), proxy.GetTableField<double>("y", index));
	}

};

template<>
class LuaEngineObjectDelegate<glm::vec3> {
public:
	static void SetObject(LuaEngineProxy proxy, const glm::vec3& value) {
		proxy.SetTableItem(value.x, "x");
		proxy.SetTableItem(value.y, "y");
		proxy.SetTableItem(value.z, "z");
	}

	static glm::vec3 GetObject(LuaEngineProxy proxy, int index = -1) {
		return glm::vec3(proxy.GetTableField<float>("x", index), proxy.GetTableField<float>("y", index), proxy.GetTableField<float>("z", index));
	}

};
