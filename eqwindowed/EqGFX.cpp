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
		std::cout << "Create device hook" << std::endl;
		return EqGFXHooks->hook_CreateDevice.original(hCreateDevice)(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	}
	IDirect3D8* WINAPI hDirect3DCreate8(UINT SDK)
	{
		std::cout << "Direct3DCreate8 called" << std::endl;
		IDirect3D8* rval = EqGFXHooks->hook_Direct3DCreate8.original(hDirect3DCreate8)(SDK);
		void** vtable = *(void***)rval;
		EqGFXHooks->hook_CreateDevice = VTableHook(vtable, 15, hCreateDevice, true);

		return rval;
		
	}



	EqGFX::EqGFX(HMODULE handle)
	{
		Console::CreateConsole();
		std::cout << "Init d3dx " << std::endl;
		hook_Direct3DCreate8 = IATHook(handle, "d3d8.dll", "Direct3DCreate8", hDirect3DCreate8);
		hook_SetWindowPos = IATHook(handle, "user32.dll", "SetWindowPos", hSetWindowPos);
	}
}