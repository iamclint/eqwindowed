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
		bool need_mousestate_reset = false;
		bool wait_lbutton_release = false;
		ULONGLONG refocused_time = 0;
		LPDIRECTINPUTDEVICE8W keyboard = nullptr;
		LPDIRECTINPUTDEVICE8W mouse = nullptr;
		LPDIRECTINPUT8 dinput = nullptr;
		IATHook hook_DirectInput;
		VTableHook hook_SetCooperativeLevel;
		VTableHook hook_GetDeviceData;
		VTableHook hook_GetDeviceState;
		VTableHook hook_Release;
		VTableHook hook_Acquire;
		VTableHook hook_Unacquire;
		VTableHook hook_CreateDevice;
		VTableHook hook_ReleaseDinput;
		void init(HMODULE handle);
		std::vector<DWORD> key_releases = { 42, 54, 56, 184 };
		void ResetCursorLocation();
		EqDInput();
	};
}