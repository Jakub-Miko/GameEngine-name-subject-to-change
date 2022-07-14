#pragma once 
#include <Core/RuntimeTag.h>
#include <World/Entity.h>

enum class PrefabStatus : char {
	OK = 0, ERROR = 1, UNINITIALIZED = 2
};

struct PrefabComponent {
	RuntimeTag("PrefabComponent");
	PrefabComponent(const std::string& file_path = "Undefined", Entity first_child = Entity()) : file_path(file_path) {}
	Entity first_child = Entity();
	std::string file_path;
	PrefabStatus status = PrefabStatus::OK;
};

JSON_SERIALIZABLE(PrefabComponent, file_path)