#include "pch.h"
#include "EqWindow.h"
#include <iostream>
#include "Console.h"
namespace EqWindowed
{
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		std::cout << "msg: " << msg << " wParam: " << wParam << " lParam: " << lParam << std::endl;
		switch (msg) {
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
		break;
		default:
			return DefWindowProcA(hwnd, msg, wParam, lParam);
		}
		return 0;
	}
	void EqWindow::MessageLoop()
	{
		MSG msg;
		while (GetMessageA(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}
	EqWindow::EqWindow()
	{
		Console::CreateConsole();
		WNDCLASSA wc = {};
		wc.lpfnWndProc = DefWindowProcA;  // Set your window procedure here
		wc.hInstance = GetModuleHandleA(NULL);
		wc.lpszClassName = "EqWindowed";

		if (RegisterClassA(&wc) == 0) {
			// Handle registration failure
			MessageBoxA(NULL, "Window class registration failed!", "Error", MB_OK | MB_ICONERROR);
			return;
		}
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		Handle = CreateWindowExA(0, "EqWindowed", "EqWindowed", dwStyle, 0, 0, 1024, 768, NULL, NULL, NULL, NULL);
		ShowWindow(Handle, SW_SHOW);
		UpdateWindow(Handle);  // This forces the window to be updated and painted
		message_thd = std::thread(&EqWindow::MessageLoop, this);
		message_thd.detach();
	}
}