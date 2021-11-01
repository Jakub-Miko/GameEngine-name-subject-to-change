#pragma once
#include <Input/Input.h>

class GlfwInput : public Input {
private:
	friend Input;
	~GlfwInput();
	GlfwInput();
public:
	bool IsKeyPressed(KeyCode key_code) override;
	virtual bool IsMouseButtonPressed(MouseButtonCode key_code) override;
	virtual glm::vec2 GetMoutePosition() override;
};