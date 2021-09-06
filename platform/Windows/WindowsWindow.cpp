#include "WindowsWindow.h"
#ifdef DirectX12
#include <Windows.h>
#include <Application.h>
#include <string>


LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(window, msg, wparam, lparam);
}


WindowsWindow::WindowsWindow(const Properties& props)
	: Window(props)
{

}

void WindowsWindow::Init() {
    ShowWindow(m_Window, SW_NORMAL);
}

void WindowsWindow::PreInit() {
    
    WNDCLASSEXW window_class;

	HINSTANCE instance = GetModuleHandle(NULL);
	memset(&window_class, '\0', sizeof(window_class));

	window_class.cbSize = sizeof(window_class);
	window_class.lpszClassName = L"Window_Class";
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.lpfnWndProc = WndProc;
	window_class.hCursor = LoadCursor(instance, IDC_ARROW);
	window_class.hIcon = LoadIcon(instance, IDI_WINLOGO);

	RegisterClassExW(&window_class);

	auto name = std::wstring(m_Properties.name.cbegin(), m_Properties.name.cend());

	RECT rect;
	rect.left = rect.top = 100;
	rect.right = m_Properties.resolution_x + 100;
	rect.bottom = m_Properties.resolution_y + 100;
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);


	m_Window = CreateWindowExW(NULL, L"Window_Class", name.c_str(), WS_OVERLAPPEDWINDOW, rect.left, rect.top, 
		rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, instance, nullptr);

}

void WindowsWindow::PollEvents() {
	PROFILE("Poll Events");
	MSG msg = {};
    if(GetMessage(&msg,NULL,0,0)) {
    TranslateMessage(&msg);
	DispatchMessageW(&msg);
    } else {
        Application::Get()->Exit();
    }

}

WindowsWindow::~WindowsWindow() {
    DestroyWindow(m_Window);
    UnregisterClassW(L"Window_Class", GetModuleHandle(NULL));
}

#endif