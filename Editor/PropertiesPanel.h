#pragma once
#include <World/Entity.h>

class PropertiesPanel {
public:
	PropertiesPanel();
	~PropertiesPanel();

	void Render();

private:
	char* text_buffer = nullptr;
	int buffer_size = 200;
	Entity last_entity = Entity();

};