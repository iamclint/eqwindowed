#pragma once
#include "IATHook.h"
#include "VTableHook.h"
namespace EqWindowed
{
    HMODULE WINAPI hLoadLibraryA(LPCSTR lpLibFileName);
    bool WINAPI hFreeLibrary(HMODULE mod);
    class EqGame
    {
    public:
        IATHook hook_LoadLibrary;
        IATHook hook_FreeLibrary;
        IATHook hook_CreateWindow;
        IATHook hook_SetWindowPos;
        IATHook hook_SetCapture;
        IATHook hook_SetWindowLongA;
        IATHook hook_ShowWindow;
        IATHook hook_DestroyWindow;
        IATHook hook_RegisterClass;
        IATHook hook_DirectInput;
        IATHook hook_ExitProcess;
        EqGame(HMODULE handle);
    };
}
