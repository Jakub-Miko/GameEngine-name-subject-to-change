#pragma once
#include <Input/Input.h>

class GlfwInput : public Input {
private:
	friend Input;
	glm::vec2 last_pos[2];
	char last_pos_index = 0;
	~GlfwInput();
	GlfwInput();
public:
	bool IsKeyPressed_impl(KeyCode key_code) override;
	virtual glm::vec2 GetMousePositionChange() override;
	virtual bool IsMouseButtonPressed_impl(MouseButtonCode key_code) override;
	virtual glm::vec2 GetMousePosition_impl() override;
	virtual void Update() override;
};