#include "SceneProxy.h"
#include <json.hpp>
#include <fstream>
#include <stdexcept>
#include <FileManager.h>
#include <ConfigManager.h>

SceneProxy::SceneProxy() : scene_path() {
	scene_path = ConfigManager::Get()->GetString("default_scene");
}

SceneProxy::SceneProxy(const std::string& scene_path) : scene_path(scene_path) {

}

