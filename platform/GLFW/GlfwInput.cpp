#include "GlfwInput.h"
#include <GLFW/glfw3.h>
#include <ConfigManager.h>
#include <Events/MouseButtonPressEvent.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Events/KeyPressEvent.h>
#include <Events/MouseMoveEvent.h>
#include <Application.h>
#include <glm/glm.hpp>
#include <Core/UnitConverter.h>

GlfwInput::~GlfwInput()
{
	glfwSetKeyCallback(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), NULL);
	glfwSetMouseButtonCallback(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), NULL);
	glfwSetCursorPosCallback(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), NULL);
}

GlfwInput::GlfwInput()
{
	glfwSetKeyCallback(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		KeyPressedEvent ev((KeyCode)key, (KeyPressType)action, (KeyModifiers)mods);
		Application::Get()->SendEvent(&ev);
		});

	glfwSetMouseButtonCallback(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), [](GLFWwindow* window, int key, int action, int mods) {
		MouseButtonPressEvent ev((MouseButtonCode)key, (KeyPressType)action, (KeyModifiers)mods);
		Application::Get()->SendEvent(&ev);
		});

	if (ConfigManager::Get()->GetInt("EnableMouseMoveEvent")) {
		glfwSetCursorPosCallback(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), [](GLFWwindow* window, double x, double y) {
			MouseMoveEvent ev(x, y);
			Application::Get()->SendEvent(&ev);
			});
	}
}

bool GlfwInput::IsKeyPressed_impl(KeyCode key_code)
{
	return glfwGetKey(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(),(int)key_code) == GLFW_PRESS;
}

bool GlfwInput::IsMouseButtonPressed_impl(MouseButtonCode key_code)
{
	return glfwGetMouseButton(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), (int)key_code) == GLFW_PRESS;
}

glm::vec2 GlfwInput::GetMoutePosition_impl()
{
	double x, y;
	glfwGetCursorPos(reinterpret_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), &x, &y);
	return UnitConverter::ScreenSpaceToNDC(glm::vec2(x, y));
}
