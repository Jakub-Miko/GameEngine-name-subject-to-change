#pragma once
#include <string>
#include <World/Entity.h>

/**
 * @brief Represents the source of the scene without it being loaded. 
*/
class SceneProxy {
public:
	SceneProxy();
	SceneProxy(const std::string& scene_path);

	std::string scene_path; ///< Path to the Scene file to load
};