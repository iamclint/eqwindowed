// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "eqwindowed.h"
#include "eqwindow.h"
#include "Console.h"
namespace EqWindowed
{
    EqGame* EqGameHooks = nullptr;
    EqMain* EqMainHooks = nullptr;
    EqGFX* EqGFXHooks = nullptr;
    EqWindow* Wnd = nullptr;
}



extern "C" __declspec(dllexport) void __stdcall InitFromEqClient() //this is set to ordinal 1 in the eqwindoweddef
{
    EqWindowed::Wnd = new EqWindowed::EqWindow();
    EqWindowed::EqGameHooks = new EqWindowed::EqGame(GetModuleHandleA(NULL));
}


void DestroyConsole() {
    FreeConsole();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        Console::CreateConsole("EqWindowed");
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    {
     //   DestroyConsole();
        //if (EqWindowed::Wnd)
        //    delete EqWindowed::Wnd;
        //if (EqWindowed::EqGameHooks)
        //    delete EqWindowed::EqGameHooks;
        break;
    }
    }
    return TRUE;
}

