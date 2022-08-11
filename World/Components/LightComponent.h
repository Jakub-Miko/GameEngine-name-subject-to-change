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
	LightComponent(LightType type = LightType::DIRECTIONAL, const glm::vec4& color = glm::vec4(1.0f)) : color(color), type(type) {}
	glm::vec4 color;
	LightType type;
};

JSON_SERIALIZABLE(LightComponent, color, type)