#pragma once

;

class RenderContext {
public:
	virtual void Init() = 0;
	virtual void PreInit() = 0;

	RenderContext(const RenderContext& ref) = delete;
	RenderContext(RenderContext&& ref) = delete;
	RenderContext& operator=(const RenderContext& ref) = delete;
	RenderContext& operator=(RenderContext&& ref) = delete;

	virtual ~RenderContext() {};

public:
	static RenderContext* Get();
	virtual void Destroy() = 0;
protected:
	RenderContext() = default;
};