#pragma once

class OpenGLRenderCommand {
public:
	virtual void Execute() = 0;
	virtual ~OpenGLRenderCommand() {  };
	OpenGLRenderCommand* next = nullptr;
};

template<typename F>
class OpenGLRenderCommandAdapter : public OpenGLRenderCommand {
public:
	OpenGLRenderCommandAdapter(F func) : func(func) {}

	virtual void Execute() override {
		func();
	};

	virtual ~OpenGLRenderCommandAdapter() {}

private:
	F func;
};