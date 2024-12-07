#include "pch.h"
#include "EqGFX.h"
#include "EqWindowed.h"
#include "EqMain.h"
#include <iostream>
#include "Console.h"
/*
* eqgfx_dx8.dll is creating the 3d device after the server is selected
*/
namespace EqWindowed
{
	HRESULT WINAPI hCreateDevice(IDirect3D8* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface)
	{
		D3DPRESENT_PARAMETERS LP = *pPresentationParameters;
		LP.Windowed = true;
		LP.hDeviceWindow = Wnd->Handle;
		std::cout << "Create device hook" << std::endl;
		HRESULT result = EqGFXHooks->hook_CreateDevice.original(hCreateDevice)(pD3D, Adapter, DeviceType, Wnd->Handle, BehaviorFlags, &LP, ppReturnedDeviceInterface);
		if (SUCCEEDED(result))
			Wnd->AdjustClientSize(LP.BackBufferWidth, LP.BackBufferHeight);
		return result;

	}
	IDirect3D8* WINAPI hDirect3DCreate8(UINT SDK)
	{
		std::cout << "Direct3DCreate8 called" << std::endl;
		IDirect3D8* rval = EqGFXHooks->hook_Direct3DCreate8.original(hDirect3DCreate8)(SDK);
		void** vtable = *(void***)rval;
		EqGFXHooks->hook_CreateDevice = VTableHook(vtable, 15, hCreateDevice, true);

		return rval;
		
	}

	HCURSOR WINAPI hSetCursor(HCURSOR h)
	{
		std::cout << "Set cursor " << h << std::endl;
		return h;
	}

	EqGFX::EqGFX(HMODULE handle)
	{
		Console::CreateConsole();
		std::cout << "Init d3dx " << std::endl;
		hook_Direct3DCreate8 = IATHook(handle, "d3d8.dll", "Direct3DCreate8", hDirect3DCreate8);
		IATHook h1 = IATHook(handle, "user32.dll", "SetWindowPos", hSetWindowPos);
		IATHook h2 = IATHook(handle, "user32.dll", "SetWindowLongA", hSetWindowLongA);
		IATHook h3 = IATHook(handle, "user32.dll", "SetCapture", hSetCapture);
		IATHook h5 = IATHook(handle, "user32.dll", "SetCursor", hSetCursor);
	}
}