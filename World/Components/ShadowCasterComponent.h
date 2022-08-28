#pragma once 
#include <Core/RuntimeTag.h>
#include <Renderer/RenderResource.h>
#include <World/World.h>
#include <Core/UnitConverter.h>

class ShadowCasterComponent {
	RuntimeTag("ShadowCasterComponent")
public:
	ShadowCasterComponent(int width = 800, int height = 800);
	~ShadowCasterComponent() {

	}
	std::shared_ptr<RenderFrameBufferResource> shadow_map = nullptr;
	int res_x = 800, res_y = 600;
	glm::mat4 light_view_matrix = glm::mat4(1.0);
};

template<>
class ComponentInitProxy<ShadowCasterComponent>{
public:

	static void OnCreate(World& world, Entity entity);

};

JSON_SERIALIZABLE(ShadowCasterComponent, res_x, res_y);