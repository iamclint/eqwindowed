#include "pch.h"
#include "EqDInput.h"
#include "EqWindowed.h"
#include <array>
namespace EqWindowed
{
	HRESULT WINAPI hKeyboardSetCooperativeLevel(LPDIRECTINPUTDEVICE8W* device, HWND wnd, DWORD flags)
	{
		std::cout << "Keyboard set cooperative level " << device << " " << wnd << " " << flags << std::endl;
		flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		HRESULT result = DInput->hook_KeyboardSetCooperativeLevel.original(hKeyboardSetCooperativeLevel)(device, wnd, flags);
		return result;
	}
	HRESULT WINAPI hMouseSetCooperativeLevel(LPDIRECTINPUTDEVICE8W* device, HWND wnd, DWORD flags)
	{
		std::cout << "Mouse set cooperative level " << device << " " << wnd << " " << flags << std::endl;
		flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		HRESULT result = DInput->hook_MouseSetCooperativeLevel.original(hMouseSetCooperativeLevel)(device, wnd, flags);
		return result;
	}
	HRESULT WINAPI hMouseGetDeviceState(LPDIRECTINPUTDEVICE8W* device, size_t buffer_size, LPDIDEVICEOBJECTDATA data)
	{
		HRESULT result = DInput->hook_MouseGetDeviceState.original(hMouseGetDeviceState)(device, buffer_size, data);
		if (!Wnd->isFocused || DInput->need_mousestate_reset || GetTickCount64()-DInput->refocused_time<250) //give time to refocus without hitting a click event
		{
				ZeroMemory(data, buffer_size);
		}
		return result;
	}
	HRESULT WINAPI hKeyboardGetDeviceState(LPDIRECTINPUTDEVICE8W* device, size_t buffer_size, LPBYTE  data)
	{
		HRESULT result = DInput->hook_KeyboardGetDeviceState.original(hKeyboardGetDeviceState)(device, buffer_size, data);
		if (!Wnd->isFocused || DInput->need_mousestate_reset)
		{
			ZeroMemory(data, buffer_size);
		}
		return DI_OK;
	}
	HRESULT WINAPI hMouseGetDeviceData(LPDIRECTINPUTDEVICE8W* device, size_t buffer_size, LPDIDEVICEOBJECTDATA data, DWORD* event_count_max, LPUNKNOWN unk)
	{
		int buffer_count = *event_count_max;
		HRESULT result = DInput->hook_MouseGetDeviceData.original(hMouseGetDeviceData)(device, buffer_size, data, event_count_max, unk);
		if (!Wnd->isFocused || DInput->need_mousestate_reset)
		{
			if (*event_count_max > 0)
			{
				ZeroMemory(data, size_t(buffer_size * buffer_count));
				DInput->need_mousestate_reset  = false; // Reset flag
			}
		}
		return result;
	}

	HRESULT WINAPI hKeyboardGetDeviceData(LPDIRECTINPUTDEVICE8W* device, size_t buffer_size, LPDIDEVICEOBJECTDATA data, DWORD* event_count_max, LPUNKNOWN unk)
	{
		int buffer_count = *event_count_max;
		HRESULT result = DInput->hook_KeyboardGetDeviceData.original(hKeyboardGetDeviceData)(device, buffer_size, data, event_count_max, unk);;
		if (DInput->need_keystate_reset || !Wnd->isFocused)
		{
			ZeroMemory(data, size_t(buffer_size * buffer_count));

			DInput->need_keystate_reset = false;
			return DI_OK;
		}

		return result;
	}
	HRESULT WINAPI hMouseAcquire(LPDIRECTINPUTDEVICE8W* device)
	{
		DInput->hook_MouseAcquire.original(hMouseAcquire)(device);
		DInput->need_mousestate_reset = true;
		return DI_OK;
	}
	HRESULT WINAPI hKeyboardAcquire(LPDIRECTINPUTDEVICE8W* device)
	{
		DInput->hook_KeyboardAcquire.original(hKeyboardAcquire)(device);
		return DI_OK;
	}
	HRESULT WINAPI hKeyboardRelease(LPDIRECTINPUTDEVICE8W* device)
	{
		std::cout << "Request Keyboard release" << std::endl;
		return DI_OK;
		//HRESULT  result = DInput->hook_KeyboardRelease.original(hKeyboardRelease)(device);
		//return result;
	}
	HRESULT WINAPI hMouseRelease(LPDIRECTINPUTDEVICE8W* device)
	{
		std::cout << "Request Mouse release" << std::endl;
		return DI_OK;
		//HRESULT result = DInput->hook_KeyboardRelease.original(hKeyboardRelease)(device);
		//return result;
	}
	HRESULT WINAPI hCreateDevice(LPDIRECTINPUT8* ppvOut, GUID& guid, LPDIRECTINPUTDEVICE8W* device, LPUNKNOWN unk)
	{
		HRESULT result = DInput->hook_CreateDevice.original(hCreateDevice)(ppvOut, guid, device, unk);
		if (IsEqualGUID(guid, GUID_SysMouse) ||
			IsEqualGUID(guid, GUID_SysMouseEm) ||
			IsEqualGUID(guid, GUID_SysMouseEm2))
		{
			if (DInput->mouse)
			{
				*device = DInput->mouse;
				DInput->mouse->Acquire();
				return DI_OK;
			}
			else
				DInput->mouse = *device;
			void** vtable = *(void***)(*device);
			DInput->hook_MouseSetCooperativeLevel = VTableHook(vtable, 13, hMouseSetCooperativeLevel);
			DInput->hook_MouseGetDeviceData = VTableHook(vtable, 10, hMouseGetDeviceData);
			DInput->hook_MouseGetDeviceState = VTableHook(vtable, 9, hMouseGetDeviceState);
			DInput->hook_MouseRelease = VTableHook(vtable, 2, hMouseRelease);
			DInput->hook_MouseAcquire = VTableHook(vtable, 7, hMouseAcquire);
			std::cout << "DInput Mouse" << std::endl;
		}
		if (IsEqualGUID(guid, GUID_SysKeyboard))
		{

			if (DInput->keyboard)
			{
				*device = DInput->keyboard;
				return DI_OK;
			}
			else
				DInput->keyboard = *device;
			void** vtable = *(void***)(*device);
			DInput->hook_KeyboardSetCooperativeLevel = VTableHook(vtable, 13, hKeyboardSetCooperativeLevel);
			DInput->hook_KeyboardGetDeviceData = VTableHook(vtable, 10, hKeyboardGetDeviceData);
			DInput->hook_KeyboardGetDeviceState = VTableHook(vtable, 9, hKeyboardGetDeviceState);
			DInput->hook_KeyboardAcquire = VTableHook(vtable, 7, hKeyboardAcquire);
			DInput->hook_KeyboardRelease = VTableHook(vtable, 2, hKeyboardRelease);
			std::cout << "DInput Keyboard" << std::endl;

		}

		return result;

	}

	HRESULT WINAPI hReleaseDinput(LPDIRECTINPUT8* ppvOut)
	{
		std::cout << "Release Dinput" << std::endl;
		return DI_OK;
	}
	HRESULT WINAPI hDirectInputCreate(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPDIRECTINPUT8* ppvOut, LPDIRECTINPUT8 punkOuter)
	{
		std::cout << "Direct Input Hook -- Version: " << dwVersion << std::endl;
		HRESULT res = DI_OK;
		if (!DInput->dinput)
		{
			res = DInput->hook_DirectInput.original(hDirectInputCreate)(hinst, dwVersion, riidltf, ppvOut, punkOuter);
			if (res == DI_OK)
			{
				DInput->dinput = *ppvOut;
				void** vtable = *(void***)(*ppvOut);
				DInput->hook_CreateDevice = VTableHook(vtable, 3, hCreateDevice);
				DInput->hook_ReleaseDinput = VTableHook(vtable, 2, hReleaseDinput);
			}
		}
		else
		{
			std::cout << "Reuse direct input" << std::endl;
			*ppvOut = DInput->dinput;
		}
		return res;

	}
	void EqDInput::init(HMODULE handle)
	{
		if (keyboard)
			keyboard->Release();
		hook_DirectInput = IATHook(handle, "dinput8.dll", "DirectInput8Create", hDirectInputCreate);
	}
	EqDInput::EqDInput()
	{
	}
}