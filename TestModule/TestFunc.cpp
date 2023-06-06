#include "TestFunc.h"
#include <Application.h>
#include <Editor/Editor.h>

extern "C" LIBEXP void CDECL_CALL TestMe()
{
	Editor::Get()->EditorError("Library Loads");
	return;
}
