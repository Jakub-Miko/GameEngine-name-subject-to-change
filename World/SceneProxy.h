#pragma once
#include <string>

class SceneProxy {
public:
	SceneProxy();
	SceneProxy(const std::string& script_path);

	std::string scene_path;
	std::string script_path;
	bool has_script = false;

};