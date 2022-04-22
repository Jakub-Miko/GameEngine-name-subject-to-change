#include "SceneProxy.h"
#include <json.hpp>
#include <fstream>
#include <stdexcept>
#include <FileManager.h>
#include <ConfigManager.h>

SceneProxy::SceneProxy() : script_path("Undefined"), scene_path(), primary_entity(Entity()) {
	using namespace nlohmann;
	scene_path = ConfigManager::Get()->GetString("default_scene");
	std::string path = FileManager::Get()->GetAssetFilePath(scene_path);
	std::ifstream file_stream(path);
	if (!file_stream.is_open()) {
		throw std::runtime_error("File could not be loaded: " + path);
	}

	json json_config;
	json_config << file_stream;
	file_stream.close();

	if (json_config.find("script_path") != json_config.end()) {
		has_script = true;
		script_path = json_config["script_path"].get<std::string>();
	}


	if (json_config.find("primary_entity") != json_config.end()) {
		primary_entity = json_config["primary_entity"].get<uint32_t>();
	}

}

SceneProxy::SceneProxy(const std::string& scene_path) : script_path("Undefined"), scene_path(scene_path) {
	using namespace nlohmann;
	std::string path = FileManager::Get()->GetAssetFilePath(scene_path);
	std::ifstream file_stream(path);
	if (!file_stream.is_open()) {
		throw std::runtime_error("File could not be loaded: " + path);
	}

	json json_config;
	json_config << file_stream;
	file_stream.close();

	if (json_config.find("script_path") != json_config.end()) {
		has_script = true;
		script_path = json_config["script_path"].get<std::string>();
	}

	if (json_config.find("primary_entity") != json_config.end()) {
		primary_entity = json_config["primary_entity"].get<uint32_t>();
	}

}

