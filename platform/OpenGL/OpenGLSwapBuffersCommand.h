#pragma once
#include <platform/OpenGL/OpenGLRenderCommand.h>

class OpenGLSwapBuffersCommand : public OpenGLRenderCommand {
public:
	OpenGLSwapBuffersCommand();
	virtual void Execute() override;
private:
};