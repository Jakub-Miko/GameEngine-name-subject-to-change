#pragma once
#include <string>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>
#include <Renderer/ReflectionMap.h>
#include <Renderer/TextureManager.h>

class SkylightComponent {
	RUNTIME_TAG("SkylightComponent")
public:
	SkylightComponent() : reflection_map_path(""), color(glm::vec4(1.0f)) {}

	SkylightComponent(const std::string& path, const glm::vec4& color = glm::vec4(1.0f)) : color(color) {
		SetReflectionMap(path);
	
	}
	SkylightComponent(const SkylightComponent& other) : color(other.color), show_background(other.show_background) {
		SetReflectionMap(other.reflection_map_path);
	}
	void SetLightColor(glm::vec4 color) {
		this->color = color;
	}

	glm::vec4 GetLightColor() const {
		return color;
	}

	std::shared_ptr<ReflectionMap> GetReflectionMap() const {
		return reflection_map;
	}

	std::string GetReflectionMapPath() const {
		return reflection_map_path;
	}

	void SetShowBackground(bool enable) {
		show_background = enable;
	}

	bool IsBackgroundVisible() const {
		return show_background;
	}

	void SetReflectionMap(const std::string& path) {
		reflection_map_path = path;
		reflection_map = TextureManager::Get()->GetReflectionMap(path);
	}

private:
	glm::vec4 color;
	bool show_background = false;
	std::shared_ptr<ReflectionMap> reflection_map = nullptr;
	std::string reflection_map_path = "";
};

template<>
class ComponentInitProxy<SkylightComponent> {
public:
	static constexpr bool can_copy = true;

};


#pragma region Json_Serialization

inline void to_json(nlohmann::json& j, const SkylightComponent& p) {
	j["reflection_map_path"] = p.GetReflectionMapPath();
	j["color"] = p.GetLightColor();
	j["show_background"] = p.IsBackgroundVisible();

}

inline void from_json(const nlohmann::json& j, SkylightComponent& p) {
	p.SetLightColor(j["color"].get<glm::vec4>());
	p.SetReflectionMap(j["reflection_map_path"].get<std::string>());
	p.SetShowBackground(j["show_background"].get<bool>());
}

#pragma endregion