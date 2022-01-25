#pragma once
#include <Renderer/RenderCommandQueue.h>
#include <Renderer/Renderer.h>

class RenderContext {
public:
	virtual void Init() = 0;
	virtual void PreInit() = 0;

	RenderContext(const RenderContext& ref) = delete;
	RenderContext(RenderContext&& ref) = delete;
	RenderContext& operator=(const RenderContext& ref) = delete;
	RenderContext& operator=(RenderContext&& ref) = delete;

	virtual void StartShutdown() = 0;
	virtual ~RenderContext() {};

public:
	static RenderContext* Get();

	static void Create();

	static void Shutdown();

protected:
	void SetRenderQueue(RenderCommandQueue* queue, RenderQueueTypes type);
	virtual void Destroy() = 0;
	static RenderContext* instance;
	RenderContext() = default;
};