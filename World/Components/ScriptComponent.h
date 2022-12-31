#pragma once
#include <string>
#include <variant>
#include <LuaEngine.h>
#include <glm/glm.hpp>
#include <Events/KeyPressEvent.h>
#include <Events/MouseButtonPressEvent.h>
#include <unordered_map>
#include <string>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/World.h>
#include <Core/RuntimeTag.h>

class ScriptComponent {
	RUNTIME_TAG("ScriptComponent")
public:
	ScriptComponent(const std::string& ref) : script_path(ref) {}
	ScriptComponent() : script_path("") {}
	std::string script_path;

};


template<>
class ComponentInitProxy<ScriptComponent> {
public:

	static void OnCreate(World& world, Entity entity) {
		if (!world.HasComponentSynced<DynamicPropertiesComponent>(entity)) {
			world.SetComponent<DynamicPropertiesComponent>(entity);
		}
	}

	static constexpr bool can_copy = false;

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
class LuaEngineObjectDelegate<Entity> {
public:
	static void SetObject(LuaEngineProxy proxy, const Entity& value) {
		proxy.SetTableItem((int)value.id, "id");
		
	}

	static Entity GetObject(LuaEngineProxy proxy, int index = -1) {
		return Entity(proxy.GetTableField<int>("id", index));
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

template<>
class LuaEngineObjectDelegate<glm::vec4> {
public:
	static void SetObject(LuaEngineProxy proxy, const glm::vec4& value) {
		proxy.SetTableItem(value.x, "x");
		proxy.SetTableItem(value.y, "y");
		proxy.SetTableItem(value.z, "z");
		proxy.SetTableItem(value.w, "w");
	}

	static glm::vec4 GetObject(LuaEngineProxy proxy, int index = -1) {
		return glm::vec4(proxy.GetTableField<float>("x", index), proxy.GetTableField<float>("y", index), proxy.GetTableField<float>("z", index), proxy.GetTableField<float>("w", index));
	}

};

template<>
class LuaEngineObjectDelegate<KeyPressedEvent> {
public:
	static void SetObject(LuaEngineProxy proxy, const KeyPressedEvent& value) {
		proxy.SetTableItem((int)value.key_code, "key_code");
		proxy.SetTableItem((int)value.key_mods, "key_mods");
		proxy.SetTableItem((int)value.press_type, "press_type");
	}

	static KeyPressedEvent GetObject(LuaEngineProxy proxy, int index = -1) {
		return KeyPressedEvent((KeyCode)proxy.GetTableField<int>("key_code", index), (KeyPressType)proxy.GetTableField<int>("press_type", index), (KeyModifiers)proxy.GetTableField<int>("key_mods", index));
	}

};

template<>
class LuaEngineObjectDelegate<MouseButtonPressEvent> {
public:
	static void SetObject(LuaEngineProxy proxy, const MouseButtonPressEvent& value) {
		proxy.SetTableItem((int)value.key_code, "key_code");
		proxy.SetTableItem((int)value.key_mods, "key_mods");
		proxy.SetTableItem((int)value.press_type, "press_type");
	}

	static MouseButtonPressEvent GetObject(LuaEngineProxy proxy, int index = -1) {
		return MouseButtonPressEvent((MouseButtonCode)proxy.GetTableField<int>("key_code", index), (KeyPressType)proxy.GetTableField<int>("press_type", index), (KeyModifiers)proxy.GetTableField<int>("key_mods", index));
	}

};