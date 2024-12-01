#pragma once
#include "IATHook.h"
#include "VTableHook.h"
#include "d3dx8/d3d8.h"
namespace EqWindowed
{
	class EqGFX
	{
	public:
		IATHook hook_Direct3DCreate8;
		IATHook hook_SetWindowPos;
		VTableHook hook_CreateDevice;
		EqGFX(HMODULE handle);
	};
}


