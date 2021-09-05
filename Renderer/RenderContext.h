#pragma once

class RenderContext {
public:
	virtual void Init() = 0;
	virtual void PreInit() = 0;

	RenderContext() = default;

	RenderContext(const RenderContext& ref) = delete;
	RenderContext(RenderContext&& ref) = delete;
	RenderContext& operator=(const RenderContext& ref) = delete;
	RenderContext& operator=(RenderContext&& ref) = delete;

public:
	static RenderContext* Get();

private:
	static RenderContext* instance;
};