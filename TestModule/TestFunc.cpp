#include "TestFunc.h"
#include <Application.h>
#include <Editor/Editor.h>

extern "C" LIBEXP module_traits CDECL_CALL InitModule() {
	return module_traits{ {"TestTrait1","TestTrait2","TestModular"}};
}

EXPORT_TYPES(TestModular, TestImpl1, TestImpl2);