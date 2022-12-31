#pragma once
#include <unordered_map>
#include <variant>
#include <glm/glm.hpp>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>

template<typename T>
class ComponentInitProxy;

using Script_Variant_type = std::variant<int, float, double, glm::vec2, glm::vec3, glm::vec4, std::string, Entity>;

class DynamicPropertiesComponent {
	RUNTIME_TAG("DynamicPropertiesComponent")
public:

	DynamicPropertiesComponent() : m_Properties() {}

	DynamicPropertiesComponent(const DynamicPropertiesComponent& ref) : m_Properties(ref.m_Properties) {}

	DynamicPropertiesComponent(DynamicPropertiesComponent&& ref) noexcept : m_Properties(std::move(ref.m_Properties)) {}

	DynamicPropertiesComponent& operator=(const DynamicPropertiesComponent& ref) {
		m_Properties = ref.m_Properties;
		return *this;
	}
	
	DynamicPropertiesComponent& operator=(DynamicPropertiesComponent&& ref) noexcept {
		m_Properties = std::move(ref.m_Properties);
		return *this;
	}

public:
	//TODO: Implement custom allocator
	std::unordered_map<std::string, Script_Variant_type> m_Properties;
};

template<>
class ComponentInitProxy<DynamicPropertiesComponent> {
public:
	static constexpr bool can_copy = true;

};

#pragma region Json_Serialization

void to_json(nlohmann::json& j, const DynamicPropertiesComponent& p);

void from_json(const nlohmann::json& j, DynamicPropertiesComponent& p);

#pragma endregion
