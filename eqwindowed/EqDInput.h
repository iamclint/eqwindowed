#pragma once
#define DIRECTINPUT_VERSION 0x0800
#define INITGUID
#include "EqWindow.h"
#include "IATHook.h"
#include "VTableHook.h"
#include <ddraw.h>
#include <dinput.h>
#include <vector>
#include <unordered_map>
namespace EqWindowed
{
	class EqDInput
	{
	public:
		POINT exit_cursor_pos;
		int key_release_index = 0;
		bool need_keystate_reset = false;
		LPDIRECTINPUTDEVICE8W keyboard = nullptr;
		LPDIRECTINPUTDEVICE8W mouse = nullptr;
		LPDIRECTINPUT8 dinput = nullptr;
		IATHook hook_DirectInput;
		VTableHook hook_KeyboardSetCooperativeLevel;
		VTableHook hook_MouseSetCooperativeLevel;
		VTableHook hook_KeyboardGetDeviceData;
		VTableHook hook_KeyboardAcquire;
		VTableHook hook_KeyboardRelease;
		VTableHook hook_MouseGetDeviceData;
		VTableHook hook_MouseRelease;
		VTableHook hook_CreateDevice;
		VTableHook hook_ReleaseDinput;
		void init(HMODULE handle);
		std::vector<DWORD> key_releases = { 42, 54, 56, 184 };
		EqDInput();
	};
}