#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>

class Material;
struct MaterialEditorWindow {
	MaterialEditorWindow();
	~MaterialEditorWindow();

	std::string path;
	std::shared_ptr<Material> mat;
	std::vector<char*> text_buffers;
};

class MaterialEditor {
public:
	MaterialEditor();
	~MaterialEditor();

	void Render();

	void OpenEditorWinow(const std::string& material_path);
	void CloseMaterialWinow(int index);

private:
	void RenderWinow(MaterialEditorWindow& window, int current_index);
	std::unordered_set<std::string> open_materials;
	std::vector<MaterialEditorWindow> windows;
};