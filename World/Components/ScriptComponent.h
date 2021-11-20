#pragma once
#include <string>

class ScriptComponent {
public:
	ScriptComponent(const std::string& ref) : script_path(ref) {}
	std::string script_path;
};
