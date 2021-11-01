#pragma once
#include <Events/KeyCodes.h>
#include <glm/fwd.hpp>

class Input {
public:
	static void Init();
	static Input* Get();
	static void Shutdown();

	virtual bool IsKeyPressed(KeyCode key_code) = 0;
	virtual bool IsMouseButtonPressed(MouseButtonCode key_code) = 0;
	virtual glm::vec2 GetMoutePosition() = 0;

private:
	static Input* instance;
};