#pragma once
#include <Core/RuntimeTag.h>
#include <vector>


template<typename T>
using RenderResourceCollection = std::vector<T>;

template<typename U>
struct RuntimeTag<RenderResourceCollection<U>, void> {

	static constexpr std::string_view GetName() {
		if constexpr (RuntimeTag<U>::GetName() != "Unidentified") {
			static std::string view = (std::string)"RenderResourceCollection_" + RuntimeTag<U>::GetName().data();
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
