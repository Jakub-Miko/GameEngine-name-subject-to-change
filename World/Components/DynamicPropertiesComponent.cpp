#include "DynamicPropertiesComponent.h"
#include <stdexcept>

template<typename T, typename ... Args>
bool TestSetProperty(std::variant<Args...>& type, const nlohmann::json& object, const std::string& type_name) {
	if (type_name == RuntimeTag<T>::GetName()) {
		type = object["value"].get<T>();
		return true;
	}
	return false;
}


template<typename ... Args>
bool SetProperty(std::variant<Args...>& type, const nlohmann::json& object, const std::string& type_name) {
	return (TestSetProperty<Args>(type, object, type_name) || ...);
}



void to_json(nlohmann::json& j, const DynamicPropertiesComponent& p) {
	j = nlohmann::json::array();
	for (auto& prop : p.m_Properties) {
		nlohmann::json current;
		current["name"] = prop.first;
		std::visit([&current](auto& property) {
			using T = std::decay_t<decltype(property)>;
			current["type"] = RuntimeTag<T>::GetName();
			current["value"] = property;
			}, prop.second);
		j.push_back(current);
	}
}

void from_json(const nlohmann::json& j, DynamicPropertiesComponent& p) {
	for (auto object : j) {
		std::string name = object["name"];
		std::string type = object["type"];
		Script_Variant_type variant;
		if (!SetProperty(variant, object, type)) {
			throw std::runtime_error("Unable to read property: " + name + "; of type: " + type);
		}
		p.m_Properties.insert(std::make_pair(name, variant));
	}
}