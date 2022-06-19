#pragma once
#include <Input/Input.h>

class GlfwInput : public Input {
private:
	friend Input;
	~GlfwInput();
	GlfwInput();
public:
	bool IsKeyPressed_impl(KeyCode key_code) override;
	virtual bool IsMouseButtonPressed_impl(MouseButtonCode key_code) override;
	virtual glm::vec2 GetMoutePosition_impl() override;
};