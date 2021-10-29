#pragma once
#include "OpenGLRenderCommand.h"

class OpenGLBindOpenGLContextCommand : public OpenGLRenderCommand {
public:
	
	OpenGLBindOpenGLContextCommand() = default;

	virtual void Execute() override;

};