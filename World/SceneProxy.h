#pragma once
#include <string>
#include <World/Entity.h>

class SceneProxy {
public:
	SceneProxy();
	SceneProxy(const std::string& script_path);

	Entity primary_entity = Entity();
	std::string scene_path;
	std::string script_path;
	bool has_script = false;

};