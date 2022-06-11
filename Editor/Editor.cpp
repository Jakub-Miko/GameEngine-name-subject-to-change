#include "Editor.h"
#include <dependencies/imgui/imgui.h>
#include <dependencies/Additional/impl_custom_imgui_backend.h>
#include <dependencies/imgui/backends/imgui_impl_glfw.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>

Editor* Editor::instance = nullptr;

void Editor::Init()
{
	if (!instance) {
		instance = new Editor;
	}
}

void Editor::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

void Editor::Run()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	ImGui::Render();
	impl_custom_imgui_backend::DrawData(ImGui::GetDrawData());
}

void Editor::Render()
{
}

Editor* Editor::Get()
{
	return instance;
}

Editor::Editor()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io_1 = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOther(static_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), true);
	impl_custom_imgui_backend::Init();

	io = &io_1;


	
}

Editor::~Editor()
{

	impl_custom_imgui_backend::Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
