#include "WindowsWindow.h"
#ifdef WIN32Window
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

	const wchar_t* name = L"Window";

	m_Window = CreateWindowExW(NULL, L"Window_Class", name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 
		CW_USEDEFAULT, 1920, 1080, nullptr, nullptr, instance, nullptr);

}

void WindowsWindow::PollEvents() {
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