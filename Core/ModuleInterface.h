#pragma once
#include <Core/Defines.h>
#include <string>
#include <vector>

struct module_traits {
	std::vector<std::string> module_trait_list;
};

struct module_types {
	std::vector<std::pair<std::string, void*>> types;
};

template<typename T, typename ... Args>
void GetTypes_internal_impl(module_types& types) {
	types.types.push_back(std::make_pair(std::string(RuntimeTag<T>::GetName()), (void*)new T));
	if constexpr(sizeof...(Args) > 0) {
		GetTypes_internal_impl<Args...>(types);
	}
}

template<typename ... Args>
module_types GetTypes_internal() {
	module_types types{};
	GetTypes_internal_impl<Args...>(types);
	return types;
}


extern "C" LIBEXP module_traits CDECL_CALL InitModule();

extern "C" LIBEXP module_types CDECL_CALL GetTypes_dummy();

#define GET_TYPE_FUNCTION(x) extern "C" LIBEXP module_types CDECL_CALL GetTypes_##x()
#define GET_TYPE_FUNCTION_INLINE(x) extern "C" inline LIBEXP module_types CDECL_CALL GetTypes_##x()

#define EXPORT_TYPES(base_types, ...) GET_TYPE_FUNCTION(base_types) {\
	return GetTypes_internal<__VA_ARGS__>();\
}
