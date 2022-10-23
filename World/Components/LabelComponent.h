#pragma once
#include <string>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>

class LabelComponent {
	RUNTIME_TAG("LabelComponent")
public:
	LabelComponent() : label("Unknown") {}
	LabelComponent(const std::string& label) : label(label) {}
	std::string label;
};

JSON_SERIALIZABLE(LabelComponent, label)