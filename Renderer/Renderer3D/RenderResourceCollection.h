#pragma once
#include <Core/RuntimeTag.h>
#include <vector>

template<typename T>
struct RenderResourceCollection {
	RenderResourceCollection() = default;
	RenderResourceCollection(const RenderResourceCollection& other) : resources(other.resources) {  };
	RenderResourceCollection(RenderResourceCollection&& other) : resources(std::move(other.resources)) {  };
	RenderResourceCollection& operator=(const RenderResourceCollection& other) {
		resources = other.resources;
		return *this;
	}

	RenderResourceCollection& operator=(RenderResourceCollection&& other) {
		resources = std::move(other.resources);
		return *this;
	}

	std::vector<T> resources;
};


template<typename U>
struct RuntimeTag<RenderResourceCollection<U>, void> {

	static constexpr std::string_view GetName() {
		if constexpr (RuntimeTag<U>::GetName() != "Unidentified") {
			std::string view = (std::string)"RenderResourceCollection_" + RuntimeTag<U>::GetName().data();
			return view;
		}
		else {
			return "Unidentified";;
		}
	}

	inline static const RuntimeTagIdType GetId() {
		if constexpr (RuntimeTag<U>::GetName() != "Unidentified") {
			static const RuntimeTagIdType id = SequentialIdGenerator::Id();
			return id;
		}
		else {
			return -1;
		}
	}

};
