#pragma once
#include <string>
#include <World/Entity.h>

class SceneProxy {
public:
	SceneProxy();
	SceneProxy(const std::string& scene_path);

	std::string scene_path;
};