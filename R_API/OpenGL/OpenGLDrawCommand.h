#pragma once
#include <R_API/OpenGL/OpenGLRenderCommand.h>

class OpenGLDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLDrawCommand(float pos_x, float pos_y, float size_x, float size_y);
	virtual void Execute() override;
private:
	float pos_x;
	float pos_y;
	float size_x;
	float size_y;
};