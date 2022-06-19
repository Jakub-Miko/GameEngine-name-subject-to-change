#pragma once
#include <Renderer/RenderResource.h>

class Viewport {
public:
	Viewport();
	~Viewport();

	void Render();

	void BeginViewportFrameBuffer();

	void EndViewportFrameBuffer();

private:
	std::shared_ptr<RenderFrameBufferResource> viewport_frame_buffer;
	int viewport_resolution_x, viewport_resolution_y;
	glm::vec2 viewport_size;


};