#include "pch.h"
#include "EqGame.h"
#include "EqWindowed.h"
#include <TlHelp32.h>
#include "Console.h"
namespace EqWindowed
{
    HWND WINAPI hgCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
    {
        std::cout << "Create window ex hook from eqgame" << std::endl;
        return EqWindowed::Wnd->Handle;
        //dwStyle = WS_OVERLAPPEDWINDOW;
       // HWND rval = EqGameHooks->hook_CreateWindow.original(hgCreateWindowEx)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
        //return rval;
    }
    HMODULE WINAPI hLoadLibraryA(LPCSTR lpLibFileName)
    {
        HMODULE hmod = EqGameHooks->hook_LoadLibrary.original(hLoadLibraryA)(lpLibFileName);
        if (hmod)
        {
            if (!_stricmp(lpLibFileName, "eqmain.dll"))
            {
                EqMainHooks = new EqMain(hmod);
            }
            if (!_stricmp(lpLibFileName, "eqgfx_dx8.dll"))
            {
                EqGFXHooks = new EqGFX(hmod);
            }
        }
        return hmod;
    }

    EqGame::EqGame(HMODULE handle)
    {
        Console::CreateConsole();
        hook_LoadLibrary = IATHook(handle, "kernel32.dll", "LoadLibraryA", hLoadLibraryA);
        hook_CreateWindow = IATHook(handle, "user32.dll", "CreateWindowExA", hgCreateWindowEx);
    }
}