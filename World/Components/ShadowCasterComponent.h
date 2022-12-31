#pragma once 
#include <Core/RuntimeTag.h>
#include <Renderer/RenderResource.h>
#include <World/World.h>
#include <Core/UnitConverter.h>

class ShadowCasterComponent {
	RUNTIME_TAG("ShadowCasterComponent")
public:
	ShadowCasterComponent(int width = 800, int height = 800, float near_plane = 0.1f, float far_plane = 500.0f);
	ShadowCasterComponent(const ShadowCasterComponent& other) : res_x(other.res_x), res_y(other.res_y), cascades(other.cascades), near_plane(other.near_plane), far_plane(other.far_plane) {}
	~ShadowCasterComponent() {

	}
	std::shared_ptr<RenderFrameBufferResource> shadow_map = nullptr;
	int res_x = 800, res_y = 600, cascades = 5;
	float near_plane = 0.1f, far_plane = 500.0f;
	glm::mat4 light_view_matrix = glm::mat4(1.0);
};

template<>
class ComponentInitProxy<ShadowCasterComponent>{
public:

	static void OnCreate(World& world, Entity entity);
	static constexpr bool can_copy = true;
};

JSON_SERIALIZABLE(ShadowCasterComponent, res_x, res_y, near_plane, far_plane);