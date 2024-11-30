#pragma once
#include "EqWindow.h"
#include "IATHook.h"
#include "VTableHook.h"
#include <ddraw.h>
#include <vector>
#include <unordered_map>
namespace EqWindowed
{

    //struct CustomDirectDrawVtbl {
    //    /*** IUnknown methods ***/
    //    HRESULT(WINAPI* QueryInterface)(IDirectDraw* lplpDD, REFIID riid, LPVOID FAR* ppvObj);
    //    ULONG(WINAPI* AddRef)(IDirectDraw* lplpDD);
    //    ULONG(WINAPI* Release)(IDirectDraw* lplpDD);

    //    /*** IDirectDraw methods ***/
    //    HRESULT(WINAPI* Compact)(IDirectDraw* lplpDD);
    //    HRESULT(WINAPI* CreateClipper)(IDirectDraw* lplpDD, DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR* lplpClipper, IUnknown FAR* pUnkOuter);
    //    HRESULT(WINAPI* CreatePalette)(IDirectDraw* lplpDD, DWORD dwFlags, LPPALETTEENTRY lpColorTable, LPDIRECTDRAWPALETTE FAR* lplpPalette, IUnknown FAR* pUnkOuter);
    //    HRESULT(WINAPI* CreateSurface)(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR* lplpDDSurface, IUnknown FAR* pUnkOuter);
    //    HRESULT(WINAPI* DuplicateSurface)(IDirectDraw* lplpDD, LPDIRECTDRAWSURFACE lpDDSurface, LPDIRECTDRAWSURFACE FAR* lplpDupSurface);
    //    HRESULT(WINAPI* EnumDisplayModes)(IDirectDraw* lplpDD, DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback);
    //    HRESULT(WINAPI* EnumSurfaces)(IDirectDraw* lplpDD, DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback);
    //    HRESULT(WINAPI* FlipToGDISurface)(IDirectDraw* lplpDD);
    //    HRESULT(WINAPI* GetCaps)(IDirectDraw* lplpDD, LPDDCAPS lpDDCaps, LPDDCAPS lpDDHELCaps);
    //    HRESULT(WINAPI* GetDisplayMode)(IDirectDraw* lplpDD, LPDDSURFACEDESC lpDDSurfaceDesc);
    //    HRESULT(WINAPI* GetFourCCCodes)(IDirectDraw* lplpDD, LPDWORD lpNumCodes, LPDWORD lpFourCCCodes);
    //    HRESULT(WINAPI* GetGDISurface)(IDirectDraw* lplpDD, LPDIRECTDRAWSURFACE FAR* lplpSurface);
    //    HRESULT(WINAPI* GetMonitorFrequency)(IDirectDraw* lplpDD, LPDWORD lpdwFrequency);
    //    HRESULT(WINAPI* GetScanLine)(IDirectDraw* lplpDD, LPDWORD lpdwScanLine);
    //    HRESULT(WINAPI* GetVerticalBlankStatus)(IDirectDraw* lplpDD, LPBOOL lpbIsInVBlank);
    //    HRESULT(WINAPI* Initialize)(IDirectDraw* lplpDD, GUID FAR* lpGUID);
    //    HRESULT(WINAPI* RestoreDisplayMode)(IDirectDraw* lplpDD);
    //    HRESULT(WINAPI* SetCooperativeLevel)(IDirectDraw* lplpDD, HWND hWnd, DWORD dwFlags);
    //    HRESULT(WINAPI* SetDisplayMode)(IDirectDraw* lplpDD, DWORD dwWidth, DWORD dwHeight, DWORD dwBpp);
    //    HRESULT(WINAPI* WaitForVerticalBlank)(IDirectDraw* lplpDD, DWORD dwFlags, HANDLE hEvent);
    //};

    //struct CustomDirectDraw {
    //    CustomDirectDrawVtbl* lpVtbl;
    //    IDirectDraw* originalDDraw;  // Pointer to the original IDirectDraw instance
    //};
    // Struct to manage the original vtable and the hook

    
    HRESULT WINAPI hDirectDrawCreate(GUID FAR* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown FAR* pUnkOuter);
    class EqMain
    {
    public:
        IDirectDraw* originalDirectDraw;
        IDirectDrawSurface* PrimarySurface = nullptr;
        IDirectDrawSurface* SecondarySurface = nullptr;
        IATHook hook_CreateDirectDraw;
        IATHook hook_CreateWindow;
        VTableHook hook_SetCooperativeLevel;
        VTableHook hook_SetDisplayMode;
        VTableHook hook_CreateSurface;
        VTableHook hook_Flip;
        VTableHook hook_GetAttachedSurface;
        EqMain(HMODULE handle);
        void InitDDraw(IDirectDraw* lplpDD);
    private:
    };
}