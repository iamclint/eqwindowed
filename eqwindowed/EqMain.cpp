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
//	HWND WINAPI hSetCapture(HWND hWnd)
//	{
//		std::cout << "SetCapture from eqmain.dll" << std::endl;
//		return 0;
//	}
//	HRESULT WINAPI hSetWindowPos(HWND hWnd,HWND hWndInsertAfter,int X,int Y,int cx,int cy,UINT uFlags)
//	{
//		std::cout << "SetWindowPos from eqmain.dll" << std::endl;
//		return 1;
//	}
//	/*
//	* Return our hwnd here for whatever window eqmain is trying to create, 
//	*/
//	HWND WINAPI hCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
//	{
//		std::cout << "CreateWindow from eqmain.dll" << std::endl;
//		if ((dwStyle & WS_CHILD))
//			dwStyle &= ~WS_CHILD;
//		if ((dwExStyle & WS_EX_TOPMOST))
//			dwExStyle &= ~WS_EX_TOPMOST;
////		std::cout << "Create window ex hook" << std::endl;
////		return EqWindowed::Wnd->Handle;
//		return EqMainHooks->hook_CreateWindow.original(hCreateWindowEx)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
//	}

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
		std::cout << "Set cooperative level " << hWnd << " == " << Wnd->Handle << " " << ((int)hWnd==(int)Wnd->Handle) << std::endl;
		 
		HRESULT res = EqMainHooks->hook_SetCooperativeLevel.original(hSetCooperativeLevel)(lplpDD, hWnd, DDSCL_NORMAL);
		if (hWnd)
		{
			SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, EqMainHooks->res.width, EqMainHooks->res.height, SWP_SHOWWINDOW);
		}
		return res;
	}

	HRESULT WINAPI hSetDisplayMode(IDirectDraw* lplpDD, DWORD dwWidth, DWORD dwHeight, DWORD dwBpp) 
	{
		std::cout << "Set Display Mode Width: " << std::dec << dwWidth << " Height: " << dwHeight << " dwBPP " << dwBpp << std::endl;
		dwWidth = EqMainHooks->res.width;
		dwHeight = EqMainHooks->res.height;
		dwBpp = 16;
		return EqMainHooks->hook_SetDisplayMode.original(hSetDisplayMode)(lplpDD, dwWidth, dwHeight, dwBpp);
	}

	HRESULT WINAPI hFlip(IDirectDrawSurface* surface, DWORD flags)
	{
		static int i = 0;
		HRESULT result = EqMainHooks->dd->WaitForVerticalBlank(1, 0);
		if (!(EqMainHooks->SecondarySurface->IsLost()==DDERR_SURFACELOST) && !(EqMainHooks->PrimarySurface->IsLost() == DDERR_SURFACELOST))
		{
			RECT srcRect = { 0, 0, EqMainHooks->res.width, EqMainHooks->res.height }; // Source rectangle
			RECT destRect = { 0, 0, EqMainHooks->res.width, EqMainHooks->res.height }; // Destination rectangle (adjust as needed)
			HRESULT result = surface->Blt(&destRect, EqMainHooks->SecondarySurface, &srcRect, DDBLT_WAIT, nullptr);
			if (result != DD_OK)
				std::cout << "BLT failed " << std::hex << result << std::endl;
		}
		return EqMainHooks->hook_Flip.original(hFlip)(surface, flags);
	}
	HRESULT WINAPI hGetAttachedSurface(IDirectDrawSurface* surface, DDSCAPS* caps, LPDIRECTDRAWSURFACE* backbuffer)
	{
		if (surface == EqMainHooks->PrimarySurface) 
		{
			*backbuffer = EqMainHooks->SecondarySurface;
			std::cout << "Returning secondary surface for attached surface:  0x" << std::hex << EqMainHooks->SecondarySurface << " " << std::endl;
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
		data.dwSize = sizeof(DDPIXELFORMAT);
		data.dwFlags = DDPF_RGB;
		data.dwRGBBitCount = 16; // Match primary surface bit depth
		data.dwRBitMask = 0xF800;  // 5 bits for Red (RGB 5-6-5)
		data.dwGBitMask = 0x07E0;  // 6 bits for Green (RGB 5-6-5)
		data.dwBBitMask = 0x001F;  // 5 bits for Blue (RGB 5-6-5)
	}
	
	void HookSurfaceVtable(IDirectDrawSurface* surface)
	{
		void** vtable = *(void***)(surface);
		EqMainHooks->hook_GetAttachedSurface = VTableHook(vtable, 12, hGetAttachedSurface, true);
		EqMainHooks->hook_Flip = VTableHook(vtable, 11, hFlip, true);


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
		else
			std::cout << "Primary Surface Creation Succeeded " << std::endl;

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
		else
			std::cout << "Secondary Surface Creation Succeeded " << std::endl;


		return result;
	}



	HRESULT WINAPI hCreateSurface(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR* lplpDDSurface, IUnknown FAR* pUnkOuter) 
	{
		std::cout << "Create Surface" << std::endl;
		if (lpDDSurfaceDesc == nullptr || lplpDDSurface == nullptr)
			return E_POINTER;

		HRESULT result = E_FAIL;
		if ((lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) && (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_FLIP) && (lpDDSurfaceDesc->dwFlags & DDSD_CAPS) && lpDDSurfaceDesc->dwBackBufferCount==1)
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

		if ((lpDDSurfaceDesc->ddsCaps.dwCaps & 0x200) == 0)
		{
			if ((lpDDSurfaceDesc->dwFlags & 0x1000) == 0)
			{
				lpDDSurfaceDesc->dwFlags |= 0x10;
				SetPixelFormat(lpDDSurfaceDesc->ddpfPixelFormat);
			}
			lpDDSurfaceDesc->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		}
		
		result = EqMainHooks->hook_CreateSurface.original(hCreateSurface)(lplpDD, lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
		if (!SUCCEEDED(result)) {
			std::cout << "Surface Creation Failed with HRESULT: " << std::hex << (DWORD)result << std::endl;
		}
		HookSurfaceVtable(*lplpDDSurface);
		(*lplpDDSurface)->GetSurfaceDesc(lpDDSurfaceDesc);
		return result;

	}
	HRESULT WINAPI hGetDisplayMode(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc)
	{
		std::cout << "Get display mode" << std::endl;
		HRESULT result = EqMainHooks->hook_GetDisplayMode.original(hGetDisplayMode)(lplpDD, lpDDSurfaceDesc);
		SetPixelFormat(lpDDSurfaceDesc->ddpfPixelFormat);
		return result;
	}
	HRESULT WINAPI hQueryInterface(IDirectDraw* lplpDD, REFIID riid, LPVOID FAR* ppvObj)
	{
		std::cout << "Query Interface" << std::endl;
		HRESULT result = EqMainHooks->hook_QueryInterface.original(hQueryInterface)(lplpDD, riid, ppvObj);
		if (SUCCEEDED(result))
		{

		}
		return result;
	}
	void EqMain::InitDDraw(IDirectDraw* DDraw)
	{
		void** vtable = *(void***)DDraw;
		hook_SetCooperativeLevel = VTableHook(vtable, 20, hSetCooperativeLevel);
		hook_SetDisplayMode= VTableHook(vtable, 21, hSetDisplayMode);
		hook_CreateSurface = VTableHook(vtable, 6, hCreateSurface);
		hook_GetDisplayMode = VTableHook(vtable, 12, hGetDisplayMode);
		hook_QueryInterface = VTableHook(vtable, 0, hQueryInterface);
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




		//if (EqMainHooks->PrimarySurface == nullptr || EqMainHooks->SecondarySurface == nullptr) {
		//	std::cerr << "Primary or Secondary surface is null!" << std::endl;
		//	return DDERR_INVALIDPARAMS;
		//}
		//DDSURFACEDESC desc;
		//ZeroMemory(&desc, sizeof(DDSURFACEDESC));
		//desc.dwSize = sizeof(DDSURFACEDESC);
		//if (EqMainHooks->SecondarySurface->GetSurfaceDesc(&desc) == DD_OK) {
		//	if (EqMainHooks->res.width > desc.dwWidth || EqMainHooks->res.height > desc.dwHeight) {
		//		std::cerr << "Source rectangle exceeds secondary surface bounds!" << std::endl;
		//		return DDERR_INVALIDRECT;
		//	}
		//}

		//if (surface == EqMainHooks->PrimarySurface)
		//{
		//	if (EqMainHooks->SecondarySurface->IsLost() == DDERR_SURFACELOST)
		//	{
		//		HRESULT result = EqMainHooks->SecondarySurface->Restore();
		//		if (result != DD_OK)
		//		{
		//			std::cout << "Failed to restore secondary surface: " << std::hex << result << std::endl;
		//			return result;  // Return the error code if restore fails
		//		}
		//	}

		//	if (EqMainHooks->PrimarySurface->IsLost() == DDERR_SURFACELOST)
		//	{
		//		HRESULT result = EqMainHooks->PrimarySurface->Restore();
		//		if (result != DD_OK)
		//		{
		//			std::cout << "Failed to restore primary surface: " << std::hex << result << std::endl;
		//			return result;  // Return the error code if restore fails
		//		}
		//	}
		//}

			//HDC ss_dc;
			//EqMainHooks->SecondarySurface->GetDC(&ss_dc);
			//HDC wnd_dc = GetDC(Wnd->Handle);
			//BitBlt(wnd_dc, 0, 0, EqMainHooks->res.width, EqMainHooks->res.height, ss_dc, 0,0, 0xcc0020);
			//EqMainHooks->SecondarySurface->ReleaseDC(ss_dc);
			//ReleaseDC(Wnd->Handle, wnd_dc);