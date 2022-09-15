#include "ShadowCasterComponent.h"
#include "LightComponent.h"

ShadowCasterComponent::ShadowCasterComponent(int width, int height, float near_plane, float far_plane)
	: res_x(width), res_y(height), shadow_map(nullptr), near_plane(near_plane), far_plane(far_plane){

}


void ComponentInitProxy<ShadowCasterComponent>::OnCreate(World& world, Entity entity) {
	if (!world.HasComponentSynced<LightComponent>(entity)) throw std::runtime_error("Shadow component can only be assigned to entity with a LightComponent");
}