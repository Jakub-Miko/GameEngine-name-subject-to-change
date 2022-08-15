#pragma once
#include <string>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>
#include <World/World.h>

enum class LightType : char {
	DIRECTIONAL = 0, POINT = 1, SPOTLIGHT = 2
};

class LightComponent;

template<>
class ComponentInitProxy<LightComponent> {
public:

	static void OnCreate(World& world, Entity entity);

};


class LightComponent {
	RuntimeTag("LightComponent")
public:
	LightComponent(LightType type = LightType::DIRECTIONAL, const glm::vec4& color = glm::vec4(1.0f)) : color(color), type(type), attenuation(glm::vec3(1.0, 0.1, 0.05)) {}
	LightComponent(glm::vec3 attenuation, const glm::vec4& color = glm::vec4(1.0f)) : color(color), type(LightType::POINT), attenuation(attenuation) {}
	float CalcRadiusFromAttenuation();
	static void ChangeType(LightType type, Entity ent);
	static void SetAttenuation(glm::vec3 attenuation_in, Entity ent);
	static void SetLightColor(glm::vec4 color, Entity ent);
	glm::vec3 GetAttenuation() const {
		return attenuation;
	}
	glm::vec4 GetLightColor() const {
		return color;
	}
	LightType type;
private:
	glm::vec4 color;
	glm::vec3 attenuation;
	JSON_SERIALIZABLE_IN_CLASS(LightComponent, color, type,attenuation)
};
