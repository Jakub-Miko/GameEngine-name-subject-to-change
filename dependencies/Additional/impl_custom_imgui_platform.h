#pragma once 
class ImGuiViewport;
class impl_custom_imgui_platform {
public:

	static void init_custom_imgui_platform();
	static void shutdown_custom_imgui_platform();

	static void UpdatePlatformWindows();
private:
	static void ImGui_custom_CreateWindow(ImGuiViewport* viewport);
		
};