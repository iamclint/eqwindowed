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
        EqGame(HMODULE handle);
    };
}
