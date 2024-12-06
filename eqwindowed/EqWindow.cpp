#include "pch.h"
#include "EqWindow.h"
#include "EqWindowed.h"
#include <iostream>
#include "Console.h"
namespace EqWindowed
{
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		Console::CreateConsole();

//		std::cout << "msg: " << msg << " wParam: " << wParam << " lParam: " << lParam << std::endl;
		static bool isCursorHidden = false;
		switch (msg) {
		case WM_SETCURSOR: {
			// Check if the cursor is in the client area
			if (LOWORD(lParam) == HTCLIENT) {
				if (!isCursorHidden) {
					SetCursor(NULL); // Hide the cursor
					isCursorHidden = true; // Update state
				}
				return TRUE; // Indicate that we handled the message
			}
			else {
				if (isCursorHidden) {
					// Restore the default cursor for non-client areas
					SetCursor(LoadCursor(NULL, IDC_ARROW));
					isCursorHidden = false; // Update state
				}
				return DefWindowProcA(hwnd, msg, wParam, lParam);
			}
		}
		case WM_ACTIVATE:
			if (EqMainHooks && EqMainHooks->keyboard)
			{
				if (GetForegroundWindow() == Wnd->Handle)
				{
					EqMainHooks->need_keystate_reset = true;
					EqMainHooks->key_release_index = 0;
					Wnd->isFocused = true;
					std::cout << "Activate" << std::endl;
					HRESULT hr = EqMainHooks->keyboard->Acquire();
					if (FAILED(hr))
					{
						std::cout << "Keyboard Acquire failed " << std::hex << hr << std::endl;
					}
				}
				else
				{

					Wnd->isFocused = false;
					std::cout << "Deactivate" << std::endl;
					EqMainHooks->keyboard->Unacquire();
				}
			}
			break;
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			CloseWindow(Wnd->Handle);
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_SIZE:
		case WM_MOVE:
		{
			Wnd->UpdateClientSize(hwnd);
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_SETFOCUS:
			if (Wnd->eqMainWndProc)
			{
				if (EqMainHooks && EqMainHooks->keyboard)
					EqMainHooks->keyboard->Acquire();
				LRESULT eqm = reinterpret_cast<LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>(Wnd->eqMainWndProc)(hwnd, msg, wParam, lParam);
				break;
			}
		default:
			
			return DefWindowProcA(hwnd, msg, wParam, lParam);
		}
		return 0;
	}
	void EqWindow::UpdateClientSize(HWND hwnd) 
	{
		RECT windowRect, clientRect;

		if (GetWindowRect(hwnd, &windowRect) && GetClientRect(hwnd, &clientRect)) 
		{
			DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
			RECT rect = { 0, 0, EqMain::res.width, EqMain::res.height }; // Client area size
			AdjustWindowRectEx(&rect, dwStyle, FALSE, 0);

			int borderWidth = (windowRect.right - windowRect.left - clientRect.right) / 2;
			int titleBarHeight = (windowRect.bottom - windowRect.top) - clientRect.bottom - borderWidth;
			UINT dpi = GetDpiForWindow(hwnd);  // Available on Windows 10+
			borderWidth = MulDiv(borderWidth, dpi, 96);
			titleBarHeight = MulDiv(titleBarHeight, dpi, 96);
			std::cout << "Border Width: " << std::dec << borderWidth << std::endl;
			std::cout << "Title Bar Height: " << titleBarHeight << std::endl;
			// Client position relative to the window
			X = windowRect.left+ rect.left+(borderWidth*2);
			Y = windowRect.top + titleBarHeight;
			//Width = windowRect.right - windowRect.left;
			//Height = windowRect.bottom - windowRect.top - titleBarHeight;
			std::cout << "X: " << X << " Y: " << Y << " Width: " << Width << " Height: " << Height << std::endl;
		}
	}
	void EqWindow::MessageLoop()
	{
		WNDCLASSA wc = {};
		wc.lpfnWndProc = WndProc;  // Set your window procedure here
		wc.hInstance = GetModuleHandleA(NULL);
		wc.lpszClassName = "EqWindowed";
		wc.style = CS_HREDRAW | CS_VREDRAW;
		if (RegisterClassA(&wc) == 0) {
			// Handle registration failure
			MessageBoxA(NULL, "Window class registration failed!", "Error", MB_OK | MB_ICONERROR);
			return;
		}
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		RECT rect = { 0, 0, EqMain::res.width, EqMain::res.height }; // Client area size
		// Adjust the rect for the specified window style
		AdjustWindowRectEx(&rect, dwStyle, FALSE, 0);
		std::cout << "Width: " << rect.right - rect.left << " Height: " << rect.bottom - rect.top << std::endl;
		Handle = CreateWindowExA(0, "EqWindowed", "EqWindowed", dwStyle, 10, 10, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, NULL, NULL);
		UpdateClientSize(Handle);
		ShowWindow(Handle, SW_SHOW);
		UpdateWindow(Handle);  // This forces the window to be updated and painted
		MSG msg;
		while (GetMessageA(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}
	EqWindow::EqWindow()
	{
		Console::CreateConsole();
		//MessageLoop();
		message_thd = std::thread(&EqWindow::MessageLoop, this);
		message_thd.detach();
	}
}