#include "Input.h"
#if defined OpenGL_API
#include <platform/GLFW/GlfwInput.h>
#endif

Input* Input::instance = nullptr;

void Input::Init()
{
	if (!instance) {
	#if defined OpenGL_API
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
