#pragma once

class OpenGLRenderCommand {
public:
	virtual void Execute() = 0;
	virtual ~OpenGLRenderCommand() {  };
	OpenGLRenderCommand* next = nullptr;
};