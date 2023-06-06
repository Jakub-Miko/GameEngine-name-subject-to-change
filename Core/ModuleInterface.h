#pragma once
#include <Core/Defines.h>
#include <string>
#include <vector>

struct module_traits {
	std::vector<std::string> module_trait_list;
};

extern "C" module_traits CDECL_CALL InitModule();