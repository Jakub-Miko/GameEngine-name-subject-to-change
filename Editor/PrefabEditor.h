#pragma once 
#include <string>
#include <World/Entity.h>
#include <vector>

struct PrefabEditorWindow {
	Entity entity;
};

class PrefabEditor {
public:
	PrefabEditor();
	~PrefabEditor();

	void Render();

	void OpenPrefabEditorWindow(Entity entity);

	void ClosePrefabEditorWindow(Entity entity);
	
private:
	void RenderWindow(PrefabEditorWindow& window);

	std::vector<PrefabEditorWindow> open_windows;
};