#pragma once
#include <Events/KeyCodes.h>
#include <glm/glm.hpp>
#ifdef EDITOR
#include <Editor/Editor.h>
#endif


class Input {
public:
	static void Init();
	static Input* Get();
	static void Shutdown();

	virtual bool IsKeyPressed_impl(KeyCode key_code) = 0;
	virtual bool IsMouseButtonPressed_impl(MouseButtonCode key_code) = 0;
	virtual glm::vec2 GetMoutePosition_impl() = 0;

	bool IsKeyPressed(KeyCode key_code) {
#ifdef EDITOR
		if (!Editor::Get()->IsViewportFocused()) return false;
#endif // EDITOR
		return IsKeyPressed_impl(key_code);
	}
	bool IsMouseButtonPressed(MouseButtonCode key_code) {
#ifdef EDITOR
		if (!Editor::Get()->IsViewportFocused()) return false;
#endif // EDITOR
		return IsMouseButtonPressed_impl(key_code);
	}

	glm::vec2 GetMoutePosition() {
#ifdef EDITOR
		if (!Editor::Get()->IsViewportFocused()) return last_pos;
		last_pos = GetMoutePosition_impl();
		return last_pos;
#endif // EDITOR
		return GetMoutePosition_impl();
	}

#ifdef EDITOR
	bool IsKeyPressed_Editor(KeyCode key_code) {
		return IsKeyPressed_impl(key_code);
	}
	bool IsMouseButtonPressed_Editor(MouseButtonCode key_code) {
		return IsMouseButtonPressed_impl(key_code);
	}

	glm::vec2 GetMoutePosition_Editor() {
		return GetMoutePosition_impl();
	}
#endif



private:
	static Input* instance;
#ifdef EDITOR
	glm::vec2 last_pos = { 0,0 };
#endif
};