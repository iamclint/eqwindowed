#include "pch.h"
#include "EqWindow.h"
#include "EqWindowed.h"
#include <iostream>
#include "Console.h"
namespace EqWindowed
{
	void EqWindow::AdjustClientSize(int clientWidth, int clientHeight)
	{
		SetWindowLong(Wnd->Handle, GWL_STYLE, dwStyle);
		RECT rect = { 0, 0, clientWidth, clientHeight }; // Client area size
		AdjustWindowRectEx(&rect, dwStyle, FALSE, 0);
		EqMainHooks->res.width = clientWidth;
		EqMainHooks->res.height = clientHeight;
		std::cout << "AdjustSize " << rect.right - rect.left << " x " << rect.bottom - rect.top << std::endl;
		SetWindowPos(Handle, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

	}
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		Console::CreateConsole();
		static bool mouse_was_exited = false;
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
		{
			if (DInput && DInput->keyboard)
			{
				if (GetForegroundWindow() == Wnd->Handle)
				{
					DInput->need_keystate_reset = true;
					DInput->key_release_index = 0;
					Wnd->isFocused = true;
					std::cout << "Activate" << std::endl;
					if (DInput->mouse)
						DInput->mouse->Acquire();
					DInput->keyboard->Acquire();
				}
				else
				{

					Wnd->isFocused = false;
					std::cout << "Deactivate" << std::endl;
					if (DInput->mouse)
						DInput->mouse->Unacquire();
					DInput->keyboard->Unacquire();
				}
			}
			break;
		}
		case WM_QUIT:
		case WM_DESTROY:
			std::cout << "Close window??" << std::endl;
			//PostQuitMessage(0);
			//CloseWindow(Wnd->Handle);
			return 0;
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
			Wnd->UpdateClientRegionPosition(hwnd);
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (mouse_was_exited)
			{
				mouse_was_exited = false;
				GetCursorPos(&DInput->exit_cursor_pos);
				std::cout << "Cursor Enter pos " << DInput->exit_cursor_pos.x << " " << DInput->exit_cursor_pos.y << std::endl;
			}
			// Register for mouse leave events
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = Wnd->Handle;  // The window to track
			TrackMouseEvent(&tme);


			break;
		}
		case WM_MOUSELEAVE:
		{
			mouse_was_exited = true;
			GetCursorPos(&DInput->exit_cursor_pos);
			std::cout << "Cursor exit pos " << DInput->exit_cursor_pos.x << " " << DInput->exit_cursor_pos.y << std::endl;
			if (DInput->mouse)
				DInput->mouse->Unacquire();
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
				if (DInput && DInput->keyboard)
					DInput->keyboard->Acquire();
				LRESULT eqm = reinterpret_cast<LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>(Wnd->eqMainWndProc)(hwnd, msg, wParam, lParam);
				break;
			}
		default:
			
			return DefWindowProcA(hwnd, msg, wParam, lParam);
		}
		return 0;
	}
	void EqWindow::UpdateClientRegionPosition(HWND hwnd) 
	{
		RECT windowRect, clientRect;

		if (GetWindowRect(hwnd, &windowRect) && GetClientRect(hwnd, &clientRect)) 
		{
			
			RECT rect;
			if (EqMainHooks)
				rect = { 0, 0, EqMainHooks->res.width, EqMainHooks->res.height }; // Client area size
			else
				rect = { 0, 0, 640, 480 }; // Client area size
			std::cout << "UpdateClientRegionPosition" << rect.bottom << "x" << rect.right << std::endl;
			AdjustWindowRectEx(&rect, dwStyle, FALSE, 0);

			int borderWidth = (windowRect.right - windowRect.left - clientRect.right) / 2;
			int titleBarHeight = (windowRect.bottom - windowRect.top) - clientRect.bottom - borderWidth;
			UINT dpi = GetDpiForWindow(hwnd);  // Available on Windows 10+
			borderWidth = MulDiv(borderWidth, dpi, 96);
			titleBarHeight = MulDiv(titleBarHeight, dpi, 96);
			X = windowRect.left+ rect.left+(borderWidth*2);
			Y = windowRect.top + titleBarHeight;
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
		RECT rect = { 0, 0, 640, 480 }; // Client area size
		// Adjust the rect for the specified window style
		AdjustWindowRectEx(&rect, dwStyle, FALSE, 0);

		Handle = CreateWindowExA(0, "EqWindowed", "EqWindowed", dwStyle, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, NULL, NULL);
		UpdateClientRegionPosition(Handle);
		ShowWindow(Handle, SW_SHOW);
		UpdateWindow(Handle);  // This forces the window to be updated and painted
		MSG msg;
		bool running = true;
		while (running)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					std::cout << "WM_QUIT ignored!" << std::endl;
					continue; // Ignore WM_QUIT
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		//while (GetMessageA(&msg, NULL, 0, 0)) {
		//	if (msg.message == WM_QUIT) {
		//		std::cout << "Ignoring WM_QUIT" << std::endl;
		//		continue;
		//	}
		//	TranslateMessage(&msg);
		//	DispatchMessageA(&msg);
		//}
	}
	EqWindow::EqWindow()
	{
		Console::CreateConsole();
		//MessageLoop();
		message_thd = std::thread(&EqWindow::MessageLoop, this);
		message_thd.detach();
	}
}