#pragma once
#include <Renderer/RenderResource.h>

enum class ViewportGizmoMode : char {
	TRANSLATION = 0, SCALE = 1, ROTATION = 2
};

class Viewport {
public:
	Viewport();
	~Viewport();

	void Render();

	void BeginViewportFrameBuffer();

	void EndViewportFrameBuffer();

	ViewportGizmoMode GetGizmoMode() const {
		return gizmo_mode;
	}

	void SetGizmoMode(ViewportGizmoMode in_gizmo_mode) {
		gizmo_mode = in_gizmo_mode;
	}

private:
	std::shared_ptr<RenderFrameBufferResource> viewport_frame_buffer;
	int viewport_resolution_x, viewport_resolution_y;
	glm::vec2 viewport_size;
	ViewportGizmoMode gizmo_mode = ViewportGizmoMode::TRANSLATION;

};