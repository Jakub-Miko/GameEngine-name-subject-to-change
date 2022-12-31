#pragma once
#include <string>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>

template<typename T>
class ComponentInitProxy;

class LabelComponent {
	RUNTIME_TAG("LabelComponent")
public:
	LabelComponent() : label("Unknown") {}
	LabelComponent(const LabelComponent& other) : label(other.label) {}
	LabelComponent(const std::string& label) : label(label) {}
	std::string label;
};

template<>
class ComponentInitProxy<LabelComponent> {
public:
	static constexpr bool can_copy = true;

};

JSON_SERIALIZABLE(LabelComponent, label)