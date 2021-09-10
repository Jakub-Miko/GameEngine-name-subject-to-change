#pragma once
#include <R_API/OpenGL/OpenGLRenderCommand.h>

class OpenGLDrawCommand : public OpenGLRenderCommand {
public:
	OpenGLDrawCommand(float pos_x, float pos_y, float size_x, float size_y);
	virtual void Execute() override;
private:
	struct QuadData {
		unsigned int vertex_array;
	};
	static bool initialized;
	static QuadData quad_data;
	float pos_x;
	float pos_y;
	float size_x;
	float size_y;
};