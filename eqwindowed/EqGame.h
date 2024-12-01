#pragma once
#include "IATHook.h"
#include "VTableHook.h"
namespace EqWindowed
{
    HMODULE WINAPI hLoadLibraryA(LPCSTR lpLibFileName);
    class EqGame
    {
    public:
        IATHook hook_LoadLibrary;
        IATHook hook_CreateWindow;
        IATHook hook_SetWindowPos;
        IATHook hook_SetCapture;
        IATHook hook_SetWindowLongA;
        IATHook hook_ShowWindow;
        IATHook hook_DestroyWindow;
        EqGame(HMODULE handle);
    };
}
