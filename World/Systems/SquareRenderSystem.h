#include <World/World.h>
#include <entt/entt.hpp>
#include <World/Components/SquareComponent.h>
#include <Renderer/Renderer.h>

inline void SquareRenderSystem(World& world) {
	entt::registry& reg = world.GetRegistry();
	auto comps = reg.view<SquareComponent>();
	auto list = Renderer::Get()->GetRenderCommandList();
	for (auto iter = comps.rbegin(); iter != comps.rend();iter++) {
		auto& comp = reg.get<SquareComponent>(*iter);
		list->DrawSquare(comp.pos, comp.size, comp.color);
	}
	Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(list);
}
