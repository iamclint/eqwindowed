#include "pch.h"
#include "EqWindow.h"
#include "EqWindowed.h"
#include <iostream>
#include "Console.h"
#include "Ini.h"
namespace EqWindowed
{
	std::unordered_map<WindowStyle, DWORD> WindowStyles = {
		{ Windowed,  WS_OVERLAPPEDWINDOW | WS_VISIBLE },
		//{ Maximized, WS_OVERLAPPEDWINDOW | WS_VISIBLE },
		{ MaximizedBorderless, WS_POPUPWINDOW | WS_VISIBLE }
	};
	//struct EQCHARINFO
	//{
	//	/* 0x0000 */ BYTE Unknown0000[2];
	//	/* 0x0002 */ CHAR Name[64]; // [0x40]
	//	/* 0x0042 */ CHAR LastName[70]; // [0x46] ; surname or title
	//};
	//struct Entity
	//{
	//	/* 0x0000 */ BYTE Unknown0000; // always equals 0x03
	//	/* 0x0001 */ CHAR Name[30]; // [0x1E]
	//	/* 0x001F */ BYTE Unknown001F[37];
	//	/* 0x0044 */ DWORD ZoneId; // EQ_ZONE_ID_x
	//	/* 0x0048 */ FLOAT Position[3];
	//	/* 0x0054 */ FLOAT Heading; // camera view left/right, yaw
	//	/* 0x0058 */ FLOAT Unk;
	//	/* 0x005C */ FLOAT MovementSpeed;
	//	/* 0x0060 */ FLOAT MovementSpeedY;
	//	/* 0x0064 */ FLOAT MovementSpeedX;
	//	/* 0x0068 */ FLOAT MovementSpeedZ;
	//	/* 0x006C */ FLOAT MovementSpeedHeading;
	//	/* 0x0070 */ FLOAT Unknown0070;
	//	/* 0x0074 */ FLOAT Pitch; // camera view up/down
	//	/* 0x0078 */ Entity* Prev;
	//	/* 0x007C */ Entity* Next;
	//	/* 0x0080 */ PVOID Unknown0080;
	//	/* 0x0084 */ PVOID ActorInfo;
	//	/* 0x0088 */ EQCHARINFO* CharInfo;

	//};
	//Entity* get_self()
	//{
	//	return *(Entity**)0x7F94CC;
	//}
	void printWindowStyle(WindowStyle style) {
		switch (style) {
		case WindowStyle::Windowed:
			std::cout << "Windowed" << std::endl;
			break;
		case WindowStyle::MaximizedBorderless:
			std::cout << "Maximized Borderless" << std::endl;
			break;
		default:
			std::cout << "Unknown Window Style" << std::endl;
		}
	}
	//void setWindowTitle()
	//{
	//	static ULONGLONG LastSetTime = 0;

	//	if (get_self() && get_self()->CharInfo && GetTickCount64()-LastSetTime>10000)
	//	{
	//		std::string title = "Everquest: " + std::string(get_self()->CharInfo->Name);
	//		SetWindowTextA(Wnd->Handle, title.c_str());
	//		LastSetTime = GetTickCount64();
	//	}
	//}

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		Console::CreateConsole();
	//	setWindowTitle();
		static bool mouse_was_exited = false;
		//std::cout << "msg: " << msg << " wParam: " << wParam << " lParam: " << lParam << std::endl;
		static bool isCursorHidden = false;
		switch (msg) {
		case WM_SETCURSOR: {
			// Check if the cursor is in the client area
			if (LOWORD(lParam) == HTCLIENT) {
				if (!Wnd->isFocused && isCursorHidden)
					DInput->ResetCursorLocation();
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
		case WM_NCACTIVATE:
		{
			DInput->need_keystate_reset = true;
			DInput->need_mousestate_reset = true;
			std::cout << "NCActivate" << std::endl;
			if (Wnd->eqMainWndProc)
			LRESULT eqm = reinterpret_cast<LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>(Wnd->eqMainWndProc)(hwnd, msg, wParam, lParam);
			return DefWindowProcA(hwnd, msg, wParam, lParam);
			break;
		}
		case WM_KILLFOCUS:
		{
			DInput->need_keystate_reset = true;
			DInput->need_mousestate_reset = true;
			Wnd->isFocused = false;
			std::cout << "Deactivate" << std::endl;
			if (DInput)
			{
				if (DInput->mouse && SUCCEEDED(DInput->mouse->Poll()))
					DInput->mouse->Unacquire();
				if (DInput->keyboard && SUCCEEDED(DInput->keyboard->Poll()))
					DInput->keyboard->Unacquire();
			}
			break;
		}
		case WM_SETFOCUS:
		{
			if (DInput)
			{
				DInput->need_keystate_reset = true;
				DInput->need_mousestate_reset = true;
				DInput->refocused_time = GetTickCount64();
				Wnd->isFocused = true;
				std::cout << "Activate" << std::endl;
				if (DInput->mouse && FAILED(DInput->mouse->Poll()))
					DInput->mouse->Acquire();
				if (DInput->keyboard && FAILED(DInput->keyboard->Poll()))
					DInput->keyboard->Acquire();
				break;
			}
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
			Wnd->UpdateClientRegion();
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			if (EqGFXHooks)
			{
				EqGFXHooks->ChangeResolution(Wnd->Width, Wnd->Height);
			}
			break;
		}
		case WM_MOUSEMOVE:
		{
			static POINT leftButtonDownPos = { 0, 0 };
			static POINT rightButtonDownPos = { 0, 0 };
			static bool leftButtonPressed = false;
			static bool rightButtonPressed = false;

			// Check for mouse button state
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) { // Left button is down
				if (!leftButtonPressed) {
					// Store initial cursor position when left button is first pressed
					GetCursorPos(&leftButtonDownPos);
					leftButtonPressed = true;
				}
				else {
					// Reset cursor position to where left button was originally pressed
					SetCursorPos(leftButtonDownPos.x, leftButtonDownPos.y);
				}
			}
			else {
				leftButtonPressed = false; // Left button released
			}

			if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) { // Right button is down
				if (!rightButtonPressed) {
					// Store initial cursor position when right button is first pressed
					GetCursorPos(&rightButtonDownPos);
					rightButtonPressed = true;
				}
				else {
					// Reset cursor position to where right button was originally pressed
					SetCursorPos(rightButtonDownPos.x, rightButtonDownPos.y);
				}
			}
			else 
			{

				rightButtonPressed = false; // Right button released
			}

			if (mouse_was_exited && GetForegroundWindow()==Wnd->Handle) {
				Wnd->isFocused = true;
				mouse_was_exited = false;
				GetCursorPos(&DInput->exit_cursor_pos);
				if (DInput->mouse)
					DInput->mouse->Acquire();
				std::cout << "Cursor Enter pos " << DInput->exit_cursor_pos.x << " " << DInput->exit_cursor_pos.y << std::endl;
			}

			// Register for mouse leave events
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = Wnd->Handle; // The window to track
			TrackMouseEvent(&tme);

			break;


			break;
		}
		case WM_MOUSELEAVE:
		{
			//Wnd->isFocused = false;
			mouse_was_exited = true;
			GetCursorPos(&DInput->exit_cursor_pos);
			std::cout << "Cursor exit pos " << DInput->exit_cursor_pos.x << " " << DInput->exit_cursor_pos.y << std::endl;
			if (DInput->mouse)
				DInput->mouse->Unacquire();
			break;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (wParam == VK_RETURN && (GetAsyncKeyState(VK_MENU) & 0x8000)) {
				// Alt+Enter pressed
				Wnd->dwStyle += 1;
				if (Wnd->dwStyle > WindowStyle::MaximizedBorderless)
					Wnd->dwStyle = WindowStyle::Windowed;
				printWindowStyle((WindowStyle)Wnd->dwStyle);

				DWORD newStyle = WindowStyles[(WindowStyle)Wnd->dwStyle];
				DWORD newExStyle = (Wnd->dwStyle == WindowStyle::MaximizedBorderless) ? WS_EX_TOPMOST : 0;

				// Apply the new styles
				SetWindowLong(Wnd->Handle, GWL_STYLE, newStyle);
				SetWindowLong(Wnd->Handle, GWL_EXSTYLE, newExStyle);

				if ((WindowStyle)Wnd->dwStyle == WindowStyle::MaximizedBorderless)// || (WindowStyle)Wnd->dwStyle == WindowStyle::Maximized)
				{				
					GetWindowRect(Wnd->Handle, &Wnd->storedDimensions);
					SetWindowPos(Wnd->Handle, 0, 0, 0,
						GetSystemMetrics(SM_CXSCREEN),
						GetSystemMetrics(SM_CYSCREEN),
						SWP_FRAMECHANGED);
				}
				else if ((WindowStyle)Wnd->dwStyle == WindowStyle::Windowed) {
					SetWindowPos(Wnd->Handle, 0,
						Wnd->storedDimensions.left,
						Wnd->storedDimensions.top,
						Wnd->storedDimensions.right - Wnd->storedDimensions.left,
						Wnd->storedDimensions.bottom - Wnd->storedDimensions.top,
						SWP_FRAMECHANGED);
				}
				RedrawWindow(Wnd->Handle, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
				EqGFXHooks->reset_viewport=true;
			}
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
	void EqWindow::UpdateClientRegion()
	{
		POINT clientTopLeft = { 0, 0 };

		// Convert the client area point (0, 0) to screen coordinates
		ClientToScreen(Handle, &clientTopLeft);
		X = clientTopLeft.x;
		Y = clientTopLeft.y;

	}
	void EqWindow::SetClientSize(int clientWidth, int clientHeight) {

		std::stringstream ss;
		ss << clientWidth << "by" << clientHeight;
		int x = INI::getValue<int>("Positions", ss.str() + "X", "./eqw.ini");
		int y = INI::getValue<int>("Positions", ss.str() + "Y", "./eqw.ini");
		std::cout << "Set window position: " << x << " " << y << " ini val: " << ss.str() << std::endl;
		SetWindowPos(Handle, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		// Get the current window styles 
		DWORD dwStyle = (DWORD)GetWindowLong(Handle, GWL_STYLE);
		DWORD dwExStyle = (DWORD)GetWindowLong(Handle, GWL_EXSTYLE);

		// Define a RECT with the desired client size
		RECT desiredRect = { 0, 0, clientWidth, clientHeight };

		// Adjust the rectangle to include non-client areas (title bar, borders, etc.)
		AdjustWindowRectEx(&desiredRect, dwStyle, FALSE, dwExStyle);

		// Calculate the width and height including non-client areas
		int windowWidth = desiredRect.right - desiredRect.left;
		int windowHeight = desiredRect.bottom - desiredRect.top;

		// Get the current window position
		RECT currentRect;
		GetWindowRect(Handle, &currentRect);

		// Keep the window's current position
		int xPos = currentRect.left;
		int yPos = currentRect.top;
		

		// Set the window size and position
		SetWindowPos(Handle, NULL, xPos, yPos, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOACTIVATE);
		if (EqMainHooks)
		{
			EqMainHooks->front_resolution.width = clientWidth;
			EqMainHooks->front_resolution.height = clientHeight;
		}
		UpdateClientRegion();
	}

	//void EqWindow::UpdateClientRegionPosition(HWND hwnd) 
	//{
	//	RECT windowRect, clientRect;

	//	if (GetWindowRect(hwnd, &windowRect) && GetClientRect(hwnd, &clientRect)) 
	//	{
	//		
	//		RECT rect;
	//		if (EqMainHooks)
	//			rect = { 0, 0,EqMainHooks->backbuffer_resolution.width, EqMainHooks->backbuffer_resolution.height }; // Client area size
	//		else
	//			rect = { 0, 0, 640, 480 }; // Client area size
	//		std::cout << "UpdateClientRegionPosition" << rect.bottom << "x" << rect.right << std::endl;
	//		AdjustWindowRectEx(&rect, WindowStyles[(WindowStyle)Wnd->dwStyle], FALSE, 0);

	//		int borderWidth = (windowRect.right - windowRect.left - clientRect.right) / 2;
	//		int titleBarHeight = (windowRect.bottom - windowRect.top) - clientRect.bottom - borderWidth;
	//		UINT dpi = GetDpiForWindow(hwnd);  // Available on Windows 10+
	//		borderWidth = MulDiv(borderWidth, dpi, 96);
	//		titleBarHeight = MulDiv(titleBarHeight, dpi, 96);
	//		X = windowRect.left+ rect.left+(borderWidth*2);
	//		Y = windowRect.top + titleBarHeight;
	//		if (EqMainHooks)
	//		{
	//			EqMainHooks->front_resolution.width = clientRect.right - clientRect.left;
	//			EqMainHooks->front_resolution.height = clientRect.bottom - clientRect.top;
	//			if (EqGFXHooks && EqGFXHooks->device)
	//				EqGFXHooks->ChangeResolution(EqMainHooks->front_resolution.width, EqMainHooks->front_resolution.height);
	//			std::cout << std::dec << EqMainHooks->front_resolution.width << "x" << EqMainHooks->front_resolution.height << std::endl;
	//		}
	//	}
	//}
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
		AdjustWindowRectEx(&rect, WindowStyles[(WindowStyle)Wnd->dwStyle], FALSE, 0);

		Handle = CreateWindowExA(0, "EqWindowed", "EqWindowed", WindowStyles[(WindowStyle)Wnd->dwStyle], 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, NULL, NULL);
		SetClientSize(640, 480);
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