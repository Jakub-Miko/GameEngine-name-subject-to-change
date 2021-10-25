#pragma once
#include <Renderer/RenderContext.h>

class OpenGLRenderContext : public RenderContext {
public:
	friend RenderContext;
	virtual void Init() override;
	virtual void PreInit() override;
	virtual void Destroy() override;
private:
	OpenGLRenderContext() { };
};