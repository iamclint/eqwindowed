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



	void EqGFX::ChangeResolution(UINT width, UINT height) {
		if (!device) {
			return; // Ensure the device exists
		}
		//DWORD oldProtection;
		//VirtualProtect((void*)(base + 0x6e885), 10, PAGE_EXECUTE_READWRITE, &oldProtection);
		//*(DWORD*)(base + 0x6e886) = width;
		//*(DWORD*)(base + 0x6e88b) = height;
		//reinterpret_cast<DWORD(__stdcall*)(UINT)>(t3dChangeDeviceResolution)(0x10000);
		//*(DWORD*)(base + 0x6e886) = 640;
		//*(DWORD*)(base + 0x6e88b) = 480;
		//VirtualProtect((void*)(base + 0x6e885), 10, oldProtection, &oldProtection);
		////byte orig_release[5] = { 0xe8, 0xb0, 0x56, 0xff, 0xff };
		//DWORD oldProtection;
		//if (VirtualProtect((void*)(base + 0x6eebb), sizeof(orig_release), PAGE_EXECUTE_READWRITE, &oldProtection))
		//{
		//	memset((void*)(base + 0x6eebb), 0x90, 5);
		//	reinterpret_cast<void(__cdecl*)()>(t3dSwitchD3DVideoMode)();
		//	memcpy((void*)(base + 0x6eebb), orig_release, 5);
		//	VirtualProtect((void*)(base + 0x6eebb), sizeof(orig_release), oldProtection, &oldProtection);
		//}
		
	}
	HRESULT WINAPI hReset(IDirect3DDevice8* Device, D3DPRESENT_PARAMETERS* Parameters)
	{
		HRESULT result = EqGFXHooks->hook_Reset.original(hReset)(Device, Parameters);
		if (SUCCEEDED(result))
		{
			Wnd->SetClientSize(Parameters->BackBufferWidth, Parameters->BackBufferHeight);
			EqMainHooks->backbuffer_resolution.width = Parameters->BackBufferWidth;
			EqMainHooks->backbuffer_resolution.height = Parameters->BackBufferHeight;
		}
		return result;
	}
	HRESULT WINAPI hCreateDevice(IDirect3D8* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface)
	{
		D3DPRESENT_PARAMETERS LP = *pPresentationParameters;
		LP.Windowed = true;
		LP.hDeviceWindow = Wnd->Handle;
		std::cout << "Create device hook " << LP.BackBufferFormat << std::endl;
		HRESULT result = EqGFXHooks->hook_CreateDevice.original(hCreateDevice)(pD3D, Adapter, DeviceType, Wnd->Handle, BehaviorFlags, &LP, ppReturnedDeviceInterface);
		if (SUCCEEDED(result))
		{
			void** vtable = *(void***)*ppReturnedDeviceInterface;
			EqGFXHooks->hook_Reset = VTableHook(vtable, 14, hReset, true);
			Wnd->SetClientSize(LP.BackBufferWidth, LP.BackBufferHeight);
			EqMainHooks->backbuffer_resolution.width = LP.BackBufferWidth;
			EqMainHooks->backbuffer_resolution.height = LP.BackBufferHeight;
			EqGFXHooks->device = *ppReturnedDeviceInterface;
		}
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
		t3dChangeDeviceResolution = (DWORD)GetProcAddress(handle, "t3dChangeDeviceResolution");
		base = DWORD(handle);
		
	}
}