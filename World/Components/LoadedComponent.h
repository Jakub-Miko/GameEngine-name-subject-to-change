#pragma once
#include <string>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>

class LoadedComponent {
	RuntimeTag("LoadedComponent")
public:
	LoadedComponent() : file_path("Unknown") {}
	LoadedComponent(const std::string& file_path) : file_path(file_path) {}
	std::string file_path;
};

JSON_SERIALIZABLE(LoadedComponent, file_path)