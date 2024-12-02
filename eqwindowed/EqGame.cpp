#include "pch.h"
#include "EqGame.h"
#include "EqWindowed.h"
#include <TlHelp32.h>
#include "Console.h"
namespace EqWindowed
{
    BOOL WINAPI hDestroyWindow(HWND wnd)
    {
        return true;
    }
    //eqgame.exe tries to maximize the window it creates we don't want that
    bool WINAPI hShowWindow(HWND wnd, DWORD flags)
    {
        return ShowWindow(wnd, 1);
    }
    LONG WINAPI hSetWindowLongA(HWND wnd, int index, long dwNewLong)
    {
        if (index == -0x4)
        {
            SetWindowLongA(wnd, -0x15, dwNewLong);
        }
        return GetWindowLongA(wnd, index);
    }
    HWND WINAPI hSetCapture(HWND hWnd)
    {
        std::cout << "SetCapture" << std::endl;
        return SetCapture(hWnd);
    }
    HRESULT WINAPI hSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
    {
        std::cout << "SetWindowPos " << X << " " << Y << " " << cx << " " << cy << std::endl;
        return 1;
    }
    HWND WINAPI hCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
    {
        //if ((dwStyle & WS_CHILD))
        //    dwStyle &= ~WS_CHILD;
        //if ((dwExStyle & WS_EX_TOPMOST))
        //    dwExStyle &= ~WS_EX_TOPMOST;
        //std::cout << "Create window" << std::endl;
        std::cout << "Crete Window " << X << " " << Y << " " << nWidth << " " << nHeight << std::endl;
        return EqWindowed::Wnd->Handle;
       // dwStyle = WS_OVERLAPPEDWINDOW;
        //HWND rval = EqGameHooks->hook_CreateWindow.original(hCreateWindowEx)(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
      // return rval;
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
        hook_CreateWindow = IATHook(handle, "user32.dll", "CreateWindowExA", hCreateWindowEx);
        hook_SetWindowPos = IATHook(handle, "user32.dll", "SetWindowPos", hSetWindowPos);
        hook_SetCapture = IATHook(handle, "user32.dll", "SetCapture", hSetCapture);
        hook_SetWindowLongA = IATHook(handle, "user32.dll", "SetWindowLongA", hSetWindowLongA);
        hook_ShowWindow = IATHook(handle, "user32.dll", "ShowWindow", hShowWindow);
        hook_DestroyWindow = IATHook(handle, "user32.dll", "DestroyWindow", hDestroyWindow);
    }
}