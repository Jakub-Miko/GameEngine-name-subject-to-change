#pragma once
#include <string>

class FileExplorer {
public:
	FileExplorer();
	~FileExplorer();

	void Render();

private:
	std::string current_path;

};