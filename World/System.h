#pragma once
#include <type_traits>
#include <entt/entt.hpp>
#include <Profiler.h>
#include <TaskSystemFence.h>
#include <Renderer/Renderer.h>
#include <World/World.h>
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

static ComponentCollectionParameters GetCollectionsFromSize(size_t size, int num_of_threads, int min_collection_size = 10) {
	if (size == 0) {
		return ComponentCollectionParameters{ 0 ,0,0};
	}	
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
		last = size - ((num_of_chunks - 1) * chunk_size);
		num_of_chunks--;
	}
	return ComponentCollectionParameters{ num_of_chunks ,chunk_size,last };
};

template<typename Component>
using system_view_type = decltype(std::declval<entt::registry>().view<Component>());

template<typename return_type>
using system_return_type = decltype(std::declval<std::vector<Future<return_type*>>>());

template<typename Component, typename return_type, typename system_function, typename result_function>
auto RunSystem(World& world, system_function sys_func, result_function res_func, int min_num_of_tasks_per_thread = 1)
-> decltype((void(),
	sys_func(std::declval<ComponentCollection>(), std::declval<system_view_type<Component>&>(), std::declval<entt::registry*>()),
	res_func(std::declval<ComponentCollectionParameters>(), std::declval<system_return_type<return_type>&>())))
{
	entt::registry& reg = world.GetRegistry();
	auto comps = reg.view<Component>();

	ComponentCollectionParameters params = GetCollectionsFromSize(comps.size(), TaskSystem::Get()->GetProps().num_of_threads + 1, min_num_of_tasks_per_thread);

	std::vector<Future<return_type*>> lists;
	lists.reserve(params.num_of_collections);

	for (int i = 0; i < params.num_of_collections; i++) {
		ComponentCollection comp{ params.collection_size,params.collection_size * i };
		auto task1 = TaskSystem::Get()->CreateTask<return_type*>(sys_func, comp, comps, &reg);
		lists.push_back(task1->GetFuture());
		TaskSystem::Get()->Submit(task1);
	}
	if (params.extra_collections_size != 0) {
		ComponentCollection comp{ params.extra_collections_size, params.collection_size * params.num_of_collections };
		auto task1 = TaskSystem::Get()->CreateTask<return_type*>(sys_func, comp, comps, &reg);
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

	res_func(params, lists);
}


template<typename Component, typename system_function>
auto RunSystemSimple(World& world, system_function sys_func, int min_num_of_tasks_per_thread = 1)
-> decltype((void(),
	sys_func(std::declval<ComponentCollection>(), std::declval<system_view_type<Component>&>(), std::declval<entt::registry*>())))
{
	entt::registry& reg = world.GetRegistry();
	auto comps = reg.view<Component>();

	ComponentCollectionParameters params = GetCollectionsFromSize(comps.size(), TaskSystem::Get()->GetProps().num_of_threads + 1, min_num_of_tasks_per_thread);

	for (int i = 0; i < params.num_of_collections; i++) {
		ComponentCollection comp{ params.collection_size,params.collection_size * i };
		auto task1 = TaskSystem::Get()->CreateTask(sys_func, comp, comps, &reg);
		TaskSystem::Get()->Submit(task1);
	}
	if (params.extra_collections_size != 0) {
		ComponentCollection comp{ params.extra_collections_size, params.collection_size * params.num_of_collections };
		auto task1 = TaskSystem::Get()->CreateTask(sys_func, comp, comps, &reg);
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

}