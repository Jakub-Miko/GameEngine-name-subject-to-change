#pragma once
#include <Core/RuntimeTag.h>

template<typename T>
class ComponentInitProxy;

class SerializableComponent {
	RUNTIME_TAG("SerializableComponent")
};


template<>
class ComponentInitProxy<SerializableComponent> {
public:
	static constexpr bool can_copy = true;

};