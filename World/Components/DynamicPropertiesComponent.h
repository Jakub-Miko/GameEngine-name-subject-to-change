#pragma once
#include <unordered_map>
#include <variant>
#include <glm/glm.hpp>

using Script_Variant_type = std::variant<int, float, double, glm::vec2, glm::vec3, std::string>;

class DynamicPropertiesComponent {
public:

	DynamicPropertiesComponent() : m_Properties() {}

	DynamicPropertiesComponent(const DynamicPropertiesComponent& ref) : m_Properties(ref.m_Properties) {}

	DynamicPropertiesComponent(DynamicPropertiesComponent&& ref) : m_Properties(std::move(ref.m_Properties)) {}

	DynamicPropertiesComponent& operator=(const DynamicPropertiesComponent& ref) {
		m_Properties = ref.m_Properties;
		return *this;
	}
	
	DynamicPropertiesComponent& operator=(DynamicPropertiesComponent&& ref) {
		m_Properties = std::move(ref.m_Properties);
		return *this;
	}

public:
	//TODO: Implement custom allocator
	std::unordered_map<std::string, Script_Variant_type> m_Properties;
};