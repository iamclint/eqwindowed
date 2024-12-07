#include "pch.h"
#include "EqWindowed.h"
#include "EqMain.h"
#include <iostream>
#include "Console.h"

/*
* eqmain.dll is creating the directdraw surface for the login/server select screen
*/

namespace EqWindowed
{
	HRESULT WINAPI hCreateSurface(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR* lplpDDSurface, IUnknown FAR* pUnkOuter);

	void* GetCallerAddress() {
		void* callerAddress;
		__asm {
			mov eax, [ebp + 4]   // Load the return address (caller address) into EAX
			mov callerAddress, eax // Store it in callerAddress
		}
		return callerAddress;
	}

    HRESULT WINAPI hDirectDrawCreate(GUID FAR* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown FAR* pUnkOuter)
    {
		std::cout << "DirectDraw Create " << std::hex << lpGUID << " " << lplpDD << " " << pUnkOuter << std::endl;
		HRESULT rval = EqMainHooks->hook_CreateDirectDraw.original(hDirectDrawCreate)(lpGUID, lplpDD, pUnkOuter);
		if (lplpDD)
		{
			EqMainHooks->dd = *lplpDD;
			EqMainHooks->InitDDraw((*lplpDD));
		}
		return rval;
    }
	
	HRESULT WINAPI hSetCooperativeLevel(IDirectDraw* lplpDD, HWND hWnd, DWORD dwFlags) 
	{
		std::cout << "Set cooperative level " << hWnd  << " Flags: " << dwFlags << std::endl;
		HRESULT res = EqMainHooks->hook_SetCooperativeLevel.original(hSetCooperativeLevel)(lplpDD, hWnd, DDSCL_NORMAL);
		return res;
	}

	HRESULT WINAPI hSetDisplayMode(IDirectDraw* lplpDD, DWORD dwWidth, DWORD dwHeight, DWORD dwBpp) 
	{
		/*std::cout << "Set Display Mode Width: " << std::dec << dwWidth << " Height: " << dwHeight << " dwBPP " << dwBpp << std::endl;
		dwWidth = EqMainHooks->res.width;
		dwHeight = EqMainHooks->res.height;
		dwBpp = 16;
		return EqMainHooks->hook_SetDisplayMode.original(hSetDisplayMode)(lplpDD, dwWidth, dwHeight, dwBpp);*/
		std::cout << "SetDisplayMode " << dwWidth << "x" << dwHeight << std::endl;
		return DD_OK;
	}

	HRESULT WINAPI hFlip(IDirectDrawSurface* surface, IDirectDrawSurface* surface2, DWORD flags)
	{
		HRESULT result = EqMainHooks->dd->WaitForVerticalBlank(1, Wnd->Handle);
		if (!(EqMainHooks->SecondarySurface->IsLost()==DDERR_SURFACELOST) && !(EqMainHooks->PrimarySurface->IsLost() == DDERR_SURFACELOST))
		{

			RECT srcRect = { 0, 0, EqMainHooks->res.width,EqMainHooks->res.height }; // Source rectangle
			RECT destRect = { Wnd->X, Wnd->Y,Wnd->X+EqMainHooks->res.width , Wnd->Y+EqMainHooks->res.height};// Wnd->Width + Wnd->X, Wnd->Height + Wnd->Y

			HRESULT result = surface->Blt(&destRect, EqMainHooks->SecondarySurface, &srcRect, DDBLT_WAIT, nullptr);
			*surface = *EqMainHooks->SecondarySurface;
		}
		return result;
			
	}
	HRESULT WINAPI hGetAttachedSurface(IDirectDrawSurface* surface, DDSCAPS* caps, LPDIRECTDRAWSURFACE* backbuffer)
	{
		if (surface == EqMainHooks->PrimarySurface) 
		{
			std::cout << "GetAttachedSurface Primary" << std::endl;
			*backbuffer = EqMainHooks->SecondarySurface;
			return DD_OK;
		}
		HRESULT result =  EqMainHooks->hook_GetAttachedSurface.original(hGetAttachedSurface)(surface, caps, backbuffer);
		if (!SUCCEEDED(result))
			std::cout << "GetAttachedSurface Failed HRESULT: " << std::hex << (DWORD)result << std::endl;
		else
			std::cout << "GetAttachedSurface Succeeded " << std::endl;
		return result;
	}

	void SetPixelFormat(DDPIXELFORMAT& data)
	{
		ZeroMemory(&data, sizeof(DDPIXELFORMAT));
		data.dwSize = sizeof(DDPIXELFORMAT);
		data.dwFlags = DDPF_RGB;
		data.dwRGBBitCount = 16; // Match primary surface bit depth
		data.dwRBitMask = 0xF800;  // 5 bits for Red (RGB 5-6-5)
		data.dwGBitMask = 0x07E0;  // 6 bits for Green (RGB 5-6-5)
		data.dwBBitMask = 0x001F;  // 5 bits for Blue (RGB 5-6-5)
	}
	HRESULT WINAPI hQueryInterface(LPDIRECTINPUT8* dinput, REFIID riid, LPVOID FAR* ppvObj)
	{
		std::cout << "Query Interface" << std::endl;
		HRESULT result = EqMainHooks->hook_QueryInterface.original(hQueryInterface)(dinput, riid, ppvObj);
		if (SUCCEEDED(result))
		{

		}
		return result;
	}
	void HookSurfaceVtable(IDirectDrawSurface* surface)
	{
		void** vtable = *(void***)(surface);
		EqMainHooks->hook_GetAttachedSurface = VTableHook(vtable, 12, hGetAttachedSurface);
		EqMainHooks->hook_Flip = VTableHook(vtable, 11, hFlip);
		//if (!EqMainHooks->hook_QueryInterface.orig_function)
		//	EqMainHooks->hook_QueryInterface = VTableHook(vtable, 0, hQueryInterface);
		//else
		//	VTableHook(vtable, 0, hQueryInterface);
	}

	/*
	* This function I am just fixing up the surfacedesc to play nice with windowed mode
	*/
	HRESULT CreatePrimarySurface(IDirectDraw* lplpDD)
	{
		DDSURFACEDESC surface_desc;
		ZeroMemory(&surface_desc, sizeof(DDSURFACEDESC));
		surface_desc.dwSize = sizeof(DDSURFACEDESC);
		surface_desc.dwFlags = DDSD_CAPS;
		surface_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		SetPixelFormat(surface_desc.ddpfPixelFormat);
		HRESULT result =  EqMainHooks->hook_CreateSurface.original(hCreateSurface)(lplpDD, &surface_desc, &EqMainHooks->PrimarySurface, NULL); //create the primary surface

		if (!SUCCEEDED(result))
		{
			std::cout << "Primary Surface Creation Failed with HRESULT: " << std::hex << (DWORD)result << std::endl;
			return result;
		}

		return result;
	}


	/*
	* Attempting to create a backbuffer so getattachedsurface doesn't fail, eqmain.dll expects this function to work to get the backbuffer
	*/
	HRESULT CreateSecondarySurface(IDirectDraw* lplpDD)
	{
		DDSURFACEDESC surface_desc;
		ZeroMemory(&surface_desc, sizeof(DDSURFACEDESC));
		surface_desc.dwSize = sizeof(DDSURFACEDESC);
		surface_desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		surface_desc.dwWidth = EqMainHooks->res.width;  // Match the primary surface resolution
		surface_desc.dwHeight = EqMainHooks->res.height; // Match the primary surface resolution
		SetPixelFormat(surface_desc.ddpfPixelFormat);
		HRESULT result = EqMainHooks->hook_CreateSurface.original(hCreateSurface)(lplpDD, &surface_desc, &EqMainHooks->SecondarySurface, NULL);
		if (!SUCCEEDED(result))
			std::cout << "Secondary Surface Creation Failed with HRESULT: " << std::hex << (DWORD)result << std::endl;
		return result;
	}



	HRESULT WINAPI hCreateSurface(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter) 
	{
		if (lpDDSurfaceDesc == nullptr || lplpDDSurface == nullptr)
			return E_POINTER;

		HRESULT result = E_FAIL;
		if ((lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) && (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_FLIP) && (lpDDSurfaceDesc->dwFlags & DDSD_CAPS) && (lpDDSurfaceDesc->dwFlags & DDSD_BACKBUFFERCOUNT) && lpDDSurfaceDesc->dwBackBufferCount==1)
		{
			
			
			if (SUCCEEDED(CreatePrimarySurface(lplpDD)) && SUCCEEDED(CreateSecondarySurface(lplpDD)))
			{
				HookSurfaceVtable(EqMainHooks->PrimarySurface);
				HookSurfaceVtable(EqMainHooks->SecondarySurface);
				LPDIRECTDRAWCLIPPER lpClipper;
				lplpDD->CreateClipper(0, &lpClipper, NULL);
				lpClipper->SetHWnd(0, EqWindowed::Wnd->Handle);
				RECT clipRect = { 0, 0, EqMainHooks->res.width, EqMainHooks->res.height }; // x, y, width, height
				EqMainHooks->PrimarySurface->SetClipper(lpClipper);
				lpClipper->Release();
				std::cout << "Surfaces created!" << std::endl;
				*lplpDDSurface = EqMainHooks->PrimarySurface;
				return DD_OK;
			}
			else
				return E_FAIL;
		}

		DDSURFACEDESC surface_desc;
		ZeroMemory(&surface_desc, sizeof(DDSURFACEDESC));
		memcpy(&surface_desc, lpDDSurfaceDesc, sizeof(DDSURFACEDESC));

		if (!(surface_desc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
		{
			if (!(surface_desc.dwFlags & DDSD_PIXELFORMAT))
			{
				surface_desc.dwFlags |= DDSD_PIXELFORMAT;
				SetPixelFormat(surface_desc.ddpfPixelFormat);
			}
			surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		}


		result = EqMainHooks->hook_CreateSurface.original(hCreateSurface)(lplpDD, &surface_desc, lplpDDSurface, pUnkOuter);
		if (!SUCCEEDED(result)) 
			std::cout << "Surface Creation Failed with HRESULT: " << std::hex << (DWORD)result << std::endl;
		HookSurfaceVtable(*lplpDDSurface);
		
		return result;

	}
	HRESULT WINAPI hGetDisplayMode(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc)
	{
		std::cout << "Get display mode" << std::endl;
		HRESULT result = EqMainHooks->hook_GetDisplayMode.original(hGetDisplayMode)(lplpDD, lpDDSurfaceDesc);
		SetPixelFormat(lpDDSurfaceDesc->ddpfPixelFormat);
		return result;
	}

	void EqMain::InitDDraw(IDirectDraw* DDraw)
	{
		void** vtable = *(void***)DDraw;
		hook_SetCooperativeLevel = VTableHook(vtable, 20, hSetCooperativeLevel);
		hook_SetDisplayMode= VTableHook(vtable, 21, hSetDisplayMode);
		hook_CreateSurface = VTableHook(vtable, 6, hCreateSurface);
		hook_GetDisplayMode = VTableHook(vtable, 12, hGetDisplayMode);
	}

	
	BOOL WINAPI hGetCursorPos(LPPOINT pt)
	{
		if (pt)
		{
			POINT cpt;
			EqMainHooks->hook_GetCursorPos.original(hGetCursorPos)(&cpt);
			ScreenToClient(Wnd->Handle, &cpt);
			pt->x = cpt.x;
			pt->y = cpt.y;
			return true;
		}
		return false;
		
	}
	BOOL WINAPI hClientToScreen(HWND wnd, LPPOINT pt)
	{
		if (wnd == Wnd->Handle)
		{
			EqMainHooks->hook_ClientToScreen.original(hClientToScreen)(wnd, pt);
			pt->x -= Wnd->X;
			pt->y -= Wnd->Y;
		}
		return true;
	}
	
    EqMain::EqMain(HMODULE handle)
    {
		Console::CreateConsole();
        hook_CreateDirectDraw = IATHook(handle, "ddraw.dll", "DirectDrawCreate", hDirectDrawCreate);
		hook_CreateWindow = IATHook(handle, "user32.dll", "CreateWindowExA", hCreateWindowEx);
		hook_SetWindowPos = IATHook(handle, "user32.dll", "SetWindowPos", hSetWindowPos);
		hook_SetCapture = IATHook(handle, "user32.dll", "SetCapture", hSetCapture);
	    hook_SetWindowLongA = IATHook(handle, "user32.dll", "SetWindowLongA", hSetWindowLongA);
		hook_DestroyWindow = IATHook(handle, "user32.dll", "DestroyWindow", hDestroyWindow);
		hook_GetCursorPos = IATHook(handle, "user32.dll", "GetCursorPos", hGetCursorPos);
		hook_ClientToScreen = IATHook(handle, "user32.dll", "ClientToScreen", hClientToScreen);
		DInput->init(handle);
    }
   
}