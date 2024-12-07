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
        return EqGameHooks->hook_ShowWindow.original(hShowWindow)(wnd, 1);
    }
    LONG WINAPI hSetWindowLongA(HWND wnd, int index, long dwNewLong)
    {
        if (index == -0x4)
        {
            Wnd->eqMainWndProc = (void*)dwNewLong;
            EqMainHooks->hook_SetWindowLongA.original(hSetWindowLongA)(wnd, -0x15, dwNewLong);
        }
        return GetWindowLongA(wnd, index);
    }
    HWND WINAPI hSetCapture(HWND hWnd)
    {
        std::cout << "SetCapture" << std::endl;
        ReleaseCapture();
        return 0;
        //return EqGameHooks->hook_SetCapture.original(hSetCapture)(hWnd);
    }
    HWND WINAPI hSetFocus(HWND hWnd)
    {
        std::cout << "SetFocus" << std::endl;
        return hWnd;
        //return EqGameHooks->hook_SetCapture.original(hSetCapture)(hWnd);
    }
    HRESULT WINAPI hSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
    {
        std::cout << "SetWindowPos " << X << " " << Y << " " << cx << " " << cy << std::endl;
        return 1;
    }
    HWND WINAPI hCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
    {
        std::cout << "Create Window " << X << " " << Y << " " << nWidth << " " << nHeight << std::endl;
        return EqWindowed::Wnd->Handle;
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
    ATOM WINAPI hRegisterClass(WNDCLASSA* cls_data)
    {
        if (cls_data->hCursor)
            cls_data->hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
        return EqGameHooks->hook_RegisterClass.original(hRegisterClass)(cls_data);
    }

    EqGame::EqGame(HMODULE handle)
    {
        Console::CreateConsole();
        hook_LoadLibrary = IATHook(handle, "kernel32.dll", "LoadLibraryA", hLoadLibraryA);
        hook_CreateWindow = IATHook(handle, "user32.dll", "CreateWindowExA", hCreateWindowEx);
        hook_SetWindowPos = IATHook(handle, "user32.dll", "SetWindowPos", hSetWindowPos);
        hook_SetCapture = IATHook(handle, "user32.dll", "SetCapture", hSetCapture);
        hook_ShowWindow = IATHook(handle, "user32.dll", "ShowWindow", hShowWindow);
        hook_DestroyWindow = IATHook(handle, "user32.dll", "DestroyWindow", hDestroyWindow);
        hook_RegisterClass = IATHook(handle, "user32.dll", "RegisterClassA", hRegisterClass);
        DInput->init(handle);
    }
}