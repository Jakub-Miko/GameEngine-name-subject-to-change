#pragma once

struct ImGuiIO;

class Editor {
public:
	Editor(const Editor& ref) = delete;
	Editor(Editor&& ref) = delete;
	Editor& operator=(const Editor& ref) = delete;
	Editor& operator=(Editor&& ref) = delete;

	static void Init();
	static void Shutdown();

	static void Run();

	static void Render();

	static Editor* Get();
private:
	static Editor* instance;

	ImGuiIO* io = nullptr;
	Editor();
	~Editor();

};