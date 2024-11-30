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
	HWND WINAPI hCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		std::cout << "Create window ex hook" << std::endl;
		//dwStyle = WS_OVERLAPPEDWINDOW;
		return EqWindowed::Wnd->Handle;
		//HWND rval = EqMainHooks->hook_CreateWindow.original(hCreateWindowEx)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		//return rval;
	}

    HRESULT WINAPI hDirectDrawCreate(GUID FAR* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown FAR* pUnkOuter)
    {
		void* caller_address = nullptr;

		// Use inline assembly to get the return address
		__asm {
			mov eax, [ebp + 4]   // Get the return address from the stack frame
			mov caller_address, eax
		}
		std::cout << "DirectDrawCreate function called: 0x" << std::hex << caller_address << std::endl;
		HRESULT rval = EqMainHooks->hook_CreateDirectDraw.original(hDirectDrawCreate)(lpGUID, lplpDD, pUnkOuter);
		if (lplpDD)
		{
			EqMainHooks->InitDDraw((*lplpDD));
		}
		return rval;
    }
	
	HRESULT WINAPI hSetCooperativeLevel(IDirectDraw* lplpDD, HWND hWnd, DWORD dwFlags) 
	{
		std::cout << "Set cooperative level" << std::endl;
		hWnd = EqWindowed::Wnd->Handle;
		dwFlags = DDSCL_NORMAL;
		return EqMainHooks->hook_SetCooperativeLevel.original(hSetCooperativeLevel)(lplpDD, hWnd, dwFlags);
	}

	HRESULT WINAPI hSetDisplayMode(IDirectDraw* lplpDD, DWORD dwWidth, DWORD dwHeight, DWORD dwBpp) 
	{
		std::cout << "Set Display Mode Width: " << std::dec << dwWidth << " Height: " << dwHeight << " dwBPP " << dwBpp << std::endl;
		dwWidth = 1024;
		dwHeight = 768;
		return EqMainHooks->hook_SetDisplayMode.original(hSetDisplayMode)(lplpDD, dwWidth, dwHeight, dwBpp);
	}

	//HDC buff_dc = 0;
//HDC wnd_dc = GetDC(EqWindowed::Wnd->Handle);
	//EqWindowed::EqMainHooks->SecondarySurface->GetDC(&buff_dc);

	//BitBlt(wnd_dc, 0, 0, 1024, 768, buff_dc, 0, 0, 0xCC0020);
//EqWindowed::EqMainHooks->SecondarySurface->ReleaseDC(buff_dc);
//ReleaseDC(EqWindowed::Wnd->Handle, wnd_dc);

//EqMainHooks->SecondarySurface->Restore();
//EqMainHooks->PrimarySurface->Restore();
	HRESULT WINAPI hFlip(IDirectDrawSurface* surface, DWORD flags)
	{
		if (surface == EqMainHooks->PrimarySurface)
		{
			// Check if either surface is lost
			if (EqMainHooks->SecondarySurface->IsLost() == DDERR_SURFACELOST)
			{
				// Attempt to restore the secondary surface
				HRESULT result = EqMainHooks->SecondarySurface->Restore();
				if (result != DD_OK)
				{
					std::cout << "Failed to restore secondary surface: " << std::hex << result << std::endl;
					return result;  // Return the error code if restore fails
				}
			}

			if (EqMainHooks->PrimarySurface->IsLost() == DDERR_SURFACELOST)
			{
				// Attempt to restore the primary surface
				HRESULT result = EqMainHooks->PrimarySurface->Restore();
				if (result != DD_OK)
				{
					std::cout << "Failed to restore primary surface: " << std::hex << result << std::endl;
					return result;  // Return the error code if restore fails
				}
			}
		}
		if (surface == EqMainHooks->PrimarySurface)
		{
			if (!(EqMainHooks->SecondarySurface->IsLost()==DDERR_SURFACELOST) && !(EqMainHooks->PrimarySurface->IsLost() == DDERR_SURFACELOST))
			{

				RECT srcRect = { 0, 0, 1024, 768 }; // Source rectangle
				RECT destRect = { 0, 0, 1024, 768 }; // Destination rectangle (adjust as needed)

				HRESULT result = EqMainHooks->PrimarySurface->Blt(&destRect, EqMainHooks->SecondarySurface, &srcRect, DDBLT_WAIT, nullptr);
				if (result != DD_OK)
					std::cout << "BLT failed " << std::hex << result << std::endl;
			}
		}
		return EqMainHooks->hook_Flip.original(hFlip)(surface, flags);
	}
	HRESULT WINAPI hGetAttachedSurface(IDirectDrawSurface* surface, DDSCAPS* caps, LPDIRECTDRAWSURFACE* backbuffer)
	{
		//if (surface == EqMainHooks->PrimarySurface) //just manually control our backbuffer
		//{
		//	*backbuffer = EqMainHooks->SecondarySurface;
		//	return DD_OK;
		//}

		HRESULT result =  EqMainHooks->hook_GetAttachedSurface.original(hGetAttachedSurface)(surface, caps, backbuffer);

		if (!SUCCEEDED(result))
			std::cout << "GetAttachedSurface Failed HRESULT: " << std::hex << (DWORD)result << std::endl;
		else
			std::cout << "GetAttachedSurface Succeeded " << std::endl;
		return result;
	}

	HRESULT WINAPI hCreateSurface(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR* lplpDDSurface, IUnknown FAR* pUnkOuter) 
	{
		std::cout << "Create Surface" << std::endl;
		if (lpDDSurfaceDesc == nullptr || lplpDDSurface == nullptr)
			return E_POINTER;
		HRESULT result = E_FAIL;
		if ((lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) && (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_FLIP) && (lpDDSurfaceDesc->dwFlags & DDSD_CAPS))
		{
			lpDDSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);
			lpDDSurfaceDesc->dwFlags = DDSD_CAPS;
			lpDDSurfaceDesc->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

			result = EqMainHooks->hook_CreateSurface.original(hCreateSurface)(lplpDD, lpDDSurfaceDesc, &EqMainHooks->PrimarySurface, pUnkOuter); //create the primary surface
			if (!SUCCEEDED(result))
				std::cout << "Primary Surface Creation Failed with HRESULT: " << std::hex << (DWORD)result << std::endl;
			else
				std::cout << "Primary Surface Creation Succeeded " << std::endl;

			void** vtable = *(void***)(EqMainHooks->PrimarySurface);
			EqMainHooks->hook_GetAttachedSurface = VTableHook(vtable, 12, hGetAttachedSurface, true);
			EqMainHooks->hook_Flip = VTableHook(vtable, 11, hFlip, true);

			LPDIRECTDRAWCLIPPER lpClipper;
			lplpDD->CreateClipper(0, &lpClipper, NULL);
			RECT clipRect = { 0, 0, 1024, 768 }; // x, y, width, height
			EqMainHooks->PrimarySurface->SetClipper(lpClipper);

			ZeroMemory(lpDDSurfaceDesc, sizeof(DDSURFACEDESC));
			lpDDSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);
			lpDDSurfaceDesc->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY; //create a secondary surface
			lpDDSurfaceDesc->dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;//| DDSD_WIDTH | DDSD_HEIGHT | ;
			lpDDSurfaceDesc->dwWidth = 1024;
			lpDDSurfaceDesc->dwHeight = 768;
			// Set a default pixel format (use DDPF_RGB for example)
			lpDDSurfaceDesc->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			lpDDSurfaceDesc->ddpfPixelFormat.dwFlags = DDPF_RGB;
			lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = 32;  // Example, 32-bit color depth
			lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0x00FF0000; // Red mask (for RGB)
			lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask = 0x0000FF00; // Green mask
			lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask = 0x000000FF; // Blue mask
			result = EqMainHooks->hook_CreateSurface.original(hCreateSurface)(lplpDD, lpDDSurfaceDesc, &EqMainHooks->SecondarySurface, pUnkOuter);


			if (!SUCCEEDED(result)) 
				std::cout << "Secondary Surface Creation Failed with HRESULT: " << std::hex << (DWORD)result << std::endl;
			else
				std::cout << "Secondary Surface Creation Succeeded " << std::endl;
			
			HRESULT hr = EqMainHooks->PrimarySurface->AddAttachedSurface(EqMainHooks->SecondarySurface);
			if (hr != DD_OK) {
				std::cout << "Failed to attach back buffer to primary surface " << std::hex << hr << std::endl;
			}
			
			*lplpDDSurface = EqMainHooks->PrimarySurface;
			return result;
		}
		result = EqMainHooks->hook_CreateSurface.original(hCreateSurface)(lplpDD, lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);


		if (!SUCCEEDED(result)) {
			std::cout << "Surface Creation Failed with HRESULT: " << std::hex << (DWORD)result << std::endl;
		}
		return result;

	}

	void EqMain::InitDDraw(IDirectDraw* DDraw)
	{
		void** vtable = *(void***)DDraw;
		hook_SetCooperativeLevel = VTableHook(vtable, 20, hSetCooperativeLevel);
		hook_SetDisplayMode= VTableHook(vtable, 21, hSetDisplayMode);
		hook_CreateSurface = VTableHook(vtable, 6, hCreateSurface);
	}
    EqMain::EqMain(HMODULE handle)
    {
		Console::CreateConsole();
        hook_CreateDirectDraw = IATHook(handle, "ddraw.dll", "DirectDrawCreate", hDirectDrawCreate);
		hook_CreateWindow = IATHook(handle, "user32.dll", "CreateWindowExA", hCreateWindowEx);
    }
   
}



//HRESULT WINAPI hQueryInterface(IDirectDraw* lplpDD, REFIID riid, LPVOID FAR* ppvObj) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->QueryInterface(lplpDD, riid, ppvObj);  // Call the original method
	//}

	//ULONG WINAPI hAddRef(IDirectDraw* lplpDD) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->AddRef(lplpDD);  // Call the original method
	//}

	//ULONG WINAPI hRelease(IDirectDraw* lplpDD) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->Release(lplpDD);  // Call the original method
	//}

	//HRESULT WINAPI hCompact(IDirectDraw* lplpDD) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->Compact(lplpDD);  // Call the original method
	//}

	//HRESULT WINAPI hCreateClipper(IDirectDraw* lplpDD, DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpClipper, IUnknown FAR* pUnkOuter) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->CreateClipper(lplpDD, dwFlags, lplpClipper, pUnkOuter);  // Call the original method
	//}

	//HRESULT WINAPI hCreatePalette(IDirectDraw* lplpDD, DWORD dwFlags, LPPALETTEENTRY lpColorTable, LPDIRECTDRAWPALETTE FAR* lplpPalette, IUnknown FAR* pUnkOuter) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->CreatePalette(lplpDD, dwFlags, lpColorTable, lplpPalette, pUnkOuter);  // Call the original method
	//}

	//HRESULT WINAPI hCreateSurface(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR* lplpDDSurface, IUnknown FAR* pUnkOuter) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->CreateSurface(lplpDD, lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);  // Call the original method
	//}

	//HRESULT WINAPI hDuplicateSurface(IDirectDraw* lplpDD, LPDIRECTDRAWSURFACE lpDDSurface, LPDIRECTDRAWSURFACE FAR* lplpDupSurface) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->DuplicateSurface(lplpDD, lpDDSurface, lplpDupSurface);  // Call the original method
	//}

	//HRESULT WINAPI hEnumDisplayModes(IDirectDraw* lplpDD, DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->EnumDisplayModes(lplpDD, dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);  // Call the original method
	//}

	//HRESULT WINAPI WINAPI hEnumSurfaces(IDirectDraw* lplpDD, DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->EnumSurfaces(lplpDD, dwFlags, lpDDSurfaceDesc, lpContext, lpEnumSurfacesCallback);  // Call the original method
	//}

	//HRESULT WINAPI hFlipToGDISurface(IDirectDraw* lplpDD) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->FlipToGDISurface(lplpDD);  // Call the original method
	//}

	//HRESULT WINAPI hGetCaps(IDirectDraw* lplpDD, LPDDCAPS lpDDCaps, LPDDCAPS lpDDHELCaps) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->GetCaps(lplpDD, lpDDCaps, lpDDHELCaps);  // Call the original method
	//}

	//HRESULT WINAPI hGetDisplayMode(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->GetDisplayMode(lplpDD, lpDDSurfaceDesc);  // Call the original method
	//}

	//HRESULT WINAPI hGetFourCCCodes(IDirectDraw* lplpDD, LPDWORD lpNumCodes, LPDWORD lpFourCCCodes) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->GetFourCCCodes(lplpDD, lpNumCodes, lpFourCCCodes);  // Call the original method
	//}

	//HRESULT WINAPI hGetGDISurface(IDirectDraw* lplpDD, LPDIRECTDRAWSURFACE FAR* lplpSurface) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->GetGDISurface(lplpDD, lplpSurface);  // Call the original method
	//}

	//HRESULT WINAPI hGetMonitorFrequency(IDirectDraw* lplpDD, LPDWORD lpdwFrequency) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->GetMonitorFrequency(lplpDD, lpdwFrequency);  // Call the original method
	//}

	//HRESULT WINAPI hGetScanLine(IDirectDraw* lplpDD, LPDWORD lpdwScanLine) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->GetScanLine(lplpDD, lpdwScanLine);  // Call the original method
	//}

	//HRESULT WINAPI hGetVerticalBlankStatus(IDirectDraw* lplpDD, LPBOOL lpbIsInVBlank) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->GetVerticalBlankStatus(lplpDD, lpbIsInVBlank);  // Call the original method
	//}

	//HRESULT WINAPI hInitialize(IDirectDraw* lplpDD, GUID FAR* lpGUID) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->Initialize(lplpDD, lpGUID);  // Call the original method
	//}

	//HRESULT WINAPI hRestoreDisplayMode(IDirectDraw* lplpDD) {
	//	// Add custom logic here if needed
	//	return EqMainHooks->originalVTable->RestoreDisplayMode(lplpDD);  // Call the original method
	//}
