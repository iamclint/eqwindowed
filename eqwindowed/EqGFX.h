#pragma once
#include "IATHook.h"
#include "VTableHook.h"
#include "d3dx8/d3d8.h"
namespace EqWindowed
{
	class EqGFX
	{
	public:
		DWORD gammaRamp;
		IATHook hook_Direct3DCreate8;
		VTableHook hook_CreateDevice;
		VTableHook hook_Reset;
		IDirect3DDevice8* device;
		IDirect3DSurface8* surface;
		DWORD t3dChangeDeviceResolution;
		DWORD base;
		void ChangeResolution(UINT width, UINT height);
		EqGFX(HMODULE handle);
	};
}


