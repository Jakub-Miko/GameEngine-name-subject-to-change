#pragma once
#include <string>
#include <chrono>

class FileExplorer {
public:
	FileExplorer();
	~FileExplorer();

	void Render();

	std::string GetSelectedFilePath() const {
		return selected_path;
	}

	void OpenImportDialog(const std::string source_relative_path_template, const std::string destination_relative_path_template);


private:
	std::string current_path;
	int import_id = 0;
	char* import_source_buffer = nullptr;
	char* import_dest_buffer = nullptr;
	int text_buffer_size = 200;
	std::string selected_path = "Unknown";
	std::chrono::steady_clock::time_point time_selected;
};