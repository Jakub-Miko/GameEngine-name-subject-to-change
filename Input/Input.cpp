#include "Input.h"
#if defined OpenGL
#include <platform/GLFW/GlfwInput.h>
#endif

Input* Input::instance = nullptr;

void Input::Init()
{
	if (!instance) {
	#if defined OpenGL
		instance = new GlfwInput();
	#endif
	}
}

Input* Input::Get()
{
	return instance;
}

void Input::Shutdown()
{
	if (instance) {
		delete instance;
	}
}
