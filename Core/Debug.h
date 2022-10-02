#pragma once

#ifdef EDITOR
#include <Editor/Editor.h>
#define EDITOR_ERROR(text) Editor::Get()->EditorError(text);
#else 
#define EDITOR_ERROR(text)
#endif