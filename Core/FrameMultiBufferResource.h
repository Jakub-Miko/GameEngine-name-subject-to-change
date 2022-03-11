#pragma once 
#include <vector>
#include <FrameManager.h>
#include <utility>
#include <type_traits>

template<typename T>
class FrameMultiBufferResource {
public:

	FrameMultiBufferResource() : resources() {

	}

	FrameMultiBufferResource(FrameMultiBufferResource&& move) : resources(std::move(move.resources)) {

	}

	FrameMultiBufferResource& operator=(FrameMultiBufferResource&& move) {
		resources.clear();
		resources = std::move(move.resources);
		return *this;
	}

	template<typename F, typename dummy = std::enable_if_t<std::is_same_v<decltype(std::declval<F>()()),T>>>
	FrameMultiBufferResource(F func) {
		int num_of_latency_frames = FrameManager::Get()->GetLatencyFrames();
		resources.reserve(num_of_latency_frames);
		for (int i = 0; i < num_of_latency_frames; i++) {
			resources.push_back(func());
		}
	}

	~FrameMultiBufferResource() {
		resources.clear();
	}

	T& GetResource() {
		return resources[(FrameManager::Get()->GetCurrentFrameNumber() % resources.size())];
	}

private:
	std::vector<T> resources;
};