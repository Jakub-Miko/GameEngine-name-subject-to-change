#include "SquareRenderSystem.h"
#include <entt/entt.hpp>
#include <Profiler.h>
#include <TaskSystemFence.h>
#include <World/Components/SquareComponent.h>
#include <Renderer/Renderer.h>
#include <TaskSystem.h>
#include <cmath>

struct ComponentCollectionParameters {
	int num_of_collections;
	int collection_size;
	int extra_collections_size;
};

struct ComponentCollection {
	int size;
	int start_index;
};

static ComponentCollectionParameters GetCollectionsFromSize(int size, int num_of_threads, int min_collection_size = 10) {
	int chunk_size = std::ceil((float)size / (float)num_of_threads);
	int num_of_chunks;
	int last;
	if (chunk_size >= min_collection_size) {
		num_of_chunks = std::ceil((float)size / (float)chunk_size);
	}
	else {
		num_of_chunks = std::ceil((float)size / (float)min_collection_size);
		chunk_size = min_collection_size;
	}
	if ((float)size / (float)num_of_chunks == (float)chunk_size) {
		last = 0;
	}
	else {
		last = size - ((num_of_chunks-1) * chunk_size);
		num_of_chunks--;
	}
	return ComponentCollectionParameters{ num_of_chunks ,chunk_size,last };
};


void SquareRenderSystem(World& world) {
	entt::registry& reg = world.GetRegistry();
	auto comps = reg.view<SquareComponent>();

	ComponentCollectionParameters params = GetCollectionsFromSize(comps.size(), TaskSystem::Get()->GetProps().num_of_threads + 1, 1);

	std::vector<Future<RenderCommandList*>> lists;
	lists.reserve(params.num_of_collections);

	auto task = [](ComponentCollection compcol, decltype(comps)& comps, entt::registry* reg) {
		PROFILE("SquareREnderRunThread");
		auto list = Renderer::Get()->GetRenderCommandList();
		for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
			auto& comp = reg->get<SquareComponent>(*iter);
			list->DrawSquare(comp.pos, comp.size, comp.color);
		};
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return list;
	};
	PROFILE("SquareRenderSubmit"); 
	for (int i = 0; i < params.num_of_collections; i++) {
		ComponentCollection comp{ params.collection_size,params.collection_size * i };
		auto task1 = TaskSystem::Get()->CreateTask<RenderCommandList*>(task, comp, comps, &reg);
		lists.push_back(task1->GetFuture());
		TaskSystem::Get()->Submit(task1);
	}
	if (params.extra_collections_size != 0) {
		ComponentCollection comp{ params.extra_collections_size, params.collection_size * params.num_of_collections };
		auto task1 = TaskSystem::Get()->CreateTask<RenderCommandList*>(task, comp, comps, &reg);
		lists.push_back(task1->GetFuture());
		TaskSystem::Get()->Submit(task1);
	}
	

	TaskSystemFence fence;
	auto task5 = [&fence]() {
		fence.Signal(1); TaskSystem::Get()->FlushLoop();
	};
	TaskSystem::Get()->SetIdleTask(TaskSystem::Get()->CreateTask(task5));
	TaskSystem::Get()->JoinTaskSystem([&fence]() -> bool { 
		return fence.IsValue(1); 
		});


	PROFILE("SUBMITTOQUEUE");
	for (int i = 0; i < params.num_of_collections + (params.extra_collections_size!=0); i++) {
		Renderer::Get()->GetCommandQueue()->ExecuteRenderCommandList(lists[i].GetValue());
	}
};


