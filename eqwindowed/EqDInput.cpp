#include "pch.h"
#include "EqDInput.h"
#include "EqWindowed.h"

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

	HRESULT WINAPI hMouseGetDeviceData(LPDIRECTINPUTDEVICE8W* device, size_t buffer_size, LPDIDEVICEOBJECTDATA mdata, DWORD* event_count_max, LPUNKNOWN unk)
	{
		HRESULT result = DInput->hook_MouseGetDeviceData.original(hMouseGetDeviceData)(device, buffer_size, mdata, event_count_max, unk);
		//for (DWORD i = 0; i < *event_count_max; i++) {
		//	DIDEVICEOBJECTDATA data = mdata[i];

		//	std::cout << "Mouse Event " << i + 1 << ": ";

		//	if (data.dwOfs == DIMOFS_X) {
		//		std::cout << "Move X: " << data.dwData << std::endl;
		//	}
		//	else if (data.dwOfs == DIMOFS_Y) {
		//		std::cout << "Move Y: " << data.dwData << std::endl;
		//	}
		//	else if (data.dwOfs == DIMOFS_BUTTON0) {
		//		std::cout << "Left Button: " << (data.dwData ? "Pressed" : "Released") << std::endl;
		//	}
		//	else if (data.dwOfs == DIMOFS_BUTTON1) {
		//		std::cout << "Right Button: " << (data.dwData ? "Pressed" : "Released") << std::endl;
		//	}
		//	else if (data.dwOfs == DIMOFS_BUTTON2) {
		//		std::cout << "Middle Button: " << (data.dwData ? "Pressed" : "Released") << std::endl;
		//	}
		//}

		return result;
	}

	HRESULT WINAPI hKeyboardGetDeviceData(LPDIRECTINPUTDEVICE8W* device, size_t buffer_size, LPDIDEVICEOBJECTDATA data, DWORD* event_count_max, LPUNKNOWN unk)
	{
		HRESULT result = DInput->hook_KeyboardGetDeviceData.original(hKeyboardGetDeviceData)(device, buffer_size, data, event_count_max, unk);
		//if (SUCCEEDED(result))
		//{
		//	for (DWORD i = 0; i < *event_count_max; ++i)
		//	{
		//		// Process each event
		//		if (data[i].dwData & 0x80)
		//		{
		//			// Key was pressed
		//			printf("Key pressed: %d\n", data[i].dwOfs);
		//		}
		//		else
		//		{
		//			// Key was released
		//			printf("Key released: %d\n", data[i].dwOfs);
		//		}
		//	}
		//}

		if (DInput->need_keystate_reset)
		{
			*event_count_max = 1;
			data[0].dwData = 0;
			data[0].dwOfs = DInput->key_releases.at(DInput->key_release_index);
			std::cout << "Release key: " << data[0].dwOfs << std::endl;
			result = S_OK;
			DInput->key_release_index++;
			if (DInput->key_release_index >= DInput->key_releases.size())
				DInput->need_keystate_reset = false;
		}


		return result;
	}
	HRESULT WINAPI hKeyboardAcquire(LPDIRECTINPUTDEVICE8W* device)
	{
		std::cout << "Request Keyboard Acquire" << std::endl;
		HRESULT result = S_OK;
		if (Wnd->isFocused)
			result = DInput->hook_KeyboardAcquire.original(hKeyboardAcquire)(device);
		return result;
	}
	HRESULT WINAPI hKeyboardRelease(LPDIRECTINPUTDEVICE8W* device)
	{
		std::cout << "Request Keyboard release" << std::endl;
		return S_OK;
		//HRESULT  result = DInput->hook_KeyboardRelease.original(hKeyboardRelease)(device);
		//return result;
	}
	HRESULT WINAPI hMouseRelease(LPDIRECTINPUTDEVICE8W* device)
	{
		std::cout << "Request Mouse release" << std::endl;
		return S_OK;
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
				return S_OK;
			}
			else
				DInput->mouse = *device;
			void** vtable = *(void***)(*device);
			DInput->hook_MouseSetCooperativeLevel = VTableHook(vtable, 13, hMouseSetCooperativeLevel);
			DInput->hook_MouseGetDeviceData = VTableHook(vtable, 10, hMouseGetDeviceData);
			DInput->hook_MouseRelease = VTableHook(vtable, 2, hMouseRelease);
			std::cout << "DInput Mouse" << std::endl;
		}
		if (IsEqualGUID(guid, GUID_SysKeyboard))
		{

			if (DInput->keyboard)
			{
				*device = DInput->keyboard;
				return S_OK;
			}
			else
				DInput->keyboard = *device;
			void** vtable = *(void***)(*device);
			DInput->hook_KeyboardSetCooperativeLevel = VTableHook(vtable, 13, hKeyboardSetCooperativeLevel);
			DInput->hook_KeyboardGetDeviceData = VTableHook(vtable, 10, hKeyboardGetDeviceData);
			DInput->hook_KeyboardAcquire = VTableHook(vtable, 7, hKeyboardAcquire);
			DInput->hook_KeyboardRelease = VTableHook(vtable, 2, hKeyboardRelease);
			std::cout << "DInput Keyboard" << std::endl;

		}

		return result;

	}

	HRESULT WINAPI hReleaseDinput(LPDIRECTINPUT8* ppvOut)
	{
		std::cout << "Release Dinput" << std::endl;
		return S_OK;
	}
	HRESULT WINAPI hDirectInputCreate(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPDIRECTINPUT8* ppvOut, LPDIRECTINPUT8 punkOuter)
	{
		std::cout << "Direct Input Hook -- Version: " << dwVersion << std::endl;
		HRESULT res = S_OK;
		if (!DInput->dinput)
		{
			res = DInput->hook_DirectInput.original(hDirectInputCreate)(hinst, dwVersion, riidltf, ppvOut, punkOuter);
			if (res == S_OK)
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