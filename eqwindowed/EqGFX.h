#pragma once
#include "IATHook.h"
#include "VTableHook.h"
#include "d3dx8/d3d8.h"
namespace EqWindowed
{

	struct EqViewPort
	{
		short x;
		short y;
		short width;
		short height;
	};
	class EqGFX
	{
	public:
		DWORD gammaRamp;
		IATHook hook_Direct3DCreate8;
		VTableHook hook_CreateDevice;
		VTableHook hook_Reset;
		VTableHook hook_BeginScene;
		IDirect3DDevice8* device;
		IDirect3DSurface8* surface;
		D3DPRESENT_PARAMETERS present;
		DWORD t3dChangeDeviceResolution;
		DWORD base;
		bool reset_viewport = false;
		void ResetViewport();
		void ChangeResolution(UINT width, UINT height);
		EqGFX(HMODULE handle);
	};
}


