#pragma once
#include <Renderer/RenderResourceManager.h>

enum class ViewportGizmoMode : char {
	TRANSLATION = 0, SCALE = 1, ROTATION = 2
};

enum class ViewportGizmoSpace : char {
	WORLD = 0, LOCAL = 1
};

enum class ViewportSelectMode : char {
	PREFABS = 0, PREFAB_CHILDREN = 1
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

	void SetGizmoSpace(ViewportGizmoSpace in_gizmo_space) {
		gizmo_space = in_gizmo_space;
	}
	
	ViewportGizmoSpace GetGizmoSpace() const {
		return gizmo_space;
	}

	void SelectEntityOnViewportPos(float x, float y);

private:
	std::shared_ptr<RenderFrameBufferResource> viewport_frame_buffer;
	Future<read_pixel_data> entity_pick_request;
	int viewport_resolution_x, viewport_resolution_y;
	glm::vec2 viewport_size;
	float translation_snap = 1.0f, scale_snap = 1.0f, rotation_snap = 1.0f;
	bool snap_enabled = false;
	ViewportGizmoMode gizmo_mode = ViewportGizmoMode::TRANSLATION;
	ViewportGizmoSpace gizmo_space = ViewportGizmoSpace::LOCAL;
	ViewportSelectMode select_mode = ViewportSelectMode::PREFABS;
	bool is_paste_pressed = false;

};