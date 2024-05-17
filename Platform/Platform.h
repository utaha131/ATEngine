#ifndef _AT_PLATFORM_H_
#define _AT_PLATFORM_H_
#ifdef _WIN32
	#include "./Win32/Win32Window.h"
	extern HINSTANCE app_instance;
#else
#endif


namespace AT {
	namespace Platform {

		inline void CreatePlatformWindow(const WindowDescription& description, IWindow*& p_window) {
#ifdef _WIN32
			WNDCLASS window_class = {};
			window_class.style = CS_HREDRAW | CS_VREDRAW;
			window_class.lpfnWndProc = MainWndProc;
			window_class.cbClsExtra = 0;
			window_class.cbWndExtra = 0;
			window_class.hInstance = app_instance;
			window_class.hIcon = LoadIcon(0, IDI_APPLICATION);
			window_class.hCursor = LoadCursor(0, IDC_ARROW);
			window_class.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
			window_class.lpszMenuName = 0;
			window_class.lpszClassName = L"MainWindow";

			if (!RegisterClass(&window_class)) {

			}

			RECT window_rect = { 0, 0, description.Width, description.Height };
			AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false);
			int width = window_rect.right - window_rect.left;
			int height = window_rect.bottom - window_rect.top;

			HWND hwnd = CreateWindow(L"MainWindow", std::wstring(description.Title.begin(), description.Title.end()).c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, app_instance, 0);

			if (!hwnd) {
				printf("failed to initialize window.");
			}
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
			p_window = new Win32Window(description, hwnd);
#else
#endif
		}
	}
}
#endif