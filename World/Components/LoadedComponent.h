#pragma once
#include <string>

class LoadedComponent {
public:
	LoadedComponent(const std::string& file_path) : file_path(file_path) {}
	std::string file_path;
};