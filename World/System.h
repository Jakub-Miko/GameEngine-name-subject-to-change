#pragma once
#include <type_traits>
#include <entt/entt.hpp>
#include <Profiler.h>
#include <TaskSystemFence.h>
#include <Renderer/Renderer.h>
#include <World/World.h>
#include <Application.h>
#include <TaskSystem.h>
#include <cmath>

/**
 * @brief Describes how Entities should be distributed among threads/batches, when submitting them to the TaskSystem
*/
struct ComponentCollectionParameters {
	int num_of_collections; ///< number of full sized batches
	int collection_size; ///< size of full sized batches
	int extra_collections_size; ///< size of the extra leftover batch
};

/**
 * @brief Describes a single batch from the batches submitted to the Task system
*/
struct ComponentCollection {
	int size; ///< number of entities in this batch
	int start_index; ///< index of the first entity this batch should process, it is and index into the system_view_type
};

/**
 * @brief Uses the number of entities and threads to produce a ComponentCollectionParameters distribution of the entities into batches
 * @param size number of entities to be processed
 * @param num_of_threads number of threads to distribute the batches over
 * @param min_collection_size minimal number of collections to prevent small amount of entities to be needlessly distributes(since the synchronization overhead would be higher)
 * @return ComponentCollectionParameters containing the batch distribution
*/
static ComponentCollectionParameters GetCollectionsFromSize(size_t size, int num_of_threads, int min_collection_size = 10) {
	if (size == 0) {
		return ComponentCollectionParameters{ 0 ,0,0};
	}	
	int chunk_size = int(std::ceil((float)size / (float)num_of_threads));
	int num_of_chunks;
	int last;
	if (chunk_size >= min_collection_size) {
		num_of_chunks = int(std::ceil((float)size / (float)chunk_size));
	}
	else {
		num_of_chunks = int(std::ceil((float)size / (float)min_collection_size));
		chunk_size = min_collection_size;
	}
	if ((float)size / (float)num_of_chunks == (float)chunk_size) {
		last = 0;
	}
	else {
		last = (int)size - ((num_of_chunks - 1) * chunk_size);
		num_of_chunks--;
	}
	return ComponentCollectionParameters{ num_of_chunks ,chunk_size,last };
};

/**
 * @brief type of an iterator containing all entities with a component
 * @tparam Component component the entities should have
*/
template<typename Component>
using system_view_type = decltype(std::declval<entt::registry>().view<Component>());

/**
 * @brief type of an agrregation of Futures holding future values of individual tasks.
 * @tparam return_type return type expected of each individual task 
*/
template<typename return_type>
using system_return_type = decltype(std::declval<std::vector<Future<return_type>>>());

/**
 * @brief Executes a system function on each entity with a component in parallel
 * 
 * Template function which abstract complexity of distributing and dispatching a multi-threads ECS system. 
 * Entities with Component are distributed and processed using the sys_func in parallel using the TaskSystem and their results are then aggregated by a res_func.
 * 
 * @tparam Component type of a component which iterated entities need to have
 * @tparam return_type type which sys_func task returns
 * @tparam system_function type of sys_func 
 * @tparam result_function type of res_func
 * @param world World from which entities are taken
 * @param sys_func task function which processes entities and returns return_type, takes ComponentCollection, system_view_type<Component> and a entt::registry pointer as arguments
 * @param res_func task which takes the system_return_type aggregation of all values return by sys_func tasks and processes them as a vector,
 * takes ComponentCollectionParameters and system_return_type<return_type> reference as arguments
 * @param min_num_of_tasks_per_thread minimal number of tasks which are going to be dispatched on a thread to prevernt overhead for small number of entities
*/
template<typename Component, typename return_type, typename system_function, typename result_function>
auto RunSystem(World& world, system_function sys_func, result_function res_func, int min_num_of_tasks_per_thread = 1)
-> decltype((void(),
	sys_func(std::declval<ComponentCollection>(), std::declval<system_view_type<Component>&>(), std::declval<entt::registry*>()),
	res_func(std::declval<ComponentCollectionParameters>(), std::declval<system_return_type<return_type>&>())))
{
	entt::registry& reg = world.GetRegistry();
	auto comps = reg.view<Component>();

	auto dispatcher = Application::GetAsyncDispather();
	bool include_async_thread = !dispatcher->IsRunning();

	if (comps.size() == 0) return;

	ComponentCollectionParameters params = GetCollectionsFromSize(comps.size(), TaskSystem::Get()->GetProps().num_of_threads + (include_async_thread ? 2 : 1), min_num_of_tasks_per_thread);

	std::vector<Future<return_type>> lists;
	lists.reserve(params.num_of_collections);

	for (int i = 0; i < params.num_of_collections; i++) {
		ComponentCollection comp{ params.collection_size,params.collection_size * i };
		auto task1 = TaskSystem::Get()->CreateTask<return_type>(sys_func, comp, comps, &reg);
		lists.push_back(task1->GetFuture());
		TaskSystem::Get()->Submit(task1);
	}
	if (params.extra_collections_size != 0) {
		ComponentCollection comp{ params.extra_collections_size, params.collection_size * params.num_of_collections };
		auto task1 = TaskSystem::Get()->CreateTask<return_type>(sys_func, comp, comps, &reg);
		lists.push_back(task1->GetFuture());
		TaskSystem::Get()->Submit(task1);
	}

	std::shared_ptr<TaskSystemFence> fence(new TaskSystemFence);
	auto task5 = [fence]() {
		fence->Signal(1);
		TaskSystem::Get()->FlushLoop();
	};
	TaskSystem::Get()->SetIdleTask(TaskSystem::Get()->CreateTask(task5));

	if (include_async_thread) {
		auto dispatch_task = dispatcher->CreateTask<void>([fence]() {
			TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
				return fence->IsValue(1);
				});
			TaskSystem::Get()->FlushLoop();
			});

		dispatcher->Submit(dispatch_task);
	}


	TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
		return fence->IsValue(1);
		});

	res_func(params, lists);
}

/**
 * @brief Executes a system function on each entity with a component in parallel without aggregating the task output
 *
 * Template function which abstract complexity of distributing and dispatching a multi-threads ECS system.
 * Entities with Component are distributed and processed using the sys_func in parallel using the TaskSystem. 
 * This version doesn not process task aggregate output.
 *
 * @tparam Component type of a component which iterated entities need to have
 * @tparam system_function type of sys_func
 * @param world World from which entities are taken
 * @param sys_func task function which processes entities, takes ComponentCollection, system_view_type<Component> and a entt::registry pointer as arguments
 * @param min_num_of_tasks_per_thread minimal number of tasks which are going to be dispatched on a thread to prevernt overhead for small number of entities
*/
template<typename Component, typename system_function>
auto RunSystemSimple(World& world, system_function sys_func, int min_num_of_tasks_per_thread = 1)
-> decltype((void(),
	sys_func(std::declval<ComponentCollection>(), std::declval<system_view_type<Component>&>(), std::declval<entt::registry*>())))
{
	entt::registry& reg = world.GetRegistry();
	auto comps = reg.view<Component>();
	auto dispatcher = Application::GetAsyncDispather();
	bool include_async_thread = !dispatcher->IsRunning();

	if (comps.size() == 0) return;

	ComponentCollectionParameters params = GetCollectionsFromSize(comps.size(), TaskSystem::Get()->GetProps().num_of_threads + (include_async_thread ? 2 : 1), min_num_of_tasks_per_thread);

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

	std::shared_ptr<TaskSystemFence> fence(new TaskSystemFence);
	auto task5 = [fence]() {
		fence->Signal(1); 
		TaskSystem::Get()->FlushLoop();
	};
	TaskSystem::Get()->SetIdleTask(TaskSystem::Get()->CreateTask(task5));


	if (include_async_thread) {
	auto dispatch_task = dispatcher->CreateTask<void>([fence]() {
		TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
			return fence->IsValue(1);
			});
		TaskSystem::Get()->FlushLoop();
		});

		dispatcher->Submit(dispatch_task);
	}

	TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
		return fence->IsValue(1);
		});

}


/**
 * @brief Executes a system function on each element within a queue in parallel without aggregating the task output
 *
 * Template function which abstract complexity of distributing and dispatching a multi-threads ECS system.
 * Elements in a queue are distributed and processed using the sys_func in parallel using the TaskSystem.
 * This version doesn not process task aggregate output.
 *
 * @tparam Queue type of element the queue contains and this function processes
 * @tparam system_function type of sys_func
 * @param world Reference of the current world
 * @param sys_func task function which processes elements, takes ComponentCollection and std::deque<Queue> reference as arguments
 * @param min_num_of_tasks_per_thread minimal number of tasks which are going to be dispatched on a thread to prevent overhead for small number of elements
*/
template<typename Queue, typename system_function>
auto RunSystemSimpleQueue(World& world,std::deque<Queue>& queue, system_function sys_func, int min_num_of_tasks_per_thread = 1)
-> decltype((void(),
	sys_func(std::declval<ComponentCollection>(), std::declval<std::deque<Queue>&>())))
{
	auto dispatcher = Application::GetAsyncDispather();
	bool include_async_thread = !dispatcher->IsRunning();

	if (queue.size() == 0) return;

	ComponentCollectionParameters params = GetCollectionsFromSize(queue.size(), TaskSystem::Get()->GetProps().num_of_threads + (include_async_thread ? 2 : 1), min_num_of_tasks_per_thread);

	for (int i = 0; i < params.num_of_collections; i++) {
		ComponentCollection comp{ params.collection_size,params.collection_size * i };
		auto task1 = TaskSystem::Get()->CreateTask(sys_func, comp, queue);
		TaskSystem::Get()->Submit(task1);
	}
	if (params.extra_collections_size != 0) {
		ComponentCollection comp{ params.extra_collections_size, params.collection_size * params.num_of_collections };
		auto task1 = TaskSystem::Get()->CreateTask(sys_func, comp, queue);
		TaskSystem::Get()->Submit(task1);
	}

	std::shared_ptr<TaskSystemFence> fence(new TaskSystemFence);
	auto task5 = [fence]() {
		fence->Signal(1);
		TaskSystem::Get()->FlushLoop();
	};
	TaskSystem::Get()->SetIdleTask(TaskSystem::Get()->CreateTask(task5));

	if (include_async_thread) {
		auto dispatch_task = dispatcher->CreateTask<void>([fence]() {
			TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
				return fence->IsValue(1);
				});
			TaskSystem::Get()->FlushLoop();
			});

		dispatcher->Submit(dispatch_task);
	}


	TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
		return fence->IsValue(1);
		});

}

/**
 * @brief Executes a system function on each element within a vector in parallel without aggregating the task output
 *
 * Template function which abstract complexity of distributing and dispatching a multi-threads ECS system.
 * Elements in a vector are distributed and processed using the sys_func in parallel using the TaskSystem.
 * This version doesn not process task aggregate output.
 *
 * @tparam Vector type of element the vector contains and this function processes
 * @tparam system_function type of sys_func
 * @param world Reference of the current world
 * @param sys_func task function which processes elements, takes ComponentCollection and std::vector<Vector> reference as arguments
 * @param min_num_of_tasks_per_thread minimal number of tasks which are going to be dispatched on a thread to prevent overhead for small number of elements
*/
template<typename Vector, typename system_function>
auto RunSystemSimpleVector(World& world, const std::vector<Vector>& vector, system_function sys_func, int min_num_of_tasks_per_thread = 1)
-> decltype((void(),
	sys_func(std::declval<ComponentCollection>(), std::declval<const std::vector<Vector>&>())))
{
	auto dispatcher = Application::GetAsyncDispather();
	bool include_async_thread = !dispatcher->IsRunning();

	if (vector.size() == 0) return;

	ComponentCollectionParameters params = GetCollectionsFromSize(vector.size(), TaskSystem::Get()->GetProps().num_of_threads + (include_async_thread ? 2 : 1), min_num_of_tasks_per_thread);

	for (int i = 0; i < params.num_of_collections; i++) {
		ComponentCollection comp{ params.collection_size,params.collection_size * i };
		auto task1 = TaskSystem::Get()->CreateTask(sys_func, comp, vector);
		TaskSystem::Get()->Submit(task1);
	}
	if (params.extra_collections_size != 0) {
		ComponentCollection comp{ params.extra_collections_size, params.collection_size * params.num_of_collections };
		auto task1 = TaskSystem::Get()->CreateTask(sys_func, comp, vector);
		TaskSystem::Get()->Submit(task1);
	}

	std::shared_ptr<TaskSystemFence> fence(new TaskSystemFence);
	auto task5 = [fence]() {
		fence->Signal(1);
		TaskSystem::Get()->FlushLoop();
	};
	TaskSystem::Get()->SetIdleTask(TaskSystem::Get()->CreateTask(task5));

	if (include_async_thread) {
		auto dispatch_task = dispatcher->CreateTask<void>([fence]() {
			TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
				return fence->IsValue(1);
				});
			TaskSystem::Get()->FlushLoop();
			});

		dispatcher->Submit(dispatch_task);
	}

	TaskSystem::Get()->JoinTaskSystem([fence]() -> bool {
		return fence->IsValue(1);
		});

}
