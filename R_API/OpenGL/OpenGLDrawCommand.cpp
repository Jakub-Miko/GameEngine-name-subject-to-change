#include "OpenGLDrawCommand.h"
#include <iostream>
#include <Utility/Profiler.h>

OpenGLDrawCommand::OpenGLDrawCommand(float pos_x, float pos_y, float size_x, float size_y)
	: pos_x(pos_x), pos_y(pos_y), size_x(size_x), size_y(size_y)
{
	
}

void OpenGLDrawCommand::Execute()
{
	PROFILE("DrawExecution");
	std::cout << "Drawing: " << pos_x << ", " << pos_y << ", " << size_x << ", " << size_y << "\n";
}
