#include "SquareRenderSystem.h"
#include <World/Components/SquareComponent.h>
#include <World/System.h>


void SquareRenderSystem(World& world) {
	auto func_1 = [](ComponentCollection compcol, system_view_type<SquareComponent>& comps, entt::registry* reg) {
		PROFILE("SquareREnderRunThread");
		auto list = Renderer::Get()->GetRenderCommandList();
		for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
			auto& comp = reg->get<SquareComponent>(*iter);
			list->DrawSquare(comp.pos, comp.size, comp.color);
		};
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return list;
	};

	auto func_2 = [](ComponentCollectionParameters params, system_return_type<RenderCommandList>& lists) {
		PROFILE("SUBMITTOQUEUE");
		for (int i = 0; i < params.num_of_collections + (params.extra_collections_size != 0); i++) {
			Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(lists[i].GetValue());
		}
	};

	RunSystem<SquareComponent, RenderCommandList>(world, func_1, func_2);
};
